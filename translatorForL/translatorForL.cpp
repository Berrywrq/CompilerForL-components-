// translatorForL.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include "iostream"
#include "string"
#include "fstream"
#include "sstream"
using namespace std;
#pragma warning(disable:4996)

//索引记录器
typedef struct recorder
{
	int tokennum;
	int symbolnum;
	int errornum;
	int currentline;
	int buffersize;//源代码缓冲区长度
	int bufferindex;//源代码缓冲区中当前读取字符的索引
	int pointnum;//一个实数中小数点数目，分析完当前实数后清零
	recorder() :tokennum(0), symbolnum(0), errornum(0), currentline(1), buffersize(0), bufferindex(0), pointnum(0) {}
};

//token
typedef struct token {
	int label;  //单词序号
	string name;  //单词本身
	int code;  //单词的机内码
	int addr;  //地址，单词为保留字时为-1，为标识符或常数时为大于0的数值，即在符号表中的入口地址。
};
//symbol 
typedef struct symbol
{
	int number;  //序号
	int type;  //类型
	string name;  //名字
};
//error
typedef struct error {
	int id;
	int line;
	string errInfo;
	error *next;
}*ptrerror;

//MachineCode
typedef struct machineCode {
	string id;
	string symbol;
};

//将代码文件读入缓冲区
void SetBuffer(string filename, char* &buffer, recorder* &recorder)
{
	fstream counter(filename);
	int count = 0;
	while (counter.get() != EOF)
	{
		count++;
	}
	counter.close();
	recorder->buffersize = count;
	buffer = new char[count];
	fstream reader(filename);
	int index = 0;
	while (index != count)
	{
		buffer[index] = reader.get();
		index++;
	}
	reader.close();
}

//从缓冲区获取字符
char GetChar(int index, char *buffer, recorder *recorder)
{
	return buffer[index];
}

//初始化机内码表
void InitialKeytab(machineCode *Keytab)
{
	fstream reader("../Debug/keytab.txt");
	char linebuffer[1024] = { 0 };
	Keytab[0].symbol = "";
	Keytab[0].id = "";
	int index = 1;
	while (!reader.eof())
	{
		reader.getline(linebuffer, sizeof(linebuffer));
		stringstream word(linebuffer);
		word >> Keytab[index].symbol;
		word >> Keytab[index].id;
		index++;
	}
	reader.close();
}

//判断单词首字符是否为字母
bool IsAlpha(string c)
{
	string alpha[26] = { "a","b","c","d","e","f","g","h","i","j",
		"k","l","m","n","o","p","q","r","s","t","u","v","w","x","y","z" };
	for (int i = 0; i < 26; i++)
	{
		if (c == alpha[i])
		{
			return true;
		}
	}
	return false;
}

//判断单词首字符是否为数字
bool IsDigit(string c)
{
	string digit[10] = { "0","1","2","3","4","5","6","7","8","9" };
	for (int i = 0; i < 10; i++)
	{
		if (c == digit[i])
		{
			return true;
		}
	}
	return false;
}

//判断单词首字符是否为空白符
bool IsBlank(string c)
{
	string blank[3] = { " ","\n","\t" };
	for (int i = 0; i < 3; i++)
	{
		if (c == blank[i])
		{
			return true;
		}
	}
	return false;
}

//判断是否为换行符
bool IsEnter(string c)
{
	if (c == "\n")
	{
		return true;
	}
	return false;
}

//判断是否为其他字符（算符界符）
bool IsOtherSymbol(string c)
{
	string symbol[13] = { "(",")","+","-","*","/",".",",",":",";","=","<",">" };
	for (int i = 0; i < 13; i++)
	{
		if (c == symbol[i])
		{
			return true;
		}
	}
	return false;
}

//判断是否为小数点
bool IsPoint(string c)
{
	if (c == ".")
	{
		return true;
	}
	return false;
}

//向错误信息链中插入一条记录
void RecordError(int line, string errorhead, ptrerror& ptrerror, recorder * recorder)
{
	int errornum = recorder->errornum;
	error *ptrrecord = ptrerror;
	if (errornum == 0)
	{
		ptrerror = new error;
		ptrerror->id = errornum + 1;
		ptrerror->line = line;
		ptrerror->errInfo = errorhead;
		ptrerror->next = NULL;
		recorder->errornum++;
	}
	else
	{
		for (int i = 0; i < errornum - 1; i++)
		{
			ptrrecord = ptrrecord->next;
		}
		ptrrecord->next = new error;
		ptrrecord = ptrrecord->next;
		ptrrecord->id = errornum + 1;
		ptrrecord->line = line;
		ptrrecord->errInfo = errorhead;
		ptrrecord->next = NULL;
		recorder->errornum++;
	}
}

