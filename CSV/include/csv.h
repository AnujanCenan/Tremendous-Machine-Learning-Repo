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


DECLARE_DYNAMIC_ARRAY(char*, Row_Info);
DECLARE_DYNAMIC_ARRAY(Row_Info*, All_Rows);

All_Rows* get_all_rows(FILE* file_ptr, bool headers);
void print_extracted_csv(All_Rows* all_rows);

#endif // CSV_H
