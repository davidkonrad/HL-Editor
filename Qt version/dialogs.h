/*
 * HL Editor
 *
 * dialogs.h by David Konrad
 *
 * Common dialogs, centered and (mostly) on top
 *
 */


//The idea is to store thw window pointer if set, and then re-use it if not set (for example called from other.h)
QWidget *window_ref = nullptr;


bool show_error(QString title, QString msg, QWidget *parent = nullptr)
{
    if (parent != nullptr && window_ref == nullptr) window_ref = parent;
    if (parent == nullptr && window_ref != nullptr) parent = window_ref;

    QMessageBox dlg(parent);
    dlg.setWindowFlags(dlg.windowFlags() | Qt::WindowStaysOnTopHint);
    dlg.raise();
    dlg.setWindowTitle(title);
    dlg.setIcon(QMessageBox::Critical);
    dlg.setText(msg);
    dlg.exec();
    return true;
}

bool show_error(QString msg, QWidget *parent = nullptr)
{
    show_error("Error", msg, parent);
    return true;
}

bool show_warning(QString msg, QWidget *parent = nullptr)
{
    if (parent != nullptr && window_ref == nullptr) window_ref = parent;
    if (parent == nullptr && window_ref != nullptr) parent = window_ref;

    QMessageBox dlg(parent);
    dlg.setWindowFlags(dlg.windowFlags() | Qt::WindowStaysOnTopHint);
    dlg.raise();
    dlg.setWindowTitle("Warning");
    dlg.setIcon(QMessageBox::Warning);
    dlg.setText(msg);
    dlg.exec();
    return true;
}

//I think about the pssibility of cancel actions, like the warnings when units are placed on 'illegal' positions
bool show_cancelable_warning(QString msg, QWidget *parent = nullptr)
{
    if (parent != nullptr && window_ref == nullptr) window_ref = parent;
    if (parent == nullptr && window_ref != nullptr) parent = window_ref;

    QMessageBox dlg(parent);
    dlg.setWindowFlags(dlg.windowFlags() | Qt::WindowStaysOnTopHint);
    dlg.raise();
    dlg.setWindowTitle("Warning");
    dlg.setStandardButtons(QMessageBox::Cancel | QMessageBox::Ok);
    dlg.setIcon(QMessageBox::Warning);
    dlg.setText(msg);
    return dlg.exec() == QMessageBox::Ok ? true : false;
}

bool ask_question(QString msg, QWidget *parent = nullptr)
{
    if (parent != nullptr && window_ref == nullptr) window_ref = parent;
    if (parent == nullptr && window_ref != nullptr) parent = window_ref;

    QMessageBox dlg(parent);
    dlg.setWindowFlags(dlg.windowFlags() | Qt::WindowStaysOnTopHint);
    dlg.raise();
    dlg.setWindowTitle("Confirm");
    dlg.setIcon(QMessageBox::Question);
    dlg.setText(msg);
    dlg.setStandardButtons(QMessageBox::Yes);
    dlg.addButton(QMessageBox::No);
    dlg.setDefaultButton(QMessageBox::No);
    return dlg.exec() == QMessageBox::Yes ? true : false;
}

int ask_cancelable_question(QString title, QString msg, QWidget *parent = nullptr)
{
    if (parent != nullptr && window_ref == nullptr) window_ref = parent;
    if (parent == nullptr && window_ref != nullptr) parent = window_ref;

    QMessageBox dlg(parent);
    dlg.setWindowFlags(dlg.windowFlags() | Qt::WindowStaysOnTopHint);
    dlg.raise();
    dlg.setWindowTitle(title);
    dlg.setIcon(QMessageBox::Question);
    dlg.setText(msg);
    dlg.setStandardButtons(QMessageBox::Cancel | QMessageBox::No | QMessageBox::Yes);
    return dlg.exec();
}

bool show_info(QString title, QString msg, QWidget *parent = nullptr)
{
    if (parent != nullptr && window_ref == nullptr) window_ref = parent;
    if (parent == nullptr && window_ref != nullptr) parent = window_ref;

    QMessageBox dlg(parent);
    dlg.setWindowFlags(dlg.windowFlags() | Qt::WindowStaysOnTopHint);
    dlg.raise();
    dlg.setWindowTitle(title);
    dlg.setIcon(QMessageBox::Information);
    dlg.setText(msg);
    dlg.exec();
    return true;
}

bool show_info(QString msg, QWidget *parent = nullptr)
{
    show_info("Notice", msg, parent);
    return true;
}

QString get_item_dialog(QString title, QString msg, QStringList items, QString current, QWidget *parent = nullptr)
{
    if (parent != nullptr && window_ref == nullptr) window_ref = parent;
    if (parent == nullptr && window_ref != nullptr) parent = window_ref;

    QInputDialog dlg(parent);
    dlg.setWindowFlags(dlg.windowFlags() | Qt::WindowStaysOnTopHint);
    dlg.setWindowTitle(title);
    dlg.setLabelText(msg);
    dlg.setComboBoxItems(items);
    if (!current.isEmpty()) dlg.setTextValue(current);
    bool ok = dlg.exec();
    return ok ? dlg.textValue() : "";
}

QString open_file_dialog(QString title, QString filter, QString directory, QWidget *parent = nullptr)
{
    if (parent != nullptr && window_ref == nullptr) window_ref = parent;
    if (parent == nullptr && window_ref != nullptr) parent = window_ref;

    QFileDialog dlg(parent);
    dlg.setWindowFlags(dlg.windowFlags() | Qt::WindowStaysOnTopHint);
    dlg.setWindowTitle(title);
    dlg.setNameFilter(filter);
    dlg.setDirectory(directory);
    dlg.setWindowModality(Qt::ApplicationModal);
    dlg.raise();
    bool ok = dlg.exec();
    return ok ? dlg.selectedFiles().first() : "";
}


