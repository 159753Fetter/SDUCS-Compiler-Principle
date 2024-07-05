#ifndef LAB2_PARSER_H
#define LAB2_PARSER_H
#include <bits/stdc++.h>
using namespace std;
enum ActionStatus {
    ACTION_ERROR, ACTION_STATE, ACTION_REDUCTION, ACTION_ACC
};

// 符号表中的符号
struct symbol {
    string name;
    int type;
    int offset;
    int index; // 处在符号表中的位置
};

class symbolTable {
public:
    void enter(string name, int type); // 往符号表中加入符号
    string lookup(string name);
    int looktype(string name);
    vector<symbol> symbols;
    int nextIndex = 0;
    int Offset = 0; // 记录当前符号表的偏移量
};

// 定义四元式类
struct Quadruple {
    string op;
    string arg1;
    string arg2;
    string result;
    Quadruple(string op, string arg1, string arg2, string result) : op(op), arg1(arg1), arg2(arg2), result(result) {}
};

class QuadrupleGen {
public:
    void gen(string op, string arg1, string arg2, string result);
    void backpatch(vector<int>&vec, int  quad);
    vector<Quadruple> quads;
};

// 每个非终结符的属性
class attribute {
public:
    int quad = 0;
    string name;
    vector<int> nextlist;
    vector<int> truelist;
    vector<int> faleselist;
    string type;
    int width = 0;
    string op;
    string place;
    vector<int> merge(vector<int> &vec, vector<int>& vec1);
    vector<int> makelist(int nxq);
};

class expression { // 文法表达式
public:
    string left;
    vector<string> right;
    int dot; // 加入点
    vector<string> lookhead;
    expression(string Left, vector<string> Right) : left(Left), right(Right) {};

    bool operator==(const expression &c) const {
        if (left == c.left && right == c.right && dot == c.dot && lookhead == c.lookhead)
            return true;
        else
            return false;
    }

    // 重载<操作符以便可以放到set中
    bool operator<(const expression &expre) const {
        if (left != expre.left)
            return left < expre.left;
        if (right != expre.right)
            return right < expre.right;
        if (dot != expre.dot)
            return dot < expre.dot;
        return lookhead < expre.lookhead;
    }
};

class closure {
public:
    set<expression> item; // 项目集规范族的所有产生式
    map<string, int> next; // 记录下跳转
    bool operator==(const closure &other) const {
        return item == other.item;
    }
};

struct actionItem {
public:
    ActionStatus status = ACTION_ERROR;
    int nextState = -1;
    string l;
    vector<string> r;
};

class Parser {
public:
    map<string, set<string>> first;
    vector<string> terminated; // 终结符
    vector<string> nonterminated; // 非终结符
    vector<closure> collection; // 所有项目集规范族
    map<string, vector<vector<string>>> P; // 文法
    map<int, vector<actionItem>> ACTION;
    map<int, vector<int>> GOTO;

    void First(); // 计算First集合
    set<string> Getfirst(vector<string> right); // 获取right集合中的first集合
    set<string> computeFirst(const vector<string> &symbols, size_t position, const vector<string> &lookahead);

    closure GenClosure(const closure &I); // 生成Closure闭包
    void GenLR1(); // 生成LR(1)分析表
    void GenLR1TABLE(); // 建表
    set<string> Getsymbols(const closure &clo); // 获取·后的符号
    closure gotoSet(const closure &I, const string &X); // 对闭包进行移进

    Parser(); // 构造函数
};

class Syntax { // 语法分析
public:
    Syntax();
    void analysis(vector<string> input, vector<string> input1);
    void GenQuad(string Left, vector<string> Right, string argv); // 产生四元式
    string newtemp(string type);
    void showSymbolTable();
    void showQuadrupleTable();
    Parser parser;
    symbolTable SymbolTable; // 符号表
    QuadrupleGen QuadrupleTable; // 四元式表
    stack<attribute> attrista; // 属性栈
    int nxq = 0;
};

#endif //LAB2_PARSER_H