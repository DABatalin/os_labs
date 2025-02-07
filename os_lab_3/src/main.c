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
#include "sys/wait.h"

#define STRING_SIZE 100

int create_process();
void read_input(char *buffer, size_t bufferSize);


int main() {

    const char* mmapped_file_name;
    mmapped_file_name = "mmaped_file";

    const char* semaphores_names[3];
    semaphores_names[0] = "/semaphoreOne"; // семафор для child1
    semaphores_names[1] = "/semaphoreTwo"; // семафор для child2
    semaphores_names[2] = "/semaphoreThree"; // семафор для parent

    shm_unlink(mmapped_file_name);
    sem_unlink(semaphores_names[0]);
    sem_unlink(semaphores_names[1]);
    sem_unlink(semaphores_names[2]);
    
    int mmapped_file_descriptor;
    char* mmapped_file_pointer;

    mmapped_file_descriptor = shm_open(mmapped_file_name, O_RDWR | O_CREAT, 0777);
    ftruncate(mmapped_file_descriptor, STRING_SIZE);
    mmapped_file_pointer = mmap(NULL, STRING_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, mmapped_file_descriptor, 0);

    sem_t* semaphores[3];
    semaphores[0] = sem_open(semaphores_names[0], O_CREAT, 0777, 0);
    semaphores[1] = sem_open(semaphores_names[1], O_CREAT, 0777, 0);
    semaphores[2] = sem_open(semaphores_names[2], O_CREAT, 0777, 0);


    pid_t pid = create_process();
    if (pid == 0) // child1
    {
		execl("child1", "", mmapped_file_name, semaphores_names[0], semaphores_names[1], NULL);
		perror("exec");
		exit(-3);
    }
    else
    {
        pid_t pid = create_process();
        if (pid == 0) // child2
        {  
            execl("child2", "", mmapped_file_name, semaphores_names[1], semaphores_names[2], NULL);
            perror("exec");
            exit(-3);
        }
        else // parent
        {
            char string[STRING_SIZE];
            read_input(string, STRING_SIZE); // читаем входные данные в string
            for (int i = 0; i < strlen(string); ++i) {
                mmapped_file_pointer[i] = string[i];
            }

            write(STDOUT_FILENO, "Строка считана и выглядит так: ", 57 * sizeof(char));
            write(STDOUT_FILENO, mmapped_file_pointer, 100 * sizeof(char));
            write(STDOUT_FILENO, "\n", 2 *sizeof(char));
            sem_post(semaphores[0]);
            sem_wait(semaphores[2]);
            wait(NULL);
            
            munmap(mmapped_file_pointer, 0);
            shm_unlink(mmapped_file_name);
            for (int i = 0; i < 3; i++) {
                sem_unlink(semaphores_names[i]);
            }
            
            write(STDOUT_FILENO, mmapped_file_pointer, STRING_SIZE * sizeof(char));
        }
    }
    return 0;
}


int create_process() {
    pid_t pid = fork();
    if (-1 == pid)
    {
        perror("Error while fork");
        exit(-2);
    }
    return pid;
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