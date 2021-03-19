#include <iostream>
#include <mpi.h>
#include <stdlib.h>
#include <chrono>
#include "utils.h"
#include <cmath>
#include <algorithm>
#include <vector>
#include <assert.h>
#include <unistd.h>
using namespace std;

#define ML MPI_LONG
#define MCW MPI_COMM_WORLD
#define MASTER rank == 0
#define BLOCK MPI_Barrier(MPI_COMM_WORLD)
#define NOW MPI_Wtime()

void print_results(const int do_verify);

const int ROOT = 0;

double p1_start;
double p1_end;

double p2_start;
double p2_end;

double p3_start;
double p3_end;

double p4_start;
double p4_end;

double startup_start;
double startup_end;

double verify_start;
double verify_end;

int p;
long EN;


int main(int argc, char ** argv) {
    if (argc != 4) {
        cout << "MISSING ARGS: EXPECTED 3, RECEIVED: " << argc << endl;
    }

    MPI_Init(&argc, &argv);

    // time since initializations completed
    startup_start = NOW;

    // get args
    const long SEED = strtol(argv[2], nullptr, 10);
    srandom(SEED);
    int rank;
    const long N = strtol(argv[1], nullptr, 10);
    long *original = new long[N];

    EN = N;
    const int do_verify = strtol(argv[3], nullptr, 10);


    MPI_Comm_rank(MCW, &rank);
    MPI_Comm_size(MCW, &p);

    // compute some values
    int p_sqrd = pow(p, 2);
    const int slice_size = N / p;
    int w = N / p_sqrd;

    // compute start indexes and sizes of the slice
    // being send scattered out to the other processes
    int displs[p];
    int scounts[p];
    for (int i = 0; i < p-1; ++i) {
        displs[i] = slice_size * i;
        scounts[i] = slice_size;
    }
    displs[p-1] = slice_size * (p-1);
    scounts[p-1] = N - displs[p-1];

    // generate the random array
    if (MASTER) {
        // Setting up input data
        for (int i = 0; i < N; ++i) {
            original[i] = random();
        }
    }

    // the part of the array this process is going to receive
    long *part = new long[scounts[rank]];

    MPI_Scatterv(original, scounts, displs, ML, part, scounts[rank], ML, ROOT, MCW);

    if (rank != 0) {
        // free up the original array if you aren't the master process
        // as its only used again for verifying correctness by the master
        delete[] original;
    }

    /// START OF PHASE 1
    BLOCK;
    if (MASTER) {
        startup_end = NOW;
        p1_start = NOW;
    }

    // sort local part
    sort(part, part + scounts[rank]);

    // create local sample
    long sample[p];
    for (int i = 0; i < p; ++i) {
        sample[i] = part[i * w];
    }

    long all_samples[p_sqrd];

    // gather samples into master
    MPI_Gather(sample, p, ML, all_samples, p, ML, ROOT, MCW);

    /// END OF PHASE 1
    /// START OF PHASE 2

    // collect pivot values
    long pivots[p-1];
    if (MASTER) {
        p1_end = NOW;
        p2_start = p1_end;
        sort(all_samples, all_samples + p_sqrd);
        int other_p = (p / 2) - 1;

        for (int i = 1; i < p; ++i) {
            pivots[i - 1] = all_samples[(i * p) + other_p];
        }
    }

    // bcast pivots to all processes
    MPI_Bcast(pivots, p-1, ML, ROOT, MCW);

    /// END OF PHASE 2
    if (MASTER) {
        p2_end = NOW;
        p3_start = p2_end;
    }

    /// START OF PHASE 3
    vector<vector<long>> partitions(p);

    int start = 0;
    int end = 0;
    int last_reached = -1;
    for (int i = 0; i < p-1; ++i) {
        while (part[end] <= pivots[i] && end < scounts[rank]) {
            partitions[i].push_back(part[end]);
            ++end;
        }
            last_reached = i;
            start = end;
    }

    for (int i = start; i < scounts[rank]; ++i) {
        partitions[last_reached + 1].push_back(part[i]);
    }

    MPI_Request send_reqs[p];
    MPI_Request recv_reqs[p];
    MPI_Status stats[p];


    // unfortunately more overhead, but the potentially memory waste of having to guess and overallocate the
    // sizes of these partitions could be pretty bad at larger input sizes
    int sizes[p] = { 0 };
    for (int i = 0; i < p; ++i) {
        int s = static_cast<int>(partitions[i].size());
        MPI_Gather(&s, 1, MPI_INT, sizes, 1, MPI_INT, i, MCW);
    }

    long **recp = new long*[p];

    for (int i = 0; i < p; ++i) {
        recp[i] = new long[sizes[i]];
    }

    // trade partitions between all processes
    for (int i = 0; i < p; ++i) {
        MPI_Isend(&partitions[i][0], partitions[i].size(), ML, i, 0, MCW, &send_reqs[i]);
        MPI_Irecv(recp[i], N, ML, i, 0, MCW, &recv_reqs[i]);
    }
    MPI_Waitall(p, recv_reqs, stats);

    /// END OF PHASE 3
    BLOCK;
    if (MASTER) {
        p3_end = NOW;
        p4_start = p3_end;
    }

    /// START OF PHASE 4
    int indexes[p] = { 0 };
    int new_size = 0;
    for (int i = 0; i < p; ++i) {
        new_size += sizes[i];
    }

    /** take the partitions received from
        the other processes and combine
        into a single sorted list through
        a simple k-way merge
    */
    long *new_part = new long[new_size];
    for (int i = 0; i < new_size; ++i) {
        int taken_index = 0;
        long smallest = INT64_MAX;
        long curr;
        for (int j = 0; j < p; ++j) {
            if (indexes[j] < sizes[j]) {
                curr = recp[j][indexes[j]];
                if (curr < smallest) {
                    smallest = curr;
                    taken_index = j;
                }
            }
        }
        indexes[taken_index] += 1;
        new_part[i] = smallest;
    }


    /// END OF PHASE 4
    if (MASTER) {
        p4_end = NOW;
        verify_start = NOW;
    }


    /// VERIFYING CORRECTNESS THE ALGORITHM IS FINISHED
    if (do_verify) {
        for (int i = 0; i < p; ++i) {
            BLOCK;
            if (rank == i) {
                verify_sorted(new_part, new_size);
            }
        }

        int final_sizes[p];
        MPI_Gather(&new_size, 1, MPI_INT, final_sizes, 1, MPI_INT, ROOT, MCW);

        // ensure the sum of elements across all processes
        // is equal to N
        if (MASTER) {
            int total = 0;
            for (int i = 0; i < p; ++i) {
                total += final_sizes[i];
            }
            assert(total == N);
        }

        // gather the partitions from each process
        long *result = new long[N];
        int displs2[p];
        displs2[0] = 0;
        for (int i = 1; i < p; ++i) {
            displs2[i] = final_sizes[i-1] + displs2[i-1];
        }

        MPI_Gatherv(new_part, new_size, ML, result, final_sizes, displs2, ML, ROOT, MCW);

        // do a simple sort on the original data and
        // ensure the original and PSRS sorted versions
        // are identical
        if (MASTER) {
            sort(original, original + N);

            for (int i = 0; i < N; ++i) {
                assert(original[i] == result[i]);
            }
            verify_end = NOW;
        }
        delete[] result;
    }

    if (MASTER)
        print_results(do_verify);

    // cleanup and exit
    for (int i = 0; i < p; ++i) {
        delete[] recp[i];
    }
    delete[] recp;

    if (MASTER)
        delete[] original;

    delete[] part;
    delete[] new_part;


    BLOCK;
    MPI_Finalize();

    return 0;
}


void print_results(const int do_verify) {
    cout << "CORES USED: " << p << endl;
    cout << "INPUT SIZE: " << EN << endl;
    cout << "TOTAL TIME: " << p4_end - p1_start << endl;
    cout << "PHASE ONE TIME: " << p1_end - p1_start << endl;
    cout << "PHASE TWO TIME: " << p2_end - p2_start << endl;
    cout << "PHASE THREE TIME: " << p3_end - p3_start << endl;
    cout << "PHASE FOUR TIME: " << p4_end - p4_start << endl;
    cout << "STARTUP TIME: " << startup_end - startup_start << endl;
    if (do_verify) {
        cout << "VERIFICATION TIME: " << verify_end - verify_start << endl;
    } else {
        cout << "VERIFICATION TIME: " << 0 << endl;
    }

}