#include <opensocdebug.h>

#include <getopt.h>
#include <readline/readline.h>
#include <readline/history.h>
#include <stdlib.h>
#include <errno.h>
#include "cli.h"

static void print_help_commands(void) {
    fprintf(stderr, "Available commands:\n"                    );
    fprintf(stderr, "  help        Print this help\n"          );
    fprintf(stderr, "  <cmd> help  Print help for command\n"   );
    fprintf(stderr, "  quit        Exit the command line\n"    );
    fprintf(stderr, "  reset       Reset the system\n"         );
    fprintf(stderr, "  start       Start the processor cores\n");
    fprintf(stderr, "  mem         Access memory\n"            );
}

static void print_help_reset(void) {
    fprintf(stderr, "Available parameters:\n"                           );
    fprintf(stderr, "  -halt       Halt processor cores until 'start'\n");
}

static void print_help_start(void) {
    fprintf(stderr, "Start cores after 'reset', no parameters\n");
}

static void print_help_mem(void) {
    fprintf(stderr, "Available subcommands:\n"                          );
    fprintf(stderr, "  help        Print this help\n"                   );
    fprintf(stderr, "  test        Run memory tests\n"                  );
    fprintf(stderr, "  loadelf     Load an elf to memory\n"             );
}

static void print_help_mem_loadelf(void) {
    fprintf(stderr, "Usage: mem loadelf <file> <memid>\n"   );
    fprintf(stderr, "  file   Filename to load\n"           );
    fprintf(stderr, "  memid  Module identifier of memory\n");
}
#define CHECK_MATCH(input, string) \
        (input && !strncmp(input, string, strlen(string)+1))

int main(int argc, char* argv[]) {
    struct osd_context *ctx;

    osd_new(&ctx, OSD_MODE_DAEMON, 0, 0);

    if (osd_connect(ctx) != OSD_SUCCESS) {
        fprintf(stderr, "Cannot connect to Open SoC Debug daemon\n");
        exit(1);
    }

    char *line = 0;
    while (1) {
        if (line) {
            free(line);
        }
        line = readline("osd> ");
        add_history(line);

        char *cmd = strtok(line, " ");

        if (CHECK_MATCH(cmd, "help") ||
                CHECK_MATCH(cmd, "h")) {
            print_help_commands();
        } else if (CHECK_MATCH(cmd, "quit") ||
                CHECK_MATCH(cmd, "q") ||
                CHECK_MATCH(cmd, "exit")) {
            return 0;
        } else if (CHECK_MATCH(cmd, "reset")) {
            int haltcpus = 0;
            char *subcmd = strtok(NULL, " ");

            if (CHECK_MATCH(subcmd, "help")) {
                print_help_reset();
            } else if (CHECK_MATCH(subcmd, "-halt")) {
                haltcpus = 1;
            }

            osd_reset_system(ctx, haltcpus);
        } else if (CHECK_MATCH(cmd, "start")) {
            char *subcmd = strtok(NULL, " ");

            if (CHECK_MATCH(subcmd, "help")) {
                print_help_start();
            } else if (subcmd) {
                fprintf(stderr, "No parameters accepted or unknown subcommand: %s", subcmd);
                print_help_start();
            } else {
                osd_start_cores(ctx);
            }
        } else if (CHECK_MATCH(cmd, "mem")) {
            char *subcmd = strtok(NULL, " ");

            if (CHECK_MATCH(subcmd, "help")) {
                print_help_mem();
            } else if (CHECK_MATCH(subcmd, "test")) {
                memory_tests(ctx);
            } else if (CHECK_MATCH(subcmd, "loadelf")) {
                subcmd = strtok(NULL, " ");

                if (CHECK_MATCH(subcmd, "help")) {
                    print_help_mem_loadelf();
                    continue;
                } else if (!subcmd){
                    fprintf(stderr, "Missing filename\n");
                    print_help_mem_loadelf();
                    continue;
                }
                char *file = subcmd;
                char *smem = strtok(NULL, " ");

                if (!smem) {
                    fprintf(stderr, "Missing memory id\n");
                    print_help_mem_loadelf();
                    continue;
                }

                errno = 0;
                unsigned int mem = strtol(smem, 0, 0);
                if (errno != 0) {
                    fprintf(stderr, "Invalid memory id: %s\n", smem);
                    print_help_mem_loadelf();
                    continue;
                }
                osd_memory_loadelf(ctx, mem, file);
            }
        } else {
            fprintf(stderr, "Unknown command: %s\n", cmd);
            print_help_commands();
        }
    }
}
