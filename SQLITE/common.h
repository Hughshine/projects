#ifndef COMMON_H
#define COMMON_H

#include "pch.h"

typedef struct
{
    char *buffer;
    size_t buffer_length;
    ssize_t input_length;
} InputBuffer;

#endif