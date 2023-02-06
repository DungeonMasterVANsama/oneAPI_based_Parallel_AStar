#include <iostream>
#include <cmath>
#include <CL/sycl.hpp>
#include "question.h"
using namespace sycl;

#define P 16       //P为数码问题的格子总数(8,15,24数码问题分别对应 P = 9，16，25......)

#define HashLength 20   //哈希表Closed的长度

#define HashLength2 10000  //哈希表Open的长度

#define ListLength 10000   //closed哈希表下属每个list的初始长度

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

int start_panel[] = { 1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,-1 };   //手动设置初始状态、目标状态的数码排布(根据P的值，从左到右、从上到下输入P个数；空白格的值置为-1)
int target_panel[] = { -1,15,14,13,12,11,10,9,8,7,6,5,4,3,2,1 };  //倒序情况是有解的..?

Node Target(target_panel, 0);   //目标状态
Node start(start_panel, 0);    //初始状态



int Node::Compute_h()  //调用Compute(),估计代价h = 每个“不在位”的数码离目标位置的距离之和
{
	return Compute(panel, Target.panel, P);
}

bool Node::Is_target()    //判断当前结点的状态是否是目标状态target
{
	return Same(panel, Target.panel, P);
}

bool Node::Is_equal(Node n)   //判断当前结点状态和结点n的状态是否相同
{
	return Same(panel, n.panel, P);
}



class return_type1 {                       //查找closed表时的返回值
public:
	int hash_index = -1;  //哈希表的索引
	int list_index = -1;  //顺序表的索引
};


class ListNode {                            //closed表结点的数据结构
public:
	Node n;
	ListNode* next = nullptr;
	int ListIndex = -1;    //该结点在hash-list中的索引值
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

class ListNode2 {                        //open表结点 的数据结构
public:
	Node n;
	ListNode2* prev = nullptr;
	ListNode2* next = nullptr;

	ListNode2()
	{
		;
	}

	ListNode2(Node n)
	{
		this->n = Node(n.panel, n.depth);
		this->n.previous = n.previous;
		this->n.present = n.present;
	}
};


//定义全局变量
ListNode* expand = nullptr;     //当前结点扩展得到的所有结点（扩展表）

ListNode2* open = nullptr;       //open表头指针
ListNode2* open_tail = nullptr;  //open表尾指针
ListNode* closed = nullptr;     //closed表

Node Array[10000];    //Array用于存放所有生成的结点；在打印路径的时候，根据每个结点的previous值在Array中找到相同下标的父节点
int Num = 1;          //给新生成的结点设置编号(start结点编号为0，其他的从1开始)


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


class SeqList {      //顺序表结构
public:
	ListNode** data_ptr;  //顺序表结构里存放的是指向open/closed的链表结点的指针
	int capacity;        //顺序表容量
	int size;            //顺序表当前长度

	SeqList()
	{
		data_ptr = new ListNode * [ListLength];  //顺序表初始长度为10【000.........?】
		for (int i = 0;i < ListLength;i++)
		{
			data_ptr[i] = nullptr;
		}
		capacity = ListLength;
		size = 0;
	}

	int Insert(ListNode* ptr)   //插入操作
	{
		if (size == capacity) //判断  如果顺序表满了，要扩容，增加capacity/2的长度【【?这段有问题........?】】
		{
			cout << "not enough space in SeqList..." << "\n";
			exit(0);
		}

		data_ptr[size] = ptr;
		size++;
		return size - 1;     //返回新插入这个元素的索引值
	}

	void Delete(int index)     //删除操作
	{
		data_ptr[index] = data_ptr[size]; //用最后一个元素替换要删除位置的元素
		size--;
	}

	int Search(Node n)  //查找表中有没有指向与结点n相同状态的结点的指针
	{
		for (int i = 0;i < size;i++)
		{
			if (data_ptr[i] != nullptr)
			{
				if (data_ptr[i]->n.Is_equal(n))
				{
					//cout<<"r:"<<i<<"\n";   //测试
					return i;     //返回查找成功的索引值
				}
			}

		}
		//cout<<"r:"<<i<<"\n";  //测试
		return -1;       //找不到则返回-1
	}

};


class HashTable_Closed {    //对应于closed表的哈希表结构，用散列方式缩短closed表的检索长度
public:
	SeqList* data[HashLength];   //哈希表每个结点存放指向顺序表SeqList的指针

	HashTable_Closed()      //构造函数
	{
		for (int i = 0;i < HashLength;i++)//初始化哈希表下的每个顺序表
		{
			data[i] = new SeqList();
		}
	}

