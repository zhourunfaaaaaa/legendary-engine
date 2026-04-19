#include <iostream>
#include <set>

using namespace std;

int main()
{
    set <int> s1;
    // set是一个集合,元素具有互异性(唯一性)
    // 自动按照从小到大排序
    // 元素一旦插入，不能直接修改(因为修改可能破坏排序)

    s1.insert(1);
    s1.insert(2);
    s1.insert(3);
    s1.insert(1);// 重复插入无效

    for (auto p = s1.begin();p != s1.end();p++) {
        cout << *p << " ";
    }
    cout << endl;

    // 查找元素(可以访问)
    auto p = s1.find(2),q = s1.find(4);
    if (q != s1.end()) {
        cout << "找到了" << *p << endl;
    } else {
        cout << "没找到2\n";
    }
    if (s1.find(4) != s1.end()) {
        cout << "找到了" << *q << endl;
    } else {
        cout << "找到了4\n";
    }

    // 查找元素是否存在(无法访问)
    if (s1.count(2)) {
        cout << "找到了2\n";
    } else {
        cout << "没找到2\n";
    }

    // 删除元素
    s1.erase(1);
    for (auto p = s1.begin();p != s1.end();p++) {
        cout << *p << " ";
    }
    cout << endl;
    
    if (s1.find(1) != s1.end()) {
        cout << "找到了1\n";
    } else {
        cout << "没找到1\n";
    }

    set <int> s2 = {10,20,30,40,50};

    // 查找第一个 >= 25 的元素
    auto it = s2.lower_bound(25);
    cout << *it << endl;

    // 查找第一个 <= 10 的元素
    it = s2.upper_bound(10);
    cout << *it << endl;

    // 查找第一个 >= 55 的元素
    if (it == s2.end()) {
        cout << "没有找到" << endl;
    }

    // 查找在[25,45]中的元素
    auto begin = s2.lower_bound(25);
    auto end = s2.upper_bound(45);

    for (auto it = begin;it != end;it++) {
        cout << *it << " ";
    }

    // unordered_set就是不会自动排序的set,在超时时可以使用
    return 0;
}