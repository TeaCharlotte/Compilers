#include <string>
#include <vector>
#include <iostream>
#include <fstream>
#include <algorithm>
#include <iomanip>

// using std::count;
using namespace std;

static string keyword[100]{
    "char", "int", "float", "break", "const", "return",
    "void", "continue", "do", "while", "if", "else", "string"
    // 按照课本p40定的13个关键字
    // 定义编码表中1-13关键字，14标识符，15整型，16浮点数，17布尔常数，18字符串
};
static vector<string> symbol_table;
static vector<string> symbol_type;
static vector<int> symbol_row;
static vector<string> constant_table;
static vector<string> constant_type;
static vector<int> constant_row;
static vector<string> token_table;
static vector<string> token_type;
static vector<int> token_row;
static int rowNum = 0;

// 当首个字符是字母时，这个单词有可能是关键字、标识符或者布尔类型常数
// 判断token里的单词是关键字、布尔值和标识符中的哪一个，返回在编码表中的位置
int isIn(string word)
{
    int loca = 0; // 编码表中14是标识符，1-13是关键字，17是布尔值
    for (int i = 0; i < 13; i++)
    {
        if (word == keyword[i])
            loca = i + 1; // 数组从0开始，关键字数组中单词的位置比在编码表里的位置小1
    }
    if (word == "false" || word == "true")
        loca = 17;
    return loca;
}

// 当前读入单词的首字符是字母时，单词可能是关键字、标识符或者布尔值。
// 进入首符号是字母的单词判定循环，直到出现不是字母或数字的字符，跳出循环
int classLetter(int k, int beginRow, string temp)
{
    string token = "";
    int begin = k;
    token += temp[k];
    k++;
    // 出现不是字母或数字的字符时，跳出循环
    while (k < temp.size() && (isalpha(temp[k]) || isdigit(temp[k])))
    {
        token += temp[k];
        k++;
        // 判断是关键字、标识符或者布尔值
        if (isIn(token) != 0)
        {
            break;
        }
    }
    int loca = isIn(token);
    if (loca < 14 && loca > 0) // token是关键字，加入符号表symbol_table
    {
        token_table.push_back(token); // 计入单词表，不需要去重
        token_type.push_back("关键字");
        token_row
            .push_back(rowNum + k - begin);
        if (count(symbol_table.begin(), symbol_table.end(), token) == 0) // 计入符号表，需要去重
        {
            symbol_table.push_back(token);
            symbol_type.push_back("关键字");
            symbol_row.push_back(rowNum + k - begin);
        }
    }
    else if (loca == 17) // token是布尔常数
    {
        token_table.push_back(token); // 计入单词表
        token_type.push_back("布尔型常数");
        token_row
            .push_back(rowNum + k - begin);
        if (count(constant_table.begin(), constant_table.end(), token) == 0) // 计入常数表，需要去重
        {
            constant_table.push_back(token);
            constant_type.push_back("布尔型常数");
            constant_row.push_back(rowNum + k - begin);
        }
    }
    else // 是标识符
    {
        token_table.push_back(token); // 计入单词表
        token_type.push_back("标识符");
        token_row
            .push_back(rowNum + k - begin);
        if (count(symbol_table.begin(), symbol_table.end(), token) == 0) // 计入符号表
        {
            symbol_table.push_back(token);
            symbol_type.push_back("标识符");
            symbol_row.push_back(rowNum - begin + k);
        }
    }

    return k - begin; // 返回匹配完成时移动的距离
}

// 当前读入单词的首字符是数字时，单词可能是浮点常数或者整型常数。
// 循环是判定下一个读到的字符是不是小数点，如果是就把flag置1，说明是浮点数
// 进入首符号是字母的单词判定循环，直到出现不是数字或小数点的字符，跳出循环
int classNum(int k, int rowNum, string temp)
{
    int flag = 0; // 判断数字是不是浮点数的flag
    int begin = k;
    string token = "";
    token += temp[k];
    k++;
    while (k < temp.size())
    {
        if (isdigit(temp[k])) // 把数字累计加在token里
        {
            token += temp[k];
            k++;
        }
        else if (temp[k] == '.')
        {
            flag = 1; // 置1
            token += temp[k];
            k++;
            if (isdigit(temp[k]))
            {
                token += temp[k];
                k++;
            }
            else
                break;
        }
        else
            break;
    }
    token_table.push_back(token); // 计入单词表
    token_row
        .push_back(rowNum + k - begin);
    if (flag == 0)
    {
        token_type.push_back("整型常数");
        if (count(constant_table.begin(), constant_table.end(), token) == 0) // 计入常数表，需要去重
        {
            constant_type.push_back("整型常数");
            constant_table.push_back(token);
            constant_row.push_back(rowNum + k - begin);
        }
    }
    else
    {
        token_type.push_back("浮点型常数");
        if (count(constant_table.begin(), constant_table.end(), token) == 0) // 计入常数表，需要去重
        {
            constant_type.push_back("浮点型常数");
            constant_table.push_back(token);
            constant_row.push_back(rowNum + k - begin);
        }
    }
    return k - begin;
}

