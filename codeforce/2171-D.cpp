#include<bits/stdc++.h>

using ll = long long;
using ld = long double;
using namespace std;

void run_case(){
    int n;
    cin >> n;

    int A[n];
    for(int i = 0; i < n; i++) {
        cin >> A[i];
    }

    int max_val = -1;
    int count = 0;

    for(int i = n - 1; i >= 0; i--) {
        max_val = max(max_val, A[i]);
        count++;

        if(count == max_val && count < n) {
            cout << "No" << endl;
            return;
        }
    }
    cout << "Yes" << endl;
}


int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);
    cout.tie(NULL);

    int tests;
    cin >> tests;
    while(tests--){
        run_case();
    }
    return 0;
}
