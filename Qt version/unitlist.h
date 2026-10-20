#ifndef UNITLIST_H
#define UNITLIST_H

#include "qwidget.h"


QT_BEGIN_NAMESPACE
class QScrollArea;
class QScrollBar;
class QLabel;
QT_END_NAMESPACE


class unitlistwindow : public QWidget
{
    Q_OBJECT

public:
    QImage        UnitListImage;
    QImage        UnitListImageScaled;
    QScrollArea   *unitscrollArea;
    QLabel        *unit_name_text;    // Text label to display unit name on mouse over

    void resetSelection();

signals:
    void SelectionChanged();

protected:
    void mousePressEvent(QMouseEvent *event) override;
    void mouseDoubleClickEvent (QMouseEvent *event) override;

private:

};

#endif // UNITLIST_H

