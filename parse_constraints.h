/* Header file for parse_constraints.c */
#ifndef PARSE_CONSTRAINTS_H
#define PARSE_CONSTRAINTS_H
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>

typedef struct constraints_s { // Holds all the constraints
    int num_sessions; /* Number of sessions */
    int num_subjects; /* Number of subjects */
    int num_teachers; /* Number of teachers */
    int num_sections; /* Number of sections */
    int num_possible; /* Number of possiblities */
    char *subjects; /* Array of null-terminated strings */
    char *teachers; /* Ditto */
    char *sections; /* Same, but for grades */
    uint64_t *teach_table; /* Which teacher teaches what (first 56 bits are (1 << teacher_index) and last byte is subject */
    int *sessions_table; /* Rectangle, width is num_subjects, height is num_grades, index using indices, num_sessions per week */
} Constraints;

Constraints *parse_constraints_file(char *path);
char *strlist_get_nth(char *stlist, int n);
int strlist_get_index(char *strlist, int length, char *needle);
#define bitscan(x) __builtin_ctzll(x) /* Bitscan Forward */
#define TEACHER_MASK 0x00FFFFFFFFFFFFFF

#endif