//删除错误信息链
void DeleteError(ptrerror & ptrerror)
{
	if (ptrerror)
	{
		DeleteError(ptrerror->next);
		delete ptrerror;
	}
}

//输出错误信息链
void PrintError(ptrerror & ptrerror, recorder *recorder)
{
	error * ptrrecord = ptrerror;
	cout << endl;
	cout << "errors(" << recorder->errornum << ")." << endl;
	while (ptrrecord)
	{
		cout << ptrrecord->id << ":" << ptrrecord->errInfo << " at line " << ptrrecord->line << endl;
		ptrrecord = ptrrecord->next;
	}
}

//创建新文件
bool NewFile(string filepath)
{
	ofstream newfile(filepath);
	if (newfile)
	{
		newfile.close();
		return true;
	}
	else
	{
		newfile.close();
		return false;
	}
}

//int转string
string IntToString(int temp)
{
	stringstream s;
	s << temp;
	return s.str();
}

//string转int
int StringToInt(string temp)
{
	return atoi(temp.c_str());
}

//向token文件插入表项
void InsertToken(token token)
{
	fstream write;
	write.open("token.txt", ios::app | ios::out);
	write << token.label << "\t" << token.name << "\t" << token.code << "\t" << token.addr << "\r\n";
	write.close();
	return;
}

//向symbol文件插入表项
void InsertSymbol(symbol symbol)
{
	fstream write;
	write.open("symbol.txt", ios::app | ios::out);
	write << symbol.number << "\t" << symbol.type << "\t" << symbol.name << "\r\n";
	write.close();
	return;
}

//判断是否为关键字
string reserve(string word, machineCode *Keytab)
{
	for (int i = 1; i <= 17; i++)
	{
		if (word == Keytab[i].symbol)
		{
			return Keytab[i].id;
		}
	}
	return "0";
}

//判断是否为界符算符
string OperSymbol(string word, machineCode *Keytab)
{
	for (int i = 21; i <= 37; i++)
	{
		if (word == Keytab[i].symbol)
		{
			return Keytab[i].id;
		}
	}
	return "0";
}

//判断Symbol表里是否已经有该符号
int IsExistInSymbol(string word,string* symrecorder,recorder* recorder)
{
	for (int i = 1; i <= recorder->symbolnum; i++)
	{
		if (word==symrecorder[i])
		{
			return i;
		}
	}
	return 0;
}

//记录由字母和数字组成的合法单词
void RecordWord(string c, char *buffer, machineCode *Keytab, recorder* recorder, string* &symrecorder)
{
	string code;
	string word = "";
	int index = recorder->bufferindex;
	do
	{
		index++;
		word = word + c;
		c = GetChar(index, buffer, recorder);
		recorder->bufferindex++;
	} while (IsAlpha(c) || IsDigit(c));
	//在机内码表Keytab中匹配，若为关键字返回机内码，否则返回0
	code = reserve(word, Keytab);
	if (code != "0")
	{
		token token;
		token.label = (recorder->tokennum) + 1;
		token.name = word;
		token.code = StringToInt(code);
		token.addr = -1;
		InsertToken(token);
		recorder->tokennum++;
	}
	else
	{
		symbol symbol;
		token token;
		int addr = IsExistInSymbol(word, symrecorder, recorder);
		if (addr==0)//该符号未写入过
		{
			//插入symbol表
			symbol.number = (recorder->symbolnum) + 1;
			symbol.name = word;
			symbol.type = 18;
			InsertSymbol(symbol);
			recorder->symbolnum++;
			symrecorder[recorder->symbolnum] = word;
			//插入token表
			token.label = (recorder->tokennum) + 1;
			token.name = "id";
			token.code = 18;
			token.addr = recorder->symbolnum;
			InsertToken(token);
			recorder->tokennum++;
		}
		else//该符号为已写入过的符号
		{
			//插入token表
			token.label = (recorder->tokennum) + 1;
			token.name = "id";
			token.code = 18;
			token.addr = addr;
			InsertToken(token);
			recorder->tokennum++;
		}
	}
}

