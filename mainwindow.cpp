#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QDebug>
#include <math.h>
#include <QStringList>
#include <QKeyEvent>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , lastOpIsBinary(false)
{
    ui->setupUi(this);
    setWindowTitle("李子诺2023414300208");

    // 初始化显示框为只读
    ui->display->setReadOnly(true);
    ui->display->setText("0");

    // 数字按钮连接
    connect(ui->btnNum0, &QPushButton::clicked, this, &MainWindow::btnNumClicked);
    connect(ui->btnNum1, &QPushButton::clicked, this, &MainWindow::btnNumClicked);
    connect(ui->btnNum2, &QPushButton::clicked, this, &MainWindow::btnNumClicked);
    connect(ui->btnNum3, &QPushButton::clicked, this, &MainWindow::btnNumClicked);
    connect(ui->btnNum4, &QPushButton::clicked, this, &MainWindow::btnNumClicked);
    connect(ui->btnNum5, &QPushButton::clicked, this, &MainWindow::btnNumClicked);
    connect(ui->btnNum6, &QPushButton::clicked, this, &MainWindow::btnNumClicked);
    connect(ui->btnNum7, &QPushButton::clicked, this, &MainWindow::btnNumClicked);
    connect(ui->btnNum8, &QPushButton::clicked, this, &MainWindow::btnNumClicked);
    connect(ui->btnNum9, &QPushButton::clicked, this, &MainWindow::btnNumClicked);

    // 双目运算符连接
    connect(ui->btnPlus, &QPushButton::clicked, this, &MainWindow::btnbinaryOperatorClicked);
    connect(ui->btnMinus, &QPushButton::clicked, this, &MainWindow::btnbinaryOperatorClicked);
    connect(ui->btnMultiple, &QPushButton::clicked, this, &MainWindow::btnbinaryOperatorClicked);
    connect(ui->btnDivide, &QPushButton::clicked, this, &MainWindow::btnbinaryOperatorClicked);

    // 其他按钮连接
    connect(ui->btnPercentage, &QPushButton::clicked, this, &MainWindow::btnUnaryOperatorClicked);
    connect(ui->btnInverse, &QPushButton::clicked, this, &MainWindow::btnUnaryOperatorClicked);
    connect(ui->btnSquare, &QPushButton::clicked, this, &MainWindow::btnUnaryOperatorClicked);
    connect(ui->btnSqrt, &QPushButton::clicked, this, &MainWindow::btnUnaryOperatorClicked);
    connect(ui->btnSign, &QPushButton::clicked, this, &MainWindow::on_btnSign_clicked);
    connect(ui->btnClearall, &QPushButton::clicked, this, &MainWindow::on_btnClearall_clicked);
}

MainWindow::~MainWindow()
{
    delete ui;
}

// 计算核心函数：处理一次运算（取第一个操作符和前两个操作数）
QString MainWindow::calculation(bool *ok)
{
    double result = 0;
    bool calculationSuccess = false;

    // 确保有足够的操作数和运算符
    if (operands.size() >= 2 && !opcodes.isEmpty()) {
        // 取操作数（使用takeFirst()移除并获取第一个元素，符合顺序计算）
        double operand1 = operands.takeFirst().toDouble();
        double operand2 = operands.takeFirst().toDouble();
        QString op = opcodes.takeFirst(); // 取第一个运算符

        // 执行运算（匹配按钮实际文本：+、-、×、÷）
        if (op == "+") {
            result = operand1 + operand2;
            calculationSuccess = true;
        } else if (op == "-") {
            result = operand1 - operand2;
            calculationSuccess = true;
        } else if (op == "×") { // 匹配乘法按钮文本
            result = operand1 * operand2;
            calculationSuccess = true;
        } else if (op == "÷") { // 匹配除法按钮文本
            if (qFuzzyCompare(operand2, 0.0)) { // 处理除数为0
                ui->statusbar->showMessage("错误：除数不能为零");
                return "错误";
            }
            result = operand1 / operand2;
            calculationSuccess = true;
        }

        if (calculationSuccess) {
            // 将结果作为新的操作数加入列表（作为下一次运算的第一个操作数）
            operands.push_front(formatNumber(result));
        }
    }

    // 返回当前结果（操作数列表的第一个元素）
    return operands.isEmpty() ? "0" : operands.first();
}

