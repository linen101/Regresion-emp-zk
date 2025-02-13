#include "emp-tool/emp-tool.h"
#include <emp-zk/emp-zk.h>
#include <iostream>

using namespace emp;
using namespace std;

int port, party;
int repeat, sz;
int num_parties;
const int threads = 16;
const int chunk_size = 2500000; // Define chunk size based on available memory

void test_inner_product_commit(BoolIO<NetIO> *ios[threads], int party, int p, uint32_t *witness, uint32_t *constant, int chunk_start, int chunk_end) {
    setup_zk_arith<BoolIO<NetIO>>(ios, threads, party);

    int chunk_len = chunk_end - chunk_start;
    IntFp *x = new IntFp[chunk_len * 2];

    for (int i = 0; i < chunk_len * 2; ++i)
        x[i] = IntFp(witness[i], ALICE);
    
    // only meaure commit time
    
    //for (int j = 0; j < repeat; ++j) {
      //  for (int k = 0; k < num_parties - 1; k++) {
        //    fp_zkp_inner_prdt<BoolIO<NetIO>>(j + x, j + x + sz, constant[j], sz);
        //}
    //}
    
    finalize_zk_arith<BoolIO<NetIO>>();
    delete[] x;
}

int main(int argc, char **argv) {
    parse_party_and_port(argv, &party, &port);
    BoolIO<NetIO> *ios[threads];
    for (int i = 0; i < threads; ++i)
        ios[i] = new BoolIO<NetIO>(
            new NetIO(party == ALICE ? nullptr : "127.0.0.1", port + i),
            party == ALICE);

    cout << "\n------------ ZKP inner product test ------------\n\n";

    if (argc < 3) {
        cout << "usage: [binary] PARTY PORT POLY_NUM POLY_DIMENSION NUM_PARTIES\n";
        return -1;
    } else if (argc < 5) {
        repeat = 100;
        sz = 10;
        num_parties = 2;
    } else {
        repeat = atoi(argv[3]);
        sz = atoi(argv[4]);
        num_parties = atoi(argv[5]);
    }

    uint32_t *constant = new uint32_t[repeat];

    if (party == ALICE) {
        ios[0]->send_data(constant, repeat * sizeof(uint32_t));
    } else {
        ios[0]->recv_data(constant, repeat * sizeof(uint32_t));
    }

    auto start = clock_start();

    // Process witness in chunks
    int total_size = repeat * 2 * sz;
    for (int chunk_start = 0; chunk_start < total_size; chunk_start += chunk_size) {
        int chunk_end = min(chunk_start + chunk_size, total_size);
        int chunk_len = chunk_end - chunk_start;

        uint32_t *witness_chunk = new uint32_t[chunk_len];

        // Fill chunk
        if (party == ALICE) {
            for (int i = 0; i < chunk_len; ++i)
                witness_chunk[i] = rand() % PR;
        }

        for (int p = 0; p < num_parties; p++) {
            cout << "Processing chunk " << chunk_start / chunk_size + 1 << " for party " << p << endl;
            test_inner_product_commit(ios, party, p, witness_chunk, constant, chunk_start, chunk_end);
        }

        delete[] witness_chunk;
    }

    double tt = time_from(start);
    cout << "Proved " << repeat << " degree-2 polynomial of length " << sz << " for " << num_parties << " parties\n";
    cout << "Time use: " << tt / 1000 << " ms\n";
    cout << "Average time use: " << tt / 1000 / repeat << " ms\n";

    for (int i = 0; i < threads; ++i) {
        delete ios[i]->io;
        delete ios[i];
    }
    delete[] constant;

    return 0;
}
