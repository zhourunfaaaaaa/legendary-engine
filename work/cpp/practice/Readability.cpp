#include <iostream>
#include <string>
#include <vector>
#include <stdexcept>
#include <cctype>
#include <cmath>

using namespace std;

int computeGrade(const string& text);

int main()
{
    vector<string> texts = {
        "Congratulations! Today is your day. You're off to Great Places! You're off and away!", // 预期: Grade 3
        "Harry Potter was a highly unusual boy in many ways.", // 预期: Grade 5
        "   ", // 毒药数据：全是空格，预期被 catch 捕获
        "As the average number of letters and words pertext increases, the Coleman-Liau index gives the text a higher reading level. If you were to take this paragraph, for instance, which has longer words andtexts than either of the prior two examples, the formula would give the text an twelfth-grade reading level." // 预期: Grade 12
    };

    for (auto& text : texts) {
        try {
            int grade = computeGrade(text);
            if (grade < 1) {
                cout << "Grade 1" << endl;
            } else if (grade >= 16) {
                cout << "Grade 16+" << endl;
            } else {
                cout << "Grade " << grade << endl;
            }
        }
        catch (runtime_error &err) {
            cout << err.what() << endl;
            continue;
        }
    }

    return 0;
}

int computeGrade(const string& text)
{
    int letters = 0, words = 0, sentences = 0;

    if (text.empty()) {
        throw runtime_error("Empty text detected!");
    }

    int is = 0;
    for (auto& c : text) {
        if (isalpha(c)) {
            letters++;
            if (!is) {
                words = 1;
                is = 1;
            }
        } else if (isspace(c)) {
            words++;
        } else if (c == '?' || c == '.' || c == '!') {
            sentences++;
        }
    }

    if (words == 0) {
        throw runtime_error("No valid words found!");
    }

    double L = 1.0 * letters / words * 100;
    double S = 1.0 * sentences / words *100;
    double index = 0.0588 * L - 0.296 * S - 15.8;

    return round(index);
}