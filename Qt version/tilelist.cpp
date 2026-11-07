/*
 * HL Editor
 *
 * tilelist.cpp by Jan Knipperts, David Konrad
 *
 * tilelist.h implementation (originally not separated)
 *
 */

#include <QtWidgets>
#include <QLabel>

#include "Lib.h"
#include "tilelist.h"


tilelistwindow::tilelistwindow(double Scale_factor): QWidget(nullptr)
{
    sf = Scale_factor;

    QRect current_geometry; //if window is already visible, store the position and size

    //if (tile_selection) current_geometry = geometry();

    setWindowFlag(Qt::SubWindow);
    setWindowFlags(Qt::WindowStaysOnTopHint | Qt::WindowTitleHint | Qt::CustomizeWindowHint);
    setWindowTitle("Tile selection");

    tile_selection_title1 = new QLabel();
    tile_selection_title1->setMaximumHeight(20);
    tile_selection_title1->setText("Basic tiles:");

    BasicTileListImage = QImage((10*Tilesize),3*Tilesize, QImage::Format_RGB16); //Create a new QImage object for the tile list
    BasicTileListImage.fill(Qt::transparent);

    int tx = 0;
    int ty = 0;

    for (int tc = 0; tc < 25; tc++)
    {
        Draw_Part(tx*Tilesize, ty*Tilesize, tc, &BasicTileListImage); //Draw the bitmap
        tx++;
        if (tx == 10)
        {
            tx = 0;
            ty++;
        }
    }

    tile_selection_title2 = new QLabel();
    tile_selection_title2->setMaximumHeight(30);
    tile_selection_title2->setText("Extended tiles (only about 78 different ones can be used):"); //dadk, changed from 80 to 78
    tile_selection_title2->setWordWrap(true);
    ExtTileListImage = QImage((10*Tilesize),((Num_Parts-25)/10)*Tilesize, QImage::Format_RGB16); //Create a new QImage object for the tile list
    ExtTileListImage.fill(Qt::transparent);

    tx = 0;
    ty = 0;

    for (int tc = 25; tc < Num_Parts; tc++)
    {
        Draw_Part(tx*Tilesize, ty*Tilesize, tc, &ExtTileListImage); //Draw the bitmap
        tx++;
        if (tx == 10)
        {
            tx = 0;
            ty++;
        }
    }

    BasicTileListImageScaled = BasicTileListImage.scaled(BasicTileListImage.width()*sf, BasicTileListImage.height()*sf); //Create a scaled version of the images
    ExtTileListImageScaled = ExtTileListImage.scaled(ExtTileListImage.width()*sf, ExtTileListImage.height()*sf); //Create a scaled version of it

    QVBoxLayout *layout = new QVBoxLayout();

    label_basic = new QLabel();
    label_basic->setPixmap(QPixmap::fromImage(BasicTileListImageScaled));

    label_ext = new QLabel();
    label_ext->setPixmap(QPixmap::fromImage(ExtTileListImageScaled));

    layout->addWidget(tile_selection_title1);

    BasicTilescrollArea = new(QScrollArea);
    BasicTilescrollArea->setBackgroundRole(QPalette::Dark);
    BasicTilescrollArea->setWidget(label_basic);
    BasicTilescrollArea->setVisible(true);
    BasicTilescrollArea->setMaximumHeight(BasicTileListImageScaled.height() + 4); //adjust max height

    layout->addWidget(BasicTilescrollArea);
    layout->addWidget(tile_selection_title2);

    ExtTilescrollArea = new(QScrollArea);
    ExtTilescrollArea->setBackgroundRole(QPalette::Dark);
    ExtTilescrollArea->setWidget(label_ext);
    ExtTilescrollArea->setVisible(true);
    layout->addWidget(ExtTilescrollArea);

    setLayout(layout);

    setMaximumWidth(label_ext->width() + (Tilesize * 1.2));
    setMaximumHeight(tile_selection_title1->height() +
                                     label_basic->height() +
                                     tile_selection_title2->height() +
                                     label_ext->height() +
                                     (Tilesize * 1.5));

/*
    //restore geometry or position window as originally
    if (!current_geometry.isNull()) {
        setGeometry(current_geometry);
    } else {
        move(screenrect.width()/2, screenrect.top());
    }
*/
    show();
}

void tilelistwindow::update_selection(int x, int y)
{
    BasicTileListImageScaled = BasicTileListImage.scaled(BasicTileListImage.width()*sf, BasicTileListImage.height()*sf);
    ExtTileListImageScaled = ExtTileListImage.scaled(ExtTileListImage.width() * sf, ExtTileListImage.height() * sf);

    if (x > -1 && selected_tile != 0xFF) {
        if (selected_tile < 25) {
            Draw_Hexagon(x, y, QPen(Qt::red, 1), &BasicTileListImageScaled, false, true, sf);
        } else {
            Draw_Hexagon(x, y, QPen(Qt::red, 1), &ExtTileListImageScaled, false, true, sf);
        }
    }

    label_basic->setPixmap(QPixmap::fromImage(BasicTileListImageScaled));
    label_ext->setPixmap(QPixmap::fromImage(ExtTileListImageScaled));
    update();
}


void tilelistwindow::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        QRect b_widgetRect = BasicTilescrollArea->geometry();
        QRect e_widgetRect = ExtTilescrollArea->geometry();

        if (b_widgetRect.contains(event->pos())) {
            int b_pos_x = BasicTilescrollArea->horizontalScrollBar()->value();
            int b_pos_y = BasicTilescrollArea->verticalScrollBar()->value();

            int b_fx = (event->pos().x()-b_widgetRect.left()+b_pos_x) / (Tilesize * sf); // Calc field position from mouse cords
            int b_fy = (event->pos().y()-b_widgetRect.top()+b_pos_y) / (Tilesize * sf);

            if (((b_fy * 10) + b_fx) < 25) {
                selected_tile = ((b_fy * 10) + b_fx);
                emit SelectionChanged();
                update_selection(b_fx, b_fy);
            }
        } else {
            if (e_widgetRect.contains(event->pos())) {
                int e_pos_x = ExtTilescrollArea->horizontalScrollBar()->value();
                int e_pos_y = ExtTilescrollArea->verticalScrollBar()->value();

                int e_fx = (event->pos().x()-e_widgetRect.left() + e_pos_x) / (Tilesize * sf); // Calc field position from mouse cords
                int e_fy = (event->pos().y()-e_widgetRect.top() + e_pos_y) / (Tilesize * sf);

                if (((e_fy*10)+e_fx) < Num_Parts-1) {
                    selected_tile = ((e_fy*10)+e_fx)+25;
                    emit SelectionChanged();
                    update_selection(e_fx, e_fy);
                }
            }
        }
    }
    if (event->button() == Qt::RightButton) {
        resetSelection();
    }
}


void tilelistwindow::resetSelection()
{
    selected_tile = 0xFF;
    emit SelectionChanged();
    update_selection();
}


