#ifndef MAINWINDOW_HPP
#define MAINWINDOW_HPP

#include <iostream>
#include <stdio.h>
#include <string.h>

#include <QMainWindow>
#include <QApplication>
#include <QFileDialog>
#include <QImage>
#include <QTimer>
#include <QtPrintSupport/QPrintDialog>

#include "opencv2/core/core.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/video/tracking.hpp"
#include "opencv2/features2d/features2d.hpp"
#include "opencv2/flann/flann.hpp"
#include "opencv2/calib3d/calib3d.hpp"
#include "opencv2/core/core_c.h"
#include "opencv2/imgproc/imgproc_c.h"
#include "opencv2/highgui/highgui_c.h"

#include "win_qextserialport.h"

#include <winioctl.h>
#include <initguid.h>

#include "inputdialog.h"
#include "modifydialog.h"
#include "getalldialog.h"
#include "qcustomplot.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    // QCustomPlot 实时曲线绘制初始化
    void initRealtimePlot();
    // 添加绘图数据点
    void addData(double x, double r, double g, double b);

protected:
    // 事件过滤器，用于返回选择像素点的坐标
    bool eventFilter(QObject *watched, QEvent *event);

private:
    Ui::MainWindow *ui;

    inputDialog *dialog;
    modifyDialog *mddialog;
    getallDialog *alldialog;

    cv::Mat src;
    cv::VideoWriter writer;
    cv::VideoCapture capture, player;

    QImage image;
    QTimer *timer1, *timer2;
    // timer1 for serial port communication
    // timer2 for camera

    Win_QextSerialPort *myPort; // 串口

    QVector<QPoint> Points;     // 光标点击位置相对于 label 的坐标
    int pointNumber;            // 最多可选择的像素点数目，在 ~.cpp 中设置

    // 实时曲线绘制所需数据
    QVector<double> xVector;    // 帧数
    QVector<double> rValue;     // RGB 色彩均值
    QVector<double> gValue;
    QVector<double> bValue;

private slots:
    void nextFrame();       // 设置下一帧图像
    void readSerialPort();  // 读串口函数

    void on_open_camera_clicked();
    void on_close_camera_clicked();

    void on_edge_detect_clicked();
    void on_gauss_blur_clicked();

    void on_record_video_clicked();
    void on_play_video_clicked();

    void on_select_points_clicked();
    void on_clear_points_clicked();

    void on_open_port_clicked();
    void on_close_port_clicked();

    void on_inputdialog_triggered();
    void on_modifydialog_triggered();
    void on_getalldialog_triggered();
};

#endif // MAINWINDOW_HPP
