#include <bits/stdc++.h>

using ll = long long;
using namespace std;

const int MOD = 1000000007;

// ll solve_elegant(int n, int limit) {
//     if (limit <= 0) return 0;
    
//     vector<ll> dp(n + 1, 0);
//     dp[0] = 1;
//     ll current_sum = 0;

//     for (int i = 1; i <= n; i++) {
//         current_sum = (current_sum + dp[i - 1]) % MOD;
//         if (i - limit - 1 >= 0) {
//             current_sum = (current_sum - dp[i - limit - 1] + MOD) % MOD;
//         }
//         dp[i] = current_sum;
//     }
//     return dp[n];
// }

// ll solve_multiplication(int n, int limit) {
//     if (limit <= 0) return 0;
//     if (n == 0) return 1;
    
//     vector<ll> dp(n + 1, 0);
//     dp[0] = 1;
//     dp[1] = 1;

//     for (int i = 2; i <= n; i++) {
//         dp[i] = (2 * dp[i - 1]) % MOD;
        
//         if (i - limit - 1 >= 0) {
//             dp[i] = (dp[i] - dp[i - limit - 1] + MOD) % MOD;
//         }
//     }
//     return dp[n];
// }

ll count_ways(int n, int limit) {
    if (limit <= 0) return 0;
    
    vector<ll> dp(n + 1, 0);
    dp[0] = 1;

    for (int i = 1; i <= n; i++) {
        for (int j = 1; j <= limit; j++) {
            if (i - j >= 0) {
                dp[i] = (dp[i] + dp[i - j]) % MOD;
            }
        }
    }
    return dp[n];
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int n, k, d;
    if (!(cin >> n >> k >> d)) return 0;

    ll all_ways = count_ways(n, k);
    ll bad_ways = count_ways(n, d - 1);

    ll ans = (all_ways - bad_ways + MOD) % MOD;
    cout << ans << endl;

    return 0;
}
