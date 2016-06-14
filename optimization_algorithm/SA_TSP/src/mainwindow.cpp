#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QDebug>
#include <QDesktopWidget>
#include <QScreen>
#include <QMessageBox>
#include <QMetaEnum>
#include <QString>
#include <QFileDialog>


MainWindow::MainWindow(QWidget *parent) :
  QMainWindow(parent),
  ui(new Ui::MainWindow)
{
  loadFile();
  ui->setupUi(this);
  setGeometry(400, 250, 1280, 768);
  setupDemo(10);

  // random number generator
  generator = std::default_random_engine(std::time(0));
  dis = std::uniform_int_distribution<int>(1, n - 1);
  u = std::uniform_real_distribution<double>(0, 1);

  temperature = 280;
  t_old = 1e30;
  inter_cnt = 0;
  cal_done = false;

  const int monte_carlo = 100 * n;
  int a, b;
  for (int i = 0; i < monte_carlo; i++)
  {
  	do
	{
		a = dis(generator);
		b = dis(generator);
	} while (a == b);

	perturb(path, a, b);

    t_new = calculate(path, city_list, n);
	if (t_new < t_old)
	{
		for (int i = 0; i < n; i++)
		{
			best[i] = path[i];
		}
		t_old = t_new;
	}
  }
}

void MainWindow::loadFile(){
    QString filename = QFileDialog::getOpenFileName(
        this,
        tr("Open Files"),
        "Resources",
        tr("All files (*.*)") );
    std::ifstream input(filename.toStdString());

    input >> n;		// number of cities
    city_list = new City[n];
    char c;
    double x, y;

    for (int i = 0; i < n; i++)
    {
        input >> c >> x >> y;
        city_list[i].set_city(c, x, y);
    }

    path = new int[n];
    best = new int[n];
    initialize(path, n);
}

void MainWindow::setupDemo(int demoIndex)
{
  setupRealtimeDataDemo(ui->customPlot);
  setWindowTitle("Simulated annealing demo by C. Wu");
  statusBar()->clearMessage();
  currentDemoIndex = demoIndex;
  ui->customPlot->replot();
}

void MainWindow::setupRealtimeDataDemo(QCustomPlot *customPlot)
{
#if QT_VERSION < QT_VERSION_CHECK(4, 7, 0)
  QMessageBox::critical(this, "", "You're using Qt < 4.7, this program needs functions that are available with Qt 4.7 to work properly");
#endif
  demoName = "Real Time Data Demo";
  
  // include this section to fully disable antialiasing for higher performance:
  /*
  customPlot->setNotAntialiasedElements(QCP::aeAll);
  QFont font;
  font.setStyleStrategy(QFont::NoAntialias);
  customPlot->xAxis->setTickLabelFont(font);
  customPlot->yAxis->setTickLabelFont(font);
  customPlot->legend->setFont(font);
  */
  customPlot->addGraph(); // blue line
  customPlot->graph(0)->setPen(QPen(Qt::red));
  customPlot->graph(0)->setBrush(QBrush(QColor(240, 255, 200)));
  customPlot->graph(0)->setAntialiasedFill(false);
  customPlot->graph(0)->setChannelFillGraph(customPlot->graph(1));

  customPlot->xAxis->setTickLabelType(QCPAxis::ltDateTime);
  customPlot->xAxis->setDateTimeFormat("hh:mm:ss");
  customPlot->xAxis->setAutoTickStep(false);
  customPlot->xAxis->setTickStep(2);
  customPlot->axisRect()->setupFullAxesBox();
  
  // make left and bottom axes transfer their ranges to right and top axes:
  connect(customPlot->xAxis, SIGNAL(rangeChanged(QCPRange)), customPlot->xAxis2, SLOT(setRange(QCPRange)));
  connect(customPlot->yAxis, SIGNAL(rangeChanged(QCPRange)), customPlot->yAxis2, SLOT(setRange(QCPRange)));
  
  // setup a timer that repeatedly calls MainWindow::realtimeDataSlot:
  connect(&dataTimer, SIGNAL(timeout()), this, SLOT(realtimeDataSlot()));
  dataTimer.start(0); // Interval 0 means to refresh as fast as possible
}

void MainWindow::realtimeDataSlot()
{
  // calculate two new data points:
#if QT_VERSION < QT_VERSION_CHECK(4, 7, 0)
  double key = 0;
#else
  double key = QDateTime::currentDateTime().toMSecsSinceEpoch()/1000.0;
#endif
  static double lastPointKey = 0;
  if (key-lastPointKey > 0.01) // at most add point every 10 ms
  {
    double value0 = qSin(key); //qSin(key*1.6+qCos(key*1.7)*2)*10 + qSin(key*1.2+0.56)*20 + 26;
    double value1 = sa_update(); //qSin(key*1.3+qCos(key*1.2)*1.2)*7 + qSin(key*0.9+0.26)*24 + 26;

    if(cal_done) return;

    if( value1 < 0){
        QString sa_result;
        for(auto i = 0 ; i < n; ++i){
            sa_result.append('A'+path[i]);
        }
        sa_result.append("\nDistance sum = ");
        sa_result.append(QString::number(t_new));
        QMessageBox::information(this,"Path",sa_result);
        cal_done = true;
        return;
    }
    // add data to lines:
    ui->customPlot->graph(0)->addData(key, value1);
    // set data of dots:
    /*
    ui->customPlot->graph(2)->clearData();
    ui->customPlot->graph(2)->addData(key, 1);
    ui->customPlot->graph(3)->clearData();
    ui->customPlot->graph(3)->addData(key, value1);
    */
    // remove data of lines that's outside visible range:
    ui->customPlot->graph(0)->removeDataBefore(key-8);
    // rescale value (vertical) axis to fit the current data:
    ui->customPlot->graph(0)->rescaleValueAxis(true);
    lastPointKey = key;
  }
  // make key axis range scroll with the data (at a constant range size of 8):
  ui->customPlot->xAxis->setRange(key+0.25, 8, Qt::AlignRight);
  ui->customPlot->replot();
  
  // calculate frames per second:
  static double lastFpsKey;
  static int frameCount;
  ++frameCount;
  if (key-lastFpsKey > 2) // average fps over 2 seconds
  {
    ui->statusBar->showMessage(
          QString("%1 FPS, Total Data points: %2")
          .arg(frameCount/(key-lastFpsKey), 0, 'f', 0)
          .arg(ui->customPlot->graph(0)->data()->count())
          , 0);
    lastFpsKey = key;
    frameCount = 0;
  }
}

