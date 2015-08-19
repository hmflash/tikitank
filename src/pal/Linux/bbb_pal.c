#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <linux/spi/spidev.h>
#include <linux/types.h>
#include <sys/ioctl.h>
#include <time.h>
#include <libgen.h>
#include <usb.h>
#include <linux/i2c-dev.h>
#include <assert.h>

#ifndef I2C_SMBUS_READ
#include <linux/i2c.h>
#endif

#include <prussdrv.h>
#include <pruss_intc_mapping.h>

#include "firmware.h"
#include "common.h"
#include "renderer.h"
#include "effects/effects.h"

#define wordoffset(obj, member) (offsetof(obj, member) / sizeof(word))

// Need trailing NULL byte for every 32 pixels

#define LATCH_BYTES(x) (((x/3)+31)/32)

#define TREADS_LEN (NUM_TREADS + LATCH_BYTES(NUM_TREADS))
#define BARREL_LEN (NUM_BARREL + LATCH_BYTES(NUM_BARREL))

struct bbb_pal {
	struct pal p;

	int fd_treads;
	int fd_barrel;
	int fd_panels[2];

	// Handle to DMX board
	usb_dev_handle* dmx;

	// Base address of PRU RAM
	void* pru;

	char treads_buf1[TREADS_LEN];
	char barrel_buf1[BARREL_LEN];

	char panels_buf1[NUM_PANELS];
	char panels_buf2[NUM_PANELS];

	struct renderer panel;
};

static
struct bbb_pal pal;

static 
int load_cape(const char* path, const char* name) {
	FILE* fp;
	char* pos = NULL;
	char* line = NULL;
	size_t len = 0;
	ssize_t read;

	fp = fopen(path, "r");
	if (!fp)
		return -1;

	while ((read = getline(&line, &len, fp)) != -1 && !pos) {
		pos = strstr(line, name);
	}

	free(line);
	fclose(fp);

	if (pos) {
		LOG(("Cape '%s' already loaded\n", name));
		return 0;
	}

	fp = fopen(path, "w");
	if (!fp)
		return -1;

	fprintf(fp, "%s", name);
	fclose(fp);

	// Give cape 1 second to load
	usleep(1000000);

	LOG(("Cape '%s' loaded\n", name));
	return 0;
}

static 
int load_capes(const char* names[]) {
	DIR* d;
	char* path = NULL;
	int ret = 0;

	d = opendir("/sys/devices");
	if (!d)
		return -1;

	while (!path) {
		struct dirent* entry = readdir(d);

		if (!entry)
			break;

		if (strncmp(entry->d_name, "bone_capemgr.", 13))
			continue;

		path = malloc(20 + strlen(entry->d_name));
		sprintf(path, "/sys/devices/%s/slots", entry->d_name);
	}

	closedir(d);

	if (!path) {
		errno = ENOENT;
		return -1;
	}

	for (; *names && !ret; ++names) {
		ret = load_cape(path, *names);
	}

	free(path);
	return ret;
}

static 
const char* capes[] = {
	"BB-SPIDEV0",
	"BB-SPIDEV1",
	"BB-BONE-PRU-01",
	"BB-ADC",
	NULL
};

static 
void safe_close(int* fd) {
	if (*fd != -1) {
		close(*fd);
		*fd = -1;
	}
}

static 
int open_spi(const char* dev) {
	int fd = -1;
	uint32_t arg;

	// Open spi device
	fd = open(dev, O_RDWR, 0);
	if (fd == -1) {
		fprintf(stderr, "Error opening %s: %s\n",
			dev, strerror(errno));
		goto err_open_spi;
	}

	// LEDs require mode 0
	arg = SPI_MODE_0;
	if (ioctl(fd, SPI_IOC_WR_MODE, &arg) == -1) {
		fprintf(stderr, "Error setting %s mode: %s\n",
			dev, strerror(errno));
		goto err_open_spi;
	}

	// LEDs are 8 bits per word
	arg = 8;
	if (ioctl(fd, SPI_IOC_WR_BITS_PER_WORD, &arg) == -1) {
		fprintf(stderr, "Error setting %s BPW: %s\n",
			dev, strerror(errno));
		goto err_open_spi;
	}

	// Run LEDs at 4mhz
	arg = 4000000;
	if (ioctl(fd, SPI_IOC_WR_MAX_SPEED_HZ, &arg) == -1) {
		fprintf(stderr, "Error setting %s speed: %s\n",
			dev, strerror(errno));
		goto err_open_spi;
	}

	return fd;

err_open_spi:
	safe_close(&fd);
	return -1;
}

