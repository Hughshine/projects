#ifndef TABLE_H
#define TABLE_H

#include "pch.h"

#define COLUMN_USERNAME_SIZE 32
#define COLUMN_EMAIL_SIZE 255
#define size_of_attribute(Struct, Attribute) sizeof(((Struct *)0)->Attribute)

extern const uint32_t ID_SIZE;
extern const uint32_t USERNAME_SIZE;
extern const uint32_t EMAIL_SIZE;
extern const uint32_t ID_OFFSET;
extern const uint32_t USERNAME_OFFSET;
extern const uint32_t EMAIL_OFFSET;
extern const uint32_t ROW_SIZE;

typedef struct
{
    uint32_t id;
    char username[COLUMN_USERNAME_SIZE + 1];
    char email[COLUMN_EMAIL_SIZE + 1];
} Row;

#define TABLE_MAX_PAGES 100

extern const uint32_t PAGE_SIZE;
// extern const uint32_t ROWS_PER_PAGE;
// extern const uint32_t TABLE_MAX_ROWS;

typedef struct {
    int fd; // file descriptor
    uint32_t file_length;
    uint32_t num_pages;
    void* pages[TABLE_MAX_PAGES];
} Pager;

typedef struct {
    // uint32_t num_rows;
    Pager* pager;
    uint32_t root_page_num;
} Table;

typedef struct {
    Table* table; // cursor与表是绑定的
    // uint32_t row_num;
    uint32_t page_num;
    uint32_t cell_num;
    bool end_of_table; // 是否指向表之外
} Cursor;

Pager* pager_open(char* filename);
void pager_flush(Pager *pager, uint32_t page_num);
void *get_page(Pager *pager, uint32_t row_num);
Table* db_open(char *filename);
void db_close(Table *table);
void free_table(Table *table);

// Cursor creation functions
Cursor* table_start(Table* table);
Cursor* table_end(Table* table);
void* cursor_value(Cursor* cursor);
void cursor_advance(Cursor* cursor);

void* row_slot(Table* table, uint32_t row_num);
void serialize_row(Row* source, void* dest);
void deserialize_row(void* source, Row* dest);
void print_row(Row* row);

// node
typedef enum {
    NODE_INTERNAL, NODE_LEAF
} NodeType;
/**
 * Common Node Header Layout
 * 1. node type
 * 2. is root
 * 3. parent pointer
 */
extern const uint32_t NODE_TYPE_SIZE;
extern const uint32_t NODE_TYPE_OFFSET;
extern const uint32_t IS_ROOT_SIZE;
extern const uint32_t IS_ROOT_OFFSET;
extern const uint32_t PARENT_POINTER_SIZE;
extern const uint32_t PARENT_POINTER_OFFSET;
extern const uint32_t COMMON_NODE_HEADER_SIZE;

/**
 * Leaf Node Header Format
 * 4. num cells
 */
extern const uint32_t LEAF_NODE_NUM_CELLS_SIZE;
extern const uint32_t LEAF_NODE_NUM_CELLS_OFFSET;
extern const uint32_t LEAF_NODE_HEADER_SIZE;

/**
 * Leaf Node Body Layout
 * - 这些属于metadata，不存在普通的树节点中。用于控制增删查。
 * - 不过暂时不知道存在哪里。
 * 5. leaf node key size
 * 6. leaf node value size
 * 7. leaf node cell size
 * 8. leaf node space for cells
 * 9. leaf node max cells
 */
extern const uint32_t ROW_SIZE;
extern const uint32_t PAGE_SIZE;

extern const uint32_t LEAF_NODE_KEY_SIZE;
extern const uint32_t LEAF_NODE_KEY_OFFSET;
extern const uint32_t LEAF_NODE_VALUE_SIZE;
extern const uint32_t LEAF_NODE_VALUE_OFFSET;
extern const uint32_t LEAF_NODE_CELL_SIZE;
extern const uint32_t LEAF_NODE_SPACE_FOR_CELLS;
extern const uint32_t LEAF_NODE_MAX_CELLS;

uint32_t* leaf_node_num_cells(void* node);
void* leaf_node_cell(void* node, uint32_t cell_num);
uint32_t* leaf_node_key(void* node, uint32_t cell_num);
void* leaf_node_value(void* node, uint32_t cell_num);
void initialize_leaf_NODE(void* node);
void leaf_node_insert(Cursor* cursor, uint32_t key, Row* value);
#endif