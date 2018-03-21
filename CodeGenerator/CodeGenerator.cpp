// CodeGenerator.cpp : �������̨Ӧ�ó������ڵ㡣
//

#include "stdafx.h"
#include "iostream"
#include "string"
#include "fstream"
#include "sstream"
#include "algorithm"
using namespace std;


//������¼��
typedef struct recorder
{
	int gennum;
	int genindex;
	int symbolnum;
	int blocknum;
	int codeindex;//��������Ĵ������ڵ�ַ
	int salistlength;
	recorder() :gennum(0), genindex(0), symbolnum(0), blocknum(0), codeindex(0), salistlength(0) {}
}*ptrrecorder;

//�Ĵ�����¼��
typedef struct Register
{
	string name;
	bool isUsing;
	string content;
	Register() :isUsing(false), content("") {}
}*ptrregister;

//VALUE������
typedef struct Value
{
	string value = "";
	Value * next = NULL;
}*ptrvalue;

//������Ϣ���Ծ��Ϣ��¼��
typedef struct standbyactive
{
	string standby;
	string active;
	standbyactive *next;
	standbyactive() :standby("unstandby"), active("unactive"), next(NULL) {}
}*ptrsa;

//�������¼��
typedef struct BasicBlock
{
	int begin;
	int end;
	int export1;
	int export2;
	int symbolnum;//��������
	string symbollist[10];//�����б�
	BasicBlock() :begin(0), end(0), export1(-1), export2(-1), symbolnum(0) {}
}*ptrblock;

//��Ԫʽ
typedef struct GenStruct
{
	int label;
	int code;//������
	int addr1;//������1�ڷ��ű��е���ڵ�ַ ��Ϊ�����Ϊ-1 ����Ϊ0��ʾ��Ϊ�յ��ڷ��ű���δ�ҵ��÷���
	int addr2;//������2�ڷ��ű��е���ڵ�ַ
	int result;//��������ڷ��ű��е���ڵ�ַ
	int out_port;  //��¼����Ԫʽ�Ƿ�Ϊһ�����������ڣ�����Ϊ1������Ϊ0
	int codebegin;//���ɴ������ʼ��ַ
	GenStruct() :label(0), code(0), addr1(0), addr2(0), result(0), out_port(0), codebegin(0) {}
}*ptrgen;

//���ű�
typedef struct Symbol
{
	int number;  //���
	int type;  //����
	string name; //����
}*ptrsymbol;

//Ŀ��������
typedef struct Code
{
	int index;
	string op;
	string opl;
	string comma;
	string opr;
	Code * next;
	Code() :index(0), op(""), opl(""), comma(""), opr(""), next(NULL) {}
}*ptrcode;

void test() {}

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

//��ʼ���Ĵ�����¼�� δ��
void InitReg(ptrregister &registers)
{
	registers[1].name = "ax";
	registers[2].name = "bx";
	registers[3].name = "cx";
	registers[4].name = "dx";
}

//��ȡ���ű��ļ�
void GetSymbol(ptrsymbol &ptrsymbol, ptrrecorder &recorder)
{
	string filepath;
	fstream detect;
	int count = 0;
	//�ж��ļ��Ƿ����
	/*do
	{
		cout << "Input the symbol file path here:" << endl;
		cin >> filepath;
		detect.open(filepath, ios::in);
		if (!detect)
		{
			cout << "�ļ������ڣ�" << endl;
		}
		else
		{*/
	filepath = "../Debug/outsymbol.txt";
	detect.open(filepath, ios::in);
	char line[16] = { 0 };
	while (!detect.eof())//���ܷ����ڴ�й¶�ĵط�
	{
		detect.getline(line, 32, '\n');
		count++;//TODO:����ӿ��ļ����һ����Ϣ���л��з�����Ҫ��һ�����˴������޻��з�
	}
	recorder->symbolnum = count - 1;
	detect.close();
	/*break;
}
} while (!detect);*/
//��ȡ���ű��ļ�
	ptrsymbol = new Symbol[count + 1];//���ű�洢����Ϊ1��count
	fstream reader(filepath);
	char linebuffer[16] = { 0 };
	for (int i = 1; i <= count; i++)
	{
		reader.getline(linebuffer, sizeof(linebuffer));
		stringstream word(linebuffer);
		word >> ptrsymbol[i].number;
		word >> ptrsymbol[i].type;
		word >> ptrsymbol[i].name;
	}
	reader.close();
}

//��ȡ�����ڷ��ű��е���ڵ�ַ
int GetSymbolAddr(ptrsymbol ptrsymbol, string s, ptrrecorder recorder)
{
	for (int i = 1; i <= recorder->gennum; i++)
	{
		if (s == ptrsymbol[i].name)
		{
			return ptrsymbol[i].number;
		}
	}
	return 0;
}

//��ȡ��Ԫʽ�ļ�
void GetQuaternary(ptrgen &ptrgen, ptrsymbol &ptrsymbol, ptrrecorder &recorder)
{
	string filepath;
	fstream detect;
	int count = 0;
	//�ж��ļ��Ƿ����
	/*do
	{
		cout << "Input the quaternary file path here:" << endl;
		cin >> filepath;
		detect.open(filepath, ios::in);
		if (!detect)
		{
			cout << "�ļ������ڣ�" << endl;
		}
		else
		{*/
	filepath = "../Debug/quadruple.txt";
	detect.open(filepath, ios::in);
	char line[16] = { 0 };
	while (!detect.eof())//���ܷ����ڴ�й¶�ĵط�
	{
		detect.getline(line, 32, '\n');
		count++;//TODO:����ӿ��ļ����һ����Ϣ���л��з�����Ҫ��һ�����˴������޻��з�
	}
	recorder->gennum = count-1;
	detect.close();
	//		break;
	//	}
	//} while (!detect);
	//��ȡ��Ԫʽ��ptrgen������
	ptrgen = new GenStruct[count + 1];//��Ԫʽ�洢����Ϊ1��count
	fstream reader(filepath);
	char linebuffer[16] = { 0 };
	string a = "";
	string b = "";
	string c = "";
	string d = "";
	for (int i = 1; i <= count; i++)
	{
		reader.getline(linebuffer, sizeof(linebuffer));
		stringstream word(linebuffer);
		word >> ptrgen[i].label;
		word >> a;
		word >> b;
		word >> c;
		word >> d;
		if (a == "51")
		{
			ptrgen[i].code = StringToInt(a);
			ptrgen[i].addr1 = GetSymbolAddr(ptrsymbol, b, recorder);
			ptrgen[i].addr2 = -1;
			ptrgen[i].result = GetSymbolAddr(ptrsymbol, d, recorder);
		}
		else if (a == "52")
		{
			ptrgen[i].code = StringToInt(a);
			ptrgen[i].addr1 = -1;
			ptrgen[i].addr2 = -1;
			ptrgen[i].result = StringToInt(d);
		}
		else if (StringToInt(a) > 40 && StringToInt(a) < 50)
		{
			ptrgen[i].code = StringToInt(a);
			ptrgen[i].addr1 = GetSymbolAddr(ptrsymbol, b, recorder);
			ptrgen[i].addr2 = GetSymbolAddr(ptrsymbol, c, recorder);
			ptrgen[i].result = GetSymbolAddr(ptrsymbol, d, recorder);
		}
		else
		{
			ptrgen[i].code = StringToInt(a);
			ptrgen[i].addr1 = GetSymbolAddr(ptrsymbol, b, recorder);
			ptrgen[i].addr2 = GetSymbolAddr(ptrsymbol, c, recorder);
			ptrgen[i].result = StringToInt(d);
		}
	}
	reader.close();
}

