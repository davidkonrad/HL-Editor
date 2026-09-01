/*
  utils.h
  Generic functions to avoid redundancy in the original code.
*/

#include <QStyle>
#include <QDesktopWidget>

//QScreen *screen; //
QRect screenGeometry; // = screen->availableGeometry();

void debug(QString s1)
{
    qDebug() << s1;
}
void debug(QString s1, QString s2)
{
    qDebug() << s1 << s2;
}

void set_screenGeometry(QRect sg)
{
    screenGeometry = sg;
}

//NOTE: QMessageBoxes should be cleaned up by themselves (?)
bool show_error(QString msg)
{
    QMessageBox dlg;
    dlg.critical(0, "Error", msg);
    dlg.setWindowFlags(dlg.windowFlags() | Qt::WindowStaysOnTopHint);
    dlg.setFixedSize(500,200);
    return true;
}

bool show_warning(QString msg)
{
    QMessageBox dlg;
    dlg.warning(0, "Warning", msg);
    dlg.setWindowFlags(dlg.windowFlags() | Qt::WindowStaysOnTopHint);
    dlg.setFixedSize(500,200);
    dlg.raise();
    return true;
}

bool show_info(QString msg)
{
    qDebug() << screenGeometry;
    qDebug() << screenGeometry.center();

    QMessageBox dlg;
    //dlg.information(0, "Notice", msg);
    dlg.setWindowFlags(dlg.windowFlags() | Qt::WindowStaysOnTopHint);
    //dlg.setFixedSize(500,200);

    dlg.setIcon(QMessageBox::Information);
    dlg.setWindowTitle("Notice");
    dlg.setText(msg);

    //dlg.raise();
    dlg.setGeometry(screenGeometry.center().y(), screenGeometry.center().x(), 500, 200);
    dlg.setInformativeText("qwerty");

    dlg.exec();
    //dlg.information(None, "Notice", msg);

    //dlg.exec();

    //QScreen *screen = QGuiApplication::primaryScreen();
    //QRect screenGeometry = screen->availableGeometry();

    return true;
}

bool ask_question(QString msg)
{
    QMessageBox dlg;
    QMessageBox::StandardButton answer;
    dlg.setWindowFlags(dlg.windowFlags() | Qt::WindowStaysOnTopHint);
    dlg.setFixedSize(500,200); //??
    answer = dlg.question(0, "Confirm", msg, QMessageBox::Yes|QMessageBox::No);
    return (answer == QMessageBox::Yes) ? true : false;
}

/*
  see main.cpp
  basically to make it work "native" in both linux and win
  use the QT_TARGET flag to determine how paths are constructed
*/
QString get_path(QString path)
{
    //qDebug() << "get_path:" << path;
    if (path.startsWith(GameDir) == false) {
        //qDebug() << "get_path: GameDir NOT included";
        path = GameDir + path;
    } else {
    }
/*
    switch (QT_TARGET) {
       case 0:
       default:
          break;
       case 1:
          //full_path.replace("/", "\\");
          path.replace("/", "\\");
          break;
    }
*/
    return path;
}

