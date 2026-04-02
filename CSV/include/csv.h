#ifndef CSV_H
#define CSV_H

#include <string.h>
#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdbool.h>

#include "dynamic_array.h"

#define INIT_NUM_COLS 1000
#define MAX_VALUE_LENGTH 100
#define INIT_NUM_LINES 20

DECLARE_DYNAMIC_ARRAY(char, Field_Info)
DECLARE_DYNAMIC_ARRAY(Field_Info*, Row_Info);
DECLARE_DYNAMIC_ARRAY(Row_Info*, All_Rows);


/**
 * Freeing all malloced resources in csv.h file.
 */
void free_resources(All_Rows* all_rows_info);


/**
 * Parameters
 * - file_ptr: an opened file ptr (naturally should point to a .csv file)
 * - hasHeaders: should be set to true if and only if the first line in the csv
 * file is a header line
 * 
 * Returns:
 * - all_rows: an address to an All_Rows struct instance (dynamic array that 
 * contains Row_Info struct pointers)
 */
All_Rows* get_all_rows(FILE* file_ptr, bool headers);


/**
 * Roughly prints the csv file contents (after extraction) in the same grid 
 * structure as the original csv
 * 
 * Parameters:
 * - all_rows: address to All_Rows struct that contains all csv information
 * 
 * Primarily for debugging purposes
 */
void print_extracted_csv(All_Rows* all_rows);


#endif // CSV_H
