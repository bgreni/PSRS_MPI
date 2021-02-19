#include <iostream>
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