#include<bits/stdc++.h>

using ll = long long;
using ld = long double;
using namespace std;

void run_case(){
    ll x, y, k;
    cin >> x >> y >> k;

    if (y == 1) {
        cout << -1 << endl;
        return;
    }

    const ll limit = 1e12;
    if (k > limit) {
        cout << -1 << endl;
        return;
    }

    ll curr_pos = k;

    for(int i = 0; i < x ;i++) {
        ll holes = (curr_pos - 1) / (y - 1);

        if (limit - holes < curr_pos) {
            cout << -1 << endl;
            return;
        }
        
        curr_pos += holes;
    }

    cout << curr_pos << endl;

}


int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);
    cout.tie(NULL);
    // int tests = 1;
    int tests;
    cin >> tests;
    while(tests--){
        run_case();
    }
    return 0;
}
