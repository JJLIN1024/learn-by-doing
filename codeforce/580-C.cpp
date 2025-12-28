#include <bits/stdc++.h>

using namespace std;

int n, m;
vector<int> hasCat;
vector<vector<int>> adj;
int ans = 0;

// u: 當前節點, p: 父親節點, current_m: 當前累積的連續貓數
void dfs(int u, int p, int current_m) {
    // 如果當前節點有貓，累積加 1；否則歸零
    if (hasCat[u]) {
        current_m++;
    } else {
        current_m = 0;
    }

    // 如果連續貓數超過 m，直接返回，不再向下搜
    if (current_m > m) return;

    bool isLeaf = true;
    for (int v : adj[u]) {
        if (v != p) {
            isLeaf = false;
            dfs(v, u, current_m);
        }
    }

    // 如果是葉子節點且沒被 return，代表這是一間可到達的餐廳
    // 注意：n=1 時根節點也是葉子，但題目說 n >= 2，所以只需判斷 degree
    if (isLeaf && u != 1) { 
        ans++;
    }
    // 特殊情況：如果根節點本身就是葉子(只有一個節點的情況，本題 n>=2 可不考慮)
}

void run_case() {
    if (!(cin >> n >> m)) return;

    hasCat.assign(n + 1, 0);
    for (int i = 1; i <= n; i++) {
        cin >> hasCat[i];
    }

    adj.assign(n + 1, vector<int>());
    for (int i = 0; i < n - 1; i++) {
        int x, y;
        cin >> x >> y;
        adj[x].push_back(y);
        adj[y].push_back(x);
    }

    // 從根節點 1 開始，父節點設為 -1，初始連續貓數 0
    dfs(1, -1, 0);

    cout << ans << endl;
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);
    run_case();
    return 0;
}