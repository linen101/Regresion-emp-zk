#include "emp-tool/emp-tool.h"
#include <emp-zk/emp-zk.h>
#include <iostream>

using namespace emp;
using namespace std;

int port, party;
int repeat, sz;
const int threads = 1;

void test_inner_product(BoolIO<NetIO> *ios[threads], int party) {
  srand(time(NULL));
  uint64_t *witness = new uint64_t[repeat * 2 * sz];

  uint64_t *constant = new uint64_t[repeat];

  setup_zk_arith<BoolIO<NetIO>>(ios, threads, party);

  IntFp *x = new IntFp[repeat * 2 * sz];

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

  auto start = clock_start();
  for (int i = 0; i < 2 * sz; ++i)
    x[i] = IntFp(witness[i], ALICE);

  
  for (int j = 0; j < repeat; ++j) {
    fp_zkp_inner_prdt<BoolIO<NetIO>>(j + x, j + x + sz, constant[j] , sz);
  }

  finalize_zk_arith<BoolIO<NetIO>>();

  double tt = time_from(start);
  cout << "prove " << repeat << " degree-2 polynomial of length " << sz << endl;
  cout << "time use: " << tt / 1000 << " ms" << endl;
  cout << "average time use: " << tt / 1000 / repeat << " ms" << endl;

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
    std::cout << "usage: [binary] PARTY PORT POLY_NUM POLY_DIMENSION"
              << std::endl;
    return -1;
  } else if (argc < 5) {
    repeat = 100;
    sz = 10;
  } else {
    repeat = atoi(argv[3]);
    sz = atoi(argv[4]);
  }

  test_inner_product(ios, party);

  for (int i = 0; i < threads; ++i) {
    delete ios[i]->io;
    delete ios[i];
  }
  return 0;
}