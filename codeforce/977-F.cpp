#include<bits/stdc++.h>

using ll = long long;
using ld = long double;
using namespace std;

void run_case(){
    int n;
    cin >> n;
    vector<int> A(n);
    for(int i = 0; i < n; i++) cin >> A[i];

    map<int, int> dp;
    int max_len = 0;
    int last_val = -1;

    for(int i = 0; i < n; i++) {
        int v = A[i];
        dp[v] = dp[v - 1] + 1;
        
        if(dp[v] > max_len) {
            max_len = dp[v];
            last_val = v;
        }
    }

    cout << max_len << "\n";

    vector<int> res;
    int cur_target = last_val - max_len + 1;
    for(int i = 0; i < n; i++) {
        if(A[i] == cur_target) {
            res.push_back(i + 1);
            cur_target++;
        }
    }

    for(int i = 0; i < res.size(); i++) {
        cout << res[i] << (i == res.size() - 1 ? "" : " ");
    }
    cout << endl;
}


int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);
    cout.tie(NULL);

    run_case();

    // int tests;
    // cin >> tests;
    // while(tests--){
    //     run_case();
    // }
    return 0;
}
