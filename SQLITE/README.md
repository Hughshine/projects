# sqlite

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
