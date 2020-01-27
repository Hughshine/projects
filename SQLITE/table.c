#include "table.h"

const uint32_t ID_SIZE = size_of_attribute(Row, id);
const uint32_t USERNAME_SIZE = size_of_attribute(Row, username);
const uint32_t EMAIL_SIZE = size_of_attribute(Row, email);
const uint32_t ID_OFFSET = 0;
const uint32_t USERNAME_OFFSET = ID_OFFSET + ID_SIZE;
const uint32_t EMAIL_OFFSET = USERNAME_OFFSET + USERNAME_SIZE;
const uint32_t ROW_SIZE = ID_SIZE + USERNAME_SIZE + EMAIL_SIZE;

const uint32_t PAGE_SIZE = 4096;
const uint32_t ROWS_PER_PAGE = PAGE_SIZE / ROW_SIZE;
const uint32_t TABLE_MAX_ROWS = ROWS_PER_PAGE * TABLE_MAX_PAGES;

Table *new_table() { 
    Table* table = malloc(sizeof(Table));
    table->num_rows = 0;
    for (uint32_t i = 0; i < TABLE_MAX_PAGES; i++) {
        table->pages[i] = NULL; // 在需要的是否才分配
    }
    return table;
}

void free_table(Table *table) {
    for(int i = 0; table->pages[i]; i++) {
        free(table->pages[i]);
    }
    free(table);
}

// 获取该table，第n个row所在的地址. 如果尚未分配，则分配。
// 问题：分配与取出规则不太匹配。如此分配，可以进行不连续的页分配。free时可能释放不掉。
// 此时的table，比较像一级页表。
void* row_slot(Table* table, uint32_t row_num) { 
    uint32_t page_num = row_num / ROWS_PER_PAGE;
    void* page = table->pages[page_num];
    if(page == NULL) {
        page = table->pages[page_num] = malloc(PAGE_SIZE);
    }
    uint32_t row_offset = row_num % ROWS_PER_PAGE;
    uint32_t byte_offset = row_offset * ROW_SIZE;
    return page + byte_offset;
}

// 并不清楚为什么需要这两个函数，感觉就是一个完整的复制
void serialize_row(Row *source, void *dest) {
    memcpy(dest + ID_OFFSET, &(source->id), ID_SIZE);
    memcpy(dest + USERNAME_OFFSET, &(source->username), USERNAME_SIZE);
    memcpy(dest + EMAIL_OFFSET, &(source->email), EMAIL_SIZE);
}

void deserialize_row(void* source, Row* dest) {
    memcpy(&(dest->id), (source + ID_OFFSET), ID_SIZE);
    memcpy(&(dest->username), (source + USERNAME_OFFSET), USERNAME_SIZE);
    memcpy(&(dest->email), (source + EMAIL_OFFSET), EMAIL_SIZE);
}

void print_row(Row *row) {
    printf("(%d, %s, %s)\n", row->id, row->username, row->email);
}
