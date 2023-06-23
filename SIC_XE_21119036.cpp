#include<bits/stdc++.h>
using namespace std;
map<string, pair<int, string>> opCode;

vector<vector<string>> instructions;
vector<int> locctr;
vector<int> locctr_pb;
vector<int> progblock;
vector<int> pc_pb;
vector<int> modification;
vector<int> modificationSize;
vector<string> errors;
vector<string> objCode;
vector<string> objProgram;
string programName;
string programLength;
int startAddr=0;
int org_address = -1;
vector<string> assmDir = {"START", "END", "BYTE", "WORD", "RESB", "RESW", "BASE", "EQU", "ORG", "LTORG"};
vector<string> registers = {"A","X","L","B","S","T","F","__","PC","SW"};

class Symbol{
  public:
    string name;
    string parent;
    int addr;
    bool isSym;
    int progBlock;
    bool relative;

    Symbol(string x){
        name=x;
        parent = "";
        isSym = 0;
        relative = 1;
        addr = -1;
        progBlock = 0;
    }

    Symbol(string x, int address){
        name = x;
        isSym = 0;
        parent = "";
        relative = 1;
        addr = address;
        progBlock = 0;
    }
};

class Literal{
  public:
    string name;
    int addr;
    int size;
    bool written;
    int progBlock;
    
    Literal(string x){
        name = x;
        addr = -1;
        progBlock = 0;
        written = 0;
        if(x[0]=='C'){
            size = x.size()-3;
        }
        if(x[0]=='X'){
            size = (x.size()-3)/2;
        }
    }
};

class ProgramBlock{
  public:
    string name;
    int startAddr;
    int length;

    ProgramBlock(string x){
        name = x;
        startAddr = 0;
        length = 0;
    }
};

vector<ProgramBlock> progBlockTable;
vector<Symbol> symTab;
vector<Literal> litTab;

int findinvec(vector<string> vec, string s){
    for(int i=0;i<vec.size();i++){
        if(vec[i]==s){
            return i;
        }
    }
    return -1;
}

int findProgBlock(string s){
    for(int i=0;i<progBlockTable.size();i++){
        if(progBlockTable[i].name == s){
            return i;
        }
    }
    return -1;
}

int findincharvec(vector<char> vec, char s){
    for(int i=0;i<vec.size();i++){
        if(vec[i]==s){
            return i;
        }
    }
    return -1;
}

int hextoint(string s){
    int result=0;
    map<char,int> map_w;
    map_w.insert({'0',0});
    map_w.insert({'1',1});
    map_w.insert({'2',2});
    map_w.insert({'3',3});
    map_w.insert({'4',4});
    map_w.insert({'5',5});
    map_w.insert({'6',6});
    map_w.insert({'7',7});
    map_w.insert({'8',8});
    map_w.insert({'9',9});
    map_w.insert({'A',10});
    map_w.insert({'B',11});
    map_w.insert({'C',12});
    map_w.insert({'D',13});
    map_w.insert({'E',14});
    map_w.insert({'F',15});
    for(int i=s.size()-1;i>=0;i--){
        result = result + pow(16,s.size()-i-1)*map_w[s[i]];
    }
    return result;
}

string inttohex(int x){
    string s = "";
    bool neg;
    int z;
    map<int,char> map_w;
    map_w.insert({0,'0'});
    map_w.insert({1,'1'});
    map_w.insert({2,'2'});
    map_w.insert({3,'3'});
    map_w.insert({4,'4'});
    map_w.insert({5,'5'});
    map_w.insert({6,'6'});
    map_w.insert({7,'7'});
    map_w.insert({8,'8'});
    map_w.insert({9,'9'});
    map_w.insert({10,'A'});
    map_w.insert({11,'B'});
    map_w.insert({12,'C'});
    map_w.insert({13,'D'});
    map_w.insert({14,'E'});
    map_w.insert({15,'F'});
    while(x>0){
        z = x%16;
        s = map_w[z] + s;
        x = x/16;
    }
    if(s==""){
        s = "0";
    }
    return s;
}

string inttohex_param(int x,int n){
    if(x>=0){
        string s = inttohex(x);
        while(s.size()<n){
            s = '0' + s;
        }
        return s;
    }
    else{
        int q = pow(16,n) - 1;
        q = q - abs(x) + 1;
        string s = inttohex(q);
        while(s.size()<n){
            s = 'F' + s;
        }
        return s;
    }
}

string hexsub(string s1, string s2){
    return inttohex(hextoint(s1) - hextoint(s2));
}

string hexadd(string s1, string s2){
    return inttohex(hextoint(s1) + hextoint(s2));
}

bool isNum(string s){
    vector<char> vec = {'0','1','2','3','4','5','6','7','8','9'};
    bool flag=0;
    for(int i=0;i<s.size();i++){
        flag=0;
        for(int j=0;j<vec.size();j++){
            if(s[i]==vec[j]){
                flag=1;
                break;
            }
        }
        if(!flag){
            return flag;
        }
    }
    return flag;
}

int findSymbol(string s){
    for(int i=0;i<symTab.size();i++){
        if(symTab[i].name==s) return i;
    }
    return -1;
}

int findLiteral(string s){
    for(int i=0;i<litTab.size();i++){
        if(litTab[i].name==s){
            return i;
        }
    }
    return -1;
}

