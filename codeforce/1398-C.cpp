#include<bits/stdc++.h>

using ll = long long;
using ld = long double;
using namespace std;

void run_case() {
    int n;
    cin >> n;
    string s;
    cin >> s;

    map<int, ll> counts;
    counts[0] = 1;

    ll sum = 0;
    ll res = 0;
    for (int j = 1; j <= n; j++) {
        sum += (s[j-1] - '0');
        int val = sum - j; 
        res += counts[val];
        counts[val]++;
    }
    cout << res << "\n";
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
