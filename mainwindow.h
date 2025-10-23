#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QStack>
#include <QKeyEvent>

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    QString operand;
    QString opcode;
    QStack<QString>operands;
    QStack<QString>opcodes;
    bool lastOpIsBinary;

    QString calculation(bool *ok = nullptr);
    QString formatNumber(double num);

    void operandHandle(const QString &digit);    // 数字输入处理
    void binaryOpHandle(const QString &op);      // 双目运算符处理
    void unaryOpHandle(const QString &op);       // 单目运算符处理

protected:

    void keyPressEvent(QKeyEvent *event) override;

private slots:
    void btnNumClicked();
    void btnbinaryOperatorClicked();
    void btnUnaryOperatorClicked();

    void on_btnPeriod_clicked();
    void on_btnDel_clicked();
    void on_btnClear_clicked();
    void on_btnEqual_clicked();
    void on_btnSign_clicked();
    void on_btnClearall_clicked();

private:
    Ui::MainWindow *ui;
};
#endif // MAINWINDOW_H