void opCodeMapping(){
    opCode.insert({"ADD", make_pair(4,"18")});
    opCode.insert({"ADDF", make_pair(4,"58")});
    opCode.insert({"ADDR", make_pair(2,"90")});
    opCode.insert({"AND", make_pair(4,"40")});
    opCode.insert({"CLEAR", make_pair(2,"B4")});
    opCode.insert({"COMP", make_pair(4,"28")});
    opCode.insert({"COMPF", make_pair(4,"88")});
    opCode.insert({"COMPR", make_pair(2,"A0")});
    opCode.insert({"DIV", make_pair(4,"24")});
    opCode.insert({"DIVF", make_pair(4,"64")});
    opCode.insert({"DIVR", make_pair(2,"9C")});
    opCode.insert({"FIX", make_pair(1,"C4")});
    opCode.insert({"FLOAT", make_pair(1,"C0")});
    opCode.insert({"HIO", make_pair(1,"F4")});
    opCode.insert({"J", make_pair(4,"3C")});
    opCode.insert({"JEQ", make_pair(4,"30")});
    opCode.insert({"JGT", make_pair(4,"34")});
    opCode.insert({"JLT", make_pair(4,"38")});
    opCode.insert({"JSUB", make_pair(4,"48")});
    opCode.insert({"LDA", make_pair(4,"00")});
    opCode.insert({"LDB", make_pair(4,"68")});
    opCode.insert({"LDCH", make_pair(4,"50")});
    opCode.insert({"LDF", make_pair(4,"70")});
    opCode.insert({"LDL", make_pair(4,"08")});
    opCode.insert({"LDS", make_pair(4,"6C")});
    opCode.insert({"LDT", make_pair(4,"74")});
    opCode.insert({"LDX", make_pair(4,"04")});
    opCode.insert({"LPS", make_pair(4,"D0")});
    opCode.insert({"MUL", make_pair(4,"20")});
    opCode.insert({"MULF", make_pair(4,"60")});
    opCode.insert({"MULR", make_pair(4,"98")});
    opCode.insert({"NORM", make_pair(1,"C8")});
    opCode.insert({"OR", make_pair(4,"44")});
    opCode.insert({"RD", make_pair(4,"D8")});
    opCode.insert({"RMO", make_pair(2,"AC")});
    opCode.insert({"RSUB", make_pair(4,"4C")});
    opCode.insert({"SHIFTL", make_pair(2,"A4")});
    opCode.insert({"SHIFTR", make_pair(2,"A8")});
    opCode.insert({"SIO", make_pair(1,"F0")});
    opCode.insert({"SSK", make_pair(4,"EC")});
    opCode.insert({"STA", make_pair(4,"0C")});
    opCode.insert({"STB", make_pair(4,"78")});
    opCode.insert({"STCH", make_pair(4,"54")});
    opCode.insert({"STF", make_pair(4,"80")});
    opCode.insert({"STI", make_pair(4,"D4")});
    opCode.insert({"STL", make_pair(4,"14")});
    opCode.insert({"STS", make_pair(4,"7C")});
    opCode.insert({"STSW", make_pair(4,"E8")});
    opCode.insert({"STT", make_pair(4,"84")});
    opCode.insert({"STX", make_pair(4,"10")});
    opCode.insert({"SUB", make_pair(4,"1C")});
    opCode.insert({"SUBF", make_pair(4,"5C")});
    opCode.insert({"SUBR", make_pair(2,"94")});
    opCode.insert({"SVC", make_pair(2,"B0")});
    opCode.insert({"TD", make_pair(4,"E0")});
    opCode.insert({"TIO", make_pair(1,"F8")});
    opCode.insert({"TIX", make_pair(4,"2C")});
    opCode.insert({"TIXR", make_pair(2,"B8")});
    opCode.insert({"WD", make_pair(4,"DC")});    
}

bool containsArithmetic(string s){
    for(int i=0;i<s.size();i++){
        if(s[i]=='+' or s[i]=='-' or s[i]=='*' or s[i]=='/'){
            return 1;
        }
    }
    return 0;
}

int isExpression(string s){
    vector<string> elements;
    vector<char> operators;
    string s1,s2,res;
    vector<int> values;
    int result = 0;
    int counter;
    int relative_count=0;
    string temp="";
    for(int i=0;i<s.size();i++){
        if(s[i]=='+' or s[i]=='-' or s[i]=='*' or s[i]=='/'){
            elements.push_back(temp);
            operators.push_back(s[i]);
            temp = "";
        }
        else{
            temp = temp + s[i];
        }
    }
    if(temp.size()!=0){
        elements.push_back(temp);
        temp = "";
    }
    if(findSymbol(elements[0])!=-1 and symTab[findSymbol(elements[0])].relative==1){
        relative_count = 1;
    }
    for(int i=0;i<elements.size();i++){
        if(isNum(elements[i]) or findSymbol(elements[i])!=-1){
            if(i>0){
                if(findSymbol(elements[i])!=-1 and symTab[findSymbol(elements[i])].relative==1){
                    if(operators[i]=='*' or operators[i]=='/' or operators[i-1]=='*' or operators[i-1]=='/'){
                        return INT_MAX;
                    }
                    else{
                        if(operators[i-1]=='+'){
                            relative_count++;
                        }
                        else{
                            relative_count--;
                        }
                    }
                }
            }    
        }
        else{
            return INT_MAX;
        }
    }
    if(relative_count<0 or relative_count>1){
        return INT_MAX;
    }
    int index = -1;
    while(findincharvec(operators,'*')!=-1 or findincharvec(operators,'/')!=-1){
        for(int i=0;i<operators.size();i++){
            if(operators[i]=='*' or operators[i]=='/'){
                index = i;
                break;
            }
        }
        if(operators[index]=='*'){
            elements[index] = to_string(stoi(elements[index])*stoi(elements[index+1]));
        }
        else{
            elements[index] = to_string(stoi(elements[index])/stoi(elements[index+1]));
        }
        elements.erase(elements.begin()+index+1);
        operators.erase(operators.begin()+index);
    }
    if(findSymbol(elements[0])==-1){
        result = stoi(elements[0]);
    }
    else{
        result = symTab[findSymbol(elements[0])].addr;
    }
    for(int i=0;i<operators.size();i++){
        // if(i!=0){
            if(operators[i]=='+'){
                if(findSymbol(elements[i+1])==-1){
                    result += stoi(elements[i+1]);
                }
                else{
                    result += symTab[findSymbol(elements[i+1])].addr;
                }
            }
            else{
                if(findSymbol(elements[i+1])==-1){
                    result -= stoi(elements[i+1]);
                }
                else{
                    result -= symTab[findSymbol(elements[i+1])].addr;
                }
            }
    }
    return result;
}

