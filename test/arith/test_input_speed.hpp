#ifndef TEST_INPUT_SPEED_H
#define TEST_INPUT_SPEED_H

#include "emp-tool/emp-tool.h"
#include <emp-zk/emp-zk.h>
#include <iostream>
using namespace emp;
using namespace std;


const long long CHUNK_SIZE = 1LL << 20; // Process in chunks of 1M elements

void test_input_speed(BoolIO<NetIO> **ios, int party, long long input_sz) {
    long long total_sz = input_sz;  // Convert log size to actual size
    std::cout << "Total input size: " << total_sz << std::endl;

    srand(time(NULL));

    uint64_t *a = new uint64_t[CHUNK_SIZE];  // Allocate buffer for a chunk
    IntFp *x = new IntFp[CHUNK_SIZE];

    //setup_zk_arith<BoolIO<NetIO>>(ios, threads, party);

    long long processed = 0;
    double total_time = 0.0;

    /* Chunked batch input */
    while (processed < total_sz) {
        long long chunk_size = min((long long)CHUNK_SIZE, total_sz - processed); // Fix applied

        // Generate random input for the current chunk
        for (long long i = 0; i < chunk_size; ++i)
            a[i] = rand() % PR;

        auto start = clock_start();
        batch_feed(x, a, chunk_size);
        double tt = time_from(start);
        total_time += tt;

        processed += chunk_size;
    }

    std::cout << "Chunked batch input average time: " 
              << (total_time / 1000 ) << " ms " << std::endl;

    //finalize_zk_arith<BoolIO<NetIO>>();

    delete[] a;
    delete[] x;
}

#endif // TEST_INPUT_SPEED_H
