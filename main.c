#include <linux/input.h>
#include <stdio.h>
#include <string.h>

#include "argtable3.h"
#include "cvector.h"
#include "uv.h"

#define SV_IMPLEMENTATION
#include "sv.h"

#define DEV_INPUT_PATH       "/dev/input/"
#define DEV_INPUT_PATH_LEN   sizeof(DEV_INPUT_PATH) - 1
#define MAX_DEV_INPUT_PATH   255
#define MAX_DEV_INPUT_EVENTS 20

// bits and pieces taken from https://gitlab.freedesktop.org/libevdev/libevdev

#define LONG_BITS (sizeof(long) * 8)
#define NLONGS(x) (((x) + LONG_BITS - 1) / LONG_BITS)

typedef struct dev_input {
  int fd;
  char name[256];
  char phys[256];
  char uniq[256];
  struct input_id ids;
  int driver_version;
  unsigned long bits[NLONGS(EV_CNT)];
  unsigned long key_bits[NLONGS(KEY_CNT)];
  unsigned long key_values[NLONGS(KEY_CNT)];
} dev_input_t;

static inline int bit_is_set(const unsigned long* array, int bit);

void fs_event_dev_input_cb(uv_fs_event_t* handle, const char* filename, int events, int status);

void dev_input_scan();
int dev_input_query(dev_input_t* dev, int fd);

int main() {
  dev_input_scan();

  // uv_fs_event_t dev_input_fs_event;
  // uv_fs_event_init(uv_default_loop(), &dev_input_fs_event);
  // uv_fs_event_start(&dev_input_fs_event, fs_event_dev_input_cb, DEV_INPUT_PATH, 0);

  // uv_run(uv_default_loop(), UV_RUN_DEFAULT);

  // uv_loop_close(uv_default_loop());

  return 0;
}

void fs_event_dev_input_cb(uv_fs_event_t* handle, const char* filename, int events, int status) {
  char path[PATH_MAX - DEV_INPUT_PATH_LEN];
  if (status != 0) return;
  if (!(events & UV_RENAME)) return;

  string_view filename_sv = sv_create_from_cstr(filename);

  if (!sv_starts_with(filename_sv, svl("event"))) return;
  filename_sv = sv_remove_prefix(filename_sv, 5);

  int event_num;
  if (!sv_parse_int(filename_sv, &event_num)) return;

  memset(path, 0, PATH_MAX - DEV_INPUT_PATH_LEN);
  snprintf(path, PATH_MAX - DEV_INPUT_PATH_LEN, "%s%s", DEV_INPUT_PATH, filename);

  printf("[%d] %s\n", event_num, path);
}

void dev_input_scan() {
  uv_fs_t fs_dir_req;
  if (!uv_fs_scandir(uv_default_loop(), &fs_dir_req, DEV_INPUT_PATH, 0, NULL)) return;

  uv_dirent_t ent;
  while (uv_fs_scandir_next(&fs_dir_req, &ent) != UV_EOF) {
    string_view filename_sv = sv_create_from_cstr(ent.name);
    int device;
    if (!sv_starts_with(filename_sv, svl("event"))) continue;
    filename_sv = sv_remove_prefix(filename_sv, 5);
    if (!sv_parse_int(filename_sv, &device)) continue;

    char path[MAX_DEV_INPUT_PATH];
    snprintf(path, MAX_DEV_INPUT_PATH, "%sevent%d", DEV_INPUT_PATH, device);

    int fd = open(path, O_RDONLY);
    if (fd < 0) {
      fprintf(stderr, "Error: %s: %s.\n", path, strerror(errno));
      continue;
    }

    dev_input_t dev;
    if (!dev_input_query(&dev, fd)) continue;

    printf("path:             " DEV_INPUT_PATH "event%d\n", device);
    printf("name:             %s\n", dev.name);
    printf("driver version:   %d.%d.%d\n", dev.driver_version >> 16, (dev.driver_version >> 8) & 0xff, dev.driver_version & 0xff);
    printf("id bus type:      %d\n", dev.ids.bustype);
    printf("id product:       %d\n", dev.ids.product);
    printf("id vendor:        %d\n", dev.ids.vendor);
    printf("id version:       %d\n", dev.ids.version);

    printf("Supported events: ");
    for (int i = 0; i < EV_CNT; i++) {
      if (bit_is_set(dev.bits, i)) {
        switch (i) {
          case EV_SYN:
            printf("EV_SYN ");
            break;
          case EV_KEY:
            printf("EV_KEY ");
            break;
          case EV_REL:
            printf("EV_REL ");
            break;
          case EV_ABS:
            printf("EV_ABS ");
            break;
          case EV_MSC:
            printf("EV_MSC ");
            break;
          case EV_SW:
            printf("EV_SW ");
            break;
          case EV_LED:
            printf("EV_LED ");
            break;
          case EV_SND:
            printf("EV_SND ");
            break;
          case EV_REP:
            printf("EV_REP ");
            break;
          case EV_FF:
            printf("EV_FF ");
            break;
          case EV_PWR:
            printf("EV_PWR ");
            break;
          case EV_FF_STATUS:
            printf("EV_FF_STATUS ");
            break;
        }
      }
    }
    printf("\n");

    printf("\n");
  }

  uv_fs_req_cleanup(&fs_dir_req);
}

