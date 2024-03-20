#include "run.h"

#include "argtable3.h"
#include "crow.h"
#include "sv.h"
#include "uv.h"

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <mutex>
#include <unordered_set>

std::mutex mtx;
std::unordered_set<crow::websocket::connection*> users;

int do_run_subcommand(const char* prog, const char* subcommand, int argc, char** argv) {
  arg_file_t* dev_arg = arg_file1(nullptr, nullptr, nullptr, "/dev/input/eventX path.");
  arg_int_t* port_arg = arg_int0("p", "port", nullptr, "The port to run the server on.");
  arg_lit_t* help_arg = arg_lit0("h", "help", "print this help and exit.");
  arg_lit_t* vers_arg = arg_lit0("v", "version", "print version information and exit.");
  arg_end_t* end_arg  = arg_end(20);
  void* argtable[]    = {dev_arg, port_arg, help_arg, vers_arg, end_arg};

  if (arg_nullcheck(argtable) != 0) {
    fprintf(stderr, "%s: insufficient memory\n", argv[0]);
    return 1;
  }

  int nerrors = arg_parse(argc, argv, argtable);

  if (help_arg->count > 0) {
    printf("Usage: %s", argv[0]);
    arg_print_syntax(stdout, argtable, "\n");
    arg_print_glossary(stdout, argtable, "  %-10s %s\n");
    arg_freetable(argtable, sizeof(argtable) / sizeof(argtable[0]));
    return 0;
  }

  if (vers_arg->count > 0) {
    printf("March 2024, Dalton Overmyer\n");
    arg_freetable(argtable, sizeof(argtable) / sizeof(argtable[0]));
    return 0;
  }

  if (nerrors > 0) {
    arg_print_errors(stderr, end_arg, argv[0]);
    fprintf(stderr, "Try '%s %s --help' for more information.\n", prog, subcommand);
    arg_freetable(argtable, sizeof(argtable) / sizeof(argtable[0]));
    return 1;
  }

  req_data_t req_data;

  strncpy(req_data.filename, dev_arg->filename[0], MAX_DEV_INPUT_PATH);

  req_data.read_req.data = &req_data;
  uv_fs_stat(uv_default_loop(), &req_data.read_req, req_data.filename, dev_fs_stat_cb);

  req_data.poll_req.data = &req_data;
  uv_fs_poll_init(uv_default_loop(), &req_data.poll_req);
  uv_fs_poll_start(&req_data.poll_req, dev_fs_poll_cb, req_data.filename, 1000);

  crow::SimpleApp app;

  CROW_ROUTE(app, "/")
  ([](const crow::request& req, crow::response& res) {
    res.set_static_file_info("static/index.html");
    res.end();
  });

  CROW_ROUTE(app, "/headers")
  ([](const crow::request& req) {
    std::string s =
        "<!DOCTYPE html>\n"
        "<html lang=\"en\">\n"
        "  <head>\n"
        "    <title>scannerd</title>\n"
        "    <style>\n"
        "    table {\n"
        "      border: 1px solid black;\n"
        "      table-layout:fixed;\n"
        "      border-collapse: collapse;\n"
        "      width: 100%;\n"
        "    }\n"
        "    th {\n"
        "      padding-top: 12px;\n"
        "      padding-bottom: 12px;\n"
        "      text-align: left;\n"
        "      background-color: #04AA6D;\n"
        "      color: white;\n"
        "    }\n"
        "    th, td {\n"
        "      border: 1px solid black;\n"
        "      padding: 8px;\n"
        "      word-wrap: break-word;\n"
        "    }\n"
        "    tr:nth-child(even){background-color: #f2f2f2;}\n"
        "    tr:hover {background-color: #ddd;}\n"
        "    </style>\n"
        "  </head>\n"
        "  <body>\n"
        "    <table>\n"
        "      <thead>\n"
        "        <tr>\n"
        "          <th>\n"
        "            Name\n"
        "          </th>\n"
        "          <th>\n"
        "            Value\n"
        "          </th>\n"
        "        </tr>\n"
        "      </thead>\n"
        "      <tbody>\n";

    for (auto& i : req.headers) {
      s += "        <tr>\n";
      s += "          <td>\n";
      s += "            " + i.first + "\n";
      s += "          </td>\n";
      s += "          <td>\n";
      s += "            " + i.second + "\n";
      s += "          </td>\n";
      s += "        </tr>\n";
    }
    s += "      </tbody>\n"
         "    </table>\n"
         "  </body>\n"
         "</html>\n";

    return s;
  });

  CROW_ROUTE(app, "/last")
  ([&req_data](const crow::request& req) {
    std::lock_guard<std::mutex> _(mtx);
    crow::json::wvalue r({{"data", std::vector<crow::json::wvalue>()}});

    auto& arr = req_data.last_input_buf.array();
    int index = 0;
    for (int i = req_data.last_input_buf.size() - 1; i >= 0; i--) {
      r["data"][index++] = arr.at(i);
    }

    return r;
  });

  CROW_ROUTE(app, "/last/<int>")
  ([&req_data](const crow::request& req, int count) {
    std::lock_guard<std::mutex> _(mtx);
    crow::json::wvalue r({{"data", std::vector<crow::json::wvalue>()}});

    auto& arr = req_data.last_input_buf.array();
    int index = 0;
    for (int i = (count > req_data.last_input_buf.size() ? req_data.last_input_buf.size() : count) - 1; i >= 0; i--) {
      r["data"][index++] = arr.at(i);
    }

    return r;
  });

  CROW_ROUTE(app, "/first")
  ([&req_data](const crow::request& req) {
    std::lock_guard<std::mutex> _(mtx);
    crow::json::wvalue r({{"data", std::vector<crow::json::wvalue>()}});

    auto& arr = req_data.last_input_buf.array();
    for (int i = 0; i < req_data.last_input_buf.size(); i++) {
      r["data"][i] = arr.at(i);
    }

    return r;
  });

  CROW_ROUTE(app, "/first/<int>")
  ([&req_data](const crow::request& req, int count) {
    std::lock_guard<std::mutex> _(mtx);
    crow::json::wvalue r({{"data", std::vector<crow::json::wvalue>()}});

    auto& arr = req_data.last_input_buf.array();
    for (int i = 0; i < (count > req_data.last_input_buf.size() ? req_data.last_input_buf.size() : count); i++) {
      r["data"][i] = arr.at(i);
    }

    return r;
  });

  CROW_WEBSOCKET_ROUTE(app, "/ws")
      .onopen([&](crow::websocket::connection& conn) {
        CROW_LOG_INFO << "new websocket connection from " << conn.get_remote_ip();
        std::lock_guard<std::mutex> _(mtx);
        users.insert(&conn);
      })
      .onclose([&](crow::websocket::connection& conn, const std::string& reason) {
        CROW_LOG_INFO << "websocket connection closed: " << reason;
        std::lock_guard<std::mutex> _(mtx);
        users.erase(&conn);
      })
      .onmessage([&](crow::websocket::connection& conn, const std::string& data, bool is_binary) {
        CROW_LOG_WARNING << "We never expect data from the client, so we close the connection.";
        conn.close();
      });

  uint16_t port = port_arg->count > 0 ? port_arg->ival[0] : 18080;

  auto f = app.port(port).run_async();

  while (1) {
    uv_run(uv_default_loop(), UV_RUN_ONCE);
    if (f.wait_for(std::chrono::seconds(0)) == std::future_status::ready) {
      break;
    }
  }

  CROW_LOG_INFO << "Exiting 'run' subcommand.";

  arg_freetable(argtable, sizeof(argtable) / sizeof(argtable[0]));
  uv_loop_close(uv_default_loop());

  return 0;
}

