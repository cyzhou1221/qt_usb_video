#ifndef INPUTDIALOG_H
#define INPUTDIALOG_H

#include <QDialog>

namespace Ui {
class inputDialog;
}

class inputDialog : public QDialog
{
    Q_OBJECT

public:
    explicit inputDialog(QWidget *parent = nullptr);
    ~inputDialog();

private slots:
    // "确认"按钮
    void on_pushButton_clicked();
    // "取消"按钮
    void on_pushButton_2_clicked();

private:
    Ui::inputDialog *ui;
};

#endif // INPUTDIALOG_H
