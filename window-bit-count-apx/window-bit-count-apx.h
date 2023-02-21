#ifndef _WINDOW_BIT_COUNT_APX_
#define _WINDOW_BIT_COUNT_APX_

#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <math.h>
#include <inttypes.h>

uint64_t N_MERGES = 0; // keep track of how many bucket merges occur

/*
    TODO: You can add code here.
*/

typedef struct StateApx {
    // struct GroupNode *head; 
    // struct GroupNode *tail;
    struct GroupNode *groups;
    int now;
    int W;
    int k;
    int *buckets;
    int last_group_idx; // index to the last valid group in groups
    // int cur_idx; // global index to global buckets array, increments by k + 1 each time we use it
} StateApx;

typedef struct GroupNode {
    // int bucket_size; // size of buckets
    int* buckets; // k+1 sized circular array of buckets
    int bucket_insert; // index where next bucket will be inserted
    int bucket_evict; // index of next bucket to evict (oldest) if group is full
    int num_buckets; // number of buckets within this group
    // struct GroupNode *next; // pointer to next group
    // struct GroupNode *prev; // pointer to previous group
} GroupNode;

void insertBucket(StateApx* self);
void evictAny(StateApx* self);
int getCount(StateApx* self);
void merge(StateApx* self, int ts, int group_idx);
// int count_buckets(StateApx* self);

// k = 1/eps
// if eps = 0.01 (relative error 1%) then k = 100
// if eps = 0.001 (relative error 0.1%) the k = 1000
uint64_t wnd_bit_count_apx_new(StateApx* self, int wnd_size, int k) {
    assert(wnd_size >= 1);
    assert(k >= 1);

    // store the constants k and window size
    self->W = wnd_size;
    self->k = k;

    // init head and tail
    // self->head = (GroupNode*) malloc(sizeof(GroupNode));
    // self->head->bucket_size = 0;
    // self->head->buckets = NULL;
    // self->head->bucket_insert = 0;
    // self->head->bucket_evict = 0;
    // self->head->num_buckets = 0;
    // self->tail = (GroupNode*) malloc(sizeof(GroupNode));
    // self->tail->bucket_size = 0;
    // self->tail->buckets = NULL;
    // self->tail->bucket_insert = 0;
    // self->tail->bucket_evict = 0;
    // self->tail->num_buckets = 0;
    self->last_group_idx = -1;

    // point head to tail
    // self->head->next = self->tail;
    // self->tail->next = NULL;

    // Initialize all the buckets we could ever dream of
    // printf(" ceil(log2(wnd_size/(k + 1) + 1)) %d\n", wnd_size/(k + 1) + 1);
    // printf("buckets mem size: %d\n", sizeof(int) * (k + 1) * (ceil(log2(wnd_size/(k + 1) + 1)) + 1));
    // printf("%d\n", (ceil(log2(wnd_size/(k + 1) + 1)) + 1));
    int mem1 = (k + 1) * (ceil(log2(wnd_size/(k + 1) + 1)) + 1);
    self->buckets = (int*) calloc(sizeof(int), mem1);
    // printf("buckets mem size: %d\n", mem1);
    // Malloc group node times number of groups
    // printf("groups mem size: %d\n", sizeof(GroupNode) * (ceil(log2(wnd_size/(k + 1) + 1)) + 1));
    int mem2 = (ceil(log2(wnd_size/(k + 1) + 1)) + 1);
    self->groups = (GroupNode*) calloc(sizeof(GroupNode), mem2);
    // printf("groups mem size: %d\n", mem2);
    // printf("SIZE OF BUCKETS MEMORY: %d\n", sizeof(int) * (k + 1) * (ceil(log2(wnd_size/(k + 1) + 1)) +1));
    
    // initialize logical timer
    self->now = 0;

    // initialize global buckets index
    // self->cur_idx = 0;

    // The function should return the total number of bytes allocated on the heap.
    // int mem = sizeof(int) * (k) * ceil(log2(wnd_size/(k))) + 2 * sizeof(struct GroupNode);
    // int firstmem = (sizeof(int) * (k + 1) * (ceil(log2(wnd_size/(k + 1) + 1)) + 1));
    // printf("first half of mem: %d\n", (sizeof(int) * (k + 1) * (ceil(log2(wnd_size/(k + 1) + 1)) + 1)));
    // printf("first half of mem: %d\n", firstmem);
    // printf("second half of mem: %d\n", (sizeof(GroupNode) * (ceil(log2(wnd_size/(k + 1) + 1)) + 1)));
    // printf("full mem: %d\n", sizeof(int) * (k + 1) * (ceil(log2(wnd_size/(k + 1) + 1)) + 1) + sizeof(GroupNode) * (ceil(log2(wnd_size/(k + 1) + 1)) + 1));
    // int mem = sizeof(int) * (k + 1) * (ceil(log2(wnd_size/(k + 1) + 1)) + 1) + sizeof(GroupNode) * (ceil(log2(wnd_size/(k + 1) + 1)) + 1);

    // printf("MEMO USAGE %d\n", mem1 + mem2);
    return mem1 + mem2;
}

