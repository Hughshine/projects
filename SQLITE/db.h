#ifndef DB_H
#define DB_H

#include "common.h"
#include "table.h"

typedef enum
{
    PREPARE_SUCCESS,
    PREPARE_UNRECOGNIZED_STATEMENT,
    PREPARE_SYNTAX_ERROR,
    PREPARE_INSERT_INVALID_LENGTH,
    PREPARE_INSERT_INVALID_ID
} PrepareResult;

typedef enum {STATEMENT_INSERT, STATEMENT_SELECT} StatementType;

typedef struct {
    StatementType type;
    Row row_to_insert;
}   Statement;

PrepareResult prepare_statement(InputBuffer *input_buffer, Statement *statement);
PrepareResult prepare_insert(InputBuffer* input_buffer, Statement* statement);
typedef enum {
    EXECUTE_SUCCESS,
    EXECUTE_TABLE_FULL
} ExecuteResult;

ExecuteResult execute_statement(Statement* statement, Table* table);
ExecuteResult execute_insert(Statement *statement, Table *table);
ExecuteResult execute_select(Statement *statement, Table *table);

#endif