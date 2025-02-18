#include "emp-tool/emp-tool.h"
#include <emp-zk/emp-zk.h>
#include <iostream>
using namespace emp;
using namespace std;

int port, party;
const int threads = 1;
const long long CHUNK_SIZE = 1LL << 20; // Process in chunks of 1M elements

void test_input_speed(BoolIO<NetIO> **ios, int party, int input_sz) {
    long long total_sz = input_sz;  // Convert log size to actual size
    std::cout << "Total input size: " << total_sz << std::endl;

    srand(time(NULL));

    uint64_t *a = new uint64_t[CHUNK_SIZE];  // Allocate buffer for a chunk
    IntFp *x = new IntFp[CHUNK_SIZE];

    setup_zk_arith<BoolIO<NetIO>>(ios, threads, party);

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

    finalize_zk_arith<BoolIO<NetIO>>();

    delete[] a;
    delete[] x;
}

int main(int argc, char **argv) {
    parse_party_and_port(argv, &party, &port);
    BoolIO<NetIO> *ios[threads + 1];
    for (int i = 0; i < threads + 1; ++i)
        ios[i] = new BoolIO<NetIO>(
            new NetIO(party == ALICE ? nullptr : "127.0.0.1", port + i),
            party == ALICE);

    std::cout << "\n------------ Chunked Input Handling for Large Sizes ------------\n" << std::endl;

    if (argc < 3) {
        std::cout << "Usage: [binary] PARTY PORT INPUT_SZ" << std::endl;
        return -1;
    }

    long long num = (argc == 3) ? 20 : atoi(argv[3]);

    test_input_speed(ios, party, num);

    for (int i = 0; i < threads + 1; ++i) {
        delete ios[i]->io;
        delete ios[i];
    }
    return 0;
}
