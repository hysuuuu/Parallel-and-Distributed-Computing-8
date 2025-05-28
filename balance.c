#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <stdbool.h>
#include <unistd.h>
#include <string.h>

#define LMIN 10     // load limit
#define LMAX 1000   
#define DMIN 100    // time
#define DMAX 1000   
#define MAX_TIME 1000000

#define EPSILON 2

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

// transfer surplus load units to raise neighboring processors' load units to average
bool balance(Processor *procs, int curr, int left, int right) {
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

// check whether processor and its neighbors satisfy (max - min <= epsilon)
bool is_local_balanced(Processor *procs, int curr, int left, int right, int epsilon)
{
    int C = procs[curr].load;
    int L = procs[left].load;
    int R = procs[right].load;

    int max = C;
    if (L > max)
        max = L;
    if (R > max)
        max = R;

    int min = C;
    if (L < min)
        min = L;
    if (R < min)
        min = R;

    return (max - min) <= epsilon;
}

// check whether the whole system is balanced (every processor is local balanced)
bool is_system_balanced(Processor *procs, int K, int epsilon)
{
    for (int i = 0; i < K; i++)
    {
        int L = (i - 1 + K) % K;
        int R = (i + 1) % K;
        if (!is_local_balanced(procs, i, L, R, epsilon))
        {
            return false;
        }
    }
    return true;
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <num_processors = {5, 10, 100}>\n", argv[0]);
        return 1;
    }
    int K = atoi(argv[1]);  

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
    bool seen[K];
    int visited = 0;
    memset(seen, 0, K * sizeof(bool));
    bool moved_this_round = false;
    bool system_balanced = false;
    
    // execute until reaching MAX_TIME or steady   
    printf("\nStart balancing...\n");
    while (1) {
        if (heap.size == 0 || heap.data[0].time >= MAX_TIME) {
            printf("Reached MAX_TIME\n");
            curr_time = MAX_TIME;
            system_balanced = is_system_balanced(procs, K, EPSILON);
            break;
        }
        HeapNode node = heap_pop(&heap);  
        int curr = node.id;
        int left = (curr - 1 + K) % K;
        int right = (curr + 1) % K;   
        curr_time = node.time;       
        
        if (!seen[curr]) {         
            seen[curr] = true;
            visited += 1;
        }

        bool moved = balance(procs, curr, left, right);  
        if (moved) {                
            moved_this_round = true;
        }

        // if all processors were visited, check if the system is steady
        if (visited == K) {
            // no transfer uccorred, the system is steady
            if (!moved_this_round) {
                printf("System is steady – no further transfers possible.\n");
                // check if the system is balanced
                system_balanced = is_system_balanced(procs, K, EPSILON);
                break;             
            }
            // if transfer still occurred, starts a new round
            memset(seen, 0, K * sizeof(bool));
            visited = 0;
            moved_this_round = false;

            seen[curr] = true;
            visited = 1;
        }

        procs[curr].next_time = curr_time + urand(DMIN, DMAX);
        heap_push(&heap, procs[curr].next_time, curr);
    }

    printf("System Balanced (ε = %d): %s.\n", EPSILON, system_balanced ? "YES" : "NO");
    printf("Finished at time: %d, Processor load:\n", curr_time);
    for (int i = 0; i < K; i++) {        
        printf("%d ", procs[i].load);        
    }
    printf("\n");
    free(procs);
    free(heap.data);
    return 0;    
}