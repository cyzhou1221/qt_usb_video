#include "mainwindow.hpp"
#include "ui_mainwindow.h"
#include <QPixmap>
#include <QDebug>
#include <QMouseEvent>

using namespace cv;

// 实时视频帧数
static int frameNumber;

// 轨迹条的最大值、初始值
static const int Max_Value = 15;
static int TrackBar_Value = 5;

// 高斯模糊的输入、输出图像
static Mat srcImage;
static Mat dstImage;

// 录制视频次数，初值为 0
static int times = 0;

// 选择像素点是否有效，初值为否
static bool select_vaild = false;

// 绘制点的最大个数
static int MAX_PLOTTING_NUM = 50;

// 外部函数声明
void on_Trackbar(int, void*);
QImage Mat2QImage(const Mat &);


MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ///----------- 显示界面
    ui->setupUi(this);

    ///----------- 串口初始化
    // 定义结构体，用来存放串口各个参数
    struct PortSettings myPortSetting = {BAUD9600, DATA_8, PAR_NONE, STOP_1, FLOW_OFF, 200};
    // 定义串口对象，指定串口名和查询模式，采用 Polling 模式
    myPort = new Win_QextSerialPort("com1", myPortSetting, QextSerialBase::Polling);
    // 定义计时器，每 200ms 更新一次显示信息
    timer1 = new QTimer(this);
    timer1->setInterval(200);
    connect(timer1, SIGNAL(timeout()), this, SLOT(readSerialPort()));

    ///----------- 图像处理初始化
    // 设置最多可选择的像素点数目
    pointNumber = 10;
    ui->point_Num->setText(QString::number(pointNumber));
    // 为 label (实时视频显示窗口)安装事件过滤器
    ui->label->installEventFilter(this);
    // QCustomPlot 实时曲线绘制初始化
    initRealtimePlot();
    // 定义定时器，用于显示实时视频
    timer2 = new QTimer(this);
    connect(timer2, SIGNAL(timeout()), this, SLOT(nextFrame()));
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::initRealtimePlot() // QCustomPlot 实时曲线绘制初始化
{
    // 设置坐标轴参数：名称及 y 轴范围
    ui->customplot->xAxis->setLabel("FN(Frame Number)");
    ui->customplot->yAxis->setLabel("RGB");    
    ui->customplot->yAxis->setRange(0, 255);

    // 添加曲线，颜色设置为红、绿、蓝，分别对应 R, G, B 三分量
    ui->customplot->addGraph();
    ui->customplot->graph(0)->setPen(QPen(Qt::red));
    ui->customplot->addGraph();
    ui->customplot->graph(1)->setPen(QPen(Qt::green));
    ui->customplot->addGraph();
    ui->customplot->graph(2)->setPen(QPen(Qt::blue));
}

void MainWindow::addData(double x, double r, double g, double b) // 添加绘图数据点
{
    if(xVector.size() == MAX_PLOTTING_NUM) // 如已满，弹出首数据
    {
        xVector.pop_front();
        rValue.pop_front();
        gValue.pop_front();
        bValue.pop_front();
    }
    xVector.push_back(x);
    rValue.push_back(r);
    gValue.push_back(g);
    bValue.push_back(b);
}

bool MainWindow::eventFilter(QObject *watched, QEvent *event) // 事件过滤器，用于返回选择像素点的坐标
{
    if(watched == ui->label) // label 表示实时视频显示窗口
    {
        if(event->type() == QEvent::MouseButtonPress && select_vaild == true) // 如果事件为鼠标点击，并且像素选择开关开启
        {
            if(Points.size() < pointNumber && capture.isOpened())
            {
                // 转化为鼠标事件
                QMouseEvent *mouseEvent = static_cast<QMouseEvent *>(event);
                // 将全局坐标转化成相对与 label 的坐标
                QPoint temp = ui->label->mapFromGlobal(mouseEvent->globalPos());
                // 存放坐标
                Points.append(temp);
                return true;
            }
            else
                return false;
        }
        else
            return false;
    }
    else
        return QMainWindow::eventFilter(watched, event);
}

void on_Trackbar(int radius, void* ) // 轨迹条
{
    // 根据轨迹条上的值计算 kernel 的尺寸
    int kernel_Size = radius*2 + 1;
    // 高斯模糊处理
    GaussianBlur(srcImage, dstImage, Size(kernel_Size, kernel_Size), 0, 0);
}

QImage Mat2QImage(const Mat &inputMat) // 将 Mat 转化为 QImage
{
    Mat tmpMat;
    QImage qImg;
    if(inputMat.channels() == 1) // 灰度图像
    {
        tmpMat = inputMat;
        qImg = QImage((const uchar*)tmpMat.data,   // 指向所用图像数据块的指针
                    tmpMat.cols, tmpMat.rows,
                    tmpMat.cols*tmpMat.channels(), // bytesPerLine，每行的字节数
                    QImage::Format_Indexed8);      // 利用 8bits 存储灰度值
    }
    else
    {   // Mat 是 BGR 排列, QImage 是 RGB 排列，利用 cvtColor 变换颜色空间
        cvtColor(inputMat, tmpMat, CV_BGR2RGB);
        qImg = QImage((const uchar*)tmpMat.data,   // 指向所用图像数据块的指针
                    tmpMat.cols, tmpMat.rows,
                    tmpMat.cols*tmpMat.channels(), // bytesPerLine，每行的字节数
                    QImage::Format_RGB888);        // 利用 24bits 真彩色存储 RGB 值
    }
    return qImg.copy(); // deep copy
}

