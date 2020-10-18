#include "getalldialog.h"
#include "ui_getalldialog.h"
#include <QtDebug>
#include <QTableView>
#include <QHeaderView>
#include <QStandardItemModel>
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
#include <QVector>
#include <QFileDialog>
#include <QDesktopServices>
#include <ActiveQt/QAxObject>
#include <QMessageBox>

//---------------------全部患者信息显示界面--------------------------//

// 用于存储所有患者身份证号的数组
static QVector<QString> id;

getallDialog::getallDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::getallDialog)
{
    ui->setupUi(this);
    setWindowTitle(QStringLiteral("全部患者信息"));

    // 建立表格
    QStandardItemModel *model = new QStandardItemModel;
    // 设置表头
    model->setHorizontalHeaderLabels({QStringLiteral("姓名"), QStringLiteral("性别"), QStringLiteral("年龄"), QStringLiteral("身份证号")});
    // 自适应所有列，布满空间
    ui->tableView->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    // 不允许对表格直接编辑
    ui->tableView->setEditTriggers(QAbstractItemView::NoEditTriggers);

    // 对记录进行处理，按照 id 进行升序(asc)排序
    QSqlQuery sql_query;
    QString select_sql = "select * from qtusb order by id asc";
    sql_query.prepare(select_sql);

    int counts = 0;
    if(!sql_query.exec())
        qDebug() << sql_query.lastError();
    else
    {
        while(sql_query.next())
        {
            // 将查询到的每条记录进行显示
            // 1.读取相应分量
            QString name = sql_query.value(0).toString();   // 姓名
            bool sex = sql_query.value(1).toBool();         // 性别
            QString age = sql_query.value(2).toString();    // 年龄
            QString curId = sql_query.value(3).toString();  // 当前记录的身份证号

            // 2.填写到表格
            model->setItem(counts, 0, new QStandardItem(name));
            if(sex)
            {
                QString _ = QStringLiteral("女");
                model->setItem(counts, 1, new QStandardItem(_));
            }
            else
            {
                QString _ = QStringLiteral("男");
                model->setItem(counts, 1, new QStandardItem(_));
            }
            model->setItem(counts, 2, new QStandardItem(age));
            model->setItem(counts, 3, new QStandardItem(curId));

            id.append(curId);
            counts++;
        }
    }

    ui->tableView->setModel(model);  
    ui->tableView->show();
}

getallDialog::~getallDialog()
{
    delete ui;
}

