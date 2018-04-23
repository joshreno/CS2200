// DO NOT MODIFY THIS FILE!

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <getopt.h>
#include "cachesim.h"

#define TRUE 1
#define FALSE 0

static void print_statistics(cache_stats_t* p_stats);

static void print_help_and_exit(void) {
    printf("cachesim [OPTIONS] < traces/file.trace\n");
    printf("  -C\t\tTotal size of the cache in bytes is 2^S\n");
    printf("  -B\t\tSize of each block in bytes is 2^B\n");
    printf("  -S\t\tNumber of blocks per set is 2^S\n");
    printf("  -p\t\tPrint out every access (use this to compare to given solutions)\n");
    printf("  -r\t\tThe replacement policy (FIFO or LRU)\n");
    printf("  -h\t\tThis helpful output\n");
    exit(0);
}

static enum REPLACEMENT_POLICY get_policy(char* name) {
    if (strcmp(name, "LRU") == 0 || strcmp(name, "lru") == 0) {
        return LRU;
    } else if (strcmp(name, "FIFO") == 0 || strcmp(name, "fifo") == 0) {
        return FIFO;
    }
    return CUSTOM;
}

static void get_policy_name(char* name, enum REPLACEMENT_POLICY policy) {
    switch (policy) {
        case FIFO:
            strcpy(name, "FIFO");
            break;
        case LRU:
            strcpy(name, "LRU");
            break;
        case CUSTOM:
            strcpy(name, "CUSTOM");
            break;
        }
}

int main(int argc, char* argv[]) {
    int opt;
    uint64_t c = DEFAULT_C;
    uint64_t b = DEFAULT_B;
    uint64_t s = DEFAULT_S;
    uint8_t should_print = FALSE;
    enum REPLACEMENT_POLICY r = FIFO;
    FILE* fin  = stdin;

    // Read arguments
    while(-1 != (opt = getopt(argc, argv, "C:B:S:r:i:ph"))) {
        switch(opt) {
            case 'C':
                c = strtoull(optarg, NULL, 0);
                break;
            case 'B':
                b = strtoull(optarg, NULL, 0);
                break;
            case 'S':
                s = strtoull(optarg, NULL, 0);
                break;
            case 'r':
                r = get_policy(optarg);
                break;
            case 'p':
                should_print = TRUE;
                break;
            case 'i':
                fin = fopen(optarg, "r");
                continue;
            case 'h':
            default:
                print_help_and_exit();
                break;
        }
    }

    char name[10];
    get_policy_name(name, r);

    printf("Cache Settings\n");
    printf("C: %" PRIu64 "\n", c);
    printf("B: %" PRIu64 "\n", b);
    printf("S: %" PRIu64 "\n", s);
    printf("Replacement policy: %s\n", name);

    // Setup the cache
    cache_init(c, b, s, r);

    // Setup statistics
    cache_stats_t stats;
    memset(&stats, 0, sizeof(cache_stats_t));
    stats.cache_access_time = 3;
    stats.memory_access_time = 120;

    // Begin reading the file
    char rw;
    uint64_t address;
    while (!feof(fin)) {
        int ret = fscanf(fin, "%c %" SCNx64 "\n", &rw, &address);
        if (ret == 2) {
            uint8_t is_hit = cache_access(rw, address, &stats);
            if (should_print) {
                printf(
                    "0x%012" PRIx64 "\t%s\t0x%012" PRIx64 "\t0x%012" PRIx64 "\n",
                    address,
                    is_hit ? "hit " : "miss",
                    get_tag(address, c, b, s),
                    get_index(address, c, b, s)
                );
            }
        }
    }

    printf("\n");
    cache_cleanup(&stats);
    print_statistics(&stats);
    fclose(fin);
    return 0;
}

static void print_statistics(cache_stats_t* p_stats) {
    // Overall stats
    printf("Cache Statistics\n");
    printf("Accesses: %" PRIu64 "\n", p_stats->accesses);
    printf("Reads: %" PRIu64 "\n", p_stats->reads);
    printf("Read misses: %" PRIu64 "\n", p_stats->read_misses);
    printf("Writes: %" PRIu64 "\n", p_stats->writes);
    printf("Write misses: %" PRIu64 "\n", p_stats->write_misses);
    printf("Misses: %" PRIu64 "\n", p_stats->misses);
    printf("Writebacks: %" PRIu64 "\n", p_stats->write_backs);

    // Access Times
    printf("Access time: %" PRIu64 "\n", p_stats->cache_access_time);
    printf("Memory access time: %" PRIu64 "\n", p_stats->memory_access_time);

    // Miss Rates
    printf("Miss rate: %f\n", p_stats->miss_rate);

    // Average Access Times
    printf("Average access time (AAT): %f\n", p_stats->avg_access_time);
}
