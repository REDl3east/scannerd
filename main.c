#include "main.h"



int main(int argc, char** argv) {
  arg_file_t* dev  = arg_file1(NULL, NULL, NULL, "/dev/input/eventX path.");
  arg_lit_t* help  = arg_lit0("h", "help", "print this help and exit.");
  arg_lit_t* vers  = arg_lit0("v", "version", "print version information and exit.");
  arg_end_t* end   = arg_end(20);
  void* argtable[] = {dev, help, vers, end};

  if (arg_nullcheck(argtable) != 0) {
    fprintf(stderr, "%s: insufficient memory\n", argv[0]);
    return 1;
  }

  int nerrors = arg_parse(argc, argv, argtable);

  if (help->count > 0) {
    printf("Usage: %s", argv[0]);
    arg_print_syntax(stdout, argtable, "\n");
    arg_print_glossary(stdout, argtable, "  %-10s %s\n");
    arg_freetable(argtable, sizeof(argtable) / sizeof(argtable[0]));
    return 0;
  }

  if (vers->count > 0) {
    printf("March 2024, Dalton Overmyer\n");
    arg_freetable(argtable, sizeof(argtable) / sizeof(argtable[0]));
    return 0;
  }

  if (nerrors > 0) {
    arg_print_errors(stderr, end, argv[0]);
    fprintf(stderr, "Try '%s --help' for more information.\n", argv[0]);
    arg_freetable(argtable, sizeof(argtable) / sizeof(argtable[0]));
    return 1;
  }

  uv_signal_t sig;
  uv_signal_init(uv_default_loop(), &sig);
  uv_signal_start(&sig, dev_signal_cb, SIGINT);

  req_data_t req_data;
  strncpy(req_data.filename, dev->filename[0], 4096);
  req_data.initalized = 0;

  req_data.read_req.data = &req_data;
  uv_fs_stat(uv_default_loop(), &req_data.read_req, req_data.filename, dev_fs_stat_cb);

  req_data.poll_req.data = &req_data;
  uv_fs_poll_init(uv_default_loop(), &req_data.poll_req);
  uv_fs_poll_start(&req_data.poll_req, dev_fs_poll_cb, req_data.filename, 1000);

  // uv_fs_event_t dev_input_fs_event;
  // uv_fs_event_init(uv_default_loop(), &dev_input_fs_event);
  // uv_fs_event_start(&dev_input_fs_event, dev_fs_dir_cb, DEV_INPUT_PATH, 0);

  uv_run(uv_default_loop(), UV_RUN_DEFAULT);

  uv_loop_close(uv_default_loop());
  arg_freetable(argtable, sizeof(argtable) / sizeof(argtable[0]));

  printf("INFO: Exiting...");

  return 0;
}

void dev_signal_cb(uv_signal_t* handle, int signum) {
  uv_stop(uv_default_loop());
}

void dev_fs_dir_cb(uv_fs_event_t* handle, const char* filename, int events, int status) {
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



void dev_fs_poll_cb(uv_fs_poll_t* req, int status, const uv_stat_t* prev, const uv_stat_t* curr) {
  req_data_t* data = (req_data_t*)req->data;

  if (status < 0) {
    printf("WARNING: '%s': %s, will wait for a connection.\n", data->filename, uv_strerror(status));
    return;
  }
  if (data->initalized) {
    printf("WARNING: '%s': device already initialized.\n", data->filename);
    return;
  }

  printf("INFO: '%s': device found, attempting to initialize.\n", data->filename);
  uv_fs_stat(uv_default_loop(), &data->read_req, data->filename, dev_fs_stat_cb);
}

void dev_fs_read_cb(uv_fs_t* req) {
  req_data_t* data = (req_data_t*)req->data;

  if (req->result < 0) {
    data->initalized = 0;
    uv_fs_req_cleanup(req);
    return;
  }

  if (data->ev.type == EV_KEY) {
    if (data->ev.value != KEY_HOLD && tracked_keys[data->ev.code] != NULL)
      // printf("Event: time f%ld.%06ld, ", data->ev.time.tv_sec, data->ev.time.tv_usec);
      printf("%s: %i (%s) %s\n", data->filename, data->ev.code, tracked_keys[data->ev.code], data->ev.value == KEY_RELEASE ? "RELEASED" : "PRESSED");
  }

  uv_fs_req_cleanup(req);
  uv_fs_read(uv_default_loop(), req, data->file_id, &data->ev_buf, 1, -1, dev_fs_read_cb);
}

void dev_fs_open_cb(uv_fs_t* req) {
  req_data_t* data = (req_data_t*)req->data;

  if (req->result < 0) {
    fprintf(stderr, "Error at opening file: '%s': %ld : %s.\n", data->filename, req->result, uv_strerror(req->result));
    uv_fs_req_cleanup(req);
    return;
  }

  data->initalized  = 1;
  data->file_id     = req->result;
  data->ev_buf.base = (char*)&data->ev;
  data->ev_buf.len  = sizeof(data->ev);

  printf("INFO: '%s': initialized\n", data->filename);

  uv_fs_req_cleanup(req);
  uv_fs_read(uv_default_loop(), req, data->file_id, &data->ev_buf, 1, -1, dev_fs_read_cb);
}

// TODO: if stat fails it may have supplied a device name
void dev_fs_stat_cb(uv_fs_t* req) {
  req_data_t* data = (req_data_t*)req->data;

  if (req->result < 0) {
    fprintf(stderr, "WARNING: '%s': %s, will wait for a connection.\n", data->filename, uv_strerror(req->result));
    uv_fs_req_cleanup(req);
    return;
  }

  if (!S_ISCHR(req->statbuf.st_mode)) {
    fprintf(stderr, "ERROR: '%s': Not a character device.\n", data->filename);
    uv_fs_req_cleanup(req);
    return;
  }

  if (!dev_input_query(&data->info, data->filename)) {
    fprintf(stderr, "ERROR: '%s': Failed to grab evdev info.\n", data->filename);
    uv_fs_req_cleanup(req);
    return;
  }

  if (!bit_is_set(data->info.bits, EV_KEY)) {
    fprintf(stderr, "ERROR: '%s': Device does not support EV_KEY event.\n", data->filename);
    uv_fs_req_cleanup(req);
    return;
  }

  uv_fs_req_cleanup(req);
  uv_fs_open(uv_default_loop(), req, data->filename, O_RDONLY, 0, dev_fs_open_cb);
}

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

  close(fd);
  return 1;
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

    dev_input_t dev;
    if (!dev_input_query(&dev, path)) continue;

    printf("path:             " DEV_INPUT_PATH "event%d\n", device);
    printf("name:             %s\n", dev.name);
    printf("driver version:   %d.%d.%d\n", dev.driver_version >> 16, (dev.driver_version >> 8) & 0xff, dev.driver_version & 0xff);
    printf("id bus type:      %d\n", dev.ids.bustype);
    printf("id product:       %d\n", dev.ids.product);
    printf("id vendor:        %d\n", dev.ids.vendor);
    printf("id version:       %d\n", dev.ids.version);

    printf("supported events: ");
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

static inline int bit_is_set(const unsigned long* array, int bit) {
  return !!(array[bit / LONG_BITS] & (1LL << (bit % LONG_BITS)));
}