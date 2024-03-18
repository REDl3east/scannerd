#ifndef _RUN_H_
#define _RUN_H_

#include "evdev.h"
#include "uv.h"

#define SOCK_FILE "/run/scannerd.sock"

#define INPUT_BUF_LENGTH 64

typedef struct req_data_t {
  int initalized;
  uv_fs_t read_req;
  uv_fs_poll_t poll_req;
  char filename[MAX_DEV_INPUT_PATH];
  dev_input_t info;
  uv_file file_id;
  uv_buf_t ev_buf;
  struct input_event ev;

  int lshift;
  int rshift;
  char input_buf[INPUT_BUF_LENGTH];
  int input_buf_index;
} req_data_t;

int do_run_subcommand(const char* prog, const char* subcommand, int argc, char** argv);

void dev_signal_cb(uv_signal_t* handle, int signum);
void dev_fs_poll_cb(uv_fs_poll_t* req, int status, const uv_stat_t* prev, const uv_stat_t* curr);
void dev_fs_read_cb(uv_fs_t* req);
void dev_fs_open_cb(uv_fs_t* req);
void dev_fs_stat_cb(uv_fs_t* req);

// sock stuff

void my_alloc_cb(uv_handle_t* handle, size_t suggested_size, uv_buf_t* buf);
void on_close_cb(uv_handle_t* client);
void on_write_complete_cb(uv_write_t* req, int status);
void on_read_cb(uv_stream_t* client, ssize_t nread, const uv_buf_t* buf);
void on_connect_cb(uv_stream_t* stream, int status);

char code_to_key(int shifted, unsigned short code);
#endif // _RUN_H_