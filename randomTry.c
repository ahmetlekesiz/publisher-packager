//
// Created by kripton on 20.01.2021.
//

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <math.h>

unsigned int
randomNumber(unsigned int min, unsigned int max)
{
    double x= (double)rand()/RAND_MAX;

    return (max - min +1)*x+ min;
}

int main() {

    for (int i = 0; i <100 ; ++i) {
        int randomBufferIndex = randomNumber(1, 50);
        printf("%d\n", randomBufferIndex);
    }
}