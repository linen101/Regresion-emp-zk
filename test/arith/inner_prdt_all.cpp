#include "emp-tool/emp-tool.h"
#include <emp-zk/emp-zk.h>
#include <iostream>

using namespace emp;
using namespace std;

int port, party;
int repeat, sz;
int num_parties;
const int threads = 16;

void test_inner_product(BoolIO<NetIO> *ios[threads], int party,  int  p, uint64_t *witness , uint64_t *constant) {
  //rand(time(NULL));

  //  emp-zk/emp-zk-arith/emp-zk-arith.h ->
  setup_zk_arith<BoolIO<NetIO>>(ios, threads, party);

  //  emp-zk/emp-zk-arith/int_fp.h -> declare a vector of length 2*sz with secret values in Fp
  long long test_n = repeat * 2 * sz;
  IntFp *x = new IntFp[test_n];

  for (int i = 0; i < test_n; ++i)
    x[i] = IntFp(witness[i], ALICE);

  
  for (int j = 0; j < repeat; ++j) {
    for (int k = 0; k < num_parties-1; k++) {
      // fp_zkp_inner_prdkt(x, y, z, n)
      // emp-zk/emp-zk-arith/int_fp.h:: fp_zkp_inner_prdt
      // which  calls emp-zk/emp-zk-arith/polynomial::zkp_inner_prdt
      fp_zkp_inner_prdt<BoolIO<NetIO>>(j + x, j + x + sz, constant[j], sz);
    } 
  }

  finalize_zk_arith<BoolIO<NetIO>>();
  //delete[] witness;
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
    std::cout << "usage: [binary] PARTY PORT POLY_NUM POLY_DIMENSION NUM_PARTIES"
              << std::endl;
    return -1;
  } else if (argc < 5) {
    repeat = 100;
    sz = 10;
    num_parties = 2;
  } else {
    repeat = atoi(argv[3]);
    sz = atoi(argv[4]);
    num_parties  = atoi(argv[5]);
  }

  // create random witness for benchmark purposes, 
  // we create only one witness
  // even if we have multiple parties
  // witness initialization does not contribute to the execution time

  
  // uint64_t constant = 0;
  
  long long test_n = repeat * 2 * sz;

  uint64_t *witness = new uint64_t[test_n];

  uint64_t *constant = new uint64_t[repeat];

  // initialize vector of witnesses to  0.
  //memset(witness, 0, repeat * 2 * sz * sizeof(uint64_t));


  // initialize vector of witnesses to  0.
  //memset(constant, 0, repeat * sizeof(uint64_t));


  // create vectors random vetors x, y
  if (party == ALICE) {
    //  create a random vector of x_i, y_i values as witnesses
    // witness vector has size 2*sz
    // first sz positions are for x values
    // last sz positions are for y values y
    
    for (int i = 0; i < repeat; ++i) {
      for (int j = 0; j < sz; ++j) {
        witness[i + j] = rand() % PR;
        witness[i + sz + j] = rand() % PR;
      }
    }
    // compute the polynomial f(x,y) = \Sigma_{i \in [n]} x_i y_i
    for (int i = 0; i < repeat; ++i) {
      uint64_t sum = 0, tmp;
      for (int j = 0; j < sz; ++j) {
        // x_i y_i
        tmp = mult_mod(witness[i + j], witness[i + sz + j]);
        // \Sigma_{j \in [i]} x_j y_j
        sum = add_mod(sum, tmp);
      }

      constant[i] = PR - sum;
    }
    
    // if its prover send data
    ios[0]->send_data(constant, repeat * sizeof( uint64_t));
  } else {
    // if its verifier receive data
    ios[0]->recv_data(constant, repeat * sizeof( uint64_t));
  }

  // zk prove for all parties
  auto start = clock_start();
  for (int p = 0; p < num_parties; p++) {
    cout << "Processing party " << p << endl;
    test_inner_product(ios, party, p, witness, constant);
  }  
 
  double tt = time_from(start);
  cout << "prove " << repeat << " degree-2 polynomial of length " << sz << endl << " for " << num_parties  << "  parties " << endl;
  cout << "time use: " << tt / 1000 << " ms" << endl;
  cout << "average time use: " << tt / 1000 / repeat << " ms" << endl;

  for (int i = 0; i < threads; ++i) {
    delete ios[i]->io;
    delete ios[i];
  }
  return 0;
}
