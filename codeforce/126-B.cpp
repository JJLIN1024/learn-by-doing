#include<bits/stdc++.h>

using ll = long long;
using ld = long double;
using namespace std;

void run_case(){
    string s;
    cin >> s;

    int n = s.length();
    if (n < 3) {
        cout << "Just a legend" << endl;
        return;
    }

    vector<int> next(n, 0); // prefix
    for(int i = 1; i < n; i++) {
        int j = next[i - 1];
        // fallback
        while(j > 0 && s[i] != s[j]) {
            j = next[j - 1];
        }
        if (s[i] == s[j]) {
            j++;
        }
        next[i] = j;
    }

    int max_mid = 0;
    for(int i = 1; i < n - 1; i++) {
        max_mid = max(max_mid, next[i]);
    }

    int curr = next[n - 1];
    while(curr > 0) {
        if (curr <= max_mid) {
            cout << s.substr(0, curr) << endl;
            return;
        }
        curr = next[curr - 1];
    }
    cout << "Just a legend" << endl;
}


int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);
    cout.tie(NULL);
    run_case();
    return 0;
}
