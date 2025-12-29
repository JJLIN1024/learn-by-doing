#include <iostream>
#include <string>
#include <algorithm>

using namespace std;
typedef long long ll;

ll nb, ns, nc;
ll pb, ps, pc;
ll rb, rs, rc; // 食譜需要的比例
ll r;

bool check(ll mid) {
    ll cost_b = max(0LL, mid * rb - nb) * pb;
    ll cost_s = max(0LL, mid * rs - ns) * ps;
    ll cost_c = max(0LL, mid * rc - nc) * pc;
    
    return (cost_b + cost_s + cost_c <= r);
}

int main() {
    string s;
    cin >> s;
    for (char c : s) {
        if (c == 'B') rb++;
        else if (c == 'S') rs++;
        else if (c == 'C') rc++;
    }
    
    cin >> nb >> ns >> nc;
    cin >> pb >> ps >> pc;
    cin >> r;

    ll low = 0, high = 1e12 + 200; // 錢最多 10^12，材料最多 100，答案可能略大於 10^12
    ll ans = 0;

    while (low <= high) {
        ll mid = low + (high - low) / 2;
        if (check(mid)) {
            ans = mid;
            low = mid + 1;
        } else {
            high = mid - 1;
        }
    }

    cout << ans << endl;
    return 0;
}