static
void* pru_init(unsigned int threshold, unsigned int delay, unsigned int ema_pow) {
	tpruss_intc_initdata pruss_intc_initdata = PRUSS_INTC_INITDATA;
	locals_t locals;
	int err;
	void* ptr;
	char* self;
	char firmware[PATH_MAX];

	err = prussdrv_init();
	if (err) {
		fprintf(stderr, "Failed to init PRUSSDRV driver: %s\n",
			strerror(errno));
		return NULL;
	}

	err = prussdrv_open(PRU_EVTOUT_0);
	if (err) {
		fprintf(stderr, "Failed to open PRU0 interrupt: %s\n",
			strerror(errno));
		goto err_open_pru;
	}

	err = prussdrv_pruintc_init(&pruss_intc_initdata);
	if (err) {
		fprintf(stderr, "Failed init PRU0 interrupt: %s\n",
			strerror(errno));
		goto err_open_pru;
	}

	memset(&locals, 0, sizeof(locals));
	locals.eyecatcher = EYECATCHER;
	locals.enc.encoder0 = 0; // ADC0
	locals.ema_pow =  ema_pow;
	locals.enc_local[0].threshold = threshold;
	locals.enc_local[0].delay = delay;
	locals.enc_local[0].speed = INITIAL_ACC_VAL;
	locals.enc_local[0].acc = INITIAL_ACC_VAL;

	err = prussdrv_pru_write_memory(PRUSS0_PRU0_DATARAM,
	                                0,
	                                (unsigned int*)&locals,
	                                sizeof(locals));
	if (err < 0) {
		fprintf(stderr, "Failed write PRU0 memory: %s\n",
			strerror(errno));
		goto err_open_pru;
	}

	err = prussdrv_map_prumem(PRUSS0_PRU0_DATARAM, &ptr);
	if (err < 0) {
		fprintf(stderr, "Failed map PRU0 memory: %s\n",
			strerror(errno));
		goto err_open_pru;
	}

	self = realpath("/proc/self/exe", NULL);
	if (!self) {
		fprintf(stderr, "Couldn't locate PRU firmware\n");
		goto err_open_pru;
	}

	strcpy(firmware, dirname(self));
	strcat(firmware, "/firmware.bin");
	free(self);

	err = prussdrv_exec_program(PRU0, firmware);
	if (err < 0) {
		fprintf(stderr, "Failed start PRU0 firmware: %s\n",
			strerror(errno));
		goto err_open_pru;
	}

	return ptr;

err_open_pru:
	prussdrv_exit();
	return NULL;
}

static
void pru_destroy() {
	unsigned int arg = 1;

	// Tell PRU to stop
	prussdrv_pru_write_memory(PRUSS0_PRU0_DATARAM,
	                          wordoffset(locals_t, flags),
	                          &arg,
	                          sizeof(arg));

	// Wait for PRU to say it is stopped
	prussdrv_pru_wait_event(PRU_EVTOUT_0);

	// Clear stop event
	prussdrv_pru_clear_event(PRU_EVTOUT_0, PRU0_ARM_INTERRUPT);

	// Disable PRU
	prussdrv_pru_disable(PRU0);

	// Close pru driver
	prussdrv_exit();
}

static int usbGetStringAscii(usb_dev_handle* dev, int index, int langid, char *buf, int buflen)
{
	char buffer[256];
	int  rval, i;

	rval = usb_control_msg(dev,
	                       USB_ENDPOINT_IN,
	                       USB_REQ_GET_DESCRIPTOR,
	                       (USB_DT_STRING << 8) + index,
	                       langid,
	                       buffer,
	                       sizeof(buffer),
	                       1000);

	if(rval < 0)
		return rval;

	if(buffer[1] != USB_DT_STRING)
		return 0;

	if ((unsigned char)buffer[0] < rval)
		rval = (unsigned char)buffer[0];

	rval /= 2;

	// lossy conversion to ISO Latin1
	for(i=1;i<rval;i++) {
		// destination buffer overflow
		if (i > buflen)
			break;

		buf[i-1] = buffer[2 * i];

		// outside of ISO Latin1 range
		if (buffer[2 * i + 1] != 0)
			buf[i-1] = '?';
	}

	buf[i-1] = 0;
	return i-1;
}

