#include <stdio.h>
#include <stdlib.h>
#include "mathfirst.h"

int main() {

    int command;
	do {
        // Ввод команды
        printf("Введите команду (1, 2)(-1 для завершения выполнения): ");
        scanf("%d", &command);
        
        switch (command) {
            case 1: {
				int iterations = 1000;
                printf("Введите количество итераций вычисления числа Пи 1: ");
                scanf("%d", &iterations);

				float pi = Pi(iterations);
                printf("Pi: %f\n", pi);
                break;
            }

            case 2: {
                int arg1, arg2;
                printf("Введите отрезок на котором нужно посчитать простые числа: ");
                scanf("%d %d", &arg1, &arg2);

				int count = PrimeCount(arg1, arg2);
                printf("Простых чисел: %d\n", count);
                break;
            }
			case -1: break;

            default:
                printf("Некорректная команда\n");
                break;
        }

    } while (command != -1); 
}