void MainWindow::nextFrame() // 设置下一帧图像
{
    Mat frame, dst;
    int size;
    int posx, posy;
    QColor color;
    QString text = "";
    int r, g, b;                   // RGB 值
    double s1 = 0, s2 = 0, s3 = 0; // 对 RGB 值分别求和
    int i;
    double temp;

    capture >> frame;
    if(!frame.empty())
    {
        frameNumber++;
        ui->label_3->setText(tr("\nSystem is running!\n\nThe number of current frame is %1.").arg(frameNumber, 5));

        //---label_2 显示灰度图像
        cvtColor(frame, dst, CV_RGB2GRAY);
        image = Mat2QImage(dst);
        ui->label_2->setPixmap(QPixmap::fromImage(image));

        //---如果已选择像素点，则更改 frame，标记像素点并显示其信息：位置和 RGB 分量
        //---否则，显示原图像
        size = Points.size();
        if(size > 0)
        {
            image = Mat2QImage(frame);
            for(i = 0; i < size; i++)
            {
                // 获取像素点坐标
                posx = Points[i].x();
                posy = Points[i].y();
                // 提取像素 RGB 值
                color = image.pixel(Points[i]);
                r = color.red();
                g = color.green();
                b = color.blue();
                s1 += r;
                s2 += g;
                s3 += b;
                // 显示像素点坐标及对应的 RGB 值
                text += QString().sprintf("Pos (X, Y): (%3d, %3d)  (R, G, B):%3d, %3d, %3d\n", posx, posy, r, g, b);
                // 标记坐标点(红色)
                circle(frame, Point(posx, posy), 3, Scalar(0, 0, 255), -1); // 3 表示半径，Scalar(B,G,R)，-1 表填充
            }
            // 设置显示文本
            ui->label_16->setText(text);
            // 添加绘图数据点
            addData(frameNumber, s1/size, s2/size, s3/size);
            ui->customplot->graph(0)->setData(xVector, rValue);
            ui->customplot->graph(1)->setData(xVector, gValue);
            ui->customplot->graph(2)->setData(xVector, bValue);
        }
        else
            ui->label_16->setText(QString(""));

        // 设置 X 轴范围
        temp = frameNumber - MAX_PLOTTING_NUM*(1.05);
        if(temp < 0)
            temp = 0;
        ui->customplot->xAxis->setRange(temp, frameNumber + MAX_PLOTTING_NUM*(0.05));
        // 显示绘制图形
        ui->customplot->replot();

        //---label 显示 frame
        image = Mat2QImage(frame);
        ui->label->setPixmap(QPixmap::fromImage(image));
    }
}

void MainWindow::readSerialPort() // 读串口函数
{
    QByteArray temp = myPort->readAll();    // 读取串口缓冲区中的所有数据
    ui->textBrowser->insertPlainText(temp); // 将数据显示在对应窗口
}

/////////////////////////camera control 相机开关
void MainWindow::on_open_camera_clicked()
{
    double rate;
    //---若有已选像素点，先清空
    Points.clear();
    //---打开相机
    if(!capture.isOpened())
        capture.open(0);
    if(capture.isOpened())
    {
        rate = capture.get(CAP_PROP_FPS);
        //---设置与 FPS 相匹配的时间间隔(毫秒)
        timer2->setInterval(int(1000/rate));
        frameNumber = 0;
        //---启动定时器，进行实时显示
        if(!timer2->isActive())
            timer2->start();
    }
    else
        QMessageBox::warning(nullptr, "Warning", QStringLiteral("相机无法正常开启！"));
}

void MainWindow::on_close_camera_clicked()
{
    //---关闭实时显示
    timer2->stop();
    //---关闭相机
    if(capture.isOpened())
        capture.release();
    //---填充空白图像
    image.fill(qRgba(191, 191, 191, 100));
    ui->label->setPixmap(QPixmap::fromImage(image));
    ui->label_2->setPixmap(QPixmap::fromImage(image));
}

/////////////////////////image processing 图像处理
void MainWindow::on_edge_detect_clicked()
{
    Mat cannyImg;
    Mat frame;
    capture >> frame;
    // 调用 canny 边缘检测算法
    Canny(frame, cannyImg, 20, 200); // (src, dst, threshold1, threshold2)
    namedWindow("Edge Detect");
    imshow("Edge Detect", cannyImg);
}

