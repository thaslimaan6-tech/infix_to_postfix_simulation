#include "simwindow.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QMessageBox>
#include <QStack>
#include <QHeaderView>

SimWindow::SimWindow(QWidget *parent)
    : QMainWindow(parent), stepIndex(0)
{
    setWindowTitle("Infix → Postfix Simulator");
    resize(900, 600);

    QWidget *central = new QWidget(this);
    QVBoxLayout *main = new QVBoxLayout(central);

    QLabel *title = new QLabel("Infix to Postfix Conversion (Step-by-Step)");
    title->setAlignment(Qt::AlignCenter);
    title->setStyleSheet("font-size:18px; font-weight:bold;");
    main->addWidget(title);

    // ------------------- INPUT ROW ---------------------
    QHBoxLayout *row1 = new QHBoxLayout;
    row1->addWidget(new QLabel("Infix Expression:"));
    inputInfix = new QLineEdit;
    row1->addWidget(inputInfix);

    btnStart = new QPushButton("Start");
    btnClear = new QPushButton("Clear");
    row1->addWidget(btnStart);
    row1->addWidget(btnClear);

    main->addLayout(row1);

    // ------------------- POSTFIX OUTPUT ---------------------
    QHBoxLayout *row2 = new QHBoxLayout;
    row2->addWidget(new QLabel("Final Postfix:"));
    finalPostfix = new QLineEdit;
    finalPostfix->setReadOnly(true);
    row2->addWidget(finalPostfix);
    main->addLayout(row2);

    // ------------------- MIDDLE AREA: INPUT LIST + STACK ---------------------
    QHBoxLayout *mid = new QHBoxLayout;

    inputList = new QListWidget;
    inputList->setFont(QFont("Courier New", 18));
    inputList->setFixedWidth(100);
    mid->addWidget(inputList);

    stackTable = new QTableWidget;
    stackTable->setColumnCount(1);
    stackTable->setHorizontalHeaderLabels(QStringList() << "Stack (Top → Bottom)");
    stackTable->horizontalHeader()->setStretchLastSection(true);
    stackTable->verticalHeader()->setVisible(false);
    stackTable->setMinimumHeight(250);
    stackTable->setFont(QFont("Courier New", 14));
    stackTable->setStyleSheet(
    "QTableWidget::item { "
    "   border: 2px solid black; "
    "   padding: 15px; "
    "   font-size: 22px; "
    "   font-weight: bold; "
    "   background: white; "
    "}"
);


    mid->addWidget(stackTable);
    main->addLayout(mid);

    // ------------------- STATUS FIELDS ---------------------
    QHBoxLayout *info = new QHBoxLayout;

    curSymbol = new QLineEdit; curSymbol->setReadOnly(true);
    stackStringView = new QLineEdit; stackStringView->setReadOnly(true);
    outputSoFar = new QLineEdit; outputSoFar->setReadOnly(true);

    QVBoxLayout *c1 = new QVBoxLayout;
    c1->addWidget(new QLabel("Current Symbol:"));
    c1->addWidget(curSymbol);

    QVBoxLayout *c2 = new QVBoxLayout;
    c2->addWidget(new QLabel("Stack String:"));
    c2->addWidget(stackStringView);

    QVBoxLayout *c3 = new QVBoxLayout;
    c3->addWidget(new QLabel("Output So Far:"));
    c3->addWidget(outputSoFar);

    info->addLayout(c1);
    info->addLayout(c2);
    info->addLayout(c3);

    main->addLayout(info);

    // ------------------- EXPLANATION ---------------------
    explanation = new QTextEdit;
    explanation->setReadOnly(true);
    explanation->setMaximumHeight(80);
    main->addWidget(new QLabel("Explanation:"));
    main->addWidget(explanation);

    // ------------------- NEXT BUTTON ---------------------
    btnNext = new QPushButton("Next Step");
    btnNext->setEnabled(false);
    main->addWidget(btnNext);

    setCentralWidget(central);

    // CONNECTIONS
    connect(btnStart, &QPushButton::clicked, this, &SimWindow::startConversion);
    connect(btnNext, &QPushButton::clicked, this, &SimWindow::nextStep);
    connect(btnClear, &QPushButton::clicked, this, &SimWindow::clearAll);
}


// ---------------------------------------------------------------------
// Helpers
// ---------------------------------------------------------------------
bool SimWindow::isOperator(QChar c) const {
    return c=='+' || c=='-' || c=='*' || c=='/' || c=='^';
}

