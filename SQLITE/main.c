#include "pch.h"
#include "cli.h"
#include "db.h"

int main(int argc, char* argv[]) {
    InputBuffer* input_buffer = new_input_buffer();
    Table* user_table = new_table();
    while(1) {
        print_prompt();
        read_input(input_buffer);
        // 如果是元指令
        if (input_buffer->buffer[0] == '.') {
            switch (do_meta_command(input_buffer)) {
            case META_COMMAND_SUCCESS:
                continue;
                break;
            case META_COMMAND_UNRECOGIZED_COMMAND:
                printf("Unrecognized command '%s'\n", input_buffer->buffer);
                continue;
            }
        }
        // 若非元指令，对它“预分析”
        Statement statement;
        switch (prepare_statement(input_buffer, &statement)) {
            case (PREPARE_SUCCESS):
                break;
            case (PREPARE_UNRECOGNIZED_STATEMENT):
                printf("Unrecognized keyword at start of '%s'.\n", input_buffer->buffer);
                continue;
            case PREPARE_SYNTAX_ERROR:
                printf("Syntax error at start of '%s'.\n", input_buffer->buffer);
                continue;
            case PREPARE_INSERT_INVALID_LENGTH:
                printf("Insertion failed due to invalid length of input. At start of '%s'.\n", input_buffer->buffer);
                continue;
            case PREPARE_INSERT_INVALID_ID:
                printf("Insertion failed due to invalid id. At start of '%s'.\n", input_buffer->buffer);
                continue;
            default:
                break;
        }
        // 执行
        switch (execute_statement(&statement, user_table)) {
            case EXECUTE_SUCCESS:
                printf("Executed.\n");
                break;
            case EXECUTE_TABLE_FULL:
                printf("Error: Table full.\n");
                break;
            default:
                printf("Error: Unknown execute result at start of '%s'.\n", input_buffer->buffer);
                break;
        }
    }
}