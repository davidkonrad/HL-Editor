/*
 * HL Editor
 *
 * unitlist.cpp by David Konrad
 *
 * unitlistwindow implementation (with original code from main,cpp, other.cpp)
 *
 */

#include <QtWidgets>

#include "Lib.h"
#include "units.h"
#include "unitlist.h"


unitlistwindow::unitlistwindow(double Scale_factor): QWidget(nullptr)
{
    sf = Scale_factor;

    setWindowFlag(Qt::SubWindow);
    setWindowFlags(Qt::WindowStaysOnTopHint | Qt::WindowTitleHint | Qt::CustomizeWindowHint);

    //resize(((12*Tilesize)*sf), (2*((Num_Units/10)+2)*Tilesize)*sf);
    setWindowTitle("Unit selection");
    setMouseTracking(true);

    //move(screenrect.width()/2, screenrect.bottom()/2);
    repaint();
    show();
}

unsigned char unitlistwindow::selected_unit()
{
    return selected;
}

void unitlistwindow::repaint()
{
    unit_name_text = new QLabel();
    unit_name_text->setText("");

    UnitListImage = QImage((10*Tilesize), 2*((Num_Units/10)+1) * Tilesize, QImage::Format_RGB16); //Create a new QImage object for the tile list
    UnitListImage.fill(Qt::transparent);

    int tx = 0;
    int ty = 0;

    for (int tc = 0; tc < Num_Units; tc++) {
        Draw_Unit(tx*Tilesize, ty*Tilesize, (tc*6)+3, 1, &UnitListImage);
        tx++;
        if (tx == 10) {
            tx = 0;
            ty++;
        }
    }
    ty++;
    tx=0;
    for (int tc = 0; tc < Num_Units; tc++) {
        Draw_Unit(tx*Tilesize, ty*Tilesize, tc*6, 2, &UnitListImage);
        tx++;
        if (tx == 10) {
            tx = 0;
            ty++;
        }
    }

    UnitListImageScaled = UnitListImage.scaled(UnitListImage.width() * sf, UnitListImage.height() * sf);

    QLabel *label = new QLabel();
    label->setPixmap(QPixmap::fromImage(UnitListImageScaled));

    unitscrollArea = new(QScrollArea);
    unitscrollArea->setBackgroundRole(QPalette::Dark);
    unitscrollArea->setWidget(label);
    unitscrollArea->setVisible(true);

    QVBoxLayout *layout = new QVBoxLayout();
    layout->addWidget(unitscrollArea);
    layout->addWidget(unit_name_text);
    setLayout(layout);

    setMaximumHeight(label->height() + (Tilesize * 2));
    setMaximumWidth(label->width() + (Tilesize * 1.2));
}


void unitlistwindow::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        if (unitscrollArea->rect().contains(event->pos())) {
            int pos_x = unitscrollArea->horizontalScrollBar()->value();
            int pos_y = unitscrollArea->verticalScrollBar()->value();
            UnitListImageScaled = UnitListImage.scaled(UnitListImage.width() * sf, UnitListImage.height() * sf); //Restore original image
            QRect widgetRect = unitscrollArea->geometry();
            int fx = (event->pos().x()-widgetRect.left()+pos_x) / (Tilesize * sf); // Calc field position from mouse cords
            int fy = (event->pos().y()-widgetRect.top()+pos_y)/ (Tilesize * sf);
            int os = selected;

            selected = ((fy * 10) + fx ) * 2;

            if (fy > 5)
              selected = selected - 119;     //Calc correct unit number for french units

            if (selected < Num_Units * 2) {
              unit_name_text->setText(Unit_Name[selected / 2]);
              Draw_Hexagon(fx, fy, QPen(Qt::red, 1), &UnitListImageScaled, false, true, sf);
            }  else {
              selected = os;
              return;
            }

            QLabel *label = new QLabel();
            label->setPixmap(QPixmap::fromImage(UnitListImageScaled));  //update the image
            unitscrollArea_current_label = label;

            unitscrollArea->setWidget(label);
            unitscrollArea->horizontalScrollBar()->setValue(pos_x); //Reset the scrollArea to last position
            unitscrollArea->verticalScrollBar()->setValue(pos_y);

            //unit_selection->update();                           //Update the window contents
        }
    }

    if (event->button() == Qt::RightButton) {
        if (unitscrollArea->rect().contains(event->pos())) {
            resetSelection();
        }
    }
}

void unitlistwindow::resetSelection()
{
    UnitListImageScaled = UnitListImage.scaled(UnitListImage.width() * sf, UnitListImage.height() * sf); //Restore original image
    selected = 0xFF;     //No unit selected

    if (unitscrollArea_current_label)
        unitscrollArea_current_label->setPixmap(QPixmap::fromImage(UnitListImageScaled));  //update the image

    unit_name_text->setText("");
}


void unitlistwindow::mouseDoubleClickEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        if (height() >= maximumHeight()) {
            resize(width(), maximumHeight() / 2); // half the height
        } else {
            resize(maximumWidth(), maximumHeight()); //set maximum size
        }
    }
}

