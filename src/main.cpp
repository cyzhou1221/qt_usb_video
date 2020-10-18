#include "mainwindow.hpp"
#include <QApplication>
#include <QDebug>
#include <QSqlDatabase>
#include <QSqlError>
#include <QString>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    // 创建数据库
    QSqlDatabase database = QSqlDatabase::addDatabase("QSQLITE");
    database.setDatabaseName("./qtusb.db");

    // 检查数据库是否开启
    if(!database.open())
        qDebug() << "Error: Failed to connect database!\n" << database.lastError();
    else
        qDebug() << "Connect to database successfully!\n";

    // 创建名为"LPI 诊断系统"的主窗口
    MainWindow w;
    w.setWindowTitle(QStringLiteral("LPI 诊断系统"));
    w.show();

    return a.exec();
}
