#include <iostream>
#include <assert.h>
using namespace std;

void show(long *v, int len) {
    for (int i = 0; i < len; ++i) {
        cout << v[i] << " ";
    }
    cout << endl;
}

void show(int *v, int len) {
    for (int i = 0; i < len; ++i) {
        cout << v[i] << " ";
    }
    cout << endl;
}

bool verify_sorted(long *v, int len) {
    for (int i = 0; i < len - 1; ++i) {
        if (v[i] > v[i+1]) {
            cout << v[i] << " " << v[i+1] << endl << endl;
        }
        assert(v[i] <= v[i+1]);
    }
    return true;
}