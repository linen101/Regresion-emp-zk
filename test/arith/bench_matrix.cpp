#include "emp-tool/emp-tool.h"
#include <emp-zk/emp-zk.h>
#include <iostream>
#include "test_input_speed.hpp"
#include "test_input_range.hpp"
#include "test_secret_input_range.hpp"

using namespace emp;
using namespace std;

int port, party, parties;
long long repeat, sz;
long long n, d;
long long test_n;
const int threads = 8;

void test_inner_product(BoolIO<NetIO> *ios[threads], int party) {
  srand(time(NULL));
  uint64_t constant = 0;
  uint64_t *witness = new uint64_t[2 * sz];
  memset(witness, 0, 2 * sz * sizeof(uint64_t));

  //setup_zk_arith<BoolIO<NetIO>>(ios, threads, party);

  IntFp *x = new IntFp[2 * sz];

  if (party == ALICE) {
    uint64_t sum = 0, tmp;
    for (int i = 0; i < sz; ++i) {
      witness[i] = rand() % PR;
      witness[sz + i] = rand() % PR;
    }
    for (int i = 0; i < sz; ++i) {
      tmp = mult_mod(witness[i], witness[sz + i]);
      sum = add_mod(sum, tmp);
    }
    constant = PR - sum;
    ios[0]->send_data(&constant, sizeof(uint64_t));
  } else {
    ios[0]->recv_data(&constant, sizeof(uint64_t));
  }

  for (int i = 0; i < 2 * sz; ++i)
    x[i] = IntFp(witness[i], ALICE);

  auto start = clock_start();
  for (int j = 0; j < repeat; ++j) {
    fp_zkp_inner_prdt<BoolIO<NetIO>>(x, x + sz, constant, sz);
  }

  //finalize_zk_arith<BoolIO<NetIO>>();

  double tt = time_from(start);
  cout << "prove [" << n << "," << d << "] matrix of " << n << " dots"  << endl;
  cout << "time use for "<< parties << " parties:" << (tt* parties*(parties-1)) / 1000 << " ms" << endl;

  delete[] witness;
  delete[] x;
}

int main(int argc, char **argv) {
  parse_party_and_port(argv, &party, &port);
  BoolIO<NetIO> *ios[threads];
  for (int i = 0; i < threads; ++i)
    ios[i] = new BoolIO<NetIO>(
        new NetIO(party == ALICE ? nullptr : "127.0.0.1", port + i),
        party == ALICE);

  std::cout << std::endl << "------------ ";
  std::cout << "ZKP inner product test";
  std::cout << " ------------" << std::endl << std::endl;
  ;

  if (argc < 3) {
    std::cout << "usage: [binary] PARTY PORT N D PARTIES"
              << std::endl;
    return -1;
  } else if (argc < 5) {
    n = 100;
    d = 10;
    parties = 2;
  } else {
    n = atoll(argv[3]);
    d = atoll(argv[4]);
    parties = atoi(argv[5]);
  }
  setup_zk_arith<BoolIO<NetIO>>(ios, threads, party);
  setup_zk_bool<BoolIO<NetIO>>(ios, threads, party);
  // Case 1: matrix = d *  n , n dots
  repeat = (n /parties)*d;
  sz  = n / parties;
  test_n = repeat * sz *2;
  test_inner_product(ios, party);
  test_input_speed(ios, party, test_n);
  


  // Case 2: size = d * d, n = d

  repeat = d*d ;
  sz  = d;
  test_n = repeat * sz *2;
  test_inner_product(ios, party);
  test_input_speed(ios, party, test_n);
  int64_t b_low = 1; 
  int64_t b_high = 10000;
  test_input_range(b_low, b_high, repeat, party, parties);

  // Case 3: size = d * 1, n varies
  repeat = d ;
  sz  = n / parties;
  test_n = repeat * sz *2;
  test_inner_product(ios, party);
  test_input_speed(ios, party, test_n);

  // Range Checks for participation set update
  test_secret_input_range(b_low, b_high, sz, party, parties);
  test_secret_input_range(b_low, b_high, sz, party, parties);
  test_secret_input_range(b_low, b_high, sz, party, parties);


  finalize_zk_arith<BoolIO<NetIO>>();
  finalize_zk_bool<BoolIO<NetIO>>();
  for (int i = 0; i < threads; ++i) {
    delete ios[i]->io;
    delete ios[i];
  }
  return 0;
}
