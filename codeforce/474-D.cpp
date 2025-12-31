#include <bits/stdc++.h>

using ll = long long;
using namespace std;

// 定義常數
const int MAXN = 100005;
const int MOD = 1000000007;

// dp[i] 表示長度為 i 的方法數
// sum[i] 表示長度 1 到 i 的方法數總和 (前綴和)
int dp[MAXN];
int sum[MAXN];

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int t, k;
    if (!(cin >> t >> k)) return 0;


    dp[0] = 1; 

    for (int i = 1; i < MAXN; i++) {

        dp[i] = dp[i-1];

        if (i >= k) {
            dp[i] = (dp[i] + dp[i-k]) % MOD;
        }
    }


    sum[0] = 0;
    for (int i = 1; i < MAXN; i++) {
        sum[i] = (sum[i-1] + dp[i]) % MOD;
    }

    while (t--) {
        int a, b;
        cin >> a >> b;
        
        int ans = (sum[b] - sum[a-1] + MOD) % MOD;
        cout << ans << "\n";
    }

    return 0;
}