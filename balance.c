#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <stdbool.h>

#define K 10
#define LMIN 10     // load limit
#define LMAX 1000   
#define DMIN 100    // distribution 
#define DMAX 1000   
#define MAX_CYCLES 1000000

//min heap to get the earliest processor
typedef struct {
    int time;
    int id;
} HeapNode;

typedef struct {
    HeapNode data[K];
    int size;
} MinHeap;

typedef struct {
    int next_time;
    int load;
    int id;
} Processor;

void heap_push(MinHeap *heap, int time, int id) {
    if (heap->size >= K) {
        fprintf(stderr, "Heap overflow\n");
        return;
    }
    int i = heap->size++;
    while (i > 0) {
        int parent = (i - 1) / 2;
        if (heap->data[parent].time <= time) break;
        heap->data[i] = heap->data[parent];
        i = parent;
    }
    heap->data[i].time = time;
    heap->data[i].id = id;
}

HeapNode heap_pop(MinHeap *heap) {
    if (heap->size <= 0) {
        fprintf(stderr, "Heap underflow\n");
        return (HeapNode){-1, -1};
    }
    HeapNode min  = heap->data[0];
    HeapNode last = heap->data[--heap->size];
    int i = 0;

    while (true) {
        int left  = 2*i + 1;
        int right = 2*i + 2;
        int smallest = i;
        if (left < heap->size && heap->data[left].time < last.time)
            smallest = left;
        if (right < heap->size && heap->data[right].time < heap->data[smallest].time)
            smallest = right;
        if (smallest == i) break;
        heap->data[i] = heap->data[smallest];
        i = smallest;
    }
    heap->data[i] = last;
    return min;
}

// Uniform random integer in [min, max]
int urand(int min, int max) {
    return min + rand() % (max - min + 1);
}

int main() {
    srand(time(NULL));

    // initialize processors    
    Processor procs[K];
    MinHeap heap = {0};
    for (int i = 0; i < K; i++) {
        procs[i].next_time = urand(DMIN, DMAX);
        procs[i].load = urand(LMIN, LMAX); 
        procs[i].id = i;
        heap_push(&heap, procs[i].next_time, i);
    }

    printf("Initial processor load:\n");
    for (int i = 0; i < K; i++) {        
        printf("%d ", procs[i].load);        
    }

    int cycles = 0;
    int stable_cycles = 0;
    // execute until reaching MAX_CYCLES or balanced (stable_cycles > 1000 cycles)
    
    printf("\nStart balancing...\n");
    while (cycles < MAX_CYCLES && stable_cycles < 1000) {
        int curr = heap_pop(&heap).id;
        int left = (curr - 1 + K) % K;
        int right = (curr + 1) % K;

        int C = procs[curr].load;
        int L = procs[left].load;
        int R = procs[right].load;
        
        int average_load = (C + L + R) / 3;  
        int left_need = (average_load > L) ? average_load - L : 0;
        int right_need = (average_load > R) ? average_load - R : 0;

        // if it is possible to balance the load
        if ((C > average_load) && (C - average_load) == (left_need + right_need)) {
            procs[curr].load = average_load;
            procs[left].load = average_load;
            procs[right].load = average_load;  
            stable_cycles = 0;
        } else {
            stable_cycles += 1;
        }
        procs[curr].next_time += urand(DMIN, DMAX);
        heap_push(&heap, procs[curr].next_time, curr);
        cycles += 1;
    }

    printf("Finished at %d cycles, Processor load:\n", cycles);
    for (int i = 0; i < K; i++) {        
        printf("%d ", procs[i].load);        
    }
    printf("\n");
    return 0;    
}