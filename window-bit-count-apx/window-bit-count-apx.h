#ifndef _WINDOW_BIT_COUNT_APX_
#define _WINDOW_BIT_COUNT_APX_

#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <math.h>

uint64_t N_MERGES = 0; // keep track of how many bucket merges occur

/*
    TODO: You can add code here.
*/

typedef struct StateApx {
    struct GroupNode *head; 
    struct GroupNode *tail;
    int now;
    int W;
    int k;
} StateApx;

typedef struct GroupNode {
    int bucket_size; // size of buckets
    int* buckets; // k+1 sized circular array of buckets
    int bucket_insert; // index where next bucket will be inserted
    int bucket_evict; // index of next bucket to evict (oldest) if group is full
    int num_buckets; // number of buckets within this group
    struct GroupNode *next; // pointer to next group
    struct GroupNode *prev; // pointer to previous group
} GroupNode;

void insertBucket(StateApx* self);
void evictAny(StateApx* self);
int getCount(StateApx* self);
void merge(StateApx* self, int ts, GroupNode *prev, int size);

// k = 1/eps
// if eps = 0.01 (relative error 1%) then k = 100
// if eps = 0.001 (relative error 0.1%) the k = 1000
uint64_t wnd_bit_count_apx_new(StateApx* self, int wnd_size, int k) {
    assert(wnd_size >= 1);
    assert(k >= 1);

    // store the constants k and window size
    self->W = wnd_size;
    self->k = k;

    // point head and tail to each other
    self->head = (GroupNode*) malloc(sizeof(GroupNode));
    self->head->bucket_size = 0;
    self->head->buckets = NULL;
    self->head->bucket_insert = 0;
    self->head->bucket_evict = 0;
    self->head->num_buckets = 0;
    self->head->prev = NULL;
    self->tail = (GroupNode*) malloc(sizeof(GroupNode));
    self->tail->bucket_size = 0;
    self->tail->buckets = NULL;
    self->tail->bucket_insert = 0;
    self->tail->bucket_evict = 0;
    self->tail->num_buckets = 0;
    self->tail->next = NULL;
    // fprintf(stdout, "Head stats: num buckets is %d and group num is %d\n", self->head->num_buckets, self->head->bucket_size);
    // fprintf(stdout, "Tail stats: num buckets is %d and group num is %d\n", self->tail->num_buckets, self->tail->bucket_size);
    self->head->next = self->tail;
    self->tail->prev = self->head;
    
    // initiate logical timer
    self->now = 0;

    // The function should return the total number of bytes allocated on the heap.
    return 0;
}

void wnd_bit_count_apx_destruct(StateApx* self) {
    // TODO: Fill me.
    // Make sure you free the memory allocated on the heap.
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
    printf("now: %d\n", self->now);


    // check if any buckets fall outside of window
    evictAny(self);

    // do nothing if our bit is a 0
    if (!item) {
        printf("item = 0\n");
        return getCount(self);
    }

    // add our new bit if its a 1
    // printf("**** APX: call insertBucket *****\n");
    insertBucket(self);

    // cascading merge to preserve invariant
    // merge(self);
    
    // get the current count
    // printf("**** APX: returning, called getCount *****\n");
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
    if (self->head->next == self->tail || self->tail->prev == self->head) {
        return;
    }
    struct GroupNode* last = self->tail->prev;
    // printf("**** APX: call evict on group: %d\n", last->bucket_size);
    printf("**** APX: looking to evict ts: %d\n", last->buckets[last->bucket_evict]);
    // printf("**** APX: Subtract: %d\n", self->now - self->W);

    if ((int) last->buckets[last->bucket_evict] <= (int) ((self->now) - (self->W))) {
        printf("Evicting a bucket\n");
        // increment bucket evict by 1
        last->bucket_evict = (last->bucket_evict + 1) % (self->k + 1);
        printf("**** APX: Printing bucket_evict %d\n", last->bucket_evict);
        last->num_buckets = last->num_buckets - 1;


        // check if group is empty, then remove group
        if (last->num_buckets == 0) {
            // if the evict index is equal to the insert index, we remove the group
            struct GroupNode* last_next = last->next;
            struct GroupNode* last_prev = last->prev;
            last_prev->next = last_next;
            last_next->prev = last_prev;
            free(self->tail->prev);
        }
    }
}



int getCount(StateApx* self) {
    // traverse the linked list backwards and count each group as if they were full, then subtract at the end
    struct GroupNode* cur = self->head->next;
    int count = 0;
    while (cur != self->tail) {
        count += cur->bucket_size * cur->num_buckets;
        cur = cur->next;
    }
    if (cur->prev != self->head) {
        count -= cur->prev->bucket_size - 1;
    }
    printf("count: %d\n", count);
    return count;
}

/**
 * Inputs:
 *  int ts - the timestamp of the bucket received from the previous merge call
 *  GroupNode *prev - pointer to the previous existing group (head if inserting into group 1)
 *  int size - the size of the current group (used to check if prev->next is correct)
*/
void merge(StateApx* self, int ts, GroupNode *prev, int size) {
    printf("inserting bucket timstamp: %d, size: %d\n", ts, size);
    // The next bucket is valid
    // printf("**** APX: call merge, next bucket size is: %d, size: %d *****\n", prev->next->bucket_size, size);
    if (prev->next != self->tail && prev->next->bucket_size == size) {
        // printf("**** APX: next bucket valid *****\n");
        // check if next bucket needs to merge
        if (prev->next->num_buckets >= self->k + 1) {
            // if so then remove the oldest two buckets, update fields, and call merge
            // removes oldest bucket
            prev->next->bucket_evict = (prev->next->bucket_evict + 1) % (self->k + 1);
            // collect newer ts of second oldest bucket
            int timestamp = prev->next->buckets[prev->next->bucket_evict];
            // removes second oldest bucket
            prev->next->bucket_evict = (prev->next->bucket_evict + 1) % (self->k + 1);
            // updates bucket count
            prev->next->num_buckets -= 2;
            // call merge
            merge(self, timestamp, prev->next, size * 2);
        }

    } else { // next bucket invalid, need to create new bucket
        // printf("**** APX: next bucket invalid *****\n");
        struct GroupNode *newNode = (GroupNode*) malloc(sizeof(GroupNode));
        newNode->bucket_size = size;
        int A[self->k + 1];
        newNode->buckets = &A[0];
        newNode->bucket_insert = 0;
        newNode->bucket_evict = 0;
        newNode->num_buckets = 0;

        // insert newNode into linked list
        newNode->next = prev->next;
        newNode->prev = prev;
        prev->next = newNode;
        newNode->next->prev = newNode;

        // printf("MERGE: New group number: %d\n", newNode->bucket_size);
        // printf("MERGE: New Number of Buckets: %d\n", newNode->num_buckets);
        // debug: checking LL
        // wnd_bit_count_apx_print(self);

        // prev->next is now newNode
    }
    // finally insert ts into group
    // printf("**** APX: insert for merge *****\n");
    prev->next->buckets[prev->next->bucket_insert] = ts;
    prev->next->bucket_insert = (prev->next->bucket_insert + 1) % (self->k + 1);
    prev->next->num_buckets++;
    // printf("MERGE: inserted into group: %d\n", prev->next->bucket_size);
    // printf("MERGE: now has: %d buckets\n", prev->next->num_buckets);
    // printf("**** APX: merge done, returning *****\n");
}


#endif // _WINDOW_BIT_COUNT_APX_
