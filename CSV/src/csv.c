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
            // curr_field = insert_into_field(curr_field, c, &field_idx, &field_capacity);
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

    All_Rows* all_rows = All_Rows_init(INIT_NUM_LINES);

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

int main()
{
    FILE* file_ptr = fopen("./example.csv", "r");
    bool headers = true;

    All_Rows* all_rows = get_all_rows(file_ptr, headers);
    print_extracted_csv(all_rows);

    free_resources(all_rows);
    return 0;
}