#define UDMX_VID 0x16C0  /* VOTI */
#define UDMX_PID 0x05DC  /* Obdev's free shared PID */
#define cmd_SetChannelRange 2

static usb_dev_handle* dmx_try_open(struct usb_device* dev) {
	usb_dev_handle *handle;
	char            buf[256];
	int             len;

	// we need to open the device in order to query strings
	handle = usb_open(dev);
	if (!handle) {
		fprintf(stderr, "Warning: cannot open USB device: %s\n", usb_strerror());
		return NULL;
	}

	// now find out whether the device actually is obdev's Remote Sensor:
	len = usbGetStringAscii(handle, dev->descriptor.iManufacturer, 0x0409, buf, sizeof(buf));
	if (len < 0) {
		fprintf(stderr, "Warning: cannot query manufacturer for device: %s\n", usb_strerror());
		goto skip_device;
	}

	if (strcmp(buf, "www.anyma.ch") != 0)
		goto skip_device;

	len = usbGetStringAscii(handle, dev->descriptor.iProduct, 0x0409, buf, sizeof(buf));
	if (len < 0) {
		fprintf(stderr, "Warning: cannot query product for device: %s\n", usb_strerror());
		goto skip_device;
	}

	if (strcmp(buf, "uDMX") == 0)
		return handle;

skip_device:
	usb_close(handle);
	return NULL;
}

static usb_dev_handle* dmx_find_device(void)
{
	struct usb_bus    *bus;
	struct usb_device *dev;
	usb_dev_handle    *handle;

	usb_init();
	usb_find_busses();
	usb_find_devices();

	for (bus=usb_busses; bus; bus=bus->next) {
		for (dev=bus->devices; dev; dev=dev->next) {
			if (dev->descriptor.idVendor != UDMX_VID)
				continue;
			if (dev->descriptor.idProduct != UDMX_PID)
				continue;
			if ((handle = dmx_try_open(dev)) != NULL)
				return handle;
		}
	}

	fprintf(stderr, "Could not find USB device www.anyma.ch/uDMX\n");

	return NULL;
}

static int dmx_write(int fd, const char* buf, size_t len) {
	int ret;

	// Helper to make dmx look like spi writes
	assert(NUM_PANELS == (10 * 3));
	assert(len == NUM_PANELS);
	assert(fd == -1);

	if (!pal.dmx)
		return -1;

	ret = usb_control_msg(pal.dmx,
	                      USB_TYPE_VENDOR | USB_RECIP_DEVICE | USB_ENDPOINT_OUT,
	                      cmd_SetChannelRange,
	                      NUM_PANELS, // Number Of Channels
	                      0,          // First Channel
	                      (char*)buf,        // Values
	                      len,     // Length
	                      5000);      // Timeout

	return ret;
}

static inline int smbus_access(int file, char read_write, __u8 command,
                               int size, union i2c_smbus_data *data)
{
	struct i2c_smbus_ioctl_data args;

	args.read_write = read_write;
	args.command = command;
	args.size = size;
	args.data = data;
	return ioctl(file, I2C_SMBUS, &args);
}

static inline int smbus_read_byte_data(int file, __u8 command)
{
	union i2c_smbus_data data;
	if (smbus_access(file, I2C_SMBUS_READ, command,
	                 I2C_SMBUS_BYTE_DATA, &data))
		return -1;
	else
		return 0x0FF & data.byte;
}

static inline int smbus_write_byte_data(int file, char command,
                                        char value)
{
	union i2c_smbus_data data;
	data.byte = value;
	return smbus_access(file,I2C_SMBUS_WRITE,command,
	                    I2C_SMBUS_BYTE_DATA, &data);
}

