#ifndef TILELIST_H
#define TILELIST_H

#include "qwidget.h"


QT_BEGIN_NAMESPACE
class QScrollArea;
class QScrollBar;
QT_END_NAMESPACE


class tilelistwindow : public QWidget
{
    Q_OBJECT

public:
    void resetSelection(unsigned char newsel = 0xFF);

protected:
    void mousePressEvent(QMouseEvent *event) override;
    void mouseDoubleClickEvent (QMouseEvent *event) override;

private:

};

#endif // TILELIST_H
