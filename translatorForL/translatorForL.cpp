// translatorForL.cpp : �������̨Ӧ�ó������ڵ㡣
//

#include "stdafx.h"
#include "iostream"
#include "string"
#include "fstream"
#include "sstream"
using namespace std;
#pragma warning(disable:4996)

//������¼��
typedef struct recorder
{
	int tokennum;
	int symbolnum;
	int errornum;
	int currentline;
	int buffersize;//Դ���뻺��������
	int bufferindex;//Դ���뻺�����е�ǰ��ȡ�ַ�������
	int pointnum;//һ��ʵ����С������Ŀ�������굱ǰʵ��������
	recorder() :tokennum(0), symbolnum(0), errornum(0), currentline(1), buffersize(0), bufferindex(0), pointnum(0) {}
};

//token
typedef struct token {
	int label;  //�������
	string name;  //���ʱ���
	int code;  //���ʵĻ�����
	int addr;  //��ַ������Ϊ������ʱΪ-1��Ϊ��ʶ������ʱΪ����0����ֵ�����ڷ��ű��е���ڵ�ַ��
};
//symbol 
typedef struct symbol
{
	int number;  //���
	int type;  //����
	string name;  //����
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

//�������ļ����뻺����
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

//�ӻ�������ȡ�ַ�
char GetChar(int index, char *buffer, recorder *recorder)
{
	return buffer[index];
}

//��ʼ���������
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

//�жϵ������ַ��Ƿ�Ϊ��ĸ
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

//�жϵ������ַ��Ƿ�Ϊ����
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

//�жϵ������ַ��Ƿ�Ϊ�հ׷�
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

//�ж��Ƿ�Ϊ���з�
bool IsEnter(string c)
{
	if (c == "\n")
	{
		return true;
	}
	return false;
}

//�ж��Ƿ�Ϊ�����ַ�����������
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

//�ж��Ƿ�ΪС����
bool IsPoint(string c)
{
	if (c == ".")
	{
		return true;
	}
	return false;
}

//�������Ϣ���в���һ����¼
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

//ɾ��������Ϣ��
void DeleteError(ptrerror & ptrerror)
{
	if (ptrerror)
	{
		DeleteError(ptrerror->next);
		delete ptrerror;
	}
}

//���������Ϣ��
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

//�������ļ�
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

//intתstring
string IntToString(int temp)
{
	stringstream s;
	s << temp;
	return s.str();
}

//stringתint
int StringToInt(string temp)
{
	return atoi(temp.c_str());
}

//��token�ļ��������
void InsertToken(token token)
{
	fstream write;
	write.open("token.txt", ios::app | ios::out);
	write << token.label << "\t" << token.name << "\t" << token.code << "\t" << token.addr << "\r\n";
	write.close();
	return;
}

//��symbol�ļ��������
void InsertSymbol(symbol symbol)
{
	fstream write;
	write.open("symbol.txt", ios::app | ios::out);
	write << symbol.number << "\t" << symbol.type << "\t" << symbol.name << "\r\n";
	write.close();
	return;
}

//�ж��Ƿ�Ϊ�ؼ���
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

//�ж��Ƿ�Ϊ������
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

//�ж�Symbol�����Ƿ��Ѿ��и÷���
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

//��¼����ĸ��������ɵĺϷ�����
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
	//�ڻ������Keytab��ƥ�䣬��Ϊ�ؼ��ַ��ػ����룬���򷵻�0
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
		if (addr==0)//�÷���δд���
		{
			//����symbol��
			symbol.number = (recorder->symbolnum) + 1;
			symbol.name = word;
			symbol.type = 18;
			InsertSymbol(symbol);
			recorder->symbolnum++;
			symrecorder[recorder->symbolnum] = word;
			//����token��
			token.label = (recorder->tokennum) + 1;
			token.name = "id";
			token.code = 18;
			token.addr = recorder->symbolnum;
			InsertToken(token);
			recorder->tokennum++;
		}
		else//�÷���Ϊ��д����ķ���
		{
			//����token��
			token.label = (recorder->tokennum) + 1;
			token.name = "id";
			token.code = 18;
			token.addr = addr;
			InsertToken(token);
			recorder->tokennum++;
		}
	}
}

//��¼������ʵ��
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
		string errorhead = "lexical error:�����г�����ĸ����С����";
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
	//�ж������ͻ���ʵ��
	if (recorder->pointnum == 0)
	{
		//����
		symbol.type = 19;
		token.code = 19;
	}
	else
	{
		//ʵ��
		symbol.type = 20;
		token.code = 20;
		recorder->pointnum = 0;//Ĩȥ
	}
	int addr = IsExistInSymbol(word, symrecorder, recorder);
	if (addr == 0)//�÷���δд���
	{
		//����symbol��
		symbol.number = (recorder->symbolnum) + 1;
		symbol.name = word;
		InsertSymbol(symbol);
		recorder->symbolnum++;
		symrecorder[recorder->symbolnum] = word;
		//����token��
		token.label = (recorder->tokennum) + 1;
		token.name = "id";
		token.addr = recorder->symbolnum;
		InsertToken(token);
		recorder->tokennum++;
	}
	else//�÷���Ϊ��д����ķ���
	{
		//����token��
		token.label = (recorder->tokennum) + 1;
		token.name = "id";
		token.addr = addr;
		InsertToken(token);
		recorder->tokennum++;
	}
}

//��¼�������ʣ����ַ��������ȣ�
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
	else//��������в����ڵ�˫����������
	{
		int line = recorder->currentline;
		string errorhead = "lexical error:�Ƿ�������";
		RecordError(line, errorhead, ptrerror, recorder);
	}

}

//���ַ�Ϊ�հ׷��Ĵ�����
void Blank(recorder * recorder, string c)
{
	if (c == "\n")
	{
		recorder->currentline++;
	}
	recorder->bufferindex++;
}

//���ʿ�ʼ�ַ�������
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
		string errorhead = "lexical error:�Ƿ��ַ�";
		RecordError(line, errorhead, ptrerror, recorder);
		recorder->bufferindex++;
	}
}

//���ʿ�ʼ�ַ�ץȡ��
void ReadBufferByFirstChar(char* &buffer, machineCode* &Keytab, recorder* &recorder, ptrerror& ptrerror,string* &symrecorder)
{
	string c;
	while (recorder->bufferindex != recorder->buffersize)
	{
		c = buffer[recorder->bufferindex];
		Sort(c, buffer, Keytab, recorder, ptrerror,symrecorder);
	}
}

//�����Ķ���
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
			cout << "�ļ������ڣ�" << endl;
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