// Registers
#define REG_MODE1          0x00
#define REG_MODE2          0x01
#define REG_SUBADR1        0x02
#define REG_SUBADR2        0x03
#define REG_SUBADR3        0x04
#define REG_ALLCALLADR     0x05
#define REG_LED0_ON_L      0x06
#define REG_LED0_ON_H      0x07
#define REG_LED0_OFF_L     0x08
#define REG_LED0_OFF_H     0x09
#define REG_ALL_LED_ON_L   0xFA
#define REG_ALL_LED_ON_H   0xFB
#define REG_ALL_LED_OFF_L  0xFC
#define REG_ALL_LED_OFF_H  0xFD
#define REG_PRESCALE       0xFE

// Mode 1 Bits
#define M1_RESTART        0x80
#define M1_SLEEP          0x10
#define M1_ALLCALL        0x01

// Mode 2 Bits
#define M2_INVRT          0x10
#define M2_OUTDRV         0x04

static
void write_single_i2c(int fd, const char* buf) {
	// On time always stays at 1
	// Off time of 0 means always on
	// Off time of 1 means always off
	// Scale our 0-255 by brightness and
	// Just add 1 to the [0-4095] value we get

	size_t i;
	int val;
	int reg;

	for (i = 0; i < 5 * 3; ++i) {
		// Scale 8bit to 12bit using brightness
		val = (buf[i] * 2 * pal.p.panel_brightness + 1) & 0xfff;
		reg = i * 4;

		smbus_write_byte_data(fd,
		                      REG_LED0_OFF_L + reg,
		                      val & 0xff);

		smbus_write_byte_data(fd,
		                      REG_LED0_OFF_H + reg,
		                      val >> 8);
	}
}

static
int write_i2c(int fd, const char* buf, size_t len) {
	// Helper to make i2c look like spi writes
	assert(NUM_PANELS == (10 * 3));
	assert(len == NUM_PANELS);
	assert(fd == -1);

	// First 5 chanels go to pwn driver 1
	write_single_i2c(pal.fd_panels[0], buf);

	// Second 5 chanels go to pwn driver 2
	write_single_i2c(pal.fd_panels[1], buf + (NUM_PANELS / 2));

	return len;
}

static
void reset_i2c(int fd) {
	// Turn off all pwm
	smbus_write_byte_data(fd, REG_ALL_LED_ON_L, 1);
	smbus_write_byte_data(fd, REG_ALL_LED_ON_H, 0);
	smbus_write_byte_data(fd, REG_ALL_LED_OFF_L, 1);
	smbus_write_byte_data(fd, REG_ALL_LED_OFF_H, 0);

	// Disable oscillator
	smbus_write_byte_data(fd, REG_MODE1, M1_SLEEP | M1_ALLCALL);
	usleep(1000);
}

static
int open_i2c(const char* dev, int address) {
	int ret;
	int fd;
	int mode1;
	int prescale;
	int freq;

	fd = open(dev, O_RDWR);
	if (fd < 0) {
		fprintf(stderr, "Failed to open %s: %s\n",
			dev, strerror(errno));
		return -1;
	}

	if (ioctl(fd, I2C_SLAVE, address) < 0) {
		fprintf(stderr, "Failed to set I2C address 0x%x: %s\n",
			address, strerror(errno));
		close(fd);
		return -1;
	}

	// Ensure the board is connected
	ret = smbus_read_byte_data(fd, REG_MODE1);
	if (ret == -1) {
		fprintf(stderr, "Error opening PCA9685 @ 0x%x: %s\n",
			address, strerror(errno));
		return -1;
	}

	// Ensure everything is turned off
	reset_i2c(fd);

	// Set the pwm freq to be 24Hz
	// val = (25000000 / (4096 * rate)) - 1
	//smbus_write_byte_data(fd, REG_PRESCALE, 0x66);

	// Configure for use w/external LED driver
	smbus_write_byte_data(fd, REG_MODE2, M2_OUTDRV);

	// Enable oscillator
	mode1 = smbus_read_byte_data(fd, REG_MODE1);
	mode1 = mode1 & ~M1_SLEEP;
	smbus_write_byte_data(fd, REG_MODE1, mode1);
	usleep(1000);

	// Query the pwm frequency
	prescale = smbus_read_byte_data(fd, REG_PRESCALE);
	freq = 25000000 / (4096 * (prescale + 1));

	LOG(("Opened PCA9685, Addr: 0x%x, Freq: %uHz (0x%x)\n",
	     address, freq, prescale));

	return fd;
}

