#include "cachesim.h"


#define UNUSED(x) (void)(x)
#define TRUE 1
#define FALSE 0

/**
 * The stuct that you may use to store the metadata for each block in
 * the cache
 */
typedef struct block {
    uint64_t tag;  // The tag stored in that block
    uint8_t valid; // Valid bit
    uint8_t dirty; // Dirty bit

    // Add any metadata here to perform LRU/FIFO replacement. For LRU,
    // think about how you could use a counter variable to track the
    // oldest block in a set. For FIFO, consider how you would
    // implement a queue with only a single pointer.

    // TODO
    uint64_t FIFO;
    uint64_t counter;
} block_t;

/**
 * A struct for storing the configuration of the cache as passed in
 * the cache_init function.
 */
typedef struct config {
    uint64_t C;
    uint64_t B;
    uint64_t S;
    enum REPLACEMENT_POLICY policy;
} config_t;

// You may add global variables, structs, and function headers below
// this line if you need any

// TODO
typedef struct {
    block_t *blocks;
} set;

set *cache;
config_t config;
uint64_t counter = 0;
int totalSets;
int blocksPerSet;
uint64_t fifo_counter = 1;

/**
 * Initializes your cache with the passed in arguments.
 *
 * @param C The total size of your cache is 2^C bytes
 * @param S The total number of blocks in a line/set of your cache are 2^S
 * @param B The size of your blocks is 2^B bytes
 * @param policy The replacement policy of your cache
 */
void cache_init(uint64_t C, uint64_t B, uint64_t S, enum REPLACEMENT_POLICY policy)
{
    // Initialize the cache here. We strongly suggest using arrays for
    // the cache meta data. The block_t struct given above may be
    // useful.
	totalSets = (1 << (C - B - S));
	cache = malloc((uint64_t) totalSets * sizeof(set));
	if (cache == NULL) {exit(0);}
    config.C = C;
    config.B = B;
    config.S = S;
    blocksPerSet = (1 << config.S);
    for (int i = 0; i < totalSets; i++) {
        cache[i].blocks = (block_t*) calloc((size_t) blocksPerSet, sizeof(block_t));
        if (cache[i].blocks == NULL) {exit(0);}
	}
	config.policy = policy;
}

/**
 * Simulates one cache access at a time.
 *
 * @param rw The type of access, READ or WRITE
 * @param address The address that is being accessed
 * @param stats The struct that you are supposed to store the stats in
 * @return TRUE if the access is a hit, FALSE if not
 */
