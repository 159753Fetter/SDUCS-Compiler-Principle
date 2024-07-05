#include <bits/stdc++.h>
using namespace std;
const int INF = INT_MAX;
struct symbol {
    string name;
    int type;
    string value;
    int offset;
};
struct quad {
    string op;
    string arg1;
    string arg2;
    string result;
};
struct LiveInfor{
    int use = -1; // 后续引用点
    int live = -1; // 活跃信息
};
vector<symbol> Symbol; // 符号表
vector<quad> Quad; // 四元式表
vector<pair<int,int>> basicblock; // 基本块
vector<int> vec_temp; // 变量的偏移量
vector<int> IsLabel; // 标识是否为变量
vector<vector<string>> result; //储存结果
vector<string> Allregister = {"R0","R1","R2"};
vector<array<LiveInfor, 3>> LiveTable;  // 用来记录每条四元式活跃信息
map<string, set<string>> Aval, Rval;
map<string,int> Vari;
vector<pair<string,string>> Instr = {
        { "+", "add" }, { "-", "sub" }, { "*", "mul" }, { "/", "div" },
        { "==", "cmp;sete" }, { "!=", "cmp;setne" }, { "<", "cmp;setl" },
        { "<=", "cmp;setle" }, { ">", "cmp;setg" }, { ">=", "cmp;setge" },
        { "&&", "and" }, { "||", "or" }, { "!", "not" }
};
vector<pair<string,string>> Tran = {
        {"j==", "je"}, {"j!=", "jne"}, {"j<",  "jl"}, {"j<=", "jle"},
        {"j>",  "jg"}, {"j>=", "jge"}
};
int symbolcnt; // 符号表个数
int tempcnt; // 临时变量个数
int Quadcnt; // 四元式个数
int Offset; // 总体偏移量
string str, no, input;
void Genblock() {
    int size = Quad.size();
    vector<int> vec; // 基本块入口数组,值为1表示是基本块入口
    vec.resize(Quad.size(),0);
    vec[0] = 1; // 第一条四元式是入口
    for (int i = 0; i < size; i++) {
        auto t = Quad[i];
        if (t.op[0] == 'j' && t.op.size() > 1) {
            int tep = atoi(t.result.c_str());
            vec[tep] = 1;
            if (i < size - 1) vec[i + 1] = 1;
        }
        if (t.op == "j") {
            int tep = atoi(t.result.c_str());
            vec[tep] = 1;
        }
        if (t.op == "R") vec[i] = 1;
        if (t.op == "W") vec[i] = 1;
    }
    int i = 0;
    while (i < size) {
        auto t = Quad[i];
        if (vec[i]) { // i是基本块入口
            if (i == size - 1) {
                basicblock.push_back(make_pair(i,i));
                break;
            }
            for (int j = i + 1; j < size; j++) {
                if (vec[j]) {
                    basicblock.push_back(make_pair(i,j-1));
                    i = j;
                    break;
                }
                else if (Quad[j].op[0] == 'j' || Quad[j].op == "End") {
                    basicblock.push_back(make_pair(i,j));
                    i = j + 1;
                    break;
                }
            }
        } else i++;
    }
}

void GenLive(int end, int start){
    vector<LiveInfor> SymbolInfor(symbolcnt, {-1, 1});
    vector<LiveInfor> TempInfor(tempcnt, {-1, 0});
    for (int i = end; i >= start; i--) { // 逆序求解
        array<string, 3> Infor = { Quad[i].arg1, Quad[i].arg2, Quad[i].result};
        for (int j = 2; j >= 0; j--) {
            if (Infor[j][0] == 'T') {
                int index = 0;
                if(Infor[j][1] == 'B') index = stoi(Infor[j].substr(2));
                else index = stoi(Infor[j].substr(1, Infor[j].find('_') - 1));
                auto &vars = Infor[j][1] == 'B' ? SymbolInfor : TempInfor;
                LiveTable[i][j] = vars[index];
                if (j == 2) vars[index] = { -1, 0 };
                else vars[index] = { i, 1 };
            }
        }
    }
}

void GetLive(){
    LiveTable.resize(Quad.size());
    for (auto item : basicblock) GenLive(item.second, item.first);
}

void find(string &Reg){
    for (const auto reg : Allregister) {
        bool flag = true;
        for (const auto Vari : Rval[reg])
            if (!Aval[Vari].count(Vari)) {
                flag = false;
                break;
            }
        if(flag) {
            Reg = reg;
            return;
        }
    }
}

string Genadd(string str){
    int ans;
    if (str[1] == 'B') ans = Symbol[stoi(str.substr(2))].offset;
    else {
        int x = stoi(str.substr(1, str.find('_') - 1));
        if (!vec_temp[x]) {
            Aval[str].insert(str);
            vec_temp[x] = Offset;
            if(str.back() == 'i') Offset += 4;
            else Offset += 8;
        }
        ans = vec_temp[x];
    }
    return "[ebp-" + to_string(ans) + "]";
}

