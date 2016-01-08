/*
*  Introduction to Artificial Intelligence Course
*  Solve Eight Digits Problem
*
* Author:  Chenming Wu <wcm15@mails.tsinghua.edu.cn>
* Student ID: 2015210928
* Date:    2015/10/28
*/

#include <iostream>
#include <fstream>
#include <set>
#include <queue>  
#include <stack>
#include <unordered_map>

const int _final[3][3] = { 1, 2, 3, 8, 0, 4, 7, 6, 5 };
const int _pos[9][2] = { { 1, 1 }, { 0, 0 }, { 1, 0 }, { 2, 0 }, { 2, 1 }, { 2, 2 }, { 1, 2 }, { 0, 2 }, { 0, 1 } };
const int MOVES[4][2] = { { 0, 1 }, { 0, -1 }, { 1, 0 }, { -1, 0 } };   //四个方向  

inline void _swap(int& a, int& b)
{
	const int c = a;
	a = b;
	b = c;
}

struct State
{
	int board[3][3];
	int x, y;
	int f, g;

	// 重载<号
	bool operator< (const State n) const
	{
		return n.f < f;
	}
	
	bool operator== (const State n) const{
		return n.get_hash() == get_hash();
	}

	int get_hash() const
	{
		int n = 0, k = 1;
		for (int i = 0; i < 3; i++)
			for (int j = 0; j < 3; j++)
			{
				n += board[i][j] * k;
				k *= 10;
			}
		return n;
	}

	// h计算方法为所有牌与其应该所处的位置的曼哈顿距离和
	int cal_h()
	{
		int v = 0;
		for (auto i = 0; i < 3; i++)
			for (auto j = 0; j < 3; j++)
			{
				const int val = board[i][j];
				if (val == 0) continue;
				v = v + std::abs(j - _pos[val][0]) + std::abs(i - _pos[val][1]);
			}
		return v;
	}

	void move(const int* dir)
	{
		_swap(board[x + dir[0]][y + dir[1]], board[x][y]);
		x += dir[0];
		y += dir[1];
	}

	bool is_ok()
	{
		for (auto i = 0; i < 3; i++)
			for (auto j = 0; j < 3; j++)
			{
				if (board[i][j] != _final[i][j])
					return false;
			}
		return true;
	}

};

class pairHash{
public:
	size_t operator()(const State &s) const{
		return s.get_hash();
	}
};

struct pairEqual{
	bool operator()(const State & a1, const State & a2)const{
		return  a1.get_hash() == a2.get_hash();
	}

};

std::vector<int> GenerateMoves(State& s)
{
	std::vector<int> MoveList;
	for (auto i = 0; i < 4; i++)
	{
		const int& _x = s.x + MOVES[i][0];
		const int& _y = s.y + MOVES[i][1];
		if (_x >= 0 && _x < 3 && _y >= 0 && _y < 3)
		{
			MoveList.push_back(i);
		}
	}
	return MoveList;
}

void A_Start(State& _in,char* filename = NULL)
{
	std::priority_queue<State> open;
	std::unordered_map<State, int, pairHash, pairEqual> closed;	// Hash Map
	std::unordered_map<State, State, pairHash, pairEqual> orders;

	bool is_solved = false;
	_in.f = _in.cal_h();
	_in.g = 0;
	open.push(_in);
	State s,v;
	
	while (!open.empty())
	{
		s = open.top();
		open.pop();	//弹出最小值

		if (s.is_ok())
		{
			is_solved = ~is_solved;
			break;
		}

		if (closed.count(s))
		{
			if (closed[s] >= s.f) continue;
		}

		closed[s] = s.f;

		std::vector<int> Moves = GenerateMoves(s);	//生成所有的可行步数
		for (auto& i : Moves)
		{
			v = s;
			v.move(MOVES[i]);	//移动
			if (!closed.count(v))
			{
				v.g = s.g + 1;
				v.f = v.g + v.cal_h();	
				open.push(v);
				orders[v] = s;
			}
		}
	}

	if (!filename)
	{
		return;
	}

	std::ofstream _output(filename);
	if (!is_solved)
	{
		_output << "no solution";
	}
	else
	{
		std::stack<State> FILO;
		FILO.push(s);
		while (true)
		{
			s = orders[s];
			FILO.push(s);
			if (orders[s] == _in) break;
		}

		_output << FILO.size() << std::endl << std::endl;

		while (!FILO.empty())
		{
			State s = FILO.top();
			FILO.pop();
			for (auto i = 0; i < 3; i++)
			{
				_output << s.board[i][0] << " "
					<< s.board[i][1] << " "
					<< s.board[i][2] << std::endl;
			}
			_output << std::endl;

		}
	}
	
	_output.close();
}

int main(int argv, char** argc)
{
	if (argv != 3)
	{
		printf("Error!\n");
		printf("Usage : [HW_1.exe] [input.txt] [output.txt]\n");
		return -1;
	}
	std::ifstream input(argc[1]);
	//std::ifstream input(argc[1]);
	State init;
	for (int i = 0; i < 3; i++)
		for (int j = 0; j < 3; j++)
		{
			input >> init.board[i][j];
			if (init.board[i][j] == 0)
			{
				init.x = i; init.y = j;
			}
		}

	A_Start(init,argc[2]);
	
	return 0;
}