#include <bits/stdc++.h>
using namespace std;

int main() {
    long long X, Y, Z;
    cin >> X >> Y >> Z;

    string S;
    cin >> S;

    long long capsOff = 0;
    long long capsOn = Z;

    for (char c : S) {
        long long typeWithCapsOff = c == 'a' ? X : Y;
        long long typeWithCapsOn = c == 'a' ? Y : X;

        long long nextCapsOff = min(capsOff, capsOn + Z) + typeWithCapsOff;
        long long nextCapsOn = min(capsOn, capsOff + Z) + typeWithCapsOn;

        capsOff = nextCapsOff;
        capsOn = nextCapsOn;
    }

    cout << min(capsOff, capsOn) << '\n';
    return 0;
}
