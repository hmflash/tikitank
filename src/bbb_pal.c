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

#include "common.h"

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

static int open_spi(const char* dev) {
	int fd;
	uint32_t arg;

	// Open spi device
	fd = open(dev, O_RDWR, 0);
	if (fd == -1)
		return -1;

	// LEDs require mode 0
	arg = SPI_MODE_0;
	if (ioctl(fd, SPI_IOC_WR_MODE, &arg) == -1) {
		perror("Mode failed");
		return -1;
	}

	// LEDs are 8 bits per word
	arg = 8;
	if (ioctl(fd, SPI_IOC_WR_BITS_PER_WORD, &arg) == -1) {
		perror("BPW failed");
		return -1;
	}

	// Run LEDs at 4mhz
	arg = 4000000;
	if (ioctl(fd, SPI_IOC_WR_MAX_SPEED_HZ, &arg) == -1) {
		perror("msh failed");
		return -1;
	}

	return fd;
}

static 
void safe_close(int* fd) {
	if (*fd != -1) {
		close(*fd);
		*fd = -1;
	}
}

int pal_init(struct pal* p) {
	memset(p, 0, sizeof(*p));
	p->fd_treads = -1;
	p->fd_barrel = -1;
	p->fd_panels = -1;

	if (load_capes(capes)) {
		perror("Failed to load the capes");
		return -1;
	}

	p->fd_treads = open_spi("/dev/spidev1.0");
	if (p->fd_treads == -1) {
		perror("Opening /dev/spidev0.0 failed");
		return -1;
	}

	p->fd_barrel = open_spi("/dev/spidev2.0");
	if (p->fd_treads == -1) {
		perror("Opening /dev/spidev0.0 failed");
		return -1;
	}

	// TODO: Write the appropriate bits to ensure
	// all leds are turned off

	return 0;
}

int pal_write_treads(struct pal* p, const char* buf, size_t len) {
	int ret;

	ret = write(p->fd_treads, buf, len);

	return ret;
}

int pal_destroy(struct pal* p) {
	safe_close(&p->fd_treads);
	safe_close(&p->fd_barrel);
	safe_close(&p->fd_panels);

	return 0;
}

