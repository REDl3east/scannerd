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

int query_input_device(int device);
void fs_event_dev_input_cb(uv_fs_event_t* handle, const char* filename, int events, int status);

void dev_input_map_query();

int main() {
  dev_input_map_query();

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

#define _GNU_SOURCE /* for asprintf */
#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <linux/input.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define BITS_PER_LONG        (sizeof(long) * 8)
#define NBITS(x)             ((((x)-1) / BITS_PER_LONG) + 1)
#define OFF(x)               ((x) % BITS_PER_LONG)
#define LONG(x)              ((x) / BITS_PER_LONG)
#define test_bit(bit, array) ((array[LONG(bit)] >> OFF(bit)) & 1)

void dev_input_map_query() {
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

    int i, j;
    int version;
    unsigned short id[4];
    unsigned long bit[EV_MAX][NBITS(KEY_MAX)];

    if (ioctl(fd, EVIOCGVERSION, &version)) {
      continue;
    }
    printf("path:           " DEV_INPUT_PATH "event%d\n", device);
    printf("driver version: %d.%d.%d\n", version >> 16, (version >> 8) & 0xff, version & 0xff);

    ioctl(fd, EVIOCGID, id);
    printf("device bus:     0x%x\n", id[ID_BUS]);
    printf("device vendor:  0x%x\n", id[ID_VENDOR]);
    printf("device product: 0x%x\n", id[ID_PRODUCT]);
    printf("device version: 0x%x\n", id[ID_VERSION]);

    // memset(bit, 0, sizeof(bit));
    // ioctl(fd, EVIOCGBIT(0, EV_MAX), bit[0]);
    // printf("Supported events:\n");
    // for (i = 0; i < EV_MAX; i++) {
    //   if (test_bit(i, bit[0])) {
    //     printf("  Event type %d\n", i);
    //     if (!i) continue;
    //     ioctl(fd, EVIOCGBIT(i, KEY_MAX), bit[i]);
    //     for (j = 0; j < KEY_MAX; j++) {
    //       if (test_bit(j, bit[i])) {
    //         printf("%d, ", j);
    //       }
    //     }
    //     printf("\n");
    //   }
    // }
    printf("\n");
  }

  uv_fs_req_cleanup(&fs_dir_req);
}

// #define _GNU_SOURCE /* for asprintf */
// #include <stdio.h>
// #include <string.h>
// #include <stdlib.h>
// #include <fcntl.h>
// #include <unistd.h>
// #include <dirent.h>
// #include <errno.h>
// #include <linux/input.h>

// #define BITS_PER_LONG (sizeof(long) * 8)
// #define NBITS(x) ((((x)-1)/BITS_PER_LONG)+1)
// #define OFF(x)  ((x)%BITS_PER_LONG)
// #define LONG(x) ((x)/BITS_PER_LONG)
// #define test_bit(bit, array)	((array[LONG(bit)] >> OFF(bit)) & 1)

// #define DEV_INPUT_EVENT "/dev/input"
// #define EVENT_DEV_NAME "event"

// int is_event_device(const struct dirent *dir) {
// 	return strncmp(EVENT_DEV_NAME, dir->d_name, 5) == 0;
// }

// char* scan_devices(void) {
// 	struct dirent **namelist;
// 	int i, ndev, devnum;
// 	char *filename;

// 	ndev = scandir(DEV_INPUT_EVENT, &namelist, is_event_device, alphasort);
// 	if (ndev <= 0) {
// 		return NULL;
// 	}

// 	printf("Available devices:\n");

// 	for (i = 0; i < ndev; i++) {
// 		char fname[64];
// 		int fd = -1;
// 		char name[256] = "???";

// 		snprintf(fname, sizeof(fname), "%s/%s", DEV_INPUT_EVENT,
// namelist[i]->d_name); 		fd = open(fname, O_RDONLY); 		if (fd >= 0) { 			ioctl(fd,
// EVIOCGNAME(sizeof(name)), name); 			close(fd);
// 		}
// 		printf("%s:  %s\n", fname, name);
// 		free(namelist[i]);
// 	}

