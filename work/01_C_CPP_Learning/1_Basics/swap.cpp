#include <iostream>

using namespace std;

void swap(int &a,int &b);

int main()
{
    int a = 2,b = 5;
    cout << "a = " << a << " b = " << b << endl;
    swap(a,b);
    cout << "a = " << a << " b = " << b << endl;

    return 0;
}

void swap(int &a,int &b) {
    int t = b;
    b = a;
    a = t;
}