#include <iostream>
#include <direct.h>
#include <io.h>
#include <stdio.h>
#include <malloc.h>
#include <string>
#include <vector>
#include <set>
#include <map>
#include <random>
#include <fstream>
#include <sstream>
#include <time.h>
#include "svm.h"
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#define Malloc(type,n) (type *)malloc((n)*sizeof(type))
const int S = 5;
const char labels[4][50] = { "c1_atheism", "c2_sci.crypt", "c3_talk.politics.guns", "c4_comp.sys.mac.hardware" };
std::set<int> test_set;
std::vector<std::string> text_files[4];
std::map<std::string, double> idf;		// log(N/df), df为出现词i的文档数
std::set<std::string> doc;	// 所有词的集合
std::vector<std::multiset<std::string> > docs;	// 每个文件的词

size_t Vocabulary = 0;
size_t n_file = 0;

static char *line = NULL;
static int max_line_len;

class vec{
public:
	double* _vec;
	vec(int n) { _vec = new double[n]; }
	vec() { _vec = new double[Vocabulary]; }
	~vec(){ delete[] _vec; }
};
vec* df;

void print_null(const char *s) {}

void exit_with_help()
{
	printf(
		"Usage: svm-train [options] training_set_file [model_file]\n"
		"options:\n"
		"-s svm_type : set type of SVM (default 0)\n"
		"	0 -- C-SVC		(multi-class classification)\n"
		"	1 -- nu-SVC		(multi-class classification)\n"
		"	2 -- one-class SVM\n"
		"	3 -- epsilon-SVR	(regression)\n"
		"	4 -- nu-SVR		(regression)\n"
		"-t kernel_type : set type of kernel function (default 2)\n"
		"	0 -- linear: u'*v\n"
		"	1 -- polynomial: (gamma*u'*v + coef0)^degree\n"
		"	2 -- radial basis function: exp(-gamma*|u-v|^2)\n"
		"	3 -- sigmoid: tanh(gamma*u'*v + coef0)\n"
		"	4 -- precomputed kernel (kernel values in training_set_file)\n"
		"-d degree : set degree in kernel function (default 3)\n"
		"-g gamma : set gamma in kernel function (default 1/num_features)\n"
		"-r coef0 : set coef0 in kernel function (default 0)\n"
		"-c cost : set the parameter C of C-SVC, epsilon-SVR, and nu-SVR (default 1)\n"
		"-n nu : set the parameter nu of nu-SVC, one-class SVM, and nu-SVR (default 0.5)\n"
		"-p epsilon : set the epsilon in loss function of epsilon-SVR (default 0.1)\n"
		"-m cachesize : set cache memory size in MB (default 100)\n"
		"-e epsilon : set tolerance of termination criterion (default 0.001)\n"
		"-h shrinking : whether to use the shrinking heuristics, 0 or 1 (default 1)\n"
		"-b probability_estimates : whether to train a SVC or SVR model for probability estimates, 0 or 1 (default 0)\n"
		"-wi weight : set the parameter C of class i to weight*C, for C-SVC (default 1)\n"
		"-v n: n-fold cross validation mode\n"
		"-q : quiet mode (no outputs)\n"
		);
	exit(1);
}

void exit_input_error(int line_num)
{
	fprintf(stderr, "Wrong input format at line %d\n", line_num);
	exit(1);
}

struct svm_parameter param;		// set by parse_command_line
struct svm_problem prob;		// set by read_problem
struct svm_model *model;
struct svm_node *x_space;
int cross_validation;
int nr_fold;


static char* readline(FILE *input)
{
	int len;

	if (fgets(line, max_line_len, input) == NULL)
		return NULL;

	while (strrchr(line, '\n') == NULL)
	{
		max_line_len *= 2;
		line = (char *)realloc(line, max_line_len);
		len = (int)strlen(line);
		if (fgets(line + len, max_line_len - len, input) == NULL)
			break;
	}
	return line;
}