void MainWindow::on_gauss_blur_clicked()
{
    //---检查相机是否开启
    if(!capture.isOpened())
    {
        QMessageBox::warning(nullptr, "Warning", QStringLiteral("相机未开启，请开启相机！"));
        return;
    }

    //---新建窗口
    namedWindow("Gauss Blur");
    //---创建轨迹条
    createTrackbar("Radius", "Gauss Blur", &TrackBar_Value, Max_Value, on_Trackbar);
    QMessageBox::information(nullptr, "Hint", QStringLiteral("开始模糊处理，可通过上方轨迹条调节模糊效果，按 'x' 结束！"));

    timer2->stop();           // 停止软件界面内视频显示
    //---开始模糊处理
    while(true)
    {
        // 读入图像
        capture >> srcImage;
        // 根据轨迹条的值进行高斯模糊
        on_Trackbar(TrackBar_Value, nullptr);
        // 显示模糊后的图像
        imshow("Gauss Blur", dstImage);
        // 按 'x' 退出
        if(waitKey(int(1000/30)) == 'x')
            break;
    }
    waitKey(1500);
    destroyWindow("Gauss Blur");
    timer2->start();         // 恢复正常视频显示
}

/////////////////////////video recording and playing 视频录制与播放
void MainWindow::on_record_video_clicked()
{
    Mat frame;
    QString str;

    //---检查相机是否开启
    if(!capture.isOpened())
    {
        QMessageBox::warning(nullptr, "Warning", QStringLiteral("相机未开启，请开启相机！"));
        return;
    }

    //---获取录制视频时长
    double rate = capture.get(CAP_PROP_FPS);
    str = ui->time_length->text();
    if(str.isEmpty())
    {
        QMessageBox::information(nullptr, "Hint", QStringLiteral("请输入录制视频时长！"));
        return;
    }
    int video_length = int(rate*str.toInt());  // 总录制帧数

    //---初始化 VideoWriter
    times++;
    str = QString("./myRecord_%1.avi").arg(times);
    int codec = VideoWriter::fourcc('P','I','M','1');
    capture >> frame;
    bool isColor = (frame.type() == CV_8UC3); // 检查是否为彩色图像
    writer.open(str.toLocal8Bit().data(), codec, rate, frame.size(), isColor);

    //---录制前提示
    str = QString::fromLocal8Bit("开始录制视频 %1，录制过程中请勿再次点击此按钮！").arg(times);
    QMessageBox::information(nullptr, "Hint", str);

    //---录制视频
    timer2->stop();           // 停止软件界面内视频显示
    namedWindow("Recording");
    while(video_length > 0)
    {
        capture.read(frame);
        writer.write(frame);
        imshow("Recording", frame);
        if(waitKey(int(1000/rate)) >= 0) // 录制过程中按任意键结束录制
            break;
        video_length--;
    }
    writer.release();

    //---录制完毕提示
    str = QString::fromLocal8Bit("视频 %1 录制完毕！").arg(times);
    QMessageBox::information(nullptr, "Hint", str);
    waitKey(1000);
    destroyWindow("Recording");
    timer2->start();         // 恢复正常视频显示
}

void MainWindow::on_play_video_clicked()
{
    Mat frame;
    double rate;
    bool flag = false;

    //---打开视频文件
    QString filename = QFileDialog::getOpenFileName(this, tr("Open Video File"), ".", tr("Video Files(*.avi *.mp4 *.flv *.mkv)"));
    player.open(filename.toLocal8Bit().data());
    if(player.isOpened())
    {
        rate = player.get(CAP_PROP_FPS);
        //---播放视频文件时暂停实时视频显示
        if(timer2->isActive())
        {
            flag = true;
            timer2->stop();
        }
        //---创建视频播放窗口
        namedWindow("Video Play");
        while(true)
        {
            player.read(frame);
            if(frame.empty())
                break;
            imshow("Video Play", frame);
            if(waitKey(int(1000/rate)) >= 0) // 播放过程中按任意键结束播放
                break;
        }
        player.release();
        waitKey(1000);
        destroyWindow("Video Play");
        //---播放完毕时恢复实时视频显示，如果之前有的话
        if(flag)
            timer2->start();
     }
}

/////////////////////////points selection 像素点选择
void MainWindow::on_select_points_clicked()
{
    if(!select_vaild)
        select_vaild = true;
    else
        select_vaild = false;
}

void MainWindow::on_clear_points_clicked()
{
    Points.clear();
}

/////////////////////////serial port 串口开关
void MainWindow::on_open_port_clicked()
{
    myPort->open(QIODevice::ReadOnly); // 以只读方式打开串口
    timer1->start();                   // 启动定时器
}

void MainWindow::on_close_port_clicked()
{
    timer1->stop();  // 关闭定时器
    myPort->close(); // 关闭串口
    ui->textBrowser->setText(""); // 关闭后清空显示区
}

/////////////////////////database 数据库部分
void MainWindow::on_inputdialog_triggered()
{
    dialog = new inputDialog;
    dialog->setModal(true); // 优先处理当前窗口
    dialog->show();
}

void MainWindow::on_modifydialog_triggered()
{
    mddialog = new modifyDialog;
    mddialog->setModal(true);
    mddialog->show();
}

void MainWindow::on_getalldialog_triggered()
{
    alldialog = new getallDialog;
    alldialog->setModal(true);
    alldialog->show();
}
