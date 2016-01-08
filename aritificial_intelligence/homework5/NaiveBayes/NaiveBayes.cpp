#include <iostream>
#include <stdlib.h>
#include <direct.h>
#include <io.h>
#include <stdio.h>
#include <malloc.h>
#include <string>
#include <vector>
#include <set>
#include <map>
#include <random>
#include <time.h>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <cctype>
const char labels[4][50] = { "c1_atheism", "c2_sci.crypt", "c3_talk.politics.guns", "c4_comp.sys.mac.hardware" };

std::vector<std::string> text_files[4];
std::map<std::string, double> Pck[4];
std::set<std::string> doc;
std::vector<std::string> docs[4];
std::vector<std::string> k_fold[4];
std::string text[4];
size_t Vocabulary = 0;
size_t nb_fold_files = 0;
int _a[4] = { 0 }, _b[4] = { 0 }, _c[4] = { 0 };

void reset()
{
	for (int i = 0; i < 4; ++i)
	{
		text_files[i].clear();
		Pck[i].clear();
		docs[i].clear();
		k_fold[i].clear();
		text[i].clear();
	}

	doc.clear();
	Vocabulary = 0;
	nb_fold_files = 0;
}

int get_class(int n)
{
	int lower_b = 0;
	for (int i = 0; i < 4; i++)
	{
		if (n >= lower_b && n < lower_b + text_files[i].size()) return i;
		lower_b += text_files[i].size();
	}
	return 0;
}

void k_fold_process(int k)
{
	std::default_random_engine generator(time(0));
	std::uniform_int_distribution<int> dis(0, 400);
	std::set<int> fold_set;
	k = nb_fold_files = (int)(1. / k * 400);
	// 取出k个文件indices
	while (k)
	{
		if (fold_set.insert(dis(generator)).second)
		{
			--k;
		}
	}

	std::vector<std::string> temp[4];
	for (size_t i = 0 ; i < 400 ; ++i)
	{
		int c = get_class(i);
		if (fold_set.find(i) != fold_set.end()){
			k_fold[c].push_back(text_files[c][i % 100]);
		}
		else{
			temp[c].push_back(text_files[c][i % 100]);
		}
	}

	for (size_t i = 0; i < 4; ++i)
	{
		text_files[i].clear();
		text_files[i] = temp[i];
		printf("(id = %d) text_files = %d, fold = %d\n", i, text_files[i].size(), k_fold[i].size());
	}
}

void getFiles(std::string path, std::vector<std::string>& files) {
	long   hFile = 0;
	struct _finddata_t fileinfo;
	std::string p;

	if ((hFile = _findfirst(p.assign("./").append(path).append("/*").c_str(), &fileinfo)) != -1)  {
		do  {
			if (strcmp(fileinfo.name, ".") != 0 && strcmp(fileinfo.name, "..") != 0)
				files.push_back(p.assign(path).append("/").append(fileinfo.name));
		} while (_findnext(hFile, &fileinfo) == 0);

		_findclose(hFile);
	}
}

inline bool IsAlpha(char& i)
{
	return (i >= 'a' && i<= 'z') || (i>= 'A' && i <= 'Z');
}

int Split2Word(std::string& filename, std::set<std::string>& _doc,int _f)
{
	std::ifstream file(filename);
	if (!file.is_open()) return 0;
	std::ostringstream buf;
	char ch;
	while (buf&&file.get(ch)) buf.put(ch);
	file.close();	// 关闭文件
	text[_f] = buf.str();
	size_t last = 0, word_count = 0;
	std::string new_str;
	for (size_t i = 0; i < text[_f].size(); ++i)	{		if (!IsAlpha(text[_f][i]))		{			new_str = text[_f].substr(last, i - last);
			// 大写转小写
			std::transform(new_str.begin(), new_str.end(), new_str.begin(), std::tolower);
			_doc.insert(new_str);
			docs[_f].push_back(new_str);
			++word_count;
			while (!IsAlpha(text[_f][i]) && i < text[_f].size()) ++i;
			last = i;
		}	}
	return word_count; //返回一共多少个单词
}

void NaiveBayesLearning(int k)
{
	// 读入文件
	for (int i = 0; i < 4; ++i) { getFiles(labels[i], text_files[i]); }	
	
	k_fold_process(k);

	// 生成总字典
	for (int i = 0; i < 4; ++i)
	{
		for (int j = 0; j < text_files[i].size(); j++)
			Vocabulary += Split2Word(text_files[i][j], doc, i);
	}

	for (size_t i = 0; i < 4; ++i)
	{
		for (size_t j = 0; j < docs[i].size(); ++j)
			Pck[i][docs[i][j]] += 1.0f;

		for (std::set<std::string>::iterator it = doc.begin(); it != doc.end(); it++)
		{
			Pck[i][*it] = (Pck[i][*it] + 1) / (docs[i].size() /*+ Vocabulary*/);
		}
	}
}

int Classify_naive_Bayes_text(std::string& filename)
{
	int c;
	double y[4];
	for (size_t i = 0; i < 4; i++)
		y[i] = (double)docs[i].size() / (double)Vocabulary;

	std::ifstream file(filename);
	if (!file.is_open()) return 0;
	std::ostringstream buf;
	char ch;
	while (buf&&file.get(ch)) buf.put(ch);
	file.close();	// 关闭文件
	std::string _new_text = buf.str();
	size_t last = 0;
	std::string new_str;
	for (size_t k = 0; k < 4; ++k)
	{
		for (size_t i = 0; i < _new_text.size(); ++i)		{			if (!IsAlpha(_new_text[i]))			{				new_str = _new_text.substr(last, i - last);
				std::transform(new_str.begin(), new_str.end(), new_str.begin(), std::tolower);
				if (doc.find(new_str) != doc.end())
				{
					y[k] += std::log(Pck[k][new_str]);
				}
				while (!IsAlpha(_new_text[i]) && i < _new_text.size()) ++i;
				last = i;
			}		}
	}

	double max = -1e30;
	for (size_t i = 0; i < 4; ++i)
	{
		if (y[i] > max){
			c = i; 
			max = y[i];
		}
	}
	return c;
}

void k_fold_verify()
{
	int nb_right = 0;
	for (size_t i = 0; i < 4; ++i)
		for (size_t j = 0; j < k_fold[i].size(); ++j)
		{
			int res = Classify_naive_Bayes_text(k_fold[i][j]);
			if (res == i) ++_a[i];
			else{
				++ _b[res];
				++ _c[i];
			}
				
		}
}

void print_result()
{
	double accurate = 0, recall = 0, f = 0;
	for (int i = 0; i < 4; i++)
	{
		accurate += _a[i] / (double)(_a[i] + _b[i]);
		recall += _a[i] / (double)(_a[i] + _c[i]);
	}

	accurate /= 4.0; recall /= 4.0; f = 2 * accurate*recall / (accurate + recall);
	printf("Accurate = %lf, Recall = %lf , F = %lf\n",accurate,recall, f);
}

int main(int argv, char** argc)
{
	if (argv != 2)
	{
		std::cout << "Usage: [NaiveBayes.exe] [k-fold]" << std::endl;
		return 1;
	}

	int fold = std::atoi(argc[1]);
	for (int i = 0; i < fold; i++)
	{
		reset();
		NaiveBayesLearning(fold);
		k_fold_verify();
	}

	print_result();
	return 0;
}