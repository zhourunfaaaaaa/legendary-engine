#include <iostream>

using namespace std;

int main()
{
    string s1 = "hello";
    string s2 = "world!";
    string s3 = s1 + s2;
    cout << s3 << endl;
    string s4;
    getline(cin,s4);// 用cin无法读取空格，如hello world只能读hello
    cout << s4 << " 这个字符串的长度是" << s4.length() << endl;
    string s1_sub1 = s1.substr(1,2);// 从第一个字符开始的两个字符组成的子串
    string s1_sub2 = s1.substr(2);// 第二个字符后的子串
    // 注意在s1中h是第0个字符！
    cout << "s1_sub1 = " << s1_sub1 << endl;
    cout << "s1_sub2 = " << s1_sub2 << endl;

    return 0;
}