// 格式化数字显示（去除多余小数位）
QString MainWindow::formatNumber(double num)
{
    if (num == floor(num)) { // 整数显示为整数（如123.0→123）
        return QString::number((long long)num);
    }
    return QString::number(num, 'g', 6); // 小数保留最多6位有效数字
}

// 数字按钮点击事件
void MainWindow::btnNumClicked()
{
    QString digit = qobject_cast<QPushButton*>(sender())->text();

    // 如果刚点击过运算符，清空当前输入（准备输入新数字）
    if (lastOpIsBinary) {
        operand.clear();
        lastOpIsBinary = false;
    }

    // 处理前导零（如输入“000”只显示“0”）
    if (operand == "0" && digit == "0") {
        return;
    }

    // 第一个数字为0且输入非0数字时，替换掉0（如“0”→输入“1”变为“1”）
    if (operand == "0" && digit != "0") {
        operand.clear();
    }

    operand += digit;
    ui->display->setText(operand);
}

// 小数点按钮点击事件
void MainWindow::on_btnPeriod_clicked()
{
    if (lastOpIsBinary) { // 刚点击过运算符，默认先输入“0.”
        operand = "0";
        lastOpIsBinary = false;
    }

    if (!operand.contains(".")) { // 确保只添加一个小数点
        if (operand.isEmpty()) { // 空输入时，先输入“0.”
            operand = "0.";
        } else {
            operand += ".";
        }
        ui->display->setText(operand);
    }
}

// 删除按钮点击事件（删除最后一位）
void MainWindow::on_btnDel_clicked()
{
    if (!operand.isEmpty()) {
        operand.chop(1); // 移除最后一个字符
        if (operand.isEmpty()) { // 删除后为空，显示“0”
            operand = "0";
        }
        ui->display->setText(operand);
    }
}

// 清除按钮点击事件（清空所有数据）
void MainWindow::on_btnClear_clicked()
{
    operand.clear();
    operands.clear();
    opcodes.clear();
    ui->display->setText("0");
    ui->statusbar->clearMessage();
}

// 双目运算符点击事件（+、-、×、÷）
void MainWindow::btnbinaryOperatorClicked()
{
    QString opcode = qobject_cast<QPushButton*>(sender())->text();
    qDebug() << "当前运算符: " << opcode;

    // 处理错误状态：如果上一次结果是错误，点击运算符重置
    if (operand == "错误") {
        operand.clear();
        operands.clear();
        opcodes.clear();
    }

    // 将当前输入的数字加入操作数列表
    if (!operand.isEmpty()) {
        operands.push_back(operand);
        operand.clear();
    }

    // 如果已有足够的操作数和运算符，先计算一次（支持连续运算，如1+2+3）
    if (operands.size() >= 2 && !opcodes.isEmpty()) {
        QString result = calculation();
        ui->display->setText(result);
    }

    // 加入新的运算符（确保操作数列表至少有一个数才能加运算符）
    if (operands.size() >= 1) {
        opcodes.push_back(opcode);
        lastOpIsBinary = true; // 标记刚点击过运算符
    }
}

