#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <stdbool.h>
#include <unistd.h>

#define MAX_K 100
#define LMIN 10     // load limit
#define LMAX 1000   
#define DMIN 100    // distribution 
#define DMAX 1000   
#define MAX_CYCLES 1000000

//min heap to get the order of processor
typedef struct {
    int time;
    int id;
} HeapNode;

typedef struct {
    HeapNode *data;
    int size, capacity;
} MinHeap;

typedef struct {
    int next_time;
    int load;
    int id;
} Processor;

void heap_push(MinHeap *heap, int time, int id) {
    if (heap->size >= heap->capacity) {
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

// only give units if it is possible give to acheive balance equal load units among three processors
// the processor cannot take load units from a neighbor.
bool strict_balance(Processor *procs, int curr, int left, int right) {
    int C = procs[curr].load;
    int L = procs[left].load;
    int R = procs[right].load;
    
    int average_load = (C + L + R) / 3;  
    int left_need = (average_load > L) ? (average_load - L) : 0;
    int right_need = (average_load > R) ? (average_load - R) : 0;

    if ((C > average_load) && (C - average_load) == (left_need + right_need)) {
        procs[curr].load = average_load;
        procs[left].load = average_load;
        procs[right].load = average_load;  
        return true;
    } 
    return false;
}

// give out extra load units, do not need to be equal
bool relaxed_balance(Processor *procs, int curr, int left, int right) {
    int C = procs[curr].load;
    int L = procs[left].load;
    int R = procs[right].load;

    int average_load = (C + L + R) / 3;  
    int left_need = (average_load > L) ? (average_load - L) : 0;
    int right_need = (average_load > R) ? (average_load - R) : 0;

    int surplus = C - average_load;
    if (surplus <= 0) {
        return false;  
    }

    int give_left = (surplus >= left_need) ? left_need : surplus;
    surplus -= give_left;
    int give_right = (surplus >= right_need) ? right_need : surplus;
    if (give_left == 0 && give_right == 0) {
        return false;
    }
    procs[curr].load  -= (give_left + give_right);
    procs[left].load  += give_left;
    procs[right].load += give_right;
    return true; 
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <num_processors>\n", argv[0]);
        return 1;
    }
    int K = atoi(argv[1]);  
    if (K != 5 && K != 10 && K != 100) {
        fprintf(stderr, "Error: num_processors must be one of {5, 10, 100}.\n");
        return 1;
    }

    srand((unsigned)(time(NULL) ^ getpid()));

    // initialize processors
    Processor *procs = malloc(K * sizeof * procs);
    MinHeap heap;
    heap.data = malloc(K * sizeof * heap.data);
    heap.capacity = K;
    heap.size = 0; 
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

    int curr_time = 0;
    int cycles = 0;
    int stable_cycles = 0;

    // execute until reaching MAX_CYCLES or balanced (stable_cycles > 1000 cycles)    
    printf("\nStart balancing...\n");
    while (cycles < MAX_CYCLES && stable_cycles < 1000) {
        HeapNode node = heap_pop(&heap);  
        int curr = node.id;
        int left = (curr - 1 + K) % K;
        int right = (curr + 1) % K;
        curr_time = node.time;  
        
        if (strict_balance(procs, curr, left, right)) {
            stable_cycles = 0;
        } else {
            stable_cycles += 1;
        }
        // if (relaxed_balance(procs, curr, left, right)) {
        //     stable_cycles = 0;
        // } else {
        //     stable_cycles += 1;
        // }

        procs[curr].next_time = curr_time + urand(DMIN, DMAX);
        heap_push(&heap, procs[curr].next_time, curr);
        cycles += 1;
    }

    printf("Finished at %d time, ran %d cycles, Processor load:\n", curr_time, cycles);
    for (int i = 0; i < K; i++) {        
        printf("%d ", procs[i].load);        
    }
    printf("\n");
    free(procs);
    free(heap.data);
    return 0;    
}