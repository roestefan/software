#include <opensocdebug.h>

#include <getopt.h>
#include <readline/readline.h>
#include <readline/history.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include "cli.h"

#include "terminal.h"

static void print_help(void) {
    fprintf(stderr, "Usage: osd-cli <parameters>\n"                                        );
    fprintf(stderr, "\n"                                                                   );
    fprintf(stderr, "Parameters:\n"                                                        );
    fprintf(stderr, "  -h, --help                  Print this help\n"                      );
    fprintf(stderr, "  -s <file>, --source=<file>  Read commands from file at start\n"     );
    fprintf(stderr, "  -b <file>, --batch=<file>   Read commands from file and exit\n"     );
    fprintf(stderr, "  --python                    Interpret -s and -b as python script\n" );
}

static void print_help_commands(void) {
    fprintf(stderr, "Available commands:\n"                                      );
    fprintf(stderr, "  help        Print this help\n"                            );
    fprintf(stderr, "  <cmd> help  Print help for command\n"                     );
    fprintf(stderr, "  quit        Exit the command line\n"                      );
    fprintf(stderr, "  reset       Reset the system\n"                           );
    fprintf(stderr, "  start       Start the processor cores\n"                  );
    fprintf(stderr, "  mem         Access memory\n"                              );
    fprintf(stderr, "  ctm         Configure core trace module\n"                );
    fprintf(stderr, "  stm         Configure software trace module\n"            );
    fprintf(stderr, "  terminal    Start terminal for device emulation module\n" );
    fprintf(stderr, "  wait        Wait for given seconds\n"                     );
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

static void print_help_stm(void) {
    fprintf(stderr, "Available subcommands:\n"                          );
    fprintf(stderr, "  help        Print this help\n"                   );
    fprintf(stderr, "  log         Log STM events to file\n"            );
}

static void print_help_stm_log(void) {
    fprintf(stderr, "Usage: stm log <file> <stmid>\n"      );
    fprintf(stderr, "  file   Filename to log to\n"        );
    fprintf(stderr, "  stmid  STM to receive logs from\n"  );
}

static void print_help_ctm(void) {
    fprintf(stderr, "Available subcommands:\n"                          );
    fprintf(stderr, "  help        Print this help\n"                   );
    fprintf(stderr, "  log         Log CTM events to file\n"            );
}

static void print_help_ctm_log(void) {
    fprintf(stderr, "Usage: ctm log <file> <ctmid> <elffile>\n"  );
    fprintf(stderr, "  file     Filename to log to\n"            );
    fprintf(stderr, "  ctmid    CTM to receive logs from\n"      );
    fprintf(stderr, "  elffile  ELF file to load symbols from\n" );
}

static void print_help_terminal(void) {
    fprintf(stderr, "Usage: terminal <id>\n"  );
    fprintf(stderr, "  id  DEM-UART to use\n" );
}

static void print_help_wait(void) {
    fprintf(stderr, "Usage: wait <n>\n"         );
    fprintf(stderr, "  n  Number of seconds\n"  );
}

#define CHECK_MATCH(input, string) \
        (input && !strncmp(input, string, strlen(string)+1))

static int interpret(struct osd_context *ctx, char *line) {
    char *cmd = strtok(line, " ");

    if (!cmd) {
        return 0;
    }

    if (CHECK_MATCH(cmd, "help") ||
            CHECK_MATCH(cmd, "h")) {
        print_help_commands();
    } else if (CHECK_MATCH(cmd, "quit") ||
            CHECK_MATCH(cmd, "q") ||
            CHECK_MATCH(cmd, "exit")) {
        return 1;
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
                return 0;
            } else if (!subcmd){
                fprintf(stderr, "Missing filename\n");
                print_help_mem_loadelf();
                return 0;
            }
            char *file = subcmd;
            char *smem = strtok(NULL, " ");

            if (!smem) {
                fprintf(stderr, "Missing memory id\n");
                print_help_mem_loadelf();
                return 0;
            }

            errno = 0;
            unsigned int mem = strtol(smem, 0, 0);
            if (errno != 0) {
                fprintf(stderr, "Invalid memory id: %s\n", smem);
                print_help_mem_loadelf();
                return 0;
            }
            osd_memory_loadelf(ctx, mem, file);
        }
    } else if (CHECK_MATCH(cmd, "stm")) {
        char *subcmd = strtok(NULL, " ");

        if (CHECK_MATCH(subcmd, "help")) {
            print_help_stm();
        } else if (CHECK_MATCH(subcmd, "log")) {
            subcmd = strtok(NULL, " ");

            if (CHECK_MATCH(subcmd, "help")) {
                print_help_stm_log();
                return 0;
            } else if (!subcmd){
                fprintf(stderr, "Missing filename\n");
                print_help_stm_log();
                return 0;
            }
            char *file = subcmd;
            char *sstm = strtok(NULL, " ");

            if (!sstm) {
                fprintf(stderr, "Missing STM id\n");
                print_help_stm_log();
                return 0;
            }

            errno = 0;
            unsigned int stm = strtol(sstm, 0, 0);
            if (errno != 0) {
                fprintf(stderr, "Invalid STM id: %s\n", sstm);
                print_help_stm_log();
                return 0;
            }
            osd_stm_log(ctx, stm, file);
        } else {
            print_help_stm();
        }
    } else if (CHECK_MATCH(cmd, "ctm")) {
        char *subcmd = strtok(NULL, " ");

        if (CHECK_MATCH(subcmd, "help")) {
            print_help_ctm();
        } else if (CHECK_MATCH(subcmd, "log")) {
            subcmd = strtok(NULL, " ");

            if (CHECK_MATCH(subcmd, "help")) {
                print_help_ctm_log();
                return 0;
            } else if (!subcmd){
                fprintf(stderr, "Missing filename\n");
                print_help_ctm_log();
                return 0;
            }
            char *file = subcmd;
            char *sctm = strtok(NULL, " ");

            if (!sctm) {
                fprintf(stderr, "Missing CTM id\n");
                print_help_ctm_log();
                return 0;
            }

            errno = 0;
            unsigned int ctm = strtol(sctm, 0, 0);
            if (errno != 0) {
                fprintf(stderr, "Invalid CTM id: %s\n", sctm);
                print_help_ctm_log();
                return 0;
            }

            char *elffile = strtok(NULL, " ");
            if (elffile) {
                osd_ctm_log(ctx, ctm, file, elffile);
            } else {
                osd_ctm_log(ctx, ctm, file, 0);
            }
        } else {
            print_help_stm();
        }
    } else if (CHECK_MATCH(cmd, "terminal")) {
        char *subcmd = strtok(NULL, " ");

        if (CHECK_MATCH(subcmd, "help")) {
            print_help_wait();
            return 0;
        }

        if (!subcmd) {
            fprintf(stderr, "Missing id\n");
            print_help_terminal();
            return 0;
        }

        errno = 0;
        unsigned int id = strtol(subcmd, 0, 0);
        if (errno != 0) {
            fprintf(stderr, "Invalid id: %s\n", subcmd);
            print_help_terminal();
            return 0;
        }

        if (!osd_module_is_terminal(ctx, id)) {
            fprintf(stderr, "No terminal at this id: %d\n", id);
            print_help_terminal();
            return 0;
        }

        struct terminal *term_arg;
        terminal_open(&term_arg);

        osd_module_claim(ctx, id);
        osd_module_register_handler(ctx, id, OSD_EVENT_PACKET,
                                    term_arg, terminal_ingress);

        osd_module_unstall(ctx, id);
    } else if (CHECK_MATCH(cmd, "wait")) {
        char *subcmd = strtok(NULL, " ");

        if (CHECK_MATCH(subcmd, "help")) {
            print_help_wait();
            return 0;
        }

        if (subcmd) {
            errno = 0;
            unsigned int sec = strtol(subcmd, 0, 0);
            if (errno != 0) {
                fprintf(stderr, "No valid seconds given: %s\n", subcmd);
                return 0;
            }

            sleep(sec);
        }
    } else {
        fprintf(stderr, "Unknown command: %s\n", cmd);
        print_help_commands();
    }

    return 0;
}