int dev_input_query(dev_input_t* dev, int fd) {
  int rc;

  if (fd < 0) return 0;

  rc = ioctl(fd, EVIOCGBIT(0, sizeof(dev->bits)), dev->bits);
  if (rc < 0) {
    fprintf(stderr, "ERROR: ioctl(fd, EVIOCGBIT(0, sizeof(dev->bits)), dev->bits);\n");
    return 0;
  }

  rc = ioctl(fd, EVIOCGNAME(sizeof(dev->name) - 1), dev->name);
  if (rc < 0) {
    fprintf(stderr, "ERROR: ioctl(fd, EVIOCGNAME(sizeof(dev->name) - 1), dev->name);\n");
    return 0;
  }

  rc = ioctl(fd, EVIOCGPHYS(sizeof(dev->phys) - 1), dev->phys);
  if (rc < 0 && errno != ENOENT) {
    fprintf(stderr, "ERROR: ioctl(fd, EVIOCGPHYS(sizeof(dev->phys) - 1), dev->phys);\n");
    return 0;
  }

  rc = ioctl(fd, EVIOCGUNIQ(sizeof(dev->uniq) - 1), dev->uniq);
  if (rc < 0 && errno != ENOENT) {
    fprintf(stderr, "ERROR: ioctl(fd, EVIOCGUNIQ(sizeof(dev->uniq) - 1), dev->uniq);\n");
    return 0;
  }

  rc = ioctl(fd, EVIOCGID, &dev->ids);
  if (rc < 0) {
    fprintf(stderr, "ERROR: ioctl(fd, EVIOCGID, &dev->ids);\n");
    return 0;
  }

  rc = ioctl(fd, EVIOCGVERSION, &dev->driver_version);
  if (rc < 0) {
    fprintf(stderr, "ERROR: ioctl(fd, EVIOCGVERSION, &dev->driver_version);\n");
    return 0;
  }

  rc = ioctl(fd, EVIOCGBIT(EV_KEY, sizeof(dev->key_bits)), dev->key_bits);
  if (rc < 0) {
    fprintf(stderr, "ERROR: ioctl(fd, EVIOCGBIT(EV_KEY, sizeof(dev->key_bits)), dev->key_bits);\n");
    return 0;
  }

  rc = ioctl(fd, EVIOCGKEY(sizeof(dev->key_values)), dev->key_values);
  if (rc < 0) {
    fprintf(stderr, "ERROR: ioctl(fd, EVIOCGKEY(sizeof(dev->key_values)), dev->key_values);\n");
    return 0;
  }

  dev->fd = fd;

  return 1;
}

static inline int bit_is_set(const unsigned long* array, int bit) {
  return !!(array[bit / LONG_BITS] & (1LL << (bit % LONG_BITS)));
}