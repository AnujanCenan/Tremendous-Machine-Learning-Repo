#include "../include/csv.h"

#define ROW_BUFFER 1000
#define FIELD_BUFFER 100
#define MAX_NUM_ROWS 1000

#define DECLARE_DYNAMIC_ARRAY(type, type_name) \
    typedef struct {                \
        type* data;                 \
        size_t size;                \
        size_t capacity;            \
    } type_name;       \
                                    \
    static inline type* insert_into_arr(type_name *arr, type value)    \
    { \
        if (arr->size >= arr->capacity) { \
            arr->capacity = arr->capacity ? arr->capacity * 2 : 1; \
            arr->data = realloc(arr->data, sizeof(type) * arr->capacity); \
        } \
        arr->data[arr->size++] = value; \
    }
    

struct csv_row_info {
    char** row;
    int num_fields;
};

struct all_rows_info
{
    Csv_Row_Info** all_rows;
    int num_rows;
};

DECLARE_DYNAMIC_ARRAY(char***, all_rows);
DECLARE_DYNAMIC_ARRAY(char**, single_row);




typedef struct csv_row_info     Csv_Row_Info;
typedef struct all_rows_info    All_Rows_Info;



/**
 * Freeing all malloced resources in this file.
 * Frees the 'strings' (field values) in each row array
 * Frees all of the row arrays
 * Frees the all_rows array.
 * Frees the all_rows_info wrapper.
 */
void free_resources(All_Rows_Info* all_rows_info)
{

    int num_rows = all_rows_info->num_rows;
    Csv_Row_Info** all_rows = all_rows_info->all_rows;

    for (int row_num = 0; row_num < num_rows; ++row_num)
    {
        Csv_Row_Info* curr_row = all_rows[row_num];

        for (int field_num = 0; field_num < curr_row->num_fields; ++field_num)
        {
            free(curr_row->row[field_num]);
        }

        free(curr_row->row);
        free(curr_row);
    }
    free(all_rows);     // shout out to leaks --atExit -- <program>
    free(all_rows_info);
}


/**
 * Inserts (a string value) into the given row/line
 * Parameters
 * - row: array of 'strings' - expect to contain some first k values from line n
 * of the csv file
 * - curr_field: the string to insert into row - a.k.a the complete, most recent 
 * value read from the csv file
 * - row_idx: the next empty position in row
 * - row_capacity: the current capacity of row
 */
char** insert_into_row(char** row, char* curr_field, int* row_idx, int* row_capacity)
{
    if (*row_idx >= *row_capacity)
    {        
        *row_capacity = *row_capacity == 0 ? 1 : *row_capacity * 2;
        row = realloc(row, *row_capacity * sizeof(char *));
    }

    row[*row_idx] = curr_field;
    (*row_idx)++;

    return row;
} 


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
 * 
 */
Csv_Row_Info** insert_row_into_table(Csv_Row_Info** all_rows, Csv_Row_Info* row, int* all_rows_idx, int* all_rows_capacity)
{
    if (*all_rows_idx >= *all_rows_capacity)
    {
        *all_rows_capacity *= 2;
        all_rows = realloc(all_rows, *all_rows_capacity * sizeof(Csv_Row_Info*));
    }

    all_rows[*all_rows_idx] = 
    (*all_rows_idx)++;
    
    return all_rows;
}


// private
Csv_Row_Info* get_row(FILE* f)
{
    int row_capacity = ROW_BUFFER;
    char** row = malloc(row_capacity * sizeof(char *));
    
    int field_capacity = FIELD_BUFFER;
    char* curr_field = malloc(field_capacity * sizeof(char *));

    
    Csv_Row_Info* csv_row_info = malloc(sizeof(Csv_Row_Info));

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
            // curr_field[field_idx++] = '\0';
            curr_field = insert_into_field(curr_field, '\0', &field_idx, &field_capacity);
            // row[row_idx++] = curr_field;
            row = insert_into_row(row, curr_field, &row_idx, &row_capacity);     // replace with this to handle realloc case
            curr_field = malloc(FIELD_BUFFER * sizeof(char *));
            field_idx = 0;
        } else 
        {
            // curr_field[field_idx++] = c;
            curr_field = insert_into_field(curr_field, c, &field_idx, &field_capacity);

        }

        c = fgetc(f);
    }

    if (field_idx > 0)
    {
        curr_field[field_idx++] = '\0';
        row[row_idx++] = curr_field;
        // row = insert_into_row(row, curr_field, &row_idx, &row_capacity); 
    }

    csv_row_info->num_fields = row_idx;
    csv_row_info->row = row;

    return csv_row_info;
}


// The final function will have the following properties
/**
 * Parameters
 * - Filename
 * - hasHeaders <boolean flag>
 * 
 * Returns:
 * - all_rows <All_Rows_Info *>
 * 
 * Accompanied with a free_resources function which, given all_rows (should) free
 * all memory
 */

int main()
{
    FILE* f = fopen("./example.csv", "r");
    if (f == NULL)
    {
        fprintf(stderr, "Failed to open the file\n");
        exit(1);
    }

    bool headers = true;

    if (headers)
    {
        char c = fgetc(f);
        while (c != '\n' && c != '\r')
        {
            c = fgetc(f);
        }
    }

    int all_row_capacity = MAX_NUM_ROWS;
    Csv_Row_Info** all_rows = malloc(all_row_capacity * sizeof(Csv_Row_Info*));
    int num_rows = 0;

    char c = fgetc(f);
    while (c != EOF)
    {
        ungetc(c, f);

        // all_rows[num_rows++] = get_row(f);
        all_rows = insert_row_into_table(all_rows, get_row(f), &num_rows, &all_row_capacity);
        c = fgetc(f);
    }

    int global_num_fields = -1;
    for (int row_num = 0; row_num < num_rows; ++row_num)
    {
        Csv_Row_Info* curr_csv_row = all_rows[row_num];
        if (global_num_fields == -1)
        {
            global_num_fields = curr_csv_row->num_fields;
        } else if (global_num_fields != curr_csv_row->num_fields)
        {
            fprintf(stderr, "I believe you have different number of fields in two lines\n");
        }
        
        for (int field_num = 0; field_num < curr_csv_row->num_fields; ++field_num)
        {
            fputs(curr_csv_row->row[field_num], stdout);
            printf("\t");
        }

        printf("\n");
    }

    All_Rows_Info* all_rows_info = malloc(sizeof(All_Rows_Info));     // 0.012
    all_rows_info->num_rows = num_rows;
    all_rows_info->all_rows = all_rows;

    free_resources(all_rows_info);

    return 0;
}