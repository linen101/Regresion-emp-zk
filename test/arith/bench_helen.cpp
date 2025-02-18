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

  
  for (int j = 0; j < repeat; ++j) {
    fp_zkp_inner_prdt<BoolIO<NetIO>>(x, x + sz, constant, sz);
  }

  //finalize_zk_arith<BoolIO<NetIO>>();

  
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
    std::cout << "usage: [binary] PARTY PORT D PARTIES"
              << std::endl;
    return -1;
  } else if (argc < 4) {
    d = 10;
    parties = 2;
  } else {
    d = atoll(argv[3]);
    parties = atoi(argv[4]);
  }
  setup_zk_arith<BoolIO<NetIO>>(ios, threads, party);
  setup_zk_bool<BoolIO<NetIO>>(ios, threads, party);
  
  auto start = clock_start();
  // Case 1: matrix = d *  d , n=d dots
  int64_t b_low = 1; 
  int64_t b_high = 10000;
  repeat = d*d;
  sz  = d;
  test_n = repeat * sz *2;
  for (int i = 0; i < 2; ++i)
    test_inner_product(ios, party);
    test_input_speed(ios, party, test_n);
    test_input_range(b_low, b_high, repeat, party, parties);


  // Case 2: size = d * 1, n = d

  repeat = d*1 ;
  sz  = d;
  test_n = repeat * sz *2;
  for (int i = 0; i < 16; ++i)
    test_inner_product(ios, party);
    test_input_speed(ios, party, test_n);
  
 
  double tt = time_from(start);
  cout << "helen ["  << d << "] feature input " << endl;
  cout << "time use for "<< parties << " parties:" << (tt* parties*(parties-1)) / 1000 << " ms" << endl;


  finalize_zk_arith<BoolIO<NetIO>>();
  finalize_zk_bool<BoolIO<NetIO>>();
  for (int i = 0; i < threads; ++i) {
    delete ios[i]->io;
    delete ios[i];
  }
  return 0;
}