//�����������
void SetEntrance(ptrgen &ptrgen, ptrrecorder &recorder, int* entrance)
{
	ptrgen[1].out_port = 1;
	entrance[0] = 1;
	int count = 1;
	for (int i = 1; i <= recorder->gennum; i++)
	{
		if (ptrgen[i].code >= 53 && ptrgen[i].code <= 58)
		{
			if (ptrgen[i + 1].out_port == 0)//֮ǰδ��⵽���˾��������� ��ֹ�ظ�
			{
				ptrgen[i + 1].out_port = 1;
				entrance[count] = i + 1;
				count++;
			}
			if (ptrgen[ptrgen[i].result].out_port == 0)
			{
				ptrgen[ptrgen[i].result].out_port = 1;
				entrance[count] = ptrgen[i].result;
				count++;
			}
		}
		else if (ptrgen[i].code == 52)
		{
			if (ptrgen[i].result == 0)
			{
				continue;
			}
			if (ptrgen[ptrgen[i].result].out_port == 0)
			{
				ptrgen[ptrgen[i].result].out_port = 1;
				entrance[count] = ptrgen[i].result;
				count++;
			}
		}
	}
	recorder->blocknum = count;
	sort(entrance, entrance + count);
}

//�����������������ҵ�������������
int FindBlock(int entrindex, int* entrance, ptrrecorder recorder)
{
	for (int i = 0; i < recorder->blocknum; i++)
	{
		if (entrance[i] == entrindex)
		{
			return i;
		}
	}
	return -1;
}

//������������
void SetBasicBlock(ptrrecorder &recorder, ptrblock &ptrblock, int *entrance, ptrgen ptrgen)
{
	//���ɻ�����
	ptrblock = new BasicBlock[recorder->blocknum];
	for (int i = 0; i < recorder->blocknum - 1; i++)
	{
		ptrblock[i].begin = entrance[i];
		ptrblock[i].end = entrance[i + 1] - 1;
	}
	ptrblock[recorder->blocknum - 1].begin = entrance[recorder->blocknum - 1];
	ptrblock[recorder->blocknum - 1].end = recorder->gennum;
	//���ӻ�����
	int currentblock = -1;//��ǰ������
	int nextblock1 = -1;//��һ������1
	int nextblock2 = -1;//��һ������2
	for (int i = 1; i <= recorder->gennum; i++)//������������ת�Ʒָ�
	{
		if (ptrgen[i].code == 52)
		{
			currentblock = FindBlock(ptrgen[i].label + 1, entrance, recorder) - 1;
			nextblock1 = FindBlock(ptrgen[i].result, entrance, recorder);
			ptrblock[currentblock].export1 = nextblock1;
		}
		if (ptrgen[i].code >= 53 && ptrgen[i].code <= 58)
		{
			//TODO:����ת�����Ļ�����������������
			currentblock = FindBlock(ptrgen[i].label + 1, entrance, recorder) - 1;
			nextblock1 = currentblock + 1;
			nextblock2 = FindBlock(ptrgen[i].result, entrance, recorder);
			ptrblock[currentblock].export1 = nextblock1;
			ptrblock[currentblock].export2 = nextblock2;
		}
	}
	for (int i = 1; i < recorder->blocknum; i++)//��ĩ����ת����䣬�����һ��
	{
		if (ptrgen[entrance[i] - 1].code < 52)
		{
			currentblock = i - 1;
			nextblock1 = i;
			ptrblock[currentblock].export1 = nextblock1;
		}
	}
}

//������ڵ�ַ�ڷ��ű��б��������ظ÷���
string FindSymbol(int addr, ptrsymbol ptrsymbol)
{
	return ptrsymbol[addr].name;
}

//ȷ���������еķ����б�������1��symbolnum��
void SetBlockSymbol(ptrrecorder &recorder, ptrblock &ptrblock, ptrgen ptrgen, ptrsymbol ptrsymbol)
{
	for (int i = 0; i < recorder->blocknum; i++)
	{
		int symbolnum = 0;
		for (int j = ptrblock[i].begin; j <= ptrblock[i].end; j++)
		{
			//���Լ�����Ԫʽ��arg1
			bool exist = false;
			for (int k = 1; k < symbolnum + 1; k++)
			{
				if (ptrgen[j].code == 52)
				{
					exist = true;//ֻ�Ǳ�ʾ����Ҫ�ٲ������������б�
					break;
				}
				if (ptrblock[i].symbollist[k] == FindSymbol(ptrgen[j].addr1, ptrsymbol))
				{
					exist = true;
					break;
				}
			}
			if (!exist&&ptrgen[j].code != 52)
			{
				ptrblock[i].symbollist[symbolnum + 1] = FindSymbol(ptrgen[j].addr1, ptrsymbol);
				symbolnum++;
			}
			exist = false;
			//���Լ�����Ԫʽ��arg2
			for (int k = 1; k < symbolnum + 1; k++)
			{
				if (ptrgen[j].code == 52 || ptrgen[j].code == 51)
				{
					exist = true;//ֻ�Ǳ�ʾ����Ҫ�ٲ������������б�
					break;
				}
				if (ptrblock[i].symbollist[k] == FindSymbol(ptrgen[j].addr2, ptrsymbol))
				{
					exist = true;
					break;
				}
			}
			if (!exist&&ptrgen[j].code != 52 && ptrgen[j].code != 51)
			{
				ptrblock[i].symbollist[symbolnum + 1] = FindSymbol(ptrgen[j].addr2, ptrsymbol);
				symbolnum++;
			}
			exist = false;
			//���Լ�����Ԫʽ��result
			for (int k = 1; k < symbolnum + 1; k++)
			{
				if (ptrgen[j].code >= 52 && ptrgen[j].code <= 58)
				{
					exist = true;//ֻ�Ǳ�ʾ����Ҫ�ٲ������������б�
					break;
				}
				if (ptrblock[i].symbollist[k] == FindSymbol(ptrgen[j].result, ptrsymbol))
				{
					exist = true;
					break;
				}
			}
			if (!exist&&ptrgen[j].code >= 41 && ptrgen[j].code <= 51)
			{
				ptrblock[i].symbollist[symbolnum + 1] = FindSymbol(ptrgen[j].result, ptrsymbol);
				symbolnum++;
			}
		}
		ptrblock[i].symbolnum = symbolnum;//������ķ����б�����Ϊ��1��symbolnum
	}
}

