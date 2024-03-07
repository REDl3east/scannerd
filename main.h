#ifndef _MAIN_H_
#define _MAIN_H_

#include <linux/input.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "argtable3.h"
#include "uv.h"

#define SV_IMPLEMENTATION
#include "sv.h"

#define DEV_INPUT_PATH       "/dev/input/"
#define DEV_INPUT_PATH_LEN   sizeof(DEV_INPUT_PATH) - 1
#define MAX_DEV_INPUT_PATH   255
#define MAX_DEV_INPUT_EVENTS 20

#define KEY_RELEASE 0
#define KEY_PRESS   1
#define KEY_HOLD    2

// bits and pieces taken from https://gitlab.freedesktop.org/libevdev/libevdev

static inline int bit_is_set(const unsigned long* array, int bit);

#define LONG_BITS (sizeof(long) * 8)
#define NLONGS(x) (((x) + LONG_BITS - 1) / LONG_BITS)

typedef struct dev_input {
  char name[256];
  char phys[256];
  char uniq[256];
  struct input_id ids;
  int driver_version;
  unsigned long bits[NLONGS(EV_CNT)];
  unsigned long key_bits[NLONGS(KEY_CNT)];
  unsigned long key_values[NLONGS(KEY_CNT)];
} dev_input_t;

typedef struct req_data_t {
  int initalized;
  uv_fs_t read_req;
  uv_fs_poll_t poll_req;
  char filename[PATH_MAX];
  dev_input_t info;
  uv_file file_id;
  uv_buf_t ev_buf;
  struct input_event ev;
} req_data_t;

static const char* tracked_keys[KEY_CNT] = {
    [KEY_0] = "0",
    [KEY_1] = "1",
    [KEY_2] = "2",
    [KEY_3] = "3",
    [KEY_4] = "4",
    [KEY_5] = "5",
    [KEY_6] = "6",
    [KEY_7] = "7",
    [KEY_8] = "8",
    [KEY_9] = "9",

    [KEY_KP0] = "KP0",
    [KEY_KP1] = "KP1",
    [KEY_KP2] = "KP2",
    [KEY_KP3] = "KP3",
    [KEY_KP4] = "KP4",
    [KEY_KP5] = "KP5",
    [KEY_KP6] = "KP6",
    [KEY_KP7] = "KP7",
    [KEY_KP8] = "KP8",
    [KEY_KP9] = "KP9",

    [KEY_A] = "A",
    [KEY_B] = "B",
    [KEY_C] = "C",
    [KEY_D] = "D",
    [KEY_E] = "E",
    [KEY_F] = "F",
    [KEY_G] = "G",
    [KEY_H] = "H",
    [KEY_I] = "I",
    [KEY_J] = "J",
    [KEY_K] = "K",
    [KEY_L] = "L",
    [KEY_M] = "M",
    [KEY_N] = "N",
    [KEY_O] = "O",
    [KEY_P] = "P",
    [KEY_Q] = "Q",
    [KEY_R] = "R",
    [KEY_S] = "S",
    [KEY_T] = "T",
    [KEY_U] = "U",
    [KEY_V] = "V",
    [KEY_W] = "W",
    [KEY_X] = "X",
    [KEY_Y] = "Y",
    [KEY_Z] = "Z",

    [KEY_ENTER]      = "ENTER",
    [KEY_TAB]        = "TAB",
    [KEY_KPENTER]    = "KPENTER",
    [KEY_SPACE]      = "SPACE",
    [KEY_LEFTSHIFT]  = "LEFTSHIFT",
    [KEY_RIGHTSHIFT] = "RIGHTSHIFT",
    [KEY_LEFTCTRL]   = "LEFTCTRL",
    [KEY_RIGHTCTRL]  = "RIGHTCTRL",
    [KEY_LEFTALT]    = "LEFTALT",
    [KEY_RIGHTALT]   = "RIGHTALT",

    [KEY_GRAVE]      = "GRAVE",
    [KEY_MINUS]      = "MINUS",
    [KEY_EQUAL]      = "EQUAL",
    [KEY_LEFTBRACE]  = "LEFTBRACE",
    [KEY_RIGHTBRACE] = "RIGHTBRACE",
    [KEY_BACKSLASH]  = "BACKSLASH",
    [KEY_SEMICOLON]  = "SEMICOLON",
    [KEY_APOSTROPHE] = "APOSTROPHE",
    [KEY_COMMA]      = "COMMA",
    [KEY_DOT]        = "DOT",
    [KEY_SLASH]      = "SLASH",

    [BTN_LEFT]   = "LEFT",
    [BTN_RIGHT]  = "RIGHT",
    [BTN_MIDDLE] = "MIDDLE",
};

void dev_signal_cb(uv_signal_t* handle, int signum);
void dev_fs_dir_cb(uv_fs_event_t* handle, const char* filename, int events, int status);
void dev_fs_poll_cb(uv_fs_poll_t* req, int status, const uv_stat_t* prev, const uv_stat_t* curr);
void dev_fs_read_cb(uv_fs_t* req);
void dev_fs_open_cb(uv_fs_t* req);
void dev_fs_stat_cb(uv_fs_t* req);
int dev_input_query(dev_input_t* dev, const char* filename);
void dev_input_scan();

#endif // _MAIN_H_