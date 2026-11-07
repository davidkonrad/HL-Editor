/*
 * HL Editor
 *
 * builadable.cpp by Jan Knipperts, David Konrad
 *
 * buildable.h implementation (originally not separated)
 *
 */

#include <QtWidgets>

#include "Lib.h"
#include "units.h"
#include "shp.h"
#include "buildable.h"


buildablewindow::buildablewindow(double Scale_factor): QWidget(nullptr)
{
    sf = Scale_factor;

    setWindowFlag(Qt::Dialog);
    setWindowFlags(Qt::Dialog | Qt::MSWindowsFixedSizeDialogHint);

    setWindowTitle("Units buildable in factories");

    BuildableImage = QImage((10*Tilesize),((Num_Units/10)+1)*Tilesize, QImage::Format_RGB16); //Create a new QImage object
    BuildableImage.fill(Qt::transparent);

    buildablescrollArea = new(QScrollArea);
    buildablescrollArea->setBackgroundRole(QPalette::Dark);
    buildablescrollArea->setVisible(true);

    buildable_unitname = new QLabel();
    buildable_unitname->setText("");

    QPushButton *Button = new QPushButton("Close");
    QObject::connect(Button, &QPushButton::clicked, [=](){
        close();
    });

    QVBoxLayout *layout = new QVBoxLayout();
    layout->addWidget(buildablescrollArea);
    layout->addWidget(buildable_unitname);
    layout->addWidget(Button);

    setLayout(layout);
    update_buildable();
    show();
}

void buildablewindow::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Escape)
        close();
}

void buildablewindow::update_buildable()
{
    int tx = 0;
    int ty = 0;

    BuildableImage.fill(Qt::transparent);

    SHP.can_be_built[13] = 1; //always force bunker as unbuildable

    for (int tc = 0; tc < Num_Units; tc++) {
        if (SHP.can_be_built[tc] == 0) {
            Draw_Unit(tx*Tilesize, ty*Tilesize, tc*6, 1, &BuildableImage);
        } else {
            Draw_Unit(tx*Tilesize, ty*Tilesize, tc*6, 3, &BuildableImage);
        }
        tx++;
        if (tx == 10) {
            tx = 0;
            ty++;
        }
    }

    BuildableImageScaled = BuildableImage.scaled(BuildableImage.width() * sf, BuildableImage.height() * sf); //Create a scaled version of it

    label = new QLabel();
    label->setPixmap(QPixmap::fromImage(BuildableImageScaled));
    buildablescrollArea->setWidget(label);
}


void buildablewindow::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        if (buildablescrollArea->rect().contains(event->pos())) {
            int pos_x = buildablescrollArea->horizontalScrollBar()->value();
            int pos_y = buildablescrollArea->verticalScrollBar()->value();
            QRect widgetRect = buildablescrollArea->geometry();
            int fx = (event->pos().x()-widgetRect.left()+pos_x) / (Tilesize*sf); // Calc field position from mouse cords
            int fy = (event->pos().y()-widgetRect.top()+pos_y) / (Tilesize*sf);

            int unit = ((fy*10)+fx);     //Calc correct unit number

            buildable_unitname->setText(Unit_Name[unit]);

            if (unit <= Num_Units) {
               if (SHP.can_be_built[unit] == 0) {
                   SHP.can_be_built[unit] = 1;
               } else {
                   SHP.can_be_built[unit] = 0;
               }
               emit BuildableChanged();
               update_buildable();
            }
        }
    }
}
