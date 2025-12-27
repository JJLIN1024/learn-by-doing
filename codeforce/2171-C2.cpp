#include<bits/stdc++.h>

using ll = long long;
using ld = long double;
using namespace std;

void run_case(){
    int n;
    cin >> n;

    int A[n], B[n];
    int AXor = 0, BXor = 0;
    for(int i = 0; i < n; i++) {
        cin >> A[i];
        AXor ^= A[i];
    }
    for(int i = 0; i < n; i++) {
        cin >> B[i];
        BXor ^= B[i];
    }

    int C = AXor ^ BXor;
    if (C == 0) {
        cout << "Tie" << endl;
        return;
    }

    int msb = 0;
    for(int i = 31; i >= 0 ; i--) {
        if (C & (1 << i)) {
            msb = i;
            break;
        }
    }

    int winnerIndex = 0;
    for(int i = n - 1; i >= 0; i--) {
        int AHasMsb = (A[i] & (1 << msb));
        int BHasMsb = (B[i] & (1 << msb));

        if (AHasMsb != BHasMsb) {
            winnerIndex = i;
            break;
        }
    }

    if (winnerIndex % 2 == 0) {
        cout << "Ajisai" << endl;
    } else {
        cout << "Mai" << endl;
    }

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
