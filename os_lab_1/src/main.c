#include "stdio.h"
#include "stdlib.h"
#include "unistd.h"
#include "sys/wait.h"

#define STRING_SIZE 100

int create_process();
void create_pipe(int* pipe_fd);
void to_lower(char* str);
void remove_spaces(char* str);
void read_input(char *buffer, size_t bufferSize);

int main() {
    int pipe_fd[2]; create_pipe(pipe_fd);
    int pipe_fd_children[2]; create_pipe(pipe_fd_children);
    int pipe_fd_final[2]; create_pipe(pipe_fd_final);
    
    pid_t pid = create_process();
    if (pid == 0) // child1
    {
        char parent_string[STRING_SIZE];
        close(pipe_fd[1]);
        close(pipe_fd_children[0]);

        read(pipe_fd[0], &parent_string, STRING_SIZE * sizeof(char));
        to_lower(parent_string);
        write(pipe_fd_children[1], &parent_string, STRING_SIZE * sizeof(char));

        close(pipe_fd[0]);
        close(pipe_fd_children[1]);
    }
    else
    {
        pid_t pid = create_process();
        if (pid == 0) // child2
        {  
            char child_string[STRING_SIZE];
            close(pipe_fd[0]);
            close(pipe_fd[1]);
            close(pipe_fd_children[1]);
            close(pipe_fd_final[0]);

            read(pipe_fd_children[0], &child_string, STRING_SIZE * sizeof(char));
            remove_spaces(child_string);
            write(pipe_fd_final[1], &child_string, STRING_SIZE * sizeof(char));

            close(pipe_fd_children[0]);
        }
        else // parent
        {
            char string[STRING_SIZE];
            close(pipe_fd[0]);
            close(pipe_fd_final[1]);

            read_input(string, STRING_SIZE);
            write(pipe_fd[1], &string, STRING_SIZE * sizeof(char));
            wait(NULL);

            read(pipe_fd_final[0], &string, STRING_SIZE * sizeof(char));
            write(STDOUT_FILENO, &string, STRING_SIZE * sizeof(char));

            close(pipe_fd[1]);
            close(pipe_fd_final[0]);
        }
    }
    return 0;
}

int create_process() {
    pid_t pid = fork();
    if (-1 == pid)
    {
        perror("Error while fork");
    }
    return pid;
}

void create_pipe(int* pipe_fd) {
    if (pipe(pipe_fd) == -1)
    {
        perror("Failed to create pipe");
        exit(-1);
    }
}

void to_lower(char *str) {
    while (*str) {
        if ((*str >= 65) && (*str <= 90))
            *str = *str + 32;
        str += 1;
    }
}

void remove_spaces(char* str) {
    int len = STRING_SIZE;
    if (len <= 1) {
        return;
    }

    char* dest = str;
    *dest++ = str[0];

    int i = 1;
    for (; str[i] != 0 && i < len ; i++) {
        if (!(str[i-1] == ' ' && str[i] == ' '))
        {
            *dest = str[i];
            dest++;
        } 
    }

    for (; i < len; i++) {
        *dest = 0;
        dest++;
    }
}

void read_input(char *buffer, size_t bufferSize) {
    ssize_t bytesRead;
    bytesRead = read(STDIN_FILENO, buffer, bufferSize - 1);

    if (bytesRead == -1) {
        perror("Read error");
        exit(-1);
    }

    for (int i = bytesRead; i < bufferSize; i++) {
        buffer[i] = 0;
    }
}