//记录由整数实数
void RecordNum(string c, char *buffer, machineCode *Keytab, recorder* recorder, ptrerror& ptrerror, string* &symrecorder)
{
	string word = "";
	int index = recorder->bufferindex;
	do
	{
		if (IsPoint(c))
		{
			recorder->pointnum++;
			if (recorder->pointnum == 2)
			{
				break;
			}
		}
		index++;
		word = word + c;
		c = GetChar(index, buffer, recorder);
		recorder->bufferindex++;
	} while (IsDigit(c) || IsPoint(c));
	if (IsAlpha(c) || IsPoint(c))
	{
		int line = recorder->currentline;
		string errorhead = "lexical error:常数中出现字母或多个小数点";
		RecordError(line, errorhead, ptrerror, recorder);
		do
		{
			index++;
			c = GetChar(index, buffer, recorder);
			recorder->bufferindex++;
		} while (IsDigit(c) || IsAlpha(c));
		//!IsBlank(c) && !IsOperator(c) && !IsEnter(c) && !IsDelimiter(c)
	}

	symbol symbol;
	token token;
	//判断是整型还是实型
	if (recorder->pointnum == 0)
	{
		//整型
		symbol.type = 19;
		token.code = 19;
	}
	else
	{
		//实型
		symbol.type = 20;
		token.code = 20;
		recorder->pointnum = 0;//抹去
	}
	int addr = IsExistInSymbol(word, symrecorder, recorder);
	if (addr == 0)//该符号未写入过
	{
		//插入symbol表
		symbol.number = (recorder->symbolnum) + 1;
		symbol.name = word;
		InsertSymbol(symbol);
		recorder->symbolnum++;
		symrecorder[recorder->symbolnum] = word;
		//插入token表
		token.label = (recorder->tokennum) + 1;
		token.name = "id";
		token.addr = recorder->symbolnum;
		InsertToken(token);
		recorder->tokennum++;
	}
	else//该符号为已写入过的符号
	{
		//插入token表
		token.label = (recorder->tokennum) + 1;
		token.name = "id";
		token.addr = addr;
		InsertToken(token);
		recorder->tokennum++;
	}
}

//记录其他单词（纯字符算符界符等）
void RecordOther(string c, char *buffer, machineCode *Keytab, recorder* recorder, ptrerror& ptrerror)
{
	string code;
	string word = "";
	int index = recorder->bufferindex;
	index++;
	word = word + c;
	c = GetChar(index, buffer, recorder);
	recorder->bufferindex++;
	if (IsOtherSymbol(c))
	{
		if (c == "=")
		{
			word = word + c;
			recorder->bufferindex++;
		}
		else if (c == ">")
		{
			word = word + c;
			recorder->bufferindex++;
		}
	}
	code = OperSymbol(word, Keytab);
	if (code != "0")
	{
		token token;
		token.label = (recorder->tokennum) + 1;
		token.name = word;
		token.code = StringToInt(code);
		token.addr = -1;
		InsertToken(token);
		recorder->tokennum++;
	}
	else//机内码表中不存在的双操作符单词
	{
		int line = recorder->currentline;
		string errorhead = "lexical error:非法操作符";
		RecordError(line, errorhead, ptrerror, recorder);
	}

}

//首字符为空白符的处理方法
void Blank(recorder * recorder, string c)
{
	if (c == "\n")
	{
		recorder->currentline++;
	}
	recorder->bufferindex++;
}

//单词开始字符分类器
void Sort(string c, char *buffer, machineCode *Keytab, recorder* recorder, ptrerror& ptrerror, string* &symrecorder)
{
	if (IsAlpha(c))
		RecordWord(c, buffer, Keytab, recorder, symrecorder);
	else if (IsDigit(c))
		RecordNum(c, buffer, Keytab, recorder, ptrerror, symrecorder);
	else if (IsOtherSymbol(c))
		RecordOther(c, buffer, Keytab, recorder, ptrerror);
	else if (IsBlank(c))
		Blank(recorder, c);
	else
	{
		int line = recorder->currentline;
		string errorhead = "lexical error:非法字符";
		RecordError(line, errorhead, ptrerror, recorder);
		recorder->bufferindex++;
	}
}

//单词开始字符抓取器
void ReadBufferByFirstChar(char* &buffer, machineCode* &Keytab, recorder* &recorder, ptrerror& ptrerror,string* &symrecorder)
{
	string c;
	while (recorder->bufferindex != recorder->buffersize)
	{
		c = buffer[recorder->bufferindex];
		Sort(c, buffer, Keytab, recorder, ptrerror,symrecorder);
	}
}

//代码阅读器
void Reader(char* &buffer, recorder* &recorder)
{
	string filepath;
	fstream detect;
	/*do
	{
		cout << "Input the file path here:" << endl;
		cin >> filepath;
		detect.open(filepath, ios::in);
		if (!detect)
		{
			cout << "文件不存在！" << endl;
		}
		else detect.close();
	} while (!detect);*/
	filepath = "../Debug/code.txt";
	SetBuffer(filepath, buffer, recorder);
}

int main()
{
	char *buffer = NULL;
	machineCode *Keytab = new machineCode[38];
	recorder *record = new recorder;
	ptrerror ptrerror = NULL;
	string *symrecorder =new string [50];//1 to record->symbolnum
	InitialKeytab(Keytab);
	Reader(buffer, record);
	ReadBufferByFirstChar(buffer, Keytab, record, ptrerror, symrecorder);
	PrintError(ptrerror, record);
	delete[] symrecorder;
	DeleteError(ptrerror);
	delete record;
	delete[]Keytab;
	delete[]buffer;
	system("pause");
	return 0;
}

