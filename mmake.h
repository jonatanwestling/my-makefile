/* 
 * This is a headerfile to the program mmake.c
 * @file: mmake.h
 * @author: Jonatan Westling(c22jwg@cs.umu.se)
 * @version information:
 *   2023-09-27: v1.0, first version.
 */
#ifndef MMAKE_H
#define MMAKE_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/wait.h>

//errors with global variables from getopt dissapered with this header 
#include <getopt.h>
#include "parser.h"

typedef struct flags flags;

void check_targets(makefile *m, const char *targetnamem, flags *f);
bool first_file_is_newer(const char *file1, const char *file2);
void compile_target_rule(rule *target_rule, flags *f);
bool non_existing_file(const char *targetname);
bool target_needs_to_rebild(const char **prereq, const char *target, flags *f);
#endif

