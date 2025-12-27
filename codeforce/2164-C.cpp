/**
 * Author: JJLIN1024 (via Gemini)
 * Strategy: Two-phase Greedy
 * 1. Process monsters that give swords back (c > 0) sorted by difficulty (b).
 * 2. Process monsters that consume swords (c == 0) sorted by difficulty (b).
 */
#include <bits/stdc++.h>

using ll = long long;
using namespace std;

struct Monster {
    int b; // Health
    int c; // Reward
};

void run_case() {
    int n, m;
    cin >> n >> m;

    multiset<int> swords;
    for (int i = 0; i < n; i++) {
        int a;
        cin >> a;
        swords.insert(a);
    }

    // 暫存怪物的 B 和 C 數值
    vector<int> B(m), C(m);
    for (int i = 0; i < m; i++) cin >> B[i];
    for (int i = 0; i < m; i++) cin >> C[i];

    // 分類怪物
    vector<Monster> renewable;
    vector<Monster> terminal;

    for (int i = 0; i < m; i++) {
        if (C[i] > 0) {
            renewable.push_back({B[i], C[i]});
        } else {
            terminal.push_back({B[i], 0});
        }
    }

    // 排序策略：
    // 對於可再生怪物，優先處理血量低 (容易殺) 的。
    // 如果血量相同，其實順序沒差，但為了穩定性可以讓回報高的先(雖不影響結果)。
    sort(renewable.begin(), renewable.end(), [](const Monster& x, const Monster& y) {
        return x.b < y.b;
    });

    // 對於終結怪物，優先處理血量低 (容易殺) 的，以最大化擊殺數。
    sort(terminal.begin(), terminal.end(), [](const Monster& x, const Monster& y) {
        return x.b < y.b;
    });

    int kills = 0;

    // 階段一：處理可再生怪物 (Renewable)
    for (const auto& monster : renewable) {
        // 在劍池中尋找一把攻擊力 >= 怪物血量(b) 的最小劍
        auto it = swords.lower_bound(monster.b);
        
        if (it != swords.end()) {
            // 找到可以殺的劍
            int current_sword_dmg = *it;
            swords.erase(it); // 移除舊劍
            
            // 獲得新劍 (攻擊力只會持平或變高)
            swords.insert(max(current_sword_dmg, monster.c));
            kills++;
        }
        // 如果找不到劍 (it == end)，表示這隻怪物殺不死，跳過
    }

    // 階段二：處理終結怪物 (Terminal)
    for (const auto& monster : terminal) {
        // 同樣尋找最適合的劍（最弱但剛好能殺死怪物的劍，節省強劍給後面的強怪）
        auto it = swords.lower_bound(monster.b);
        
        if (it != swords.end()) {
            swords.erase(it); // 這把劍用完就沒了
            kills++;
        }
    }

    cout << kills << "\n";
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);
    cout.tie(NULL);

    int tests;
    cin >> tests;
    while (tests--) {
        run_case();
    }
    return 0;
}