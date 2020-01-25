# [Stephen Brennan's LSH](https://brennan.io/2015/01/16/write-a-shell-in-c/)

> implemented and recorded by hughshine

## Features TODO

* Only whitespace separating arguments, no quoting or backslash escaping.
* No piping or redirection.
* Few standard builtins.
* No globbing.

## Problems encountered

### [`conflicting types for xxx`](https://stackoverflow.com/questions/1549631/getting-conflicting-types-for-function-in-c-why/31892393)

实际就是函数未声明便使用的警告。为什么不报“function not declared before called” error?

1. 在c89/c90时代，函数的返回值和列表会被deduce，允许直接使用未声明函数。此时没有 not declared before called error.
2. c99之后，不再允许deduce返回类型，但依然可以deduce参数列表（`()` 与 `(void)` 的差别）。

> The original C language (C89/90) did not have "function not declared before called" error. It was perfectly legal to call undeclared functions in C. Instead of issuing an error the language was required to analyze the call and "deduce" the implicit declaration for the function. That's how the language was originally designed. So, yes in C89/90 you could legally call an undeclared function, as long as the "deduced" function declaration was compatible with the actual one. – AnT 

### [`implicit declaration of function free is invalid in c99`](https://stackoverflow.com/questions/19401658/implicit-declaration-of-function-free-is-invalid-in-c99)

solution: add `#include <stdlib.h>`

c 中没有built-in functions，所有函数都需要有函数声明。若使用c标准中的函数，则需要引入对应头文件。

> You get that warning because you're calling a function without first declaring it, so the compiler doesn't know about the function.
> 
> All functions need to be declared before being called, there are no "built-in" functions in C.
> 
> It's true that free() is a function defined in the standard, but it's still not built-in, you must have a prototype for it.

### 小问题

1. `strtok(char* string, char* delim)`
   * 在 `string.h` 中
   * 返回的指针直接指向原串中对应位置，并将deliminator都改成了'\0'
   * 连续的delim不会返回空串
   * 第一次传入原串，之后传入NULL

2. `perror(char* s)`
   perror(s) 用来将上一个函数发生错误的原因输出到标准设备(stderr)。参数 s 所指的字符串会先打印出，后面再加上错误原因字符串。此错误原因依照全局变量errno的值来决定要输出的字符串。一些函数出错时（尤其是系统函数），errno会被修正。

3. `exec`系列函数差别：从环境变量查找（有无p）、参数传递形式等。

4. [`waitpid()`](https://blog.csdn.net/Roland_Sun/article/details/32084825)
   * 阻塞父进程。等待对应子进程
   * 有特殊的pid参数
   * status返回子进程退出原因
    * > 一系列宏被用于判断状态的真假
   * options参数控制函数行为. 参数可以用`|`连接使用

5. 借助函数数组简化选择过程。

```c
char* builtin_str[] = { "cd", "help", "exit"};

int (*builtin_func[]) (char**) = {
    &lsh_cd,
    &lsh_help,
    &lsh_exit
};
```

6. `man` command is not builtin