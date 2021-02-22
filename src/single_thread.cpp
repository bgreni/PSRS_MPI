#include <iostream>
#include <chrono>
#include <stdlib.h>
#include <algorithm>
using namespace std;
#define MICRO_IN_SECONDS (1000000.0)


int main(int argc, char ** argv) {
    const long N = strtol(argv[1], nullptr, 10);
    const long SEED = strtol(argv[2], nullptr, 10);
    srandom(SEED);
    long *original = new long[N];

    for (int i = 0; i < N; ++i) {
        original[i] = random();
    }

    auto start = chrono::high_resolution_clock::now();
    sort(original, original + N);
    auto end = chrono::high_resolution_clock::now();
    long long dur = chrono::duration_cast<chrono::microseconds>(end - start).count();

    double duration = double(dur) / MICRO_IN_SECONDS;

    cout << "Cores used: 1" << endl;
    cout << "Size of vector: " << N << endl;
    cout << "Baseline sort on single thread: " << duration << endl << endl;

    delete[] original;
}