// 单目运算符点击事件（%、1/x、x²、√）
void MainWindow::btnUnaryOperatorClicked()
{
    // 如果没有当前输入，使用上一次的结果作为操作数
    if (operand.isEmpty() && !operands.isEmpty()) {
        operand = operands.last();
    }

    if (!operand.isEmpty() && operand != "错误") {
        double result = operand.toDouble();
        QString op = qobject_cast<QPushButton*>(sender())->text();

        // 执行单目运算
        if (op == "%") {
            result /= 100.0; // 百分比：x → x/100
        } else if (op == "1/x") { // 倒数
            if (qFuzzyCompare(result, 0.0)) {
                ui->display->setText("错误");
                operand = "错误";
                return;
            }
            result = 1 / result;
        } else if (op == "x^2") { // 平方
            result *= result;
        } else if (op == "√") { // 平方根
            if (result < 0) {
                ui->display->setText("错误");
                operand = "错误";
                return;
            }
            result = sqrt(result);
        }

        // 显示并更新结果
        QString formattedResult = formatNumber(result);
        ui->display->setText(formattedResult);
        operand = formattedResult;

        // 更新操作数列表（替换最后一个操作数）
        if (!operands.isEmpty()) {
            operands.pop_back();
        }
        operands.push_back(operand);
    }
}

// 等于按钮点击事件（计算所有剩余运算）
void MainWindow::on_btnEqual_clicked()
{
    // 处理错误状态
    if (operand == "错误") {
        operand.clear();
        operands.clear();
        opcodes.clear();
        ui->display->setText("0");
        return;
    }

    // 将最后输入的数字加入操作数列表
    if (!operand.isEmpty()) {
        operands.push_back(operand);
        operand.clear();
    }

    // 循环计算所有剩余的运算符（支持连续运算，如1+2×3-4）
    while (operands.size() >= 2 && !opcodes.isEmpty()) {
        QString result = calculation();
        // 如果计算错误，直接退出循环
        if (result == "错误") {
            break;
        }
        ui->display->setText(result);
    }

    // 将结果作为下一次计算的初始值
    if (!operands.isEmpty()) {
        operand = operands.first();
    } else {
        operand = "0";
    }

    lastOpIsBinary = false;
}

// CE按钮（清除当前输入）
void MainWindow::on_btnClearall_clicked()
{
    operand.clear();
    ui->display->setText("0");
    ui->statusbar->showMessage("已清除当前输入");
}

void MainWindow::on_btnSign_clicked()
{
    if (!operand.isEmpty() && operand != "0" && operand != "错误") {
        if (operand.startsWith('-')) {
            operand.remove(0, 1); // 移除负号
        } else {
            operand.prepend('-'); // 添加负号
        }
        ui->display->setText(operand);
        return; // 处理完输入状态，直接返回
    }

    // 场景2：已计算出结果（operand为空，结果存在display中，如"456"）
    QString displayText = ui->display->text();
    if (!displayText.isEmpty() && displayText != "0" && displayText != "错误") {
        if (displayText.startsWith('-')) {
            displayText.remove(0, 1);
        } else {
            displayText.prepend('-');
        }
        ui->display->setText(displayText);
        operand = displayText; // 同步到operand，确保后续运算正确
        return;
    }

    ui->statusbar->showMessage("无法切换正负号（当前为0/错误/空输入）");
}