string getReg(quad q, int index){
    if (q.op[0] != 'j' && q.op != "W" && q.op != "R" && q.op != "End") {
        for (auto Reg : Aval[q.arg1]) {
            auto Val = Rval[Reg];
            if (Val.size() == 1 && *Val.begin() == q.arg1)
                if(q.arg1 == q.result || LiveTable[index][0].live == 0)
                    return Reg;
        }
    }
    for (int i = 0; i < 3; i++) {
        string Reg = "R" + to_string(i);
        if (Rval[Reg].empty())
            return Reg;
    }
    string Reg;
    find(Reg);
    if (Reg.empty()) {
        string maxReg;
        int cnt = -1;
        for (const auto& reg : Allregister) {
            int minn = INT_MAX;
            for (auto var : Rval[reg])
                minn = min(minn, Vari[var]);
            if (minn > cnt) {
                maxReg = reg;
                cnt = minn;
            }
        }
        Reg = maxReg;
    }

    for (auto var : Rval[Reg]) {
        if (!Aval[var].count(var) && var != q.result) {
            string instruction = "mov " + Genadd(var) + ", " + Reg;
            result[index].push_back(instruction);
        }
        if (var == q.arg1 || (var == q.arg2 && Rval[Reg].count(q.arg1))) Aval[var] = {var, Reg};
        else Aval[var] = {var};
    }
    Rval[Reg].clear();
    return Reg;
}

string GetReg(string op){
    for(auto item : Aval[op])
        if(item[0] == 'R')
            return item;
    return op;
}

void ReaReg(string vari){
    if (vari[1] != 'B') { // 只处理临时变量
        if (!Aval[vari].empty()) {
            vector<string> Era;
            for (auto& reg : Aval[vari]) {
                if (reg[0] == 'R') { // 是寄存器
                    Rval[reg].erase(vari);
                    Era.push_back(reg);
                }
            }
            for (auto& reg : Era) Aval[vari].erase(reg);
        }
    }
}

void GenInstr(string op,string ag, string ag1, int index){
    for(auto item : Instr){
        if(item.first == op){
            string str = item.second;
            size_t pos = str.find(';');
            if (pos != string::npos) {
                result[index].push_back(str.substr(0, pos) + " " + ag + ", " + ag1);
                result[index].push_back(str.substr(pos + 1) + " " + ag);
            }
            else result[index].push_back(str + " " + ag + ", " + ag1);
            break;
        }
    }
}

void GenMov(int index){
    for (int k = 0; k < Symbol.size(); k++) {
        string str = "TB" + to_string(k);
        if (!Aval[str].empty() && Aval[str].count(str) == 0)
            for (auto Reg: Aval[str])
                if (Reg[0] == 'R'){
                    result[index].emplace_back("mov [ebp-" + to_string(Symbol[k].offset) + "], " + Reg);
                    break;
                }
    }
}

void GenJ(quad tep, int Index){
    string Op1 = tep.arg1;
    string Op2 = tep.arg2;
    for (auto str: Aval[Op1])
        if (str[0] == 'R') {
            Op1 = str;
            break;
        }
    for (auto str: Aval[Op2])
        if (str[0] == 'R') {
            Op2 = str;
            break;
        }
    if (Op1 == tep.arg1) {
        Op1 = getReg(tep, Index);
        result[Index].push_back("mov " + Op1 + ", " + Genadd(tep.arg1));
    }
    if (Op2[0] == 'T') result[Index].push_back("cmp " + Op1 + ", " + Genadd(Op2));
    else result[Index].push_back("cmp " + Op1 + ", " + Op2);
    for(auto t : Tran)
        if(t.first == tep.op){
            result[Index].push_back(t.second + " ?" + tep.result);
            break;
        }
    int islabel = stoi(tep.result);
    IsLabel[islabel] = true;
}

void GetJ(quad tep, int Index, int temp){
    if (tep.op == "j") {
        result[Index].push_back("jmp ?" + Quad[temp].result);
        int islabel = stoi(tep.result);
        IsLabel[islabel] = true;
    }
    else if (tep.op == "jnz") {
        string Op1 = tep.arg1;
        for (auto str: Aval[Op1])
            if (str[0] == 'R') {
                Op1 = str;
                break;
            }
        if (Op1 == tep.arg1) {
            Op1 = getReg(tep, temp);
            result[Index].push_back("mov " + Op1 + ", " + Genadd(tep.arg1));
        }
        result[Index].push_back("cmp " + Op1 + ", 0");
        result[Index].push_back("jne ?" + tep.result);
        int islabel = stoi(tep.result);
        IsLabel[islabel] = true;
    }
    else if (tep.op == "End") result[Index].push_back("halt");
    else if (tep.op[0] == 'j') GenJ(tep,Index);
}

