/*
 * HL Editor
 *
 * replace.cpp by Jan Knipperts, David Konrad
 *
 * replace.h implementation (originally not separated)
 *
 */

#include <QtWidgets>

#include "Lib.h"
#include "fin.h"
#include "replace.h"


replacewindow::replacewindow(double Scale_factor, tilelistwindow *tw): QWidget(nullptr)
{
    sf = Scale_factor;
    tile_window = tw;

    replace_accepted = false;
    r1 = (tile_window->selected_tile != 0xFF) ? tile_window->selected_tile : 0;
    r2 = 0;

    setWindowFlag(Qt::Dialog);
    setWindowFlags(Qt::Dialog | Qt::MSWindowsFixedSizeDialogHint);

    resize(((2*Tilesize) * sf) + 20, Tilesize*sf);
    setWindowTitle("Replace Tile");

    QLabel *textlabel1 = new QLabel();
    textlabel1->setText("Replace");
    QLabel *textlabel2 = new QLabel();
    textlabel2->setText("with");

    tile_image1 = QImage(Tilesize, Tilesize, QImage::Format_RGB16);
    tile_image1.fill(Qt::transparent);
    Draw_Part(0, 0, r1, &tile_image1);

    tile_image2 = QImage(Tilesize, Tilesize, QImage::Format_RGB16);
    tile_image2.fill(Qt::transparent);
    Draw_Part(0, 0, r2, &tile_image2);

    tile_image1 = tile_image1.scaled(tile_image1.width()*sf, tile_image1.height()*sf); //scale it
    tile_image2 = tile_image2.scaled(tile_image2.width()*sf, tile_image2.height()*sf); //scale it

    Tile1 = new QLabel();
    Tile1->setPixmap(QPixmap::fromImage(tile_image1));

    Tile2 = new QLabel();
    Tile2->setPixmap(QPixmap::fromImage(tile_image2));

    QPushButton *replaceBtn = new QPushButton("Replace");
    replaceBtn->setIcon(QIcon(":/images/replace.png"));
    QObject::connect(replaceBtn, &QPushButton::clicked, [=]() {
         replace_accepted = true;
         replace();
    });

    QPushButton *closeBtn = new QPushButton("Close");
    QObject::connect(closeBtn, &QPushButton::clicked, [=]() {
         close();
    });

    QVBoxLayout *btnLayout = new QVBoxLayout();
    btnLayout->setContentsMargins(10, 10, 10, 10);
    btnLayout->addWidget(replaceBtn);
    btnLayout->addWidget(closeBtn);

    QHBoxLayout *layout = new QHBoxLayout();
    layout->addWidget(textlabel1);
    layout->addWidget(Tile1);
    layout->addWidget(textlabel2);
    layout->addWidget(Tile2);
    layout->addLayout(btnLayout);

    setLayout(layout);
    show();
}

void replacewindow::update_tiles()
{
    tile_image1 = QImage(Tilesize,Tilesize, QImage::Format_RGB16);
    tile_image1.fill(Qt::transparent);
    Draw_Part(0, 0, r1, &tile_image1);

    tile_image2 = QImage(Tilesize,Tilesize, QImage::Format_RGB16);
    tile_image2.fill(Qt::transparent);
    Draw_Part(0, 0, r2, &tile_image2);

    tile_image1 = tile_image1.scaled(tile_image1.width() * sf, tile_image1.height() * sf); //scale it
    tile_image2 = tile_image2.scaled(tile_image2.width() * sf, tile_image2.height() * sf); //scale i

    Tile1->setPixmap(QPixmap::fromImage(tile_image1));
    Tile2->setPixmap(QPixmap::fromImage(tile_image2));

    update();
}

void replacewindow::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        QRect widgetRect = Tile1->geometry();
        if (widgetRect.contains(event->pos())) {
            r1 = tile_window->selected_tile;
            update_tiles();
        }
        widgetRect = Tile2->geometry();
        if (widgetRect.contains(event->pos())) {
            r2 = tile_window->selected_tile;
            update_tiles();
        }
    }
}

void replacewindow::replace()
{
    if ((replace_accepted) && (r1 != r2)) {
        int                      x,y,offset;
        field_info               fielddata;
        offset = 0;

        for (y = 0; y < Map.height; y++) {
            for (x = 0; x < Map.width; x++) {
                memcpy(&fielddata, Map.data + offset, sizeof(fielddata));
                if (fielddata.Part == r1) {
                    fielddata.Part = r2;
                    memcpy(Map.data + offset,&fielddata, sizeof(fielddata));
                }
                offset = offset + sizeof(Field);
            }
        }
        emit TilesReplaced(); //will trigger paint and changes update
    }
}

