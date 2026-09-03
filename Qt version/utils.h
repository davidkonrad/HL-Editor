/*
  utils.h
  Generic functions to avoid redundancy in the original code.
*/

#include <QStyle>
#include <QDesktopWidget>


bool show_error(QString msg, QWidget *parent = nullptr)
{
    QMessageBox dlg;
    dlg.critical(parent, "Error", msg);
    dlg.setWindowFlags(dlg.windowFlags() | Qt::WindowStaysOnTopHint);
    dlg.setFixedSize(500,200);
    return true;
}

bool show_warning(QString msg, QWidget *parent = nullptr)
{
    QMessageBox dlg;
    dlg.warning(parent, "Warning", msg);
    dlg.setWindowFlags(dlg.windowFlags() | Qt::WindowStaysOnTopHint);
    dlg.setFixedSize(500,200);
    dlg.raise();
    return true;
}

bool ask_question(QString msg, QWidget *parent = nullptr)
{
    QMessageBox dlg;
    QMessageBox::StandardButton answer;
    dlg.setWindowFlags(dlg.windowFlags() | Qt::WindowStaysOnTopHint);
    answer = dlg.question(parent, "Confirm", msg, QMessageBox::Yes | QMessageBox::No);
    return (answer == QMessageBox::Yes) ? true : false;
}

bool show_info(QString msg, QWidget *parent = nullptr)
{
    QMessageBox dlg;
    dlg.setWindowFlags(dlg.windowFlags() | Qt::WindowStaysOnTopHint);
    dlg.information(parent, "Notice", msg);
    return true;
}

/*
 ...
*/
QString get_path(QString path)
{
    if (path.startsWith(GameDir) == false)
        path = GameDir + path;
    return path;
}

