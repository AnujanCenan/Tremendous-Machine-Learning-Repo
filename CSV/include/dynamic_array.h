#ifndef DYNAMIC_ARRAY_H
#define DYNAMIC_ARRAY_H

/**
 * Macro for generically typed dynamic arrays
 * Parameters
 * - type: the type that each object in the array has
 * e.g. if type == int, then this is an integer array
 * - label: the name given to the struct; basically the type name for the struct
 * 
 * 
 * _append function allows a new element of type _type_ to be added to the end of 
 * the dynamic array. Handles reallocation if capacity is full.
 */
#define DECLARE_DYNAMIC_ARRAY(type, label) \
    typedef struct {                \
        type* data;                 \
        size_t size;                \
        size_t capacity;            \
    } label;       \
                                    \
    void label##_append(label* arr, type value)    \
    { \
        if (arr->size >= arr->capacity) \
        { \
            printf("Need to realloc; capacity to small\n"); \
            arr->capacity = arr->capacity > 0 ? arr->capacity * 2 : 1; \
            arr->data = realloc(arr->data, sizeof(type) * arr->capacity); \
        } \
        if (arr->data == NULL) \
        { \
            printf("Realloc seems to have failed -- may cause data issues!\n"); \
        } \
        arr->data[(arr->size)++] = value; \
    }


# endif // DYNAMIC_ARRAY_H