//���ݷ��źͷ������ڻ����������ҵ����ڻ����������Ϣ���е�����
int FindSymbolIndex(string s, int blockindex, ptrblock ptrblock)
{
	for (int i = 1; i <= ptrblock[blockindex].symbolnum; i++)
	{
		if (ptrblock[blockindex].symbollist[i] == s)
		{
			return i;
		}
	}
	return 0;
}

//���û�Ծ��¼��������(�����Ϣ��)
void NewNode(ptrsa &salist, string standby, string active)
{
	ptrsa ptrrecord = salist;
	if (!ptrrecord)
	{
		salist = new standbyactive;
		salist->standby = standby;
		salist->active = active;
	}
	else
	{
		while (ptrrecord->next)
		{
			ptrrecord = ptrrecord->next;
		}
		ptrrecord->next = new standbyactive;
		//ptrrecord = ptrrecord->next;//test
		ptrrecord->standby = standby;//TODO:Important
		ptrrecord->active = active;
	}
}

//������Ϣ���ͻ�Ծ��Ϣ������
void SetSa(ptrsa* &salist, ptrsa* &satable, ptrgen ptrgen, ptrsymbol ptrsymbol, ptrrecorder& recorder, ptrblock ptrblock)
{
	//�����б��ȳ�ʼ��
	int length = 0;
	for (int i = 0; i < recorder->blocknum; i++)
	{
		length += ptrblock[i].symbolnum;
	}
	recorder->salistlength = length;
	salist = new ptrsa[length + 1]{ NULL };
	/*for (int p = 1; p <=length; p++)
	{
		NewNode(salist[p],"unstandby","unactive");
	}*/
	for (int i = 0; i < recorder->blocknum; i++)
	{
		for (int j = ptrblock[i].begin; j <= ptrblock[i].end; j++)
		{
			satable[j] = new standbyactive[ptrblock[i].symbolnum + 1];
		}
	}
	//��Ծ��Ϣ����һ���ڵ��ʼ��
	for (int i = 1; i <= length; i++)
	{
		NewNode(salist[i], "unstandby", "unactive");
	}
	//��һ��ɨ��
	for (int i = 0; i < recorder->blocknum; i++)
	{
		for (int j = ptrblock[i].end; j >= ptrblock[i].begin; j--)
		{
			//result
			if (ptrgen[j].code >= 41 && ptrgen[j].code <= 51)
			{
				int previousindex = 0;
				for (int k = 0; k <= i - 1; k++)
				{
					previousindex += ptrblock[k].symbolnum;
				}
				int symbolindex = FindSymbolIndex(FindSymbol(ptrgen[j].result, ptrsymbol), i, ptrblock);//��������
				ptrsa lastnode = salist[previousindex + symbolindex];
				while (lastnode->next)
				{
					lastnode = lastnode->next;
				}
				satable[j][symbolindex].standby = lastnode->standby;
				satable[j][symbolindex].active = lastnode->active;

				NewNode(salist[previousindex + symbolindex], "unstandby", "unactive");
			}
			//arg1
			if (ptrgen[j].code != 52)
			{
				int previousindex = 0;
				for (int k = 0; k <= i - 1; k++)
				{
					previousindex += ptrblock[k].symbolnum;
				}
				int symbolindex = FindSymbolIndex(FindSymbol(ptrgen[j].addr1, ptrsymbol), i, ptrblock);
				ptrsa lastnode = salist[previousindex + symbolindex];
				while (lastnode->next)
				{
					lastnode = lastnode->next;
				}
				satable[j][symbolindex].standby = lastnode->standby;
				satable[j][symbolindex].active = lastnode->active;

				NewNode(salist[previousindex + symbolindex], IntToString(j), "active");
			}
			//arg2
			if (ptrgen[j].code != 52 && ptrgen[j].code != 51)
			{
				int previousindex = 0;
				for (int k = 0; k <= i - 1; k++)
				{
					previousindex += ptrblock[k].symbolnum;
				}
				int symbolindex = FindSymbolIndex(FindSymbol(ptrgen[j].addr2, ptrsymbol), i, ptrblock);
				ptrsa lastnode = salist[previousindex + symbolindex];
				while (lastnode->next)
				{
					lastnode = lastnode->next;
				}
				satable[j][symbolindex].standby = lastnode->standby;
				satable[j][symbolindex].active = lastnode->active;

				NewNode(salist[previousindex + symbolindex], IntToString(j), "active");
			}
		}
	}
}