void dev_signal_cb(uv_signal_t* handle, int signum) {
  uv_stop(uv_default_loop());
}

void dev_fs_poll_cb(uv_fs_poll_t* req, int status, const uv_stat_t* prev, const uv_stat_t* curr) {
  req_data_t* data = (req_data_t*)req->data;

  if (status < 0) {
    CROW_LOG_WARNING << "'" << data->filename << "': " << uv_strerror(status) << ", will wait for a connection.";
    return;
  }
  if (data->initalized) {
    CROW_LOG_WARNING << "'" << data->filename << "': device already initialized.";
    return;
  }

  CROW_LOG_INFO << "'" << data->filename << "': device found, attempting to initialize.";
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
      } else if (data->ev.code == KEY_ENTER || data->ev.code == KEY_KPENTER) {
        if (data->input_buf.size() != 0) {
          std::lock_guard<std::mutex> _(mtx);
          CROW_LOG_INFO << data->info.name << ": " << data->input_buf;

          for (auto u : users) {
            u->send_text(data->input_buf);
          }

          data->last_input_buf.push(data->input_buf);
          data->input_buf.clear();
        }
      } else {
        int shifted = data->lshift || data->rshift;
        if (code_to_key(shifted, data->ev.code) != '\0') {
          if (data->input_buf.size() <= INPUT_BUF_LENGTH - 1) {
            data->input_buf += code_to_key(shifted, data->ev.code);
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
    CROW_LOG_ERROR << "Error opening file: '" << data->filename << "': " << uv_strerror(req->result);
    uv_fs_req_cleanup(req);
    return;
  }

  data->initalized  = 1;
  data->file_id     = req->result;
  data->ev_buf.base = (char*)&data->ev;
  data->ev_buf.len  = sizeof(data->ev);

  data->input_buf = "";
  data->lshift    = 0;
  data->rshift    = 0;

  CROW_LOG_INFO << data->filename
                << ": initialized"
                << " (name: '"
                << data->info.name
                << "' phys: '"
                << data->info.phys
                << "' uniq: '"
                << data->info.uniq
                << "')";

  uv_fs_req_cleanup(req);
  uv_fs_read(uv_default_loop(), req, data->file_id, &data->ev_buf, 1, -1, dev_fs_read_cb);
}

void dev_fs_stat_cb(uv_fs_t* req) {
  req_data_t* data = (req_data_t*)req->data;

  if (req->result < 0) {
    CROW_LOG_WARNING << data->filename << ": " << uv_strerror(req->result) << ", will wait for a connection.";
    uv_fs_req_cleanup(req);
    return;
  }

  if (!S_ISCHR(req->statbuf.st_mode)) {
    CROW_LOG_ERROR << data->filename << ": Not a character device.";
    uv_fs_req_cleanup(req);
    return;
  }

  if (!dev_input_query(&data->info, data->filename)) {
    CROW_LOG_ERROR << data->filename << ": Failed to grab evdev info.";
    uv_fs_req_cleanup(req);
    return;
  }

  if (!bit_is_set(data->info.bits, EV_KEY)) {
    CROW_LOG_ERROR << data->filename << ": Device does not support EV_KEY event.";
    uv_fs_req_cleanup(req);
    return;
  }

  uv_fs_req_cleanup(req);
  uv_fs_open(uv_default_loop(), req, data->filename, O_RDONLY, 0, dev_fs_open_cb);
}

char code_to_key(int shifted, unsigned short code) {
  if (!shifted) {
    switch (code) {
        // clang-format off
      case KEY_0:          return '0';
      case KEY_1:          return '1';
      case KEY_2:          return '2';
      case KEY_3:          return '3';
      case KEY_4:          return '4';
      case KEY_5:          return '5';
      case KEY_6:          return '6';
      case KEY_7:          return '7';
      case KEY_8:          return '8';
      case KEY_9:          return '9';
      case KEY_KP0:        return '0';
      case KEY_KP1:        return '1';
      case KEY_KP2:        return '2';
      case KEY_KP3:        return '3';
      case KEY_KP4:        return '4';
      case KEY_KP5:        return '5';
      case KEY_KP6:        return '6';
      case KEY_KP7:        return '7';
      case KEY_KP8:        return '8';
      case KEY_KP9:        return '9';
      case KEY_A:          return 'a';
      case KEY_B:          return 'b';
      case KEY_C:          return 'c';
      case KEY_D:          return 'd';
      case KEY_E:          return 'e';
      case KEY_F:          return 'f';
      case KEY_G:          return 'g';
      case KEY_H:          return 'h';
      case KEY_I:          return 'i';
      case KEY_J:          return 'j';
      case KEY_K:          return 'k';
      case KEY_L:          return 'l';
      case KEY_M:          return 'm';
      case KEY_N:          return 'n';
      case KEY_O:          return 'o';
      case KEY_P:          return 'p';
      case KEY_Q:          return 'q';
      case KEY_R:          return 'r';
      case KEY_S:          return 's';
      case KEY_T:          return 't';
      case KEY_U:          return 'u';
      case KEY_V:          return 'v';
      case KEY_W:          return 'w';
      case KEY_X:          return 'x';
      case KEY_Y:          return 'y';
      case KEY_Z:          return 'z';
      case KEY_TAB:        return '\t';
      case KEY_SPACE:      return ' ';
      case KEY_GRAVE:      return '`';
      case KEY_MINUS:      return '-';
      case KEY_EQUAL:      return '=';
      case KEY_LEFTBRACE:  return '[';
      case KEY_RIGHTBRACE: return ']';
      case KEY_BACKSLASH:  return '\\';
      case KEY_SEMICOLON:  return ';';
      case KEY_APOSTROPHE: return '\'';
      case KEY_COMMA:      return ',';
      case KEY_DOT:        return '.';
      case KEY_SLASH:      return '/';
      default:             return '\0';
        // clang-format on
    }
  } else {
    switch (code) {
        // clang-format off
        case KEY_0:          return ')';
        case KEY_1:          return '!';
        case KEY_2:          return '@';
        case KEY_3:          return '#';
        case KEY_4:          return '$';
        case KEY_5:          return '%';
        case KEY_6:          return '^';
        case KEY_7:          return '&';
        case KEY_8:          return '*';
        case KEY_9:          return '(';
        case KEY_A:          return 'A';
        case KEY_B:          return 'B';
        case KEY_C:          return 'C';
        case KEY_D:          return 'D';
        case KEY_E:          return 'E';
        case KEY_F:          return 'F';
        case KEY_G:          return 'G';
        case KEY_H:          return 'H';
        case KEY_I:          return 'I';
        case KEY_J:          return 'J';
        case KEY_K:          return 'K';
        case KEY_L:          return 'L';
        case KEY_M:          return 'M';
        case KEY_N:          return 'N';
        case KEY_O:          return 'O';
        case KEY_P:          return 'P';
        case KEY_Q:          return 'Q';
        case KEY_R:          return 'R';
        case KEY_S:          return 'S';
        case KEY_T:          return 'T';
        case KEY_U:          return 'U';
        case KEY_V:          return 'V';
        case KEY_W:          return 'W';
        case KEY_X:          return 'X';
        case KEY_Y:          return 'Y';
        case KEY_Z:          return 'Z';
        case KEY_TAB:        return '\t';
        case KEY_SPACE:      return ' ';
        case KEY_GRAVE:      return '~';
        case KEY_MINUS:      return '_';
        case KEY_EQUAL:      return '+';
        case KEY_LEFTBRACE:  return '{';
        case KEY_RIGHTBRACE: return '}';
        case KEY_BACKSLASH:  return '|';
        case KEY_SEMICOLON:  return ':';
        case KEY_APOSTROPHE: return '"';
        case KEY_COMMA:      return '<';
        case KEY_DOT:        return '>';
        case KEY_SLASH:      return '?';
        default:             return '\0';
        // clang-format on
    }
  }
  return '\0';
}