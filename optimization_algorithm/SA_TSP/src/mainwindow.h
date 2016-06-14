#ifndef MAINWINDOW_H
#define MAINWINDOW_H
#include <iostream>
#include <fstream>
#include <random>
#include <functional>
#include <ctime>
#include <iomanip>

#include <QMainWindow>
#include <QTimer>
#include "../../qcustomplot.h" 

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
  struct City{
      char name;
      double x;
      double y;
      void set_city(char c, double _x, double _y){
          name = c; x = _x; y = _y;
      }
  };
  City* city_list;
  int* path;
  int* best;
  double temperature;
  double t_old, t_new, delta_t;
  bool cal_done;

  std::default_random_engine generator;
  std::uniform_int_distribution<int> dis;
  std::uniform_real_distribution<double> u;

  void loadFile();
  void initialize(int* path, int n);
  void perturb(int* path, int a, int b);
  void reverse(int* path, int a, int b);
  double calculate(int* path, City* city, int n);
  double sa_update();
};

#endif // MAINWINDOW_H