//���ݻ������¼������Ϣɨ��ĳ�������ڿ�����Ƿ����ø÷��ţ�У׼���ڱ�����Ծ��Ϣ
void SetExportActiveVariables(ptrsa* &salist, ptrsa* &satable, ptrgen ptrgen, ptrsymbol ptrsymbol, ptrrecorder& recorder, ptrblock ptrblock)
{
	for (int i = 0; i < recorder->blocknum - 1; i++)
	{
		bool find = false;
		for (int j = ptrblock[i].end; j >= ptrblock[i].begin; j--)
		{
			if (ptrgen[j].code >= 41 && ptrgen[j].code <= 51)
			{
				for (int k = 1; k <= ptrblock[i + 1].symbolnum; k++)
				{
					if (FindSymbol(ptrgen[j].result, ptrsymbol) == ptrblock[i + 1].symbollist[k])
					{
						int previousindex = 0;
						for (int m = 0; m <= i - 1; m++)
						{
							previousindex += ptrblock[m].symbolnum;
						}
						int symbolindex = FindSymbolIndex(FindSymbol(ptrgen[j].result, ptrsymbol), i, ptrblock);
						salist[previousindex + symbolindex]->active = "active";
						satable[j][symbolindex].active = "active";
						find = true;
						break;
					}
				}
			}
			if (find == true)
			{
				break;
			}
		}
	}
}

//����ʱ����֮��ķ�����avalue�о���ʼ����һ���ڵ�valueֵΪM
void Initavalue(ptrvalue* &avalue, ptrsymbol ptrsymbol, ptrrecorder recorder)
{
	for (int i = 1; i <= recorder->symbolnum; i++)
	{
		if (ptrsymbol[i].type != 0)
		{
			avalue[i] = new Value;
			avalue[i]->value = "M";
		}
	}
}

//�ж�ĳ�����Ƿ������ĳ�Ĵ����У����ظüĴ�������
int IsInRegister(ptrvalue* avalue, string Y, ptrsymbol ptrsymbol, ptrrecorder recorder)
{
	/*for (int i = 1; i <=2; i++)
	{
		ptrvalue ptrrecord = rvalue[i];
		while (ptrrecord)
		{
			if (ptrrecord->value==Y)
			{
				return i;
			}
			else
			{
				ptrrecord = ptrrecord->next;
			}
		}
	}
	return 0;*/
	ptrvalue ptrrecord = avalue[GetSymbolAddr(ptrsymbol, Y, recorder)];
	while (ptrrecord)
	{
		if (ptrrecord->value == "1" || ptrrecord->value == "2")
		{
			return StringToInt(ptrrecord->value);
		}
		else
		{
			ptrrecord = ptrrecord->next;
		}
	}
	return 0;
}

//�ж�ĳ�����Ĵ������Ƿ�ֻ��ĳָ������
bool IsOnlyExist(ptrvalue* rvalue, string Y, int regindex)
{
	if (rvalue[regindex]->value == Y&&rvalue[regindex]->next == NULL)
	{
		return true;
	}
	return false;
}

//�ж��Ƿ��п��мĴ��� ���з���������
int GetIdleReg(ptrvalue *rvalue)
{
	for (int i = 1; i <= 2; i++)
	{
		if (rvalue[i] == NULL)
		{
			return i;
		}
	}
	return 0;
}

//�ж�ĳ�����Ƿ����ڴ���
bool IsInMemory(ptrvalue* &avalue, ptrrecorder recorder, string Y, ptrsymbol ptrsymbol)
{
	ptrvalue ptrrecord = avalue[GetSymbolAddr(ptrsymbol, Y, recorder)];
	while (ptrrecord)
	{
		if (ptrrecord->value == "M")
		{
			return true;
		}
		ptrrecord = ptrrecord->next;
	}
	return false;
}

void GenMove(string opl, string opr, ptrrecorder &recorder, ptrcode &ptrcodelist);

//�Ĵ���������A ����41-51��Ԫʽ ���ؼĴ�������
int GetRegisterA(ptrvalue* &rvalue, ptrvalue* &avalue, ptrgen ptrgen, int genindex, int blockindex, ptrblock ptrblock, ptrsa* salist, ptrsa* satable, ptrsymbol ptrsymbol, ptrrecorder &recorder, ptrcode &ptrcodelist)
{
	//�����ԪʽX��=Y op Z
	string X = FindSymbol(ptrgen[genindex].result, ptrsymbol);
	string Y = FindSymbol(ptrgen[genindex].addr1, ptrsymbol);
	int registerindex = IsInRegister(avalue, Y, ptrsymbol, recorder);
	if (registerindex != 0 && IsOnlyExist(rvalue, Y, registerindex))
	{
		if (X == Y

			|| (satable[genindex][FindSymbolIndex(Y, blockindex, ptrblock)].standby == "unstandby"
				&&satable[genindex][FindSymbolIndex(Y, blockindex, ptrblock)].active == "unactive"))
		{
			return registerindex;
		}
	}
	else if ((registerindex = GetIdleReg(rvalue)) != 0)
	{
		return registerindex;
	}
	else
	{
		//ForRegister1
		ptrvalue ptrrecord = rvalue[1];
		while (ptrrecord)
		{
			if (ptrrecord->value != X&&!IsInMemory(avalue, recorder, ptrrecord->value, ptrsymbol))
			{
				GenMove("M", "BX", recorder, ptrcodelist);
				ptrgen[genindex].codebegin = recorder->codeindex;
				ptrvalue ptravalue = avalue[GetSymbolAddr(ptrsymbol, ptrrecord->value, recorder)];
				if (!ptravalue)
				{
					avalue[GetSymbolAddr(ptrsymbol, ptrrecord->value, recorder)] = new Value;
					avalue[GetSymbolAddr(ptrsymbol, ptrrecord->value, recorder)]->value = "M";
				}
				else
				{
					while (ptravalue->next)
					{
						ptravalue = ptravalue->next;
					}
					ptravalue = new Value;
					ptravalue->value = "M";
				}
				ptrrecord->value = "";//��Ҫɾ���Ľڵ��value��Ϊ�մ�
			}
			ptrrecord = ptrrecord->next;
		}
		return 1;
	}
}

