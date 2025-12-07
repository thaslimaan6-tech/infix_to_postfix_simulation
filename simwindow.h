#ifndef SIMWINDOW_H
#define SIMWINDOW_H

#include <QMainWindow>
#include <QLineEdit>
#include <QTableWidget>
#include <QTextEdit>
#include <QPushButton>
#include <QListWidget>
#include <QVector>

struct SimStep {
    QString current;
    QString stack;
    QString output;
    QString message;
};

class SimWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit SimWindow(QWidget *parent = nullptr);

private slots:
    void startConversion();
    void nextStep();
    void clearAll();

private:
    QLineEdit *inputInfix;
    QLineEdit *finalPostfix;

    QLineEdit *curSymbol;
    QLineEdit *stackStringView;
    QLineEdit *outputSoFar;

    QTableWidget *stackTable;
    QListWidget *inputList;
    QTextEdit *explanation;

    QPushButton *btnStart;
    QPushButton *btnNext;
    QPushButton *btnClear;

    QVector<SimStep> steps;
    int stepIndex;

    void generateSteps(const QString &exp);
    void displayStep(int i);

    bool isOperator(QChar c) const;
    int precedence(QChar c) const;
};

#endif