#include<bits/stdc++.h>

using ll = long long;
using ld = long double;
using namespace std;

int n, m, k;
vector<string> a;
vector<vector<bool>> vis;

int dx[] = {0, 0, 1, -1};
int dy[] = {1, -1, 0, 0};

void dfs(int x, int y) {
    vis[x][y] = true;
    
    for(int i = 0; i < 4; i++) {
        int nx = x + dx[i];
        int ny = y + dy[i];
        
        if(nx >= 0 && nx < n && ny >= 0 && ny < m && !vis[nx][ny] && a[nx][ny] == '.') {
            dfs(nx, ny);
        }
    }
    
    if(k > 0) {
        a[x][y] = 'X';
        k--;
    }
}

void run_case(){
    cin >> n >> m >> k;
    a.resize(n);
    vis.assign(n, vector<bool>(m, false));
    
    int sx = -1, sy = -1;
    for(int i = 0; i < n; i++){
        cin >> a[i];
        for(int j = 0; j < m; j++){
            if(a[i][j] == '.'){
                // 只需要找到任意一個起點即可
                sx = i; 
                sy = j;
            }
        }
    }
    
    if(sx != -1) {
        dfs(sx, sy);
    }
    
    for(int i = 0; i < n; i++){
        cout << a[i] << "\n";
    }
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);
    cout.tie(NULL);

    run_case(); 
    
    return 0;
}