void getallDialog::on_pushButton_released()
{
    // 导出到 Excel 表格
    // 设置文件保存路径
    QString fileName = QFileDialog::getSaveFileName(ui->tableView, QStringLiteral("保存"),
                                                    QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation),
                                                    QStringLiteral("Excel 文件(*.xlsx *.xls)"));
    if(fileName != "")
    {
        QAxObject *excel = new QAxObject;
        // 连接 Excel 控件
        if(excel->setControl("Excel.Application"))
        {
            /// 新建工作薄，填写标题和表头
            // 不显示窗体
            excel->dynamicCall("SetVisible (bool Visible)", "false");
            // 不显示类似于"文件已修改，是否保存"的提示.
            excel->setProperty("DisplayAlerts", false);
            // 新建一个工作簿
            QAxObject *workbooks = excel->querySubObject("WorkBooks");
            workbooks->dynamicCall("Add");
            // 获取当前工作簿
            QAxObject *workbook = excel->querySubObject("ActiveWorkBook");
            // 在 Sheet1 中输入记录
            QAxObject *worksheet = workbook->querySubObject("Worksheets(int)", 1);

            // 设置循环变量 i,j
            int i,j;
            // QTableView 列数
            int colcount = ui->tableView->model()->columnCount();
            // QTableView 行数
            int rowcount = ui->tableView->model()->rowCount();

            QAxObject *cell, *col;
            // 设置表格标题行
            cell = worksheet->querySubObject("Cells(int, int)", 1, 1);
            cell->dynamicCall("SetValue(const QString&)", QStringLiteral("全部患者信息"));
            cell->querySubObject("Font")->setProperty("Size", 18);
            // 行高 30
            worksheet->querySubObject("Range(const QString&)", "1:1")->setProperty("RowHeight", 30);
            // 合并标题行
            QString cellTitle;
            // 设置合并范围
            cellTitle.append("A1:");
            cellTitle.append(QChar(colcount - 1 + 'A'));
            cellTitle.append(QString::number(1));          
            QAxObject *range = worksheet->querySubObject("Range(const QString&)", cellTitle);
            // 自动换行
            range->setProperty("WrapText", true);
            // 合并单元格
            range->setProperty("MergeCells", true);
            // 设置位置为水平垂直居中
            range->setProperty("HorizontalAlignment", -4108);
            range->setProperty("VerticalAlignment", -4108);

            // 列标题，即"姓名、性别、年龄、身份证号"
            for(i = 0; i < colcount; i++)
            {
                QString columnName;
                columnName.append(QChar(i + 'A'));
                columnName.append(":");
                columnName.append(QChar(i + 'A'));
                col = worksheet->querySubObject("Columns(const QString&)", columnName);
                // 设置列宽
                col->setProperty("ColumnWidth", ui->tableView->columnWidth(i)/6);
                cell = worksheet->querySubObject("Cells(int, int)", 2, i+1);
                // 获取 QTableView 表头信息
                columnName = ui->tableView->model()->headerData(i, Qt::Horizontal, Qt::DisplayRole).toString();
                // 填写信息
                cell->dynamicCall("SetValue(const QString&)", columnName);
                // 设置单元格字体和颜色：加粗、灰色单元格
                cell->querySubObject("Font")->setProperty("Bold", true);              
                cell->querySubObject("Interior")->setProperty("Color", QColor(191, 191, 191));
                // 设置位置为水平垂直居中
                cell->setProperty("HorizontalAlignment", -4108);
                cell->setProperty("VerticalAlignment", -4108);
            }

            /// 获取 QTableView 数据，填入患者信息
            for(i = 0; i < rowcount; i++)
            {
                for(j = 0; j < colcount; j++)
                {
                    QModelIndex index = ui->tableView->model()->index(i, j);
                    // 转化成字符串
                    QString strdata = ui->tableView->model()->data(index).toString();
                    if(j >= colcount-2)
                        strdata.insert(0, QString("'")); // 增添字符，使之以文本形式存储数字
                    // 填写到 Excel 表格中
                    worksheet->querySubObject("Cells(int, int)", i+3, j+1)->dynamicCall("SetValue(const QString&)", strdata);
                }
            }

            /// 绘制表格框线
            // 锁定区域范围
            QString lrange;
            lrange.append("A2:");
            lrange.append(colcount - 1 + 'A');
            lrange.append(QString::number(ui->tableView->model()->rowCount() + 2));           
            range = worksheet->querySubObject("Range(const QString&)", lrange);
            // 设置框线样式
            range->querySubObject("Borders")->setProperty("LineStyle", QString::number(1));
            // 颜色为黑色
            range->querySubObject("Borders")->setProperty("Color", QColor(0, 0, 0));

            // 调整数据区行高
            // 锁定行范围
            QString rowsName;
            rowsName.append("2:");
            rowsName.append(QString::number(ui->tableView->model()->rowCount() + 2));
            range = worksheet->querySubObject("Range(const QString&)", rowsName);
            range->setProperty("RowHeight", 20);

            /// 保存文件至导出路径：fileName
            workbook->dynamicCall("SaveAs(const QString&)", QDir::toNativeSeparators(fileName));
            // 关闭工作簿
            workbook->dynamicCall("Close()");
            // 关闭 Excel
            excel->dynamicCall("Quit()");
            delete excel;           
            excel = nullptr;
            if (QMessageBox::question(nullptr, QStringLiteral("完成"), QStringLiteral("文件已导出，是否打开？"),
                                      QMessageBox::Yes|QMessageBox::No) == QMessageBox::Yes)
            {
                QDesktopServices::openUrl(QUrl("file:///" + QDir::toNativeSeparators(fileName)));
            }
        }
        else
            QMessageBox::warning(nullptr, QStringLiteral("错误"), QStringLiteral("未能创建 Excel 对象，请安装 Microsoft Excel！"), QMessageBox::Apply);
    }
}