//�Ĵ���������B ����52-58��Ԫʽ ���ؼĴ�������
int GetRegisterB(ptrvalue* &rvalue, ptrvalue* &avalue, ptrgen ptrgen, int genindex, int blockindex, ptrblock ptrblock, ptrsa* salist, ptrsa* satable, ptrsymbol ptrsymbol, ptrrecorder &recorder, ptrcode &ptrcodelist)
{
	//�������ת�����
	string Y = FindSymbol(ptrgen[genindex].addr1, ptrsymbol);
	int registerindex = IsInRegister(avalue, Y, ptrsymbol, recorder);
	if (registerindex != 0 && IsOnlyExist(rvalue, Y, registerindex))
	{
		return registerindex;
	}
	else if ((registerindex = GetIdleReg(rvalue)) != 0)
	{
		return registerindex;
	}
	else
	{
		//ForRegister1
		ptrvalue ptrrecord = rvalue[1];
		while (ptrrecord)
		{
			if (ptrrecord->value != Y&&!IsInMemory(avalue, recorder, ptrrecord->value, ptrsymbol))
			{
				GenMove("M", "DX", recorder, ptrcodelist);
				ptrgen[genindex].codebegin = recorder->codeindex;
				ptrvalue ptravalue = avalue[GetSymbolAddr(ptrsymbol, ptrrecord->value, recorder)];
				if (!ptravalue)
				{
					avalue[GetSymbolAddr(ptrsymbol, ptrrecord->value, recorder)] = new Value;
					avalue[GetSymbolAddr(ptrsymbol, ptrrecord->value, recorder)]->value = "M";
				}
				else
				{
					while (ptravalue->next)
					{
						ptravalue = ptravalue->next;
					}
					ptravalue = new Value;
					ptravalue->value = "M";
				}
				ptrrecord->value = "";//��Ҫɾ���Ľڵ��value��Ϊ�մ�
			}
			ptrrecord = ptrrecord->next;
		}
		return 1;
	}
}

//���Y��Z���д��λ��
string GetAddr(ptrvalue* &avalue, string s, ptrrecorder recorder, ptrsymbol ptrsymbol, int addr1regindex, bool isop2)
{
	if (isop2)
	{
		ptrvalue ptrrecord = avalue[GetSymbolAddr(ptrsymbol, s, recorder)];
		while (ptrrecord)
		{
			if (ptrrecord->value == "1" || ptrrecord->value == "2")
			{
				if (ptrrecord->value == "M"&&ptrrecord->value == IntToString(addr1regindex))
				{
					ptrrecord->value = "";
					return s;//���ر������������ڴ��е�ֵ
				}
				else
				{
					return ptrrecord->value;
				}
			}
			else if (ptrrecord->value == "M")
			{
				return "M";
			}
			ptrrecord = ptrrecord->next;
		}
	}
	else
	{
		ptrvalue ptrrecord = avalue[GetSymbolAddr(ptrsymbol, s, recorder)];
		while (ptrrecord)
		{
			if (ptrrecord->value == IntToString(addr1regindex))
			{
				ptrrecord->value = "";
				return IntToString(addr1regindex);
			}
			ptrrecord = ptrrecord->next;
		}
		return "0";
	}
}

//����ĳ��value��ռ�õ��ڴ�
void DeleteValue(ptrvalue value)
{
	ptrvalue record = value;
	if (record)
	{
		DeleteValue(record->next);
		delete record;
	}
}

void GenMove(string opl, string opr, ptrrecorder &recorder, ptrcode &ptrcodelist)
{
	/*fstream write;
	write.open("Asm.txt", ios::app | ios::out);
	write << (recorder->codeindex + 1) << "\t" << "MOV" << " " << opl << "," << opr << "\r\n";
	write.close();
	recorder->codeindex++;
	return;*/
	if (!ptrcodelist)
	{
		ptrcodelist = new Code;
		ptrcodelist->index = recorder->codeindex + 1;
		ptrcodelist->op = "MOV";
		ptrcodelist->opl = opl;
		ptrcodelist->comma = ",";
		ptrcodelist->opr = opr;
	}
	else
	{
		ptrcode record = ptrcodelist;
		while (record->next)
		{
			record = record->next;
		}
		record->next = new Code;
		record = record->next;
		record->index = recorder->codeindex + 1;
		record->op = "MOV";
		record->opl = opl;
		record->comma = ",";
		record->opr = opr;
	}
	recorder->codeindex++;
}

void GenOp(string opl, string opr, ptrrecorder &recorder, int opera, ptrcode &ptrcodelist)
{
	string op;
	switch (opera)
	{
	case 43:
		op = "ADD";
		break;
	case 45:
		op = "SUB";
		break;
	case 41:
		op = "MUL";
		break;
	case 48:
		op = "DIV";
		break;
	}
	/*fstream write;
	write.open("Asm.txt", ios::app | ios::out);
	write << (recorder->codeindex + 1) << "\t" << op << " " << opl << "," << opr << "\r\n";
	write.close();
	recorder->codeindex++;
	return;*/
	if (!ptrcodelist)
	{
		ptrcodelist = new Code;
		ptrcodelist->index = recorder->codeindex + 1;
		ptrcodelist->op = op;
		ptrcodelist->opl = opl;
		ptrcodelist->comma = ",";
		ptrcodelist->opr = opr;
	}
	else
	{
		ptrcode record = ptrcodelist;
		while (record->next)
		{
			record = record->next;
		}
		record->next = new Code;
		record = record->next;
		record->index = recorder->codeindex + 1;
		record->op = op;
		record->opl = opl;
		record->comma = ",";
		record->opr = opr;
	}
	recorder->codeindex++;
}

void GenCmp(string opl, string opr, ptrrecorder &recorder, ptrcode &ptrcodelist)
{
	/*fstream write;
	write.open("Asm.txt", ios::app | ios::out);
	write << (recorder->codeindex + 1) << "\t" << "CMP" << " " << opl << "," << opr << "\r\n";
	write.close();
	recorder->codeindex++;
	return;*/
	if (!ptrcodelist)
	{
		ptrcodelist = new Code;
		ptrcodelist->index = recorder->codeindex + 1;
		ptrcodelist->op = "CMP";
		ptrcodelist->opl = opl;
		ptrcodelist->comma = ",";
		ptrcodelist->opr = opr;
	}
	else
	{
		ptrcode record = ptrcodelist;
		while (record->next)
		{
			record = record->next;
		}
		record->next = new Code;
		record = record->next;
		record->index = recorder->codeindex + 1;
		record->op = "CMP";
		record->opl = opl;
		record->comma = ",";
		record->opr = opr;
	}
	recorder->codeindex++;
}

