#include<bits/stdc++.h>

using ll = long long;
using ld = long double;
using namespace std;

void run_case(){
    int n;
    cin >> n;
    int firstRow[n];
    int secondRow[n];

    for(int i = 0; i < n; i++) {
        cin >> firstRow[i];
    }

    for(int i = 0; i < n; i++) {
        cin >> secondRow[i];
    }

    int low = firstRow[0];
    int high = low;

    for(int i = 0; i < n; i++) {
        low = min(low, firstRow[i]);
        low = min(low, secondRow[i]);
        high = max(high, firstRow[i]);
        high = max(high, secondRow[i]);
    }

    cout << low * (2 * n - high) << endl;


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
