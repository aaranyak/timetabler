/* parse_constraints.c - Parses a .csv file into a structured set of constraints */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "parse_constraints.h"

char *strlist_get_nth(char *strlist, int n) {
    /* Gets the n'th item in a strlist */
    for (int i = 0; i < n; i++) strlist += strlen(strlist) + 1; /* Move onto the next string */
    return strlist; /* This string should ideally be null-terminated */
}

int strlist_get_index(char *strlist, int length, char *needle) {
    /* Like searching for a needle in a haystack */
    for (int i = 0; i < length; i++) { /* Max will be the length */
        if (!strcmp(strlist_get_nth(strlist, i), needle)) return i; /* Ok, less efficient, but should work */
    } return -1;
}


Constraints *parse_constraints_file(char *path) {
    /* Parses a csv file of constraints */
    FILE *handle = fopen(path, "r"); /* Text CSV File */
    int file_length; for (file_length = 0; !feof(handle); file_length++) fgetc(handle); rewind(handle); /* Calculate file length */
    char *file_data = (char*)malloc(file_length); for (int i = 0; i < file_length; i++) file_data[i] = fgetc(handle); /* Get contents */
    fclose(handle); /* Close the file so that we don't leave it open (tautology) */
    int read_head = 0, write_head = 0; /* This is the thing we use for reading things from the file */
    char buffer[256]; /* Reading cells into this */
    // Create the object
    Constraints *constraints = (Constraints*)malloc(sizeof(Constraints)); /* Not that hard, is it? */

    // Get number of sessions.
    while (file_data[read_head] != ',') read_head++; /* Skip until the comma */
    read_head++; /* Skip the comma as well */
    while (file_data[read_head] != ',') buffer[write_head++] = file_data[read_head++]; /* Copy in the number */
    buffer[write_head] = 0; write_head = 0; sscanf(buffer, "%d", &constraints->num_sessions); /* Read in the num_sessions */ 
    // Now what?

    // Load in the names of grades
    while (file_data[read_head] != '\n') read_head++; /* Skip until the newline */
    while (file_data[read_head] != ',') read_head++; /* Skip until the next comma */ read_head++; /* And skip again */
    constraints->sections = (char*)malloc(sizeof(char)); /* Will make this bigger slowly */
    constraints->num_sections = 0; int list_location = 0; /* Initialise */
    while (file_data[read_head] != '\n') { /* Loop until the newline */
        if (file_data[read_head] != ',') buffer[write_head++] = file_data[read_head]; /* Read all this stuff */
        else { /* Add this to the list */
            buffer[write_head++] = 0; /* Add the null terminator */
            constraints->sections = (char*)realloc(constraints->sections, list_location + write_head); /* Add xtra space */
            memcpy(constraints->sections + list_location, buffer, write_head); /* Copy the string to the buffer */
            list_location += write_head; write_head = 0; /* Reset some stuff */
            constraints->num_sections++; /* Add another section */
        }
        if (file_data[read_head] == ',' && file_data[read_head + 1] == ',') break;
        read_head++; /* Move forward */
    }
    // Now What? (ZZ9 Plural Z Alpha)
    
    // Load in the subjects
    while (file_data[read_head] != '\n') read_head++; /* Skip until the newline */
    while (file_data[read_head] != ',') read_head++; /* Skip until the next comma */ read_head++; /* And skip again */
    constraints->subjects = (char*)malloc(sizeof(char)); /* Will make this bigger slowly */
    constraints->num_subjects = 0; list_location = 0; /* Initialise */
    while (file_data[read_head] != '\n') { /* Loop until the newline */
        if (file_data[read_head] != ',') buffer[write_head++] = file_data[read_head]; /* Read all this stuff */
        else { /* Add this to the list */
            buffer[write_head++] = 0; /* Add the null terminator */
            constraints->subjects = (char*)realloc(constraints->subjects, list_location + write_head); /* Add xtra space */
            memcpy(constraints->subjects + list_location, buffer, write_head); /* Copy the string to the buffer */
            list_location += write_head; write_head = 0; /* Reset some stuff */
            constraints->num_subjects++; /* Add another section */
        }
        if (file_data[read_head] == ',' && file_data[read_head + 1] == ',') break;
        read_head++; /* Move forward */
    }
    
    // Time to load in the teachers. One by one or something more complex?
    while (file_data[read_head] != '\n') read_head++; /* Skip until the newline */ read_head++; /* Skip the newline */
    while (file_data[read_head] != '\n') read_head++; /* Skip until the newline */ read_head++; /* Skip the newline */
    constraints->teachers = (char*)malloc(sizeof(char)); /* Will make this bigger slowly */
    constraints->num_teachers = 0; list_location = 0; /* Initialise */
    int get_back = read_head; /* Get back to here once you're done */
    while (1) { /* Until all the teachers are loaded */
        while (file_data[read_head] != ',') buffer[write_head++] = file_data[read_head++]; /* Load in the name of the teacher */
        buffer[write_head++] = 0; /* Add the null terminator */
        if (!strcmp(buffer, "Requirements")) break; /* Check if we are done loading teachers */
        constraints->teachers = (char*)realloc(constraints->teachers, list_location + write_head); /* Add xtra space */
        memcpy(constraints->teachers + list_location, buffer, write_head); /* Copy in this string */
        list_location += write_head; write_head = 0; constraints->num_teachers++;/* Reset the position pointers for the next one */
        while (file_data[read_head] != '\n') read_head++; /* Skip until the newline */ read_head++; /* Skip the newline */
    }

    // Time to load in which teachers teach what.
    read_head = get_back; /* Go back to the beginning of the teachers list */
    constraints->teach_table = (uint64_t*)malloc(sizeof(uint64_t)); /* Add space for the first one, 0 */
    *(constraints->teach_table) = 0; constraints->num_possible = 1; list_location = sizeof(uint64_t); /* Teacher/Subject combos */
    while (1) { /* Until scanning this stuff is fully over */
        write_head = 0; /* This is what caused all the errors probably */
        while (file_data[read_head] != ',') buffer[write_head++] = file_data[read_head++]; /* Load in the name of the teacher */
        buffer[write_head++] = 0; /* Add the null terminator */
        if (!strcmp(buffer, "Requirements")) break; /* Check if we are done loading teachers */
        uint64_t teacher = (uint64_t)1 << strlist_get_index(constraints->teachers, constraints->num_teachers, buffer); /* Teacher id */
        write_head = 0; /* Reset the buffer writer */
        while (file_data[++read_head] != '\n') { /* Loop through the next few characters and handle them */
            if (file_data[read_head] != ',') buffer[write_head++] = file_data[read_head]; /* This is a normal one */
            else { /* Otherwise, we have to HAAANDLE it */
                if (write_head == 0) continue; /* Remember to get to the end next time */
                buffer[write_head++] = 0; /* Add null terminator */
                constraints->teach_table = (uint64_t*)realloc(constraints->teach_table, list_location + sizeof(uint64_t)); /* space */
                uint64_t id = teacher | (uint64_t)strlist_get_index(constraints->subjects, constraints->num_subjects, buffer) << 56;
                constraints->teach_table[constraints->num_possible] = id; list_location += sizeof(uint64_t); /* Hope this works */
                write_head = 0; /* Set the write head back */
                constraints->num_possible++; /* Inc the counter */
            }
        } if (write_head != 0) { /* There is still something left */
            buffer[write_head++] = 0; /* Add null terminator */
            constraints->teach_table = (uint64_t*)realloc(constraints->teachers, list_location + sizeof(uint64_t)); /* Add xtra space */
            uint64_t id = teacher | (uint64_t)strlist_get_index(constraints->subjects, constraints->num_subjects, buffer) << 56;
            constraints->teach_table[constraints->num_possible] = id; list_location += sizeof(uint64_t); /* Hope this works */
            write_head = 0; /* Set the write head back */
            constraints->num_possible++; /* Inc the counter */
        } 
        read_head++; /* Move on to the next line */
    }

    // Time to load in the requirements (tough)
    while (file_data[read_head] != '\n') read_head++; /* Skip until the newline */ read_head++; /* Skip the newline */
    constraints->sessions_table = (int*)malloc(sizeof(int) * constraints->num_subjects * constraints->num_sections); /* The table */
    for (int i = 0; i < constraints->num_sections * constraints->num_subjects; i++) constraints->sessions_table[i] = 0; /*Cleartable*/
    for (int i = 0; i < constraints->num_sections; i++) { /* Loop through all the sections */
        write_head = 0; /* Clear the buffer */
        while (file_data[read_head] != ',') buffer[write_head++] = file_data[read_head++]; /* Read the name of the grade */
        buffer[write_head] = 0; /* Add null terminator */ read_head++; /* skip the comma */
        int section_index = strlist_get_index(constraints->sections, constraints->num_sections, buffer); /* Get the section id */
        while (1) { /* Until we have reached the end */
            if (file_data[read_head] == ',' || file_data[read_head] == '\n' || read_head >= file_length - 1) break; /* Check if we have reached the end of a line */
            write_head = 0; /* Clear the buffer */
            while (file_data[read_head] != ',') buffer[write_head++] = file_data[read_head++]; /* Read in the next one */
            buffer[write_head] = 0; /* Add null terminator */
            int subject_index = strlist_get_index(constraints->subjects, constraints->num_subjects, buffer); /* Get the subject id */
            read_head++; write_head = 0; /* Get ready for loading the next cell */
            while (file_data[read_head] != ',' && file_data[read_head] != '\n' && read_head < file_length) buffer[write_head++] = file_data[read_head++]; /* Read in the next cell */
            read_head++; /* Skip the comma or newline */
            buffer[write_head] = 0; /* Add null terminator */
            sscanf(buffer, "%d", &constraints->sessions_table[section_index * constraints->num_subjects + subject_index]); /* Set the number imn the correct index */
        }
        if (file_data[read_head] == ',') while (file_data[read_head] != '\n') read_head++; /* Skip to the newline */
        read_head++; /* skip the newline */
    }

    return constraints; // Oh Well...
}