void MainWindow::keyPressEvent(QKeyEvent *event)
{
    // 小键盘按键对应的整数常量（Qt内部定义，兼容所有版本）
    const int NumPad0 = 0x1030;    // 小键盘0
    const int NumPad1 = 0x1031;    // 小键盘1
    const int NumPad2 = 0x1032;    // 小键盘2
    const int NumPad3 = 0x1033;    // 小键盘3
    const int NumPad4 = 0x1034;    // 小键盘4
    const int NumPad5 = 0x1035;    // 小键盘5
    const int NumPad6 = 0x1036;    // 小键盘6
    const int NumPad7 = 0x1037;    // 小键盘7
    const int NumPad8 = 0x1038;    // 小键盘8
    const int NumPad9 = 0x1039;    // 小键盘9
    const int NumPadPlus = 0x106B;  // 小键盘+
    const int NumPadMinus = 0x106D; // 小键盘-
    const int NumPadMultiply = 0x106A; // 小键盘*
    const int NumPadDivide = 0x106F;   // 小键盘/
    const int NumPadDecimal = 0x106E;  // 小键盘.

    // 仅处理小键盘及必要功能键
    switch (event->key()) {
    // 1. 小键盘数字键（0-9）
    case NumPad0:
    case NumPad1:
    case NumPad2:
    case NumPad3:
    case NumPad4:
    case NumPad5:
    case NumPad6:
    case NumPad7:
    case NumPad8:
    case NumPad9: {
        QString digit = event->text(); // 获取数字文本（如"1""2"）
        operandHandle(digit);
        return;
    }

    // 2. 小键盘运算符
    case NumPadPlus:
        binaryOpHandle("+");
        return;
    case NumPadMinus:
        binaryOpHandle("-");
        return;
    case NumPadMultiply:
        binaryOpHandle("×");
        return;
    case NumPadDivide:
        binaryOpHandle("÷");
        return;

    // 3. 小键盘小数点
    case NumPadDecimal:
        on_btnPeriod_clicked();
        return;

    // 4. 回车键（等于）
    case Qt::Key_Enter:
    case Qt::Key_Return:
        on_btnEqual_clicked();
        return;

    // 5. 辅助功能键
    case Qt::Key_Backspace: // 退格（删除）
        on_btnDel_clicked();
        return;
    case Qt::Key_Escape:    // ESC（清空）
        on_btnClear_clicked();
        return;

    // 其他键忽略
    default:
        break;
    }
}

void MainWindow::operandHandle(const QString &digit)
{
    // 调试：确认函数被调用且digit正确
    qDebug() << "进入operandHandle，数字：" << digit;

    if (lastOpIsBinary) {
        operand.clear();
        lastOpIsBinary = false;
        qDebug() << "刚按过运算符，清空operand";
    }

    // 处理前导零（简化逻辑，确保所有情况都能正确拼接）
    if (operand == "0") {
        if (digit == "0") {
            // 输入0时，保持显示"0"
            ui->display->setText("0");
            qDebug() << "前导零处理：保持0";
            return;
        } else {
            // 非0数字替换0
            operand = digit;
        }
    } else {
        // 正常拼接数字
        operand += digit;
    }

    // 强制更新显示框
    ui->display->setText(operand);
    qDebug() << "更新显示：" << operand;
}

// 新增：双目运算符统一处理（复用原运算逻辑）
void MainWindow::binaryOpHandle(const QString &op)
{
    if (operand == "错误") {
        operand.clear();
        operands.clear();
        opcodes.clear();
    }

    if (!operand.isEmpty()) {
        operands.push_back(operand);
        operand.clear();
    }

    if (operands.size() >= 2 && !opcodes.isEmpty()) {
        QString result = calculation();
        ui->display->setText(result);
    }

    if (operands.size() >= 1) {
        opcodes.push_back(op);
        lastOpIsBinary = true;
    }
}

// 新增：单目运算符统一处理（复用原运算逻辑）
void MainWindow::unaryOpHandle(const QString &op)
{
    if (operand.isEmpty() && !operands.isEmpty()) {
        operand = operands.last();
    }

    if (!operand.isEmpty() && operand != "错误") {
        double result = operand.toDouble();

        if (op == "%") {
            result /= 100.0;
        } else if (op == "1/x") {
            if (qFuzzyCompare(result, 0.0)) {
                ui->display->setText("错误");
                operand = "错误";
                return;
            }
            result = 1 / result;
        } else if (op == "x^2") {
            result *= result;
        } else if (op == "√") {
            if (result < 0) {
                ui->display->setText("错误");
                operand = "错误";
                return;
            }
            result = sqrt(result);
        }

        QString formattedResult = formatNumber(result);
        ui->display->setText(formattedResult);
        operand = formattedResult;

        if (!operands.isEmpty()) {
            operands.pop_back();
        }
        operands.push_back(operand);
    }
    qDebug() << "键盘输入单目运算符：" << op;
}
