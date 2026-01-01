#include <bits/stdc++.h>
using namespace std;

using ll = long long;

void run_case() {
    int n, m;
    if (!(cin >> n >> m)) return;

    vector<ll> a(n);
    for (int i = 0; i < n; ++i) {
        cin >> a[i];
    }

    ll G = 0; 
    for (int i = 1; i < n; ++i) {
        G = __gcd(G, abs(a[i] - a[0]));
    }

    for (int j = 0; j < m; ++j) {
        ll b_j;
        cin >> b_j;
        cout << __gcd(a[0] + b_j, G) << (j == m - 1 ? "" : " ");
    }
    cout << endl;
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    run_case();

    return 0;
}