void do_cross_validation()
{
	int i;
	int total_correct = 0;
	double total_error = 0;
	double sumv = 0, sumy = 0, sumvv = 0, sumyy = 0, sumvy = 0;
	double *target = Malloc(double, prob.l);

	svm_cross_validation(&prob, &param, nr_fold, target);
	if (param.svm_type == EPSILON_SVR ||
		param.svm_type == NU_SVR)
	{
		for (i = 0; i < prob.l; i++)
		{
			double y = prob.y[i];
			double v = target[i];
			total_error += (v - y)*(v - y);
			sumv += v;
			sumy += y;
			sumvv += v*v;
			sumyy += y*y;
			sumvy += v*y;
		}
		printf("Cross Validation Mean squared error = %g\n", total_error / prob.l);
		printf("Cross Validation Squared correlation coefficient = %g\n",
			((prob.l*sumvy - sumv*sumy)*(prob.l*sumvy - sumv*sumy)) /
			((prob.l*sumvv - sumv*sumv)*(prob.l*sumyy - sumy*sumy))
			);
	}
	else
	{
		for (i = 0; i < prob.l; i++)
			if (target[i] == prob.y[i])
				++total_correct;
		printf("Cross Validation Accuracy = %g%%\n", 100.0*total_correct / prob.l);
	}
	free(target);
}

void parse_command_line(int _nr)
{
	int i;
	void(*print_func)(const char*) = NULL;	// default printing to stdout

	// default values
	param.svm_type = C_SVC;
	param.kernel_type = RBF;
	param.degree = 3;
	param.gamma = 0;	// 1/num_features
	param.coef0 = 0;
	param.nu = 0.5;
	param.cache_size = 100;
	param.C = 1;
	param.eps = 1e-3;
	param.p = 0.1;
	param.shrinking = 1;
	param.probability = 0;
	param.nr_weight = 0;
	param.weight_label = NULL;
	param.weight = NULL;
	cross_validation = 0;
	param.kernel_type = 0;
	cross_validation = 1;
	nr_fold = _nr;

	svm_set_print_string_function(print_func);
}

// read in a problem (in svmlight format)