bool absOrRelative(string s){
    vector<string> elements;
    vector<char> operators;
    string s1,s2,res;
    vector<int> values;
    int result = 0;
    int counter;
    int relative_count=0;
    string temp="";
    for(int i=0;i<s.size();i++){
        if(s[i]=='+' or s[i]=='-' or s[i]=='*' or s[i]=='/'){
            elements.push_back(temp);
            operators.push_back(s[i]);
            temp = "";
        }
        else{
            temp = temp + s[i];
        }
    }
    if(temp.size()!=0){
        elements.push_back(temp);
        temp = "";
    }
    if(findSymbol(elements[0])!=-1 and symTab[findSymbol(elements[0])].relative==1){
        relative_count = 1;
    }
    for(int i=0;i<elements.size();i++){
        if(isNum(elements[i]) or findSymbol(elements[i])!=-1){
            if(i>0){
                if(findSymbol(elements[i])!=-1 and symTab[findSymbol(elements[i])].relative==1){
                    if(operators[i]=='*' or operators[i]=='/' or operators[i-1]=='*' or operators[i-1]=='/'){
                        return INT_MAX;
                    }
                    else{
                        if(operators[i-1]=='+'){
                            relative_count++;
                        }
                        else{
                            relative_count--;
                        }
                    }
                }
            }    
        }
        else{
            return INT_MAX;
        }
    }
    if(relative_count==0){
        return 0;
    }
    else{
        return 1;
    }
}

void printSymTab(){
    cout<<"\n\n\tSYMBOL TABLE\n----------------------------------------------------------\n";
    for(int i=0;i<symTab.size();i++){
        cout<<symTab[i].name<<" : "<<inttohex(symTab[i].addr)<<"\n";
    }
    cout<<"-----------------------------------------------------------\n";
}

void printLitTab(){
    cout<<"\n\tLITERAL TABLE\n--------------------------------------------------------------------\n";
    for(int i=0;i<litTab.size();i++){
        cout<<litTab[i].name<<" : "<<litTab[i].size<<" , "<<inttohex(litTab[i].addr)<<"\n";
    }
    cout<<"--------------------------------------------------------------------\n";
}

void printProgBlockTable(){
    cout<<"\n\tPROGRAM BLOCK TABLE\n--------------------------------------------------------------------\n";
    for(int i=0;i<progBlockTable.size();i++){
        cout<<progBlockTable[i].name<<" : "<<inttohex(progBlockTable[i].startAddr)<<" : "<<inttohex(progBlockTable[i].length)<<"\n";
    }
    cout<<"--------------------------------------------------------------------\n";
}

vector<string> getElements(string inst){
    vector<string> el;
    string temp="";
    for(int i=0;i<inst.size();i++){
        if(inst[i]==' ' || inst[i]==','){
            el.push_back(temp);
            temp="";
        }
        else temp=temp+inst[i];
    }
    if(temp!=""){
        el.push_back(temp);
    }
    return el;
}

void getInstructions(){
    string s;
    while(1){
        getline(cin, s);
        vector<string> elements=getElements(s);
        instructions.push_back(elements);
        if(elements[0]=="END") break;
        else if(elements.size()>1 && elements[1]=="END") break;
    }
}

void printListingFile(vector<vector<string>> v){
    cout<<"\n";
    cout<<"\t\t"<<setw(31)<<"LISTING FILE\n\n";
    cout<<"\tADDRESS\t\tINSTRUCTION";
    cout<<setw(39)<<"OBJECT CODE\n";
    for(int i=0;i<v.size();i++){
        int charlength = 0;
        cout<<"\t"<<inttohex(locctr_pb[i])<<"\t";
        cout<<"\t";
        for(int j=0;j<v[i].size();j++){
            cout<<v[i][j];
            charlength += v[i][j].size();
            if(j!=v[i].size()-1){
                cout<<" ";
                charlength++;
            }
        }
        cout<<setw(50-charlength)<<objCode[i]<<"\n";
    }
    cout<<"\n\nLENGTH OF PROGRAM: "<<programLength<<"\n";
    cout<<"---------------------------------------------------------------------------------------------------------------------\n";
}

