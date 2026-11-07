/*
 * HL Editor
 *
 * mapinfounits.cpp by David Konrad
 *
 * implementation for mapinfounits dialog
 *
 */


#include <QtWidgets>

#include "fin.h"
#include "shp.h"
#include "units.h"

#include "mapinfounits.h"


int MAPINFOUNITS_MAXTILES = 16;

int mapInfoUnits_processUnit(int unit_num, int side, int &x_pos, int &y_pos, int &strength, QImage &image)
{
    int units = 0;

    //cycle through buildings and transporters
    for (int i = 0; i < Building_stat.num_buildings; i++) {
        if (Building_info[i].Properties->Owner == side) {
            for (int inv = 0; inv < 7; inv++) {
                if (Building_info[i].Properties->Units[inv] == unit_num) {
                    units = units + 1;
                    strength = strength + unit_get_resources(unit_num);
                }
            }
        }
    }

    //cycle through map
    for (int offset = 0; offset < (Map.width * Map.height) * 2; offset += 2) {
        if (Map.data[offset+1]/2 == unit_num) {
            if (Map.data[offset+1] != 0xFF) {
                if ((Map.data[offset+1] % 2) != 1) {
                    if (side == 0) {
                        units = units + 1;
                        strength = strength + unit_get_resources(unit_num);
                    }
                } else {
                    if (side == 1) {
                        units = units + 1;
                        strength = strength + unit_get_resources(unit_num);
                    }
                }
            }
        }
    }

    if (units > 0) {
        for (int u = 0; u < units; u++) {
            int extra = (u % 2 != 0) ? 4 : 0;
            Draw_Unit(x_pos, y_pos + extra, (unit_num * 6) + 3, side + 1, &image);
            x_pos = x_pos + (Tilesize / 2);
            if (x_pos >= ((MAPINFOUNITS_MAXTILES - 1) * Tilesize)) {
               x_pos = 0;
               y_pos = y_pos + Tilesize + 5;
            }
       }
       if (x_pos > 0) x_pos = x_pos + (Tilesize / 2);
       if (x_pos >= ((MAPINFOUNITS_MAXTILES - 1) * Tilesize)) {
          x_pos = 1;
          y_pos = y_pos + Tilesize + 5;
       }
    }
    return units;
}

int mapInfoUnits_processLargeUnit(int unit_part1, int unit_part2, int side, int &x_pos, int &y_pos, int &strength, QImage &image)
{
    int units1 = 0;
    int units2 = 0;

    //cycle through buildings and transporters
    for (int i = 0; i < Building_stat.num_buildings; i++) {
        if (Building_info[i].Properties->Owner == side) {
            for (int inv = 0; inv < 7; inv++) {
                if (Building_info[i].Properties->Units[inv] == unit_part1) {
                    units1 = units1 + 1;
                    strength = strength + unit_get_resources(unit_part1);
                }
                if (Building_info[i].Properties->Units[inv] == unit_part2) {
                    units2 = units2 + 1;
                    strength = strength + unit_get_resources(unit_part2);
                }
            }
        }
    }

    //cycle through map
    for (int offset = 0; offset < (Map.width * Map.height) * 2; offset += 2) {
        if (Map.data[offset+1]/2 == unit_part1) {
            if (Map.data[offset+1] != 0xFF) {
                if ((Map.data[offset+1] % 2) != 1) {
                    if (side == 0) {
                        units1 = units1 + 1;
                        strength = strength + unit_get_resources(unit_part1);
                    }
                } else {
                    if (side == 1) {
                        units1 = units1 + 1;
                        strength = strength + unit_get_resources(unit_part1);
                    }
                }
            }
        }
        if (Map.data[offset+1]/2 == unit_part2) {
            if (Map.data[offset+1] != 0xFF) {
                if ((Map.data[offset+1] % 2) != 1) {
                    if (side == 0) {
                        units2 = units2 + 1;
                        strength = strength + unit_get_resources(unit_part2);
                    }
                } else {
                    if (side == 1) {
                        units2 = units2 + 1;
                        strength = strength + unit_get_resources(unit_part2);
                    }
                }
            }
        }
    }

    //for now, dont care about not correctly placed ships
    for (int lc = 0; lc < units2; lc++) {
        int extra = (lc % 2 != 0) ? 4 : 0;
        Draw_Unit(x_pos, y_pos + extra, (unit_part2 * 6) + 3, side + 1, &image);
        Draw_Unit(x_pos, y_pos + Tilesize + extra, (unit_part1 * 6) + 3, side + 1, &image);
        x_pos = x_pos + (Tilesize / 2);
        if (x_pos > ((MAPINFOUNITS_MAXTILES -1) * Tilesize)) {
           x_pos = 1;
           y_pos = y_pos + (Tilesize * 2) + 5;
        }
    }
    return units1;
}

