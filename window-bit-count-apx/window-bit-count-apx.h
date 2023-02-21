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

typedef struct StateApx {
    struct GroupNode *groups;
    int now;
    int W;
    int k;
    int *buckets;
    int last_group_idx; // index to the last valid group in groups
    int bit_count;
} StateApx;

typedef struct GroupNode {
    int* buckets; // k+1 sized circular array of buckets
    int bucket_insert; // index where next bucket will be inserted
    int bucket_evict; // index of next bucket to evict (oldest) if group is full
    int num_buckets; // number of buckets within this group
} GroupNode;

void evictAny(StateApx* self);
int getCount(StateApx* self);
void merge(StateApx* self, int ts, int group_idx);

// k = 1/eps
// if eps = 0.01 (relative error 1%) then k = 100
// if eps = 0.001 (relative error 0.1%) the k = 1000
uint64_t wnd_bit_count_apx_new(StateApx* self, int wnd_size, int k) {
    assert(wnd_size >= 1);
    assert(k >= 1);

    // store the constants k and window size
    self->W = wnd_size;
    self->k = k;

    self->last_group_idx = -1;
    self->bit_count = 0;

    // Initialize all the buckets we could ever dream of
    int mem1 = (k + 1) * (ceil(log2(wnd_size/(k + 1) + 1)) + 1);
    self->buckets = (int*) calloc(sizeof(int), mem1);

    // Malloc group node times number of groups
    int mem2 = (ceil(log2(wnd_size/(k + 1) + 1)) + 1);
    self->groups = (GroupNode*) calloc(sizeof(GroupNode), mem2); (k + 1) * (ceil(log2(wnd_size/(k + 1) + 1)) +1);
    
    // initialize logical timer
    self->now = 0;

    // The function should return the total number of bytes allocated on the heap.
    return mem1 + mem2;
}

void wnd_bit_count_apx_destruct(StateApx* self) {
    // Make sure you free the memory allocated on the heap.
    free(self->buckets);
    free(self->groups);
}

void wnd_bit_count_apx_print(StateApx* self) {
    // This is useful for debugging.
}

int wnd_bit_count_apx_next(StateApx* self, bool item) {
    // increment logical clock
    self->now++;
    
    // check if any buckets fall outside of window
    evictAny(self);

    // do nothing if our bit is a 0
    if (!item) {
        return getCount(self);
    }
    self->bit_count++;
    // add our new bit if its a 1 and potentially cascade merge
    merge(self, self->now, 0); // index 0 is group 1
    
    // get the current count
    return getCount(self);
}

void evictAny(StateApx* self) {
    if (self->last_group_idx == -1) { // if no valid groups
        return;
    }

    struct GroupNode *last = &(self->groups[self->last_group_idx]);

    // checks if the oldest bucket in the last group is too old
    if ((int) last->buckets[last->bucket_evict] <= (int) ((self->now) - (self->W))) {
        // increment bucket evict by 1
        last->bucket_evict = (last->bucket_evict + 1) % (self->k + 1);
        last->num_buckets--;
        self->bit_count -= (int)(pow(2, self->last_group_idx) + 1e-9);

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
    }
}

int getCount(StateApx* self) {
    if (self->last_group_idx < 0) {
        return 0;
    }
     // locate the last group and do the approximation math
    int apx_count = self->bit_count - (int)(pow(2, self->last_group_idx) + 1e-9) + 1;
    return apx_count;
}

/**
 * Inputs:
 *  int ts - the timestamp of the bucket received from the previous merge call
 *  int group_idx - the index of the current group
*/
void merge(StateApx* self, int ts, int group_idx) {
    int size = (int)(pow(2, group_idx) + 1e-9);
    struct GroupNode *group = &(self->groups[group_idx]);
    
    if (group->num_buckets != 0) {
        // check if bucket needs to merge
        if (group->num_buckets >= self->k + 1) {
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
        struct GroupNode *newNode = &(self->groups[(int) log2(size)]);
        
        newNode->buckets = &(self->buckets[(int) log2(size) * (self->k + 1)]); // use global idx (newNode->bucket_ize - 1) * k + 1

        newNode->bucket_insert = 0;
        newNode->bucket_evict = 0;
        newNode->num_buckets = 0;

        // possibly update last group idx
        if (group_idx > self->last_group_idx) {
            self->last_group_idx = group_idx;
        }

    }
    // finally insert ts into group
    group->buckets[group->bucket_insert] = ts;
    group->bucket_insert = (group->bucket_insert + 1) % (self->k + 1);
    group->num_buckets++;
}




#endif // _WINDOW_BIT_COUNT_APX_