	int Hash_Function(int n) //散列函数
	{
		return n % HashLength;
	}

	void Insert(Node n)    //插入操作
	{
		int index = Hash_Function(n.h);  //根据当前状态到目标状态的曼哈顿距离h来寻找散列槽

		Insert_closed(n);  //由于closed表不需要维护元素顺序，因此直接插入代价很小

		ListNode* ptr = closed;  //closed表使用头插法，closed就是新插入结点的指针

		int d = data[index]->Insert(ptr); //将指针插入相应槽的顺序表中，返回新插入这个指针在顺序表中的索引值

		closed->ListIndex = d;   //closed链表中记录这个索引值
	}


	return_type1 Search(Node n)   //查找操作                    【【【******重点 实现并行******】】】
	{
		gpu_selector selector;
		queue q(selector);
		device my_device = q.get_device();
		std::cout << "Device: " << my_device.get_info<info::device::name>() << "\n";

		auto R1 = range<1>(2);
		auto R2 = range<1>(HashLength);
		//return_type1 temp;

		int a[2] = { -1 };

		int r[HashLength] = { -2 };
		buffer buf_a(a, R1);

		buffer buf_d(data, R2);

		buffer buf_r(r, R2);

		q.submit([&](handler& h) {
			accessor A(buf_a, h, read_write);
			accessor D(buf_d, h, read_only);
			accessor R(buf_r, h, write_only);

			h.parallel_for(range<1>(HashLength), [=](auto i) {
				if (A[0] != -1 && A[1] != -1)
				{
					;
				}
				else {
					int r = D[i]->Search(n);
					R[i] = r;
					if (r != -1)
					{
						A[0] = i;
						A[1] = r;
					}
				}
				});

			});

		host_accessor A_(buf_a, read_write);
		host_accessor D_(buf_d, read_write);
		host_accessor R_(buf_r, read_write);

		//cout<<"a[0]:"<<A_[0]<<"a[1]:"<<A_[1]<<"\n";
		//cout<<"r:"<<"\n";
		/*
		for(int m=0;m<HashLength;m++)
		{
		   cout<<R_[m]<<' ';
		}
		cout<<"\n";
		*/
		return_type1 temp;
		if (A_[0] != -1 && A_[1] != -1)
		{
			temp.hash_index = A_[0];
			temp.list_index = A_[1];
		}
		return temp;    //如果所有槽的list中都没找到，则temp里两个成员的值就都还是-1
	}


	void Delete(return_type1 r)        //删除操作
	{
		if (r.hash_index != -1 && r.list_index != -1)  //当查找返回的索引有效
		{
			data[r.hash_index]->Delete(r.list_index);  //只是删除了哈希表下属顺序表中存放的指针，实际上并没有真正删除closed表中的结点
		}
	}
};

void help_print(int[]);

void Print_open()      //打印查看open表状态
{
	std::cout << "打印当前open表：" << "\n";
	ListNode2* ptr = open;
	while (ptr != nullptr)
	{
		help_print(ptr->n.panel);
		ptr = ptr->next;
	}
}

class HashTable_Open {        //对应于open表的哈希表结构，主要是在保持open表有序的前提下优化了插入速度
public:
	ListNode2* data[HashLength2];   //哈希表每个结点存放指向open表对应结点的指针
	int last_index;        //当前哈希表最后一个非空的槽的索引值

	HashTable_Open()      //构造函数,将所有槽的指针都初始化为空
	{
		for (int i = 0;i < HashLength2;i++)
		{
			data[i] = nullptr;
		}
		last_index = -1; //一开始哈希表中全是指针，last_index值置为-1
	}

	int Hash_Function(Node n)     //哈希函数，根据结点的f值直接确定槽的索引值
	{
		return (n.depth + 10 * n.h);
	}

