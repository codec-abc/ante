#include "lexer.h"
#include <cstdlib>
#include <cstring>

using namespace ante;

/*
 *  Maps each non-literal token to a string representing
 *  its type.
 */
map<int, const char*> tokDict = {
    {Tok_Ident, "Identifier"},
    {Tok_UserType, "UserType"},

    //types
    {Tok_I8, "I8"},
    {Tok_I16, "I16"},
    {Tok_I32, "I32"},
    {Tok_I64, "I64"},
    {Tok_U8, "U8"},
    {Tok_U16, "U16"},
    {Tok_U32, "U32"},
    {Tok_U64, "U64"},
    {Tok_Isz, "Isz"},
    {Tok_Usz, "Usz"},
    {Tok_F16, "F16"},
    {Tok_F32, "F32"},
    {Tok_F64, "F64"},
    {Tok_C8, "C8"},
    {Tok_C32, "C32"},
    {Tok_Bool, "Bool"},
    {Tok_Void, "Void"},

    {Tok_Eq, "Eq"},
    {Tok_NotEq, "NotEq"},
    {Tok_AddEq, "AddEq"},
    {Tok_SubEq, "SubEq"},
    {Tok_MulEq, "MulEq"},
    {Tok_DivEq, "DivEq"},
    {Tok_GrtrEq, "GrtrEq"},
    {Tok_LesrEq, "LesrEq"},
    {Tok_Or, "Or"},
    {Tok_And, "And"},
    {Tok_Range, "Range"},
    {Tok_Returns, "Returns"},

    //literals
    {Tok_True, "True"},
    {Tok_False, "False"},
    {Tok_IntLit, "IntLit"},
    {Tok_FltLit, "FltLit"},
    {Tok_StrLit, "StrLit"},

    //keywords
    {Tok_Return, "Return"},
    {Tok_If, "If"},
    {Tok_Elif, "Elif"},
    {Tok_Else, "Else"},
    {Tok_For, "For"},
    {Tok_While, "While"},
    {Tok_Do, "Do"},
    {Tok_In, "In"},
    {Tok_Continue, "Continue"},
    {Tok_Break, "Break"},
    {Tok_Import, "Import"},
    {Tok_Let, "Let"},
    {Tok_Var, "Var"},
    {Tok_Match, "Match"},
    {Tok_Data, "Data"},
    {Tok_Enum, "Enum"},
    {Tok_Fun, "Fun"},
    {Tok_Ext, "Ext"},

    //modifiers
    {Tok_Pub, "Pub"},
    {Tok_Pri, "Pri"},
    {Tok_Pro, "Pro"},
    {Tok_Raw, "Raw"},
    {Tok_Const, "Const"},
    {Tok_Noinit, "Noinit"},
    {Tok_Pathogen, "Pathogen"},

    //other
    {Tok_Where, "Where"},
    {Tok_Infect, "Infect"},
    {Tok_Cleanse, "Cleanse"},
    {Tok_Ct, "Ct"},

    {Tok_Newline, "Newline"},
    {Tok_Indent, "Indent"},
    {Tok_Unindent, "Unindent"},
};

/*
 *  Maps each keyword to its corresponding TokenType
 */
map<string, int> keywords = {
    {"i8",       Tok_I8},
    {"i16",      Tok_I16},
    {"i32",      Tok_I32},
    {"i64",      Tok_I64},
    {"u8",       Tok_U8},
    {"u16",      Tok_U16},
    {"u32",      Tok_U32},
    {"u64",      Tok_U64},
    {"isz",      Tok_Isz},
    {"usz",      Tok_Usz},
    {"f16",      Tok_F16},
    {"f32",      Tok_F32},
    {"f64",      Tok_F64},
    {"c8",       Tok_C8},
    {"c32",      Tok_C32},
    {"bool",     Tok_Bool},
    {"void",     Tok_Void},
    
    {"or",       Tok_Or},
    {"and",      Tok_And},
    {"true",     Tok_True},
    {"false",    Tok_False},
    
    {"return",   Tok_Return},
    {"if",       Tok_If},
    {"elif",     Tok_Elif},
    {"else",     Tok_Else},
    {"for",      Tok_For},
    {"while",    Tok_While},
    {"do",       Tok_Do},
    {"in",       Tok_In},
    {"continue", Tok_Continue},
    {"break",    Tok_Break},
    {"import",   Tok_Import},
    {"let",      Tok_Let},
    {"var",      Tok_Var},
    {"match",    Tok_Match},
    {"data",     Tok_Data},
    {"enum",     Tok_Enum},
    {"fun",      Tok_Fun},
    {"ext",      Tok_Ext},
    
    {"pub",      Tok_Pub},
    {"pri",      Tok_Pri},
    {"pro",      Tok_Pro},
    {"raw",      Tok_Raw},
    {"const",    Tok_Const},
    {"noinit",   Tok_Noinit},
    {"pathogen", Tok_Pathogen},

    //other
    {"where",    Tok_Where},
    {"infect",   Tok_Infect},
    {"cleanse",  Tok_Cleanse},
    {"ct",       Tok_Ct},
};

        
/* Raw text to store identifiers and usertypes in */
char *lextxt;

