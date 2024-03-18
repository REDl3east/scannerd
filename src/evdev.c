#include "evdev.h"

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

int dev_input_query(dev_input_t* dev, const char* filename) {
  int fd = open(filename, O_RDONLY);
  if (fd < 0) {
    fprintf(stderr, "ERROR: %s: %s.\n", filename, strerror(errno));
    return 0;
  }

  int rc;

  if (fd < 0) {
    close(fd);
    return 0;
  }

  rc = ioctl(fd, EVIOCGBIT(0, sizeof(dev->bits)), dev->bits);
  if (rc < 0) {
    close(fd);
    return 0;
  }

  rc = ioctl(fd, EVIOCGNAME(sizeof(dev->name) - 1), dev->name);
  if (rc < 0) {
    close(fd);
    return 0;
  }

  rc = ioctl(fd, EVIOCGPHYS(sizeof(dev->phys) - 1), dev->phys);
  if (rc < 0 && errno != ENOENT) {
    close(fd);
    return 0;
  }

  rc = ioctl(fd, EVIOCGUNIQ(sizeof(dev->uniq) - 1), dev->uniq);
  if (rc < 0 && errno != ENOENT) {
    close(fd);
    return 0;
  }

  rc = ioctl(fd, EVIOCGID, &dev->ids);
  if (rc < 0) {
    close(fd);
    return 0;
  }

  rc = ioctl(fd, EVIOCGVERSION, &dev->driver_version);
  if (rc < 0) {
    close(fd);
    return 0;
  }

  rc = ioctl(fd, EVIOCGBIT(EV_KEY, sizeof(dev->key_bits)), dev->key_bits);
  if (rc < 0) {
    close(fd);
    return 0;
  }

  rc = ioctl(fd, EVIOCGKEY(sizeof(dev->key_values)), dev->key_values);
  if (rc < 0) {
    close(fd);
    return 0;
  }

  strncpy(dev->path, filename, MAX_DEV_INPUT_PATH);

  close(fd);
  return 1;
}

inline int bit_is_set(const unsigned long* array, int bit) {
  return !!(array[bit / LONG_BITS] & (1LL << (bit % LONG_BITS)));
}