void GenJmp(string opl, string opr, ptrrecorder &recorder, ptrgen ptrgen, int genindex, ptrcode &ptrcodelist)
{
	int opera = ptrgen[genindex].code;
	int jmptogenindex = ptrgen[genindex].result;
	string op;
	switch (opera)
	{
	case 53:
		op = "JL";
		break;
	case 54:
		op = "JLE";
		break;
	case 55:
		op = "JNZ";
		break;
	case 56:
		op = "JZ";
		break;
	case 57:
		op = "JG";
		break;
	case 58:
		op = "JNL";
		break;
	}
	/*fstream write;
	write.open("Asm.txt", ios::app | ios::out);
	write << (recorder->codeindex + 1) << "\t" << op << " " << ptrgen[jmptogenindex].codebegin << "\r\n";
	write.close();
	recorder->codeindex++;
	return;*/
	if (!ptrcodelist)
	{
		ptrcodelist = new Code;
		ptrcodelist->index = recorder->codeindex + 1;
		ptrcodelist->op = op;
		ptrcodelist->opl = IntToString(jmptogenindex);
		ptrcodelist->comma = "";
		ptrcodelist->opr = "";
	}
	else
	{
		ptrcode record = ptrcodelist;
		while (record->next)
		{
			record = record->next;
		}
		record->next = new Code;
		record = record->next;
		record->index = recorder->codeindex + 1;
		record->op = op;
		record->opl = IntToString(jmptogenindex);
		record->comma = "";
		record->opr = "";
	}
	recorder->codeindex++;
}

void GenJ(ptrrecorder &recorder, ptrgen ptrgen, int genindex, ptrcode &ptrcodelist)
{
	/*int jmptogenindex = ptrgen[genindex].result;
	fstream write;
	write.open("Asm.txt", ios::app | ios::out);
	write << (recorder->codeindex + 1) << "\t" << "JMP" << " " << ptrgen[jmptogenindex].codebegin << "\r\n";
	write.close();
	recorder->codeindex++;
	return;*/
	int jmptogenindex = ptrgen[genindex].result;
	if (!ptrcodelist)
	{
		ptrcodelist = new Code;
		ptrcodelist->index = recorder->codeindex + 1;
		ptrcodelist->op = "JMP";
		ptrcodelist->opl = IntToString(jmptogenindex);
		ptrcodelist->comma = "";
		ptrcodelist->opr = "";
	}
	else
	{
		ptrcode record = ptrcodelist;
		while (record->next)
		{
			record = record->next;
		}
		record->next = new Code;
		record = record->next;
		record->index = recorder->codeindex + 1;
		record->op = "JMP";
		record->opl = IntToString(jmptogenindex);
		record->comma = "";
		record->opr = "";
	}
	recorder->codeindex++;
}

//���Ĵ�������תΪ�Ĵ�����
string IndexToReg(int regindex)
{
	if (regindex == 1)
	{
		return "BX";
	}
	else if (regindex == 2)
	{
		return "DX";
	}
}

//���ɷ�ת����Ԫʽ�Ĵ���
void GenCodeA(ptrvalue* &rvalue, ptrvalue* &avalue, ptrgen ptrgen, int genindex, int blockindex, ptrblock ptrblock, ptrsa* salist, ptrsa* satable, ptrsymbol ptrsymbol, ptrrecorder &recorder, ptrcode &ptrcodelist)
{
	//X:=Y op Z
	int regindex = 0;
	string op1 = FindSymbol(ptrgen[genindex].addr1, ptrsymbol);
	string op2 = FindSymbol(ptrgen[genindex].addr2, ptrsymbol);
	if (ptrgen[genindex].code >= 41 && ptrgen[genindex].code <= 51)
	{
		regindex = GetRegisterA(rvalue, avalue, ptrgen, genindex, blockindex, ptrblock, salist, satable, ptrsymbol, recorder, ptrcodelist);
	}
	else
	{
		regindex = GetRegisterB(rvalue, avalue, ptrgen, genindex, blockindex, ptrblock, salist, satable, ptrsymbol, recorder, ptrcodelist);
	}
	string addr1 = GetAddr(avalue, op1, recorder, ptrsymbol, regindex, false);//������1��ŵļĴ������ڴ�
	string addr2 = GetAddr(avalue, op2, recorder, ptrsymbol, regindex, true);
	if (addr1 == "0")
	{
		string reg = IndexToReg(regindex);
		GenMove(reg, op1, recorder, ptrcodelist);//�˷�����Ҫ�޸ı�ʾ���ɴ����ַ��ȫ�ֱ���
		if (ptrgen[genindex].codebegin == 0)
		{
			ptrgen[genindex].codebegin = recorder->codeindex;
		}
		GenOp(reg, addr2, recorder, ptrgen[genindex].code, ptrcodelist);
		GenMove(FindSymbol(ptrgen[genindex].result, ptrsymbol), reg, recorder, ptrcodelist);
	}
	else if (addr1 != "0")
	{
		string reg = IndexToReg(regindex);
		GenOp(reg, addr2, recorder, ptrgen[genindex].code, ptrcodelist);
		if (ptrgen[genindex].codebegin == 0)
		{
			ptrgen[genindex].codebegin = recorder->codeindex;
		}
		GenMove(FindSymbol(ptrgen[genindex].result, ptrsymbol), reg, recorder, ptrcodelist);

	}
	DeleteValue(avalue[ptrgen[genindex].result]);
	avalue[ptrgen[genindex].result] = new Value;
	avalue[ptrgen[genindex].result]->value = IntToString(regindex);
	DeleteValue(rvalue[regindex]);
	rvalue[regindex] = new Value;
	rvalue[regindex]->value = FindSymbol(ptrgen[genindex].result, ptrsymbol);

	if (satable[genindex][FindSymbolIndex(op1, blockindex, ptrblock)].active == "unactive")
	{
		ptrvalue record = rvalue[regindex];
		while (record)
		{
			if (record->value == op1)
			{
				record->value = "";
				break;
			}
			record = record->next;
		}
		record = avalue[ptrgen[genindex].addr1];
		while (record)
		{
			if (record->value == IntToString(regindex))
			{
				record->value = "";
				break;
			}
			record = record->next;
		}
	}
	ptrvalue record = avalue[ptrgen[genindex].addr2];
	while (record)
	{
		if ((record->value == "1" || record->value == "2") && satable[genindex][FindSymbolIndex(op2, blockindex, ptrblock)].active == "unactive")
		{
			ptrvalue rrecord = rvalue[StringToInt(record->value)];
			while (rrecord)
			{
				if (rrecord->value == op1)
				{
					rrecord->value = "";
					break;
				}
				rrecord = rrecord->next;
			}
			record->value == "";
			break;
		}
		record = record->next;
	}
	//���ɴ�����Ƿ��޸���rvalue��avalue��ֵ:��δ�޸�
}

