#include "../include/csv.h"

/////////////////////////////////// PRIVATE ////////////////////////////////////

/**
 * Extracts the values from a particular line of a csv file. Mallocs a Row_Info and
 * chucks those values into a Row_Info dynamic array (vector<char *>).
 * Returns that Row_Info's address
 * 
 * Parameters:
 * - f: the file pointer. Assumes the file pointer points to the first byte of a line
 */
Row_Info* get_row(FILE* f)
{
    Row_Info* csv_row_info = Row_Info_init(INIT_NUM_COLS);
    
    int field_capacity = MAX_VALUE_LENGTH;
    Field_Info* curr_field_info = Field_Info_init(MAX_VALUE_LENGTH);
 
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
                Field_Info_append(curr_field_info, '\"');
                
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
            Field_Info_append(curr_field_info, '\0');
            
            Row_Info_append(csv_row_info, curr_field_info);

            curr_field_info= Field_Info_init(MAX_VALUE_LENGTH);

        } else 
        {
            Field_Info_append(curr_field_info, c);
        }

        c = fgetc(f);
    }

    if (curr_field_info->size > 0)
    {
        Field_Info_append(curr_field_info, '\0');
        Row_Info_append(csv_row_info, curr_field_info);
    } else {
        free(curr_field_info->data);        // cheeky bastard
        free(curr_field_info);
    }

    return csv_row_info;
}
//////////////////////////////////// PUBLIC ////////////////////////////////////

void free_resources(All_Rows* all_rows_info)
{
    if (all_rows_info == NULL)
    {
        return;
    }

    for (int row_info_num = 0; row_info_num < all_rows_info->size; ++row_info_num)
    {
        Row_Info* row_info = all_rows_info->data[row_info_num];
        for (int field_info_num = 0; field_info_num < row_info->size; ++field_info_num)
        {
            Field_Info* field_info = row_info->data[field_info_num];
            free(field_info->data);
            free(field_info);
        }
        free(row_info->data);
        free(row_info);
    }
    free(all_rows_info->data);
    free(all_rows_info);
}

void skip_line(FILE* file_ptr)
{
    char c = fgetc(file_ptr);
    while (c != '\n' && c != '\r' && c != EOF)
    {
        c = fgetc(file_ptr);
    }

    if (c == EOF)
    {
        printf("<csv> SKIP_LINE warning: end-of-file found; file pointer points past the file content now\n");
    }
}


All_Rows* get_all_rows(FILE* file_ptr, bool headers)
{
    if (file_ptr == NULL)
    {
        fprintf(stderr, "GET_ALL_ROWS: file_ptr is null - potentially failed to open file\n");
        exit(1);
    }

    char c = fgetc(file_ptr);
    if (c == EOF)
    {
        return NULL;
    }

    ungetc(c, file_ptr);

    if (headers)
    {
        skip_line(file_ptr);
    }

    c = fgetc(file_ptr);

    All_Rows* all_rows = All_Rows_init(INIT_NUM_LINES);

    
    while (c != EOF)
    {
        ungetc(c, file_ptr);

        All_Rows_append(all_rows, get_row(file_ptr));
        c = fgetc(file_ptr);
    }

    fclose(file_ptr);

    return all_rows;
}


All_Rows* get_next_k_row(FILE* file_ptr, int batch_size)
{
    if (!file_ptr)
    {
        fprintf(stderr, "GET NEXT K ROWS: file_ptr is null - potentially failed to open file\n");
        exit(1);
    }

    char c = fgetc(file_ptr);
    All_Rows* all_rows = All_Rows_init(batch_size);
    for (int curr_batch = 0; curr_batch < batch_size; ++curr_batch)
    {
        if (c == EOF)
        {
            fclose(file_ptr);
            break;
        }

        ungetc(c, file_ptr);

        All_Rows_append(all_rows, get_row(file_ptr));
        
        c = fgetc(file_ptr);
    }

    ungetc(c, file_ptr);

    return all_rows;
}

/**
 * Assumes file_ptr does not point to the header line
 */
All_Rows* get_next_row(FILE* file_ptr)
{
    if (!file_ptr)
    {
        fprintf(stderr, "GET_NEXT_ROW: file_ptr is null - potentially failed to open file\n");
        exit(1);
    }

    return get_next_k_row(file_ptr, 1);
}

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
            Field_Info* field = curr_csv_row->data[field_num];
            fputs(field->data, stdout);
            printf("\t");
        }

        printf("\n");
    }
}

////////////////////////////////// ENTRYPOINT //////////////////////////////////

void test1()
{
    FILE* file_ptr = fopen("./example.csv", "r");
    bool headers = true;

    All_Rows* all_rows = get_all_rows(file_ptr, headers);
    print_extracted_csv(all_rows);

    free_resources(all_rows);
}

void test2()
{
    FILE* file_ptr = fopen("./example.csv", "r");

    int num_lines = 5;
    skip_line(file_ptr);

    for (int l = 1; l <= num_lines; ++l)
    {
        printf("Curr Line = %d\n", l);
        All_Rows* curr = get_next_row(file_ptr);
        printf("\t");
        print_extracted_csv(curr);
        free_resources(curr);
    }

    fclose(file_ptr);
}

void test3()
{
    FILE* file_ptr = fopen("./example.csv", "r");
    int num_lines = 5;
    skip_line(file_ptr);
    int batch_size = 2;

    int num_batches = num_lines / batch_size;

    if (num_lines % batch_size != 0) 
    {
        ++num_batches ;
    }
    
    for (int batch_num = 1; batch_num <= num_batches; ++batch_num)
    {
        printf("Curr batch = %d\n", batch_num);
        All_Rows* curr = get_next_k_row(file_ptr, batch_size);
        print_extracted_csv(curr);
        free_resources(curr);
    }

    fclose(file_ptr);
    
}


int main()
{
    test1();
    printf("\n");
    test2();
    printf("\n");
    test3();
    return 0;
}