#include <iostream>
#include <cmath>
#include <windows.h>
#include "question.h"
using namespace std;

#define P 25       //P为数码问题的格子总数(8,15,24数码问题分别对应 P = 9，16，25......)

class Node {
public:
	int panel[P];  //当前结点的数字排布状态
	int depth;     //当前节点的搜索深度
	int h;         //当前节点到目标结点的估计代价
	int previous = -1;    //当前结点的父节点编号
	int present = -1;     //结点自身的编号

	Node()
	{
		;
	}

	Node(int panel[], int depth)       //带参数构造函数，给结点设置  状态、深度、代价估计
	{
		for (int i = 0;i < P;i++)
		{
			this->panel[i] = panel[i];
		}
		this->depth = depth;
		this->h = this->Compute_h();
	}

	int Compute_h();          //用于计算当前状态到目标状态的估计代价

	bool Is_target();         //判断当前状态是否是目标状态

	bool Is_equal(Node n);    //判断当前结点状态和结点n的状态是否相同


	bool operator < (const Node& n) const    //重载比较大小运算符，以总代价f = h + depth 值作为比较标准
	{
		return 10 * h + depth < 10 * n.h + n.depth;
	}
	bool operator > (const Node& n) const
	{
		return 10 * h + depth > 10 * n.h + n.depth;
	}
	bool operator == (const Node& n) const
	{
		return 10 * h + depth == 10 * n.h + n.depth;
	}


	void operator = (const Node& n)          //重载赋值运算符
	{
		this->depth = n.depth;
		this->h = n.h;
		for (int i = 0;i < P;i++)
		{
			this->panel[i] = n.panel[i];
		}
		this->previous = n.previous;
		this->present = n.present;
	}
};


//定义全局变量

int start_panel[] = { 1,2,3,4,5,6,7,8,-1,10,11,12,13,9,15,16,17,19,14,20,21,22,18,23,24 };   //手动设置初始状态、目标状态的数码排布(根据P的值，从左到右、从上到下输入P个数；空白格的值置为-1)
int target_panel[] = { -1,7,10,2,12,13,6,5,3,8,9,16,1,18,19,14,22,17,4,11,15,20,23,21,24 };

Node target(target_panel, 0);   //目标状态
Node start(start_panel, 0);    //初始状态



int Node::Compute_h()  //调用Compute(),估计代价h = 每个“不在位”的数码离目标位置的距离之和
{
	return Compute(panel, target.panel, P);
}

bool Node::Is_target()    //判断当前结点的状态是否是目标状态target
{
	return Same(panel, target.panel, P);
}

bool Node::Is_equal(Node n)   //判断当前结点状态和结点n的状态是否相同
{
	return Same(panel, n.panel, P);
}


class ListNode {                            //open,closed表结点的数据结构
public:
	Node n;
	ListNode* next = nullptr;
	ListNode()
	{
		;
	}
	ListNode(Node n)
	{
		this->n = Node(n.panel, n.depth);
		this->n.previous = n.previous;
		this->n.present = n.present;
	}
};

//定义全局变量
ListNode* expand = nullptr;     //当前结点扩展得到的所有结点（扩展表）

ListNode* open = nullptr;       //open表头指针
ListNode* closed = nullptr;     //closed表

Node Array[100000];    //Array用于存放所有生成的结点；在打印路径的时候，根据每个结点的previous值在Array中找到相同下标的父节点
int Num = 1;          //给新生成的结点设置编号(start结点编号为0，其他的从1开始)


//对closed表的操作：搜索、删除、插入
ListNode* Search_closed(Node n)   //查找closed表中是否有和结点n状态相同的结点，返回指向这个结点的指针
{
	ListNode* ptr = closed;
	while (ptr != nullptr)
	{
		if (ptr->n.Is_equal(n))
		{
			return ptr;
		}
		ptr = ptr->next;
	}
	return nullptr;
}

void Delete_closed(ListNode* ptr)   //删除closed表中ptr指向的结点
{
	ListNode* p = closed;
	if (p == ptr)           //1.如果要删除表首结点
	{
		closed = closed->next;
	}
	else {
		for (;p != nullptr;p = p->next)  //2.其他情况
		{
			if (p->next == ptr)
			{
				break;
			}
		}
		p->next = ptr->next;
	}
}

void Insert_closed(Node n)    //头插法，插入closed表
{
	if (closed == nullptr)
	{
		closed = new ListNode(n);
	}
	else {
		ListNode* ptr = new ListNode(n);
		ptr->next = closed;
		closed = ptr;
	}
}




//对open表的操作：删除、插入
void Delete_open()          //由于open表维持升序排列，每次只要删除表首元素
{
	open = open->next;
}

void Insert_open(Node n)    //在保证open表有序的前提下将结点n插入
{
	if (open == nullptr)    //1.若open表为空
	{
		open = new ListNode(n);
		return;
	}
	else {
		if (open->n > n || open->n == n)  //2.若open表头结点的f值大于结点n
		{
			ListNode* ptr = new ListNode(n);
			ptr->next = open;
			open = ptr;
			return;
		}
		else {                           //3.在之后的结点中查找比结点n的f值更大的结点，将n插入其前面
			ListNode* ptr1 = open->next;
			ListNode* ptr2 = open;

			for (;ptr1 != nullptr;ptr1 = ptr1->next)
			{
				if (ptr1->n > n)
				{
					ListNode* ptr = new ListNode(n);
					ptr2->next = ptr;
					ptr->next = ptr1;
					return;
				}
				ptr2 = ptr2->next;
			}

			ptr2->next = new ListNode(n);  //4.如果没有比结点n的f值更大的了，就把n插在表尾
			return;
		}
	}
}


