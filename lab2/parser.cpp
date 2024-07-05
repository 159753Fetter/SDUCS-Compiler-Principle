#include "parser.h"
using namespace std;
set<string> se; //符号表查重
int nowtempcnt = 0;
Parser::Parser() {
    terminated = { // 终结符
            "INTSYM", "SPACE", "DOUBLESYM", "SCANFSYM", "PRINTFSYM", "IFSYM", "THENSYM", "WHILESYM", "DOSYM",
            "COMMA", "SEMICOLON", "PLUS", "MINUS", "TIMES", "DIVISION", "EQU", "EQU1", "NOTEQU", "GREAT", "GE", "LESS",
            "LE", "LBRACE",  // (
            "RBRACE",  // )
            "LBRACE1", // {
            "RBRACE1", // }
            "NOT", "AND", "OR", "UFLOAT", "UINT", "IDENT", "END"
    };

    nonterminated = { // 非终结符
            "START", "PROG", "SUBPROG", "M", "N", "VARIABLES", "STATEMENT", "VARIABLE", "T",
            "ASSIGN", "SCANF", "PRINTF", "L", "B",
            "EXPR", "ORITEM", "ANDITEM", "RELITEM", "NOITEM", "ITEM", "FACTOR", "BORTERM", "BANDTERM", "BFACTOR",
            "PLUS_MINUS", "MUL_DIV",
            "REL", "SCANF_BEGIN", "PRINTF_BEGIN", "ID"
    };

    //总程序部分
    P["START"] = {{"PROG"}},
    P["PROG"] = {{"SUBPROG"}},
    P["SUBPROG"] = {{"M", "VARIABLES", "STATEMENT"}},
    P["M"] = {{"SPACE"}},
    P["N"] = {{"SPACE"}},

            //变量声明部分
    P["VARIABLES"] = {{"VARIABLES", "VARIABLE", "SEMICOLON"},
                      {"VARIABLE",  "SEMICOLON"}},
    P["T"] = {{"INTSYM"},
              {"DOUBLESYM"}},
    P["ID"] = {{"IDENT"}},
    P["VARIABLE"] = {{"T",        "ID"},
                     {"VARIABLE", "COMMA", "ID"}},

            //语句部分
    P["STATEMENT"] = {{"ASSIGN"},
                      {"SCANF"},
                      {"PRINTF"},
                      {"SPACE"},
                      {"LBRACE1",  "L", "SEMICOLON", "RBRACE1"},
                      {"WHILESYM", "N", "B",         "DOSYM", "N", "STATEMENT"},
                      {"IFSYM",    "B", "THENSYM",   "N",     "STATEMENT"}},
    P["ASSIGN"] = {{"ID", "EQU", "EXPR"}},
    P["L"] = {{"L", "SEMICOLON", "N", "STATEMENT"},
              {"STATEMENT"}},

            //数值表达式部分
    P["EXPR"] = {{"EXPR", "OR", "ORITEM"},
                 {"ORITEM"}},
    P["ORITEM"] = {{"ORITEM", "AND", "ANDITEM"},
                   {"ANDITEM"}},
    P["ANDITEM"] = {{"NOITEM"},
                    {"NOT", "NOITEM"}},
    P["NOITEM"] = {{"NOITEM", "REL", "RELITEM"},
                   {"RELITEM"}},
    P["RELITEM"] = {{"RELITEM", "PLUS_MINUS", "ITEM"},
                    {"ITEM"}},
    P["ITEM"] = {{"FACTOR"},
                 {"ITEM", "MUL_DIV", "FACTOR"}},
    P["FACTOR"] = {{"ID"},
                   {"UINT"},
                   {"UFLOAT"},
                   {"LBRACE",     "EXPR", "RBRACE"},
                   {"PLUS_MINUS", "FACTOR"}},

            //条件控制的表达式
    P["B"] = {{"B", "OR", "N", "BORTERM"},
              {"BORTERM"}},
    P["BORTERM"] = {{"BORTERM", "AND", "N", "BANDTERM"},
                    {"BANDTERM"}},
    P["BANDTERM"] = {{"LBRACE",  "B",   "RBRACE"},
                     {"NOT",     "BANDTERM"},
                     {"BFACTOR", "REL", "BFACTOR"},
                     {"BFACTOR"}},
    P["BFACTOR"] = {{"UINT"},
                    {"UFLOAT"},
                    {"ID"}},

            //运算符
    P["PLUS_MINUS"] = {{"PLUS"},
                       {"MINUS"}},
    P["MUL_DIV"] = {{"TIMES"},
                    {"DIVISION"}},
    P["REL"] = {{"EQU1"},
                {"NOTEQU"},
                {"LESS"},
                {"LE"},
                {"GREAT"},
                {"GE"}},

            //读写语句
    P["SCANF"] = {{"SCANF_BEGIN", "RBRACE"}},
    P["SCANF_BEGIN"] = {{"SCANF_BEGIN", "COMMA",  "ID"},
                        {"SCANFSYM",    "LBRACE", "ID"}},
    P["PRINTF"] = {{"PRINTF_BEGIN", "RBRACE"}},
    P["PRINTF_BEGIN"] = {{"PRINTFSYM",    "LBRACE", "ID"},
                         {"PRINTF_BEGIN", "COMMA",  "ID"}};
}

