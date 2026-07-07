/*
📖 背景故事 (Background)
你受雇于一家网络安全公司。你们的服务器刚刚拦截到了一批来自未知来源的加密通信数据，这些数据被存储在一个 vector<string> 中。 不幸的是，这批数据里混入了一些被污染的“毒药数据”（比如包含非法数字，或者干脆是空数据），如果你直接对毒药数据进行解密，会导致整个核心系统崩溃。
🎯 你的任务 (Specification)
你需要编写一个健壮的 C++ 程序，逐条过滤并解密这些信息。程序永远不能崩溃，对于毒药数据要能精准拦截、报错，并继续处理下一条数据。
🛠️ 具体要求 (Requirements)
核心函数 1：数据安检员 validateData
参数要求： 接收一个字符串。为了极致性能与安全，必须使用我们刚学的常量引用（const string &）。
逻辑要求：
如果字符串为空（使用 empty()），立刻 throw 一个 runtime_error，提示“拦截到空数据！”。
遍历字符串中的每一个字符（推荐使用范围 for 循环配合 const auto &）。书中第 82 页介绍过 <cctype> 头文件，如果发现字符是数字（使用 isdigit(c)），立刻 throw 一个 runtime_error，提示“拦截到非法数字！”。
核心函数 2：解码引擎 decryptData
参数要求： 接收一个字符串。因为你需要直接在原字符串上进行修改，且不能发生拷贝，请问你要用什么类型的参数？（这里留给你自己思考，不能加 const 了哦）。
逻辑要求：
遍历修改字符串：将所有的小写字母变成大写，大写字母变成小写。（提示：使用 <cctype> 里的 isupper, tolower, toupper）。
将字符的 ASCII 码加 1（即简单的移位密码，比如 'A' 变成 'B'）。
调度中心：main 函数
遍历我为你提供的 vector<string> intercepts。
对于每一条数据，使用 try-catch 结构包裹：
在 try 块中：先调用 validateData 检查，若通过，再调用 decryptData 解密，最后打印出解密后的成功信息。
在 catch 块中：捕获 runtime_error，打印出危险警告以及 err.what() 里的具体报错信息，保证程序不死，继续处理下一条。
统计并打印出“成功解密了多少条数据”（使用 decltype 或 auto 来定义统计变量的类型，感受一下现代 C++）。
*/


#include <iostream>
#include <string>
#include <vector>
#include <stdexcept>
#include <cctype> // 提供 isdigit, isupper, tolower, toupper 等字符操作函数

using namespace std;

void validateData(const string& s);

void decryptData(string& s);

int main() {
    // 这是你拦截到的绝密数据，里面混入了毒药（第2条和第4条）
    vector<string> intercepts = {
        "hELLO wORLD",    // 正常数据
        "sEcREt 123",     // 毒药：包含数字
        "aTtAcK",         // 正常数据
        "",               // 毒药：空字符串
        "zEbRa"           // 正常数据
    };

    int successCount = 0; // 成功计数器

    cout << "--- 系统启动：开始清洗并解密数据 ---" << endl;

    for (auto& s : intercepts) {
        try {
            validateData(s);
            decryptData(s);

            cout << "解密成功: " << s << endl;
            successCount++;
        }

        catch (const runtime_error &err) {
            cout << "警告！跳过毒药数据，原因: " << err.what() << endl;
            continue;
        }
    }

    cout << "--- 处理完毕。成功解密数量: " << successCount << " ---" << endl;

    return 0;
}

void validateData(const string& s) {
    if (s.empty()) {
        throw runtime_error("拦截到空数据！");
    }
    for (auto& c : s) {
        if (isdigit(c)) {
            throw runtime_error("拦截到非法数据！");
        }
    }
}

void decryptData(string& s) {
    for (auto& c : s) {
        if (isupper(c)) {
            c = tolower(c);
        } else {
            c = toupper(c);
        }
        c++;
    }
}