int classOpPlus(int k, int rowNum, string temp) //+ += ++
{
    int begin = k;
    string token = "";
    token += temp[k];
    k++;
    if (k < temp.size() && temp[k] == '=') //+=
    {
        token += '=';
        k++;
        token_table.push_back(token); // 加入单词表
        token_type.push_back("运算符:加等于");
        token_row
            .push_back(rowNum + k - begin);
        if (count(symbol_table.begin(), symbol_table.end(), token) == 0) // 加入符号表，需要去重
        {
            symbol_table.push_back(token);
            symbol_type.push_back("运算符:加等于");
            symbol_row.push_back(rowNum + k - begin);
        }
    }
    else if (k < temp.size() && temp[k] == '+') //++
    {
        token += '+';
        k++;
        token_table.push_back(token); // 加入单词表
        token_type.push_back("运算符:加加");
        token_row
            .push_back(rowNum + k - begin);
        if (count(symbol_table.begin(), symbol_table.end(), token) == 0) // 加入符号表，需要去重
        {
            symbol_table.push_back(token);
            symbol_type.push_back("运算符:加加");
            symbol_row.push_back(rowNum + k - begin);
        }
    }
    else //+
    {
        token_table.push_back(token); // 加入单词表
        token_type.push_back("运算符:加");
        token_row
            .push_back(rowNum + k - begin);
        if (count(symbol_table.begin(), symbol_table.end(), token) == 0) // 加入符号表，需要去重
        {
            symbol_table.push_back(token);
            symbol_type.push_back("运算符:加");
            symbol_row.push_back(rowNum + k - begin);
        }
    }
    return k - begin;
}

int classOpMinus(int k, int rowNum, string temp) //- -= --
{
    int begin = k;
    string token = "";
    token += temp[k];
    k++;
    if (k < temp.size() && temp[k] == '=') //-=
    {
        token += '=';
        k++;
        token_table.push_back(token); // 加入单词表
        token_type.push_back("运算符:减等于");
        token_row
            .push_back(rowNum + k - begin);
        if (count(symbol_table.begin(), symbol_table.end(), token) == 0) // 加入符号表，需要去重
        {
            symbol_table.push_back(token);
            symbol_type.push_back("运算符:减等于");
            symbol_row.push_back(rowNum + k - begin);
        }
    }
    else if (k < temp.size() && temp[k] == '-') //--
    {
        token += '-';
        k++;
        token_table.push_back(token); // 加入单词表
        token_type.push_back("运算符:加加");
        token_row
            .push_back(rowNum + k - begin);
        if (count(symbol_table.begin(), symbol_table.end(), token) == 0) // 加入符号表，需要去重
        {
            symbol_table.push_back(token);
            symbol_type.push_back("运算符:减减");
            symbol_row.push_back(rowNum + k - begin);
        }
    }
    else //-
    {
        token_table.push_back(token); // 加入单词表
        token_type.push_back("运算符:减");
        token_row
            .push_back(rowNum + k - begin);
        if (count(symbol_table.begin(), symbol_table.end(), token) == 0) // 加入符号表，需要去重
        {
            symbol_table.push_back(token);
            symbol_type.push_back("运算符:减");
            symbol_row.push_back(rowNum + k - begin);
        }
    }
    return k - begin;
}