struct pal* pal_init(unsigned int enc_thresh, unsigned int enc_delay, unsigned int ema_pow) {
	memset(&pal, 0, sizeof(pal));
	pal.fd_treads = -1;
	pal.fd_barrel = -1;
	pal.fd_panels[0] = -1;
	pal.fd_panels[1] = -1;
	pal.p.panel_brightness = 8;

	if (load_capes(capes)) {
		perror("Failed to load the capes");
		return NULL;
	}

	pal.fd_treads = open_spi("/dev/spidev1.0");
	if (pal.fd_treads == -1) {
		return NULL;
	}

	pal.fd_barrel = open_spi("/dev/spidev2.0");
	if (pal.fd_barrel == -1) {
		return NULL;
	}

	pal.fd_panels[0] = open_i2c("/dev/i2c-1", 0x40);
	pal.fd_panels[1] = open_i2c("/dev/i2c-1", 0x41);
	pal.dmx = dmx_find_device();

	pal.pru = pru_init(enc_thresh, enc_delay, ema_pow);
	if (pal.pru) {
		unsigned int* p = (unsigned int*)pal.pru;

		pal.p.enc_timer = p + wordoffset(locals_t, timer);
		pal.p.enc_raw   = p + wordoffset(locals_t, enc_local[0].raw);
		pal.p.enc_min   = p + wordoffset(locals_t, enc_local[0].min);
		pal.p.enc_max   = p + wordoffset(locals_t, enc_local[0].max);
		pal.p.enc_ticks = p + wordoffset(locals_t, enc_local[0].ticks);
		pal.p.enc_speed = p + wordoffset(locals_t, enc_local[0].speed);
	}

	pal.p.treads_buf = pal.treads_buf1;
	pal.p.barrel_buf = pal.barrel_buf1;

	if (renderer_init(&pal.panel,
	              "Panels",
	              -1,
	              pal.panels_buf1,
	              pal.panels_buf2,
	              NUM_PANELS,
	              dmx_write,
	              &pal.p.panels_buf)) {
		LOG(("Failed to start panel renderer thread: %s\n", strerror(errno)));
	}

	// TODO: Write the appropriate bits to ensure
	// all leds are turned off

	return &pal.p;
}

void pal_treads_write() {
	write(pal.fd_treads, pal.p.treads_buf, sizeof(pal.treads_buf1));
}

void pal_barrel_write() {
	write(pal.fd_barrel, pal.p.barrel_buf, sizeof(pal.barrel_buf1));
}

void pal_panels_write() {
	if (renderer_swap(&pal.panel, &pal.p.panels_buf)) {
		TRACE_LOG(("Dropping panel frame!\n"));
	}
}

void pal_destroy() {
	renderer_destroy(&pal.panel);

	if (pal.pru) {
		pru_destroy();
		pal.pru = NULL;
	}

	if (pal.dmx != NULL) {
		// Turn off LEDs w/o using renderer obj
		memset(pal.p.panels_buf, 0, NUM_PANELS);
		dmx_write(-1, pal.p.panels_buf, NUM_PANELS);

		usb_close(pal.dmx);
		pal.dmx = NULL;
	}

	if (pal.fd_treads != -1) {
		// Turn off LEDs
		memset(pal.p.treads_buf, 0x80, NUM_TREADS);
		pal_treads_write();

		close(pal.fd_treads);
		pal.fd_treads = -1;
	}

	if (pal.fd_barrel != -1) {
		// Turn off LEDs
		memset(pal.p.barrel_buf, 0x80, NUM_BARREL);
		pal_barrel_write();

		close(pal.fd_barrel);
		pal.fd_barrel = -1;
	}

	if (pal.fd_panels[0] != -1) {
		reset_i2c(pal.fd_panels[0]);
		close(pal.fd_panels[0]);
		pal.fd_panels[0] = -1;
	}

	if (pal.fd_panels[1] != -1) {
		reset_i2c(pal.fd_panels[1]);
		close(pal.fd_panels[1]);
		pal.fd_panels[1] = -1;
	}

	pal.p.treads_buf = NULL;
	pal.p.barrel_buf = NULL;
	pal.p.panels_buf = NULL;
}

int pal_clock_gettime(struct timespec* tv) {
	return clock_gettime(CLOCK_MONOTONIC, tv);
}