void handler(){
    cout << "Syntax Error" << endl;
    exit(0);
}

void Parser::First() {
    for (auto &it: terminated) // 终结符的first就是它自己
        first[it].insert(it);
    for (auto &it: nonterminated) // 非终结符的first要算
        first[it] = set<string>();
    while (1) {
        bool flag = false; // 用来判断每次是否有变化，没有就退出
        for (auto &it: P) {
            auto l = it.first; // 文法左部
            for(auto &item : it.second) {
                auto r = item; // 文法右部
                set<string> result = Getfirst(r);
                if (first[l].size() == 0 && result.size() != 0) {
                    first[l] = result;
                    flag = true;
                } else {
                    for (auto &str: result) {
                        auto res = first[l].insert(str);
                        if (res.second)
                            flag = true;
                    }
                }
            }
        }
        if (!flag) break;
    }
}

set<string> Parser::Getfirst(vector<string> right) {
    set<string> ans;
    int size = right.size();
    if ((right.size() == 1) && (right[0] == "SPACE")) {
        ans.insert("SPACE");
        return ans;
    }
    ans = first[right[0]];
    ans.erase("SPACE");
    bool isEmpty = true;
    for (int i = 1; i < size; i++) {
        if (first[right[i - 1]].find("SPACE") != first[right[i - 1]].end()) {
            ans.insert(first[right[i]].begin(), first[right[i]].end());
            ans.erase("SPACE");
        } else {
            isEmpty = false;
            break;
        }
    }
    if (isEmpty && first[right[size - 1]].find("SPACE") != first[right[size - 1]].end())
        ans.insert("SPACE");
    return ans;
}

set<string> Parser::Getsymbols(const closure &clo) {
    set<string> result;
    for (auto item: clo.item)
        if (item.dot < item.right.size())
            result.insert(item.right[item.dot]);
    return result;
}

// 用于算closure
set<string> Parser::computeFirst(const vector<string> &symbols, size_t position, const vector<string> &lookahead) {
    set<string> result;
    for (size_t i = position; i < symbols.size(); ++i) {
        const string &symbol = symbols[i];
        if (first.find(symbol) == first.end()) { // 终结符
            result.insert(symbol);
            return result;
        } else { // 非终结符
            result.insert(first[symbol].begin(), first[symbol].end());
            if (first[symbol].find("SPACE") == first[symbol].end()) {
                return result;
            }
        }
    }
    for (auto &it: lookahead)
        result.insert(it);
    return result;
}

