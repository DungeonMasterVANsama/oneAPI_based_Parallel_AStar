#pragma once
#include <iostream>
using namespace std;

int Compute(int panel1[], int panel2[], int P)          //根据当前的格子数，计算两个排布状态之间的距离
{
	int sum = 0;
	int a = (int)sqrt(P);
	for (int i = 0;i < P;i++)
	{
		for (int j = 0;j < P;j++)
		{
			if (panel1[i] == panel2[j] && panel1[i] != -1)  //空格的数码值为-1，不计算距离
			{
				int row1 = i / a + 1;      //通过一维数组的下标反推当前数字在sqrt(P)*sqrt(P)个格子里的行、列
				int col1 = i % a + 1;

				int row2 = j / a + 1;
				int col2 = j % a + 1;

				sum += abs(row1 - row2) + abs(col1 - col2);

			}
		}
	}
	return sum;
}

bool Same(int panel1[], int panel2[], int P)     //判断两种状态是否相同
{
	/*
	cout << "测试：" << endl;
	for (int i = 0;i < P;i++)  //测试用
	{
		cout << panel1[i] << ' ';
	}
	cout << endl;
	for (int i = 0;i < P;i++)
	{
		cout << panel2[i] << ' ';
	}
	cout << endl;
	*/
	for (int i = 0;i < P;i++)
	{
		if (panel1[i] != panel2[i])
		{
			return false;
		}
	}
	return true;
}

int* help_Expand(int panel[], int P)                   //根据当前P数码问题的规模，计算对于panel所表示的状态，有几种可能的扩展情况
{
	int a = (int)sqrt(P);    //计算正方形的边长

	int empty = -1;          //先定位当前节点状态中空白块所在的数组下标empty值（空白块的数码为-1）
	for (int i = 0;i < P;i++)
	{
		if (panel[i] == -1)
		{
			empty = i;
		}
	}

	int* direction = new int[4];
	for (int i = 0;i < 4;i++)
	{
		direction[i] = -1;
	}

	//每种状态，围绕空白格，有4种(如果在中心)/3种（如果在边框上）/2种（如果在四角处）可能的扩展方向

	if (empty == 0 || empty == a - 1 || empty == P - 1 || empty == P - a)  //如果空白格在四角处，则有2种扩展方向
	{
		if (empty == 0)
		{
			direction[0] = 1;           //剩下两个方向为-1表示无法扩展
			direction[1] = a;
			return direction;
		}
		else if (empty == a - 1)
		{
			direction[0] = a - 2;
			direction[1] = 2 * a - 1;
			return direction;
		}
		else if (empty == P - 1)
		{
			direction[0] = P - 2;
			direction[1] = P - a - 1;
			return direction;
		}
		else if (empty == P - a)
		{
			direction[0] = P - a + 1;
			direction[1] = P - 2 * a;
			return direction;
		}
	}
	else if ((empty / a) + 1 == 1 || (empty / a) + 1 == a || (empty % a) + 1 == 1 || (empty % a) + 1 == a)  //如果空白格不在四角的位置，但又在第一行/最后一行/第一列/最后一列，则有3种扩展方向
	{
		if ((empty / a) + 1 == 1)       //若空白格在第1行
		{
			direction[0] = empty - 1;
			direction[1] = empty + 1;
			direction[2] = empty + a;
			return direction;
		}
		else if ((empty / a) + 1 == a)  //若空白格在第a行
		{
			direction[0] = empty - 1;
			direction[1] = empty + 1;
			direction[2] = empty - a;
			return direction;
		}
		else if ((empty % a) + 1 == 1)  //若空白格在第1列
		{
			direction[0] = empty + 1;
			direction[1] = empty + a;
			direction[2] = empty - a;
			return direction;
		}
		else if ((empty % a) + 1 == a)  //若空白格在第a列
		{
			direction[0] = empty - 1;
			direction[1] = empty - a;
			direction[2] = empty + a;
			return direction;
		}
	}
	else {                                                    //其余的中间位置均有4种扩展方式
		direction[0] = empty - 1;
		direction[1] = empty + 1;
		direction[2] = empty - a;
		direction[3] = empty + a;
		return direction;
	}

	std::cout << "error!" << "\n";
	exit(0);
}


int* Swap(int array[], int i, int j)    //作用是将array[i],array[j]的值交换，返回新数组
{
	int temp = array[i];
	array[i] = array[j];
	array[j] = temp;
	return array;
}





