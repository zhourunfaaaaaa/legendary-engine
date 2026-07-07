from cs50 import get_string

sentence = get_string("输入一段英语: ")

letters_cnt = 0
sentences_cnt = 0

for ch in sentence:
    if (ch >= 'a' and ch <= 'z') or (ch >= 'A' and ch <= 'Z'):
        letters_cnt += 1
    elif ch == '.' or ch == '?' or ch == '!':
        sentences_cnt += 1

words_cnt = len(sentence.split()) # split把句子拆成单词列表，再用len获取长度

L = letters_cnt / words_cnt * 100
S = sentences_cnt / words_cnt * 100
rst = round(0.0588 * L - 0.296 * S - 15.8)

if rst < 1:
    print("Before Grade 1")
elif rst >= 16:
    print("Grade 16+")
else:
    print(f"Grade {rst}")