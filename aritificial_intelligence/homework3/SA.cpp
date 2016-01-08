/*
*  Introduction to Artificial Intelligence Course
*  TSP by SA
*
* Author:  Chenming Wu <wcm15@mails.tsinghua.edu.cn>
* Student ID: 2015210928
* Date:    2015/11/21
*/

#include <iostream> 
#include <fstream>
#include <random>
#include <functional> 
#include <ctime>
#include<iomanip>

struct City{
	char name;
	double x;
	double y;
	void set_city(char c, double _x, double _y){
		name = c; x = _x; y = _y;
	}
};

void initialize(int* path, int n)
{
	for (int i = 0; i < n; i++)
	{
		path[i] = i;
	}
}

void perturb(int* path, int a, int b){
	int tmp = path[a];
	path[a] = path[b];
	path[b] = tmp;
}

void reverse(int* path, int a, int b)
{
	if (a > b) {
		int tmp = a;
		a = b;
		b = a;
	}

	int length = (b - a) / 2;
	for (int i = 0; i < length; i++)
	{
		perturb(path, a++, b--);
	}
}

double calculate(int* path, City* city, int n)
{
	double length = 0.0f;
	for (int i = 0; i < n - 1; i++)
	{
		length = length + std::sqrt((city[path[i]].x - city[path[i + 1]].x)*(city[path[i]].x - city[path[i + 1]].x)
			+ (city[path[i]].y - city[path[i + 1]].y)*(city[path[i]].y - city[path[i + 1]].y));
	}
	length = length + std::sqrt((city[path[0]].x - city[path[n - 1]].x)*(city[path[0]].x - city[path[n - 1]].x)
		+ (city[path[0]].y - city[path[n - 1]].y)*(city[path[0]].y - city[path[n - 1]].y));
	return length;
}


/* 
** 模拟退火算法
** 初始会使用Monte Carlo的思想随机取值，取一个稍微还不错的初始解
** 随后使用温度280度，0.97比例降温，直到温度小于0.001
** 会有10%的概率随机交换path中的两个值a、b
** 另外90%的概率为将a->b的所有值逆序
*/
int sa(int n, City* cities, std::ofstream& output)
{
	int a, b;
	std::default_random_engine generator(std::time(0));
	std::uniform_int_distribution<int> dis(1, n - 1);
	std::uniform_real_distribution<double> u(0, 1);
	int* path = new int[n];
	int* best = new int[n];
	initialize(path, n);
	double temperature = 280;
	// 模拟退火前先生成一个无序随机较优的结果
	double t_old = 1e30, t_new, delta_t, f_best = 1e30;	// 求解函数值 
	const int monte_carlo = 100 * n;
	for (int i = 0; i < monte_carlo; i++)
	{
		do
		{
			a = dis(generator);
			b = dis(generator);
		} while (a == b);

		perturb(path, a, b);

		t_new = calculate(path, cities, n);
		if (t_new < t_old)
		{
			for (int i = 0; i < n; i++)
			{
				best[i] = path[i];
			}
			t_old = t_new;
		}
	}

	while (temperature > 0.001)
	{
		for (int c = 0; c < 100 * n; c++)
		{
			do
			{
				a = dis(generator);
				b = dis(generator);
			} while (a == b);

			bool is_reverse = false;
			if (u(generator)>0.9) is_reverse = true;
			if (is_reverse) reverse(path, a, b);
			else perturb(path, a, b);
			t_new = calculate(path, cities, n);
			delta_t = t_new - t_old;
			if (delta_t < 0)
			{
				t_old = t_new;
			}
			else
			{
				if (exp(-delta_t / temperature) > u(generator))
				{

					t_old = t_new;
				}
				else
				{
					if (is_reverse) reverse(path, a, b);
					else perturb(path, a, b);
				}
			}
		}

		for (int i = 0; i < n; i++)
		{
			output << char('A' + path[i]) << " ";
		}
		output << std::setiosflags(std::ios::fixed) << std::setprecision(6) << t_old << std::endl;
		temperature *= 0.98;
	}



	return 0;
}

int main(int argv, char** argc)
{
	if (argv < 3){
		std::cerr << "Error! Usage: SA.EXE [input.txt] [output.txt]";
		return -1;		// false, return main
	}
		 
	int n = 0;
	std::ifstream input(argc[1]);
	std::ofstream output(argc[2]);
	input >> n;		// number of cities
	City* city_list = new City[n];
	char c;
	double x, y;
	for (int i = 0; i < n; i++)
	{
		input >> c >> x >> y;
		city_list[i].set_city(c, x, y);
	}
	sa(n, city_list, output);
	delete[] city_list;
	return 0;
}
