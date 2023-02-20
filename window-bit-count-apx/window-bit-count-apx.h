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
    struct GroupNode *head; 
    struct GroupNode *tail;
    struct GroupNode *groups;
    int now;
    int W;
    int k;
    int *buckets;
    // int cur_idx; // global index to global buckets array, increments by k + 1 each time we use it
} StateApx;

typedef struct GroupNode {
    int bucket_size; // size of buckets
    int* buckets; // k+1 sized circular array of buckets
    int bucket_insert; // index where next bucket will be inserted
    int bucket_evict; // index of next bucket to evict (oldest) if group is full
    int num_buckets; // number of buckets within this group
    struct GroupNode *next; // pointer to next group
    // struct GroupNode *prev; // pointer to previous group
} GroupNode;

void insertBucket(StateApx* self);
void evictAny(StateApx* self);
int getCount(StateApx* self);
void merge(StateApx* self, int ts, GroupNode *prev, int size);
int count_buckets(StateApx* self);

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
    self->head = (GroupNode*) malloc(sizeof(GroupNode));
    self->head->bucket_size = 0;
    self->head->buckets = NULL;
    self->head->bucket_insert = 0;
    self->head->bucket_evict = 0;
    self->head->num_buckets = 0;
    self->tail = (GroupNode*) malloc(sizeof(GroupNode));
    self->tail->bucket_size = 0;
    self->tail->buckets = NULL;
    self->tail->bucket_insert = 0;
    self->tail->bucket_evict = 0;
    self->tail->num_buckets = 0;

    // point head to tail
    self->head->next = self->tail;
    self->tail->next = NULL;

    // Initialize all the buckets we could ever dream of
    // printf(" ceil(log2(wnd_size/(k + 1) + 1)) %d\n", wnd_size/(k + 1) + 1);
    self->buckets = (int*) malloc(sizeof(int) * (k + 1) * (ceil(log2(wnd_size/(k + 1) + 1)) + 1));
    // Malloc group node times number of groups
    self->groups = (GroupNode*) malloc(sizeof(GroupNode) * (ceil(log2(wnd_size/(k + 1) + 1)) + 1));
    // printf("SIZE OF BUCKETS MEMORY: %d\n", sizeof(int) * (k + 1) * (ceil(log2(wnd_size/(k + 1) + 1)) +1));

    // fprintf(stdout, "Head stats: num buckets is %d and group num is %d\n", self->head->num_buckets, self->head->bucket_size);
    // fprintf(stdout, "Tail stats: num buckets is %d and group num is %d\n", self->tail->num_buckets, self->tail->bucket_size);
    
    // initialize logical timer
    self->now = 0;

    // initialize global buckets index
    // self->cur_idx = 0;

    // The function should return the total number of bytes allocated on the heap.
    // int mem = sizeof(int) * (k) * ceil(log2(wnd_size/(k))) + 2 * sizeof(struct GroupNode);
    int mem = sizeof(int) * (k + 1) * (ceil(log2(wnd_size/(k + 1) + 1)) + 1) + sizeof(GroupNode) * (ceil(log2(wnd_size/(k + 1) + 1)) + 1);

    printf("MEMO USAGE %d\n", mem);
    return mem;
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
    // printf("now: %d\n", self->now);


    // check if any buckets fall outside of window
    evictAny(self);

    // do nothing if our bit is a 0
    if (!item) {
        // printf("item = 0\n");
        return getCount(self);
    }

    // add our new bit if its a 1
    // printf("**** APX: call insertBucket *****\n");
    insertBucket(self);

    // cascading merge to preserve invariant
    // merge(self);
    
    // get the current count
    // printf("**** APX: returning, called getCount *****\n");
    // printf("****** COUNT OF BUCKETS %d\n", count_buckets(self));
    return getCount(self);
}

void insertBucket(StateApx* self) {
    // TODO: rewriting merge so that insertBucket can just call merge
    // (treating the head as the previous group to group1)
    merge(self, self->now, self->head, 1);

    // check if there is no 1-valued group
    // if (self->head->next == self->tail || (*(self->head->next)).bucket_size != 1) {
    //     struct GroupNode *newNode = (GroupNode*) malloc(sizeof(GroupNode));
    //     newNode->bucket_size = 1;
    //     int A[self->k + 1];
    //     newNode->buckets = &A[0];
    //     newNode->buckets[0] = self->now; // insert timestamp into buckets array
    //     newNode->bucket_insert = 1; // insert index is now 1
    //     newNode->bucket_evict = 0;
    //     newNode->num_buckets = 1; // starts at 1

    //     // insert newNode after head
    //     newNode->next = self->head->next;
    //     newNode->prev = self->head;
    //     self->head->next->prev = newNode;
    //     self->head->next = newNode;
    // } else { // the group of buckets size 1 already exists
    //     // check if the group is full
    //     if (self->head->next->num_buckets >= self->k + 1) {
    //         // if the group would be full, call merge using new bucket on first group
    //         merge(self, self->now, self->head->next);
    //     } else { // if not, just insert the bucket
    //         struct GroupNode *group1 = self->head->next;
    //         group1->buckets[group1->bucket_insert] = self->now;
    //         group1->bucket_insert = (group1->bucket_insert + 1) % (self->k + 1);
    //         group1->num_buckets += 1;
    //     }
    // }
}

