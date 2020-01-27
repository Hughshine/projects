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

ExecuteResult execute_statement(Statement *statement, Table *table)
{
    switch (statement->type)
    {
    case STATEMENT_INSERT:
        return execute_insert(statement, table);
    case STATEMENT_SELECT:
        return execute_select(statement, table);
    }
}

ExecuteResult execute_insert(Statement* statement, Table *table) {
    if (table->num_rows >= TABLE_MAX_ROWS) {
        return EXECUTE_TABLE_FULL;
    }
    
    Row* row_to_insert = &(statement->row_to_insert);
    serialize_row(row_to_insert, row_slot(table, table->num_rows));
    table->num_rows++;
    
    return EXECUTE_SUCCESS;
}

ExecuteResult execute_select(Statement* statement, Table *table) {
    Row row;
    for (uint32_t i = 0; i < table->num_rows; i++) {
        deserialize_row(row_slot(table, i), &row);
        print_row(&row);
    }
    return EXECUTE_SUCCESS;
}