#include "modifydialog.h"
#include "updatedialog.h"
#include "ui_updatedialog.h"
#include <QDebug>
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
#include <QMessageBox>

//---------------------患者信息更新界面--------------------------//

extern QString curId; // 当前身份证号
// 设置更新框中的变量
static QString addName, addAge, addID;
static int addSex;

updateDialog::updateDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::updateDialog)
{
    ui->setupUi(this);
    setWindowTitle(QStringLiteral("更新患者信息"));
    // 同inputdialog.cpp
    // radioButton 对应 0，表示男性
    // radioButton_2 对应 1，表示女性
    ui->buttonGroup->setId(ui->radioButton, 0);
    ui->buttonGroup->setId(ui->radioButton_2, 1);
    // 默认患者为男性，可以更改
    ui->radioButton->setChecked(true);
}

updateDialog::~updateDialog()
{
    delete ui;
}

void updateDialog::on_pushButton_2_clicked()
{
    // "返回"按钮，关闭窗口
    this->close();
}

void updateDialog::on_pushButton_clicked()
{
    // 收集输入的更新信息并判断输入格式是否正确
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

    // "更新"按钮，更新患者信息
    QSqlQuery sql_query;
    // 更新当前身份证号所在记录
    QString update_sql = "update qtusb set name = :name, sex = :sex, age = :age, id = :newid where id = :id";
    sql_query.prepare(update_sql);
    sql_query.bindValue(":name", addName);
    sql_query.bindValue(":sex", addSex);
    sql_query.bindValue(":age", addAge);
    sql_query.bindValue(":newid", addID);
    sql_query.bindValue(":id", curId);

    if(!sql_query.exec())  // 若没有执行语句，输出最后一条错误信息
       qDebug() << sql_query.lastError();
    else
        QMessageBox::about(nullptr, QStringLiteral("提示"), QStringLiteral("更新成功!"));

    // 关闭窗口
    this->close();
}