	void Insert(Node n)     //插入操作
	{
		int index = Hash_Function(n);   //先获取该结点在哈希表中对应的索引值

		if (last_index == -1)   //1.如果open表为空,直接插入头结点
		{
			open = new ListNode2(n);
			open_tail = open;

			data[index] = open;
			last_index = index;
		}
		else {                 //2.如果open表不为空
			if (index >= last_index)     //2.1  如果槽索引大于等于当前最后一个非空指针的索引值，则直接把指针插入对应的槽、把结点插入open表尾
			{
				ListNode2* ptr = new ListNode2(n);
				open_tail->next = ptr;
				ptr->prev = open_tail;
				open_tail = ptr;

				data[index] = ptr;
				last_index = index;
			}
			else {                     //2.2  如果槽索引小于最后一个非空指针的索引值
				if (data[index] != nullptr)    //2.2.1  如果该槽的指针非空(说明open表中已经有与要插入结点f值相同的结点了，把这一堆f值相同的结点用一个首指针管理即可)
				{
					if (data[index] == open)   //如果当前槽指针指向的结点是open表首节点
					{
						open = new ListNode2(n);
						open->next = data[index];
						data[index]->prev = open;

						data[index] = open;                  //然后将当前槽的指针改为指向新插入的结点
					}
					else {                              //如果当前槽指向的结点前驱不为空，则正常插在该结点之前
						ListNode2* ptr = new ListNode2(n);
						data[index]->prev->next = ptr;
						ptr->prev = data[index]->prev;
						ptr->next = data[index];
						data[index]->prev = ptr;

						data[index] = ptr;                   //然后将当前槽的指针改为指向新插入的结点
					}
				}
				else {                         //2.2.2  如果该槽的指针为空（说明open表中还没有与要插入的结点f值相同的结点） 
					for (int i = index + 1;i <= last_index;i++)  //查找哈希表后面一段的第一个非空指针
					{
						if (data[i] != nullptr)
						{
							if (data[i] == open)    //如果当前槽指针指向的结点是open表首节点
							{
								open = new ListNode2(n);
								open->next = data[i];
								data[i]->prev = open;

								data[index] = open;             //然后将当前槽的指针改为指向新插入的结点
							}
							else {
								ListNode2* ptr = new ListNode2(n);  //如果当前槽指针指向的结点的前驱不为空，则正常插在该结点之前
								data[i]->prev->next = ptr;
								ptr->prev = data[i]->prev;
								ptr->next = data[i];
								data[i]->prev = ptr;

								data[index] = ptr;                   //然后将当前槽的指针改为指向新插入的结点
							}
							break;
						}
					}
				}
			}

		}
	}

	void Delete_first()    //删除open表首结点
	{
		if (open == open_tail)  //1.  如果整个Open表只剩下一个结点了，则弹出后要把头、尾指针置空
		{
			data[Hash_Function(open->n)] = nullptr;  //首先要把这最后一个结点对应的槽指针清空

			open = nullptr;
			open_tail = nullptr;
			last_index = -1;
		}
		else {                 //2.   如果open表还剩下不止一个结点
			if (Hash_Function(open->n) == Hash_Function(open->next->n))   //2.1  如果要弹出的结点与其下一个结点的f值相同
			{
				data[Hash_Function(open->n)] = open->next;                //那就让原来指向要弹出结点的槽指针指向它的下一个结点

				open = open->next;
			}
			else {                                                        //2.2  如果要弹出的结点与其下一个结点的f值不同
				data[Hash_Function(open->n)] = nullptr;                   ////那就清除原来指向要弹出结点的槽指针

				open = open->next;
			}
		}
	}


};


//定义全局变量：
HashTable_Closed ht_c;    //closed哈希表
HashTable_Open ht_o;      //open哈希表


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
void Expand(Node& n)    //对当前结点n进行扩展 【？是否需要并行化】
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

void Process()          //处理expand表中扩展结点的函数
{
	ListNode* ptr = expand;

	for (;ptr != nullptr;ptr = ptr->next) //循环处理每个扩展结点
	{
		//“延迟的单表搜索”：对于扩展结点，不进行查找而直接插入open表
		ht_o.Insert(ptr->n);
	}
}


Node A_Star() //A*算法主体部分
{
	while (open)  //当open表非空时一直循环
	{
		Node n = open->n;   //1.获取open表首元素，即f值最小的状态结点

		ht_o.Delete_first();

		if (n.Is_target())  //2.如果open表弹出的首结点就是目标结点，则查找成功，返回结点n
		{
			return n;
		}

		return_type1 r = ht_c.Search(n);

		if (r.hash_index != -1 && r.list_index != -1 && (ht_c.data[r.hash_index]->data_ptr[r.list_index]->n < n || ht_c.data[r.hash_index]->data_ptr[r.list_index]->n == n))
		{
			continue;
		}
		else if (r.hash_index != -1 && r.list_index != -1)
		{
			ht_c.Delete(r);
			Expand(n);
			Process();
			ht_c.Insert(n);
		}
		else {
			Expand(n);
			Process();
			ht_c.Insert(n);
		}

		if (Num >= 9995)       //如果在一定步数内无法求出，也直接退出
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
	start.present = 0;                           //初始结点的编号为0，其父节点默认编号-1

	ht_o.Insert(start);
	Array[0] = start;                            //也将start存入Array

	Node n = A_Star();                           //调用A*算法，得到目标结点n

	print_trace(n);                              //反向打印路径

	return 0;
}
