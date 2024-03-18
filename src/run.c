#include "run.h"

#include "argtable3.h"
#include "sv.h"
#include "uv.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int do_run_subcommand(const char* prog, const char* subcommand, int argc, char** argv) {
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
    fprintf(stderr, "Try '%s %s --help' for more information.\n", prog, subcommand);
    arg_freetable(argtable, sizeof(argtable) / sizeof(argtable[0]));
    return 1;
  }

  uv_signal_t sig;
  uv_signal_init(uv_default_loop(), &sig);
  uv_signal_start(&sig, dev_signal_cb, SIGINT);

  req_data_t req_data;
  memset(&req_data, 0, sizeof(req_data_t));

  strncpy(req_data.filename, dev->filename[0], MAX_DEV_INPUT_PATH);

  req_data.read_req.data = &req_data;
  uv_fs_stat(uv_default_loop(), &req_data.read_req, req_data.filename, dev_fs_stat_cb);

  req_data.poll_req.data = &req_data;
  uv_fs_poll_init(uv_default_loop(), &req_data.poll_req);
  uv_fs_poll_start(&req_data.poll_req, dev_fs_poll_cb, req_data.filename, 1000);

  // uv_fs_t unlink_req;
  // uv_fs_unlink(uv_default_loop(), &unlink_req, SOCK_FILE, NULL);
  // if (unlink_req.result < 0) {
  //   printf("INFO: Could not unlink '%s': %s\n", SOCK_FILE, uv_strerror(unlink_req.result));
  // }
  // uv_fs_req_cleanup(&unlink_req);

  // uv_pipe_t pipe;
  // uv_pipe_init(uv_default_loop(), &pipe, 0);
  // uv_pipe_bind(&pipe, SOCK_FILE);
  // uv_listen((uv_stream_t*)&pipe, 0, on_connect_cb);

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
  char path[MAX_DEV_INPUT_PATH - DEV_INPUT_PATH_LEN];
  if (status != 0) return;
  if (!(events & UV_RENAME)) return;

  string_view filename_sv = sv_create_from_cstr(filename);

  if (!sv_starts_with(filename_sv, svl("event"))) return;
  filename_sv = sv_remove_prefix(filename_sv, 5);

  int event_num;
  if (!sv_parse_int(filename_sv, &event_num)) return;

  memset(path, 0, MAX_DEV_INPUT_PATH - DEV_INPUT_PATH_LEN);
  snprintf(path, MAX_DEV_INPUT_PATH - DEV_INPUT_PATH_LEN, "%s%s", DEV_INPUT_PATH, filename);

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
    if (data->ev.value == KEY_PRESS) {
      if (data->ev.code == KEY_LEFTSHIFT) {
        data->lshift = 1;
      } else if (data->ev.code == KEY_RIGHTSHIFT) {
        data->rshift = 1;
      } else if (data->ev.code == KEY_ENTER) {
        printf("%.*s\n", data->input_buf_index, data->input_buf);
        data->input_buf_index = 0;
      } else {
        int shifted = data->lshift || data->rshift;
        if (code_to_key[shifted][data->ev.code] != '\0') {
          if (data->input_buf_index <= INPUT_BUF_LENGTH - 1) {
            data->input_buf[data->input_buf_index] = code_to_key[shifted][data->ev.code];
            data->input_buf_index++;
          }
        }
      }
    } else if (data->ev.value == KEY_RELEASE) {
      if (data->ev.code == KEY_LEFTSHIFT) {
        data->lshift = 0;
      } else if (data->ev.code == KEY_RIGHTSHIFT) {
        data->rshift = 0;
      }
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