int classOpMul(int k, int rowNum, string temp) //* *=
{
    int begin = k;
    string token = "";
    token += temp[k];
    k++;
    if (k < temp.size() && temp[k] == '=') //*=
    {
        token += '=';
        k++;
        token_table.push_back(token); // 加入单词表
        token_type.push_back("运算符:乘等于");
        token_row
            .push_back(rowNum + k - begin);
        if (count(symbol_table.begin(), symbol_table.end(), token) == 0) // 加入符号表，需要去重
        {
            symbol_table.push_back(token);
            symbol_type.push_back("运算符:乘等于");
            symbol_row.push_back(rowNum + k - begin);
        }
    }
    else //*
    {
        token_table.push_back(token); // 加入单词表
        token_type.push_back("运算符:乘");
        token_row
            .push_back(rowNum + k - begin);
        if (count(symbol_table.begin(), symbol_table.end(), token) == 0) // 加入符号表，需要去重
        {
            symbol_table.push_back(token);
            symbol_type.push_back("运算符:乘");
            symbol_row.push_back(rowNum + k - begin);
        }
    }
    return k - begin;
}

int classOpDivi(int k, int rowNum, string temp) /// /=
{
    int begin = k;
    string token = "";
    token += temp[k];
    k++;
    if (k < temp.size() && temp[k] == '=') //*=
    {
        token += '=';
        k++;
        token_table.push_back(token); // 加入单词表
        token_type.push_back("运算符:除等于");
        token_row
            .push_back(rowNum + k - begin);
        if (count(symbol_table.begin(), symbol_table.end(), token) == 0) // 加入符号表，需要去重
        {
            symbol_table.push_back(token);
            symbol_type.push_back("运算符:除等于");
            symbol_row.push_back(rowNum + k - begin);
        }
    }
    else // /
    {
        token_table.push_back(token); // 加入单词表
        token_type.push_back("运算符:除");
        token_row
            .push_back(rowNum + k - begin);
        if (count(symbol_table.begin(), symbol_table.end(), token) == 0) // 加入符号表，需要去重
        {
            symbol_table.push_back(token);
            symbol_type.push_back("运算符:除");
            symbol_row.push_back(rowNum + k - begin);
        }
    }
    return k - begin;
}

int classOpNotEqual(int k, int rowNum, string temp) //! !=
{
    int begin = k;
    string token = "";
    token += temp[k];
    k++;
    if (k < temp.size() && temp[k] == '=') //!=
    {
        token += '=';
        k++;
        token_table.push_back(token); // 加入单词表
        token_type.push_back("运算符:不等于");
        token_row
            .push_back(rowNum + k - begin);
        if (count(symbol_table.begin(), symbol_table.end(), token) == 0) // 加入符号表，需要去重
        {
            symbol_table.push_back(token);
            symbol_type.push_back("运算符:不等于");
            symbol_row.push_back(rowNum + k - begin);
        }
    }
    else // !
    {
        token_table.push_back(token); // 加入单词表
        token_type.push_back("运算符:逻辑非");
        token_row
            .push_back(rowNum + k - begin);
        if (count(symbol_table.begin(), symbol_table.end(), token) == 0) // 加入符号表，需要去重
        {
            symbol_table.push_back(token);
            symbol_type.push_back("运算符:逻辑非");
            symbol_row.push_back(rowNum + k - begin);
        }
    }
    return k - begin;
}

int classOpLeq(int k, int rowNum, string temp) //< <=
{
    int begin = k;
    string token = "";
    token += temp[k];
    k++;
    if (k < temp.size() && temp[k] == '=') //<=
    {
        token += '=';
        k++;
        token_table.push_back(token); // 加入单词表
        token_type.push_back("运算符:小于等于");
        token_row
            .push_back(rowNum + k - begin);
        if (count(symbol_table.begin(), symbol_table.end(), token) == 0) // 加入符号表，需要去重
        {
            symbol_table.push_back(token);
            symbol_type.push_back("运算符:小于等于");
            symbol_row.push_back(rowNum + k - begin);
        }
    }
    else // <
    {
        token_table.push_back(token); // 加入单词表
        token_type.push_back("运算符:小于");
        token_row
            .push_back(rowNum + k - begin);
        if (count(symbol_table.begin(), symbol_table.end(), token) == 0) // 加入符号表，需要去重
        {
            symbol_table.push_back(token);
            symbol_type.push_back("运算符:小于");
            symbol_row.push_back(rowNum + k - begin);
        }
    }
    return k - begin;
}

