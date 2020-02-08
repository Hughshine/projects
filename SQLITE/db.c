#include "db.h"

// 对非元指令做预处理，解析为Statement. 可以理解为“虚拟机语言”
PrepareResult prepare_statement(InputBuffer *input_buffer, Statement *statement)
{
    if (strncmp(input_buffer->buffer, "insert", 6) == 0)
    { // strncmp, 比较指定长度的字符串
        return prepare_insert(input_buffer, statement);
    }
    else if (strncmp(input_buffer->buffer, "select", 6) == 0)
    {
        statement->type = STATEMENT_SELECT;
        return PREPARE_SUCCESS;
    }

    return PREPARE_UNRECOGNIZED_STATEMENT;
}

// 限制输入大小不能超过上限
PrepareResult prepare_insert(InputBuffer *input_buffer, Statement *statement) {
    statement->type = STATEMENT_INSERT;

    char* keyword = strtok(input_buffer->buffer, " ");
    char* id_string = strtok(NULL, " ");
    char* username = strtok(NULL, " ");
    char* email = strtok(NULL, " ");

    if(id_string == NULL || username == NULL || email == NULL)
        return PREPARE_SYNTAX_ERROR;
    int id = atoi(id_string);
    if (id <= 0) 
        return PREPARE_INSERT_INVALID_ID;
    
    if(strlen(username) > COLUMN_USERNAME_SIZE
        || strlen(email) > COLUMN_EMAIL_SIZE ) 
        return PREPARE_INSERT_INVALID_LENGTH;

    statement->row_to_insert.id = id;
    strcpy(statement->row_to_insert.username, username);
    strcpy(statement->row_to_insert.email, email);

    return PREPARE_SUCCESS;
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
    void* node = get_page(table->pager, table->root_page_num);
    if((*leaf_node_num_cells(node) >= LEAF_NODE_MAX_CELLS)) {
        return EXECUTE_TABLE_FULL;
    }
    Cursor* cursor = table_end(table);
    Row* row_to_insert = &(statement->row_to_insert); // 指向实际数据
    leaf_node_insert(cursor, row_to_insert->id, row_to_insert);
    free(cursor);
    return EXECUTE_SUCCESS;
}

ExecuteResult execute_select(Statement* statement, Table *table) {
    Row row;
    Cursor* cursor = table_start(table);
    void* node = get_page(table->pager, table->root_page_num);
    for (uint32_t i = 0; i < *leaf_node_num_cells(node); i++) {
        void* value = leaf_node_value(node, i);
        deserialize_row(value, &row);
        print_row(&row);
        cursor_advance(cursor);
    }
    free(cursor);
    return EXECUTE_SUCCESS;
}