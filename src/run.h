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

static char code_to_key[2][KEY_CNT] = {
    {
        [KEY_0]          = '0',
        [KEY_1]          = '1',
        [KEY_2]          = '2',
        [KEY_3]          = '3',
        [KEY_4]          = '4',
        [KEY_5]          = '5',
        [KEY_6]          = '6',
        [KEY_7]          = '7',
        [KEY_8]          = '8',
        [KEY_9]          = '9',
        [KEY_KP0]        = '0',
        [KEY_KP1]        = '1',
        [KEY_KP2]        = '2',
        [KEY_KP3]        = '3',
        [KEY_KP4]        = '4',
        [KEY_KP5]        = '5',
        [KEY_KP6]        = '6',
        [KEY_KP7]        = '7',
        [KEY_KP8]        = '8',
        [KEY_KP9]        = '9',
        [KEY_A]          = 'a',
        [KEY_B]          = 'b',
        [KEY_C]          = 'c',
        [KEY_D]          = 'd',
        [KEY_E]          = 'e',
        [KEY_F]          = 'f',
        [KEY_G]          = 'g',
        [KEY_H]          = 'h',
        [KEY_I]          = 'i',
        [KEY_J]          = 'j',
        [KEY_K]          = 'k',
        [KEY_L]          = 'l',
        [KEY_M]          = 'm',
        [KEY_N]          = 'n',
        [KEY_O]          = 'o',
        [KEY_P]          = 'p',
        [KEY_Q]          = 'q',
        [KEY_R]          = 'r',
        [KEY_S]          = 's',
        [KEY_T]          = 't',
        [KEY_U]          = 'u',
        [KEY_V]          = 'v',
        [KEY_W]          = 'w',
        [KEY_X]          = 'x',
        [KEY_Y]          = 'y',
        [KEY_Z]          = 'z',
        [KEY_TAB]        = '\t',
        [KEY_SPACE]      = ' ',
        [KEY_GRAVE]      = '`',
        [KEY_MINUS]      = '-',
        [KEY_EQUAL]      = '=',
        [KEY_LEFTBRACE]  = '[',
        [KEY_RIGHTBRACE] = ']',
        [KEY_BACKSLASH]  = '\\',
        [KEY_SEMICOLON]  = ';',
        [KEY_APOSTROPHE] = '\'',
        [KEY_COMMA]      = ',',
        [KEY_DOT]        = '.',
        [KEY_SLASH]      = '/',
    },
    {
        [KEY_0]          = ')',
        [KEY_1]          = '!',
        [KEY_2]          = '@',
        [KEY_3]          = '#',
        [KEY_4]          = '$',
        [KEY_5]          = '%',
        [KEY_6]          = '^',
        [KEY_7]          = '&',
        [KEY_8]          = '*',
        [KEY_9]          = '(',
        [KEY_A]          = 'A',
        [KEY_B]          = 'B',
        [KEY_C]          = 'C',
        [KEY_D]          = 'D',
        [KEY_E]          = 'E',
        [KEY_F]          = 'F',
        [KEY_G]          = 'G',
        [KEY_H]          = 'H',
        [KEY_I]          = 'I',
        [KEY_J]          = 'J',
        [KEY_K]          = 'K',
        [KEY_L]          = 'L',
        [KEY_M]          = 'M',
        [KEY_N]          = 'N',
        [KEY_O]          = 'O',
        [KEY_P]          = 'P',
        [KEY_Q]          = 'Q',
        [KEY_R]          = 'R',
        [KEY_S]          = 'S',
        [KEY_T]          = 'T',
        [KEY_U]          = 'U',
        [KEY_V]          = 'V',
        [KEY_W]          = 'W',
        [KEY_X]          = 'X',
        [KEY_Y]          = 'Y',
        [KEY_Z]          = 'Z',
        [KEY_TAB]        = '\t',
        [KEY_SPACE]      = ' ',
        [KEY_GRAVE]      = '~',
        [KEY_MINUS]      = '_',
        [KEY_EQUAL]      = '+',
        [KEY_LEFTBRACE]  = '{',
        [KEY_RIGHTBRACE] = '}',
        [KEY_BACKSLASH]  = '|',
        [KEY_SEMICOLON]  = ':',
        [KEY_APOSTROPHE] = '"',
        [KEY_COMMA]      = '<',
        [KEY_DOT]        = '>',
        [KEY_SLASH]      = '?',
    }};

#endif // _RUN_H_