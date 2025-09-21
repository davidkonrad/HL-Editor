/*
  utils.h
  Generic functions to avoid redundancy in the original code.
*/

void debug(QString s1)
{
    qDebug() << s1;
}
void debug(QString s1, QString s2)
{
    qDebug() << s1 << s2;
}

//NOTE: QMessageBoxes should be cleaned up by themselves (?)
bool show_error(QString msg)
{
    QMessageBox dlg;
    dlg.critical(0, "Error", msg);
    dlg.setFixedSize(500,200);
    return true;
}

bool show_warning(QString msg)
{
    QMessageBox dlg;
    dlg.warning(0, "Warning", msg);
    dlg.setFixedSize(500,200);
    return true;
}

bool ask_question(QString msg)
{
    QMessageBox dlg;
    QMessageBox::StandardButton answer;
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
    //QString full_path = (GameDir + path);
    //prefix path with GameDir if it is not alread added
    //QString full_path = path.startsWith(GameDir) == ? path : GameDir + path;
    qDebug() << "get_path:" << path;

    if (path.startsWith(GameDir) == false) {
        //show_error("get_path: " + path);
        qDebug() << "get_path: GameDir NOT included";
        path = GameDir + path;
    } else {
        //show_error("get_path WITH GAMEDIR "+ path);
        qDebug() << "get_path: GameDir IS included";
    }

    switch (QT_TARGET) {
       case 0:
       default:
          break;
       case 1:
          //full_path.replace("/", "\\");
          path.replace("/", "\\");
          break;
    }
    return path;
}
