#include "range_check.hpp"
#include "host_ip.hpp"
#include "random_linear_combination.hpp"

int port, party;
const int threads = 16;

// number of entries to go through the range checks
// set this number to be high enough so offline phase batching pattern does not affect too much
int num_records = 1000;

// the range length [1, range_len]
int range_len = 100000000;

typedef struct _data_entry {
	uint64_t inp;
	IntFp zk_inp;
} data_entry;

vector<data_entry> create_random_dataset(vector<uint64_t> &precomputed_dataset, vector<IntFp> &zk_precomputed_dataset) {
	// randomly generate a dataset where all elements are all in the right ranges

	vector<data_entry> dataset;
	for(int i = 0; i < num_records; i++) {
		data_entry new_entry;
		new_entry.inp = 99999999; // since 1 is always in the range, we use 1 in the benchmark
		new_entry.zk_inp = IntFp(new_entry.inp, ALICE);

		precomputed_dataset.push_back(new_entry.inp);
		zk_precomputed_dataset.push_back(new_entry.zk_inp);

		dataset.push_back(new_entry);
	}

	return dataset;
}

int main(int argc, char** argv) {
	parse_party_and_port(argv, &party, &port);
	BoolIO<NetIO> *ios[threads];
	for (int i = 0; i < threads; ++i)
		ios[i] = new BoolIO<NetIO>(
			new NetIO(party == ALICE ? nullptr : "127.0.0.1", port + i),
			party == ALICE);

	std::cout << std::endl << "------------ ";
	std::cout << "ZKP range check";
	std::cout << " ------------" << std::endl << std::endl;
	
	if (argc < 3) {
		std::cout << "usage: [binary] PARTY PORT NUM_RECORDS RANGE"
				<< std::endl;
		return -1;
	} else if (argc < 5) {
		num_records = 1000;
		range_len = 1000;
	} else {
		num_records = atoi(argv[3]);
		range_len = atoi(argv[4]);
	}

	auto total_time_start = clock_start();

	setup_zk_bool<BoolIO<NetIO>>(ios, threads, party);
	setup_zk_arith<BoolIO<NetIO>>(ios, threads, party);

	vector<IntFp> zk_zero_checking;
	vector<uint64_t> precomputed_dataset;
	vector<IntFp> zk_precomputed_dataset;

	/************************************************************************************/

	//if(party == ALICE) {
	//	cout << "start to create fake data and input them, used OT triples = " << ZKFpExec::zk_exec->print_total_triple() << endl;
	//}

	auto dataset = create_random_dataset(precomputed_dataset, zk_precomputed_dataset);

	if(party == ALICE) {
		cout << "num of entries: " << dataset.size() << endl;
		//cout << "after loading the dataset, used OT triples = " << ZKFpExec::zk_exec->print_total_triple() << endl;
	}

	for(int i = 0; i < dataset.size(); i++) {
		range_check(dataset[i].inp, dataset[i].zk_inp, 99998474, range_len, zk_zero_checking);
	}

	//if(party == ALICE) {
		//cout << "after all the range checks, used OT triples = " << ZKFpExec::zk_exec->print_total_triple() << endl;
	//}

	// compute the random linear combination
	// temporarily, we set the challenges to be 7 and 8

	uint64_t randlc_res_1;
	IntFp zk_randlc_res_1 = IntFp(0, PUBLIC);

	uint64_t randlc_res_2;
	IntFp zk_randlc_res_2 = IntFp(0, PUBLIC);

	if(party == ALICE) {
		cout << "num of elements in the precomputed data = " << precomputed_dataset.size() << endl;
	}

	random_linear_combination(precomputed_dataset, zk_precomputed_dataset, 7, randlc_res_1, zk_randlc_res_1);
	random_linear_combination(precomputed_dataset, zk_precomputed_dataset, 8, randlc_res_2, zk_randlc_res_2);

	if(party == ALICE) {
		cout << "checksum 1 = " << randlc_res_1 << endl;
		cout << "checksum 2 = " << randlc_res_2 << endl;
	}

	batch_reveal_check_zero(zk_zero_checking.data(), zk_zero_checking.size());
	finalize_zk_bool<BoolIO<NetIO>>();
	finalize_zk_arith<BoolIO<NetIO>>();

	auto total_time = time_from(total_time_start);
	printf("total time in sec: %f\n", total_time/ CLOCKS_PER_SEC);
	{
		FILE * fp = fopen("./benchmark_result.txt", "w");
		fprintf(fp, "%f\n", total_time);
		fclose(fp);
	}

	for(int i = 0; i < threads+1; ++i) {
		delete ios[i]->io;
		delete ios[i];
	}
	return 0;
}

//
// binary execution: ./bin/test_arith_range_check_local 1 8080
// 1 is the number of party (can be 0/1)
// 8080 is the port
// the range is [a,b]
// where a is defined in the program
// and b in the benchmark_input.txt