closure Parser::GenClosure(const closure &I) {
    // 计算单个项目集的闭包
    closure closureSet = I;
    bool flag = true;

    while (flag) {
        flag = false;
        set<expression> newItems;
        for (const auto &item: closureSet.item) {
            if (item.dot < item.right.size()) {
                string B = item.right[item.dot];  // 项目集中一个项目当前即将进行移进的符号
                if (P.find(B) != P.end()) {  // 若在产生式中存在对应的产生式，说明为非终结符号
                    // 计算非终结符号B的First集合，即对于[A->α·Bβ,a]有First(βa)为目标First集合
                    set<string> firstSet = computeFirst(item.right, item.dot + 1, item.lookhead);
                    for (const auto &prod: P.at(B)) {
                        for (const auto &b: firstSet) {
                            expression newItem{B, prod};
                            newItem.dot = 0;
                            newItem.lookhead.push_back(b);
                            // 将[B->·prod,b]加入当前的项目集中(唯一插入)，直到不再有新的项目加入为止
                            if (closureSet.item.find(newItem) == closureSet.item.end()) {
                                newItems.insert(newItem);
                                flag = true;
                            }
                        }
                    }
                }
            }
        }
        closureSet.item.insert(newItems.begin(), newItems.end());
    }
    return closureSet;
}

closure Parser::gotoSet(const closure &I, const string &str) {
    closure Closure;
    for (const auto &item: I.item) {
        // 对于所有的非归约项目，计算出通过X进行移进后的新项目加入新的项目集闭包中
        if (item.dot < item.right.size() && item.right[item.dot] == str) {
            expression newItem = item;
            newItem.dot++;
            Closure.item.insert(newItem);
        }
    }
    // 计算新项目集Closure的闭包
    return GenClosure(Closure);
}

void Parser::GenLR1() {
    // 加入拓广文法S->PROG
    expression expre{"START", {{"PROG"}}};
    expre.dot = 0, expre.lookhead.push_back("END");
    // 构建初始项目
    closure I = GenClosure({{expre}});
    collection.push_back(I);

    bool flag = true;
    while (flag) {
        flag = false;
        vector<pair<closure, string>> newSets;
        // 对于每个项目集，用他们所有的当前待移进符号进行移进产生新的项目集并计算闭包加入规范族中
        int cnt = collection.size();
        for (auto &one_set: collection) {
            for (const auto &symbol: Getsymbols(one_set)) {
                closure gotoResult = gotoSet(one_set, symbol);
                // 将goto得到的新的项目集唯一地加入闭包中
                // 且可知每个已经加入的项目集都是已经确定且唯一的
                auto tep = find(collection.begin(), collection.end(), gotoResult);
                if (!gotoResult.item.empty() && tep == collection.end()) {
                    newSets.push_back({gotoResult, symbol});
                    one_set.next[symbol] = cnt++;
                    flag = true;
                }
                if (!gotoResult.item.empty() && tep != collection.end()) {
                    int t = tep - collection.begin();
                    one_set.next[symbol] = t;
                }
            }
        }
        for (const auto &newSet: newSets)
            collection.push_back(newSet.first);
    }
}

void Parser::GenLR1TABLE() {
    for (int i = 0; i < collection.size(); i++) {
        vector<actionItem> newaction(terminated.size());
        vector<int> newgoto(nonterminated.size(), -1);
        auto set = collection[i].item;
        map<string, int> next = collection[i].next;
        for (auto q = next.begin(); q != next.end(); q++) {
            vector<string>::iterator pos;
            if ((pos = find(nonterminated.begin(), nonterminated.end(), q->first)) != nonterminated.end())
                newgoto[pos - nonterminated.begin()] = q->second;
            else if ((pos = find(terminated.begin(), terminated.end(), q->first)) != terminated.end()) {
                newaction[pos - terminated.begin()].status = ACTION_STATE;
                newaction[pos - terminated.begin()].nextState = q->second;
            }
        }
        for (auto &it: set) {
            if (it.dot == it.right.size()) {
                if (it.left == "START")
                    (newaction.end() - 1)->status = ACTION_ACC;
                else {
                    expression tmpcan = it;
                    for (int j = 0; j < tmpcan.lookhead.size(); j++) {
                        auto pos = find(terminated.begin(), terminated.end(), tmpcan.lookhead[j]);
                        newaction[pos - terminated.begin()].status = ACTION_REDUCTION;
                        newaction[pos - terminated.begin()].l = tmpcan.left;
                        newaction[pos - terminated.begin()].r = tmpcan.right;
                    }
                }
            }
        }
        ACTION[i] = newaction;
        GOTO[i] = newgoto;
    }
}

