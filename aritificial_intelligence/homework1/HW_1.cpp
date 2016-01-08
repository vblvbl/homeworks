/*
*  Introduction to Artificial Intelligence Course
*  For solving SUDOK problem
*
* Author:  Chenming Wu <wcm15@mails.tsinghua.edu.cn>
* Student ID: 2015210928
* Date:    2015/10/6
*/

#include <iostream>
#include <string>
#include <fstream>
#include <map>
#include <vector>

#define SIZE 9
#define SUB_SIZE 3


class Board
{
public:
	// Ĭ�Ϲ����� 
	Board() : m_curLine(0), m_curIndex(0), completed(false)
	{ }

	// ���ⲿ�ļ��ж����� 
	bool read_line(std::string& str)
	{
		if (m_curLine > SIZE)	return false;
		for (int i = 0; i < str.size(); i++)
		{
			m_grids[m_curLine][i].get_val() = static_cast<int>(str[i] - '0');
			m_grids[m_curLine][i].get_available() = SIZE;

			if (m_grids[m_curLine][i].get_val() == 0)
			{
				blank_grids.push_back(std::make_pair(m_curLine, i));
			}
			else
			{
				int t_x = m_curLine, t_y = i;
				m_grids[t_x][t_y].add_count(m_grids[m_curLine][i].get_val());
				for (int m = 0; m < SIZE; m++)
				{
					if (m != t_y)
					{
						m_grids[t_x][m].add_count(m_grids[m_curLine][i].get_val());
					}
				}
				for (int m = 0; m < SIZE; m++)
				{
					if (m != t_x)
					{
						m_grids[m][t_y].add_count(m_grids[m_curLine][i].get_val());
					}
				}

				for (int m = (t_x / SUB_SIZE) * SUB_SIZE; m < (t_x / SUB_SIZE) * SUB_SIZE + SUB_SIZE; m++)
				{
					for (int n = (t_y / SUB_SIZE) * SUB_SIZE; n < (t_y / SUB_SIZE) * SUB_SIZE + SUB_SIZE; n++)
					{
						if (m == t_x || n == t_y)
						{
							continue;
						}
						m_grids[m][n].add_count(m_grids[m_curLine][i].get_val());
					}
				}
			}
		}
		m_curLine++;
		return true;
	}

	// �������д�ӡ���̣����ڵ��� 
	void print_board()
	{
		for (unsigned i = 0; i < SIZE; i++)
		{
			for (unsigned j = 0; j < SIZE; j++)
			{
				printf("%d", m_grids[i][j].get_val());
			}
			printf("\n");
		}
	}

	// ������������ļ� 
	void save_board()
	{
		for (unsigned i = 0; i < SIZE; i++)
		{
			for (unsigned j = 0; j < SIZE; j++)
			{
				out << m_grids[i][j].get_val();
			}
			out << std::endl;
		}
	}

	// �ж��Ƿ�������i 
	bool is_satisfy(int i)
	{
		return !m_grids[blank_grids[m_curIndex].first][blank_grids[m_curIndex].second].is_conflict(i);
	}

	// �������м���i 
	void add_number(int i)
	{
		m_grids[blank_grids[m_curIndex].first][blank_grids[m_curIndex].second].set_num(i);
		m_grids[blank_grids[m_curIndex].first][blank_grids[m_curIndex].second].add_count(i);
		add_update(i);
		m_curIndex++;
	}

	// ɾ�����̣�����m_curIndex--�ķ�ʽ 
	void back_number()
	{
		m_curIndex--;
		int num = m_grids[blank_grids[m_curIndex].first][blank_grids[m_curIndex].second].get_val();
		m_grids[blank_grids[m_curIndex].first][blank_grids[m_curIndex].second].set_num(0);
		m_grids[blank_grids[m_curIndex].first][blank_grids[m_curIndex].second].del_count(num);
		back_update(num);
	}