void MainWindow::bracketDataSlot()
{
#if QT_VERSION < QT_VERSION_CHECK(4, 7, 0)
  double secs = 0;
#else
  double secs = QDateTime::currentDateTime().toMSecsSinceEpoch()/1000.0;
#endif
  
  // update data to make phase move:
  int n = 500;
  double phase = secs*5;
  double k = 3;
  QVector<double> x(n), y(n);
  for (int i=0; i<n; ++i)
  {
    x[i] = i/(double)(n-1)*34 - 17;
    y[i] = qExp(-x[i]*x[i]/20.0)*qSin(k*x[i]+phase);
  }
  ui->customPlot->graph()->setData(x, y);
  
  itemDemoPhaseTracer->setGraphKey((8*M_PI+fmod(M_PI*1.5-phase, 6*M_PI))/k);
  
  ui->customPlot->replot();
  
  // calculate frames per second:
  double key = secs;
  static double lastFpsKey;
  static int frameCount;
  ++frameCount;
  if (key-lastFpsKey > 2) // average fps over 2 seconds
  {
    ui->statusBar->showMessage(
          QString("%1 FPS, Total Data points: %2")
          .arg(frameCount/(key-lastFpsKey), 0, 'f', 0)
          .arg(ui->customPlot->graph(0)->data()->count())
          , 0);
    lastFpsKey = key;
    frameCount = 0;
  }
}

void MainWindow::setupPlayground(QCustomPlot *customPlot)
{
  Q_UNUSED(customPlot)
}

MainWindow::~MainWindow()
{
  delete ui;
}

void MainWindow::screenShot()
{
#if QT_VERSION < QT_VERSION_CHECK(5, 0, 0)
  QPixmap pm = QPixmap::grabWindow(qApp->desktop()->winId(), this->x()+2, this->y()+2, this->frameGeometry().width()-4, this->frameGeometry().height()-4);
#else
  QPixmap pm = qApp->primaryScreen()->grabWindow(qApp->desktop()->winId(), this->x()+2, this->y()+2, this->frameGeometry().width()-4, this->frameGeometry().height()-4);
#endif
  QString fileName = demoName.toLower()+".png";
  fileName.replace(" ", "");
  pm.save("./screenshots/"+fileName);
  qApp->quit();
}

void MainWindow::allScreenShots()
{
#if QT_VERSION < QT_VERSION_CHECK(5, 0, 0)
  QPixmap pm = QPixmap::grabWindow(qApp->desktop()->winId(), this->x()+2, this->y()+2, this->frameGeometry().width()-4, this->frameGeometry().height()-4);
#else
  QPixmap pm = qApp->primaryScreen()->grabWindow(qApp->desktop()->winId(), this->x()+2, this->y()+2, this->frameGeometry().width()-4, this->frameGeometry().height()-4);
#endif
  QString fileName = demoName.toLower()+".png";
  fileName.replace(" ", "");
  pm.save("./screenshots/"+fileName);
  
  if (currentDemoIndex < 19)
  {
    if (dataTimer.isActive())
      dataTimer.stop();
    dataTimer.disconnect();
    delete ui->customPlot;
    ui->customPlot = new QCustomPlot(ui->centralWidget);
    ui->verticalLayout->addWidget(ui->customPlot);
    setupDemo(currentDemoIndex+1);
    // setup delay for demos that need time to develop proper look:
    int delay = 250;
    if (currentDemoIndex == 10) // Next is Realtime data demo
      delay = 12000;
    else if (currentDemoIndex == 15) // Next is Item demo
      delay = 5000;
    QTimer::singleShot(delay, this, SLOT(allScreenShots()));
  } else
  {
    qApp->quit();
  }
}

void MainWindow::initialize(int* path, int n)
{
    for (int i = 0; i < n; i++)
    {
        path[i] = i;
    }
}

void MainWindow::perturb(int* path, int a, int b){
    int tmp = path[a];
    path[a] = path[b];
    path[b] = tmp;
}

void MainWindow::reverse(int* path, int a, int b)
{
    if (a > b) {
        int tmp = a;
        a = b;
        b = tmp;
    }

    int length = (b - a) / 2;
    for (int i = 0; i < length; i++)
    {
        perturb(path, a++, b--);
    }
}

double MainWindow::calculate(int* path, City* city, int n)
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
double MainWindow::sa_update()
{
    int a, b;
    if (temperature > 0.01)
    {
        for(int c = 0 ; c < 100*n ; ++c)
        {
        	++ inter_cnt;
            do
            {
                a = dis(generator);
                b = dis(generator);
            } while (a == b);

            bool is_reverse = false;
            if (u(generator)>0.9) is_reverse = true;
            if (is_reverse) reverse(path, a, b);
            else perturb(path, a, b);
            t_new = calculate(path, city_list, n);
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
        temperature *= 0.98;
    } else
    {
        t_new = t_old;
        t_old = -1;
    }
    return t_old;
}





































