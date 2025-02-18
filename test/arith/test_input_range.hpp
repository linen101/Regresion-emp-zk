#ifndef EMP_ZK_INPUT_RANGE_CHECK_H
#define EMP_ZK_INPUT_RANGE_CHECK_H

#include "emp-tool/emp-tool.h"
#include <emp-zk/emp-zk.h>
#include <iostream>
#include "utils.hpp"
using namespace emp;
using namespace std;

typedef struct _data_entry {
	uint64_t inp;
	IntFp zk_inp;
} data_entry;

vector<data_entry> create_random_dataset(vector<uint64_t> &precomputed_dataset, vector<IntFp> &zk_precomputed_dataset, long long num_records) {
	// randomly generate a dataset where all elements are all in the right ranges

	vector<data_entry> dataset;
	for(int i = 0; i < num_records; i++) {
		data_entry new_entry;
		new_entry.inp = 1; // since 1 is always in the range, we use 1 in the benchmark
		new_entry.zk_inp = IntFp(new_entry.inp, ALICE);

		precomputed_dataset.push_back(new_entry.inp);
		zk_precomputed_dataset.push_back(new_entry.zk_inp);

		dataset.push_back(new_entry);
	}

	return dataset;
}


void test_input_range( uint64_t B_low, uint64_t B_high, long long num_records, int party) {
    vector<IntFp> zk_zero_checking;
	vector<uint64_t> precomputed_dataset;
	vector<IntFp> zk_precomputed_dataset;
    auto dataset = create_random_dataset(precomputed_dataset, zk_precomputed_dataset, num_records);

	if(party == ALICE) {
		cout << "num of entries: " << dataset.size() << endl;
		//cout << "after loading the dataset, used OT triples = " << ZKFpExec::zk_exec->print_total_triple() << endl;
	}
    auto total_time_start = clock_start();
	for(int i = 0; i < dataset.size(); i++) {
           // range_check(dataset[i].inp, dataset[i].zk_inp, 99998474, range_len, zk_zero_checking);
        

        assert(dataset[i].inp >= B_low);
        assert(dataset[i].inp <= B_high);

        // Check that each value is of the correct range
        // y[i] = (inp[i] - B_low) - \sum_{0..} (bits[j] * 2^j)
        int needed_bits_range_check = get_num_range_bits(B_low, B_high);

        IntFp tmp0, tmp1;

        tmp0 = dataset[i].zk_inp.negate();
        tmp0 = tmp0 + B_low;

        tmp1 = IntFp(B_high, PUBLIC);
        tmp1 = tmp1.negate();
        tmp1 = tmp1 + dataset[i].zk_inp;

        // Generate the binary testing bits
        // First, compute how many bits are needed.
        // And then add it to ZK_bits

        bool* bits = new bool[needed_bits_range_check * 2];

        uint64_t delta_low = dataset[i].inp - B_low;
        uint64_t delta_high = B_high - dataset[i].inp;

        for (int j = 0; j < needed_bits_range_check; j++) {
            bits[j] = delta_low & 1;
            delta_low >>= 1;

            bits[needed_bits_range_check + j] = delta_high & 1;
            delta_high >>= 1;
        }

        IntFp* zk_bits = new IntFp[needed_bits_range_check * 2];
        for (int i = 0; i < needed_bits_range_check * 2; i++) {
            zk_bits[i] = IntFp(bits[i], ALICE);
        }

        for (int i = 0; i < needed_bits_range_check * 2; i++) {
            IntFp tmp;
            tmp = zk_bits[i] * zk_bits[i];
            tmp = tmp + zk_bits[i].negate();

            zk_zero_checking.emplace_back(tmp);
        }

        uint64_t cur = 1;
        for (int j = 0; j < needed_bits_range_check; ++j) {
            tmp0 = tmp0 + zk_bits[j] * cur;
            tmp1 = tmp1 + zk_bits[needed_bits_range_check + j] * cur;

            cur <<= 1;
        }

        zk_zero_checking.emplace_back(tmp0);
        zk_zero_checking.emplace_back(tmp1);

        delete[] bits;
        delete[] zk_bits;
    }
    batch_reveal_check_zero(zk_zero_checking.data(), zk_zero_checking.size());
    auto total_time = time_from(total_time_start);
	printf("total time in sec for range check: %f\n", total_time/ CLOCKS_PER_SEC);
}

#endif //EMP_ZK_INPUT_RANGE_CHECK_H