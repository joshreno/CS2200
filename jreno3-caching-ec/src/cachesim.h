// DO NOT MODIFY THIS FILE!

#ifndef CACHESIM_H
#define CACHESIM_H

#include <inttypes.h>
#include <stdlib.h>

typedef struct cache_stats {
    uint64_t accesses;

    uint64_t reads;
    uint64_t read_misses;

    uint64_t writes;
    uint64_t write_misses;

    uint64_t misses;
    uint64_t write_backs;

    uint64_t cache_access_time;
    uint64_t memory_access_time;

    double miss_rate;
    double avg_access_time;
} cache_stats_t;

enum REPLACEMENT_POLICY { FIFO = 0, LRU = 1, CUSTOM = 2};

void cache_init(uint64_t C,  uint64_t S, uint64_t B, enum REPLACEMENT_POLICY policy);
uint8_t cache_access(char rw, uint64_t address, cache_stats_t* stats);
void cache_cleanup(cache_stats_t* stats);

uint64_t get_tag(uint64_t address, uint64_t C, uint64_t B, uint64_t S);
uint64_t get_index(uint64_t address, uint64_t C, uint64_t B, uint64_t S);

static const uint64_t DEFAULT_C = 15;
static const uint64_t DEFAULT_B = 5;
static const uint64_t DEFAULT_S = 3;
static const enum REPLACEMENT_POLICY DEFAULT_R = FIFO;

static const char READ = 'r';
static const char WRITE = 'w';

#endif