//51:=
void GenCodeB(ptrvalue* &rvalue, ptrvalue* &avalue, ptrgen ptrgen, int genindex, int blockindex, ptrblock ptrblock, ptrsa* salist, ptrsa* satable, ptrsymbol ptrsymbol, ptrrecorder &recorder, ptrcode &ptrcodelist)
{
	int regindex = 0;
	string op1 = FindSymbol(ptrgen[genindex].addr1, ptrsymbol);
	if (ptrgen[genindex].code >= 41 && ptrgen[genindex].code <= 51)
	{
		regindex = GetRegisterA(rvalue, avalue, ptrgen, genindex, blockindex, ptrblock, salist, satable, ptrsymbol, recorder, ptrcodelist);
	}
	else
	{
		regindex = GetRegisterB(rvalue, avalue, ptrgen, genindex, blockindex, ptrblock, salist, satable, ptrsymbol, recorder, ptrcodelist);
	}
	string addr1 = GetAddr(avalue, op1, recorder, ptrsymbol, regindex, false);//������1��ŵļĴ������ڴ�
	if (addr1 == "0")
	{
		string reg = IndexToReg(regindex);
		GenMove(reg, op1, recorder, ptrcodelist);//�˷�����Ҫ�޸ı�ʾ���ɴ����ַ��ȫ�ֱ���
		if (ptrgen[genindex].codebegin == 0)
		{
			ptrgen[genindex].codebegin = recorder->codeindex;
		}
		GenMove(FindSymbol(ptrgen[genindex].result, ptrsymbol), reg, recorder, ptrcodelist);
	}
	else if (addr1 != "0")
	{
		string reg = IndexToReg(regindex);
		GenMove(FindSymbol(ptrgen[genindex].result, ptrsymbol), reg, recorder, ptrcodelist);
		if (ptrgen[genindex].codebegin == 0)
		{
			ptrgen[genindex].codebegin = recorder->codeindex;
		}
	}
	DeleteValue(avalue[ptrgen[genindex].result]);
	avalue[ptrgen[genindex].result] = new Value;
	avalue[ptrgen[genindex].result]->value = IntToString(regindex);
	DeleteValue(rvalue[regindex]);
	rvalue[regindex] = new Value;
	rvalue[regindex]->value = FindSymbol(ptrgen[genindex].result, ptrsymbol);

	if (satable[genindex][FindSymbolIndex(op1, blockindex, ptrblock)].active == "unactive")
	{
		ptrvalue record = rvalue[regindex];
		while (record)
		{
			if (record->value == op1)
			{
				record->value = "";
				break;
			}
			record = record->next;
		}
		record = avalue[ptrgen[genindex].addr1];
		while (record)
		{
			if (record->value == IntToString(regindex))
			{
				record->value = "";
				break;
			}
			record = record->next;
		}
	}

	//���ɴ�����Ƿ��޸���rvalue��avalue��ֵ:��δ�޸�
}

//>=53
void GenCodeC(ptrvalue* &rvalue, ptrvalue* &avalue, ptrgen ptrgen, int genindex, int blockindex, ptrblock ptrblock, ptrsa* salist, ptrsa* satable, ptrsymbol ptrsymbol, ptrrecorder &recorder, ptrcode &ptrcodelist)
{
	int regindex = 0;
	string op1 = FindSymbol(ptrgen[genindex].addr1, ptrsymbol);
	string op2 = FindSymbol(ptrgen[genindex].addr2, ptrsymbol);
	if (ptrgen[genindex].code >= 41 && ptrgen[genindex].code <= 51)
	{
		regindex = GetRegisterA(rvalue, avalue, ptrgen, genindex, blockindex, ptrblock, salist, satable, ptrsymbol, recorder, ptrcodelist);
	}
	else
	{
		regindex = GetRegisterB(rvalue, avalue, ptrgen, genindex, blockindex, ptrblock, salist, satable, ptrsymbol, recorder, ptrcodelist);
	}
	string addr1 = GetAddr(avalue, op1, recorder, ptrsymbol, regindex, false);//������1��ŵļĴ������ڴ�
	string addr2 = GetAddr(avalue, op2, recorder, ptrsymbol, regindex, true);
	if (addr1 == "0")
	{
		string reg = IndexToReg(regindex);
		GenMove(reg, op1, recorder, ptrcodelist);//�˷�����Ҫ�޸ı�ʾ���ɴ����ַ��ȫ�ֱ��� �׾�ǰҪ����
		if (ptrgen[genindex].codebegin == 0)
		{
			ptrgen[genindex].codebegin = recorder->codeindex;
		}
		GenCmp(reg, addr2, recorder, ptrcodelist);
		GenJmp(op1, op2, recorder, ptrgen, genindex, ptrcodelist);//�˷����в�ѯҪ��������Ԫʽ�����ɵĴ������ڵ�ַ
	}
	else if (addr1 != "0")
	{
		string reg = IndexToReg(regindex);
		GenCmp(reg, addr2, recorder, ptrcodelist);
		if (ptrgen[genindex].codebegin == 0)
		{
			ptrgen[genindex].codebegin = recorder->codeindex;
		}
		GenJmp(op1, op2, recorder, ptrgen, genindex, ptrcodelist);

	}
	DeleteValue(avalue[ptrgen[genindex].result]);
	avalue[ptrgen[genindex].result] = new Value;
	avalue[ptrgen[genindex].result]->value = IntToString(regindex);
	DeleteValue(rvalue[regindex]);
	rvalue[regindex] = new Value;
	rvalue[regindex]->value = FindSymbol(ptrgen[genindex].result, ptrsymbol);

	if (satable[genindex][FindSymbolIndex(op1, blockindex, ptrblock)].active == "unactive")
	{
		ptrvalue record = rvalue[regindex];
		while (record)
		{
			if (record->value == op1)
			{
				record->value = "";
				break;
			}
			record = record->next;
		}
		record = avalue[ptrgen[genindex].addr1];
		while (record)
		{
			if (record->value == IntToString(regindex))
			{
				record->value = "";
				break;
			}
			record = record->next;
		}
	}
	ptrvalue record = avalue[ptrgen[genindex].addr2];
	while (record)
	{
		if ((record->value == "1" || record->value == "2") && satable[genindex][FindSymbolIndex(op2, blockindex, ptrblock)].active == "unactive")
		{
			ptrvalue rrecord = rvalue[StringToInt(record->value)];
			while (rrecord)
			{
				if (rrecord->value == op1)
				{
					rrecord->value = "";
					break;
				}
				rrecord = rrecord->next;
			}
			record->value == "";
			break;
		}
		record = record->next;
	}
	//���ɴ�����Ƿ��޸���rvalue��avalue��ֵ:��δ�޸�
}