// 	fprintf(stderr, "Select the device event number [0-%d]: ", ndev - 1);
// 	scanf("%d", &devnum);

// 	if (devnum >= ndev || devnum < 0) {
// 		return NULL;
// 	}

// 	asprintf(&filename, "%s/%s%d", DEV_INPUT_EVENT, EVENT_DEV_NAME, devnum);
// 	return filename;
// }

// static int print_device_info(int fd) {
// 	int i, j;
// 	int version;
// 	unsigned short id[4];
// 	unsigned long bit[EV_MAX][NBITS(KEY_MAX)];

// 	if (ioctl(fd, EVIOCGVERSION, &version)) {
// 		perror("can't get version");
// 		return 1;
// 	}
// 	printf("Input driver version is %d.%d.%d\n",
// 	       version >> 16, (version >> 8) & 0xff, version & 0xff);

// 	ioctl(fd, EVIOCGID, id);
// 	printf("Input device ID: bus 0x%x vendor 0x%x product 0x%x version
// 0x%x\n", 		id[ID_BUS], id[ID_VENDOR], id[ID_PRODUCT], id[ID_VERSION]);

// 	memset(bit, 0, sizeof(bit));
// 	ioctl(fd, EVIOCGBIT(0, EV_MAX), bit[0]);
// 	printf("Supported events:\n");
// 	for (i = 0; i < EV_MAX; i++) {
//  		if (test_bit(i, bit[0])) {
// 			printf("  Event type %d\n", i);
// 			if (!i) continue;
// 			ioctl(fd, EVIOCGBIT(i, KEY_MAX), bit[i]);
// 			for (j = 0; j < KEY_MAX; j++) {
// 				if (test_bit(j, bit[i])) {
// 					printf("%d, ", j);
// 				}
// 			}
// 			printf("\n");
// 		}
// 	}
// 	return 0;
// }

// int print_events(int fd) {
// 	struct input_event ev;
// 	unsigned int size;

// 	printf("Testing ... (interrupt to exit)\n");

// 	while (1) {
// 		size = read(fd, &ev, sizeof(struct input_event));

// 		if (size < sizeof(struct input_event)) {
// 			printf("expected %lu bytes, got %u\n", sizeof(struct
// input_event), size); 			perror("\nerror reading"); 			return EXIT_FAILURE;
// 		}

// 		printf("Event: time %ld.%06ld, ", ev.time.tv_sec,
// ev.time.tv_usec); 		printf("type: %i, code: %i, value: %i\n", ev.type, ev.code,
// ev.value);
// 	}
// }

// int main () {
// 	int fd, grabbed;
// 	char *filename;

// 	if (getuid() != 0) {
// 		fprintf(stderr, "Not running as root, no devices may be
// available.\n");
// 	}

// 	filename = scan_devices();
// 	if (!filename) {
// 		fprintf(stderr, "Device not found\n");
// 		return EXIT_FAILURE;
// 	}

// 	if ((fd = open(filename, O_RDONLY)) < 0) {
// 		perror("");
// 		if (errno == EACCES && getuid() != 0) {
// 			fprintf(stderr, "You do not have access to %s. Try "
// 					"running as root instead.\n", filename);
// 		}
// 		return EXIT_FAILURE;
// 	}

// 	free(filename);

// 	if (print_device_info(fd)) {
// 		return EXIT_FAILURE;
// 	}

// 	grabbed = ioctl(fd, EVIOCGRAB, (void *) 1);
// 	ioctl(fd, EVIOCGRAB, (void *) 0);
// 	if (grabbed) {
// 		printf("This device is grabbed by another process. Try switching
// VT.\n"); 		return EXIT_FAILURE;
// 	}

// 	return print_events(fd);
// }
