#include <iostream>
#include <vector>
#include <string>
#include <sstream>
#include <cctype>
// remove_if 在 <algorithm> 里
#include <algorithm>

using namespace std;

// 将输入的一整行按空白切分成单词，连续空格不会产生空字符串
vector<string> split_words(const string &line);

// 原地把所有单词转换成小写，所以参数必须是非 const 引用
void to_lowercase(vector<string>& words);

// 判断一个单词是否完全由字母组成
bool is_alpha_word(const string& word);

// 删除长度小于 min_len 的单词，练习 vector::erase 和迭代器失效
void remove_short_words(vector<string>& words, string::size_type min_len);

// 删除包含数字、标点等非字母字符的单词
void remove_non_alpha_words(vector<string>& words);

// 返回最长单词；如果清洗后没有单词，返回空字符串
string longest_word(const vector<string>& words);

int main()
{
    string buffer;
    getline(cin, buffer); // 读取整行文本，不能用 cin >> buffer，否则只能读到第一个单词

    vector<string> words = split_words(buffer);
    auto original_count = words.size(); // 清洗前的单词数量
    
    to_lowercase(words);
    remove_short_words(words, 3);
    remove_non_alpha_words(words);

    cout << "Original word count: " << original_count << endl;
    cout << "Cleaned word count: " << words.size() << endl;
    cout << "The longest word is " << longest_word(words) << endl;
    cout << "Final size: " << words.size() << endl;
    cout << "Final capacity: " << words.capacity() << endl;

    // 输出清洗后保留下来的单词
    for (auto& word : words) {
        cout << word << endl;
    }

    return 0;
}

vector<string> split_words(const string& line)
{
    // istringstream 可以把字符串当成输入流来读
    istringstream iss(line);
    vector<string> words;
    string word;

    while (iss >> word) {
        words.push_back(word);
    }

    return words;
} 

void to_lowercase(vector<string> &words)
{
    // word 必须是引用，否则改到的是副本
    for (auto& word : words) {
        // ch 也必须是引用，否则改不到 string 里的原字符
        for (auto &ch : word) {
            // cctype 函数接收 unsigned char 更稳，避免 char 为负数时出问题
            ch = static_cast<char>(tolower(static_cast<unsigned char>(ch)));
        }
    }
}

bool is_alpha_word(const string& word)
{
    for (const auto& ch : word) {
        // 只要有一个字符不是字母，整个单词就应该被删除
        if (!isalpha(static_cast<unsigned char>(ch))) {
            return false;
        }
    }

    return true;
}

void remove_short_words(vector<string>& words, string::size_type min_len)
{
    // erase-remove_if 写法：
    // remove_if 先把不该删的单词挪到前面，并返回新的逻辑结尾
    // erase 再把 [逻辑结尾, 真正结尾) 这段尾巴删掉，真正改变 vector 的 size
    words.erase(
        // min_len 是外部参数，lambda 里要用它，所以用 [min_len] 按值捕获
        remove_if(words.begin(), words.end(), [min_len](const string& word) {
            return word.size() < min_len;
        }),
        words.end()
    );
}

void remove_non_alpha_words(vector<string>& words)
{
    // 这里不需要捕获外部变量，因为 is_alpha_word 是普通函数，可以直接调用
    words.erase(
        remove_if(words.begin(), words.end(), [](const string& word) {
            return !is_alpha_word(word);
        }),
        words.end()
    );
}

string longest_word(const vector<string>& words)
{
    // 空 vector 不能解引用 begin()
    if (words.empty()) {
        return "";
    }

    auto longest = words.begin();
    for (auto it = words.begin(); it != words.end(); ++it) {
        if (it->size() > longest->size()) {
            longest = it;
        }
    }

    return *longest;
}
