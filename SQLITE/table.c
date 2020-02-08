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

const uint32_t NODE_TYPE_SIZE = sizeof(uint8_t);
const uint32_t NODE_TYPE_OFFSET = 0;
const uint32_t IS_ROOT_SIZE = sizeof(uint8_t);
const uint32_t IS_ROOT_OFFSET = NODE_TYPE_SIZE;
const uint32_t PARENT_POINTER_SIZE = sizeof(uint32_t);
const uint32_t PARENT_POINTER_OFFSET = IS_ROOT_OFFSET + IS_ROOT_SIZE;
const uint32_t COMMON_NODE_HEADER_SIZE = 
    NODE_TYPE_SIZE + IS_ROOT_SIZE + PARENT_POINTER_SIZE;

const uint32_t LEAF_NODE_NUM_CELLS_SIZE = sizeof(uint32_t);
const uint32_t LEAF_NODE_NUM_CELLS_OFFSET = COMMON_NODE_HEADER_SIZE;
const uint32_t LEAF_NODE_HEADER_SIZE =
    COMMON_NODE_HEADER_SIZE + LEAF_NODE_NUM_CELLS_SIZE;

const uint32_t LEAF_NODE_KEY_SIZE = sizeof(uint32_t);
const uint32_t LEAF_NODE_KEY_OFFSET = 0;
const uint32_t LEAF_NODE_VALUE_SIZE = ROW_SIZE;
const uint32_t LEAF_NODE_VALUE_OFFSET =
    LEAF_NODE_KEY_OFFSET + LEAF_NODE_KEY_SIZE;
const uint32_t LEAF_NODE_CELL_SIZE = LEAF_NODE_KEY_SIZE + LEAF_NODE_VALUE_SIZE;
const uint32_t LEAF_NODE_SPACE_FOR_CELLS = PAGE_SIZE - LEAF_NODE_HEADER_SIZE;
const uint32_t LEAF_NODE_MAX_CELLS =
    LEAF_NODE_SPACE_FOR_CELLS / LEAF_NODE_CELL_SIZE;

uint32_t* leaf_node_num_cells(void* node) {
    return node + LEAF_NODE_NUM_CELLS_OFFSET;
}

void* leaf_node_cell(void* node, uint32_t cell_num) {
    return node + LEAF_NODE_HEADER_SIZE + cell_num * LEAF_NODE_CELL_SIZE; 
}

uint32_t* leaf_node_key(void* node, uint32_t cell_num) {
    return leaf_node_cell(node, cell_num);
}

void* leaf_node_value(void* node, uint32_t cell_num) {
    return leaf_node_cell(node, cell_num) + LEAF_NODE_KEY_SIZE;
}

void initialize_leaf_node(void* node) {
    *leaf_node_num_cells(node) = 0;
}

void leaf_node_insert(Cursor *cursor, uint32_t key, Row *value) {
    // 如何知道cursor的指向第几个cell呢？
    void* node = get_page(cursor->table->pager, cursor->page_num);
    uint32_t num_cells = *leaf_node_num_cells(node);
    // printf("Num Cells: %d\n", num_cells);
    if(num_cells >= LEAF_NODE_MAX_CELLS) {
        printf("Need to implement splitting a leaf node.\n");
        exit(EXIT_FAILURE);
    }

    if(cursor->cell_num < num_cells) {
        for (uint32_t i=num_cells; i>cursor->cell_num; i--) {
            memcpy(leaf_node_cell(node, i), leaf_node_cell(node, i-1), LEAF_NODE_CELL_SIZE);
        }
    }
    
    *(leaf_node_num_cells(node)) += 1;
    *(leaf_node_key(node, cursor->cell_num)) = key;
    serialize_row(value, leaf_node_value(node, cursor->cell_num));

    // for(int i=0;i<num_cells+1;i++) {
    //     printf("%d", *leaf_node_key(node, i));
    // }
}

Table* db_open(char* filename) { 
    Pager* pager = pager_open(filename);
    Table* table = malloc(sizeof(Table));
    table->pager = pager;
    table->root_page_num = 0;
    if (pager->num_pages == 0) {
        // 是一个新的file，将0号page初始化为root
        void* root_node = get_page(pager, 0);
        initialize_leaf_node(root_node);
    }
    return table;
}


void db_close(Table* table) { 
    Pager* pager = table->pager;

    for (uint32_t i = 0; i < pager->num_pages; i++) {
        if(pager->pages[i] == NULL) continue; // 内存中可能没有取过这一页
        pager_flush(pager, i); 
        free(pager->pages[i]);
        pager->pages[i] = NULL;
        printf("Flush page %d successed.\n", i);
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
    pager->num_pages = file_length / PAGE_SIZE;
    
    if(file_length % PAGE_SIZE != 0) {
        printf("Db file is not a whole number of pages. Corrupt file.\n");
        exit(EXIT_FAILURE);
    }

    for (uint32_t i = 0; i < TABLE_MAX_PAGES; i++) {
        pager->pages[i] = NULL;
    }

    return pager;
}

void pager_flush(Pager* pager, uint32_t page_num) {
    if (pager->pages[page_num] == NULL) {
        printf("Error: type to flush a NULL page\n");
        exit(EXIT_FAILURE);
    }

    // 必须按序增长磁盘文件的大小
    off_t file_offset = lseek(pager->fd, page_num*PAGE_SIZE, SEEK_SET);

    if (file_offset == -1) {
        printf("Error seeking:%d\n", errno);
        exit(EXIT_FAILURE);
    }

    ssize_t bytes_written = write(pager->fd, pager->pages[page_num], PAGE_SIZE);

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
        if(page_num >= pager->num_pages) {
            pager->num_pages = page_num + 1;
        }
    }
    return pager->pages[page_num];
}

Cursor* table_start(Table *table) {
    Cursor *cursor = malloc(sizeof(Cursor));
    cursor->table = table;
    // cursor->row_num = 0;
    // cursor->end_of_table = (table->num_rows == 0);
    cursor->page_num = table->root_page_num;
    cursor->cell_num = 0;

    void* root_node = get_page(table->pager, table->root_page_num);
    uint32_t num_cells = *leaf_node_num_cells(root_node);
    
    cursor->end_of_table = (num_cells == 0);
    return cursor;
}

Cursor* table_end(Table *table) {
    Cursor *cursor = malloc(sizeof(Cursor));
    cursor->table = table;
    // cursor->row_num = table->num_rows;
    cursor->page_num = table->root_page_num;
    void *root_node = get_page(table->pager, table->root_page_num);
    uint32_t num_cells = *leaf_node_num_cells(root_node);
    cursor->cell_num = num_cells;
    cursor->end_of_table = true; // 应该改个名字，叫end_of_node

    return cursor;
}

/**
 * Cursor的值是void*————就是指向的行的地址嘛。
 */ 
void* cursor_value(Cursor* cursor) {
    // uint32_t row_num = cursor->row_num;
    // uint32_t page_num = row_num / ROWS_PER_PAGE;
    uint32_t page_num = cursor->page_num;
    void* page = get_page(cursor->table->pager, page_num); // 不存在，则去磁盘上读
    // uint32_t row_offset = row_num % ROWS_PER_PAGE;
    // uint32_t byte_offset = row_offset * ROW_SIZE;
    return leaf_node_value(page, cursor->cell_num);
}

void cursor_advance(Cursor* cursor) { 
    uint32_t page_num = cursor->page_num;
    void* node = get_page(cursor->table->pager, page_num);
    cursor->cell_num += 1;
    if(cursor->cell_num >= (*leaf_node_num_cells(node))) {
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

