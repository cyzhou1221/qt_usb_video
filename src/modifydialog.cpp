#include "modifydialog.h"
#include "ui_modifydialog.h"
#include <QDebug>
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
#include <QMessageBox>
#include <QTableView>
#include <QStandardItemModel>

//---------------------患者信息查询、更新、删除界面--------------------------//

static int curRow;       // 选中单元格的行数
QString curId;           // 与选中单元格同行的身份证号
static QStandardItemModel *model = new QStandardItemModel(); // 信息表格
static QString id[30];   // 建立数组，用于记录符合条件的患者 id，便于查询


modifyDialog::modifyDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::modifyDialog)
{
    ui->setupUi(this);
    setWindowTitle(QStringLiteral("查看患者信息"));

    // 创建表格
    ui->tableView->setModel(model);  
    model->setColumnCount(4);  // 列数为 4
    // 设置下表头
    model->setHeaderData(0, Qt::Horizontal, QStringLiteral("姓名"));
    model->setHeaderData(1, Qt::Horizontal, QStringLiteral("性别"));
    model->setHeaderData(2, Qt::Horizontal, QStringLiteral("年龄"));
    model->setHeaderData(3, Qt::Horizontal, QStringLiteral("身份证号"));
    // 统一居左
    ui->tableView->horizontalHeader()->setDefaultAlignment(Qt::AlignLeft);
    // 防止表格内的文本被直接编辑
    ui->tableView->setEditTriggers(QAbstractItemView::NoEditTriggers);
}

modifyDialog::~modifyDialog()
{
    delete ui;
}


void modifyDialog::on_pushButton_clicked()
{
    // "查询"按钮
    QSqlQuery sql_query;
    // 从输入框中读取患者姓名
    QString Queryname = ui->textEdit->toPlainText();
    // SQL 查询语句
    QString select_sql = "select * from qtusb where name = '" + Queryname + "'";
    // 查询到的记录条数，初值为 0
    int counts = 0;
    // 开始查询
    sql_query.prepare(select_sql);
    if(!sql_query.exec())
        qDebug() << sql_query.lastError();
    else
    {   // 进行查询
        while(sql_query.next())
        {   // 读取查询到的信息
            QString name = sql_query.value(0).toString(); // 姓名
            bool sex = sql_query.value(1).toBool();       // 性别
            QString age = sql_query.value(2).toString();  // 年龄
            id[counts] = sql_query.value(3).toString();   // 存储 id 的数组

            model->setItem(counts, 0, new QStandardItem(name));      // 显示姓名
            if(sex) // sex == 1，表示"女性"                           // 显示性别
            {
                QString _ = QStringLiteral("女");
                model->setItem(counts, 1, new QStandardItem(_));
            }
            else    // sex == 0，表示"男性"
            {
                QString _ = QStringLiteral("男");
                model->setItem(counts, 1, new QStandardItem(_));
            }
            model->setItem(counts, 2, new QStandardItem(age));        // 显示年龄
            model->setItem(counts, 3, new QStandardItem(id[counts])); // 显示身份证号
            counts++;
        }
        if(counts == 0)
            QMessageBox::about(nullptr, QStringLiteral("查询结果"), QStringLiteral("未查询到该患者的记录！\n请检查输入的姓名是否正确！"));
    }
}


void modifyDialog::on_tableView_clicked()
{
    // 读取选中记录的行数，之后提取身份证号
    curRow = ui->tableView->currentIndex().row();
    curId = id[curRow];
}


void modifyDialog::on_pushButton_2_clicked()
{
    // "删除"按钮
    if(curId.isNull())
        QMessageBox::about(nullptr, QStringLiteral("提示"), QStringLiteral("未选中要删除记录的任何单元格！"));
    else
    {
        // 利用 SQL 语句执行删除操作
        QSqlQuery sql_query;
        QString delete_sql = "delete from qtusb where id = '" + curId + "'";
        sql_query.prepare(delete_sql);
        if(!sql_query.exec())
            qDebug() << sql_query.lastError();
        else
            QMessageBox::about(nullptr, QStringLiteral("提示"), QStringLiteral("删除成功！"));

        // 清空表格
        model->removeRows(0, model->rowCount());
    }
}


void modifyDialog::on_pushButton_3_clicked()
{
    // "更新"按钮
    if(curId.isNull())
        QMessageBox::about(nullptr, QStringLiteral("提示"), QStringLiteral("未选中要更新记录的任何单元格！"));
    else
    {
        upddialog = new updateDialog;
        upddialog->setModal(true);  // 优先处理当前窗口
        upddialog->show();
        // 清空表格
        model->removeRows(0, model->rowCount());
    }
}
