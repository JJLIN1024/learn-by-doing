#include<bits/stdc++.h>

using ll = long long;
using namespace std;

int colToNum(string s) {
    int res = 0;
    for (char c : s) {
        res = res * 26 + (c - 'A' + 1);
    }
    return res;
}

string numToCol(int n) {
    string res = "";
    while (n > 0) {
        n--; // 關鍵：因為是 1-based (A=1)，所以要先減 1 對應到 0-25
        res += (char)('A' + (n % 26));
        n /= 26;
    }
    reverse(res.begin(), res.end());
    return res;
}

void run_case() {
    string s;
    cin >> s;

    bool isRXCY = false;
    if (s[0] == 'R' && isdigit(s[1])) {
        for (int i = 1; i < s.length(); i++) {
            if (s[i] == 'C' && isdigit(s[i-1])) {
                isRXCY = true;
                break;
            }
        }
    }

    if (isRXCY) {
        int cPos = s.find('C');
        string rowStr = s.substr(1, cPos - 1);
        string colStr = s.substr(cPos + 1);
        
        int colNum = stoi(colStr);
        
        cout << numToCol(colNum) << rowStr << "\n";
    } else {
        string colStr = "";
        string rowStr = "";
        
        for (char c : s) {
            if (isalpha(c)) {
                colStr += c;
            } else {
                rowStr += c;
            }
        }
        
        int colNum = colToNum(colStr);
        cout << "R" << rowStr << "C" << colNum << "\n";
    }
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int tests;
    cin >> tests;
    while(tests--){
        run_case();
    }
    return 0;
}