#include <dirent.h>
#include <errno.h>
#include <glob.h>
#include <regex.h> 
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "filesearch.h"

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

int write_result_to_output_stream(const char *path, Config opts) {
    if (!object_matches_type(path, opts.type)) {
        return 0;
    }

    char *result;
    if (opts.show_stats) {
        struct stat s;
        if (stat(path, &s) != -1) {
            asprintf(&result, "%s\t%lld\t%ld\t%ld\t%ld\t%d\t%d\t%d",
                     path, s.st_size,
                     s.st_atime, s.st_mtime, s.st_ctime,
                     s.st_uid, s.st_gid, s.st_dev);
        } else {
            // Print what we have in consistent format
            asprintf(&result, "%s\t\t\t\t\t\t\t", path);
        }
    } else {
        asprintf(&result, "%s", path);
    }

    if (strcmp(opts.output_file, "-") == 0) {
        printf("%s\n", result);
    } else {
        FILE * fp;
        fp = fopen(opts.output_file, "a");
        fprintf(fp, "%s\n", result);
        fclose(fp);
    }

    free(result);
    return 0;
}

int print_glob_matching_files(const char *path, const char *pattern, Config opts) {
    bool path_has_slash = strncmp(&path[strlen(path)-1], "/", 1) == 0 ? 1 : 0;
    char *fixed_path = (char *)malloc(sizeof(char*) * strlen(path) + (path_has_slash ? 0 : 1));
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
        fprintf(stderr, "glob() failed with error: %d\n", retval);
        return retval;
    }
   
    int idx;
    for(idx = 0; idx < paths.gl_pathc; idx++) {
        write_result_to_output_stream(paths.gl_pathv[idx], opts);
    }
    globfree(&paths);
    
    DIR *dp;
    dp = opendir(fixed_path);
    if (dp == NULL) {
        return 0;
    }
    
    struct dirent *entry;
    while((entry = readdir(dp))) {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
            continue;
        }
        char sub_dir_path[strlen(fixed_path) + strlen(entry->d_name)];
        strcpy(sub_dir_path, fixed_path);
        strcat(sub_dir_path, entry->d_name);

        if (object_matches_type(sub_dir_path, DIR_TYPE)) {
            print_glob_matching_files(sub_dir_path, pattern, opts);
        }
    }

    free(fixed_path);
    closedir(dp);
    return 0;
}

int print_regex_matching_files(const char *path, regex_t regex, Config opts) {
    bool path_has_slash = strncmp(&path[strlen(path)-1], "/", 1) == 0 ? 1 : 0;
    char *fixed_path = (char *)malloc(sizeof(char*) * strlen(path) + (path_has_slash ? 0 : 1));
    strcpy(fixed_path, path);
    if (!path_has_slash) {
        strcat(fixed_path, "/");
    }

    DIR *dp;
    dp = opendir(fixed_path);
    if (dp == NULL) {
        return 0;
    }

    struct dirent *entry;
    while((entry = readdir(dp))) {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
            continue;
        }

        char sub_dir_path[strlen(fixed_path) + strlen(entry->d_name)];
        strcpy(sub_dir_path, fixed_path);
        strcat(sub_dir_path, entry->d_name);

        // Print filename that matches regular expression
        int reti = regexec(&regex, entry->d_name, 0, NULL, 0);
        if (reti == 0) {
            write_result_to_output_stream(sub_dir_path, opts);
        }

        if (object_matches_type(sub_dir_path, DIR_TYPE)) {
            print_regex_matching_files(sub_dir_path, regex, opts);
        }
    }

    free(fixed_path);
    closedir(dp);
    return 0;
}

int filesearch(const char *path, const char *pattern, Config opts) {
    if (!object_matches_type(path, DIR_TYPE)) {
        fprintf(stderr, "Invalid directory: %s\n", path);
        return -1;
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
        fprintf(stderr, "Invalid pattern mode\n");
        return -1;
    }
}