int mapInfoUnits_processUnits(QList<int> unit_nums, int side, int &x_pos, int &y_pos, int &strength, QImage &image)
{
    int unit_count = 0;
    for (int unit_num : unit_nums) {
        unit_count = unit_count + mapInfoUnits_processUnit(unit_num, side, x_pos, y_pos, strength, image);
    }
    return unit_count;
}

int mapInfoUnits_processSide(int side, int &x_pos, int &y_pos, int &strength, QImage &image)
{
    int unit_count = 0;
    QList<int> bunker = {13};
    QList<int> infantry = {23, 21, 18, 17, 20, 19, 22, 14, 15, 16};
    QList<int> artillery = {10, 11, 12};
    QList<int> airplanes = {2, 3, 4, 5, 6, 7, 8, 9, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45};
    QList<int> tanks = {0, 1, 46, 47, 48, 49, 50};
    QList<int> trains = {24, 25, 26};
    QList<int> small_ships = {27, 28, 31};

    unit_count = unit_count + mapInfoUnits_processUnits(bunker, side, x_pos, y_pos, strength, image);
    unit_count = unit_count + mapInfoUnits_processUnits(infantry, side, x_pos, y_pos, strength, image);
    unit_count = unit_count + mapInfoUnits_processUnits(artillery, side, x_pos, y_pos, strength, image);
    unit_count = unit_count + mapInfoUnits_processUnits(airplanes, side, x_pos, y_pos, strength, image);
    unit_count = unit_count + mapInfoUnits_processUnits(tanks, side, x_pos, y_pos, strength, image);
    unit_count = unit_count + mapInfoUnits_processUnits(trains, side, x_pos, y_pos, strength, image);
    unit_count = unit_count + mapInfoUnits_processUnits(small_ships, side, x_pos, y_pos, strength, image);

    if (x_pos > 1) {
        x_pos = 1;
        y_pos = y_pos + Tilesize;
    }

    unit_count = unit_count + mapInfoUnits_processLargeUnit(32, 33, side, x_pos, y_pos, strength, image); //destroyer
    if (x_pos > 1) x_pos = x_pos + (Tilesize / 2);

    unit_count = unit_count + mapInfoUnits_processLargeUnit(29, 30, side, x_pos, y_pos, strength, image); //submarine
    if (x_pos > 1) x_pos = x_pos + (Tilesize / 2);

    unit_count = unit_count + mapInfoUnits_processLargeUnit(34, 35, side, x_pos, y_pos, strength, image); //battleship
    if (x_pos == 1) y_pos = y_pos - Tilesize;

    return unit_count;
}

//stupid, not in the mood for refactor just to get the unit count
void mapInfoUnits_setMaxTiles() {
    QImage test_Image = QImage((Tilesize * MAPINFOUNITS_MAXTILES), Tilesize * MAPINFOUNITS_MAXTILES, QImage::Format_RGB16);
    int x_pos = 1;
    int y_pos = 5;
    int strength = 0;
    int gcount = mapInfoUnits_processSide(0, x_pos, y_pos, strength, test_Image);
    x_pos = 1;
    y_pos = 5;
    int fcount = mapInfoUnits_processSide(1, x_pos, y_pos, strength, test_Image);
    x_pos = 1;
    y_pos = 5;
    int ncount = mapInfoUnits_processSide(2, x_pos, y_pos, strength, test_Image);

    //update MAPINFOUNITS_MAXTILES
    if (gcount > 75 || fcount > 75 || ncount > 75)
        MAPINFOUNITS_MAXTILES = 20;

    if (gcount > 100 || fcount > 100 || ncount > 100)
        MAPINFOUNITS_MAXTILES = 24;

    if (gcount < 50 && fcount < 50 && ncount < 50)
        MAPINFOUNITS_MAXTILES = 12;

}

void mapinfounitswindow::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Escape)
        close();
}

