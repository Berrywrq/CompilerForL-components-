// CodeGenerator.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include "iostream"
#include "string"
#include "fstream"
#include "sstream"
#include "algorithm"
using namespace std;


//索引记录器
typedef struct recorder
{
	int gennum;
	int genindex;
	int symbolnum;
	int blocknum;
	int codeindex;//刚生成完的代码的入口地址
	int salistlength;
	recorder() :gennum(0), genindex(0), symbolnum(0), blocknum(0), codeindex(0), salistlength(0) {}
}*ptrrecorder;

//寄存器记录器
typedef struct Register
{
	string name;
	bool isUsing;
	string content;
	Register() :isUsing(false), content("") {}
}*ptrregister;

//VALUE链表结点
typedef struct Value
{
	string value = "";
	Value * next = NULL;
}*ptrvalue;

//待用信息与活跃信息记录器
typedef struct standbyactive
{
	string standby;
	string active;
	standbyactive *next;
	standbyactive() :standby("unstandby"), active("unactive"), next(NULL) {}
}*ptrsa;

//基本块记录器
typedef struct BasicBlock
{
	int begin;
	int end;
	int export1;
	int export2;
	int symbolnum;//符号数量
	string symbollist[10];//符号列表
	BasicBlock() :begin(0), end(0), export1(-1), export2(-1), symbolnum(0) {}
}*ptrblock;

//四元式
typedef struct GenStruct
{
	int label;
	int code;//操作码
	int addr1;//操作数1在符号表中的入口地址 若为空则记为-1 若记为0表示不为空但在符号表中未找到该符号
	int addr2;//操作数2在符号表中的入口地址
	int result;//结果变量在符号表中的入口地址
	int out_port;  //记录该四元式是否为一个基本块的入口，是则为1，否则为0
	int codebegin;//生成代码的起始地址
	GenStruct() :label(0), code(0), addr1(0), addr2(0), result(0), out_port(0), codebegin(0) {}
}*ptrgen;

//符号表
typedef struct Symbol
{
	int number;  //序号
	int type;  //类型
	string name; //名字
}*ptrsymbol;

//目标代码表项
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

//初始化寄存器记录器 未用
void InitReg(ptrregister &registers)
{
	registers[1].name = "ax";
	registers[2].name = "bx";
	registers[3].name = "cx";
	registers[4].name = "dx";
}

