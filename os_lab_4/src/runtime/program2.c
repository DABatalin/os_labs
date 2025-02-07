#include <stdio.h>
#include <stdlib.h>
#include <dlfcn.h>
#include "mathsecond.h"

int main() {
	void *library_handler;
	float (*Pi)(int K);
	int (*PrimeCount)(int A, int B);

	library_handler = dlopen("../linking/libMathFirst.so", RTLD_LAZY);
	if (!library_handler){
		fprintf(stderr, "dlopen() error: %s\n", dlerror());
		exit(1);
	};
	
	int command;
	int contract = 0;
	do {
        // Ввод команды
        printf("Введите команду (0, 1, 2)(-1 для завершения выполнения): ");
        scanf("%d", &command);
        
        switch (command) {
            case 0:
                if (contract == 0) {
					contract = 1;
					dlclose(library_handler);
					library_handler = dlopen("libMathsecond.so", RTLD_LAZY);
					if (!library_handler){
						fprintf(stderr, "dlopen() error: %s\n", dlerror());
						exit(1);
					};
				} else {
					contract = 0;
					dlclose(library_handler);
					library_handler = dlopen("../linking/libMathFirst.so", RTLD_LAZY);
					if (!library_handler){
						fprintf(stderr, "dlopen() error: %s\n", dlerror());
						exit(1);
					};
				}
                printf("Реализация переключения контрактов\n");
                break;

            case 1: {
				int iterations = 1000;
                printf("Введите количество итераций вычисления числа Пи 1: ");
                scanf("%d", &iterations);

				Pi = dlsym(library_handler, "Pi");
				float pi = (*Pi)(iterations);

                printf("Pi: %f\n", pi);
                break;
            }

            case 2: {
                int arg1, arg2;
                printf("Введите отрезок на котором нужно посчитать простые числа: ");
                scanf("%d %d", &arg1, &arg2);

                
            	PrimeCount = dlsym(library_handler, "PrimeCount");
				int primeResult = (*PrimeCount)(arg1, arg2);
                printf("Простых чисел: %d\n", primeResult);
                break;
            }
			case -1: break;

            default:
                printf("Некорректная команда\n");
                break;
        }

    } while (command != -1); 
}