uint8_t cache_access(char rw, uint64_t address, cache_stats_t* stats)
{
    // Find the tag and index and check if it is a hit. If it is a
    // miss, then simulate getting the value from memory and putting
    // it in the cache. Make sure you track the stats along the way!
	uint8_t isHit = FALSE;
    uint8_t DirectMapped = FALSE;
    if (config.S == 0) {
        DirectMapped = TRUE;
    }
    uint64_t C = config.C;
    uint64_t B = config.B;
    uint64_t S = config.S;
    uint64_t tagBits = (64 - B - (C - B - S));
    uint64_t tag = address >> (uint64_t) (64 - tagBits); 
    uint64_t indexBits = C - B - S;
    uint64_t index = (address >> B) & (uint64_t) ((0x1 << indexBits) - 1);
    for (int i = 0; i < blocksPerSet; i++) {
        if ((cache[index].blocks[i].tag == tag) && cache[index].blocks[i].valid) {
        	cache[index].blocks[i].FIFO = 0;
            isHit = TRUE;
            if (rw == WRITE) {
                cache[index].blocks[i].dirty = TRUE; 
            }
            cache[index].blocks[i].counter = counter;
            counter++;
            if (cache[index].blocks[i].FIFO == 0) {
            	cache[index].blocks[i].FIFO = fifo_counter;
            	fifo_counter++;
            }
        } else {
        	cache[index].blocks[i].FIFO++;
        }
    }

    if (rw == READ) {
        stats->reads++;
        if (!isHit) {stats->read_misses++;} 
    } else {
        stats->writes++;
        if (!isHit) {stats->write_misses++;} 
    }
    stats->accesses++;
    stats->misses = stats->read_misses + stats->write_misses;
    int blockIndex = 0;
    if (!isHit) {
        if (!DirectMapped) {
            uint8_t foundValid = FALSE;
            for (int i = 0; i < blocksPerSet; i++) {
                if (!cache[index].blocks[i].valid) {
                    blockIndex = i;
                    i = blocksPerSet;
                    foundValid = TRUE;
                }
            }
            if (!foundValid) {
            	if (config.policy == LRU) {
            		uint64_t count = cache[index].blocks[0].counter;
                	for (int i = 0; i < blocksPerSet; i++) {
                    	if (cache[index].blocks[i].counter < count) {
                        	blockIndex = i;
                        	count = cache[index].blocks[i].counter;
                    	}
                	}
            	} else if (config.policy == FIFO) {
            		uint64_t count = cache[index].blocks[0].FIFO;
            		for (int i = 0; i < blocksPerSet; i++) {
            			if (cache[index].blocks[i].FIFO < count) {
            				count = cache[index].blocks[i].FIFO;
            			}
            		}
            	}
                
            }
        }
        if (cache[index].blocks[blockIndex].valid && cache[index].blocks[blockIndex].dirty) {
            stats->write_backs++;
            cache[index].blocks[blockIndex].dirty = FALSE;
        }
        cache[index].blocks[blockIndex].counter = counter;
        counter++;
        cache[index].blocks[blockIndex].tag = tag;
        cache[index].blocks[blockIndex].valid = TRUE;
        if (rw == WRITE) {
            cache[index].blocks[blockIndex].dirty = TRUE;
        }
    }
    if (!isHit) {return FALSE;
    } else {return TRUE;}
}

/**
 * Frees up memory and performs any final calculations before the
 * statistics are outputed by the driver
 */
void cache_cleanup(cache_stats_t* stats)
{
    // Make sure to free any malloc'd memory here. To check if you
    // have freed up the the memory, run valgrind. For more
    // information, google how to use valgrind.
	stats->misses = stats->read_misses+stats->write_misses;
    stats->miss_rate= (double)(stats->misses)/(stats->accesses);
    stats->avg_access_time= stats->cache_access_time + (stats->memory_access_time*stats->miss_rate);
    for (int i = 0; i < totalSets; i++) {
        free(cache[i].blocks);
    }   
    free(cache);
}

/**
 * Computes the tag of a given address based on the parameters passed
 * in
 *
 * @param address The address whose tag is to be computed
 * @param C The size of the cache (i.e. Size of cache is 2^C)
 * @param B The size of the cache block (i.e. Size of block is 2^B)
 * @param S The set associativity of the cache (i.e. Set-associativity is 2^S)
 * @return The computed tag
 */
uint64_t get_tag(uint64_t address, uint64_t C, uint64_t B, uint64_t S)
{
    UNUSED(C);
    address >>= (B+S);
    int tag_mask = (1 << (64-B-S)) -1;  //00001000 - 1 = 00000111
    return address & (unsigned long long) tag_mask;
}

/**
 * Computes the index of a given address based on the parameters
 * passed in
 *
 * @param address The address whose tag is to be computed
 * @param C The size of the cache (i.e. Size of cache is 2^C)
 * @param B The size of the cache block (i.e. Size of block is 2^B)
 * @param S The set associativity of the cache (i.e. Set-associativity is 2^S)
 * @return The computed index
 */
uint64_t get_index(uint64_t address, uint64_t C, uint64_t B, uint64_t S)
{
    UNUSED(C);
    address >>= B;
    int index_mask = ~(-1 << S);
    return address & (unsigned long long) index_mask;
}