int classOpGeq(int k, int rowNum, string temp) //> >=
{
    int begin = k;
    string token = "";
    token += temp[k];
    k++;
    if (k < temp.size() && temp[k] == '=') //>=
    {
        token += '=';
        k++;
        token_table.push_back(token); // 加入单词表
        token_type.push_back("运算符:大于等于");
        token_row
            .push_back(rowNum + k - begin);
        if (count(symbol_table.begin(), symbol_table.end(), token) == 0) // 加入符号表，需要去重
        {
            symbol_table.push_back(token);
            symbol_type.push_back("运算符:大于等于");
            symbol_row.push_back(rowNum + k - begin);
        }
    }
    else // >
    {
        token_table.push_back(token); // 加入单词表
        token_type.push_back("运算符:大于");
        token_row
            .push_back(rowNum + k - begin);
        if (count(symbol_table.begin(), symbol_table.end(), token) == 0) // 加入符号表，需要去重
        {
            symbol_table.push_back(token);
            symbol_type.push_back("运算符:大于");
            symbol_row.push_back(rowNum + k - begin);
        }
    }
    return k - begin;
}

int classOpEqual(int k, int rowNum, string temp) //= ==
{
    int begin = k;
    string token = "";
    token += temp[k];
    k++;
    if (k < temp.size() && temp[k] == '=') //==
    {
        token += '=';
        k++;
        token_table.push_back(token); // 加入单词表
        token_type.push_back("运算符:恒等于");
        token_row
            .push_back(rowNum + k - begin);
        if (count(symbol_table.begin(), symbol_table.end(), token) == 0) // 加入符号表，需要去重
        {
            symbol_table.push_back(token);
            symbol_type.push_back("运算符:恒等于");
            symbol_row.push_back(rowNum + k - begin);
        }
    }
    else // =
    {
        token_table.push_back(token); // 加入单词表
        token_type.push_back("运算符:等于");
        token_row
            .push_back(rowNum + k - begin);
        if (count(symbol_table.begin(), symbol_table.end(), token) == 0) // 加入符号表，需要去重
        {
            symbol_table.push_back(token);
            symbol_type.push_back("运算符:等于");
            symbol_row.push_back(rowNum + k - begin);
        }
    }
    return k - begin;
}

int classOpAnd(int k, int rowNum, string temp) //& &&
{
    int begin = k;
    string token = "";
    token += temp[k];
    k++;
    if (k < temp.size() && temp[k] == '=') //&&
    {
        token += '=';
        k++;
        token_table.push_back(token); // 加入单词表
        token_type.push_back("运算符:逻辑与");
        token_row
            .push_back(rowNum + k - begin);
        if (count(symbol_table.begin(), symbol_table.end(), token) == 0) // 加入符号表，需要去重
        {
            symbol_table.push_back(token);
            symbol_type.push_back("运算符:逻辑与");
            symbol_row.push_back(rowNum + k - begin);
        }
    }
    else // &
    {
        token_table.push_back(token); // 加入单词表
        token_type.push_back("运算符:按位与");
        token_row
            .push_back(rowNum + k - begin);
        if (count(symbol_table.begin(), symbol_table.end(), token) == 0) // 加入符号表，需要去重
        {
            symbol_table.push_back(token);
            symbol_type.push_back("运算符:按位与");
            symbol_row.push_back(rowNum + k - begin);
        }
    }
    return k - begin;
}

int classOpOr(int k, int rowNum, string temp) // | ||
{
    int begin = k;
    string token = "";
    token += temp[k];
    k++;
    if (k < temp.size() && temp[k] == '=') // ||
    {
        token += '=';
        k++;
        token_table.push_back(token); // 加入单词表
        token_type.push_back("运算符:逻辑或");
        token_row
            .push_back(rowNum + k - begin);
        if (count(symbol_table.begin(), symbol_table.end(), token) == 0) // 加入符号表，需要去重
        {
            symbol_table.push_back(token);
            symbol_type.push_back("运算符:逻辑或");
            symbol_row.push_back(rowNum + k - begin);
        }
    }
    else // |
    {
        token_table.push_back(token); // 加入单词表
        token_type.push_back("运算符:按位或");
        token_row
            .push_back(rowNum + k - begin);
        if (count(symbol_table.begin(), symbol_table.end(), token) == 0) // 加入符号表，需要去重
        {
            symbol_table.push_back(token);
            symbol_type.push_back("运算符:按位或");
            symbol_row.push_back(rowNum + k - begin);
        }
    }
    return k - begin;
}

