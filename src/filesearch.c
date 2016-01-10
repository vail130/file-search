#include <argp.h>
#include <dirent.h>
#include <glob.h>
#include <regex.h> 
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>

typedef enum { GLOB_MODE, REGEX_MODE } PatternMode;
typedef enum { ALL_TYPE, FILE_TYPE, DIR_TYPE } ResultType;
typedef struct config {
    PatternMode mode;
    ResultType type;
    bool show_stats;
} Config;

bool object_exists(const char *path, const char *type) {
    struct stat s;
    if(stat(path, &s) == -1) {
        if(ENOENT == errno) {
            // does not exist
            return false;
        } else {
            // error
            return false;
        }
    } else {
        if(strcmp(type, "dir") == 0 && S_ISDIR(s.st_mode)) {
            // it's a dir
            return true;
        } else if (strcmp(type, "file") == 0 && S_ISREG(s.st_mode)) {
            // it's a file
            return true;
        } else {
            // exists but is not dir or file
            return false;
        }
    }
}

bool object_matches_type(const char *path, ResultType type) {
    if (type == FILE_TYPE && object_exists(path, "file")) {
        return true;
    } else if (type == DIR_TYPE && object_exists(path, "dir")) {
        return true;
    } else if (type == ALL_TYPE) {
        return true;
    } else {
        return false;
    }
}

int print_glob_matching_files(const char *path, const char *pattern, Config opts) {
    bool path_has_slash = strncmp(&path[strlen(path)-1], "/", 1) == 0 ? 1 : 0;
    char* fixed_path = (char *)malloc(sizeof(char*) * strlen(path) + (path_has_slash ? 0 : 1));
    strcpy(fixed_path, path);
    if (!path_has_slash) {
        strcat(fixed_path, "/");
    }

    char full_pattern[strlen(fixed_path) + strlen(pattern)];
    strcpy(full_pattern, fixed_path);
    strcat(full_pattern, pattern);

    glob_t paths;
    paths.gl_pathc = 0;
    paths.gl_pathv = NULL;
    paths.gl_offs = 0;
   
    int retval = glob(full_pattern, GLOB_NOSORT, NULL, &paths);
    if(retval != 0 && retval != GLOB_NOMATCH) {
        printf("glob() failed with error: %d\n", retval);
        return retval;
    }
    
    for(int idx = 0; idx < paths.gl_pathc; idx++) {
        if (!object_matches_type(paths.gl_pathv[idx], opts.type)) {
            continue;
        }

        if (opts.show_stats) {
            struct stat s;
            int stat_return = stat(paths.gl_pathv[idx], &s);
            if (stat_return != -1) {
                printf("%s\t%lld\t%ld\t%ld\t%ld\t%d\t%d\t%d\n",
                        paths.gl_pathv[idx], s.st_size,
                        s.st_atime, s.st_mtime, s.st_ctime,
                        s.st_uid, s.st_gid, s.st_dev);
            }
        } else {
            puts(paths.gl_pathv[idx]);
        }
    }
    globfree(&paths);
    
    DIR *dp1, *dp2;
    dp1 = opendir(fixed_path);
    if (dp1 == NULL) {
        return -1;
    }
    
    struct dirent *entry;
    while((entry = readdir(dp1))) {
        // Compare more characters than length of test strings to ensure equality
        if (strncmp(entry->d_name, ".", 2) == 0 || strncmp(entry->d_name, "..", 3) == 0) {
            continue;
        }
        char sub_dir_path[strlen(fixed_path) + strlen(entry->d_name)];
        strcpy(sub_dir_path, fixed_path);
        strcat(sub_dir_path, entry->d_name);

        dp2 = opendir(sub_dir_path);
        if (dp2 != NULL) {
            closedir(dp2);
            print_glob_matching_files(sub_dir_path, pattern, opts);
        }
    }

    free(fixed_path);
    closedir(dp1);
    return retval;
}

int print_regex_matching_files(const char *path, regex_t regex, Config opts) {
    bool path_has_slash = strncmp(&path[strlen(path)-1], "/", 1) == 0 ? 1 : 0;
    char* fixed_path = (char *)malloc(sizeof(char*) * strlen(path) + (path_has_slash ? 0 : 1));
    strcpy(fixed_path, path);
    if (!path_has_slash) {
        strcat(fixed_path, "/");
    }

    DIR *dp1, *dp2;
    dp1 = opendir(fixed_path);
    if (dp1 == NULL) {
        return -1;
    }

    struct dirent *entry;
    while((entry = readdir(dp1))) {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
            continue;
        }

        char sub_dir_path[strlen(fixed_path) + strlen(entry->d_name)];
        strcpy(sub_dir_path, fixed_path);
        strcat(sub_dir_path, entry->d_name);

        // Print filename that matches regular expression
        int reti = regexec(&regex, entry->d_name, 0, NULL, 0);
        if (reti == 0 && object_matches_type(sub_dir_path, opts.type)) {
            if (!object_matches_type(sub_dir_path, opts.type)) {
                continue;
            }

            if (opts.show_stats) {
                struct stat s;
                int stat_return = stat(sub_dir_path, &s);
                if (stat_return != -1) {
                    printf("%s\t%lld\t%ld\t%ld\t%ld\t%d\t%d\t%d\n",
                            sub_dir_path, s.st_size,
                            s.st_atime, s.st_mtime, s.st_ctime,
                            s.st_uid, s.st_gid, s.st_dev);
                }
            } else {
                puts(sub_dir_path);
            }
        }

        dp2 = opendir(sub_dir_path);
        if (dp2 != NULL) {
            closedir(dp2);
            print_regex_matching_files(sub_dir_path, regex, opts);
        }
    }

    free(fixed_path);
    closedir(dp1);
    return 0;
}

int filesearch(const char *path, const char *pattern, Config opts) {
    DIR *dp;
    dp = opendir(path);
    if (dp == NULL) {
        fprintf(stderr, "Invalid directory: %s\n", path);
        return -1;
    } else {
        closedir(dp);
    }
    
    if (opts.mode == GLOB_MODE) {
        return print_glob_matching_files(path, pattern, opts);
    } else if (opts.mode == REGEX_MODE) {
        regex_t regex;
        int reti = regcomp(&regex, pattern, REG_EXTENDED);
        if (reti != 0) {
            fprintf(stderr, "Could not compile regex\n");
            return -1;
        }

        int retval = print_regex_matching_files(path, regex, opts);

        regfree(&regex);
        return retval;
    } else {
        return -1;
    }
}

const char *argp_program_version = "filesearch 0.0.2";
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
