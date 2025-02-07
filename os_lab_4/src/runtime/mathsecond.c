#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include "mathsecond.h"

float Pi(int K) {
    float result = 1;
    for (int i = 1; i < K; i++) {
        result *= (4*i*i) / (4*i*i - 1.0);
    }
    return result * 2;
}


int PrimeCount(int A, int B) {
    if (B < 2 || A > B) {
        return 0;  // Нет простых чисел в заданном диапазоне
    }
    // Создаем массив для решета Эратосфена и инициализируем его
    bool *isPrime = (bool *)malloc((B + 1) * sizeof(bool));
    for (int i = 0; i <= B; i++) {
        isPrime[i] = true;
    }
    // Исключаем из решета числа 0 и 1
    isPrime[0] = isPrime[1] = false;

    // Применяем решето Эратосфена
    for (int i = 2; i * i <= B; i++) {
        if (isPrime[i]) {
            for (int j = i * i; j <= B; j += i) {
                isPrime[j] = false;
            }
        }
    }
    // Подсчитываем количество простых чисел в заданном диапазоне
    int count = 0;
    for (int i = A; i <= B; i++) {
        if (isPrime[i]) {
            count++;
        }
    }
    // Освобождаем память, выделенную для решета
    free(isPrime);

    return count;
}