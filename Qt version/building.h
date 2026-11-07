#ifndef BUILDING_H
#define BUILDING_H

#include "qwidget.h"
#include <QLineEdit>
#include <QLabel>

#include "unitlist.h"

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

    explicit buildingwindow(int index, double Scale_factor, unitlistwindow *uw);

protected:
    unsigned char old_units[7];
    int      old_resources;

    double   sf;
    bool     Update_Ressources;
    unitlistwindow *unit_window;

    void mousePressEvent (QMouseEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;

    void update_tiles();
    void cancel_dialog();
};


#endif // BUILDING_H
