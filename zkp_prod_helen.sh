#!/bin/bash

# Define values for d and n
d_values=(25 50 75 100)
parties_values=(2 3 4)
port=8080

# Create a log directory
log_dir="zkp_logs_helen"
mkdir -p "$log_dir"

# Step 1: Compile the program (No extra arguments passed)
echo "Compiling $PROGRAM..."
g++ -std=c++17 -march=native -o bin/test_arith_bench_helen     test/arith/bench_helen.cpp     -I/usr/local/include -L/usr/local/lib     -lemp-tool  -lemp-zk -lssl -lcrypto -lgmp

# Iterate over all d and n combinations
for parties in "${parties_values[@]}"; do
    for d in "${d_values[@]}"; do

        echo "Running test for d=$d, parties=$parties"

        # Run party 0 and party 1 in parallel
        ./bin/test_arith_bench_helen 0 $port $d $parties > "$log_dir/party0_p${parties}_d${d}.log" 2>&1 &
        pid0=$!

        ./bin/test_arith_bench_helen 1 $port $n $d $parties > "$log_dir/party1_p${parties}_d${d}.log" 2>&1 &
        pid1=$!

        # Wait for both parties to finish
        wait $pid0
        wait $pid1

        echo "Completed test for d=$d"
    done
done

echo "All tests completed."
