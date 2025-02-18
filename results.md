# RESULTS

## COMPILATION MATRIX MUL

1. g++ -std=c++17 -march=native -o bin/test_arith_matrix_mul_local \
    test/arith/matrix_mul.cpp \
    -I/usr/local/include -L/usr/local/lib \
    -lemp-tool  -lemp-zk -lssl -lcrypto -lgmp
2. g++ -std=c++17 -march=native -O3 -flto -o bin/test_arith_matrix_mul_local \
    test/arith-local/matrix_mul.cpp \
    -I/usr/local/include -L/usr/local/lib \
    -lemp-tool  -lemp-zk -lssl -lcrypto -lgmp  




# compilation with custom install

cmake .. -DCMAKE_INSTALL_PREFIX=$HOME/emp-zk
make -j$(nproc)
make install

3. g++ -std=c++17 -march=native -O3 -flto \
    -o bin/test_arith_inner_prdt_all test/arith/inner_prdt_all.cpp \
    -I$HOME/emp-zk-lib/include -L$HOME/emp-zk-lib/lib \
    -lemp-tool -lemp-zk -lssl -lcrypto -lgmp




./bin/test_arith_inner_prdt_all 0 8090 625 25 2
./bin/test_arith_inner_prdt_all 1 8090 625 25 2


### ARITHMETIC CIRCUIT MAT MUL

#### provided bin
N = 1024
TIME = 1.45921e+08

#### Compiled bin
1. N = 1024
TIME = 7.30633e+08

2. N = 1024
TIME = 1.54157e+08

## COMPILATION inner prod

1. g++ -std=c++17 -march=native -O3 -flto -o bin/test_arith_inner_prod_local \
    test/arith-local/inner_prod.cpp \
    -I/usr/local/include -L/usr/local/lib \
    -lemp-tool  -lemp-zk -lssl -lcrypto -lgmp 

### ARITHMETIC CIRCUIT POLY INNER PRODUCT (no witness commitment)
#### provided bin
POLY length = 1024
Number of inner prods = 1048576
TIME =  6775.22 ms

POLY length = 1000
Number of inner prods = 1000000
TIME = 6340.88 ms


#### Compiled bin 
buffer size = 1024
POLY length = 1024
Number of inner prods = 1048576
TIME = 7505.37 ms

#### Modified buffer size to 1048576 ()
buffer size = 1048576
POLY length = 1024
Number of inner prods = 1048576
TIME = 6973.58 ms


### ARITHMETIC CIRCUIT POLY INNER PRODUCT (with witness commitment)
#### provided bin
-

#### Compiled bin 

##### 100 x 100 matrix
buffer size = 10000
POLY length = 100
Number of inner prods = 10000
TIME = 861.242 ms = 0.861 sec

##### 100 x 1000 matrix
buffer size = 10000
POLY length = 1000
Number of inner prods = 10000
TIME = 2717.82 ms = 2.718 sec

##### TEST
buffer size = 100000
POLY length = 1000
Number of inner prods = 100000
TIME = 26314.1 ms = 26.314 sec

##### ESTIMATION: 1000 x 1000 matrix
buffer size = 1000000
POLY length = 1000
Number of inner prods = 1000000
TIME = ~ 270000 ms ~ 270 sec