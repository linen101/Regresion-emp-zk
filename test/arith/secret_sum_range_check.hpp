#ifndef EMP_ZK_RANGE_CHECK_H
#define EMP_ZK_RANGE_CHECK_H

#include "emp-tool/emp-tool.h"
#include <emp-zk/emp-zk.h>
#include <iostream>
#include "utils.hpp"
using namespace emp;
using namespace std;

void secret_range_check(int64_t inp, IntFp zk_inp, int64_t B_low, int64_t B_high, vector<IntFp> &zk_zero_checking, IntFp &zk_l) {
    //assert(inp >= B_low);
    //assert(inp <= B_high);

	// changed in utils to handle negative B_low values
    int needed_bits_range_check = get_num_range_bits(B_low, B_high) + 1; // For the sign bit;

    IntFp tmp0, tmp1;

    tmp0 = zk_inp.negate();
    tmp0 = tmp0 + B_low;

    tmp1 = IntFp(B_high, PUBLIC);
    tmp1 = tmp1.negate();
    tmp1 = tmp1 + zk_inp;

    bool* bits = new bool[needed_bits_range_check * 2];

    // Mask for the target bit width (64 bits in this example)
    const int target_bit_width = needed_bits_range_check; // Adjust as necessary
    if (target_bit_width >= 64) {
        throw std::invalid_argument("target_bit_width must be less than 64 for int64_t");
    }
    const int64_t mask = (1LL << target_bit_width) - 1;


    // x - a -> changed to signed int
    int64_t delta_low = (inp - B_low) & mask;
    if (delta_low & (1LL << (target_bit_width - 1))) {
        delta_low -= (1LL << target_bit_width); // Handle sign extension
    }

    // b - x -> changed to signed int
    int64_t delta_high = (B_high - inp) & mask;
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

#endif //EMP_ZK_RANGE_CHECK_H
