#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "mathfirst.h"

float Pi(int K) {
    float result = 0;
    for (size_t i = 0; i < K; i++) {
        result += pow(-1, i) / (2*i + 1);
    }
    return result * 4;
}


int PrimeCount(int A, int B)  {
    int count = 0;
    for (int i = A; i <= B; i++) {
        int check = 1;
        if (i <= 1) {
            check = 0; // Число не является простым
        }
        for (int j = 2; j < i; j++) {
            if (i % j == 0) {
                check = 0; // Число не является простым
                break;
            }
        }
        if (check == 1) {
            count++;
        }
    }
    return count;
}