void symbolTable::enter(string name, int type) {
    if (!se.count(name)) {
        se.insert(name);
        symbols.push_back({name, type, Offset, nextIndex++});
    } else handler();
}

string symbolTable::lookup(string name) {
    if (!se.count(name)) {
        handler();
    } else {
        for(auto &it : symbols)
            if(it.name == name)
                return "TB" + to_string(it.index);
    }
}

int symbolTable::looktype(string name) {
    if (!se.count(name)) {
        handler();
    } else{
        for(auto &it : symbols)
            if(it.name == name)
                return it.type;
    }
}
void QuadrupleGen::gen(std::string op, std::string arg1, std::string arg2, std::string result) {
    quads.push_back({op, arg1, arg2, result});
}

void QuadrupleGen::backpatch(vector<int>&vec, int  quad){
    for(auto &it : vec)
        quads[it].result = to_string(quad);
}

Syntax::Syntax() {
    parser.First();
    parser.GenLR1();
    parser.GenLR1TABLE();
}

void Syntax::analysis(vector<string> input, vector<string> input1) {
    vector<int> state; // 状态栈
    vector<string> Symbol; // 符号栈

    // 初始化
    state.push_back(0);
    Symbol.push_back("END");
    input.push_back("END"),input1.push_back("");
    input.push_back("END"),input1.push_back("");
    string str = "SPACE";
    string st, st1;
    do {
        int pos = find(parser.terminated.begin(), parser.terminated.end(), str) - parser.terminated.begin();
        if(state.empty() || state.back() < 0 || state.back() >= parser.ACTION.size() || pos >= parser.terminated.size()) handler();
        actionItem item = parser.ACTION[state.back()][pos];
        if (item.status == ACTION_ERROR) {
            item.nextState = parser.ACTION[state.back()][find(parser.terminated.begin(), parser.terminated.end(), "SPACE") -
                    parser.terminated.begin()].nextState;
            if (item.nextState != -1) {
                Symbol.push_back("SPACE");
                state.push_back(item.nextState);
                if(state.empty() || state.back() < 0 || state.back() >= parser.ACTION.size() || pos >= parser.terminated.size()) handler();
                item = parser.ACTION[state.back()][pos];
            }
        }
        switch (item.status) {
            case ACTION_ACC:
                return;
            case ACTION_ERROR:
                handler();
            case ACTION_STATE:
                state.push_back(item.nextState);
                Symbol.push_back(str);
                str = *input.begin();
                st1 = st;
                st = *input1.begin();
                input.erase(input.begin());
                input1.erase(input1.begin());
                break;
            case ACTION_REDUCTION: {
                for (int i = item.r.size() - 1; i >= 0; i--) {
                    if (!Symbol.empty() && !state.empty() && Symbol.back() == item.r[i]) {
                        Symbol.pop_back();
                        state.pop_back();
                    } else {
                        handler();
                    }
                }
                Symbol.push_back((item.l));
                GenQuad(item.l,item.r,st1);
                int S = find(parser.nonterminated.begin(), parser.nonterminated.end(), Symbol.back()) - parser.nonterminated.begin();
                if(state.empty() || state.back() < 0 || state.back() >= parser.ACTION.size() || S >= parser.nonterminated.size()) handler();
                state.push_back(parser.GOTO[state.back()][S]);
                break;
            }
        }
    } while(!input.empty() && !state.empty());
}

