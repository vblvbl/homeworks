/*
*  Introduction to Artificial Intelligence Course
*  TSP by GA
*
* Author:  Chenming Wu <wcm15@mails.tsinghua.edu.cn>
* Student ID: 2015210928
* Date:    2015/11/26
*/
#include <iostream> 
#include <fstream>
#include <random>
#include <algorithm>
#include <functional> 
#include <unordered_set>
#include <vector>
#include <assert.h>
#include <time.h>

struct City{
	char name;
	double x;
	double y;
	void set_city(char c, double _x, double _y){
		name = c; x = _x; y = _y;
	}
};

City* citylist = NULL;

struct Path{
	int path[50];		// 路径
	double f;			// 距离
	int n;				// 数量
	double adp;			// 适应值

	Path(){};

	Path(int _n, std::default_random_engine& generator)			// 构造函数生成n个随机的值
	{
		n = _n;
		std::unordered_set<int> temp_set;
		std::uniform_int_distribution<int> dis(0, n - 1);
		for (int i = 0; i < n; i++)
		{
			while (!temp_set.insert(dis(generator)).second) {}
		}

		assert(temp_set.size() == n);

		int i = 0;
		for (auto it : temp_set)
		{
			path[i++] = it;
		}
		if (citylist)  update_f();
	}

	void assign(Path& pa)
	{
		n = pa.n;
		for (int i = 0; i < n; i++) path[i] = pa.path[i];
		f = pa.f; adp = pa.adp;
	}

	void update_f()
	{
		f = calculate(path, n, citylist);	// 计算距离
	}

	static double calculate(int* path, int n, City* city)
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

	void perturb(int a, int b){
		int tmp = path[a];
		path[a] = path[b];
		path[b] = tmp;
	}

	void reverse(int a, int b)
	{
		if (a > b) {
			int tmp = a;
			a = b;
			b = a;
		}

		int length = (b - a) / 2;
		for (int i = 0; i < length; i++)
		{
			perturb(a++, b--);
		}
	}

	bool operator < (const Path b) const { return adp > b.adp; };
};

Path global;

void evaluate(std::vector<Path>& paths)
{
	int n = paths.size();

	for (int i = 0; i < n; i++) paths[i].update_f();

	double p_greatest = 0;

	for (int i = 0; i < n; i++)
	{
		//paths[i].adp = paths[i].f / dis_sum;
		paths[i].adp = 1 / paths[i].f;
		p_greatest += paths[i].adp;
	}

	for (int i = 0; i < n; i++) paths[i].adp /= p_greatest;
}

/*
** 遗传算法
** 新种群选择方法为模拟轮盘赌
** 适应度为1/dist的概率
** 种群数目1000，交配概率0.65，变异概率0.25
** 交配方法为课件中所提到的常规交配法
** 变异方法为50%概率随机交换a、b，50%概率随机逆序a->b
*/
int ga(int n, City* cities)
{
	// 全局最优解
	global.f = 1e30;

	// 遗传算法参数设定
	const int N = 1000;		// 种群数目
	const double p_c = 0.65;	// 交配概率
	const double p_m = 0.25;	// 变异概率
	double sel_p[N];
	double sel_pop[N + 1];
	int sel_temp[N];
	std::default_random_engine generator(time(0));
	std::uniform_int_distribution<int> dis(0, n - 1);
	std::uniform_real_distribution<double> real(0, 1);
	// 初始化种群
	std::vector<Path> population;
	for (int i = 0; i < N; i++)
	{
		population.push_back(Path(n, generator));
	}

	// 遗传算法入口
	for (int i = 0; i < 10000; i++)
	{
		// 更新全局最优解
		evaluate(population);
		std::sort(population.begin(), population.end());	// 排序
		if (population[0].f < global.f) global = population[0];
	
		// 选择阶段
		double sum = -population[0].adp;
		sel_pop[N] = 1;
		for (int i = 0; i < N; i++)
		{
			sel_p[i] = real(generator);
			sel_pop[i] = sum + population[i].adp;
			sum = sel_pop[i];
		}

		std::sort(sel_p, sel_p + N);	// 排序

		for (int i = 0, j = 0; i < N; i++)
		{
			while (sel_p[i] < sel_pop[j] || sel_p[i] >= sel_pop[j + 1]) j++;
			sel_temp[i] = j;
		}

		std::vector<Path> temp(population);
		for (int i = 0; i < N; i++)	population[i] = temp[sel_temp[i]];

		// 交配阶段
		for (int i = 0; i < N; i++)
		{
			if (real(generator) <= p_c)	//交配
			{
				for (int j = i + 1; j < N; j++)
				{
					if (real(generator) <= p_c)
					{
						Path par_1 = population[i], par_2 = population[j];

						std::unordered_set<int> multiple;		// 去重
						/*交配过程*/
						int point;
						point = dis(generator);

						int l = 0;	// 截断位置
						for (int k = 0; k < point; k++)	multiple.insert(par_1.path[k]);
						for (int k = point; k < n; k++)
						{
							while (multiple.find(par_2.path[l]) != multiple.end()) l++;
							population[i].path[k] = par_2.path[l];
							l++;
						}

						multiple.clear();
						l = 0;
						for (int k = 0; k < point; k++)	multiple.insert(par_2.path[k]);
						for (int k = point; k < n; k++)
						{
							while (multiple.find(par_1.path[l]) != multiple.end()) l++;
							population[j].path[k] = par_1.path[l];
							l++;
						}
						i = j + 1;
						break;
					}
				}
			}
		}

		// 变异阶段
		for (int i = 0; i < N; i++)
		{
			if (real(generator) <= p_m)
			{
				int a = dis(generator), b = dis(generator);
				if (real(generator) <= 0.5)	population[i].reverse(a, b);
				else population[i].perturb(a, b);
			}
		}

	}

	return 1;
}

int main(int argv, char** argc)
{
	if (argv < 3){
		std::cerr << "Error! Usage: GA.EXE [input.txt] [output.txt]";
		return -1;		// false, return main
	}
	int n;
	std::ifstream input(argc[1]);
	std::ofstream output(argc[2]);
	input >> n;		// number of cities
	citylist = new City[n];
	char c;
	double x, y;
	for (size_t i = 0; i < n; i++)
	{
		input >> c >> x >> y;
		citylist[i].set_city(c, x, y);
	}
	ga(n, citylist);

	for (int i = 0; i < n; i++)
	{
		output << char('A' + global.path[i]) << " ";
	}
	output << global.f;
	return 0;
}
