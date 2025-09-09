/*
  utils.h
  Generic functions to avoid redundancy in the original code.
*/


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

/*
  see main.cpp
  basically to make it work "native" in both linux and win
  use the QT_TARGET flag to determine how paths are constructed
*/
QString get_path(QString path)
{
    QString full_path = (GameDir + path);
    switch (QT_TARGET) {
       case 0:
       default:
          break;
       case 1:
          full_path.replace("/", "\\");
          break;
    }
    return full_path;
}
