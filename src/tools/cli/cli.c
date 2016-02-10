#include <opensocdebug.h>

#include <getopt.h>
#include <readline/readline.h>
#include <readline/history.h>

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

        if (!strcmp(cmd, "quit") ||
                !strcmp(cmd, "q") ||
                !strcmp(cmd, "exit")) {
            return 0;
        } else if (!strcmp(cmd, "reset")) {
            int haltcpus = 0;
            char *param = strtok(NULL, " ");

            if (param && !strcmp(param, "-halt")) {
                haltcpus = 1;
            }

            osd_reset_system(ctx, haltcpus);
        } else if (!strcmp(cmd, "start")) {
            osd_start_cores(ctx);
        }
    }
}
