#include "pch.h"
#include "cli.h"
#include "db.h"

int main(int argc, char* argv[]) {
    InputBuffer* input_buffer = new_input_buffer();
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
            default:
                break;
        }
        // 执行
        execute_statement(&statement);
        printf("Executed.\n");
    }
}