vector<int> attribute::makelist(int nxq) {
    vector<int> result;
    if(nxq != 0) result.push_back(nxq);
    return result;
}

vector<int> attribute::merge(vector<int> &vec, vector<int>& vec1){
    vector<int> result = vec;
    for(auto it : vec1) result.push_back(it);
    return result;
}

string Syntax::newtemp(string type) {
    int i = nowtempcnt++;
    string str = "T" + to_string(i);
    if(type == "UINT") str += "_i";
    else str += "_d";
    return str;
}

void Syntax::GenQuad(string Left, vector<string> Right, string argv) {
    if(Left == "SUBPROG"){
        if(attrista.empty()) handler();
        attribute att;
        auto t = attrista.top();
        attrista.pop();
        if(attrista.empty()) handler();
        attrista.pop();
        if(attrista.empty()) handler();
        attrista.pop(); // 连续弹出三个
        QuadrupleTable.backpatch(t.nextlist,nxq);
        QuadrupleTable.gen("End","-","-","-");
        nxq++;
        attrista.push(att);
    }
    else if(Left == "M"){
        attribute att;
        SymbolTable.Offset = 0;
        attrista.push(att); // 入栈
    }
    else if(Left == "N"){
        attribute att;
        att.quad = nxq;
        attrista.push(att); // 压入栈中
    }
    else if(Left == "VARIABLES"){
        attribute att;
        if(attrista.empty()) handler();
        attrista.pop();
        if(Right.size() == 3) {
            if(attrista.empty()) handler();
            attrista.pop();
        }
        attrista.push(att);
    }
    else if(Left == "T"){
        attribute att;
        if(Right[0] == "INTSYM"){
            att.type = "UINT";
            att.width = 4;
        }
        else{
            att.type = "UFLAOT";
            att.width = 8;
        }
        attrista.push(att);
    }
    else if(Left == "ID"){
        attribute att;
        att.name = argv;
        attrista.push(att);
    }
    else if(Left == "VARIABLE"){
        attribute att;
        if(attrista.empty()) handler();
        auto t = attrista.top(); // ID
        attrista.pop();
        if(attrista.empty()) handler();
        auto t1 = attrista.top(); // T | variable
        attrista.pop();
        int Type;
        if(t1.type == "UINT") Type = 0;
        else Type = 1;
        SymbolTable.enter(t.name,Type); // 加入符号表
        SymbolTable.Offset += t1.width;
        att.type = t1.type;
        att.width = t1.width;
        attrista.push(att);
    }
    else if(Left == "STATEMENT"){
        if(attrista.empty()) handler();
        attribute att;
        auto t = attrista.top();
        attrista.pop();
        if(Right.size() == 1) {
            att.nextlist = att.makelist(0);
            if(Right[0] == "SPACE") attrista.push(att);
        }
        else if(Right[0] == "LBRACE1")
            att.nextlist = t.nextlist;
        else if(Right[0] == "WHILESYM"){// t statement_1
            if(attrista.empty()) handler();
            auto t1 = attrista.top(); // t1 N_2
            attrista.pop();
            if(attrista.empty()) handler();
            auto t2 = attrista.top(); // t2 B
            attrista.pop();
            if(attrista.empty()) handler();
            auto t3 = attrista.top(); // t3 N_1
            attrista.pop();
            QuadrupleTable.backpatch(t.nextlist, t3.quad);
            QuadrupleTable.backpatch(t2.truelist, t1.quad);
            att.nextlist = t2.faleselist;
            QuadrupleTable.gen("j", "-", "-", to_string(t3.quad));
            nxq++;
        }
        else{
            if(attrista.empty()) handler();
            auto t1 = attrista.top(); // t1 N
            attrista.pop();
            if(attrista.empty()) handler();
            auto t2 = attrista.top(); // t2 B
            attrista.pop();
            QuadrupleTable.backpatch(t2.truelist, t1.quad);
            att.nextlist = att.merge(t2.faleselist, t.nextlist);
        }
        attrista.push(att);
    }
    else if(Left == "ASSIGN"){
        if(attrista.empty()) handler();
        attribute att;
        auto t = attrista.top();
        attrista.pop();
        if(attrista.empty()) handler();
        auto t1 = attrista.top();
        attrista.pop();
        auto p = SymbolTable.lookup(t1.name);
        QuadrupleTable.gen("=",t.place,"-",p);
        nxq++;
        attrista.push(att);
    }
    else if(Left == "L"){
        if(attrista.empty()) handler();
        attribute att;
        auto t = attrista.top();
        attrista.pop();
        if(Right.size() != 1){
            if(attrista.empty()) handler();
            auto t1 = attrista.top();
            attrista.pop();
            if(attrista.empty()) handler();
            auto t2 = attrista.top();
            attrista.pop();
            QuadrupleTable.backpatch(t2.nextlist,t1.quad);
        }
        att.nextlist = t.nextlist;
        attrista.push(att);
    }
    else if(Left == "EXPR"){
        if(attrista.empty()) handler();
        attribute att;
        auto t = attrista.top();
        attrista.pop();
        if(Right.size() == 1){
            att.place = t.place;
            att.type = t.type;
        }
        else{
            if(attrista.empty()) handler();
            auto t1 = attrista.top();
            attrista.pop();
            att.place = newtemp("UINT");
            att.type = "UINT";
            QuadrupleTable.gen("||", t1.place, t.place, att.place);
            nxq++;
        }
        attrista.push(att);
    }
    else if(Left == "ORITEM"){
        if(attrista.empty()) handler();
        attribute att;
        auto t = attrista.top();
        attrista.pop();
        if(Right.size() == 1){
            att.place = t.place;
            att.type = t.type;
        }
        else{
            if(attrista.empty()) handler();
            auto t1 = attrista.top();
            attrista.pop();
            att.place = newtemp("UINT");
            att.type = "UINT";
            QuadrupleTable.gen("&&", t1.place, t.place, att.place);
            nxq++;
        }
        attrista.push(att);
    }
    else if(Left == "ANDITEM"){
        if(attrista.empty()) handler();
        attribute att;
        auto t = attrista.top();
        attrista.pop();
        if(Right.size() == 1){
            att.place = t.place;
            att.type = t.type;
        }
        else{
            att.place = newtemp("UINT");
            att.type = "UINT";
            QuadrupleTable.gen("!",t.place,"-",att.place);
            nxq++;
        }
        attrista.push(att);
    }
    else if(Left == "NOITEM"){
        if(attrista.empty()) handler();
        attribute att;
        auto t = attrista.top(); // RELITEM
        attrista.pop();
        if(Right.size() == 1){
            att.place = t.place;
            att.type = t.type;
        }
        else{
            if(attrista.empty()) handler();
            auto t1 = attrista.top(); // REL
            attrista.pop();
            if(attrista.empty()) handler();
            auto t2 = attrista.top(); // NOITEM1
            attrista.pop();
            att.place = newtemp("UINT");
            att.type = "UINT";
            QuadrupleTable.gen(t1.op, t2.place, t.place, att.place);
            nxq++;
        }
        attrista.push(att);
    }
    else if(Left == "RELITEM"){
        if(attrista.empty()) handler();
        attribute att;
        auto t = attrista.top(); // ITEM
        attrista.pop();
        if(Right.size() == 1){
            att.place = t.place;
            att.type = t.type;
        }
        else{
            if(attrista.empty()) handler();
            auto t1 = attrista.top(); // PLUS_MINUS
            attrista.pop();
            if(attrista.empty()) handler();
            auto t2 = attrista.top(); // RELITEM1
            attrista.pop();
            att.place = newtemp(t2.type);
            att.type = t2.type;
            QuadrupleTable.gen(t1.op, t2.place, t.place, att.place);
            nxq++;
        }
        attrista.push(att);
    }
    else if(Left == "ITEM"){
        if(attrista.empty()) handler();
        attribute att;
        auto t = attrista.top(); // FACTOR
        attrista.pop();
        if(Right.size() == 1){
            att.place = t.place;
            att.type = t.type;
        }
        else{
            if(attrista.empty()) handler();
            auto t1 = attrista.top(); // MUL_DIV
            attrista.pop();
            if(attrista.empty()) handler();
            auto t2 = attrista.top(); // ITEM
            attrista.pop();
            att.place = newtemp(t.type);
            att.type = t.type;
            QuadrupleTable.gen(t1.op, t2.place, t.place, att.place);
            nxq++;
        }
        attrista.push(att);
    }
    else if(Left == "FACTOR"){
        if(attrista.empty()) handler();
        attribute att;
        auto t = attrista.top();
        attrista.pop();
        if(Right.size() == 1){
            if(Right[0] == "ID") {
                att.place = SymbolTable.lookup(t.name);
                auto T= SymbolTable.looktype(t.name);
                if(T == 0) att.type = "UINT";
                else att.type = "UFLOAT";
            }
            else if(Right[0] == "UINT"){
                att.place = newtemp("UINT");
                att.type = "UINT";
                QuadrupleTable.gen("=", argv,"-",att.place);
                nxq++;
                attrista.push(t);
            }
            else{
                att.place = newtemp("UFLOAT");
                att.type = "UFLOAT";
                QuadrupleTable.gen("=", argv,"-",att.place);
                nxq++;
                attrista.push(t);
            }
        }
        else if(Right.size() == 2){ // factor -> plus_minus factor
            if(attrista.empty()) handler();
            auto t1 = attrista.top(); // PLUS_MINUS
            attrista.pop();
            att.place = newtemp(t.type);
            att.type = t.type;
            QuadrupleTable.gen(t1.op, "0", t.place, att.place);
            nxq++;
        }
        else{
            att.place = t.place;
            att.type = t.type;
        }
        attrista.push(att);
    }
    else if(Left == "B"){
        if(attrista.empty()) handler();
        attribute att;
        auto t = attrista.top(); // BORTERM
        attrista.pop();
        if(Right.size() == 1){
            att.truelist = t.truelist;
            att.faleselist = t.faleselist;
        }
        else{
            if(attrista.empty()) handler();
            auto t1 = attrista.top(); // N
            attrista.pop();
            if(attrista.empty()) handler();
            auto t2 = attrista.top(); // B_1
            attrista.pop();
            QuadrupleTable.backpatch(t2.faleselist, t1.quad);
            att.truelist = att.merge(t.truelist, t2.truelist);
            att.faleselist = t.faleselist;
        }
        attrista.push(att);
    }
    else if(Left == "BORTERM"){
        if(attrista.empty()) handler();
        attribute att;
        auto t = attrista.top(); // BANDTERM
        attrista.pop();
        if(Right.size() == 1){
            att.truelist = t.truelist;
            att.faleselist = t.faleselist;
        }
        else{
            if(attrista.empty()) handler();
            auto t1 = attrista.top(); // N
            attrista.pop();
            if(attrista.empty()) handler();
            auto t2 = attrista.top(); // BORTEAM_1
            attrista.pop();
            QuadrupleTable.backpatch(t2.truelist, t1.quad);
            att.faleselist = att.merge(t.faleselist, t2.faleselist);
            att.truelist = t.truelist;
        }
        attrista.push(att);
    }
    else if(Left == "BANDTERM"){
        if(attrista.empty()) handler();
        attribute att;
        auto t = attrista.top();
        attrista.pop();
        if(Right[0] == "LBRACE"){ // t B
            att.truelist = t.truelist;
            att.faleselist = t.faleselist;
        }
        else if(Right[0] == "NOT"){// t bandterm
            att.truelist = t.faleselist;
            att.faleselist = t.truelist;
        }
        else if(Right[0] == "BFACTOR" && Right.size() == 3){// t BFACTOR_2
            if(attrista.empty()) handler();
            auto t1 = attrista.top(); // REL
            attrista.pop();
            if(attrista.empty()) handler();
            auto t2 = attrista.top(); // BFACTOR_1
            attrista.pop();
            att.truelist = att.makelist(nxq);
            att.faleselist = att.makelist(nxq + 1);
            QuadrupleTable.gen("j" + t1.op, t2.place, t.place, "0");
            QuadrupleTable.gen("j", "-", "-", "0");
            nxq += 2;
        }
        else{// t BFACTOR
            att.truelist = att.makelist(nxq);
            att.faleselist = att.makelist(nxq + 1);
            QuadrupleTable.gen("jnz",t.place,"-","0");
            QuadrupleTable.gen("j","-","-","0");
            nxq += 2;
        }
        attrista.push(att);
    }
    else if(Left == "BFACTOR"){
        attribute att;
        if(Right[0] == "UINT"){
            att.place = newtemp("UINT");
            att.type = "UINT";
            QuadrupleTable.gen("=", argv, "-", att.place);
            nxq++;
        }
        else if(Right[0] == "UFLOAT"){
            att.place = newtemp("UFLOAT");
            att.type = "UFLOAT";
            QuadrupleTable.gen("=", argv, "-", att.place);
            nxq++;
        }
        else{
            if(attrista.empty()) handler();
            auto t = attrista.top();
            attrista.pop();
            att.place = SymbolTable.lookup(t.name);
            auto T = SymbolTable.looktype(t.name);
            if (T == 0) att.type = "UINT";
            else att.type = "UFLOAT";
        }
        attrista.push(att);
    }
    else if(Left == "PLUS_MINUS"){
        attribute att;
        if(Right[0] == "PLUS") att.op = "+";
        else att.op = "-";
        attrista.push(att);
    }
    else if(Left == "MUL_DIV"){
        attribute att;
        if(Right[0] == "TIMES") att.op = "*";
        else att.op = "/";
        attrista.push(att);
    }
    else if(Left == "REL"){
        auto t = argv;
        attribute att;
        if(t == "==")
            att.op = "==";
        else if(t == "!=") att.op = "!=";
        else if(t == "<") att.op = "<";
        else if(t == "<=") att.op = "<=";
        else if(t == ">") att.op = ">";
        else if(t == ">=") att.op = ">=";
        attrista.push(att);
    }
    else if(Left == "SCANF_BEGIN"){
        auto p = SymbolTable.lookup(argv);
        if(attrista.empty()) handler();
        attrista.pop();
        if(Right[0] == "SCANF_BEGIN") {
            if(attrista.empty()) attrista.pop();
            QuadrupleTable.gen("R", "-", "-", p);
        }
        else
            QuadrupleTable.gen("R", "-", "-", p);
        nxq++;
        attribute att;
        attrista.push(att);
    }
    else if(Left == "PRINTF_BEGIN"){
        if(attrista.empty()) handler();
        auto p = SymbolTable.lookup(argv);
         attrista.pop();
        if(Right[0] == "PRINTF_BEGIN") {
            if(attrista.empty()) handler();
            attrista.pop();
            QuadrupleTable.gen("W", "-", "-", p);
        }
        else // PRINTF_BEGIN -> printf
            QuadrupleTable.gen("W","-","-",p);
        nxq++;
        attribute att;
        attrista.push(att);
    }
}

void Syntax::showSymbolTable() {
    cout << SymbolTable.symbols.size() << endl;
    for (auto &it: SymbolTable.symbols) // 输出符号表
        cout << it.name << " " << it.type << " " << "null" << " " << it.offset << endl;
}

void Syntax::showQuadrupleTable() {
    int size = QuadrupleTable.quads.size();
    cout << nowtempcnt << endl;
    cout << size << endl;
    for (int i = 0; i < size; i++) {
        auto it = QuadrupleTable.quads[i];
        cout << i << ": " << "(" << it.op << "," << it.arg1 << "," << it.arg2 << "," << it.result << ")" << endl;
    }
}

