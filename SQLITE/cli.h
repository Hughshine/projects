#ifndef CLI_H
#define CLI_H

#include "common.h"

// 以.开头的指令为元指令
typedef enum
{
    META_COMMAND_SUCCESS,
    META_COMMAND_UNRECOGIZED_COMMAND
} MetaCommandResult;

MetaCommandResult do_meta_command(InputBuffer *input_buffer);
void print_prompt();
InputBuffer *new_input_buffer();
void close_input_buffer(InputBuffer *input_buffer);
void read_input(InputBuffer *input_buffer);

#endif