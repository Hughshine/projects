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

Table* db_open(char* filename) { 
    Pager* pager = pager_open(filename);
    Table* table = malloc(sizeof(Table));
    table->pager = pager;
    table->num_rows = pager->file_length / ROW_SIZE;
    return table;
}

// 将内存中的table的每一行写回磁盘。也即将pager中每个page按序写回磁盘
// 仅根据pager，找不到最后一页
void db_close(Table* table) { 
    // printf("I'm I here?\n");
    Pager* pager = table->pager;
    uint32_t full_page_count = table->num_rows / ROWS_PER_PAGE;

    for (uint32_t i = 0; i < full_page_count; i++) {
        if(pager->pages[i] == NULL) continue; // 内存中可能没有取过这一页
        pager_flush(pager, i, PAGE_SIZE); 
        free(pager->pages[i]);
        pager->pages[i] = NULL;
    }
    
    uint32_t rest_rows_count = table->num_rows % ROWS_PER_PAGE;
    if(rest_rows_count > 0) {
        // 将最后一个有效页的有效行 写回。 
        // 虽然代码没有禁止 随机的页访问，但代码逻辑是从小到大依次访问的。
        uint32_t last_page = full_page_count;
        if (pager->pages[last_page] != NULL) {
            pager_flush(pager, last_page, rest_rows_count * ROW_SIZE);
            free(pager->pages[last_page]);
            pager->pages[last_page] = NULL;
        }
    }
    
    int result = close(pager->fd);
    if (result == -1) {
        printf("Error closing db.\n");
        exit(EXIT_FAILURE);
    }
    // 每个页都应已free
    free(pager);
    free(table);
}

void free_table(Table *table) {
    Pager* pager = table->pager;
    for(int i = 0; pager->pages[i]; i++) {
        free(pager->pages[i]);
    }
    free(pager);
    free(table);
}

Pager *pager_open(char *filename) {
    int fd = open(filename, O_RDWR | O_CREAT, // 读写mood，不存在即create
                            S_IWUSR | S_IRUSR // 用户读 用户写权限
                            );
    if (fd == -1) {
        printf("Cannot open file: %s\n", filename);
        exit(EXIT_FAILURE);
    }

    off_t file_length = lseek(fd, 0, SEEK_END); // TODO

    Pager* pager = malloc(sizeof(Pager));
    pager->fd = fd;
    pager->file_length = file_length;

    for (uint32_t i = 0; i < TABLE_MAX_PAGES; i++) {
        pager->pages[i] = NULL;
    }

    return pager;
}

void pager_flush(Pager* pager, uint32_t page_num, uint32_t size) {
    if (pager->pages[page_num] == NULL) {
        printf("Error: type to flush a NULL page\n");
        exit(EXIT_FAILURE);
    }
    if(size > PAGE_SIZE) size = PAGE_SIZE;

    // 必须按序增长磁盘文件的大小
    off_t file_offset = lseek(pager->fd, page_num*PAGE_SIZE, SEEK_SET);

    if (file_offset == -1) {
        printf("Error seeking:%d\n", errno);
        exit(EXIT_FAILURE);
    }

    ssize_t bytes_written = write(pager->fd, pager->pages[page_num], size);

    if(bytes_written == -1) {
        printf("Error writing:%d\n", errno);
        exit(EXIT_FAILURE);
    }
}
 
// 只当磁盘上存在时，有读取磁盘动作.
void* get_page(Pager* pager, uint32_t page_num) { 
    if (page_num > TABLE_MAX_PAGES) {
        printf("Page num out of bounds. %d", page_num);
        exit(EXIT_FAILURE);
    }

    if(pager->pages[page_num] == NULL) {
        void* page = malloc(sizeof(PAGE_SIZE));

        uint32_t pages_count = pager->file_length / PAGE_SIZE;
        if(pager->file_length % PAGE_SIZE != 0) pages_count++;

        if(page_num < pages_count) { // read from disk, else do nothing.
            lseek(pager->fd, page_num * PAGE_SIZE, SEEK_SET);
            ssize_t bytes_read = read(pager->fd, page, PAGE_SIZE);
            if(bytes_read < 0) {
                printf("Error reading file: %d\n", errno);
                exit(EXIT_FAILURE);
            }
        }

        pager->pages[page_num] = page;
    }
    return pager->pages[page_num];
}

Cursor* table_start(Table *table) {
    Cursor *cursor = malloc(sizeof(Cursor));
    cursor->table = table;
    cursor->row_num = 0;
    cursor->end_of_table = (table->num_rows == 0);
    
    return cursor;
}

Cursor* table_end(Table *table) {
    Cursor *cursor = malloc(sizeof(Cursor));
    cursor->table = table;
    cursor->row_num = table->num_rows;
    cursor->end_of_table = (table->num_rows == 0);

    return cursor;
}

/**
 * Cursor的值是void*————就是指向的行的地址嘛。
 */ 
void* cursor_value(Cursor* cursor) {
    uint32_t row_num = cursor->row_num;
    uint32_t page_num = row_num / ROWS_PER_PAGE;
    void* page = get_page(cursor->table->pager, page_num); // 不存在，则去磁盘上读
    uint32_t row_offset = row_num % ROWS_PER_PAGE;
    uint32_t byte_offset = row_offset * ROW_SIZE;
    return page + byte_offset;
}

void cursor_advance(Cursor* cursor) { 
    cursor->row_num += 1;
    if(cursor->row_num >= cursor->table->num_rows) {
        cursor->end_of_table = true;
    }
}

/*
 * 获取该table，第n个row所在的地址. 如果尚未分配，则分配。
 * 问题：分配与取出规则不太匹配。如此分配，可以进行不连续的页分配。free时可能释放不掉。
 * 此时的table，比较像一级页表。
 * 改使用Cursor抽象，用于分离对表的直接修改。
 */
// void* row_slot(Table* table, uint32_t row_num) { // TODO
//     uint32_t page_num = row_num / ROWS_PER_PAGE;
//     void* page = get_page(table->pager, page_num); // 不存在，则去磁盘上读
//     uint32_t row_offset = row_num % ROWS_PER_PAGE;
//     uint32_t byte_offset = row_offset * ROW_SIZE;
//     return page + byte_offset;
// }

// 指针指向的值，需要被按序放置，是序列化的作用。结构体里，两个string只是两个指针。
void serialize_row(Row *source, void *dest) {
    memcpy(dest + ID_OFFSET, &(source->id), ID_SIZE);
    strncpy(dest + USERNAME_OFFSET, (source->username), USERNAME_SIZE);
    strncpy(dest + EMAIL_OFFSET, (source->email), EMAIL_SIZE);
}

void deserialize_row(void* source, Row* dest) {
    memcpy(&(dest->id), (source + ID_OFFSET), ID_SIZE);
    memcpy(&(dest->username), (source + USERNAME_OFFSET), USERNAME_SIZE);
    memcpy(&(dest->email), (source + EMAIL_OFFSET), EMAIL_SIZE);
}

void print_row(Row *row) {
    printf("(%d, %s, %s)\n", row->id, row->username, row->email);
}
