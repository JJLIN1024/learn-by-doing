#include<bits/stdc++.h>

using ll = long long;
using ld = long double;
using namespace std;

void run_case(){
    int n;
    cin >> n;
    vector<ll> A(1e5 + 1, 0);
    for(int i = 0; i < n; i++) {
        int t;
        cin >> t;
        A[t] += t;
    }

    for(int i = 2; i < 1e5 + 1; i++) {
        A[i] = max(A[i - 2] + A[i], A[i - 1]);
    }

    cout << A[100000] << endl;
}


int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);
    cout.tie(NULL);

    // int tests;
    // cin >> tests;
    // while(tests--){
    //     run_case();
    // }
    run_case();
    return 0;
}