mapinfounitswindow::mapinfounitswindow(): QWidget(nullptr)
{
    setWindowFlag(Qt::SubWindow);
    setWindowFlags(Qt::WindowStaysOnTopHint | Qt::WindowTitleHint | Qt::CustomizeWindowHint);
    setWindowTitle("Units overview");

    mapInfoUnits_setMaxTiles();

    QImage german_units_Image = QImage((Tilesize * MAPINFOUNITS_MAXTILES), Tilesize * MAPINFOUNITS_MAXTILES, QImage::Format_RGB16);
    QImage french_units_Image = QImage((Tilesize * MAPINFOUNITS_MAXTILES), Tilesize * MAPINFOUNITS_MAXTILES, QImage::Format_RGB16);
    QImage neutral_units_Image = QImage((Tilesize * MAPINFOUNITS_MAXTILES), Tilesize * MAPINFOUNITS_MAXTILES, QImage::Format_RGB16);

    QString bg = "#dddddd";
    german_units_Image.fill(bg);
    french_units_Image.fill(bg);
    neutral_units_Image.fill(bg);

    //process german
    int x_pos = 1;
    int y_pos = 5;
    int strength = 0;
    int unit_count = mapInfoUnits_processSide(0, x_pos, y_pos, strength, german_units_Image);

    QImage german_units_processed = QImage(german_units_Image.width(), y_pos + (Tilesize * 2), german_units_Image.format());
    QPainter gpainter(&german_units_processed);
    gpainter.drawImage(0, 0, german_units_Image, 0, 0, german_units_Image.width(), y_pos + (Tilesize * 2));
    gpainter.end();

    //process french
    int fx_pos = 1;
    int fy_pos = 5;
    int fstrength = 0;
    int funit_count = mapInfoUnits_processSide(1, fx_pos, fy_pos, fstrength, french_units_Image);

    QImage french_units_processed = QImage(french_units_Image.width(), fy_pos + (Tilesize * 2), french_units_Image.format());
    QPainter fpainter(&french_units_processed);
    fpainter.drawImage(0, 0, french_units_Image, 0, 0, french_units_Image.width(), fy_pos + (Tilesize * 2));
    fpainter.end();

    //process neutral
    int nx_pos = 1;
    int ny_pos = 5;
    int nstrength = 0;
    int nunit_count = mapInfoUnits_processSide(2, nx_pos, ny_pos, nstrength, neutral_units_Image);

    QImage neutral_units_processed = QImage(neutral_units_Image.width(), ny_pos + (Tilesize * 2), neutral_units_Image.format());
    QPainter npainter(&neutral_units_processed);
    npainter.drawImage(0, 0, neutral_units_Image, 0, 0, french_units_Image.width(), ny_pos + (Tilesize * 2));
    npainter.end();

    QScrollArea *panel = new QScrollArea();
    panel->setFrameShape(QFrame::NoFrame);
    panel->setMaximumHeight(30);

    QPushButton *button = new QPushButton("Close");
    button->setIcon(QApplication::style()->standardIcon(QStyle::SP_DialogApplyButton));
    QObject::connect(button, &QPushButton::clicked, [=]() {
        close();
    });

    panel->setWidget(button);
    panel->setAlignment(Qt::AlignRight);

    QImage german_units_imageScaled = german_units_processed.scaled(german_units_processed.width() * 2, german_units_processed.height() * 2);
    QImage french_units_imageScaled = french_units_processed.scaled(french_units_processed.width() * 2, french_units_processed.height() * 2);
    QImage neutral_units_imageScaled = neutral_units_processed.scaled(neutral_units_processed.width() * 2, neutral_units_processed.height() * 2);

    QLabel *glabel = new QLabel();
    glabel->setPixmap(QPixmap::fromImage(german_units_imageScaled));

    QLabel *glabel_text = new QLabel();
    glabel_text->setFrameShape(QFrame::Panel);
    glabel_text->setFrameShadow(QFrame::Raised);
    glabel_text->setLineWidth(2);
    glabel_text->setText("German side: " + QString::number(unit_count) + " units, " + QLocale(QLocale::German).toString(strength) + " resources");

    QLabel *flabel = new QLabel();
    flabel->setPixmap(QPixmap::fromImage(french_units_imageScaled));

    QLabel *flabel_text = new QLabel();
    flabel_text->setFrameShape(QFrame::Panel);
    flabel_text->setFrameShadow(QFrame::Raised);
    flabel_text->setLineWidth(2);
    flabel_text->setText("French side: " + QString::number(funit_count) + " units, " + QLocale(QLocale::German).toString(fstrength) + " resources");

    QLabel *nlabel = new QLabel();
    nlabel->setPixmap(QPixmap::fromImage(neutral_units_imageScaled));

    QLabel *nlabel_text = new QLabel();
    nlabel_text->setFrameShape(QFrame::Panel);
    nlabel_text->setFrameShadow(QFrame::Raised);
    nlabel_text->setLineWidth(2);
    nlabel_text->setText("Neutral: " + QString::number(nunit_count) + " units, " + QLocale(QLocale::German).toString(nstrength) + " resources");

    QVBoxLayout *layout = new QVBoxLayout();
    layout->addWidget(glabel_text);
    layout->addWidget(glabel);
    layout->addWidget(flabel_text);
    layout->addWidget(flabel);

    layout->addWidget(nlabel_text);
    layout->addWidget(nlabel);
    layout->addWidget(panel);

    setLayout(layout);
    show();
}