Lexer *yylexer;

/* Sets lexer instance for yylex to use */
void setLexer(Lexer *l){
    yylexer = l;
}

int yylex(...){
    return yylexer->next();
}
    

/*
 * Initializes lexer
 */
Lexer::Lexer(const char* file) : 
    fileName{file},
    in{new ifstream(file)},
    row{1},
    col{1},
    tokRow{1},
    tokCol{1},
    cur{0},
    nxt{0},
    scopes{new stack<unsigned int>()},
    cscope{0}
{
    if(!*in){
        cerr << "Error: Unable to open file '" << file << "'\n";
        exit(EXIT_FAILURE);
    }

    incPos();
    incPos();
    scopes->push(0);
}

Lexer::~Lexer(){
    delete scopes;
    delete in;
}

char Lexer::peek(){
    return cur;
}

unsigned int Lexer::getRow(){
    return tokRow;
}

unsigned int Lexer::getCol(){
    return tokCol;
}


/*
*  Prints a token's type to stdout
*/
void Lexer::printTok(int t){
    cout << getTokStr(t);
}

/*
*  Translates a token's type to a string
*/
string Lexer::getTokStr(int t){
    string s = "";
    if(IS_LITERAL(t)){
        s += (char)t;
    }else{
        s += tokDict[t];
    }
    return s;
}

inline void Lexer::incPos(void){
    cur = nxt;
    col++;
    if(in->good())
        in->get(nxt);
    else
        nxt = 0;
}

void Lexer::incPos(int end){
    for(int i = 0; i < end; i++){
        cur = nxt;
        col++;
        if(in->good())
            in->get(nxt);
        else
            nxt = 0;
    }
}

int Lexer::handleComment(){
    if(cur == '`'){
        do{
            incPos();
            if(cur == '\n'){
                row++;
                col = 0;
            }
        }while(cur != '`' && cur != EOF);
        incPos();
    }else{ // c == '~'
        while(cur != '\n' && cur != EOF) incPos();
    }
    return next();
}

/*
*  Allocates a new string for lextxt without
*  freeing its previous value.  The previous value
*  should always be stored in a node during parsing
*  and freed later.
*/
void Lexer::setlextxt(string *str){
    size_t size = str->size() + 1;
    lextxt = (char*)malloc(size);
    strcpy(lextxt, str->c_str());
    lextxt[size-1] = '\0';
}

int Lexer::genAlphaNumTok(){
    string s = "";
    tokRow = row;
    tokCol = col;

    while(IS_ALPHANUM(cur)){
        s += cur;
        incPos();
    }

    auto key = keywords.find(s.c_str());
    if(key != keywords.end()){
        return key->second;
    }else{
        setlextxt(&s);
        return (s[0] >= 'A' && s[0] <= 'Z') ? Tok_UserType : Tok_Ident;
    }
}

int Lexer::genNumLitTok(){
    string s = "";
    bool flt = false;
    tokRow = row;
    tokCol = col;

    while(IS_NUMERICAL(cur) || (cur == '.' && !flt && IS_NUMERICAL(nxt)) || cur == '_'){
        if(cur != '_'){
            s += cur;
            if(cur == '.') flt = true;
        }
        incPos();
    }

    //check for type suffix
    if(flt){
        if(cur == 'f'){
            s += 'f';
            incPos();
            if(cur == '1' && nxt == '6'){
                s += "16";
                incPos();
                incPos();
            }else if(cur == '3' && nxt == '2'){
                s += "32";
                incPos();
                incPos();
            }else if(cur == '6' && nxt == '4'){
                s += "64";
                incPos();
                incPos();
            }
            
            if(IS_NUMERICAL(cur)){
                lexErr("Extraneous numbers after type suffix.");
            }
        }
    }else{
        if(cur == 'i' || cur == 'u'){
            s += cur;
            incPos();
            if(cur == '8'){
                s += '8';
                incPos();
            }else if(cur == '1' && nxt == '6'){
                s += "16";
                incPos();
                incPos();
            }else if(cur == '3' && nxt == '2'){
                s += "32";
                incPos();
                incPos();
            }else if(cur == '6' && nxt == '4'){
                s += "64";
                incPos();
                incPos();
            }

            if(IS_NUMERICAL(cur)){
                lexErr("Extraneous numbers after type suffix.");
            }
        }
    }

    setlextxt(&s);
    return flt? Tok_FltLit : Tok_IntLit;
}

