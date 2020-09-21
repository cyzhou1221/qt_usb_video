#include "inputdialog.h"
#include "ui_inputdialog.h"
#include <QDebug>
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
#include <QMessageBox>

//---------------------患者信息输入界面--------------------------//

// 设置变量，即将要录入的信息
static QString addName, addAge, addID;
static int addSex;

inputDialog::inputDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::inputDialog)
{
    ui->setupUi(this);
    setWindowTitle(QStringLiteral("输入患者信息"));
    // radioButton 对应 0，表示男性
    // radioButton_2 对应 1，表示女性
    ui->buttonGroup->setId(ui->radioButton, 0);
    ui->buttonGroup->setId(ui->radioButton_2, 1);
    // 默认患者为男性，可以更改
    ui->radioButton->setChecked(true);
}

inputDialog::~inputDialog()
{
    delete ui;
}

void inputDialog::on_pushButton_2_clicked()
{
    // "取消"按钮，关闭窗口
    this->close();
}

void inputDialog::on_pushButton_clicked()
{
    // "确认"按钮，插入信息
    QSqlQuery sql_query; // 用于执行 SQL 语句
    // 创建表格
    if(!sql_query.exec("create table qtusb(name text, sex numeric, age int, id text primary key)"))
        qDebug() << "Error: Fail to create table.\n" << sql_query.lastError();
    else
        qDebug() << "Table created!\n";

    // 收集输入的信息并判断输入格式是否正确
    addName = ui->textEdit->toPlainText(); // 姓名
    if(addName.isEmpty())                  // 若忘记输入
    {
        QMessageBox::critical(nullptr, QStringLiteral("错误"), QStringLiteral("请输入患者姓名"), QMessageBox::Ok);
        return;
    }
    addSex = ui->buttonGroup->checkedId(); // 性别
    addAge = ui->spinBox->text();          // 年龄
    if(addAge.isEmpty())  // 若忘记输入
    {
        QMessageBox::critical(nullptr, QStringLiteral("错误"), QStringLiteral("请输入患者年龄"), QMessageBox::Ok);
        return;
    }
    addID = ui->textEdit_2->toPlainText();     // 身份证号
    if(addID.isEmpty() || addID.size() != 18)  // 若忘记输入或者不是18位
    {
        QMessageBox::critical(nullptr, QStringLiteral("错误"), QStringLiteral("请输入正确的18位身份证号"), QMessageBox::Ok);
        return;
    }

    // 插入记录
    QString insert_sql = "insert into qtusb values (?, ?, ?, ?)";
    sql_query.prepare(insert_sql);    
    sql_query.addBindValue(addName);
    sql_query.addBindValue(addSex);
    sql_query.addBindValue(addAge.toInt());
    sql_query.addBindValue(addID);

    if(!sql_query.exec()) // 若没有执行语句，输出最后一条错误信息
        qDebug() << sql_query.lastError();
    else
        QMessageBox::about(nullptr, QStringLiteral("提示"), QStringLiteral("插入成功!"));

    // 关闭窗口
    this->close();
}
