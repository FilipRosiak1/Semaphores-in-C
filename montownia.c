#include <pthread.h> 
#include <stdio.h> 
#include <stdlib.h> 
#include <unistd.h> 
#include <semaphore.h> 
#include <time.h> 
#include <stdbool.h>

#define MAG_SIZE 10 // maksymalna pojemność magazynu

#include <stdio.h>
#include <stdlib.h>

sem_t sem_mag_size;
sem_t sem_max_A;
sem_t sem_A_available;
sem_t sem_max_B;
sem_t sem_B_available;
sem_t sem_MagA;

void debug() {
    int A, B, isA, isB, Mag, MagA;
    sem_getvalue(&sem_max_A, &A);
    sem_getvalue(&sem_max_B, &B);
    sem_getvalue(&sem_A_available, &isA);
    sem_getvalue(&sem_B_available, &isB);
    sem_getvalue(&sem_mag_size, &Mag);
    sem_getvalue(&sem_MagA, &MagA);
    printf("%d %d %d %d %d %d\n", A, B, isA, isB, Mag, MagA); 
}

char *magazyn[MAG_SIZE];
int indeksyA[MAG_SIZE-1];
int indeksyB[MAG_SIZE-1];
int indeksyMag[MAG_SIZE];
int lastfreeIndex = 0;
int kompA = 0;
int kompB = 0;

void magprint() {
    for(int i = 0; i < MAG_SIZE; i++) {
        printf("%s ", magazyn[i]);
    }
    printf("\n");
}

void *producentA(void *arg) {
    int lastA = 0;
    while(1) {
        sleep(0.000001);
        sem_wait(&sem_max_A);
        sem_wait(&sem_mag_size);
        sem_wait(&sem_MagA);
        //sleep(rand()%2 + 1);
        
        int freeforA = indeksyMag[lastfreeIndex%MAG_SIZE];
        lastfreeIndex = (lastfreeIndex+1)%MAG_SIZE;
        indeksyA[lastA] = freeforA;
        magazyn[freeforA] = "A";
        kompA++;
        lastA = (lastA+1)%MAG_SIZE;
        
        printf("ProducentA dostarczył komponent A na miejsce %d, magazyn (A, B): %d, %d\n", freeforA, kompA, kompB);
        magprint();
        sem_post(&sem_A_available);
        sem_post(&sem_MagA);
        
        // printf("ProdA\n");
        // debug();
    }
}

void *producentB(void *arg) {
    int lastB = 0;
    while(1) {
        sleep(0.000001);
        sem_wait(&sem_max_B);
        sem_wait(&sem_mag_size);
        sem_wait(&sem_MagA);
        //sleep(rand()%2 + 1);
        
        int freeforB = indeksyMag[lastfreeIndex%MAG_SIZE];
        lastfreeIndex = (lastfreeIndex+1)%MAG_SIZE;
        indeksyB[lastB] = freeforB;
        magazyn[freeforB] = "B";
        kompB++;
        lastB = (lastB+1)%MAG_SIZE;

        printf("ProducentB dostarczył komponent B na miejsce %d, magazyn (A, B): %d, %d\n", freeforB, kompA, kompB);
        magprint();
        sem_post(&sem_B_available);
        sem_post(&sem_MagA);
        // printf("ProdB\n");
        // debug();
    }
}

void *montownia(void *arg) {
    int lastA = 0;
    int lastB = 0;
    int lastforIndex = 0;
    while(1) {
        sleep(0.000001);
        sem_wait(&sem_A_available);
        sem_wait(&sem_B_available);
        sem_wait(&sem_MagA);
        //sleep(rand()%2 + 1);

        int freeIndexA = indeksyA[lastA%MAG_SIZE];
        lastA = (lastA+1)%MAG_SIZE;
        int freeIndexB = indeksyB[lastB%MAG_SIZE];
        lastB = (lastB+1)%MAG_SIZE;

        indeksyMag[lastforIndex%MAG_SIZE] = freeIndexA;
        lastforIndex = (lastforIndex+1)%MAG_SIZE;
        indeksyMag[lastforIndex%MAG_SIZE] = freeIndexB;
        lastforIndex = (lastforIndex+1)%MAG_SIZE;

        magazyn[freeIndexA] = NULL;
        magazyn[freeIndexB] = NULL;
        kompA--;
        kompB--;
        printf("Montownia złożyła produkt końcowy, użyte komponenty: %d %d, magazyn (A, B): %d, %d\n",freeIndexA, freeIndexB, kompA, kompB);
        magprint();
        sem_post(&sem_max_A);
        sem_post(&sem_max_B);
        sem_post(&sem_mag_size);
        sem_post(&sem_mag_size); 
        sem_post(&sem_MagA);
        // printf("Mont\n");
        // debug();
    }
}

int main() {
    srand(time(NULL));

    for(int i = 0; i < MAG_SIZE; i++) {
        indeksyMag[i] = i;
    }

    sem_init(&sem_max_A, 0, MAG_SIZE-1);
    sem_init(&sem_max_B, 0, MAG_SIZE-1);
    sem_init(&sem_mag_size, 0, MAG_SIZE);
    sem_init(&sem_A_available, 0, 0);
    sem_init(&sem_B_available, 0, 0);
    sem_init(&sem_MagA, 0, 1);

    pthread_t prodA, prodB, mont;
    pthread_create(&prodA, NULL, producentA, NULL);
    pthread_create(&prodB, NULL, producentB, NULL);
    pthread_create(&mont, NULL, montownia, NULL);

    pthread_join(prodA, NULL);
    pthread_join(prodB, NULL);
    pthread_join(mont, NULL);

    return 0;
}