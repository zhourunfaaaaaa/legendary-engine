#include <iostream>
#include <vector>
#include <algorithm>

using namespace std;

int main()
{
    vector <int> v1;// 初始创建的vector的大小是0
    cout << "一开始v1大小是" << v1.size() << endl;
    v1.resize(10);// 分配数组大小
    for (int i = 0;i < 10;i++) {
        v1[i] = i;
    }

    cout << "分配数组大小后v1大小是" << v1.size() << endl << "v1:\n";

    for (int i = 0;i < 10;i++) {
        cout << v1[i] << " ";
    }
    
    vector <int> v2(10,2);// 第一个参数是vector的大小,第二个参数是把vector所有元素初始化为2
    cout << "\n一开始v2大小是" << v2.size() << endl;
    cout << "v2:"<< endl;
    for (int i = 0;i < 10;i++) {
        cout << v2[i] << " ";
    }

    vector <int> v3(10);// 如果没有第二个参数,就默认初始化为0
    cout << "\nv3:"<< endl;
    for (int i = 0;i < 10;i++) {
        cout << v3[i] << " ";
    }

    v1.push_back(10);// 在v1后面添加新的数据
    cout << "\n添加一个数据后v1的大小是" << v1.size() << endl << "v1:\n";
    for (auto p = v1.begin();p != v1.end();p++) {
        cout << *p << " ";
    }// 一个常见的迭代器,可以用来遍历这个新的vector

    cout << "\ns1_capacity = " << v1.capacity() << endl;
    // size:实际元素个数
    // capacity:已分配内存可容纳的元素个数
    // size <= capacity始终成立

    // 拷贝构造
    vector <int> v4(v1);
    for (auto p = v4.begin();p != v4.end();p++) {
        cout << *p << " ";
    }
    cout << "\nv4_size = " << v4.size() << endl;

    // 移动构造(类似于剪切)
    vector <int> v5(move(v4));
    for (auto p = v5.begin();p != v5.end();p++) {
        cout << *p << " ";
    }
    if (v4.empty()) {
        cout << "\nv4已经清空\n";// set,vector都可以用empty来判断是否为空
    }

    cout << "v5的第一个元素是" << v5.front() << endl;
    cout << "v5的最后一个元素是" << v5.back() << endl;

    vector <int> v6 = {3,1,2,3,2,1};
    for (auto p = v6.begin();p != v6.end();p++) {
        cout << *p << " ";
    }
    cout << endl;
    // 排序
    sort(v6.begin(),v6.end());
    for (auto p = v6.begin();p != v6.end();p++) {
        cout << *p << " ";
    }
    cout << endl;

    // 去重(需要先排序)
    auto last = unique(v6.begin(),v6.end());
    v6.erase(last,v6.end());
    for (auto p = v6.begin();p != v6.end();p++) {
        cout << *p << " ";
    }
    cout << endl;

    // 示例: v = [3, 1, 2, 1, 3, 2, 1]
    // 
    // 1. 排序后: [1, 1, 1, 2, 2, 3, 3]
    // 
    // 2. unique 将相邻重复元素移到末尾，返回第一个重复位置的迭代器
    //    结果: [1, 2, 3, 2, 2, 3, 3]
    //               ↑ last指向这里
    // 
    // 3. erase 删除 last 到末尾的元素
    //    最终: [1, 2, 3]
    

    // reserve()可以对数组进行预分配,修改的是capacity,size不变
    // emplace_back()是移动构造,push_back是拷贝构造,在int情况下,二者无区别

    // 错误1：越界访问
    // vector<int> v;
    // v[0] = 1;                    // 未定义行为，v是空的

    // 正确：先添加或指定大小
    // v.push_back(1);              // 或 vector<int> v(1);


    // 错误2：使用失效迭代器
    // auto it = v.begin();
    // v.insert(v.begin(), 10);     // it失效
    // *it = 20;                    // 未定义行为


    // 错误3：size_t 和 int 混用
    // for (int i = 0; i < v.size(); i++)  // 可能有符号警告
    // 正确：用 size_t
    // for (size_t i = 0; i < v.size(); i++)


    // 错误4：假设capacity和size一样
    // v.reserve(100);
    // v[50] = 5;                   // 错误！size还是0
    
    return 0;
}