void wnd_bit_count_apx_destruct(StateApx* self) {
    // TODO: Fill me.
    // Make sure you free the memory allocated on the heap.
    // struct GroupNode *cur = self->head;
    // int count = 0;
    free(self->buckets);
    free(self->groups);
    // while (cur != NULL) {
    //     struct GroupNode *temp = cur->next;
    //     free(cur->buckets);
    //     free(cur);
    //     cur = temp;
    // }
}

void wnd_bit_count_apx_print(StateApx* self) {
//     printf("APX_PRINT: Printing\n");
//     // This is useful for debugging.
//     struct GroupNode* cur = self->head->next;
//     // fprintf(stdout, "Print Head stats: num buckets is %d and group num is %d\n", self->head->num_buckets, self->head->bucket_size);
//     // fprintf(stdout, "Print Tail stats: num buckets is %d and group num is %d\n", self->tail->num_buckets, self->tail->bucket_size);

//     while (cur != self->tail) {
//         printf("APX_PRINT: Group number: %d\n", cur->bucket_size);
//         printf("APX_PRINT: Number of Buckets: %d\n", cur->num_buckets);
//         if (cur->next == NULL) {
//             return;
//         }
//         cur = cur->next;
//         // printf("APX_PRINT: Done with one iteration\n");
//     }
//     printf("APX_PRINT: Done Printing\n");
}

int wnd_bit_count_apx_next(StateApx* self, bool item) {
    // increment logical clock
    self->now++;
    // printf("NOW: %d\n", self->now);
    // printf("now: %d\n", self->now);


    // check if any buckets fall outside of window
    // printf("1\n");
    evictAny(self);

    // do nothing if our bit is a 0
    // printf("2\n");
    if (!item) {
        // printf("item = 0\n");
        return getCount(self);
    }

    // add our new bit if its a 1 and potentially cascade merge
    // printf("3\n");
    merge(self, self->now, 0); // index 0 is group 1
    
    // get the current count
    // printf("**** APX: returning, called getCount *****\n");
    // printf("****** COUNT OF BUCKETS %d\n", count_buckets(self));
    // printf("4\n");
    return getCount(self);
}

void evictAny(StateApx* self) {
    if (self->last_group_idx == -1) { // if no valid groups
        return;
    }

    struct GroupNode *last = &(self->groups[self->last_group_idx]);

    // checks if the oldest bucket in the last group is too old
    if ((int) last->buckets[last->bucket_evict] <= (int) ((self->now) - (self->W))) {
    // if ((int) *(last->buckets + last->bucket_evict) <= (int) ((self->now) - (self->W))) {
        // printf("Evicting a bucket\n");
        // increment bucket evict by 1
        last->bucket_evict = (last->bucket_evict + 1) % (self->k + 1);
        // printf("**** APX: Printing bucket_evict %d\n", last->bucket_evict);
        last->num_buckets--;

        // possibly update last group idx
        if (last->num_buckets == 0) {
            // iterate backwards from last_group_idx to 0 to find new last group
            int i;
            for (i = self->last_group_idx; i >= 0; i--) {
                if (self->groups[i].num_buckets != 0) {
                    self->last_group_idx = i;
                    return;
                }
            }
        }
        // check if group is empty, then remove group
        // if (last->num_buckets == 0) {
        //     // if the evict index is equal to the insert index, we remove the group
        //     struct GroupNode* last_next = last->next;
        //     struct GroupNode* last_prev = last->prev;
        //     last_prev->next = last_next;
        //     last_next->prev = last_prev;
        //     free(self->tail->prev);
        // }
    }
}



