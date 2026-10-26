/*
 * HL Editor
 *
 * nativemap.h by David Konrad
 *
 * header for the nativemap dialog
 *
 */

#ifndef NATIVEMAP_H
#define NATIVEMAP_H

#include "qwidget.h"


QT_BEGIN_NAMESPACE
class QScrollArea;
class QScrollBar;
class QLabel;
class QComboBox;
QT_END_NAMESPACE


class nativemapwindow : public QWidget
{
    Q_OBJECT

public:
    explicit nativemapwindow();

signals:

private slots:

protected:

private:

};

#endif // NATIVEMAP_H