//52j
void GenCodeD(ptrgen ptrgen, int genindex, ptrrecorder &recorder, ptrcode &ptrcodelist)
{
	GenJ(recorder, ptrgen, genindex, ptrcodelist);//�˷����в�ѯҪ��������Ԫʽ�����ɵĴ������ڵ�ַ ͬʱҪ�޸ı�ʾ���ɴ����ַ��ȫ�ֱ��� �׾�ǰҪ����
	if (ptrgen[genindex].codebegin == 0)
	{
		ptrgen[genindex].codebegin = recorder->codeindex;
	}
}

//��Ԫʽ������
void Reader(ptrvalue* &rvalue, ptrvalue* &avalue, ptrgen ptrgen, ptrblock ptrblock, ptrsa* salist, ptrsa* satable, ptrsymbol ptrsymbol, ptrrecorder &recorder, ptrcode &ptrcodelist)
{
	for (int i = 1; i <= recorder->gennum; i++)
	{
		int blockindex;
		for (int j = 0; j <= recorder->blocknum; j++)
		{
			if (i >= ptrblock[j].begin&&i <= ptrblock[j].end)
			{
				blockindex = j;
				break;
			}
		}
		if (ptrgen[i].code >= 41 && ptrgen[i].code <= 48)
		{
			GenCodeA(rvalue, avalue, ptrgen, i, blockindex, ptrblock, salist, satable, ptrsymbol, recorder, ptrcodelist);
		}
		else if (ptrgen[i].code == 51)
		{
			GenCodeB(rvalue, avalue, ptrgen, i, blockindex, ptrblock, salist, satable, ptrsymbol, recorder, ptrcodelist);
		}
		else if (ptrgen[i].code == 52)
		{
			GenCodeD(ptrgen, i, recorder, ptrcodelist);
		}
		else
		{
			GenCodeC(rvalue, avalue, ptrgen, i, blockindex, ptrblock, salist, satable, ptrsymbol, recorder, ptrcodelist);
		}
	}
}

//����Ŀ�������ת���ĵ�ַ
void BackPatch(ptrcode &ptrcodelist, ptrrecorder recorder, ptrgen ptrgen)
{
	ptrcode record = ptrcodelist;
	while (record)
	{
		if (record->op == "JL" || record->op == "JLE" ||
			record->op == "JNZ" || record->op == "JZ" ||
			record->op == "JG" || record->op == "JNL" ||
			record->op == "JMP")
		{
			if (record->opl != "0")
			{
				record->opl = IntToString(ptrgen[StringToInt(record->opl)].codebegin);
			}
		}
		record = record->next;
	}
}

//������д���ļ�
void OutputCode(ptrcode &ptrcodelist)
{
	ptrcode record = ptrcodelist;
	fstream write;
	write.open("asm.txt", ios::app | ios::out);
	while (record)
	{
		write << record->index << "\t" << record->op << " " << record->opl << record->comma << record->opr << "\r\n";
		record = record->next;
	}
	write.close();
}

void DeleteCodeList(ptrcode ptrcodelist)
{
	ptrcode record = ptrcodelist;
	if (record)
	{
		DeleteCodeList(record->next);
		delete record;
	}
}

void DeleteRAValue(ptrvalue* value, ptrrecorder recorder, bool isrvalue)
{
	if (isrvalue)
	{
		for (int i = 0; i < 3; i++)
		{
			DeleteValue(value[i]);
		}
		delete[]value;
	}
	else
	{
		for (int i = 0; i <= recorder->symbolnum; i++)
		{
			DeleteValue(value[i]);
		}
		delete[]value;
	}

}

void DeleteSa(ptrsa sa)
{
	ptrsa record = sa;
	if (record)
	{
		DeleteSa(record->next);
		delete record;
	}
}

void DeleteSaList(ptrsa *salist, ptrrecorder recorder)
{
	int length = recorder->salistlength;
	for (int i = 0; i <= length; i++)
	{
		DeleteSa(salist[i]);
	}
	delete[]salist;
}

void DeleteSaTable(ptrsa *satable, ptrrecorder recorder)
{
	int length = recorder->gennum;
	for (int i = 1; i <= length; i++)
	{
		delete[]satable[i];
	}
	delete[]satable;
}

int main()
{
	ptrgen ptrgen = NULL;
	ptrsymbol ptrsymbol = NULL;
	ptrrecorder record = new recorder;

	GetSymbol(ptrsymbol, record);
	GetQuaternary(ptrgen, ptrsymbol, record);

	ptrsa *salist = NULL;//������1��symbolnum
	ptrsa *satable = new ptrsa[record->gennum + 1];//������1��gennum
	ptrblock ptrblock = NULL;
	ptrvalue* rvalue = new ptrvalue[3]{ NULL };//������1��ʼ
	ptrvalue* avalue = new ptrvalue[record->symbolnum + 1]{ NULL };//������1��ʼ
	int *entrance = new int[record->gennum]{ 0 };//�������
	ptrcode ptrcodelist = NULL;//Ŀ�������
	SetEntrance(ptrgen, record, entrance);
	SetBasicBlock(record, ptrblock, entrance, ptrgen);
	SetBlockSymbol(record, ptrblock, ptrgen, ptrsymbol);
	SetSa(salist, satable, ptrgen, ptrsymbol, record, ptrblock);
	SetExportActiveVariables(salist, satable, ptrgen, ptrsymbol, record, ptrblock);
	Initavalue(avalue, ptrsymbol, record);
	Reader(rvalue, avalue, ptrgen, ptrblock, salist, satable, ptrsymbol, record, ptrcodelist);
	BackPatch(ptrcodelist, record, ptrgen);
	OutputCode(ptrcodelist);

	DeleteRAValue(rvalue, record, true);
	DeleteRAValue(avalue, record, false);
	delete[]entrance;
	DeleteSaList(salist, record);
	DeleteSaTable(satable, record);
	delete[]ptrsymbol;
	delete record;
	delete[]ptrgen;
	DeleteCodeList(ptrcodelist);
	//delete[]ptrblock; //�˴���ʱ�׳��쳣 ����Ϊ�ڴ�������� δ�����
	system("pause");
	return 0;
}

