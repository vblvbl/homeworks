#ifndef MAINWINDOW_H
#define MAINWINDOW_H
#include <iostream>
#include <fstream>
#include <random>
#include <functional>
#include <ctime>
#include <iomanip>
#include <unordered_set>
#include <assert.h>
#include <time.h>
#include <QMainWindow>
#include <QTimer>
#include "../../qcustomplot.h" 



const int N = 1000;		// 种群数目
const double p_c = 0.65;	// 交配概率
const double p_m = 0.25;	// 变异概率
static double sel_p[N];
static double sel_pop[N + 1];
static int sel_temp[N];

struct City{
    char name;
    double x;
    double y;
    void set_city(char c, double _x, double _y){
        name = c; x = _x; y = _y;
    }
};
static City* city_list;

struct Path{
  int path[50];
  double f;
  int n;
  double adp;

  Path(){};

  Path(int _n, std::default_random_engine& generator)
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
    if (city_list)  update_f();
  }

  void assign(Path& pa)
  {
    n = pa.n;
    for (int i = 0; i < n; i++) path[i] = pa.path[i];
    f = pa.f; adp = pa.adp;
  }

  void update_f()
  {
    f = calculate(path, n, city_list); // 计算距离
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


namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
  Q_OBJECT
  
public:
  explicit MainWindow(QWidget *parent = 0);
  ~MainWindow();
  
  void setupDemo(int demoIndex);
  void setupRealtimeDataDemo(QCustomPlot *customPlot);
  void setupPlayground(QCustomPlot *customPlot);

private slots:
  void realtimeDataSlot();
  void bracketDataSlot();
  void screenShot();
  void allScreenShots();
  
private:
  Ui::MainWindow *ui;
  QString demoName;
  QTimer dataTimer;
  QCPItemTracer *itemDemoPhaseTracer;
  int currentDemoIndex;

  int n, inter_cnt;

  int* path;
  int* best;
  double temperature;
  double t_old, t_new, delta_t;
  bool cal_done;
  std::default_random_engine generator;
  std::uniform_int_distribution<int> dis;
  std::uniform_real_distribution<double> u;
  std::uniform_real_distribution<double> real;
  int* path_ga;
  int* best_ga;
  std::vector<Path> population;

  Path global;

  void loadFile();
  void initialize(int* path, int n);
  void perturb(int* path, int a, int b);
  void reverse(int* path, int a, int b);
  double calculate(int* path, City* city, int n);
  void evaluate(std::vector<Path>& paths);
  double sa_update();
  double ga_update();
};

#endif // MAINWINDOW_H
