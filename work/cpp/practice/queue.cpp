#include <iostream>
#include <queue>

using namespace std;

int main()
{
    queue <int> q;
    q.push(1);
    q.push(2);
    q.push(3);
    q.push(4);

    cout << "q的第一个元素是:" << q.front() << endl;
    cout << "q的最后一个元素是:" << q.back() << endl;

    q.pop();
    cout << "现在q的第一个元素是:" << q.front() << endl;

    return 0;
}