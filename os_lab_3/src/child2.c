#include <sys/types.h>
#include <sys/mman.h>
#include <stdio.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h> 	// pid_t declaration is here
#include <unistd.h> 	// fork() and getpid() declarations are here
#include <pthread.h>
#include <string.h>
#include "semaphore.h"

#define STRING_SIZE 100

void remove_spaces(char* str);


int main(int argc, char* argv[]) {
    // write(STDOUT_FILENO, "2 дочерний процесс запущен!\n", 51 * sizeof(char));
	char mmapped_file_name[STRING_SIZE];
    strcpy(mmapped_file_name, argv[1]);

    char semaphore_child2_name[STRING_SIZE];
    char semaphore_parent_name[STRING_SIZE];
    strcpy(semaphore_child2_name, argv[2]); // семафор для child1
    strcpy(semaphore_parent_name, argv[3]); // семафор для child2

    int mmapped_file_descriptor = shm_open(mmapped_file_name, O_RDWR, 0777);
    ftruncate(mmapped_file_descriptor, STRING_SIZE);
    char* mmapped_file_pointer = mmap(NULL, STRING_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, mmapped_file_descriptor, 0);

    sem_t* semaphore_child2 = sem_open(semaphore_child2_name, 0);
    sem_t* semaphore_parent = sem_open(semaphore_parent_name, 0);
    char string[STRING_SIZE];
    // write(STDOUT_FILENO, "Я около семафора в 2 процессе!\n", 55 * sizeof(char));
    sem_wait(semaphore_child2);

    remove_spaces(mmapped_file_pointer);

    munmap(mmapped_file_pointer, 0);
    sem_post(semaphore_parent);
    sem_close(semaphore_child2);
    sem_close(semaphore_parent);
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