int getCount(StateApx* self) {
    // traverse the groups array until last_group_idx and count each group as if they were full, except the last group
    if (self->last_group_idx < 0) {
        return 0;
    }
    int i;
    int count = 0;
    for (i = 0; i <= self->last_group_idx; i++) {
        count += (int)(pow(2, i) + 1e-9) * self->groups[i].num_buckets;
    }
    // subtract one bucket from the last group and add 1
    count -= (int)(pow(2, self->last_group_idx) + 1e-9) - 1;
    
    return count;

    // // traverse the linked list and count each group as if they were full, except the last group
    // struct GroupNode* cur = self->head->next;
    // if (cur == self->tail) { // list empty
    //     return 0;
    // }
    // int count = 0;
    // while (cur->next != self->tail) { // stops one before the last group
    //     printf("groupnode #: %d\n", cur->bucket_size);
    //     count += cur->bucket_size * cur->num_buckets;
    //     cur = cur->next;
    // }
    // // printf("pre-count: %d\n", count);
    // // printf("num_buckets: %d\n", cur->num_buckets);
    // // printf("bucket_size: %d\n", cur->bucket_size);
    // // now do the last group, except make one of the buckets = 1
    // count += cur->bucket_size * (cur->num_buckets - 1) + 1;
    // printf("post-count: %d\n", count);
    // return count;
}

/**
 * Inputs:
 *  int ts - the timestamp of the bucket received from the previous merge call
 *  int group_idx - the index of the current group
*/
void merge(StateApx* self, int ts, int group_idx) {
    int size = (int)(pow(2, group_idx) + 1e-9);
    // int size = exp(log(2) * group_idx); // derives bucket size using group idx
    // printf("size: %d\n", size);
    // printf("SIZE1: %d\n", prev->next->bucket_size);
    // printf("inserting bucket timstamp: %d, size: %d\n", ts, size);
    // The next bucket is valid
    // printf("**** APX: call merge, next bucket size is: %d, size: %d *****\n", prev->next->bucket_size, size);
    struct GroupNode *group = &(self->groups[group_idx]);
    // printf("(pre if block) group.buckets: %p\n", group.buckets);
    // printf("1\n");
    
    if (group->num_buckets != 0) {
        // printf("**** APX: next bucket valid *****\n");
        // check if bucket needs to merge
        // printf("2\n");
        if (group->num_buckets >= self->k + 1) {
            // printf("3\n");
            // printf("**** APX: group full; need to merge *****\n");
            // if so then remove the oldest two buckets, update fields, and call merge
            // removes oldest bucket
            group->bucket_evict = (group->bucket_evict + 1) % (self->k + 1);
            // collect newer ts of second oldest bucket
            int timestamp = group->buckets[group->bucket_evict];
            // int timestamp = *(prev->next->buckets + prev->next->bucket_evict);
            // removes second oldest bucket
            group->bucket_evict = (group->bucket_evict + 1) % (self->k + 1);
            // updates bucket count
            group->num_buckets -= 2;
            // call merge
            N_MERGES++;
            merge(self, timestamp, group_idx + 1);
        }

    } else { // next bucket invalid, need to create new bucket
        // printf("**** APX: next bucket invalid *****\n");
        // printf("4\n");
        struct GroupNode *newNode = &(self->groups[(int) log2(size)]);
        
        // struct GroupNode *newNode = (GroupNode*) malloc(sizeof(GroupNode));
        // newNode->bucket_size = size;
        // printf("__________________________________________ %d\n", self->k+1);
        // int *A = (int*) malloc(sizeof(int) * (self->k + 1));
        // newNode->buckets = &A[0];
        // printf("***bucket idx: %d\n", (int) log2(newNode.bucket_size) * (self->k + 1));
        newNode->buckets = &(self->buckets[(int) log2(size) * (self->k + 1)]); // use global idx (newNode->bucket_ize - 1) * k + 1
        // printf("newNode.buckets: %p\n", newNode->buckets);
        // printf("buckets start: %p\n", self->buckets);
        // self->cur_idx += self->k + 1; // increment global idx
        newNode->bucket_insert = 0;
        newNode->bucket_evict = 0;
        newNode->num_buckets = 0;

        // possibly update last group idx
        if (group_idx > self->last_group_idx) {
            self->last_group_idx = group_idx;
        }

    }
    // printf("5\n");
    // finally insert ts into group
    // printf("SIZE2: %d\n", prev->next->bucket_size);
    // printf("insert: %d\n", group->bucket_insert);
    // printf("group.buckets[group.bucket_insert]: %d\n", group.buckets[group.bucket_insert]);
    // printf("group.buckets: %p\n", group->buckets);
    group->buckets[group->bucket_insert] = ts;
    // printf("6\n");
    // *(prev->next->buckets + prev->next->bucket_insert) = ts;
    // printf("SIZE3: %d\n", prev->next->bucket_size);
    group->bucket_insert = (group->bucket_insert + 1) % (self->k + 1);
    // printf("7\n");
    //printf("post insert val: %d\n", prev->next->bucket_insert);
    group->num_buckets++;
    // printf("8\n");
}




#endif // _WINDOW_BIT_COUNT_APX_
