#include "main.h"

#include "query.h"
#include "run.h"

#include <string.h>

int run_subcommand(const char* prog, const char* subcommand, int argc, char** argv, subcommand_type type);

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

int do_help_subcommand(const char* prog, const char* subcommand, int argc, char** argv) {
  printf("Usage:\n");
  printf("   %s query\n", prog);
  printf("   %s run\n", prog);
  printf("   %s (-h | --help)\n", prog);

  return 0;
}

int run_subcommand(const char* prog, const char* subcommand, int argc, char** argv, subcommand_type type) {
  switch (type) {
    case SUBCOMMAND_QUERY:
      return do_query_subcommand(prog, subcommand, argc, argv);
    case SUBCOMMAND_RUN:
      return do_run_subcommand(prog, subcommand, argc, argv);
    case SUBCOMMAND_HELP:
      return do_help_subcommand(prog, subcommand, argc, argv);
    default: {
      fprintf(stderr, "%s: Expected (query | run | -h | --help) subcommand, got '%s'\n", prog, subcommand);
      return 1;
    }
  }

  return 1;
}