int Lexer::genWsTok(){
    if(cur == '\n'){
        unsigned int newScope = 0;

        while(IS_WHITESPACE(cur) && cur != EOF){
            switch(cur){
                case ' ': newScope++; break;
                case '\n': newScope = 0; row++; col = 0; break;
                case '\t':
                    lexErr("Tab characters are invalid whitespace.");
                default: break;
            }
            incPos();
            if(IS_COMMENT(cur)) return handleComment();
        }

        if(!scopes->empty() && newScope == scopes->top()){
            //tokRow is not set to row for newline tokens in case there are several newlines.
            //In this case, if set to row, it would become the row of the last newline.
            //Incrementing it from its previous token (guarenteed to be non-newline) fixes this.
            tokRow++;
            tokCol = 0;
            return Tok_Newline; /* Scope did not change, just return a Newline */
        }
        cscope = newScope;
        return next();
    }else{
        incPos();
        return next();
    }
}

int Lexer::genStrLitTok(char delim){
    string s = "";
    tokRow = row;
    tokCol = col;
    incPos();
    while(cur != delim && cur != EOF){
        if(cur == '\\'){
            switch(nxt){
                case 'a': s += '\a'; break;
                case 'b': s += '\b'; break;
                case 'f': s += '\f'; break;
                case 'n': s += '\n'; break;
                case 'r': s += '\r'; break;
                case 't': s += '\t'; break;
                case 'v': s += '\v'; break;
                default:  s += nxt; break;
            }
            incPos();
        }else{
            s += cur;
        }
        incPos();
    }
    incPos();
    setlextxt(&s);
    return Tok_StrLit;
}

int Lexer::next(){
    if(shouldReturnNewline){
        shouldReturnNewline = false;
        return Tok_Newline;
    }

    if(scopes->top() != cscope){
        if(cscope > scopes->top()){
            scopes->push(cscope);
            return Tok_Indent;
        }else{
            scopes->pop();
            shouldReturnNewline = true;
            return Tok_Unindent;
        }
    }

    if(IS_COMMENT(cur))    return handleComment();
    if(IS_NUMERICAL(cur))  return genNumLitTok();
    if(IS_ALPHANUM(cur))   return genAlphaNumTok();
    if(IS_WHITESPACE(cur)) return genWsTok();

    if(cur == '"' || cur == '\'') 
        return genStrLitTok(cur);
   
    //If the token is none of the above, it must be a symbol, or a pair of symbols.
    //Set the beginning of the token about to be created here.
    tokRow = row;
    tokCol = col;

    //substitute -> for an indent and ;; for an unindent
    if(cur == '-' && nxt == '>'){
        cscope++;
        RETURN_PAIR(next());
    }else if(cur == ';' && nxt == ';'){
        unsigned int curScope = scopes->top();
        if(curScope != 0){
            scopes->pop();
            cscope = scopes->top();
            scopes->push(curScope);
        }else{
            lexErr("Extraneous ;; leads to scope underflow.");
        }
        RETURN_PAIR(next());
    }else if(cur == '\\' && nxt == '\n'){ //ignore newline
        incPos(2);
        col = 1;
        row++;
        return next();
    }

    if(cur == '.' && nxt == '.') RETURN_PAIR(Tok_Range);
    if(cur == '=' && nxt == '>') RETURN_PAIR(Tok_Returns);

    if(nxt == '='){
        switch(cur){
            case '=': RETURN_PAIR(Tok_Eq);
            case '+': RETURN_PAIR(Tok_AddEq);
            case '-': RETURN_PAIR(Tok_SubEq);
            case '*': RETURN_PAIR(Tok_MulEq);
            case '/': RETURN_PAIR(Tok_DivEq);
            case '!': RETURN_PAIR(Tok_NotEq);
            case '>': RETURN_PAIR(Tok_GrtrEq);
            case '<': RETURN_PAIR(Tok_LesrEq);
        }
    }
    
    if(cur == 0 || cur == EOF) return 0; //End of input

    //If the character is nota, assume it is an operator and return it by value.
    char ret = cur;
    incPos();
    return ret;
}


void Lexer::lexErr(const char *msg){
    error(msg, fileName, row, col);
    exit(EXIT_FAILURE);//lexing errors are always fatal
}
