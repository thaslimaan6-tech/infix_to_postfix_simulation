#include <QApplication>
#include "simwindow.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    SimWindow w;
    w.show();

    return app.exec();
}