int firstPass(){
    string error_message;
    programName=instructions[0][0];
    org_address = -1;
    if(programName.size()>6){
        error_message = "error at line 1: NAME OF PROGRAM ENTERED IS MORE THAN 6 COLUMNS!";
        errors.push_back(error_message);
    }
    if(instructions[0].size()>1){
        if(instructions[0][1]!="START"){
            error_message = "error at line 1: START assembler directive not found";
            errors.push_back(error_message);
        }
        else{
            if(instructions[0].size()==3) startAddr=hextoint(instructions[0][2]);
            if(instructions[0].size()>3){
                error_message = "error at line 1: Invalid instruction!";
                errors.push_back(error_message);
                
            }
        }
    }
    else if(instructions[0].size()==1){
        error_message = "error at line 1: Invalid instruction!";
        errors.push_back(error_message);
    }
    //initialise the location counter vectors and the pc vector
    locctr.push_back(startAddr);
    locctr_pb.push_back(startAddr);
    progblock.push_back(0);
    pc_pb.push_back(startAddr);
    //All instructions are mapped to the DEFAULT program block until a USE statement is encountered
    ProgramBlock defBlock("DEFAULT");
    defBlock.startAddr = startAddr;
    progBlockTable.push_back(defBlock);
    int curr_addr = startAddr;
    int prev_addr = startAddr;
    int curr_addr_pb = startAddr;
    int curr_pb=0;
    //We iterate over each instruction of the program
    for(int iterator=1;iterator<instructions.size()-1;iterator++){
        vector<string> inst=instructions[iterator];
        int format_supported;
        int curr_format = 0;
        int counter_literal=0;
        string opcode;
        bool flag=0;
        bool flag_progBlock = 0;
        bool form4 = 0;
        //We iterate over each element of each instruction
        for(int i=0;i<inst.size();i++){
            //flag : Represents whether we have encountered an OPCODE or an ASSEMBLER DIRECTIVE in the instruction yet
            //If there exists a format 1 OPCODE in the inctruction, but there are arguments after the OPCODE
            if(flag and format_supported==1){
                error_message = "error at line " + to_string(iterator+1)+" : INVALID INSTRUCTION FORMAT!";
                errors.push_back(error_message);
            }
            form4=0;
            if(flag and inst[i-1]!="BYTE" and inst[i-1]!="EQU"){
                //For the element after an OPCODE or ASSEMBLER DIRECTIVE except EQU and BYTE
                if(findinvec(registers,inst[i])==-1){
                    //NOT A REGISTER
                    if(inst[i][0]=='#' or inst[i][0]=='@'){
                        if(!isNum(inst[i].substr(1))){
                            //#LABEL or @LABEL
                            if(findSymbol(inst[i].substr(1))==-1){
                                //If we have encountered this symbol/label for the first time
                                Symbol operand(inst[i].substr(1));
                                operand.progBlock = curr_pb;
                                symTab.push_back(operand);
                            }
                        }
                    }
                    else if(inst[i][0]=='='){
                        //LITERALS
                        if(inst[i][1]=='C' or inst[i][1]=='X'){
                            Literal lit(inst[i].substr(1));
                            if(findLiteral(inst[i].substr(1))==-1){
                                //If we have encountered this literal for the first time
                                litTab.push_back(lit);
                            }
                        }
                        else{
                            // cout<<"error at line "<<iterator+1<<": Invalid Literal definition!\n";
                            error_message = "error at line " + to_string(iterator+1)+": Invalid Literal definition!";
                            errors.push_back(error_message);
                        }
                    }
                    else{
                        if(!isNum(inst[i])){
                            //LABEL
                            if(findSymbol(inst[i])==-1){
                                //Encountered the symbol/label for the first time
                                Symbol operand(inst[i]);
                                operand.progBlock = curr_pb;
                                symTab.push_back(operand);
                            }
                        }
                        //No operation required in case of a number
                    }
                }
            }
            if(flag and inst[i-1]=="ORG"){
                //Address after ORG statement
                if(inst.size()!=2){
                    error_message = "error at line " + to_string(iterator+1)+": INVALID ORG STATEMENT!";
                    errors.push_back(error_message);
                }
                else{
                    if(isNum(inst[i])==1){
                        //If absolute address is given in the ORG statement
                        curr_format = stoi(inst[i]) - curr_addr_pb;
                        org_address = curr_addr_pb; //Represents the return address for the ORG statement
                    }
                    else{
                        //If ORG address is represented as a symbol/label
                        if(findSymbol(inst[i])!=-1 and symTab[findSymbol(inst[i])].addr != -1){
                            curr_format = symTab[findSymbol(inst[i])].addr - curr_addr_pb;
                            org_address = curr_addr_pb;
                        }
                        //The symbol/label needs to be already defined otherwise error as forward references are not allowed
                        else{
                            error_message = "error at line " + to_string(iterator+1)+": INVALID ORG STATEMENT AS FORWARD REFERENCE IS NOT ALLOWED!";
                            errors.push_back(error_message);
                        }
                    }
                }
            }
            if(flag and inst[i-1]=="EQU"){
                //EQU statements
                if((i-2)!=0){
                    // cout<<"error at line "<<iterator+1<<": NO SYMBOL NAME!\n";
                    error_message = "error at line " + to_string(iterator+1)+": NO SYMBOL NAME!";
                    errors.push_back(error_message);
                }
                else{
                    //The symbol needs to be already defined otherwise error as forward references are not allowed
                    if(findSymbol(inst[i-2])!=-1 and symTab[findSymbol(inst[i-2])].addr != curr_addr_pb){
                        error_message = "error at line " + to_string(iterator+1)+": Forward reference in the symbol " + inst[i-2]+"!";
                        errors.push_back(error_message);
                    }
                    else{
                        symTab[findSymbol(inst[i-2])].isSym = 1;
                        if(isNum(inst[i])){
                            //If the symbol is defined as an absolute number eg.- SYMBOL1 EQU 1000
                            symTab[findSymbol(inst[i-2])].isSym = 1;
                            symTab[findSymbol(inst[i-2])].relative = 0;
                            symTab[findSymbol(inst[i-2])].addr = stoi(inst[i]);
                            symTab[findSymbol(inst[i-2])].progBlock = curr_pb;
                        }
                        else if(findSymbol(inst[i])!=-1){
                            //Symbol/Label is already present in the symbol table eg.- LABEL WORD 100
                            //                                                         SYMBOL1 EQU LABEL       
                            symTab[findSymbol(inst[i-2])].isSym = 1;
                            symTab[findSymbol(inst[i-2])].parent = inst[i];
                            symTab[findSymbol(inst[i-2])].addr = symTab[findSymbol(inst[i])].addr;
                            symTab[findSymbol(inst[i-2])].relative = symTab[findSymbol(inst[i])].relative;    
                            symTab[findSymbol(inst[i-2])].progBlock = curr_pb;
                        }
                        else{
                            if(inst[i]=="*"){
                                //If the symbol is given the value of current program counter
                                symTab[findSymbol(inst[i-2])].addr = curr_addr_pb;
                                symTab[findSymbol(inst[i-2])].progBlock = curr_pb;
                            }
                            else if(containsArithmetic(inst[i])){
                                //If the symbol is an expression eg.- SYMBOL1 EQU BUFFEND-BUFFER
                                Symbol expression(inst[i]);
                                symTab[findSymbol(inst[i-2])].parent = inst[i];
                                symTab[findSymbol(inst[i-2])].progBlock = curr_pb;
                                symTab.push_back(expression);
                            }
                            else{
                                error_message = "error at line " + to_string(iterator+1)+": Invalid declaration of symbol!";
                                errors.push_back(error_message);
                            }    
                        }
                    }
                }
            }
            if(inst[i]=="USE"){
                //USE statement
                if(inst.size()>2 or inst[0]!="USE"){
                    error_message = "error at line " + to_string(iterator+1)+": Invalid Instruction!";
                    errors.push_back(error_message);
                }
                else{
                    flag_progBlock = 1;
                    if(inst.size()==1){
                        //Return to default program block in case of USE
                        progBlockTable[curr_pb].length += curr_addr_pb - prev_addr;
                        curr_addr_pb = progBlockTable[0].startAddr + progBlockTable[0].length;
                        curr_pb = 0;
                        prev_addr = curr_addr_pb;
                    }
                    else if(findProgBlock(inst[i+1])!=-1){
                        //The program block to use is already defined
                        progBlockTable[curr_pb].length += curr_addr_pb - prev_addr;
                        curr_addr_pb = progBlockTable[findProgBlock(inst[i+1])].startAddr + progBlockTable[findProgBlock(inst[i+1])].length;
                        curr_pb = findProgBlock(inst[i+1]);
                        prev_addr = curr_addr_pb;
                    }
                    else{
                        //The program block to use has been called the first time
                        ProgramBlock block(inst[i+1]);
                        progBlockTable[curr_pb].length += curr_addr_pb - prev_addr;
                        progBlockTable.push_back(block);
                        curr_pb = findProgBlock(inst[i+1]);
                        curr_addr_pb = progBlockTable[findProgBlock(inst[i+1])].startAddr + progBlockTable[findProgBlock(inst[i+1])].length;
                        prev_addr = curr_addr_pb;
                    }
                }
            }
            if(inst[i][0]=='+'){
                //Opcode starting with + refers to format 4
                form4 = 1;
            }
            if((opCode.find(inst[i]) != opCode.end()) or (form4 and (opCode.find(inst[i].substr(1)) != opCode.end()))){
                //In case of OPCODE or +OPCODE
                if(!flag){
                    flag=1;
                }
                else{
                    error_message = "error at line " + to_string(iterator+1);
                    errors.push_back(error_message);
                }
                if(inst[i][0]=='+'){
                    opcode = opCode[inst[i].substr(1)].second;
                    format_supported = opCode[inst[i].substr(1)].first;
                }
                else{
                    opcode = opCode[inst[i]].second;
                    format_supported = opCode[inst[i]].first;
                }
                if(inst[i][0] == '+'){
                    if(format_supported == 4){
                        curr_format = 4;
                    }
                    else{
                        //Incase of + used for OPCODE which does not support format 4 instruction
                        error_message = "error at line " + to_string(iterator+1)+": Format 4 not supported for this opcode!";
                        errors.push_back(error_message);
                    }
                }
                else if(format_supported==4){
                    curr_format = 3;
                }
                else{
                    curr_format = format_supported;
                }
            }
            if(findinvec(assmDir,inst[i])!=-1){
                //Assembler Directive encountered
                if(!flag){
                    flag=1;
                    if(inst[i]=="BYTE"){
                        //BYTE Assembler Directive
                        if(inst.size()!=(i+2)){
                            error_message = "error at line " + to_string(iterator+1)+": Invalid Instruction!";
                            errors.push_back(error_message);
                        }
                        else{
                            //Calculate the space required for this instruction by decoding the initialised value
                            if(inst[i+1][0]=='C' or inst[i+1][0]=='c' or inst[i+1][0]=='X'){
                                if(inst[i+1][0]=='C' or inst[i+1][0]=='c'){
                                    curr_format = inst[i+1].size() - 3;
                                }
                                else{
                                    curr_format = (inst[i+1].size() - 3)/2;
                                }
                            }
                            else{
                                error_message = "error at line " + to_string(iterator+1)+": Invalid Instruction!";
                                errors.push_back(error_message);
                            }
                        }
                    }
                    else if(inst[i]=="WORD"){
                        //WORD Assembler Directive
                        curr_format = 3;
                        if(inst.size()!=(i+2)){
                            error_message = "error at line " + to_string(iterator+1)+": Invalid Instruction!";
                            errors.push_back(error_message);
                        }
                    }
                    else if(inst[i]=="RESB"){
                        //RESB Assembelr Directive
                        if(inst.size()==(i+2)) curr_format = stoi(inst[i+1]);
                        else{
                            error_message = "error at line " + to_string(iterator+1)+": Invalid Instruction!";
                            errors.push_back(error_message);
                        }
                    }
                    else if(inst[i]=="RESW"){
                        //RESW Assembler Directive
                        if(inst.size()==(i+2)) curr_format = 3*stoi(inst[i+1]);
                        else{
                            error_message = "error at line " + to_string(iterator+1)+": Invalid Instruction!";
                            errors.push_back(error_message);
                        }
                    }
                    else if(inst[i]=="BASE"){
                        //BASE Assembler Directive
                        if(inst.size()==(i+2)) curr_format = 0;
                        else{
                            error_message = "error at line " + to_string(iterator+1)+": Invalid Instruction!";
                            errors.push_back(error_message);
                        }
                    }
                    else if(inst[i]=="ORG"){
                        //ORG Assembler Directive
                        if(inst.size()!=2){
                            if(org_address==-1){
                                //If "ORG" is encountered without any prior ORG instruction
                                error_message = "error at line " + to_string(iterator+1)+" : ORG statement incomplete!";
                                errors.push_back(error_message);
                            }
                            else{
                                curr_format = org_address - curr_addr_pb;
                            }
                        }
                    }
                    else if(inst[i]=="LTORG"){
                        //LTORG Assembler Directive
                        if(inst.size()!=1){
                            error_message = "error at line " + to_string(iterator+1)+" : Invalid LTORG statement!";
                            errors.push_back(error_message);
                        }
                        else{
                            //Write all the literals that have been defined before this statement and have not been written
                            counter_literal = 0;
                            for(int i=0;i<litTab.size();i++){
                                vector<string> vec;
                                if(litTab[i].written==0){
                                    vec.push_back("*");
                                    vec.push_back('='+litTab[i].name);
                                    litTab[i].written = 1;
                                    litTab[i].progBlock = curr_pb;
                                }
                                instructions.insert(instructions.begin()+iterator+1+counter_literal, vec);
                                counter_literal++;
                            }
                        }
                    }
                }
                else{
                    error_message = "error at line " + to_string(iterator+1);
                    errors.push_back(error_message);
                }
            }
            if(i==0 and flag==0 and flag_progBlock==0){
                //Incase of the first element not being an OPCODE or ASSEMBLER DIRECTIVE i.e. Label
                if(inst[i]!="*"){
                    Symbol label(inst[i],curr_addr_pb);
                    label.progBlock = curr_pb;
                    if(findSymbol(inst[i])==-1 or symTab[findSymbol(inst[i])].addr==-1){
                        if(findSymbol(inst[i])==-1){
                            //Symbol/Label encountered first time
                            symTab.push_back(label);
                        }
                        else{
                            //Symbol/Label was referenced earlier, is defined now
                            symTab[findSymbol(inst[i])].addr = curr_addr_pb;
                            symTab[findSymbol(inst[i])].progBlock = curr_pb;
                        }    
                    }
                    else{
                        error_message = "error at line " + to_string(iterator+1)+": Symbol "+inst[i]+" has multiple instances!";
                        errors.push_back(error_message);
                    }
                }
            }
            if(i==1 and flag==0 and flag_progBlock==0){
                if(inst[i-1]=="*"){
                    curr_format = litTab[findLiteral(inst[i].substr(1))].size;
                    litTab[findLiteral(inst[i].substr(1))].addr = curr_addr_pb;
                    litTab[findLiteral(inst[i].substr(1))].progBlock = curr_pb;
                }
                else{
                    error_message = "error at line " + to_string(iterator+1)+": No OPCODE or ASSEMBLER DIRECTIVE FOUND!";
                    errors.push_back(error_message);
                }
            }
        }
        //Add the location counter values to the location counter vectors
        locctr.push_back(curr_addr);
        locctr_pb.push_back(curr_addr_pb);
        pc_pb.push_back(curr_addr_pb+curr_format);
        progblock.push_back(curr_pb);
        curr_addr += curr_format;
        curr_addr_pb += curr_format;
    }
    //Evaluate the expressions
    for(int i=0;i<symTab.size();i++){
        if(containsArithmetic(symTab[i].name)){
            if(symTab[i].addr == -1){
                if(isExpression(symTab[i].name)!=INT_MAX){
                    symTab[i].addr = isExpression(symTab[i].name);
                    symTab[i].relative = absOrRelative(symTab[i].name);
                    symTab[i].isSym = 1;
                }
                else{
                    error_message = "error: Expression " + symTab[i].name + "is not a valid expression!";
                    errors.push_back(error_message);
                }
            }
        }
    }
    //Set the values for Symbols defined by expressions
    for(int i=0;i<symTab.size();i++){
        if(symTab[i].parent != ""){
            symTab[i].addr = symTab[findSymbol(symTab[i].parent)].addr;
            symTab[i].relative = symTab[findSymbol(symTab[i].parent)].relative;
        }
        if(symTab[i].addr==-1){
            error_message = "error: Label " + symTab[i].name + " is accessed without definition!";
            errors.push_back(error_message);
        }
    }
    //Write the literals which have not been written yet
    for(int i=0;i<litTab.size();i++){
        vector<string> vec;
        if(litTab[i].written==0){
            vec.push_back("*");
            vec.push_back('='+litTab[i].name);
            litTab[i].written = 1;
            instructions.insert(instructions.begin()+instructions.size()-1, vec);
            litTab[i].addr = curr_addr_pb;
            locctr.push_back(curr_addr);
            locctr_pb.push_back(curr_addr_pb);
            progblock.push_back(curr_pb);
            curr_addr = curr_addr + litTab[i].size;
            curr_addr_pb += litTab[i].size;
        }
    }
    locctr.push_back(curr_addr);
    locctr_pb.push_back(curr_addr_pb);
    progblock.push_back(curr_pb);
    progBlockTable[curr_pb].length += curr_addr_pb - prev_addr;
    //Set the start addresses and lengths for the program blocks
    for(int i=1;i<progBlockTable.size();i++){
        progBlockTable[i].startAddr = progBlockTable[i-1].startAddr + progBlockTable[i-1].length;
    }
    programLength = inttohex(curr_addr - locctr[0]);
    //Update the symbol table for correct values according to program block addresses
    for(int i=0;i<symTab.size();i++){
        if(symTab[i].relative==1){
            if(symTab[i].progBlock!=0){
                symTab[i].addr = symTab[i].addr + progBlockTable[symTab[i].progBlock].startAddr;
            }
        }
    }
    //Update the literal table for correct values according to program block addresses
    for(int j=0;j<litTab.size();j++){
        litTab[j].addr += progBlockTable[litTab[j].progBlock].startAddr - startAddr;
    }
    //Set the correct values of program counter for each instruction according to program blocks
    for(int i=0;i<pc_pb.size();i++){
        pc_pb[i] = pc_pb[i] + progBlockTable[progblock[i]].startAddr - startAddr;
    }
    return 1;
}