void read_problem(const char *filename)
{
	int max_index, inst_max_index, i;
	size_t elements, j;
	FILE *fp = fopen(filename, "r");
	char *endptr;
	char *idx, *val, *label;

	if (fp == NULL)
	{
		fprintf(stderr, "can't open input file %s\n", filename);
		exit(1);
	}

	prob.l = 0;
	elements = 0;

	max_line_len = 1024;
	line = Malloc(char, max_line_len);
	while (readline(fp) != NULL)
	{
		char *p = strtok(line, " \t"); // label

		// features
		while (1)
		{
			p = strtok(NULL, " \t");
			if (p == NULL || *p == '\n') // check '\n' as ' ' may be after the last feature
				break;
			++elements;
		}
		++elements;
		++prob.l;
	}
	rewind(fp);

	prob.y = Malloc(double, prob.l);
	prob.x = Malloc(struct svm_node *, prob.l);
	x_space = Malloc(struct svm_node, elements);

	max_index = 0;
	j = 0;
	for (i = 0; i < prob.l; i++)
	{
		inst_max_index = -1; // strtol gives 0 if wrong format, and precomputed kernel has <index> start from 0
		readline(fp);
		prob.x[i] = &x_space[j];
		label = strtok(line, " \t\n");
		if (label == NULL) // empty line
			exit_input_error(i + 1);

		prob.y[i] = strtod(label, &endptr);
		if (endptr == label || *endptr != '\0')
			exit_input_error(i + 1);

		while (1)
		{
			idx = strtok(NULL, ":");
			val = strtok(NULL, " \t");

			if (val == NULL)
				break;

			errno = 0;
			x_space[j].index = (int)strtol(idx, &endptr, 10);
			if (endptr == idx || errno != 0 || *endptr != '\0' || x_space[j].index <= inst_max_index)
				exit_input_error(i + 1);
			else
				inst_max_index = x_space[j].index;

			errno = 0;
			x_space[j].value = strtod(val, &endptr);
			if (endptr == val || errno != 0 || (*endptr != '\0' && !isspace(*endptr)))
				exit_input_error(i + 1);

			++j;
		}

		if (inst_max_index > max_index)
			max_index = inst_max_index;
		x_space[j++].index = -1;
	}

	if (param.gamma == 0 && max_index > 0)
		param.gamma = 1.0 / max_index;

	if (param.kernel_type == PRECOMPUTED)
		for (i = 0; i < prob.l; i++)
		{
			if (prob.x[i][0].index != 0)
			{
				fprintf(stderr, "Wrong input format: first column must be 0:sample_serial_number\n");
				exit(1);
			}
			if ((int)prob.x[i][0].value <= 0 || (int)prob.x[i][0].value > max_index)
			{
				fprintf(stderr, "Wrong input format: sample_serial_number out of range\n");
				exit(1);
			}
		}

	fclose(fp);
}
int predictor(int _nr)
{
	char input_file_name[1024] = "train_set";
	char model_file_name[1024] = "train_set.model";
	const char *error_msg;

	parse_command_line(_nr);
	read_problem(input_file_name);
	error_msg = svm_check_parameter(&prob, &param);

	if (error_msg)
	{
		fprintf(stderr, "ERROR: %s\n", error_msg);
		exit(1);
	}

	if (cross_validation)
	{
		do_cross_validation();
	}
	else
	{
		model = svm_train(&prob, &param);
		if (svm_save_model(model_file_name, model))
		{
			fprintf(stderr, "can't save model to file %s\n", model_file_name);
			exit(1);
		}
		svm_free_and_destroy_model(&model);
	}
	svm_destroy_param(&param);
	free(prob.y);
	free(prob.x);
	free(x_space);
	free(line);

	return 0;
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

void random_vector(std::vector<int>& a)
{
	int index, tmp, i, n = a.size();
	srand(time(NULL));
	for (i = 0; i < n; i++)
	{
		index = rand() % (n - i) + i;
		if (index != i)
		{
			tmp = a[i];
			a[i] = a[index];
			a[index] = tmp;
		}
	}
}

inline bool IsAlpha(char& i)
{
	return (i >= 'a' && i <= 'z') || (i >= 'A' && i <= 'Z');
}

inline bool IsZero(double d)
{
	return (d > -1e-10 && d < 1e-10);
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

int Split2Word(std::string& filename)
{
	std::ifstream file(filename);
	if (!file.is_open()) return 0;
	std::ostringstream buf;
	char ch;
	while (buf&&file.get(ch)) buf.put(ch);
	file.close();	// 关闭文件
	std::string text = buf.str();
	size_t last = 0, word_count = 0;
	std::string new_str;
	docs.push_back(std::multiset<std::string>());
	for (size_t i = 0; i < text.size(); ++i)	{		if (!IsAlpha(text[i]))		{			new_str = text.substr(last, i - last);
			doc.insert(new_str);		// 读入总词表
			docs[n_file].insert(new_str);
			++word_count;
			while (!IsAlpha(text[i]) && i < text.size()) ++i;
			last = i;
		}	}
	++n_file;	// 读下一个文件
	return word_count; //返回一共多少个单词
}

void SVM()
{
	n_file = 0;		// 初始化文件序号
	for (int i = 0; i < 4; ++i) { getFiles(labels[i], text_files[i]); }	// 读入文件
	for (int i = 0; i < 4; ++i)
	{
		for (int j = 0; j < text_files[i].size(); j++)
			Vocabulary += Split2Word(text_files[i][j]);
	}

	//test_set_init();
	df = new vec(Vocabulary);
	// 计算df值
	int n_set = 0;
	for (std::set<std::string>::iterator it = doc.begin(); it != doc.end(); ++it)
	{
		df->_vec[n_set] = 0;
		for (int i = 0; i < docs.size(); ++i){
			if (docs[i].find(*it) != docs[i].end()){
				df->_vec[n_set] ++;
			}
		}
		++n_set;
	}

	// 计算idf值
	for (int i = 0; i < doc.size(); ++i)
	{
		df->_vec[i] = std::log(docs.size() / df->_vec[i]);
	}

	vec* doc_vec = new vec[docs.size()];
	for (int i = 0; i < docs.size(); ++i)	// 遍历所有文件
	{
		n_set = 0;
		for (std::set<std::string>::iterator it = doc.begin(); it != doc.end(); ++it)
		{
			doc_vec[i]._vec[n_set] = docs[i].count(*it) * df->_vec[n_set];
			++n_set;
		}
	}

	std::ofstream nor("train_set");

	std::vector<int> random_indices;
	for (int i = 0; i < docs.size(); ++i)
	{
		random_indices.push_back(i);
	}

	random_vector(random_indices);

	for (std::vector<int>::iterator it = random_indices.begin();
		it != random_indices.end(); ++it)
	{
		int i = *it;
		nor << get_class(i) << " ";
		for (int j = 0; j < Vocabulary; ++j)
		{
			if (!IsZero(doc_vec[i]._vec[j]))
				nor << j << ":" << doc_vec[i]._vec[j] << " ";
		}
		nor << std::endl;
	}
	nor.close();
	delete[] doc_vec;
}

int main(int argv, char** argc)
{
	if (argv != 2)
	{
		std::cout << "Usage: [SVM.exe] [k_fold]" << std::endl;
		return -1;
	}
	SVM();
	predictor(atoi(argc[1]));
	//std::cout << "Class = " << labels[Classify_naive_Bayes_text(std::string(argc[1]))];
	return 0;
}