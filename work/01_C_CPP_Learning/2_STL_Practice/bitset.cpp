#include <iostream>
#include <bitset>

using namespace std;

int main()
{
    // bitset类似一个字符数组,以二进制低位到高位依次为b[0],b[1]...
    bitset <5> b(19);// 5是大小,19是b二进制转十进制的大小

    cout << b << endl;

    for (int i = 0;i < 5;i++) {
        cout << b[i] << " ";
    }
    cout << endl;

    cout << "是否有1: " << b.any() << endl;
    cout << "是否不存在1: " << b.none() << endl;
    cout << "1的个数: " << b.count() << endl;
    
    for (int i = 0;i < 5;i++) {
        cout << "下标为 " << i << " 的元素是不是1: " << b.test(i) << endl;
    }

    cout << b << endl;
    b.reset(0);// 第0个元素归零
    cout << b << endl;
    b.reset();// 全部元素归零
    cout << b << endl;

    b.flip();// 全部取反
    cout << b << endl;
    b.flip(0);// 第0个元素取反
    cout << b << endl;
    
    return 0;
}