int secondPass(){
    int pc = pc_pb[1];
    bool basedefined = 0;
    int base = 0;
    objCode.push_back("");
    //Iterate over each instruction
    for(int iterator=1;iterator<instructions.size()-1;iterator++){
        //Intitialise all required variables
        bool flag = 0;
        bool form4 = 0;
        bool assDir = 0;
        bool byteorword = 0;    
        string objcode = "";
        string opcode = "";
        string error_message;
        string assem_operand = "";
        string ascii_operand = "";
        int opcodeformat = 0;
        string templabel = "";
        int displacement=0;
        string regnum = "";
        int N = 1;
        int I = 1;
        int X = 0;
        int B = 0;
        int P = 0;
        int E = 0;
        vector<string> inst = instructions[iterator];
        //Iterate over each element of the instruction
        for(int i=0;i<inst.size();i++){
            if(inst[i][0] == '+'){
                form4 = 1;
            }
            if(inst[0]=="*"){
                flag=1;
            }
            if((opCode.find(inst[i]) != opCode.end())){
                //OPCODE MATCHED eg.- LDA
                flag=1;
                opcodeformat = opCode[inst[i]].first;
                opcode = opCode[inst[i]].second;
                E = 0;//FORMAT 3
            }
            else if(form4 and (opCode.find(inst[i].substr(1)) != opCode.end())){
                //OPCODE MATCHED eg.- +LDA
                flag=1;
                opcodeformat = opCode[inst[i].substr(1)].first;
                opcode = opCode[inst[i].substr(1)].second;
                E = 1;
                P = 0;
                B = 0;
                X = 0;
            }
            else if(findinvec(assmDir,inst[i])!=-1){
                //Assembler Directive
                flag=1;
                assDir = 1;
                if(inst[i] == "WORD" or inst[i] == "BYTE"){
                    byteorword = 1;
                    if(findSymbol(inst[i+1])!=-1){
                        if(symTab[findSymbol(inst[i+1])].relative==1){
                            modification.push_back(pc_pb[iterator-1]);
                            modificationSize.push_back(6);
                        }
                    }
                }
                else if(inst[i]=="BASE"){
                    //BASE Assembler Directive
                    basedefined = 1;
                    if(inst.size()<=i+1){
                        error_message = "error at line " + to_string(iterator+1) + " : No label after BASE Assembler Directive!";
                        errors.push_back(error_message);
                    }
                    base = symTab[findSymbol(inst[i+1])].addr;
                }
                else if(inst[i] == "NOBASE"){
                    basedefined = 0;
                }
            }
            else if(inst[i][0]=='#' or inst[i][0]=='@'){
                if(inst[i][0] == '#'){
                    //IMMEDIATE MODE OF ADDRESSING
                    N = 0;
                }
                else{
                    //INDIRECT MODE OF ADDRESSING
                    I = 0;
                }
                templabel = inst[i].substr(1);
                if(flag and (findSymbol(inst[i].substr(1))!=-1)){
                    // #LABEL or @LABEL
                    if(E==1){
                        //FORMAT 4
                        if(symTab[findSymbol(inst[i].substr(1))].relative==1){
                            modification.push_back(pc_pb[iterator-1]+1);
                            modificationSize.push_back(5); 
                        }
                        displacement = symTab[findSymbol(templabel)].addr;
                        if(displacement >= 1048576){
                            error_message = "error at line " + to_string(iterator+1) + " : Value is too big to represent in 20 bits!";
                            errors.push_back(error_message);
                        }
                    }
                    else if(inst[i][0]=='@'){
                        //INDIRECT MODE
                        displacement = symTab[findSymbol(templabel)].addr - pc;
                        if(abs(displacement)>=2048){
                            if(basedefined){
                                B = 1;
                                displacement = symTab[findSymbol(templabel)].addr - base;
                                if(displacement<0 or displacement>=4096){
                                    error_message = "error at line " + to_string(iterator+1) + " : Can use neither PC nor BASE Relative addressing!";
                                    errors.push_back(error_message);
                                }
                            }
                            else{
                                error_message = "error at line " + to_string(iterator+1) + " : Require base addressing but base undefined!";
                                errors.push_back(error_message);
                            }
                        }
                        else{
                            P = 1;
                        }
                    }
                    else if(inst[i][0]=='#'){
                        //IMMEDIATE MODE
                        if(symTab[findSymbol(templabel)].isSym==1){
                            displacement = symTab[findSymbol(templabel)].addr;
                            modification.push_back(pc_pb[iterator-1]+1);
                            modificationSize.push_back(3);
                            if(displacement >= 4096){
                                error_message = "error at line " + to_string(iterator+1) + " : Value of immediate can't be expressed in 12 bits!";
                                errors.push_back(error_message);
                            }
                        }
                        else{
                            displacement = symTab[findSymbol(templabel)].addr - pc;
                            if(abs(displacement)>=2048){
                                if(basedefined){
                                    B = 1;
                                    displacement = symTab[findSymbol(templabel)].addr - base;
                                    if(displacement<0 or displacement>=4096){
                                        error_message = "error at line " + to_string(iterator+1) + " : Can use neither PC nor BASE Relative addressing!";
                                        errors.push_back(error_message);
                                    }
                                }
                                else{
                                    error_message = "error at line " + to_string(iterator+1) + " : Require base addressing but base undefined!";
                                    errors.push_back(error_message);
                                }
                            }
                            else{
                                P = 1;
                            }    
                        }
                    }
                }
                else{
                    // #1000 or @1000
                    displacement = stoi(templabel);
                }    
            }
            else if(flag and findSymbol(inst[i])!=-1){
                // LABEL
                if(inst[i-1]=="WORD"){
                    assem_operand = inttohex_param(symTab[findSymbol(inst[i])].addr,6);
                }
                else if(E==0){
                    displacement = symTab[findSymbol(inst[i])].addr - pc;
                    if(abs(displacement)>=2048){
                        if(basedefined){
                            B = 1;
                            displacement = symTab[findSymbol(inst[i])].addr - base;
                            if(displacement<0 or displacement>=4096){
                                error_message = "error at line " + to_string(iterator+1) + " : Can use neither PC nor BASE Relative addressing!";
                                errors.push_back(error_message);
                            }
                        }
                        else{
                            error_message = "error at line " + to_string(iterator+1) + " : Require base addressing but base undefined!";
                            errors.push_back(error_message);
                        }
                    }
                    else{
                        P = 1;
                    }
                }
                else{
                    modification.push_back(pc_pb[iterator-1]+1);
                    modificationSize.push_back(5);
                    displacement = symTab[findSymbol(inst[i])].addr;
                    P = 0;
                    B = 0;
                    X = 0;
                }
            }
            else if(flag){
                //1000 or C'EOF' or X'F1' or X,A,...
                if(findLiteral(inst[i].substr(1))!=-1){
                    //LITERAL
                    if(inst[i-1]=="*"){
                        if(inst[i][1]=='C' and inst[i][2]=='\''){
                            for(int iter=3; iter<inst[i].size()-1;iter++){
                                objcode = objcode + inttohex_param(int(inst[i][iter]),2);
                            }
                        }
                        else{
                            objcode = inst[i].substr(3,inst[i].size()-4);
                        }
                    }
                    else{
                        if(E==0){
                            displacement = litTab[findLiteral(inst[i].substr(1))].addr - pc;
                            if(abs(displacement)>=2048){
                                if(basedefined){
                                    B = 1;
                                    displacement = litTab[findLiteral(inst[i].substr(1))].addr - base;
                                    if(displacement<0 or displacement>=4096){
                                        error_message = "error at line " + to_string(iterator+1) + " : Can use neither PC nor BASE Relative addressing!";
                                        errors.push_back(error_message);
                                    }
                                }
                                else{
                                    error_message = "error at line " + to_string(iterator+1) + " : Require base addressing but base undefined!";
                                    errors.push_back(error_message);
                                }
                            }
                            else{
                                P = 1;
                            }
                        }
                        else{
                            displacement = litTab[findLiteral(inst[i].substr(1))].addr;
                            modification.push_back(pc_pb[iterator-1]+1);
                            modificationSize.push_back(5);
                        }
                    }
                }
                else if(isNum(inst[i]) and !assDir){
                    //DIRECT MODE
                    displacement = stoi(inst[i]) - pc;
                    if(abs(displacement)>=2048){
                        if(basedefined){
                            B = 1;
                            displacement = stoi(inst[i]) - base;
                            if(displacement<0 or displacement>=4096){
                                error_message = "error at line " + to_string(iterator+1) + " : Can use neither PC nor BASE Relative addressing!";
                                errors.push_back(error_message);
                            }
                        }
                        else{
                            error_message = "error at line " + to_string(iterator+1) + " : Require base addressing but base undefined!";
                            errors.push_back(error_message);
                        }
                    }
                    else{
                        P = 1;
                    }
                }
                else if(isNum(inst[i]) and assDir){
                    assem_operand = inttohex_param(stoi(inst[i]),6);
                }
                else if(findinvec(registers, inst[i])!=-1){
                    //REGISTER i.e. A,X,....
                    if(opcodeformat==2){
                        regnum = regnum + to_string(findinvec(registers,inst[i]));
                    }
                    else if((form4==1 and opcodeformat==4) or (opcodeformat == 1)){
                        if(form4==1 and opcodeformat==4){
                            if(inst[i]=="X"){
                                X=1;
                            }
                            else{
                                error_message = "error at line " + to_string(iterator+1) + " : Invalid use of register!";
                                errors.push_back(error_message);
                            }
                        }
                        else{
                            error_message = "error at line " + to_string(iterator+1) + " : Invalid use of register!";
                            errors.push_back(error_message);
                        }
                    }
                    else if((opcodeformat==4 and form4==0)){
                        if(inst[i]!="X"){
                            error_message = "error at line " + to_string(iterator+1) + " : Invalid use of register!";
                            errors.push_back(error_message);
                        }
                        else{
                            X = 1;
                        }
                    }                      
                }
                else if((inst[i][0]=='C' or inst[i][0]=='X') and (inst[i][1]=='\'')){
                    assem_operand = inst[i].substr(2,inst[i].size()-3);
                    if(inst[i][0]=='C'){
                        for(int j=0;j<assem_operand.size();j++){
                            ascii_operand = ascii_operand + inttohex_param(int(assem_operand[j]),2);
                        }
                        assem_operand = ascii_operand;
                    }
                }
            }
        }
        //WRITE THE OBJECT CODE
        if(assDir){
            if(byteorword){
                objcode = assem_operand;
            }
        }
        else if(opcodeformat==1){
            objcode = opcode;
        }
        else if(opcodeformat==2){
            objcode = opcode + regnum;
            while(objcode.size()<4){
                objcode = objcode + '0';
            }
        }
        else if(opcodeformat==4){
            if(E==0){
                //FORMAT 3
                objcode = inttohex_param(hextoint(opcode)+2*N+I,2) + inttohex(8*X + 4*B + 2*P + E) + inttohex_param(displacement,3);
            }
            else{
                //FORMAT 4
                objcode = inttohex_param(hextoint(opcode)+2*N+I,2) + inttohex(8*X + E) + inttohex_param(displacement,5);
            }
        }
        objCode.push_back(objcode);
        pc = pc_pb[1+iterator];
    }
    objCode.push_back("");
    if(errors.size()!=0){
        return 0;
    }
    else{
        return 1;
    }
}

