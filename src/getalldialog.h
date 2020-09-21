#ifndef GETALLDIALOG_H
#define GETALLDIALOG_H

#include <QDialog>

namespace Ui {
class getallDialog;
}

class getallDialog : public QDialog
{
    Q_OBJECT

public:
    explicit getallDialog(QWidget *parent = nullptr);
    ~getallDialog();

private slots:
    // "导出到表格"按钮
    void on_pushButton_released();

private:
    Ui::getallDialog *ui;
};

#endif // GETALLDIALOG_H
