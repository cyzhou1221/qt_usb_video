#ifndef MODIFYDIALOG_H
#define MODIFYDIALOG_H

#include <QDialog>
#include "updatedialog.h"

namespace Ui {
class modifyDialog;
}

class modifyDialog : public QDialog
{
    Q_OBJECT

public:
    explicit modifyDialog(QWidget *parent = nullptr);
    ~modifyDialog();

private slots:
    // "查询"按钮
    void on_pushButton_clicked();
    // 用于探知选中单元格所在记录的身份证号
    void on_tableView_clicked();
    // "删除"按钮
    void on_pushButton_2_clicked();
    // "更新"按钮
    void on_pushButton_3_clicked();

private:
    Ui::modifyDialog *ui;
    updateDialog *upddialog;
};

#endif // MODIFYDIALOG_H
