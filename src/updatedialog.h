#ifndef UPDATEDIALOG_H
#define UPDATEDIALOG_H

#include <QDialog>

namespace Ui {
class updateDialog;
}

class updateDialog : public QDialog
{
    Q_OBJECT

public:
    explicit updateDialog(QWidget *parent = nullptr);
    ~updateDialog();

private slots:
    // "更新"按钮
    void on_pushButton_clicked();
    // "返回"按钮
    void on_pushButton_2_clicked();

private:
    Ui::updateDialog *ui;
};

#endif // UPDATEDIALOG_H
