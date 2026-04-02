#include "../include/csv.h"

/////////////////////////////////// PRIVATE ////////////////////////////////////


/**
 * Inserts (a string value) into the given row/line
 * Parameters
 * - field: the current string being 'built'; i.e. the current csv value being read in
 * - c: the next character to add to field
 * - field_idx: the next empty position in field
 * - field_capacity: the current capacity of field
 */
char* insert_into_field(char* field, char c, int* field_idx, int* field_capacity)
{

    if (*field_idx >= *field_capacity)
    {
        *field_capacity *= 2;
        field = realloc(field, *field_capacity * sizeof(c));
    }
    field[*field_idx] = c;
    (*field_idx)++;

    return field;
}

/**
 * Extracts the values from a particular line of a csv file. Mallocs a Row_Info and
 * 
 * chucks those values into a Row_Info dynamic array (vector<char *>).
 * Returns that Row_Info's address
 * 
 * Parameters:
 * - f: the file pointer. Assumes the file pointer points to the first byte of a line
 */
Row_Info* get_row(FILE* f)
{
    Row_Info* csv_row_info = malloc(sizeof(Row_Info));
    csv_row_info->capacity = INIT_NUM_COLS;
    csv_row_info->data = malloc(INIT_NUM_COLS * sizeof(char *));
    csv_row_info->size = 0;

    char** row = malloc(INIT_NUM_COLS * sizeof(char *));
    
    int field_capacity = MAX_VALUE_LENGTH;
    char* curr_field = malloc(field_capacity * sizeof(char *));

    int row_idx = 0;
    int field_idx = 0;

    char c = fgetc(f);
    bool opened_quotes = false;
    while (opened_quotes || (c != '\n' && c != '\r'))
    {
        if (c == '\"' && !opened_quotes)
        {
            opened_quotes = true;
        } else if (c == '\"' && opened_quotes)
        {
            char peek = fgetc(f);
            if (peek == '\"')
            {
                c = peek;
                curr_field[field_idx++] = '\"';
                
            } else if (peek == ',' || peek == '\n' || peek == '\r')
            {
                opened_quotes = false;
                ungetc(peek, f);
            } else 
            {
                fprintf(stderr, "I think this is illegal - closing a quote should have a comma after it i think\n");
                exit(1);
            }
        } else if (c == ',' && !opened_quotes)
        {
            curr_field = insert_into_field(curr_field, '\0', &field_idx, &field_capacity);
            
            Row_Info_append(csv_row_info, curr_field);

            curr_field = malloc(MAX_VALUE_LENGTH * sizeof(char *));
            field_idx = 0;
        } else 
        {
            curr_field = insert_into_field(curr_field, c, &field_idx, &field_capacity);
        }

        c = fgetc(f);
    }

    if (field_idx > 0)
    {
        curr_field[field_idx++] = '\0';
        Row_Info_append(csv_row_info, curr_field);
    }

    return csv_row_info;
}
//////////////////////////////////// PUBLIC ////////////////////////////////////

/**
 * Freeing all malloced resources in this file.
 * Frees the 'strings' (field values) in each row array
 * Frees all of the row arrays
 * Frees the all_rows array.
 * Frees the all_rows_info wrapper.
 */
void free_resources(All_Rows* all_rows_info)
{
    // Row_Info* row_info = all_rows_info.data;
    
    for (int i = 0; i < all_rows_info->size; ++i)
    {
        Row_Info* row_info = all_rows_info->data[i];
        for (int j = 0; j < row_info->size; ++j)
        {
            free(row_info->data[j]);
        }

        free(row_info);
    }

    free(all_rows_info);
}

// The final function will have the following properties
/**
 * Parameters
 * - file_ptr: an opened file ptr (naturally should point to a .csv file)
 * - hasHeaders: should be set to true if and only if the first line in the csv
 * file is a header line
 * 
 * Returns:
 * - all_rows <All_Rows_Info *>
 */

All_Rows* get_all_rows(FILE* file_ptr, bool headers)
{
    if (file_ptr == NULL)
    {
        fprintf(stderr, "file_ptr is null - potentially failed to open file\n");
        exit(1);
    }

    if (headers)
    {
        char c = fgetc(file_ptr);
        while (c != '\n' && c != '\r')
        {
            c = fgetc(file_ptr);
        }
    }

    All_Rows* all_rows = malloc(sizeof(All_Rows));
    all_rows->capacity = INIT_NUM_LINES;
    all_rows->data = malloc(INIT_NUM_LINES * sizeof(Row_Info *));
    all_rows->size = 0;

    char c = fgetc(file_ptr);
    while (c != EOF)
    {
        ungetc(c, file_ptr);

        All_Rows_append(all_rows, get_row(file_ptr));
        c = fgetc(file_ptr);
    }

    fclose(file_ptr);

    return all_rows;
}


/**
 * Roughly prints the csv file contents (after extraction) in the same grid 
 * structure as the original csv
 * 
 * Primarily for debugging purposes
 */
void print_extracted_csv(All_Rows* all_rows)
{
    int global_num_fields = -1;

    for (int row_num = 0; row_num < all_rows->size; ++row_num)
    {
        Row_Info* curr_csv_row = all_rows->data[row_num];
        if (global_num_fields == -1)
        {
            global_num_fields = curr_csv_row->size;
        } else if (global_num_fields != curr_csv_row->size)
        {
            fprintf(stderr, "I believe you have different number of fields in two lines\n");
        }
        
        for (int field_num = 0; field_num < curr_csv_row->size; ++field_num)
        {
            fputs(curr_csv_row->data[field_num], stdout);
            printf("\t");
        }

        printf("\n");
    }
}

////////////////////////////////// ENTRYPOINT //////////////////////////////////

int main()
{
    FILE* file_ptr = fopen("./example.csv", "r");
    bool headers = true;

    All_Rows* all_rows = get_all_rows(file_ptr, headers);
    print_extracted_csv(all_rows);

    free_resources(all_rows);
    return 0;
}