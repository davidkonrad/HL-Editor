/*
 * HL Editor
 *
 * dialogs.h by David Konrad
 *
 * Common dialogs, centered and (mostly) on top
 *
 */

#ifndef DIALOGS_H
#define DIALOGS_H

#include <QMessageBox>
#include <QInputDialog>
#include <QFileDialog>


//The idea is to store thw window pointer if set, and then re-use it if not set (for example called from other.h)
//QWidget *window_ref = nullptr;
extern QWidget *window_ref;

extern bool show_error(QString title, QString msg, QWidget *parent = nullptr);
extern bool show_error(QString msg, QWidget *parent = nullptr);

extern bool show_warning(QString msg, QWidget *parent = nullptr);
extern bool show_cancelable_warning(QString msg, QWidget *parent = nullptr);

extern bool ask_question(QString msg, QWidget *parent = nullptr);
extern int ask_cancelable_question(QString title, QString msg, QWidget *parent = nullptr);

extern bool show_info(QString title, QString msg, QWidget *parent = nullptr);
extern bool show_info(QString msg, QWidget *parent = nullptr);

extern QString get_item_dialog(QString title, QString msg, QStringList items, QString current, QWidget *parent = nullptr);
extern QString open_file_dialog(QString title, QString filter, QString directory, QWidget *parent = nullptr);


#endif // DIALOGS_H