//读取符号表文件
void GetSymbol(ptrsymbol &ptrsymbol, ptrrecorder &recorder)
{
	string filepath;
	fstream detect;
	int count = 0;
	//判断文件是否存在
	/*do
	{
		cout << "Input the symbol file path here:" << endl;
		cin >> filepath;
		detect.open(filepath, ios::in);
		if (!detect)
		{
			cout << "文件不存在！" << endl;
		}
		else
		{*/
	filepath = "../Debug/outsymbol.txt";
	detect.open(filepath, ios::in);
	char line[16] = { 0 };
	while (!detect.eof())//可能发生内存泄露的地方
	{
		detect.getline(line, 32, '\n');
		count++;//TODO:如果接口文件最后一条信息后有换行符，需要减一处理，此处假设无换行符
	}
	recorder->symbolnum = count - 1;
	detect.close();
	/*break;
}
} while (!detect);*/
//读取符号表文件
	ptrsymbol = new Symbol[count + 1];//符号表存储区间为1到count
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

//获取符号在符号表中的入口地址
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

//读取四元式文件
void GetQuaternary(ptrgen &ptrgen, ptrsymbol &ptrsymbol, ptrrecorder &recorder)
{
	string filepath;
	fstream detect;
	int count = 0;
	//判断文件是否存在
	/*do
	{
		cout << "Input the quaternary file path here:" << endl;
		cin >> filepath;
		detect.open(filepath, ios::in);
		if (!detect)
		{
			cout << "文件不存在！" << endl;
		}
		else
		{*/
	filepath = "../Debug/quadruple.txt";
	detect.open(filepath, ios::in);
	char line[16] = { 0 };
	while (!detect.eof())//可能发生内存泄露的地方
	{
		detect.getline(line, 32, '\n');
		count++;//TODO:如果接口文件最后一条信息后有换行符，需要减一处理，此处假设无换行符
	}
	recorder->gennum = count-1;
	detect.close();
	//		break;
	//	}
	//} while (!detect);
	//读取四元式到ptrgen数组中
	ptrgen = new GenStruct[count + 1];//四元式存储区间为1到count
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

//入口语句分析器
void SetEntrance(ptrgen &ptrgen, ptrrecorder &recorder, int* entrance)
{
	ptrgen[1].out_port = 1;
	entrance[0] = 1;
	int count = 1;
	for (int i = 1; i <= recorder->gennum; i++)
	{
		if (ptrgen[i].code >= 53 && ptrgen[i].code <= 58)
		{
			if (ptrgen[i + 1].out_port == 0)//之前未检测到过此句是入口语句 防止重复
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

//根据入口语句索引号找到基本块索引号
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

//基本块生成器
void SetBasicBlock(ptrrecorder &recorder, ptrblock &ptrblock, int *entrance, ptrgen ptrgen)
{
	//生成基本块
	ptrblock = new BasicBlock[recorder->blocknum];
	for (int i = 0; i < recorder->blocknum - 1; i++)
	{
		ptrblock[i].begin = entrance[i];
		ptrblock[i].end = entrance[i + 1] - 1;
	}
	ptrblock[recorder->blocknum - 1].begin = entrance[recorder->blocknum - 1];
	ptrblock[recorder->blocknum - 1].end = recorder->gennum;
	//链接基本块
	int currentblock = -1;//当前块索引
	int nextblock1 = -1;//下一块索引1
	int nextblock2 = -1;//下一块索引2
	for (int i = 1; i <= recorder->gennum; i++)//无条件和条件转移分割
	{
		if (ptrgen[i].code == 52)
		{
			currentblock = FindBlock(ptrgen[i].label + 1, entrance, recorder) - 1;
			nextblock1 = FindBlock(ptrgen[i].result, entrance, recorder);
			ptrblock[currentblock].export1 = nextblock1;
		}
		if (ptrgen[i].code >= 53 && ptrgen[i].code <= 58)
		{
			//TODO:条件转移语句的基本块链接索引处理
			currentblock = FindBlock(ptrgen[i].label + 1, entrance, recorder) - 1;
			nextblock1 = currentblock + 1;
			nextblock2 = FindBlock(ptrgen[i].result, entrance, recorder);
			ptrblock[currentblock].export1 = nextblock1;
			ptrblock[currentblock].export2 = nextblock2;
		}
	}
	for (int i = 1; i < recorder->blocknum; i++)//块末语句非转移语句，除最后一块
	{
		if (ptrgen[entrance[i] - 1].code < 52)
		{
			currentblock = i - 1;
			nextblock1 = i;
			ptrblock[currentblock].export1 = nextblock1;
		}
	}
}

//根据入口地址在符号表中遍历并返回该符号
string FindSymbol(int addr, ptrsymbol ptrsymbol)
{
	return ptrsymbol[addr].name;
}

//确定基本块中的符号列表（索引从1到symbolnum）
void SetBlockSymbol(ptrrecorder &recorder, ptrblock &ptrblock, ptrgen ptrgen, ptrsymbol ptrsymbol)
{
	for (int i = 0; i < recorder->blocknum; i++)
	{
		int symbolnum = 0;
		for (int j = ptrblock[i].begin; j <= ptrblock[i].end; j++)
		{
			//尝试加入四元式的arg1
			bool exist = false;
			for (int k = 1; k < symbolnum + 1; k++)
			{
				if (ptrgen[j].code == 52)
				{
					exist = true;//只是表示不需要再插入基本块符号列表
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
			//尝试加入四元式的arg2
			for (int k = 1; k < symbolnum + 1; k++)
			{
				if (ptrgen[j].code == 52 || ptrgen[j].code == 51)
				{
					exist = true;//只是表示不需要再插入基本块符号列表
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
			//尝试加入四元式的result
			for (int k = 1; k < symbolnum + 1; k++)
			{
				if (ptrgen[j].code >= 52 && ptrgen[j].code <= 58)
				{
					exist = true;//只是表示不需要再插入基本块符号列表
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
		ptrblock[i].symbolnum = symbolnum;//基本块的符号列表索引为从1到symbolnum
	}
}

//根据符号和符号所在基本块索引找到其在基本块符号信息链中的索引
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

//待用活跃记录项生成器(针对信息链)
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

//待用信息链和活跃信息表构造器
void SetSa(ptrsa* &salist, ptrsa* &satable, ptrgen ptrgen, ptrsymbol ptrsymbol, ptrrecorder& recorder, ptrblock ptrblock)
{
	//两个列表长度初始化
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
	//活跃信息链第一个节点初始化
	for (int i = 1; i <= length; i++)
	{
		NewNode(salist[i], "unstandby", "unactive");
	}
	//第一遍扫描
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
				int symbolindex = FindSymbolIndex(FindSymbol(ptrgen[j].result, ptrsymbol), i, ptrblock);//块内索引
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

//根据基本块记录器的信息扫描某符号所在块后面是否引用该符号，校准出口变量活跃信息
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

//除临时变量之外的符号在avalue中均初始化第一个节点value值为M
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

//判断某变量是否存在于某寄存器中，返回该寄存器索引
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

//判断某给定寄存器中是否只含某指定符号
bool IsOnlyExist(ptrvalue* rvalue, string Y, int regindex)
{
	if (rvalue[regindex]->value == Y&&rvalue[regindex]->next == NULL)
	{
		return true;
	}
	return false;
}

//判断是否有空闲寄存器 若有返回其索引
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

//判断某符号是否在内存中
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

//寄存器分配器A 处理41-51四元式 返回寄存器索引
int GetRegisterA(ptrvalue* &rvalue, ptrvalue* &avalue, ptrgen ptrgen, int genindex, int blockindex, ptrblock ptrblock, ptrsa* salist, ptrsa* satable, ptrsymbol ptrsymbol, ptrrecorder &recorder, ptrcode &ptrcodelist)
{
	//针对四元式X：=Y op Z
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
				ptrrecord->value = "";//需要删除的节点的value置为空串
			}
			ptrrecord = ptrrecord->next;
		}
		return 1;
	}
}

//寄存器分配器B 处理52-58四元式 返回寄存器索引
int GetRegisterB(ptrvalue* &rvalue, ptrvalue* &avalue, ptrgen ptrgen, int genindex, int blockindex, ptrblock ptrblock, ptrsa* salist, ptrsa* satable, ptrsymbol ptrsymbol, ptrrecorder &recorder, ptrcode &ptrcodelist)
{
	//针对条件转移语句
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
				ptrrecord->value = "";//需要删除的节点的value置为空串
			}
			ptrrecord = ptrrecord->next;
		}
		return 1;
	}
}

//获得Y、Z现行存放位置
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
					return s;//返回变量本身，即用内存中的值
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

//回收某个value链占用的内存
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

//将寄存器索引转为寄存器名
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

//生成非转移四元式的代码
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
	string addr1 = GetAddr(avalue, op1, recorder, ptrsymbol, regindex, false);//操作数1存放的寄存器或内存
	string addr2 = GetAddr(avalue, op2, recorder, ptrsymbol, regindex, true);
	if (addr1 == "0")
	{
		string reg = IndexToReg(regindex);
		GenMove(reg, op1, recorder, ptrcodelist);//此方法中要修改表示生成代码地址的全局变量
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
	//生成代码后是否修改了rvalue和avalue的值:暂未修改
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
	string addr1 = GetAddr(avalue, op1, recorder, ptrsymbol, regindex, false);//操作数1存放的寄存器或内存
	if (addr1 == "0")
	{
		string reg = IndexToReg(regindex);
		GenMove(reg, op1, recorder, ptrcodelist);//此方法中要修改表示生成代码地址的全局变量
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

	//生成代码后是否修改了rvalue和avalue的值:暂未修改
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
	string addr1 = GetAddr(avalue, op1, recorder, ptrsymbol, regindex, false);//操作数1存放的寄存器或内存
	string addr2 = GetAddr(avalue, op2, recorder, ptrsymbol, regindex, true);
	if (addr1 == "0")
	{
		string reg = IndexToReg(regindex);
		GenMove(reg, op1, recorder, ptrcodelist);//此方法中要修改表示生成代码地址的全局变量 首句前要换行
		if (ptrgen[genindex].codebegin == 0)
		{
			ptrgen[genindex].codebegin = recorder->codeindex;
		}
		GenCmp(reg, addr2, recorder, ptrcodelist);
		GenJmp(op1, op2, recorder, ptrgen, genindex, ptrcodelist);//此方法中查询要调到的四元式所生成的代码的入口地址
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
	//生成代码后是否修改了rvalue和avalue的值:暂未修改
}

//52j
void GenCodeD(ptrgen ptrgen, int genindex, ptrrecorder &recorder, ptrcode &ptrcodelist)
{
	GenJ(recorder, ptrgen, genindex, ptrcodelist);//此方法中查询要调到的四元式所生成的代码的入口地址 同时要修改表示生成代码地址的全局变量 首句前要换行
	if (ptrgen[genindex].codebegin == 0)
	{
		ptrgen[genindex].codebegin = recorder->codeindex;
	}
}

//四元式分析器
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

//回填目标代码跳转到的地址
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

//将代码写入文件
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

	ptrsa *salist = NULL;//索引从1到symbolnum
	ptrsa *satable = new ptrsa[record->gennum + 1];//索引从1到gennum
	ptrblock ptrblock = NULL;
	ptrvalue* rvalue = new ptrvalue[3]{ NULL };//索引从1开始
	ptrvalue* avalue = new ptrvalue[record->symbolnum + 1]{ NULL };//索引从1开始
	int *entrance = new int[record->gennum]{ 0 };//入口语句表
	ptrcode ptrcodelist = NULL;//目标代码链
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
	//delete[]ptrblock; //此处有时抛出异常 可能为内存溢出问题 未做检查
	system("pause");
	return 0;
}