	// ���ݹ��򣬸����������ӵ�״̬ 
	void add_update(int value)
	{
		int t_x = blank_grids[m_curIndex].first, t_y = blank_grids[m_curIndex].second;
		for (int m = 0; m < SIZE; m++)
		{
			if (m != t_y)
			{
				m_grids[t_x][m].add_count(value);
			}
		}
		for (int m = 0; m < SIZE; m++)
		{
			if (m != t_x)
			{
				m_grids[m][t_y].add_count(value);
			}
		}

		for (int m = (t_x / SUB_SIZE) * SUB_SIZE; m < (t_x / SUB_SIZE) * SUB_SIZE + SUB_SIZE; m++)
		{
			for (int n = (t_y / SUB_SIZE) * SUB_SIZE; n < (t_y / SUB_SIZE) * SUB_SIZE + SUB_SIZE; n++)
			{
				if (m == t_x || n == t_y)
				{
					continue;
				}
				m_grids[m][n].add_count(value);
			}
		}
	}

	void back_update(int value)
	{
		int t_x = blank_grids[m_curIndex].first, t_y = blank_grids[m_curIndex].second;
		for (int m = 0; m < SIZE; m++)
		{
			if (m != t_y)
			{
				m_grids[t_x][m].del_count(value);
			}
		}
		for (int m = 0; m < SIZE; m++)
		{
			if (m != t_x)
			{
				m_grids[m][t_y].del_count(value);
			}
		}

		for (int m = (t_x / SUB_SIZE) * SUB_SIZE; m < (t_x / SUB_SIZE) * SUB_SIZE + SUB_SIZE; m++)
		{
			for (int n = (t_y / SUB_SIZE) * SUB_SIZE; n < (t_y / SUB_SIZE) * SUB_SIZE + SUB_SIZE; n++)
			{
				if (m == t_x || n == t_y)
				{
					continue;
				}
				m_grids[m][n].del_count(value);
			}
		}
	}

	// �ж��Ƿ�� 
	bool is_solved()
	{
		if (completed) return completed;

		if (m_curIndex == blank_grids.size())
		{
			completed = true;
			save_board();
		}
		return completed;
	}

	// ��ǰ��֦ 
	bool is_unsolvable()
	{
		int t_x = blank_grids[m_curIndex].first, t_y = blank_grids[m_curIndex].second;
		return m_grids[t_x][t_y].get_available() == 0;	// �������� 
	}

	// ��� 
	void OpenOutput(char* filename)
	{
		out.open(filename, std::ios::out);
	}


private:
	// �࣬����ÿһ������ 
	class Grid
	{
	public:
		// Ĭ�Ϲ��캯�� 
		Grid(){}

		// �ж��Ƿ��ͻ 
		bool is_conflict(int val)
		{
			return valMap.find(val) != valMap.end();
		}

		// �趨ֵ 
		void set_num(int _val)
		{
			val = _val;
		};

		// ��ֵ 
		void add_count(int _val)
		{
			valMap[_val]++;
			nAvailableGrids = 9 - valMap.size();
		}

		// ɾֵ 
		void del_count(int _val)
		{
			valMap[_val]--;
			if (valMap[_val] == 0)
			{
				valMap.erase(_val);
			}
			nAvailableGrids = 9 - valMap.size();
		}

		// ��ȡֵ 
		int& get_val() { return val; }

		int& get_available() { return nAvailableGrids; }
	private:
		int val;
		int nAvailableGrids;
		std::map<int, int> valMap;
	};
	std::ofstream out;
	Grid m_grids[SIZE][SIZE];
	int m_curIndex;
	int m_curLine;
	std::vector<std::pair<int, int> > blank_grids;
	bool completed;
};

// �����㷨��� 
void back_tracking(Board& state)
{
	if (state.is_solved())
	{
		return;
	}

	if (state.is_unsolvable())
	{
		return;
	}

	for (int i = 1; i <= SIZE; i++)
	{
		if (!state.is_satisfy(i))
		{
			continue;
		}
		state.add_number(i);
		back_tracking(state);	// back_tracking process
		state.back_number();
	}

}

int main(int argv, char** argc)
{
	if (argv != 3)
	{
		printf("Error!\n");
		printf("Usage : [HW_1.exe] [input.txt] [output.txt]\n");
		return -1;
	}
	Board board;	// �����������̶��� 

	std::ifstream input(argc[1]);
	board.OpenOutput(argc[2]);
	if (!input)
	{
		printf("An error occured (Read input file)\n");
		return -1;
	}

	std::string str;
	while (getline(input, str))
	{
		board.read_line(str);
	}
	input.close();
	back_tracking(board);

	return 0;
}