void evictAny(StateApx* self) {
    if (self->head->next == self->tail) {
        return;
    }
    // struct GroupNode* last = self->tail->prev;
    // looping through LL to get last
    struct GroupNode* last = self->head;
    while (last->next != self->tail) {
        last = last->next;
    }
    // printf("**** APX: call evict on group: %d\n", last->bucket_size);
    // printf("**** APX: last bucket_evict: %d\n", last->bucket_evict);
    // printf("**** APX: looking to evict ts: %d\n", last->buckets[last->bucket_evict]);
    // printf("**** APX: Subtract: %d\n", self->now - self->W);

    if ((int) last->buckets[last->bucket_evict] <= (int) ((self->now) - (self->W))) {
    // if ((int) *(last->buckets + last->bucket_evict) <= (int) ((self->now) - (self->W))) {
        // printf("Evicting a bucket\n");
        // increment bucket evict by 1
        last->bucket_evict = (last->bucket_evict + 1) % (self->k + 1);
        // printf("**** APX: Printing bucket_evict %d\n", last->bucket_evict);
        last->num_buckets = last->num_buckets - 1;


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
    // traverse the linked list and count each group as if they were full, except the last group
    struct GroupNode* cur = self->head->next;
    if (cur == self->tail) { // list empty
        return 0;
    }
    int count = 0;
    while (cur->next != self->tail) { // stops one before the last group
        printf("groupnode #: %d\n", cur->bucket_size);
        count += cur->bucket_size * cur->num_buckets;
        cur = cur->next;
    }
    // printf("pre-count: %d\n", count);
    // printf("num_buckets: %d\n", cur->num_buckets);
    // printf("bucket_size: %d\n", cur->bucket_size);
    // now do the last group, except make one of the buckets = 1
    count += cur->bucket_size * (cur->num_buckets - 1) + 1;
    // printf("post-count: %d\n", count);
    return count;
}

/**
 * Inputs:
 *  int ts - the timestamp of the bucket received from the previous merge call
 *  GroupNode *prev - pointer to the previous existing group (head if inserting into group 1)
 *  int size - the size of the current group (used to check if prev->next is correct)
*/
void merge(StateApx* self, int ts, GroupNode *prev, int size) {
    // printf("SIZE1: %d\n", prev->next->bucket_size);
    // printf("inserting bucket timstamp: %d, size: %d\n", ts, size);
    // The next bucket is valid
    // printf("**** APX: call merge, next bucket size is: %d, size: %d *****\n", prev->next->bucket_size, size);
    if (prev->next != self->tail && prev->next->bucket_size == size) {
        // printf("**** APX: next bucket valid *****\n");
        // check if next bucket needs to merge
        if (prev->next->num_buckets >= self->k + 1) {
            printf("**** APX: group full; need to merge *****\n");
            // if so then remove the oldest two buckets, update fields, and call merge
            // removes oldest bucket
            prev->next->bucket_evict = (prev->next->bucket_evict + 1) % (self->k + 1);
            // collect newer ts of second oldest bucket
            int timestamp = prev->next->buckets[prev->next->bucket_evict];
            // int timestamp = *(prev->next->buckets + prev->next->bucket_evict);
            // removes second oldest bucket
            prev->next->bucket_evict = (prev->next->bucket_evict + 1) % (self->k + 1);
            // updates bucket count
            prev->next->num_buckets -= 2;
            // call merge
            merge(self, timestamp, prev->next, size * 2);
        }

    } else { // next bucket invalid, need to create new bucket
        // printf("**** APX: next bucket invalid *****\n");
        struct GroupNode newNode = self->groups[(int) log2(size)];
        // struct GroupNode *newNode = (GroupNode*) malloc(sizeof(GroupNode));
        newNode.bucket_size = size;
        // printf("__________________________________________ %d\n", self->k+1);
        // int *A = (int*) malloc(sizeof(int) * (self->k + 1));
        // newNode->buckets = &A[0];
        // printf("***bucket idx: %d\n", (int) log2(newNode->bucket_size) * (self->k + 1));
        newNode.buckets = &(self->buckets[(int) log2(newNode.bucket_size) * (self->k + 1)]); // use global idx (newNode->bucket_ize - 1) * k + 1
        // self->cur_idx += self->k + 1; // increment global idx
        newNode.bucket_insert = 0;
        newNode.bucket_evict = 0;
        newNode.num_buckets = 0;

        // insert newNode into linked list
        // newNode->next = prev->next;
        // newNode->prev = prev;
        // prev->next = newNode;
        // newNode->next->prev = newNode;
        newNode.next = prev->next;
        prev->next = &(newNode);

        // printf("MERGE: New group number: %d\n", newNode->bucket_size);
        // printf("MERGE: New Number of Buckets: %d\n", newNode->num_buckets);
        // debug: checking LL
        // wnd_bit_count_apx_print(self);

        // prev->next is now newNode
    }
    // finally insert ts into group
    // printf("**** APX: insert for merge *****\n");
    // printf("SIZE2: %d\n", prev->next->bucket_size);
    // printf("insert: %d\n", prev->next->bucket_insert);
    prev->next->buckets[prev->next->bucket_insert] = ts;
    // *(prev->next->buckets + prev->next->bucket_insert) = ts;
    // printf("SIZE3: %d\n", prev->next->bucket_size);
    prev->next->bucket_insert = (prev->next->bucket_insert + 1) % (self->k + 1);
    //printf("post insert val: %d\n", prev->next->bucket_insert);
    prev->next->num_buckets++;
    // printf("MERGE: inserted into group: %d\n", prev->next->bucket_size);
    // printf("MERGE: now has: %d buckets\n", prev->next->num_buckets);
    // printf("**** APX: merge done, returning *****\n");
}

int count_buckets(StateApx* self){
    struct GroupNode* cur = self->head->next;
    int count = 0;
    while (cur != self->tail) {
        count += cur->num_buckets;
        cur = cur->next;
    }

    return count;
}


#endif // _WINDOW_BIT_COUNT_APX_