bool classDeComma(int k, int rowNum, string temp) // ,
{
    string token = "";
    token += temp[k];
    k++;
    token_table.push_back(token); // 加入单词表
    token_type.push_back("界符:逗号");
    token_row
        .push_back(rowNum + 1);
    if (count(symbol_table.begin(), symbol_table.end(), token) == 0) // 加入符号表，需要去重
    {
        symbol_table.push_back(token);
        symbol_type.push_back("界符:逗号");
        symbol_row.push_back(rowNum + 1);
    }
    return true;
}

bool classDeSemicolon(int k, int rowNum, string temp) // ;
{
    string token = "";
    token += temp[k];
    k++;
    token_table.push_back(token); // 加入单词表
    token_type.push_back("界符:分号");
    token_row
        .push_back(rowNum + 1);
    if (count(symbol_table.begin(), symbol_table.end(), token) == 0) // 加入符号表，需要去重
    {
        symbol_table.push_back(token);
        symbol_type.push_back("界符:分号");
        symbol_row.push_back(rowNum + 1);
    }
    return true;
}

bool classDeColon(int k, int rowNum, string temp) // :
{
    string token = "";
    token += temp[k];
    k++;
    token_table.push_back(token); // 加入单词表
    token_type.push_back("界符:冒号");
    token_row
        .push_back(rowNum + 1);
    if (count(symbol_table.begin(), symbol_table.end(), token) == 0) // 加入符号表，需要去重
    {
        symbol_table.push_back(token);
        symbol_type.push_back("界符:冒号");
        symbol_row.push_back(rowNum + 1);
    }
    return true;
}

bool classDeLeftCurlyBra(int k, int rowNum, string temp) // {
{
    string token = "";
    token += temp[k];
    k++;
    token_table.push_back(token); // 加入单词表
    token_type.push_back("界符:左大括号");
    token_row
        .push_back(rowNum + 1);
    if (count(symbol_table.begin(), symbol_table.end(), token) == 0) // 加入符号表，需要去重
    {
        symbol_table.push_back(token);
        symbol_type.push_back("界符:左大括号");
        symbol_row.push_back(rowNum + 1);
    }
    return true;
}

bool classDeRightCurlyBra(int k, int rowNum, string temp) // }
{
    string token = "";
    token += temp[k];
    k++;
    token_table.push_back(token); // 加入单词表
    token_type.push_back("界符:右大括号");
    token_row
        .push_back(rowNum + 1);
    if (count(symbol_table.begin(), symbol_table.end(), token) == 0) // 加入符号表，需要去重
    {
        symbol_table.push_back(token);
        symbol_type.push_back("界符:右大括号");
        symbol_row.push_back(rowNum + 1);
    }
    return true;
}

bool classDeLeftParen(int k, int rowNum, string temp) // (
{
    string token = "";
    token += temp[k];
    k++;
    token_table.push_back(token); // 加入单词表
    token_type.push_back("界符:左小括号");
    token_row
        .push_back(rowNum + 1);
    if (count(symbol_table.begin(), symbol_table.end(), token) == 0) // 加入符号表，需要去重
    {
        symbol_table.push_back(token);
        symbol_type.push_back("界符:左小括号");
        symbol_row.push_back(rowNum + 1);
    }
    return true;
}

bool classDeRightParen(int k, int rowNum, string temp) // )
{
    string token = "";
    token += temp[k];
    k++;
    token_table.push_back(token); // 加入单词表
    token_type.push_back("界符:右小括号");
    token_row
        .push_back(rowNum + 1);
    if (count(symbol_table.begin(), symbol_table.end(), token) == 0) // 加入符号表，需要去重
    {
        symbol_table.push_back(token);
        symbol_type.push_back("界符:右小括号");
        symbol_row.push_back(rowNum + 1);
    }
    return true;
}

