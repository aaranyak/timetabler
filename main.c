/* timetabler is a program that can generate timetables based on certain constraints */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "parse_constraints.h"

int main(int argc, char **argv) {
    /* Do any kind of testing/debugging from here */
    char *file_path = 0; if (argc - 1) file_path = argv[1]; /* Get first argument */ 
    if (!file_path) return 1; /* Only continue if a file has been provided */
    Constraints *constraints = parse_constraints_file(file_path); /* Do this */
    for (int i = 0; i < constraints->num_sections; i++) {
        printf("%s - ", strlist_get_nth(constraints->sections, i)); /* Print the section name */
        for (int j = 0; j < constraints->num_subjects; j++) printf("%s - %d sessions%s", strlist_get_nth(constraints->subjects, j), constraints->sessions_table[i * constraints->num_subjects + j], j != constraints->num_subjects - 1 ? ", " : "\n");
    }
    return 0;
}