int main(int argc, char* argv[]) {
    struct osd_context *ctx;

    int c;
    char *source = NULL;
    int batch = 0;

    while (1) {
        static struct option long_options[] = {
            {"help",        no_argument,       0, 'h'},
            {"source",      required_argument, 0, 's'},
            {"batch",       required_argument, 0, 'b'},
            {"python",      no_argument,       0, '0'},
            {0, 0, 0, 0}
        };
        int option_index = 0;

        c = getopt_long(argc, argv, "hs:b:0", long_options, &option_index);
        if (c == -1) {
            break;
        }

        switch (c) {
        case 0:
            /* If this option set a flag, do nothing else now.   */
            if (long_options[option_index].flag != 0) {
                break;
            }
            break;
        case 's':
            source = optarg;
            break;
        case 'b':
            source = optarg;
            batch = 1;
            break;
        case '0':
            fprintf(stderr, "Python not supported\n");
            break;
        case 'h':
            print_help();
            return 0;
        default:
            print_help();
            return -1;
        }
    }

    osd_new(&ctx, OSD_MODE_DAEMON, 0, 0);

    if (osd_connect(ctx) != OSD_SUCCESS) {
        fprintf(stderr, "Cannot connect to Open SoC Debug daemon\n");
        exit(1);
    }

    char *line = 0;

    if (source != NULL) {
        FILE* fp = fopen(source, "r");
        if (fp == NULL) {
            fprintf(stderr, "cannot open file '%s'\n", source);
            return 1;
        }

        size_t n = 64;
        line = malloc(64);
        ssize_t len;
        while ((len = getline(&line, &n, fp)) > 0) {
            if (line[len-1] == '\n') {
                line[len-1] = 0;
            }
            printf("execute: %s\n", line);
            int rv = interpret(ctx, line);
            if (rv != 0) {
                return rv;
            }
        }

        free(line);
        line = 0;

        if (batch != 0) {
            return 0;
        }
    }

    while (1) {
        if (line) {
            free(line);
        }
        line = readline("osd> ");
        add_history(line);

        int rv = interpret(ctx, line);
        if (rv != 0) {
            return rv;
        }
    }
}
