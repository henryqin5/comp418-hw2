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
    uint32_t W;
    uint32_t k;
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
uint32_t getCount(StateApx* self);
void merge(StateApx* self, int ts, GroupNode *group);

// k = 1/eps
// if eps = 0.01 (relative error 1%) then k = 100
// if eps = 0.001 (relative error 0.1%) the k = 1000
uint64_t wnd_bit_count_apx_new(StateApx* self, uint32_t wnd_size, uint32_t k) {
    assert(wnd_size >= 1);
    assert(k >= 1);

    // store the constants k and window size
    self->W = wnd_size;
    self->k = k;

    // point head and tail to each other
    self->head = &((GroupNode) {0, NULL, 0, 0, 0, NULL, NULL});
    self->tail = &((GroupNode) {0, NULL, 0, 0, 0, NULL, NULL});
    self->head->next = self->tail;
    self->head->prev = NULL;
    self->tail->prev = self->head;
    self->tail->next = NULL;
    
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
    // This is useful for debugging.
}

uint32_t wnd_bit_count_apx_next(StateApx* self, bool item) {
    // increment logical clock
    self->now++;

    // check if any buckets fall outside of window
    evictAny(self);

    // do nothing if our bit is a 0
    if (!item) {
        return getCount(self);
    }

    // add our new bit if its a 1
    insertBucket(self);

    // cascading merge to preserve invariant
    // merge(self);
    
    // get the current count
    return getCount(self);
}

void insertBucket(StateApx* self) {
    // check if there is no 1-valued group
    if (self->head->next == self->tail || (*(self->head->next)).bucket_size != 1) {
        struct GroupNode *newNode = (GroupNode*) malloc(sizeof(GroupNode));
        newNode->bucket_size = 1;
        int A[self->k + 1];
        newNode->buckets = &A[0];
        newNode->buckets[0] = self->now; // insert timestamp into buckets array
        newNode->bucket_insert = 1; // insert index is now 1
        newNode->bucket_evict = 0;
        newNode->num_buckets = 1; // starts at 1

        // insert newNode after head
        newNode->next = self->head->next;
        newNode->prev = self->head;
        self->head->next->prev = newNode;
        self->head->next = newNode;
    } else { // the group of buckets size 1 already exists
        // check if the group is full
        if (self->head->next->num_buckets >= self->k + 1) {
            // if the group would be full, call merge using new bucket on first group
            merge(self, self->now, self->head->next);
        } else { // if not, just insert the bucket
            struct GroupNode *group1 = self->head->next;
            group1->buckets[group1->bucket_insert] = self->now;
            group1->bucket_insert = (group1->bucket_insert + 1) % (self->k + 1);
            group1->num_buckets += 1;
        }
    }
}

void evictAny(StateApx* self) {
    if (self->head->next == NULL || self->tail->prev == NULL) {
        return;
    }
    struct GroupNode last = *(self->tail->prev);

    if (last.buckets[last.bucket_evict] < self->now - self->W) {
        // increment bucket evict by 1
        last.bucket_evict = (last.bucket_evict + 1) % (self->k + 1);
        last.num_buckets--;

        // check if group is empty, then remove group
        if (last.num_buckets == 0) {
            // if the evict index is equal to the insert index, we remove the group
            struct GroupNode* last_next = last.next;
            struct GroupNode* last_prev = last.prev;
            last_prev->next = last_next;
            last_next->prev = last_prev;
            free(self->tail->prev);
        }
    }
}



uint32_t getCount(StateApx* self) {

}

/**
 * Inputs:
 *  int ts - the timestamp of the bucket received from the previous merge call
 *  GroupNode *group - pointer to the previous existing group
*/
void merge(StateApx* self, int ts, GroupNode *group) {
    // check if group->next exists and is the correct bucket size, if not then create new group
    // if (group) {

    // }
    // before inserting, check if need to merge
        // if so, then remove oldest two buckets (collect newer ts)
    // insert ts into current group

    // call merge using newer ts and group->next
}


#endif // _WINDOW_BIT_COUNT_APX_