// 字符串判定
int classString(int k, int rowNum, string temp) //   ""
{
    int begin = k;
    string token = "";
    token += temp[k];
    k++;
    if (k < temp.size() && temp[k] != '"')
    {
        while (k < temp.size() && temp[k] != '"')
        {
            token += temp[k];
            k++;
        }
        k++;
        token += '"';
        token_table.push_back(token);
        token_type.push_back("字符串型常量");
        token_row
            .push_back(rowNum + k - begin);
        if (count(constant_table.begin(), constant_table.end(), token) == 0)
        {
            constant_table.push_back(token);
            constant_type.push_back("字符串型常量");
            constant_row.push_back(rowNum + k - begin);
        }
    }
    else if (k < temp.size() && temp[k] == '"')
    {
        k++;
        token_table.push_back(token);
        token_type.push_back("界符:双引号");
        token_row
            .push_back(rowNum + k - begin);
        if (count(symbol_table.begin(), symbol_table.end(), token) == 0)
        {
            symbol_table.push_back(token);
            symbol_type.push_back("界符:双引号");
            symbol_row.push_back(rowNum + k - begin);
        }
    }
    return k - begin;
}

int main()
{
    // code数组用来存放输入文件里的源代码
    string code[100];
    int lineNum = 0;

    // 读入源程序
    ifstream infile("test1.txt", ios::in);
    if (!infile.fail())
    {
        while (!infile.eof())
        {
            string line;
            getline(infile, line);
            code[lineNum] = line;
            lineNum++;
        }
    }
    infile.close();

    // 进入内循环
    for (int i = 0; i < lineNum; i++)
    {
        string temp = code[i];
        int k = 0;
        while (k < temp.size())
        {
            if (temp[k] == ' ') // 读到空格跳过
            {
                k++;
                rowNum++;
            }
            else if (isalpha(temp[k])) // 单词开头是字母
            {
                int mov = classLetter(k, rowNum, temp);
                rowNum += mov;
                k += mov;
            }
            else if (isdigit(temp[k]))
            {
                int mov = classNum(k, rowNum, temp);
                rowNum += mov;
                k += mov;
            }
            else if (temp[k] == '+')
            {
                classOpPlus(k, rowNum, temp);
                rowNum += 1;
                k += 1;
            }
            else if (temp[k] == '-')
            {
                classOpMinus(k, rowNum, temp);
                rowNum += 1;
                k += 1;
            }
            else if (temp[k] == '*')
            {
                classOpMul(k, rowNum, temp);
                rowNum += 1;
                k += 1;
            }
            else if (temp[k] == '/')
            {
                classOpDivi(k, rowNum, temp);
                rowNum += 1;
                k += 1;
            }
            else if (temp[k] == '!')
            {
                classOpNotEqual(k, rowNum, temp);
                rowNum += 1;
                k += 1;
            }
            else if (temp[k] == '<')
            {
                classOpLeq(k, rowNum, temp);
                rowNum += 1;
                k += 1;
            }
            else if (temp[k] == '>')
            {
                classOpGeq(k, rowNum, temp);
                rowNum += 1;
                k += 1;
            }
            else if (temp[k] == '=')
            {
                classOpEqual(k, rowNum, temp);
                rowNum += 1;
                k += 1;
            }
            else if (temp[k] == '&')
            {
                classOpAnd(k, rowNum, temp);
                rowNum += 1;
                k += 1;
            }
            else if (temp[k] == '|')
            {
                classOpOr(k, rowNum, temp);
                rowNum += 1;
                k += 1;
            }
            else if (temp[k] == ',')
            {
                classDeComma(k, rowNum, temp);
                rowNum += 1;
                k += 1;
            }
            else if (temp[k] == ':')
            {
                classDeColon(k, rowNum, temp);
                rowNum += 1;
                k += 1;
            }
            else if (temp[k] == ';')
            {
                classDeSemicolon(k, rowNum, temp);
                rowNum += 1;
                k += 1;
            }
            else if (temp[k] == '{')
            {
                classDeLeftCurlyBra(k, rowNum, temp);
                rowNum += 1;
                k += 1;
            }
            else if (temp[k] == '}')
            {
                classDeRightCurlyBra(k, rowNum, temp);
                rowNum += 1;
                k += 1;
            }
            else if (temp[k] == '(')
            {
                classDeLeftParen(k, rowNum, temp);
                rowNum += 1;
                k += 1;
            }
            else if (temp[k] == ')')
            {
                classDeRightParen(k, rowNum, temp);
                rowNum += 1;
                k += 1;
            }
            else if (temp[k] == '"')
            {
                int mov = classString(k, rowNum, temp);
                rowNum += mov;
                k += mov;
            }
        }
    }

    // 输出符号表
    cout << "                 下面输出符号表:                " << endl;
    cout << "                                                " << endl;
    cout << setw(10) << setfill(' ') << "符号名" << setw(20) << setfill(' ') << "单词符号类型" << setw(20) << setfill(' ') << "列计数" << endl;
    cout << "   ——————————————————————————————" << endl;
    for (int i = 0; i < symbol_table.size(); i++)
    {
        cout << setw(10) << setfill(' ') << symbol_table[i] << setw(30) << setfill(' ') << symbol_type[i] << setw(10) << setfill(' ') << symbol_row[i] << endl;
    }

    ofstream os1;
    os1.open("symbol_table.txt", ofstream::app);
    fstream file1("symbol_table.txt", ios::out);
    os1 << setw(20) << setfill(' ') << "符号名" << setw(20) << setfill(' ') << "单词符号类型" << setw(20) << setfill(' ') << "列计数" << endl;
    os1 << "——————————————————————————————" << endl;
    for (int i = 0; i < symbol_table.size(); i++)
    {
        os1 << setw(10) << setfill(' ') << symbol_table[i] << setw(30) << setfill(' ') << symbol_type[i] << setw(10) << setfill(' ') << symbol_row[i] << endl; // 写入数据
    }
    os1.close(); // 关闭文档

    // 输出常量表
    cout << "                                                " << endl;
    cout << "                                                " << endl;
    cout << "                                                " << endl;
    cout << "                                                " << endl;
    cout << "                 下面输出常量表:                " << endl;
    cout << "                                                " << endl;
    cout << setw(20) << setfill(' ') << "符号名" << setw(20) << setfill(' ') << "单词符号类型" << setw(20) << setfill(' ') << "列计数" << endl;
    cout << "   ——————————————————————————————" << endl;
    for (int i = 0; i < constant_table.size(); i++)
    {
        cout << setw(10) << setfill(' ') << constant_table[i] << setw(30) << setfill(' ') << constant_type[i] << setw(10) << setfill(' ') << constant_row[i] << endl;
    }

    ofstream os2;
    os2.open("constant_table.txt", ofstream::app);
    fstream file2("constant_table.txt", ios::out);
    os2 << setw(20) << setfill(' ') << "符号名" << setw(20) << setfill(' ') << "单词符号类型" << setw(20) << setfill(' ') << "列计数" << endl;
    os2 << "——————————————————————————————" << endl;
    for (int i = 0; i < constant_table.size(); i++)
    {
        os2 << setw(10) << setfill(' ') << constant_table[i] << setw(30) << setfill(' ') << constant_type[i] << setw(10) << setfill(' ') << constant_row[i] << endl; // 写入数据
    }
    os2.close(); // 关闭文档

    // 输出词法分析结果
    cout << "                                                " << endl;
    cout << "                                                " << endl;
    cout << "                                                " << endl;
    cout << "                                                " << endl;
    cout << "                 下面输出词法分析结果:                " << endl;
    cout << "                                                " << endl;
    cout << setw(20) << setfill(' ') << "符号名" << setw(20) << setfill(' ') << "单词符号类型" << setw(20) << setfill(' ') << "列计数" << endl;
    cout << "   ——————————————————————————————" << endl;
    for (int i = 0; i < token_table.size(); i++)
    {
        cout << setw(10) << setfill(' ') << token_table[i] << setw(30) << setfill(' ') << token_type[i] << setw(10) << setfill(' ') << token_row[i] << endl;
    }

    ofstream os3;
    os3.open("token_table.txt", ofstream::app);
    fstream file3("token_table.txt", ios::out);
    os3 << setw(20) << setfill(' ') << "符号名" << setw(20) << setfill(' ') << "单词符号类型" << setw(20) << setfill(' ') << "列计数" << endl;
    os3 << "——————————————————————————————" << endl;
    for (int i = 0; i < token_table.size(); i++)
    {
        os3 << setw(10) << setfill(' ') << token_table[i] << setw(30) << setfill(' ') << token_type[i] << setw(10) << setfill(' ') << token_row[i] << endl; // 写入数据file3
    }
    os3.close();

    cout << "                                                " << endl;
    cout << "                                                " << endl;
    cout << "                                                " << endl;
    cout << "分析结果已保存到文件夹中" << endl;
    return 0;
}
