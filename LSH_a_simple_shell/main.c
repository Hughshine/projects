#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
// #include <sys/wait.h>
#define LSH_RL_BUFSIZE 1024
#define LSH_TOK_BUFSIZE 64
#define LSH_TOK_DELIM " \t\r\n\a"
 
void lsh_loop(void); // `()` means auto deduce by compiler
char* lsh_read_line(void);
char** lsh_split_line(char *);
int lsh_execute(char**);
int lsh_launch(char**);

int lsh_cd(char** args);
int lsh_help(char** args);
int lsh_exit(char** args);

char* builtin_str[] = { "cd", "help", "exit"};

int (*builtin_func[]) (char**) = {
    &lsh_cd,
    &lsh_help,
    &lsh_exit
};

int lsh_num_builtins() {
    return sizeof(builtin_str) / sizeof(char *);
} 

int main(int argc, char *argv[]) {
    // load configs, if any.
    // run command loop
    lsh_loop();
    // // perform cleanup
    return EXIT_SUCCESS;
}

void lsh_loop(void)
{
    char *line;
    char **args;
    int status;

    do {
        printf("> "); // print a prompt 提示符
        line = lsh_read_line();
        args = lsh_split_line(line);
        status = lsh_execute(args);

        free(line);
        free(args);
    } while (status); // 为1，循环
}

// reallocate the buffer every time exceeded.
char* lsh_read_line(void)
{
    int bufsize = LSH_RL_BUFSIZE;
    int position = 0;
    char * buffer = malloc(sizeof(char) * bufsize);
    int c;

    if(!buffer) {
        fprintf(stderr, "lsh: allocation error\n");
        exit(EXIT_FAILURE);
    }

    while(1) {
        c = getchar(); // getchar() return the ASCII code, therefore store it as a int
        if(c == EOF || c == '\n') {
            buffer[position] = '\0'; // change the string `end`
            return buffer;
        } else {
            buffer[position] = c;
        }
        position ++;

        // if exceeds, reallocate
        if(position >= bufsize)
        {
            bufsize += LSH_RL_BUFSIZE;
            buffer = realloc(buffer, bufsize);
            if(!buffer) {
                fprintf(stderr, "lsh: reallocation error\n");
                exit(EXIT_FAILURE);
            }
        }
    }   
}

/**
 * parse te cmd. regardless of '\' and quoting
 */ 
char** lsh_split_line(char* line)
{
    int bufsize = LSH_TOK_BUFSIZE, position = 0;
    char** tokens = malloc(bufsize * sizeof(char*));
    char* token;

    if (!tokens) {
        fprintf(stderr, "lsh: allocation error\n");
        exit(EXIT_FAILURE);
    }
    token = strtok(line, LSH_TOK_DELIM); // TODO what do strtok() return
    while(token != NULL) {
        tokens[position] = token;
        position++;

        if(position >= bufsize) {
            bufsize += LSH_TOK_BUFSIZE; // bufsize 是指指针数量
            tokens = realloc(tokens, bufsize * sizeof(char *));
            if(!tokens) {
                fprintf(stderr, "lsh: allocation error\n");
                exit(EXIT_FAILURE);
            }
        }

        token = strtok(NULL, LSH_TOK_DELIM); // 继续分割上一次传入的串。遇见连续的分隔符会做忽略。
    }
    tokens[position] = NULL;
    return tokens;
}

// user may call builtins or local files.
int lsh_execute(char ** args) {
    int i;

    if(args[0] == NULL) {
        return 1;
    }

    for(i = 0; i < lsh_num_builtins(); i++) {
        if(strcmp(args[0], builtin_str[i]) == 0) {
            return (*builtin_func[i])(args);
        }
    }
    return lsh_launch(args);
}

// use fork() & exec() to launch local file
int lsh_launch(char** args) {
    pid_t pid, wpid;
    int status;

    pid = fork();
    if(pid == 0)
    {
        if (execvp(args[0], args) == -1) { // 从环境变量里找到符合参数file的文件，并执行。并将第二个作为参数传入
            perror("lsh");
        }
        exit(EXIT_FAILURE);
    } else if (pid < 0) {
        perror("lsh");
    } else {
        // 下面的判断主要是为了，等待子进程真的exit or killed
        do {
            wpid = waitpid(pid, &status, WUNTRACED); // 当指定等待的子进程运行结束，会返回。正常父进程会被阻塞。第三个是options参数，主要有两个。WUNTRACED是，如果子进程进入暂停状态，则马上返回。
        } while(!WIFEXITED(status) && !WIFSIGNALED(status)); // 正常结束？因为一个未捕获的信号而终止？
    }

    return 1;
}

/**
 * bulitin funcs seg
 */

int lsh_cd(char **args) {
    if(args[1] == NULL)
        fprintf(stderr,"lsh: espected argument to \"cd\"\n");
    else {
        if(chdir(args[1]) != 0) {
            perror("lsh");
        }
    }
    return 1;
}


int lsh_help(char** args) {
    int i;
    printf("Stephen Brennan's LSH implemented by hughshine\n");
    printf("Type program names and arguments, and hit enter.\n");
    printf("The following are built in:\n");

    for( i = 0; i < lsh_num_builtins(); i++ )
        printf("\t%s\n", builtin_str[i]);
    
    printf("Use the man command for information on other programs.\n");
    return 1;
}

int lsh_exit(char** args) {
    return 0;
}