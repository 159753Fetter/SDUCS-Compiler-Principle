#include "parser.h"
using namespace std;
string str;
vector<string> ans, ans1; // ans输出原程序，ans1输出词法分析后的程序
int now = 0; // 用于记录处理每一行所在的位置
int state = 0; //当前所处状态
enum status {
    Begin,
    Readletter,
    Readnumber
};

// 关键字
const map<string, string> mp = {
        {"int",    "INTSYM"},
        {"double", "DOUBLESYM"},
        {"scanf",  "SCANFSYM"},
        {"printf", "PRINTFSYM"},
        {"if",     "IFSYM"},
        {"then",   "THENSYM"},
        {"while",  "WHILESYM"},
        {"do",     "DOSYM"}
};

// 符号
const map<string, string> mp1 = {
        {",",  "COMMA"},
        {";",  "SEMICOLON"},
        {"+",  "PLUS"},
        {"-",  "MINUS"},
        {"*",  "TIMES"},
        {"/",  "DIVISION"},
        {"=",  "EQU"},
        {"==", "EQU1"},
        {"!=", "NOTEQU"},
        {">",  "GREAT"},
        {">=", "GE"},
        {"<",  "LESS"},
        {"<=", "LE"},
        {"||", "OR"},
        {"&&", "AND"},
        {"!",  "NOT"},
        {"(",  "LBRACE"},
        {")",  "RBRACE"},
        {"{",  "LBRACE1"},
        {"}",  "RBRACE1"}
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
        while (str[now] == ' ' && now < str.size())// 去掉空格
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
                if (tep.size() != 0 && tep[0] >= '0' && tep[0] <= '9') { // 说明是数字
                    int cnt = 0;//用于记录字符串有多少个小数点
                    for (int i = 0; i < tep.size(); i++)
                        if (tep[i] == '.')
                            cnt++;
                    if (cnt == 0) { // 说明是整数
                        ans.push_back(tep);
                        ans1.push_back("UINT");
                    } else if (cnt == 1) { // 说明是小数
                        double d = atof(tep.c_str());
                        ans.push_back(to_string(d));
                        ans1.push_back("UFLOAT");
                    }
                } else { // 说明是字母
                    ans.push_back(tep);
                    ans1.push_back("IDENT");
                }
            }
        }
    }
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
    Syntax syn; // 用来进行分析
    syn.analysis(ans1,ans);
    syn.showSymbolTable();
    syn.showQuadrupleTable();
    return 0;
}
