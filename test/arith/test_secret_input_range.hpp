#ifndef EMP_ZK_RANGE_CHECK_H
#define EMP_ZK_RANGE_CHECK_H

#include "emp-tool/emp-tool.h"
#include <emp-zk/emp-zk.h>
#include <iostream>
#include "utils.hpp"
using namespace emp;
using namespace std;
/*
typedef struct _data_entry {
	int64_t inp;
	IntFp zk_inp;
} data_entry;
*/
vector<data_entry> create_random_dataset(vector<int64_t> &precomputed_dataset, vector<IntFp> &zk_precomputed_dataset, long long num_records) {
	// randomly generate a dataset where all elements are all in the right ranges

	vector<data_entry> dataset;
	for(int i = 0; i < num_records; i++) {
		data_entry new_entry;
		
		// Set inp based on whether i is even or odd
        if (i % 2 == 0) {
            new_entry.inp = 1; // Even index
        } else {
            new_entry.inp = -1; // Odd index
        }

		new_entry.zk_inp = IntFp(new_entry.inp, ALICE);

		precomputed_dataset.push_back(new_entry.inp);
		zk_precomputed_dataset.push_back(new_entry.zk_inp);

		dataset.push_back(new_entry);
	}

	return dataset;
}


void test_secret_input_range(int64_t B_low, int64_t B_high, long long num_records, int party, int parties) {
    int64_t l = num_records / 2;
	IntFp zk_l = IntFp(l, ALICE);
	zk_l = zk_l.negate();
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
        // changed in utils to handle negative B_low values
        int needed_bits_range_check = get_num_range_bits(B_low, B_high) + 1; // For the sign bit;

        IntFp tmp0, tmp1;

        tmp0 = dataset[i].zk_inp.negate();
        tmp0 = tmp0 + B_low;

        tmp1 = IntFp(B_high, PUBLIC);
        tmp1 = tmp1.negate();
        tmp1 = tmp1 + dataset[i].zk_inp;

        bool* bits = new bool[needed_bits_range_check * 2];

        // Mask for the target bit width (64 bits in this example)
        const int target_bit_width = needed_bits_range_check; // Adjust as necessary
        if (target_bit_width >= 64) {
            throw std::invalid_argument("target_bit_width must be less than 64 for int64_t");
        }
        const int64_t mask = (1LL << target_bit_width) - 1;


        // x - a -> changed to signed int
        int64_t delta_low = (dataset[i].inp - B_low) & mask;
        if (delta_low & (1LL << (target_bit_width - 1))) {
            delta_low -= (1LL << target_bit_width); // Handle sign extension
        }

        // b - x -> changed to signed int
        int64_t delta_high = (B_high - dataset[i].inp) & mask;
        if (delta_high & (1LL << (target_bit_width - 1))) {
            delta_high -= (1LL << target_bit_width); // Handle sign extension
        }

        // Extract bits of x-a, b-x in the clear
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

        // Check if they are bits
        for (int i = 0; i < needed_bits_range_check * 2; i++) {
            IntFp tmp;
            tmp = zk_bits[i] * zk_bits[i];
            tmp = tmp + zk_bits[i].negate();

            zk_zero_checking.emplace_back(tmp);
        }

        // Check the bit decomposition for two's complement :done
        uint64_t cur = 1;
        for (int j = 0; j < needed_bits_range_check - 1; ++j) {
            tmp0 = tmp0 + zk_bits[j] * cur;
            tmp1 = tmp1 + zk_bits[needed_bits_range_check + j] * cur;
            cur <<= 1;
        }

        // handle MSB for two's complement : done
        tmp0 = tmp0 + (zk_bits[needed_bits_range_check - 1] * cur).negate();
        tmp1 = tmp1 + (zk_bits[2 * needed_bits_range_check - 1] * cur).negate();

        // update sum of sign bits indicating if this value passes the range check
        zk_l = zk_l + zk_bits[needed_bits_range_check - 1] + zk_bits[2 * needed_bits_range_check - 1];
        zk_zero_checking.emplace_back(tmp0);
        zk_zero_checking.emplace_back(tmp1);
        delete[] bits;
        delete[] zk_bits;
    }
    
    batch_reveal_check_zero(zk_zero_checking.data(), zk_zero_checking.size());
    auto total_time = time_from(total_time_start);
    cout << "prove [" << num_records << "] secret range checks" << endl;
    cout << "time use for "<< parties << " parties:" << (total_time* parties*(parties-1)) /  CLOCKS_PER_SEC << " sec" << endl;

    
}

#endif //EMP_ZK_RANGE_CHECK_H