void generateObjectProgram(){
    string header = "H";
    header = header + programName;
    while(header.size()<7){
        header = header + ' ';
    }

    header = header + inttohex_param(startAddr,6) + inttohex_param(hextoint(programLength),6);
    objProgram.push_back(header);
    string textrec = "";
    string modifrec = "M";
    string endrec = "E";
    bool flag = 0;
    int counter = 0;
    int startaddr = startAddr;
    for(int i=1;i<objCode.size()-1;i++){
        flag = 0;
        if(pc_pb[i-1]!=pc_pb[i]){ //This instruction has been allocated some memory
            if(objCode[i]!=""){
                textrec = textrec + objCode[i];
                if(textrec.size()+objCode[i+1].size()>60){
                    flag=1;
                }

            }
            if(flag){
                //PUSH INTO TEXT RECORD VECTOR
                textrec = "T" + inttohex_param(startAddr,6) + inttohex_param((textrec.size()/2),2) + textrec;
                objProgram.push_back(textrec);
                textrec = "";
                startAddr = pc_pb[i];
                while(objCode[i+counter]==""){
                    startAddr = pc_pb[i+counter];
                    counter++;
                }
                i = i + counter - 1;
                counter = 0;
            }
            else if(objCode[i]==""){
                textrec = "T" + inttohex_param(startAddr,6) + inttohex_param((textrec.size()/2),2) + textrec;
                objProgram.push_back(textrec);
                textrec = "";
                startAddr = pc_pb[i-1];
                while(objCode[i+counter]==""){ //There is an object code for the instruction
                    startAddr = pc_pb[i+counter];
                    counter++;
                }
                i = i + counter - 1;
                counter = 0;
            }
        }
    }
    if(textrec!=""){
        textrec = "T" + inttohex_param(startAddr,6) + inttohex_param((textrec.size()/2),2) + textrec;
        objProgram.push_back(textrec);
    }
    for(int j=0;j<modification.size();j++){
        modifrec = modifrec + inttohex_param(modification[j],6) + inttohex_param(modificationSize[j],2);
        objProgram.push_back(modifrec);
        modifrec = "M";
    }
    for(int j=0;j<objCode.size();j++){
        if(objCode[j]!=""){
            endrec = endrec + inttohex_param(pc_pb[j-1],6);
            objProgram.push_back(endrec);
            break;
        }
    }
}

void printObjectProgram(){
    cout<<"\n\t\tOBJECT PROGRAM\n\n";
    for(int i=0;i<objProgram.size();i++){
        cout<<objProgram[i]<<"\n";
    }
    cout<<"\n\n";
}

int main(){
    opCodeMapping(); //Generates a map containing the op-codes and their respective formats
    getInstructions(); //Takes instructions as input and stores them
    cout<<"---------------------------------------------------------------------------------------------------------------------"<<endl;
    int a = firstPass();
    int b;
    b = secondPass();
    if(b==1){
        printListingFile(instructions); //Displays the Listing File of the program
        generateObjectProgram(); //Generates the object program and stores it
        printObjectProgram(); //Displays the object program
        cout<<"---------------------------------------------------------------------------------------------------------------------"<<endl;
    }
    else{
        cout<<"\nCould not assemble program due to the following errors:\n";
        for(int i=0;i<errors.size();i++){
            cout<<errors[i]<<"\n";
        }
        cout<<"\n********************************************************************************************************************\n";
    }
    return 0;
}