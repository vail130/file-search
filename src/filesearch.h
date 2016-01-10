#ifndef FILESEARCH_H
#define FILESEARCH_H

#include <stdbool.h>

typedef enum { GLOB_MODE, REGEX_MODE } PatternMode;
typedef enum { ALL_TYPE, FILE_TYPE, DIR_TYPE } ResultType;
typedef struct config {
    PatternMode mode;
    ResultType type;
    bool show_stats;
    char *output_file;
} Config;

int filesearch(const char *path, const char *pattern, Config opts);

#endif /* FILESEARCH_H */
