#include "main.h"

typedef enum subcommand_type {
  SUBCOMMAND_NONE,
  SUBCOMMAND_QUERY,
  SUBCOMMAND_RUN,
  SUBCOMMAND_HELP,
} subcommand_type;

int run_query_subcommand(const char* prog, const char* subcommand, int argc, char** argv) {
  arg_lit_t* help  = arg_lit0("h", "help", "print this help and exit.");
  arg_lit_t* vers  = arg_lit0("v", "version", "print version information and exit.");
  arg_lit_t* meta  = arg_lit0("m", "meta", "Include ID bus type, product, vendor, and version.");
  arg_end_t* end   = arg_end(20);
  void* argtable[] = {help, vers, meta, end};

  if (arg_nullcheck(argtable) != 0) {
    fprintf(stderr, "%s: insufficient memory\n", argv[0]);
    return 1;
  }

  int nerrors = arg_parse(argc, argv, argtable);

  if (help->count > 0) {
    printf("Usage: %s %s", prog, subcommand);
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

  dev_input_scan(meta->count);
  return 0;
}

int run_run_subcommand(const char* prog, const char* subcommand, int argc, char** argv) {
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

  uv_fs_t unlink_req;
  uv_fs_unlink(uv_default_loop(), &unlink_req, SOCK_FILE, NULL);
  if (unlink_req.result < 0) {
    printf("INFO: Could not unlink '%s': %s\n", SOCK_FILE, uv_strerror(unlink_req.result));
  }
  uv_fs_req_cleanup(&unlink_req);

  uv_pipe_t pipe;
  uv_pipe_init(uv_default_loop(), &pipe, 0);
  uv_pipe_bind(&pipe, SOCK_FILE);
  uv_listen((uv_stream_t*)&pipe, 0, on_connect_cb);

  // uv_fs_event_t dev_input_fs_event;
  // uv_fs_event_init(uv_default_loop(), &dev_input_fs_event);
  // uv_fs_event_start(&dev_input_fs_event, dev_fs_dir_cb, DEV_INPUT_PATH, 0);

  uv_run(uv_default_loop(), UV_RUN_DEFAULT);

  uv_loop_close(uv_default_loop());
  arg_freetable(argtable, sizeof(argtable) / sizeof(argtable[0]));

  printf("INFO: Exiting...");

  return 0;
}

int run_help_subcommand(const char* prog, const char* subcommand, int argc, char** argv) {
  printf("Usage:\n");
  printf("   %s query\n", prog);
  printf("   %s run\n", prog);
  printf("   %s (-h | --help)\n", prog);

  return 0;
}

int run_subcommand(const char* prog, const char* subcommand, int argc, char** argv, subcommand_type type) {
  switch (type) {
    case SUBCOMMAND_QUERY:
      return run_query_subcommand(prog, subcommand, argc, argv);
    case SUBCOMMAND_RUN:
      return run_run_subcommand(prog, subcommand, argc, argv);
    case SUBCOMMAND_HELP:
      return run_help_subcommand(prog, subcommand, argc, argv);
    default: {
      fprintf(stderr, "%s: Expected (query | run | -h | --help) subcommand, got '%s'\n", prog, subcommand);
      return 1;
    }
  }

  return 1;
}

int main(int argc, char** argv) {
  if (argc < 2) {
    fprintf(stderr, "%s: Expected (query | run | -h | --help) subcommand, got EOF.\n", argv[0]);
    return 1;
  }

  subcommand_type type = SUBCOMMAND_NONE;
  if (strcmp(argv[1], "query") == 0) {
    type = SUBCOMMAND_QUERY;
  } else if (strcmp(argv[1], "run") == 0) {
    type = SUBCOMMAND_RUN;
  } else if (strcmp(argv[1], "--h") == 0 || strcmp(argv[1], "--help") == 0) {
    type = SUBCOMMAND_HELP;
  }

  const char* prog       = argv[0];
  const char* subcommand = argv[1];

  return run_subcommand(prog, subcommand, --argc, ++argv, type);
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
    if (data->ev.value != KEY_HOLD && tracked_keys[data->ev.code] != NULL) {
      // printf("Event: time f%ld.%06ld, ", data->ev.time.tv_sec, data->ev.time.tv_usec);
      printf("%s: %i (%s) %s\n", data->filename, data->ev.code, tracked_keys[data->ev.code], data->ev.value == KEY_RELEASE ? "RELEASED" : "PRESSED");
    }
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

static int dev_input_compare(const struct dirent** d1, const struct dirent** d2) {
  string_view d1_sv = sv_create_from_cstr((*d1)->d_name);
  string_view d2_sv = sv_create_from_cstr((*d2)->d_name);

  if (sv_starts_with(d1_sv, svl("event")) && sv_starts_with(d2_sv, svl("event"))) {
    sv_index_t i1 = sv_find_last_not_of(d1_sv, svl("0123456789"), SV_NPOS);
    sv_index_t i2 = sv_find_last_not_of(d2_sv, svl("0123456789"), SV_NPOS);

    if (i1 != SV_NPOS && i2 != SV_NPOS) {
      int num1 = -1;
      int num2 = -1;

      sv_parse_int(sv_substr(d1_sv, i1 + 1, SV_NPOS), &num1);
      sv_parse_int(sv_substr(d2_sv, i2 + 1, SV_NPOS), &num2);

      return num1 > num2;
    } else if (i1 != SV_NPOS && i2 == SV_NPOS) {
      return 1;
    } else if (i1 == SV_NPOS && i2 != SV_NPOS) {
      return -1;
    }
  } else if (sv_starts_with(d1_sv, svl("event")) && !sv_starts_with(d2_sv, svl("event"))) {
    return 1;
  } else if (!sv_starts_with(d1_sv, svl("event")) && sv_starts_with(d2_sv, svl("event"))) {
    return -1;
  }

  return strcoll((*d1)->d_name, (*d2)->d_name);
}

void dev_input_scan(int meta) {
  struct dirent** namelist;
  int n = scandir(DEV_INPUT_PATH, &namelist, NULL, dev_input_compare);
  if (n == -1) return;

  dev_input_t* devices = malloc(n * sizeof(dev_input_t));
  int device_count     = 0;

  for (int i = 0; i < n; i++) {
    string_view filename_sv = sv_create_from_cstr(namelist[i]->d_name);
    int device;
    if (!sv_starts_with(filename_sv, svl("event"))) continue;

    filename_sv = sv_remove_prefix(filename_sv, 5);
    if (!sv_parse_int(filename_sv, &device)) continue;

    char path[MAX_DEV_INPUT_PATH];
    snprintf(path, MAX_DEV_INPUT_PATH, "%sevent%d", DEV_INPUT_PATH, device);

    dev_input_t* dev = devices + device_count;
    if (!dev_input_query(dev, path)) continue;
    if (!bit_is_set(dev->bits, EV_KEY)) continue;
    device_count++;
  }

  int col1_max = 0;
  int col2_max = 0;

  for (int i = 0; i < device_count; i++) {
    int path_len = strlen(devices[i].path);
    if (path_len > col1_max) col1_max = path_len;

    int name_len = strlen(devices[i].name);
    if (name_len > col2_max) col2_max = name_len;
  }

  int spacing = 4;

  const char* path_lbl = "Path:";
  int path_lbl_len     = strlen(path_lbl);
  if (path_lbl_len > col1_max) col1_max = path_lbl_len;

  const char* name_lbl = "Name:";
  int name_lbl_len     = strlen(name_lbl);
  if (name_lbl_len > col2_max) col2_max = name_lbl_len;

  const char* meta_lbl = "Bus Type:    Product:    Vendor:    ID Version:";
  int meta_lbl_len     = strlen(meta_lbl);

  printf("%s", path_lbl);
  for (int i = 0; i < col1_max - path_lbl_len + spacing; i++) {
    printf(" ");
  }
  printf("%s", name_lbl);
  for (int i = 0; i < col2_max - name_lbl_len + spacing; i++) {
    printf(" ");
  }

  if (!meta) {
    printf("\n");
  } else {
    printf("%s\n", meta_lbl);
  }

  for (int i = 0; i < device_count; i++) {
    int path_len = strlen(devices[i].path);
    int name_len = strlen(devices[i].name);

    printf("%s", devices[i].path);
    for (int i = 0; i < col1_max - path_len + spacing; i++)
      printf(" ");
    printf("%s", devices[i].name);

    if (!meta) {
      printf("\n");
      continue;
    }

    for (int i = 0; i < col2_max - name_len + spacing; i++)
      printf(" ");

    printf("0x%.4x       ", devices[i].ids.bustype);
    printf("0x%.4x      ", devices[i].ids.product);
    printf("0x%.4x     ", devices[i].ids.vendor);
    printf("0x%.4x\n", devices[i].ids.version);
  }

  free(devices);
  for (int i = 0; i < n; i++)
    free(namelist[i]);
  free(namelist);

  return;
}

// sock stuff

void my_alloc_cb(uv_handle_t* handle, size_t suggested_size, uv_buf_t* buf) {
  buf->base = malloc(suggested_size);
  buf->len  = suggested_size;
}

void on_close_cb(uv_handle_t* client) {
  free(client);
}

void on_write_complete_cb(uv_write_t* req, int status) {
  // uv_close((uv_handle_t*)req->handle, on_close_cb);
  free(req->data);
  free(req);
}

// Each buffer is used only once and the user is responsible for freeing it in the uv_udp_recv_cb or the uv_read_cb callback.
// We do it in the write complete callback :)
void on_read_cb(uv_stream_t* client, ssize_t nread, const uv_buf_t* buf) {
  if (nread > 0) {
    printf("read: ");
    printf("%.*s", (int)buf->len, buf->base);

    uv_write_t* write_req = malloc(sizeof(uv_write_t));
    write_req->data       = (void*)buf->base;

    uv_write(write_req, client, buf, 1, on_write_complete_cb);
  }

  if (nread < 0 || nread == UV_EOF) {
    printf("disconnected\n");
    uv_close((uv_handle_t*)client, on_close_cb);
    return;
  }
}

void on_connect_cb(uv_stream_t* stream, int status) {
  uv_pipe_t* client = (uv_pipe_t*)malloc(sizeof(uv_pipe_t));
  uv_pipe_init(stream->loop, client, 0);
  int r = uv_accept(stream, (uv_stream_t*)client);

  if (r < 0) {
    // error
  }

  uv_read_start((uv_stream_t*)client, my_alloc_cb, on_read_cb);
  printf("connected...\n");
}