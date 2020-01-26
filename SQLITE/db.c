#include "db.h"

// 对非元指令做预处理，解析为Statement. 可以理解为“虚拟机语言”
PrepareResult prepare_statement(InputBuffer *input_buffer, Statement *statement)
{
    if (strncmp(input_buffer->buffer, "insert", 6) == 0)
    { // strncmp, 比较指定长度的字符串
        statement->type = STATEMENT_INSERT;

        int args_assigned = sscanf(input_buffer->buffer, "insert %d %s %s", &(statement->row_to_insert.id), statement->row_to_insert.username, statement->row_to_insert.email);
        if (args_assigned < 3)
            return PREPARE_SYNTAX_ERROR;

        return PREPARE_SUCCESS;
    }
    else if (strncmp(input_buffer->buffer, "select", 6) == 0)
    {
        statement->type = STATEMENT_SELECT;
        return PREPARE_SUCCESS;
    }

    return PREPARE_UNRECOGNIZED_STATEMENT;
}

void execute_statement(Statement *statement)
{
    switch (statement->type)
    {
    case STATEMENT_INSERT:
        printf("Do an insert here.\n");
        break;
    case STATEMENT_SELECT:
        printf("Do a select here.\n");
        break;
    }
}