void help_print(int[]);

void Print_open()      //打印查看open表状态
{
	std::cout << "打印当前open表：" << "\n";
	ListNode* ptr = open;
	while (ptr != nullptr)
	{
		help_print(ptr->n.panel);
		ptr = ptr->next;
	}
}



//对扩展表expand的操作：插入、清空
void Insert_expand(Node n)    //头插法
{
	if (expand == nullptr)
	{
		expand = new ListNode(n);
	}
	else {
		ListNode* ptr = new ListNode(n);
		ptr->next = expand;
		expand = ptr;
	}
}

void Refresh_expand()      //清空expand表
{
	expand = nullptr;
}



//A*算法中的关键步骤：扩展新结点、处理扩展结点
void Expand(Node& n)    //对当前结点n进行扩展 【？需要并行化】
{
	Refresh_expand();   //每次扩展前清空扩展表

	int* direct = help_Expand(n.panel, P);  //获取当前结点的扩展情况

	for (int i = 0;i < 4;i++)   //检索4种可能的扩展方向
	{
		if (direct[i] != -1)    //若当前索引值不为-1，该方向扩展有效，开始生成新的扩展结点：
		{
			int array[P];
			for (int j = 0;j < P;j++)  //1.首先把父节点n的数码排布复制一份
			{
				array[j] = n.panel[j];
			}

			int empty = -1;          //2.定位当前状态中空白块所在的数组下标empty值（空白块的数码为-1）
			for (int k = 0;k < P;k++)
			{
				if (array[k] == -1)
				{
					empty = k;
				}
			}

			int* new_array = Swap(array, direct[i], empty);  //3.将空白格和扩展方向的格子内的数字交换，生成新的排布数组

			Node new_n = Node(new_array, n.depth + 1);       //4.生成一个扩展结点,深度+1

			new_n.previous = n.present;     //设置父节点编号
			new_n.present = Num;            //设置自身编号

			Insert_expand(new_n);           //将扩展结点插入expand表

			Array[Num] = new_n;             //更新Array表
			Num++;

		}
	}
}

void Process()          //处理expand表中扩展结点的函数【需要并行化】
{
	ListNode* ptr = expand;

	for (;ptr != nullptr;ptr = ptr->next) //循环处理每个扩展结点
	{
		Insert_open(ptr->n);       //“延迟的单表搜索”：对于扩展结点，不进行查找而直接插入open表
	}
}


Node A_Star() //A*算法主体部分
{
	while (open)  //当open表非空时一直循环
	{
		//Print_open();  //打印open表，测试

		Node n = open->n;   //1.获取open表首元素，即f值最小的状态结点
		Delete_open();      //获取后移除表首元素

		if (n.Is_target())  //2.如果open表弹出的首结点就是目标结点，则查找成功，返回结点n
		{
			return n;
		}

		ListNode* ptr = Search_closed(n);  //3.“延迟的单表搜索”：在扩展open表首结点前，先在closed表中查找

		if (ptr != nullptr && (ptr->n < n || ptr->n == n)) //3.1  如果closed表中找到了状态相同且代价比n小的结点，则直接丢弃n
		{
			continue;
		}
		else if (ptr != nullptr)  //3.2  如果状态相同的结点代价比n大，则弹出这个结点，并对n进行扩展
		{
			Delete_closed(ptr);       //删除closed表中ptr指向的结点
			Expand(n);
			Process();                //处理每一个扩展结点
			Insert_closed(n);         //将扩展完毕的结点n插入closed表
		}
		else {                   //3.3  如果closed表中没有状态相同的结点，直接扩展n
			Expand(n);
			Process();                //处理每一个扩展结点
			Insert_closed(n);         //将扩展完毕的结点n插入closed表
		}


		if (Num >= 99950)       //3.4  如果在一定步数内无法求出，也直接退出
		{
			break;
		}
	}

	cout << "当前状态到目标状态无解！" << "\n";
	return Node();
	//exit(0);
}




void help_print(int panel[])
{
	int a = sqrt(P);        //根据P计算出每行/列有几个数码

	int cal = 0;
	for (int i = 0;i < P;i++)     //根据P的规模分行输出数码排布
	{
		cout << panel[i] << ' ';
		cal++;
		if (cal == a)
		{
			cout << "\n";
			cal = 0;
		}
	}
	cout << "\n";
}


void print_trace(Node n)     //从结点n出发，反向打印输出结果的路径
{
	help_print(n.panel);        //首先输出n自身的数码排布

	int father = n.previous;    //然后倒序输出n所有祖先结点的数码排布

	while (father != -1)
	{
		help_print(Array[father].panel);
		father = Array[father].previous;
	}

}




int main()
{

	DWORD Start = GetTickCount();

	start.present = 0;                           //初始结点的编号为0，其父节点默认编号-1

	Insert_open(start);                          //将start结点插入open表
	Array[0] = start;                            //也将start存入Array

	Node n = A_Star();                           //调用A*算法，得到目标结点n

	DWORD End = GetTickCount();

	cout << "耗时：" << End - Start << "ms" << endl;

	print_trace(n);                              //反向打印路径

	return 0;
}