int SimWindow::precedence(QChar c) const {
    if(c=='+' || c=='-') return 1;
    if(c=='*' || c=='/') return 2;
    if(c=='^') return 3;
    return 0;
}

static QString stackToString(QStack<QChar> st) {
    if(st.isEmpty()) return "(empty)";
    QVector<QChar> v;
    while(!st.isEmpty()) v.prepend(st.pop());
    QString s;
    for(QChar c : v) s.append(c);
    return s;
}


// ---------------------------------------------------------------------
// Generate Steps
// ---------------------------------------------------------------------
void SimWindow::generateSteps(const QString &expr)
{
    steps.clear();
    QString exp = expr;
    exp.remove(' ');

    QString out;
    QStack<QChar> st;

    steps.append({"-", "(empty)", "(empty)", "Starting conversion..."} );

    for(QChar c : exp)
    {
        if(c.isLetterOrNumber()) {
            out += c;
            steps.append({QString(c), stackToString(st), out, "Operand → added to output"});
        }
        else if(c == '(') {
            st.push(c);
            steps.append({QString(c), stackToString(st), out, "'(' pushed"});
        }
        else if(c == ')') {
            while(!st.isEmpty() && st.top() != '(') {
                out += st.pop();
                steps.append({")", stackToString(st), out, "Popping until '('"});
            }
            st.pop();
            steps.append({")", stackToString(st), out, "Removed '('"});
        }
        else if(isOperator(c)) {
            while(!st.isEmpty() && precedence(st.top()) >= precedence(c)) {
                out += st.pop();
                steps.append({QString(c), stackToString(st), out, "Popped due to precedence"});
            }
            st.push(c);
            steps.append({QString(c), stackToString(st), out, "Operator pushed"});
        }
    }

    while(!st.isEmpty()) {
        QChar t = st.pop();
        if(t != '(') out += t;
        steps.append({"END", stackToString(st), out, QString("Popped '%1'").arg(t)});
    }

    finalPostfix->setText(out);
}


// ---------------------------------------------------------------------
// Display Step
// ---------------------------------------------------------------------
void SimWindow::displayStep(int i)
{
    const SimStep &s = steps[i];

    curSymbol->setText(s.current);
    stackStringView->setText(s.stack);
    outputSoFar->setText(s.output);
    explanation->setPlainText(s.message);

    // Highlight current input character
    if(i < inputList->count())
        inputList->setCurrentRow(i);

    // STACK DISPLAY
    stackTable->setRowCount(0);

    if(s.stack == "(empty)") {
        stackTable->insertRow(0);
        stackTable->setItem(0, 0, new QTableWidgetItem("EMPTY"));
        stackTable->setRowHeight(0, 40);
        return;
    }

    QString st = s.stack;

    for(int r = 0; r < st.size(); r++) {

        QString box = QString(st[r]);


        stackTable->insertRow(r);
        QTableWidgetItem *item = new QTableWidgetItem(box);
        item->setTextAlignment(Qt::AlignCenter);
        item->setFont(QFont("Courier New", 14));

        stackTable->setRowHeight(r, 60);

        if(r == 0) { // TOP
            item->setBackground(QColor("#C8FACC"));
        }

        stackTable->setItem(r, 0, item);
    }
}


// ---------------------------------------------------------------------
// Start Conversion
// ---------------------------------------------------------------------
void SimWindow::startConversion()
{
    QString exp = inputInfix->text().trimmed();
    if(exp.isEmpty()) {
        QMessageBox::warning(this, "Error", "Please enter infix expression");
        return;
    }

    generateSteps(exp);

    inputList->clear();
    for(QChar c : exp)
        inputList->addItem(QString(c));

    stepIndex = 0;
    displayStep(stepIndex);

    btnNext->setEnabled(true);
}


// ---------------------------------------------------------------------
void SimWindow::nextStep()
{
    if(stepIndex < steps.size() - 1) {
        stepIndex++;
        displayStep(stepIndex);
    }

    if(stepIndex >= steps.size() - 1)
        btnNext->setEnabled(false);
}


// ---------------------------------------------------------------------
void SimWindow::clearAll()
{
    inputInfix->clear();
    finalPostfix->clear();
    curSymbol->clear();
    stackStringView->clear();
    outputSoFar->clear();
    explanation->clear();

    inputList->clear();
    stackTable->setRowCount(0);

    steps.clear();
    stepIndex = 0;

    btnNext->setEnabled(false);
}
