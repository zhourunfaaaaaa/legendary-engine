#include <iostream>
#include <stack>

using namespace std;

int main()
{
    stack <int> s;// 数据先进后出,后进先出
    s.push(1);
    s.push(2);
    s.push(3);

    cout << "s的长度是: " << s.size() << endl;
    cout << s.top() << endl;

    s.pop();
    cout << s.top() << endl;

    return 0;
}