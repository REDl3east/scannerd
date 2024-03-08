#include "query.h"

#include "argtable3.h"
#include "evdev.h"
#include "sv.h"

#include <dirent.h>
#include <stdlib.h>
#include <string.h>

static int dev_input_compare(const struct dirent** d1, const struct dirent** d2);
void dev_input_scan(int meta);

int do_query_subcommand(const char* prog, const char* subcommand, int argc, char** argv) {
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
    fprintf(stderr, "Try '%s %s --help' for more information.\n", prog, subcommand);
    arg_freetable(argtable, sizeof(argtable) / sizeof(argtable[0]));
    return 1;
  }

  dev_input_scan(meta->count);
  return 0;
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

  if (device_count > 0) {
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
  }

  free(devices);
  for (int i = 0; i < n; i++)
    free(namelist[i]);
  free(namelist);

  return;
}