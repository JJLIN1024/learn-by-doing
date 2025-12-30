#include <bits/stdc++.h>

using ll = long long;
using namespace std;

void run_case() {
    int n;
    if (!(cin >> n)) return;
    
    priority_queue<ll, vector<ll>, greater<ll>> pq;
    
    ll current_health = 0;
    int drunk_count = 0;
    
    for (int i = 0; i < n; ++i) {
        ll x;
        cin >> x;
        
        current_health += x;
        drunk_count++;

        if (x < 0) {
            pq.push(x);
        }
        
        while (current_health < 0) {
            if (!pq.empty()) {
                ll worst_potion = pq.top();
                pq.pop();
                
                current_health -= worst_potion;
                drunk_count--;
            }
        }
    }
    
    cout << drunk_count << "\n";
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);
    
    run_case();
    
    return 0;
}