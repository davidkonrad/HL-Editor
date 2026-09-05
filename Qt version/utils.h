/*
  utils.h
  Generic functions to avoid redundancy in the original code.
*/


bool show_error(QString msg, QWidget *parent = nullptr)
{
/*
    QMessageBox dlg;
    dlg.critical(parent, "Error", msg);
    dlg.setWindowFlags(dlg.windowFlags() | Qt::WindowStaysOnTopHint);
    dlg.setFixedSize(500,200);
    return true;
*/
    QMessageBox dlg(parent);
    dlg.setWindowFlags(dlg.windowFlags() | Qt::WindowStaysOnTopHint);
    dlg.raise();
    dlg.setWindowTitle("Error");
    dlg.setIcon(QMessageBox::Critical);
    dlg.setText(msg);
    dlg.exec();
    return true;
}

bool show_warning(QString msg, QWidget *parent = nullptr)
{
    QMessageBox dlg(parent);
    dlg.setWindowFlags(dlg.windowFlags() | Qt::WindowStaysOnTopHint);
    dlg.raise();
    dlg.setWindowTitle("Warning");
    dlg.setIcon(QMessageBox::Warning);
    dlg.setText(msg);
    dlg.exec();
    return true;
}

bool ask_question(QString msg, QWidget *parent = nullptr)
{
    QMessageBox dlg(parent);
    dlg.setWindowFlags(dlg.windowFlags() | Qt::WindowStaysOnTopHint);
    dlg.raise();
    dlg.setWindowTitle("Confirm");
    dlg.setIcon(QMessageBox::Question);
    dlg.setText(msg);
    dlg.setStandardButtons(QMessageBox::Yes);
    dlg.addButton(QMessageBox::No);
    dlg.setDefaultButton(QMessageBox::No);
    if (dlg.exec() == QMessageBox::Yes)
        return true;
    else
        return false;
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

