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

    explicit buildablewindow(double Scale_factor);

protected:
    double sf;
    void mousePressEvent(QMouseEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;

    void update_buildable();

};

#endif // BUILDABLE_H
