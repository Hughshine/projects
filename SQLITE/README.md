# sqlite

## usage

```
make test
```

## google coding style of C

文件名均小写，可以包含`_`，`-`。没有约定，则使用下划线。并尽可能的使文件名更明确。

类型名，每个首字母均大写。

变量名（包括函数参数）、数据成员名之间一律小写，单词之间用下划线连接。类的成员变量以下划线结尾，结构体的不用。

常量命名，以k开头，大小写混合。

常规函数使用大小写混合, 取值和设值函数则要求与变量名匹配: MyExcitingFunction(), MyExcitingMethod(), my_exciting_member_variable(), set_my_exciting_member_variable(). 

枚举名 与 常量 或 宏 一致。

## 存储

暂使用数组而非树存储（内存数据库）

1. 使用page为单位存储
2. 每个page存尽可能多的row
3. Rows被序列化，以更紧凑的方式表示
4. 只在需要时分配page
5. keep a fixed-size array of pointers to pages(?)

可选择的Table结构：

|               | unsorted array | sorted array | tree                             |
| ------------- | -------------- | ------------ | -------------------------------- |
| pages contain | only data      | only data    | metadata, primary keys, and data |
| rows per page | more           | more         | fewer                            |
| insertion     | O(1)           | O(n)         | O(logn)                          |
| deletion      | O(n)           | O(n)         | O(logn)                          |
| lookup        | O(n)           | O(logn)      | O(logn)                          |

使用树结构，用一定的空间换取了更好的操作效率。

由数组更改为树，要注意的地方：
1. 每个cell以key-value格式存储，id被视为key，但它在value中也存了一遍
2. 


### B树，B+树

Sqlite使用B树存储索引，使用B+树存储表。一般的随机访问，B+树更稳定。

B+树只有叶子节点存储数据。同阶情况，它的每个内部节点的孩子更多。

这里只讲B+树。m阶b+树，每个内部结点最多有m个孩子，最多有m-1个k、最少有ceil(m/2)-1个keys。需要额外指定每个叶子节点的存储的键值对大小（这里与B树不同），确定叶节点何时分裂（分裂叶节点时，同时要增加父节点的key，可能导致递归调整）。当根节点也分裂时，增加一个根节点，此时层数增高。

## notes

1. 分块（独立出虚拟机，前端后端），减少复杂度，允许cache
2. strcmp 系列
```
strcmp() 比较整个是否相同，区分大小写
strncmp() 比较指定长度的字符串，区分大小写
strcasecmp() 不区分大小写的全串比较
# strnatcmp — 使用自然排序算法比较字符串，区分大小写。(php)
```
3. [借助`extern`处理const常量的duplicate symbol问题](https://blog.csdn.net/Angel69Devil/article/details/76557066)
4. **`free()`怎么使用？谁需要被free？多层数组怎么free？**
5. c 是不是不很需要 预编译头文件呀。。先不处理了。
6. [`extern struct S`, `struct S`](https://stackoverflow.com/questions/50557424/extern-struct-forward-declaration)
7. https://stackoverflow.com/questions/3041797/how-to-use-a-defined-struct-from-another-source-file/3041836
8. `strcpy`, `strncpy`, `memcpy`.
   > strncpy会复制到`'\0'`结束，剩余部分填充`'\0'`（一定会写size个字符）。复制字符串就用这个吧。
   > memcpy就是完全复制
   > strcpy以`'\0'`判断结束，要保证有。
9. 序列化：内存中对象的表示，一般含有指针等复杂类型，并且非顺序存放。序列化时，需要将它完整准确存入某个位置，并将指针等没用的东西去掉。
   > 不知道为什么`memcpy(dest + USERNAME_OFFSET, (source->username), USERNAME_SIZE);`  和 `memcpy(dest + USERNAME_OFFSET, &(source->username), USERNAME_SIZE);` 是等价的， (source->username) = &(source->username)..
   > 
   > 原因 10 
10. 数组行为与指针行为的差异。虽然一般认为数组与指针等价，但对两者 取地址 的行为不同.对数组去地址，只更改了类型（维度+1），地址值不改变（可以说，得不到存储数组首地址的地址）。数组与指针可以取地址的层数与它们的维度数相同，过多就会报“不能取右值的地址”。
    ```c
    char str[] = "123";
    // char(* p)[] = &str;
    printf("s1: %p\n", (str));
    printf("&s1: %p\n", &(str));

    char* str2 = str;
    printf("s2: %p\n", (str2));
    printf("&s2: %p\n", &(str2));
    ```
    ```
    s1: 0x7fff589c53ac
    &s1: 0x7fff589c53ac
    s2: 0x7fff589c53ac
    &s2: 0x7fff589c53a0
    ```
11. 不去强制类型转换malloc, calloc, realloc的返回值；并使用`source_t *temp = malloc(sizeof *temp);`动态分配空间————而不是使用类型作为malloc的参数。

## 问题

1. 不会用gdb，bad
2. 出现了奇怪的错误，又不能精确复现了

```
db > .exit
1123db(45727,0x7fff9539a3c0) malloc: *** error for object 0x7feee7c027c8: incorrect checksum for freed object - object was probably modified after being freed.
*** set a breakpoint in malloc_error_break to debug
make: *** [test] Abort trap: 6
```

> 关闭数据库时，把二进制文件重新输出了（可能只输出了一部分），然后报错。
> 
> 原以为是select的问题，最初select后直接.exit会出现问题；又不知为什么不报错了。
> 
> 可能的原因是，我用vscode同时打开了它。。但好像又不是。。啊。。