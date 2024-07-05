#include <bits/stdc++.h>

using namespace std;
string str;
vector<string> ans, ans1; // ans输出原程序，ans1输出词法分析后的程序
//nvector<string> middle; // 用于记录中间处理过程
int now = 0; // 用于记录处理每一行所在的位置
int state = 0; //当前所处状态
enum status {
    Begin,
    Readletter,
    Readnumber
};

// 关键字
const map<string,string> mp = {
        {"int","INTSYM"},{"double","DOUBLESYM"},{"scanf","SCANFSYM"},
        {"printf","PRINTFSYM"},{"if","IFSYM"},{"then","THENSYM"},
        {"while","WHILESYM"},{"do","DOSYM"}
};

// 符号
const map<string,string> mp1 = {
        {"=","AO"},{"==","RO"},{">","RO"},{">=","RO"},
        {"<","RO"},{"<=","RO"},{"||","LO"},{"&&","LO"},
        {"!","LO"},{"!=","RO"},{"+","PLUS"},{"-","MINUS"},
        {"*","TIMES"},{"/","DIVISION"},{",","COMMA"},{"(","BRACE"},
        {")","BRACE"},{"{","BRACE"},{"}","BRACE"},{";","SEMICOLON"}
};

// 用于处理注释
void process() {
    // 去除 //
    while (str.find("//") != -1) {
        int start = str.find("//");
        int end = 0;
        for (int i = start + 2; i < str.size(); i++) {
            if (str[i] == '\n') {
                end = i;
                break;
            }
        }
        str.erase(start, end - start);
    }
    // 去除 /**/
    while (str.find("/*") != -1) {
        int start = str.find("/*");
        int end = str.find("*/");
        if (end != -1)str.erase(start, end - start + 2);
        else {
            str.erase(start, str.size() - start);
            str += '\n';//如果只有/*，而没有*/，在最后加一个换行符
        }
    }

    for (int i = 0; i < str.size(); i++)
        if (str[i] == '\t' || str[i] == '\r' || str[i] == '\n')
            str[i] = ' ';
}

// 判断是否是字符
bool IsLetter(char c) {
    if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_') return true;
    else return false;
}

// 判断是否是数字
bool IsDigit(char c) {
    if (c >= '0' && c <= '9') return true;
    else return false;
}

// 判断是否有可能是单独符号
bool Single(char c) {
    if (c == '!' || c == '(' || c == ')' || c == '{' || c == '}' || c == ';' || c == ',' || c == '/' || c == '+' ||
        c == '-' || c == '*' || c == '>' || c == '<' || c == '=')
        return true;
    else return false;
}

string GetString() {
    state = 0; // 记录当前所在状态
    string token; // 用于记录解析的每一个字符串
    while (now < str.size()) {// 一直不断循环判断
        switch (state) {
            case Begin: // 每一个初始状态
                if (IsLetter(str[now])) { // 读字符
                    token += str[now];
                    state = Readletter;
                } else if (IsDigit(str[now]) || str[now] == '.') { // 读数字
                    token += str[now];
                    state = Readnumber;
                } else if (str[now] == '>' || str[now] == '<' || str[now] == '!' || str[now] == '=') {
                    token += str[now];
                    now++;
                    if (str[now] == '=') {
                        token += str[now];
                        now++;
                    }
                    return token;
                } else if (str[now] == '=' || str[now] == '&' || str[now] == '|') { // 这三个符号是可以重复被定义的
                    token += str[now];
                    now++;
                    if (str[now - 1] == str[now]) {
                        token += str[now];
                        now++;
                    }
                    return token;
                } else if (Single(str[now])) {
                    token += str[now];
                    now++;
                    return token;
                } else {
                    cout << "Unrecognizable characters.";
                    exit(0);
                }
                now++;
                break;
            case Readletter: // 读字母
                if (IsLetter(str[now]) || IsDigit(str[now])) // 如果还是字母或者数字，接下往下读
                    token += str[now];
                else
                    return token;
                now++;
                break;
            case Readnumber:
                if (IsDigit(str[now]) || str[now] == '.') // 判读是否是数字或者.
                    token += str[now];
                else
                    return token;
                now++;
            default:
                break;
        }
    }
    return token;
}

void final() {
    while (now < str.size()) {
        while(str[now] == ' ' && now < str.size())// 去掉空格
            now++;
        auto tep = GetString();

        if (tep.size() != 0 && tep[0] != ' ') {
            if (mp.find(tep) != mp.end()) {// 说明它是关键字
                ans.push_back(tep);
                ans1.push_back(mp.at(tep));
            } else if (mp1.find(tep) != mp1.end()) { // 说明它是符号
                ans.push_back(tep);
                ans1.push_back(mp1.at(tep));
            } else { // 说明是标识符或者数字
                if (tep.size() != 0 && tep[0] == '.') {// 说明第一个字符是小数点
                    cout << "Malformed number: Decimal point at the beginning or end of a floating point number.";
                    exit(0);
                } else if (tep.size() != 0 && tep[0] >= '0' && tep[0] <= '9') { // 说明是数字
                    int cnt = 0;//用于记录字符串有多少个小数点
                    for (int i = 0; i < tep.size(); i++)
                        if (tep[i] == '.')
                            cnt++;
                    if (cnt == 0) { // 说明是整数
                        if (tep[0] == '0' && tep.size() != 1) { // 说明有前导0
                            cout << "Malformed number: Leading zeros in an integer.";
                            exit(0);
                        }
                        ans.push_back(tep);
                        ans1.push_back("INT");
                    } else if (cnt == 1) { // 说明是小数
                        if (tep.size() >= 2 && tep[0] == '0' && tep[1] != '.') {
                            cout << "Malformed number: Leading zeros in an integer.";
                            exit(0);
                        }
                        if (tep[tep.size() - 1] == '.') {
                            cout << "Malformed number: Decimal point at the beginning or end of a floating point number.";
                            exit(0);
                        }
                        ans.push_back(tep);
                        ans1.push_back("DOUBLE");
                    } else { // 说明有多个小数点
                        cout << "Malformed number: More than one decimal point in a floating point number.";
                        exit(0);
                    }
                } else { // 说明是字母
                    ans.push_back(tep);
                    ans1.push_back("IDENT");
                }
            }
        }
    }
}

// 最后结果
void output() {
    for (int i = 0; i < ans.size(); i++)
        cout << ans[i] << " " << ans1[i] << endl;
}

int main() {
    ios::sync_with_stdio(false);
    string str1;
    while (getline(cin, str1)) { // 不断读入字符
        str += str1; // 存入到str中
        str += '\n';// str末尾置空，否则最后一个元素很有可能无法正常输出
    }
    process();
    final();
    output();
    return 0;
}