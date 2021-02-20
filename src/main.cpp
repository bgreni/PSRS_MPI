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


const int ROOT = 0;

double phase_one_time;
double phase_two_time;
double phase_three_time;
double phase_four_time;

double p1_start;
double p1_end;

double p2_start;
double p2_end;

double p3_start;
double p3_end;

double p4_start;
double p4_end;

int p;


// BIGGEST ARRAY WE CAN DO IS LIKE 100 MIL

int main(int argc, char ** argv) {

    const long SEED = strtol(argv[2], nullptr, 10);
    srandom(SEED);
    int rank;
    const long N = strtol(argv[1], nullptr, 10);
//    const long N = 36;
    long *original = new long[N];// {16 ,2 ,17, 24, 33, 28, 30, 1, 0, 27, 9, 25, 34, 23, 19, 18, 11, 7, 21, 13, 8, 35, 12, 29, 6, 3, 4, 14, 22, 15, 32, 10, 26, 31, 20, 5};
    MPI_Init(&argc, &argv);

    MPI_Comm_rank(MCW, &rank);
    MPI_Comm_size(MCW, &p);

    int p_sqrd = pow(p, 2);
    const int slice_size = N / p;
    int w = N / p_sqrd;
    int displs[p];
    int scounts[p];
    for (int i = 0; i < p-1; ++i) {
        displs[i] = slice_size * i;
        scounts[i] = slice_size;
    }
    displs[p-1] = slice_size * (p-1);
    scounts[p-1] = N - displs[p-1];

    if (MASTER) {
//        show(&displs[0], p);
//        show(&scounts[0], p);
        // Setting up input data
        for (int i = 0; i < N; ++i) {
//            original[i] = random();
        }
    }

//    for (int i = 0; i < p; ++i) {
//        MPI_Barrier(MCW);
//        if (rank == i) {
//            show(&original[0], N);
//        }
//    }

    long part[scounts[rank]];

    MPI_Scatterv(original, scounts, displs, ML, part, scounts[rank], ML, ROOT, MCW);

    /// START OF PHASE 1
    BLOCK;
    if (MASTER)
        p1_start = NOW;

//    for (int i = 0; i < p; ++i) {
//        BLOCK;
//        if (rank == i) {
//            cout << "PARTS" << endl;
//            show(&part[0], scounts[rank]);
//            cout << "\n\n";
//        }
//    }

    sort(part, part + scounts[rank]);

//    for (int i = 0; i < p; ++i) {
//        BLOCK;
//        if (rank == i) {
//            cout << "PARTS SORTED" << endl;
//            show(&part[0], scounts[rank]);
//            cout << "\n\n";
//        }
//    }

    long sample[p];

    for (int i = 0; i < p; ++i) {
        sample[i] = part[i * w];
    }

    long all_samples[p_sqrd];


    MPI_Gather(sample, p, ML, all_samples, p, ML, ROOT, MCW);

    /// END OF PHASE 1
    /// START OF PHASE 2

    long pivots[p-1];
    if (MASTER) {
        p1_end = NOW;
        p2_start = p2_end;
//        cout << "COMBINED SAMPLE" << endl;
        sort(all_samples, all_samples + p_sqrd);
//        show(&all_samples[0], p_sqrd);
        int other_p = (p / 2) - 1;

        for (int i = 1; i < p; ++i) {
            pivots[i - 1] = all_samples[(i * p) + other_p];
        }
    }

    MPI_Bcast(pivots, p-1, ML, ROOT, MCW);

    /// END OF PHASE 2
    BLOCK;
    if (MASTER) {
        p2_end = NOW;
        p3_start = p2_end;
    }

    /// START OF PHASE 3
    vector<vector<long>> partitions(p);
//    for (int i = 0; i < p; ++i) {
//        // rough estimation of how big each partition will be
//        partitions[i].reserve(slice_size / 3);
//    }

//    for (int i = 0; i < p; ++i) {
//        BLOCK;
//        if (rank == i) {
//            cout << "PARTS SORTED " << rank << endl;
//            show(&part[0], scounts[rank]);
//            cout << "\n\n";
//        }
//    }

    int start = 0;
    int end = 0;
    int last_reached = -1;
    for (int i = 0; i < p-1; ++i) {
        for(;end < scounts[rank]; ++end) {
//            while (part[end] < pivots[i]]) {
//                partitions[i].push_back(part[par])
//            }
            if (part[end] > pivots[i]) {
                for (int j = start; j < end; ++j) {
                    partitions[i].push_back(part[j]);
                }
                last_reached = i;
                start = end;
                break;
            }
        }
    }

    for (int i = start; i < scounts[rank]; ++i) {
        partitions[last_reached + 1].push_back(part[i]);
    }


//    for (int i = 0; i < p; ++i) {
//        BLOCK;
//        if (rank == i) {
//            int tot = 0;
//            for (int j = 0; j < p; ++j) {
////                show(&partitions[j][0], partitions[j].size());
//                tot += partitions[j].size();
//            }
//            cout << tot << endl;
//        }
//    }


//    if (rank == 2) {
//        cout << "RANK 2 SENDING TO RANK 1" << endl;
//        show(&partitions[1][0], partitions[1].size());
//        cout << "\n\n";
//    }

    MPI_Request send_reqs[p];
    MPI_Request recv_reqs[p];
    MPI_Status stats[p];

    // this has to be here for some reason
    BLOCK;


    // unfortunately more overhead, but potentially memory waste of having to guess the
    // sizes of these partitions could be pretty bad at larger input sizes
    int sizes[p] = { 0 };
    for (int i = 0; i < p; ++i) {
        int s = static_cast<int>(partitions[i].size());
//        if (rank == i) cout << s << endl;
//        MPI_Isend(&s, 1, MPI_INT, i, 0, MCW, &send_reqs[i]);
//        MPI_Irecv(&sizes[i], 1, MPI_INT, i, 0, MCW, &recv_reqs[i]);
        MPI_Gather(&s, 1, MPI_INT, sizes, 1, MPI_INT, i, MCW);
    }

//    for (int i = 0; i < p; ++i) {
//        BLOCK;
//        if (rank == i) {
//            int tot = 0;
//            for (int j = 0; j < p; ++j) {
//                tot += sizes[i];
//            }
//            cout << tot << "\n\n\n";
//        }
//    }

//    MPI_Waitall(p, recv_reqs, stats);



    int real_sizes[p];

//    for (int i = 0; i < p; ++i) {
//        BLOCK;
//        if (rank == i) {
//            cout << "SIZES" << endl;
//            for (int i = 0; i < p; ++i) {
//                cout << sizes[i] << " ";
//            }
//            cout << endl;
//            cout << "\n\n";
//        }
//    }


    BLOCK;
    long **recp = new long*[p];

    for (int i = 0; i < p; ++i) {
        recp[i] = new long[sizes[i]];
    }

    BLOCK;
    for (int i = 0; i < p; ++i) {
//        for (int j = 0; j < p; ++j) {
//            BLOCK;
//            if (rank == j) {
//                cout << "RANK: " << rank << " " << i << " " << s << endl;
//                cout << sizes[j] << endl;
//                cout << "\n\n";
//            }
//        }

        MPI_Isend(&partitions[i][0], partitions[i].size(), ML, i, 0, MCW, &send_reqs[i]);
        MPI_Irecv(recp[i], N, ML, i, 0, MCW, &recv_reqs[i]);
    }
    MPI_Waitall(p, recv_reqs, stats);

//
//    long **recp = new long*[p];
//
//    for (int i = 0; i < p; ++i) {
//        recp[i] = new long[sizes[i]];
//    }
//
//    for (int i = 0; i < p; ++i) {
//        for (int j = 0; j < sizes[i]; ++j) {
//            recp[i][j] = recp1[i][j];
//        }
//    }

//    int total_size = 0;
//    for (auto e : partitions) {
//        total_size += e.size();
//    }
//
//    vector<long> all_things(e);
//
//    for (auto e : partitions) {
//        all_things.insert(all_things.end(), e.start(), e.end());
//    }
//
//    int inds[]


    BLOCK;


//    for (int i = 0; i < p; ++i) {
//        for (int j = 0; j < p; ++j) {
//            BLOCK;
//            if (rank == i) {
//                int s = static_cast<int>(partitions[i].size());
//                MPI_Send(&partitions[j][0], s, ML, j, 0, MCW);
//            }
//            else if (rank == j) {
//                MPI_Recv(recp[i], N, ML, i, 0, MCW, &stats[i]);
//            }
//        }
//    }


    for (int i = 0; i < p; ++i) {
        MPI_Get_count(&stats[i], MPI_INT, &real_sizes[i]);
    }

//    if (rank == 1) {
//        cout << "RANK 1 RECIEVED FROM RANK 2" << endl;
//        show(recp[2], sizes[2]);
//        cout << "\n\n";
//    }

//    for (int i = 0; i < p; ++i) {
//        BLOCK;
//        if (rank == i) {
//            //        assert(sizes[i] == real_sizes[i]);
//            cout << "WTF" << endl;
//            show(&sizes[0], p);
//            show(&real_sizes[0], p);
//            sleep(1);
//        }
//    }


//    for (int i = 0; i < p; ++i) {
//        BLOCK;
//        if (rank == i) {
//            cout << "RECEIVED" << endl;
//            for (int j = 0; j < p; ++j) {
//                show(recp[j], sizes[j]);
//            }
//            cout << "\n\n";
//        }
//    }

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

//    for (int i = 0; i < p; ++i) {
//        BLOCK;
//        if (rank == i) {
//            cout << "NEW SIZES" << endl;
//            cout << new_size << endl;
//        }
//    }

    long new_part[new_size];
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
    BLOCK;
    if (MASTER)
        p4_end = NOW;



//    for (int i = 0; i < p; ++i) {
//        MPI_Barrier(MCW);
//        if (rank == i) {
//            cout << "NEW PARTS" << endl;
//            show(&new_part[0], new_size);
//            cout << "\n\n";
//        }
//    }

    for (int i = 0; i < p; ++i) {
        BLOCK;
        if (rank == i) {
            verify_sorted(new_part, new_size);
        }
    }

    int final_sizes[p];
    MPI_Gather(&new_size, 1, MPI_INT, final_sizes, 1, MPI_INT, ROOT, MCW);

    if (MASTER) {
        int total = 0;
        for (int i = 0; i < p; ++i) {
//            cout << final_sizes[i] << endl;
            total += final_sizes[i];
        }
//        cout << total << " " << N << endl;
        assert(total == N);
    }

    long result[N];
    int displs2[p];
    displs2[0] = 0;
    for (int i = 1; i < p; ++i) {
        displs2[i] = final_sizes[i-1] + displs2[i-1];
    }

    MPI_Gatherv(new_part, new_size, ML, result, final_sizes, displs2, ML, ROOT, MCW);

    if (MASTER) {
        sort(original, original + N);

        for (int i = 0; i < N; ++i) {
//            cout << original[i] << " " << result[i] << endl;
            assert(original[i] == result[i]);
        }
    }




    for (int i = 0; i < p; ++i) {
        delete[] recp[i];
    }
    delete[] recp;

    delete[] original;

    BLOCK;
    if (MASTER)
        cout << "done :)" << endl;
    MPI_Finalize();

    return 0;
}


void print_results() {
    cout << "NODES USED: " << p << endl;
}