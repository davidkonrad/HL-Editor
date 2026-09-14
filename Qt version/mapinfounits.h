#ifndef MAPINFOUNITS_H
#define MAPINFOUNITS_H

#include "qwidget.h"

QT_BEGIN_NAMESPACE
class QScrollArea;
class QScrollBar;
QT_END_NAMESPACE


class mapinfounitswindow : public QWidget
{
    Q_OBJECT

public:

protected:
    void keyPressEvent(QKeyEvent *event) override;

private:

};


#endif // MAPINFOUNITS_H
