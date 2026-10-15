#ifndef BUILDING_H
#define BUILDING_H

#include "qwidget.h"
#include <QLineEdit>
#include <QLabel>

QT_BEGIN_NAMESPACE
class QScrollArea;
class QScrollBar;
QT_END_NAMESPACE


class buildingwindow : public QWidget
{
    Q_OBJECT

private:

signals:
    void BuildingChanged();

public:
    QImage        Building_Image; //variable names as they were in main.cpp
    QImage        Building_Image_Scaled;
    QLabel        *bitmaplabel;
    QScrollArea   *Building_ScrollArea;
    QWidget       *building_window;
    QLineEdit     *RessourceEdit;
    int           selected_building = -1;  // No building is selected by default

protected:
    void mousePressEvent (QMouseEvent *event) override;
};


#endif // BUILDING_H
