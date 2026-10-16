#ifndef BUILDABLE_H
#define BUILDABLE_H

#include "qwidget.h"


QT_BEGIN_NAMESPACE
class QScrollArea;
class QScrollBar;
class QLabel;
QT_END_NAMESPACE


class buildablewindow : public QWidget
{
    Q_OBJECT
private:

signals:
    void BuildableChanged();

public:
    QImage        BuildableImage;
    QImage        BuildableImageScaled;
    QLabel        *label;
    QLabel        *buildable_unitname;
    QScrollArea   *buildablescrollArea;

protected:
    void mousePressEvent(QMouseEvent *event) override;

};

#endif // BUILDABLE_H
