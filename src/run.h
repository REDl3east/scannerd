#ifndef _RUN_H_
#define _RUN_H_

#include "evdev.h"
#include "uv.h"

#include "CircularArray.h"
#include <string>

#define INPUT_BUF_LENGTH 64
#define INPUT_BUF_KEEP   5

typedef struct req_data_t {
  int initalized;
  uv_fs_t read_req;
  uv_fs_poll_t poll_req;
  char filename[MAX_DEV_INPUT_PATH];
  dev_input_t info;
  uv_file file_id;
  uv_buf_t ev_buf;
  struct input_event ev;

  int lshift = 0;
  int rshift = 0;

  std::string input_buf;
  CircularArray<std::string, INPUT_BUF_KEEP> last_input_buf;
} req_data_t;

int do_run_subcommand(const char* prog, const char* subcommand, int argc, char** argv);

void dev_signal_cb(uv_signal_t* handle, int signum);
void dev_fs_poll_cb(uv_fs_poll_t* req, int status, const uv_stat_t* prev, const uv_stat_t* curr);
void dev_fs_read_cb(uv_fs_t* req);
void dev_fs_open_cb(uv_fs_t* req);
void dev_fs_stat_cb(uv_fs_t* req);

char code_to_key(int shifted, unsigned short code);

#endif // _RUN_H_