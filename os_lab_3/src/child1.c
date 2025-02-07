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

void to_lower(char* str);


int main(int argc, char* argv[]) {
    // write(STDOUT_FILENO, "1 дочерний процесс запущен!\n", 51 * sizeof(char));
    char mmapped_file_name[STRING_SIZE];
    strcpy(mmapped_file_name, argv[1]);

    // write(STDOUT_FILENO, "Название mmap файла: ", 35 * sizeof(char));
    // write(STDOUT_FILENO, mmapped_file_name, 100 * sizeof(char));

    char semaphore_child1_name[STRING_SIZE];
    char semaphore_child2_name[STRING_SIZE];
    strcpy(semaphore_child1_name, argv[2]); // семафор для child1
    strcpy(semaphore_child2_name, argv[3]); // семафор для child2

    int mmapped_file_descriptor = shm_open(mmapped_file_name, O_RDWR, 0777);
    ftruncate(mmapped_file_descriptor, STRING_SIZE);
    char* mmapped_file_pointer = mmap(NULL, STRING_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, mmapped_file_descriptor, 0);

    sem_t* semaphore_child1 = sem_open(semaphore_child1_name, 0);
    sem_t* semaphore_child2 = sem_open(semaphore_child2_name, 0);
    char string[STRING_SIZE];
    
    // write(STDOUT_FILENO, "Я около семафора в 1 процессе!\n", 55 * sizeof(char));
    sem_wait(semaphore_child1);

    to_lower(mmapped_file_pointer);

    munmap(mmapped_file_pointer, 0);
    sem_post(semaphore_child2);
    sem_close(semaphore_child1);
    sem_close(semaphore_child2);
}


void to_lower(char *str) {
    while (*str) {
        if ((*str >= 65) && (*str <= 90))
            *str = *str + 32;
        str += 1;
    }
}
