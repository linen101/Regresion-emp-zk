#include "emp-tool/emp-tool.h"
#include <emp-zk/emp-zk.h>
#include <iostream>

#include <vector>
#include <cstdlib>
#include <cstring>

using namespace emp;
using namespace std;

int port, party;
int repeat, sz;
int num_parties;
const int threads = 16;



int main(int argc, char **argv) {
  parse_party_and_port(argv, &party, &port);
  BoolIO<NetIO> *ios[threads];
  for (int i = 0; i < threads; ++i)
    ios[i] = new BoolIO<NetIO>(
        new NetIO(party == ALICE ? nullptr : "127.0.0.1", port + i),
        party == ALICE);

  std::cout << "\n------------ ZKP inner product test ------------\n\n";

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
    num_parties = atoi(argv[5]);
  }

  // Chunk size - Define based on memory capacity
  size_t chunk_size = std::size_t(sz);

  
  auto start = clock_start();

  std::vector<uint64_t> constant(repeat, 0);

  if (party == ALICE) {
    // Process witnesses in chunks
    for (int i = 0; i < repeat; ++i) {
      std::vector<uint64_t> witness(2 * chunk_size, 0);

      // Generate random witness values
      for (size_t j = 0; j < chunk_size; ++j) {
        witness[j] = rand() % PR;
        witness[chunk_size + j] = rand() % PR;
      }

      // Compute the polynomial f(x, y) = Σ x_i * y_i
      uint64_t sum = 0, tmp;
      for (size_t j = 0; j < chunk_size; ++j) {
        tmp = mult_mod(witness[j], witness[chunk_size + j]);
        sum = add_mod(sum, tmp);
      }

      constant[i] = PR - sum;

      // Send chunked witness data if necessary
    }

    // Send the constant values
    ios[0]->send_data(constant.data(), repeat * sizeof(uint64_t));
  } else {
    // If verifier, receive data
    ios[0]->recv_data(constant.data(), repeat * sizeof(uint64_t));
  }
  
  double tt = time_from(start);
  cout << "Commit to " << repeat << " degree-2 polynomial of length " << sz << endl
       << " for " << num_parties << " parties " << endl;
  cout << "Time used: " << tt / 1000 << " ms" << endl;
  cout << "Average time used: " << tt / 1000 / repeat << " ms" << endl;

  for (int i = 0; i < threads; ++i) {
    delete ios[i]->io;
    delete ios[i];
  }
  return 0;
}
