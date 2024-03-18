#ifndef _EVDEV_H_
#define _EVDEV_H_

// bits and pieces taken from https://gitlab.freedesktop.org/libevdev/libevdev

#include <linux/input.h>

#define DEV_INPUT_PATH       "/dev/input/"
#define DEV_INPUT_PATH_LEN   sizeof(DEV_INPUT_PATH) - 1
#define MAX_DEV_INPUT_PATH   256
#define MAX_DEV_INPUT_EVENTS 20

#define KEY_RELEASE 0
#define KEY_PRESS   1
#define KEY_HOLD    2

#define LONG_BITS (sizeof(long) * 8)
#define NLONGS(x) (((x) + LONG_BITS - 1) / LONG_BITS)

typedef struct dev_input {
  char path[256];
  char name[256];
  char phys[256];
  char uniq[256];
  struct input_id ids;
  int driver_version;
  unsigned long bits[NLONGS(EV_CNT)];
  unsigned long key_bits[NLONGS(KEY_CNT)];
  unsigned long key_values[NLONGS(KEY_CNT)];
} dev_input_t;

int dev_input_query(dev_input_t* dev, const char* filename);
int bit_is_set(const unsigned long* array, int bit);

#endif // _EVDEV_H_