#ifndef EMP_ZK_UTILS_H
#define EMP_ZK_UTILS_H




// int64_t for negative B_low
int get_num_range_bits(int64_t B_low, int64_t B_high) {
	return ceil(log2(B_high - B_low + 1));
}

#endif //EMP_ZK_UTILS_H