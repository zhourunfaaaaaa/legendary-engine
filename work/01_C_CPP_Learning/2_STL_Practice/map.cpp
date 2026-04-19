#include <iostream>
#include <map>

using namespace std;

int main()
{
    map <string,int> m;

    m["hello"] = 4;
    m["world"] = 3;
    m["hallo"] = 5;

    cout << "hello : " << m["hello"] << endl;

    for (auto p = m.begin();p != m.end();p++) {
        cout << p->first << " : " << p->second << endl;
    }// 键值对的遍历,m.begin()这个迭代器实际上是一个结构体指针

    cout << m["hello"] << endl;// "world"存在就返回值
    cout << m["ciallo"] << endl;// "ciallo"不存在就返回0

    // 键值对的排序是根据键的ASCII码的顺序来的
    // 也可以用size()来得到大小
    // unordered_map就是不会自动排序的map,在超时时可以使用
    return 0;
}