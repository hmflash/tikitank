#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <prussdrv.h>
#include <dirent.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <linux/spi/spidev.h>
#include <linux/types.h>
#include <sys/ioctl.h>

#include <prussdrv.h>
#include <pruss_intc_mapping.h>

#include "firmware.h"
#include "common.h"

static
struct pal pal;

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

	// Give cape 200ms to load
	usleep(200000);

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
void* pru_init(unsigned int threshold, unsigned int delay) {
	tpruss_intc_initdata pruss_intc_initdata = PRUSS_INTC_INITDATA;
	locals_t locals;
	int err;
	void* ptr;

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
	locals.enc.encoder0 = 0xff;
	locals.enc_local[0].threshold = threshold;
	locals.enc_local[0].delay = delay;
	locals.enc_local[0].speed = INITIAL_ACC_VAL;
	locals.enc_local[0].acc = INITIAL_ACC_VAL;

	err = prussdrv_pru_write_memory(PRUSS0_PRU0_DATARAM,
	                                0,
	                                (unsigned int*)&locals,
	                                sizeof(locals));
	if (err) {
		fprintf(stderr, "Failed write PRU0 memory: %s\n",
			strerror(errno));
		goto err_open_pru;
	}

	err = prussdrv_map_prumem(PRUSS0_PRU0_DATARAM, &ptr);
	if (err) {
		fprintf(stderr, "Failed map PRU0 memory: %s\n",
			strerror(errno));
		goto err_open_pru;
	}

	err = prussdrv_exec_program(PRU0, "firmware.bin");
	if (err) {
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
	                          offsetof(locals_t, flags),
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

struct pal* pal_init(unsigned int enc_thresh, unsigned int enc_delay) {
	unsigned int* pru;

	memset(&pal, 0, sizeof(pal));
	pal.fd_treads = -1;
	pal.fd_barrel = -1;
	pal.fd_panels = -1;

	if (load_capes(capes)) {
		perror("Failed to load the capes");
		return NULL;
	}

	pal.fd_treads = open_spi("/dev/spidev1.0");
	if (pal.fd_treads == -1) {
		return NULL;
	}

	pal.fd_barrel = open_spi("/dev/spidev2.0");
	if (pal.fd_treads == -1) {
		return NULL;
	}

	pru = (unsigned int*)pru_init(enc_thresh, enc_delay);
	if (!pru) {
		return NULL;
	}

	pal.enc_timer = pru + offsetof(locals_t, timer);
	pal.enc_raw   = pru + offsetof(locals_t, enc_local[0].raw);
	pal.enc_min   = pru + offsetof(locals_t, enc_local[0].min);
	pal.enc_max   = pru + offsetof(locals_t, enc_local[0].max);
	pal.enc_ticks = pru + offsetof(locals_t, enc_local[0].ticks);
	pal.enc_speed = pru + offsetof(locals_t, enc_local[0].speed);

	// TODO: Write the appropriate bits to ensure
	// all leds are turned off

	return &pal;
}

int pal_treads_write(struct pal* p, const char* buf, size_t len) {
	int ret = 0;

	ret = write(p->fd_treads, buf, len);

	return ret;
}

int pal_barrel_write(struct pal* p, const char* buf, size_t len) {
	int ret = 0;

	ret = write(p->fd_barrel, buf, len);

	return ret;
}

int pal_panels_write(struct pal* p, const char* buf, size_t len) {
	int ret = 0;

	ret = write(p->fd_panels, buf, len);

	return ret;
}

void pal_destroy(struct pal* p) {
	pru_destroy();

	safe_close(&p->fd_treads);
	safe_close(&p->fd_barrel);
	safe_close(&p->fd_panels);
}