void GenObjectCode(){
    for(auto item : basicblock){
        for(int i = item.first; i <= item.second; i++){
            auto t = Quad[i];
            if(t.op[0] != 'j' && t.op != "W" && t.op != "R" && t.op != "End"){ // 说明它是运算类型的
                if (t.arg1[0] == 'T'){
                    if(LiveTable[i][0].use == -1) Vari[t.arg1] = INF;
                    else Vari[t.arg1] = LiveTable[i][0].use;
                }
                if (t.arg2[0] == 'T') {
                    if(LiveTable[i][1].use == -1) Vari[t.arg2] = INF;
                    else Vari[t.arg2] = LiveTable[i][1].use;

                }
                if (t.result[0] == 'T') {
                    if(LiveTable[i][2].use == -1) Vari[t.result] = INF;
                    else Vari[t.result] = LiveTable[i][2].use;
                }
                string Rz = getReg(t,i); // 分配寄存器
                string temp = GetReg(t.arg1);
                string temp1 = t.arg2;
                if(temp1 != "-") temp1 = GetReg(t.arg2);
                if (temp == Rz) {
                    if (temp1 != "-") { // 一个操作数的情况
                        string val;
                        if (temp1 == t.arg2 && temp1[0] == 'T') val = Genadd(temp1);
                        else val = temp1;
                        GenInstr(t.op, Rz,  val, i);
                    }
                    if (t.op == "!") result[i].push_back("not " + temp);
                    Aval[t.arg1].erase(Rz);
                }
                else {
                    string x;
                    if (temp == t.arg1 && temp[0] == 'T') x = Genadd(temp);
                    else x = temp;
                    result[i].push_back("mov " + Rz + ", " + x);
                    if (temp1 != "-") {
                        string val;
                        if (temp1 == t.arg2 && temp1[0] == 'T') val = Genadd(temp1);
                        else val = temp1;//寄存器
                        GenInstr(t.op, Rz, val, i);
                    }
                }
                if (temp1 == Rz) Aval[t.arg2].erase(Rz);
                Rval[Rz].clear();
                Rval[Rz].insert(t.result);
                Aval[t.result].clear();
                Aval[t.result].insert(Rz);
                ReaReg(t.arg1);
                ReaReg(t.arg2);
                if (LiveTable[i][2].use == -1) Vari[t.result] = INF;
                else Vari[t.result] = LiveTable[i][2].use;

            }
            else if(t.op == "R") result[i].push_back("jmp ?read(" + Genadd(t.result) + ")");
            else if(t.op == "W") result[i].push_back("jmp ?write(" + Genadd(t.result) + ")");
        }

        GenMov(item.second);
        GetJ(Quad[item.second], item.second, item.second);
        for (auto &pair: Rval) pair.second.clear();
        for (auto &pair: Aval) pair.second.clear();
        Vari.clear();
    }
}

void final(){
    IsLabel.resize(Quadcnt,0);
    result.resize(Quadcnt);
    Genblock(); // 产生基本块
    GetLive(); // 求解待用信息
    GenObjectCode(); // 生成目标代码
    for(auto it : basicblock){
        if(IsLabel[it.first]) cout << "?" << it.first << ":" << endl;
        for (int i = it.first; i <= it.second; i++)
            for (auto str : result[i])
                cout << str << endl;
    }
}

signed main() {
    cin >> str;
    if (str == "Syntax") {
        cout << "halt" << endl;
        return 0;
    }
    else symbolcnt = stoi(str);
    str = "";
    for (int i = 0; i < symbolcnt; i++) {
        symbol Sym;
        cin >> Sym.name >> Sym.type >> Sym.value >> Sym.offset;
        Symbol.push_back(Sym);
    }
    cin >> tempcnt >> Quadcnt;
    if(Quadcnt == 1){
        cout << "halt" << endl;
        return 0;
    }
    vec_temp.resize(tempcnt,0);
    auto t = Symbol.back();
    if(!t.type) Offset = t.offset + 4; // 说明是int
    else Offset = t.offset + 8;
    for (int i = 0; i < Quadcnt; i++) {
        cin >> no >> input;
        vector<string> vec;
        quad q;
        for (int k = 1; k < input.size() - 1; k++) {
            if (input[k] == ',') {
                vec.push_back(str);
                str = "";
            } else str += input[k];
        }
        vec.push_back(str);
        str = "";
        q.op = vec[0], q.arg1 = vec[1], q.arg2 = vec[2], q.result = vec[3];
        Quad.push_back(q);
    }
    final();
    return 0;
}