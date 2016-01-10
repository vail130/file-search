#include <stdbool.h>
#include <string.h>

#include "../lib/argp/include/argp.h"
#include "filesearch.h"

const char *argp_program_version = "filesearch 0.0.3";
const char *argp_program_bug_address = "https://github.com/vail130/filesearch/issues";
static char doc[] = "Find files with an easy syntax";
static char args_doc[] = "[PATH] [PATTERN]";
static struct argp_option options[] = {
    { "regex", 'r', 0, 0, "Use regular expression pattern to match filenames."},
    { "type", 't', "TYPE", 0, "Types of objects to include in output: f (files) or d (directories)"},
    { "stats", 's', 0, 0, "Print stats for each result."},
    { 0 }
};
const int NUM_ARGS = 2;
struct arguments {
    char *args[2];
    PatternMode mode;
    ResultType type;
    bool show_stats;
};

static error_t parse_opt(int key, char *arg, struct argp_state *state) {
    struct arguments *arguments = state->input;
    switch (key) {
        case 'r':
            arguments->mode = REGEX_MODE;
            break;
        case 's':
            arguments->show_stats = true;
            break;
        case 't':
            if (strcmp(arg, "f") == 0 || strcmp(arg, "=f") == 0) {
                arguments->type = FILE_TYPE;
            } else if (strcmp(arg, "d") == 0 || strcmp(arg, "=d") == 0) {
                arguments->type = DIR_TYPE;
            } else {
                arguments->type = ALL_TYPE;
            }
            break;
        case ARGP_KEY_ARG:
            if (state->arg_num >= NUM_ARGS) {
                argp_usage(state);
            }
            arguments->args[state->arg_num] = arg;
            break;
        case ARGP_KEY_END:
            if (state->arg_num < NUM_ARGS) {
                argp_usage(state);
            }
            break;
        default:
            return ARGP_ERR_UNKNOWN;
    }
    return 0;
}

static struct argp argp = { options, parse_opt, args_doc, doc, 0, 0, 0 };

int main(int argc, char *argv[]) {
    struct arguments arguments;
    arguments.mode = GLOB_MODE;
    arguments.type = ALL_TYPE;
    arguments.show_stats = false;
    argp_parse(&argp, argc, argv, 0, 0, &arguments);

    Config opts;
    opts.mode = arguments.mode;
    opts.type = arguments.type;
    opts.show_stats = arguments.show_stats;
    return filesearch(arguments.args[0], arguments.args[1], opts);
}

