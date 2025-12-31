#include<bits/stdc++.h>

using ll = long long;
using ld = long double;
using namespace std;

void run_case(){
    int n;
    cin >> n;

    if (n < 4) {
        cout << -1 << endl;
        return;
    }

    for (int i = (n % 2 == 0 ? n - 1 : n); i >= 1; i -= 2) {
        cout << i << " ";
    }

    cout << 4 << " " << 2 << " ";

    for (int i = 6; i <= n; i += 2) {
        cout << i << " ";
    }

    cout << endl;
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
