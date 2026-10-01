
#include "mapinfounits.h";

#include <QtWidgets>
#include <QMouseEvent>


void mapinfounitswindow::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Escape)
        close();
}

/*
void mapinfounitswindow::populate()
{
    QImage german_units_Image = QImage((Tilesize * MAPINFOUNITS_MAXTILES), Tilesize * MAPINFOUNITS_MAXTILES, QImage::Format_RGB16);
    QImage french_units_Image = QImage((Tilesize * MAPINFOUNITS_MAXTILES), Tilesize * MAPINFOUNITS_MAXTILES, QImage::Format_RGB16);
    QImage neutral_units_Image = QImage((Tilesize * MAPINFOUNITS_MAXTILES), Tilesize * MAPINFOUNITS_MAXTILES, QImage::Format_RGB16);

    german_units_Image.fill(QWidget::palette().color(QWidget::backgroundRole()));
    french_units_Image.fill(QWidget::palette().color(QWidget::backgroundRole()));
    neutral_units_Image.fill(QWidget::palette().color(QWidget::backgroundRole()));

    int x_pos = 1;
    int y_pos = 5;
    mapInfoUnits_processSide(0, x_pos, y_pos, german_units_Image);

    QImage german_units_processed = QImage(german_units_Image.width(), y_pos + (Tilesize * 2), german_units_Image.format());
    QPainter gpainter(&german_units_processed);
    gpainter.drawImage(0, 0, german_units_Image, 0, 0, german_units_Image.width(), y_pos + (Tilesize * 2));
    gpainter.end();

    int fx_pos = 1;
    int fy_pos = 5;
    mapInfoUnits_processSide(1, fx_pos, fy_pos, french_units_Image);

    QImage french_units_processed = QImage(french_units_Image.width(), fy_pos + (Tilesize * 2), french_units_Image.format());
    QPainter fpainter(&french_units_processed);
    fpainter.drawImage(0, 0, french_units_Image, 0, 0, french_units_Image.width(), fy_pos + (Tilesize * 2));
    fpainter.end();

    int nx_pos = 1;
    int ny_pos = 5;
    mapInfoUnits_processSide(2, nx_pos, ny_pos, neutral_units_Image);

    QImage neutral_units_processed = QImage(neutral_units_Image.width(), ny_pos + (Tilesize * 2), neutral_units_Image.format());
    QPainter npainter(&neutral_units_processed);
    npainter.drawImage(0, 0, neutral_units_Image, 0, 0, french_units_Image.width(), ny_pos + (Tilesize * 2));
    npainter.end();

    //---
    QScrollArea *panel = new QScrollArea();
    panel->setFrameShape(QFrame::NoFrame);
    panel->setMaximumHeight(35);

    //QVBoxLayout *panel_layout = new QVBoxLayout();

    QPushButton *button = new QPushButton("Close");
    QIcon okIcon = style()->standardIcon(QStyle::SP_DialogOkButton);
    button->setIcon(okIcon);
    button-> move(200, 20);//mapinfounits_window->width()-50, 10);
    //button->setGeometry(menu_x_pos+120, menu_y_pos,10,20);

    QObject::connect(button, &QPushButton::clicked, [=]()
    {
        mapinfounits_window->close();
    });
    panel->setWidget(button);

    QImage german_units_imageScaled = german_units_processed.scaled(german_units_processed.width() * 2, german_units_processed.height() * 2);
    QImage french_units_imageScaled = french_units_processed.scaled(french_units_processed.width() * 2, french_units_processed.height() * 2);
    QImage neutral_units_imageScaled = neutral_units_processed.scaled(neutral_units_processed.width() * 2, neutral_units_processed.height() * 2);

    QLabel *glabel = new QLabel();
    glabel->setPixmap(QPixmap::fromImage(german_units_imageScaled));

    QLabel *glabel_text = new QLabel();
    glabel_text->setFrameShape(QFrame::Panel);
    glabel_text->setFrameShadow(QFrame::Raised);
    glabel_text->setLineWidth(2);
    glabel_text->setText("German units");

    QLabel *flabel = new QLabel();
    flabel->setPixmap(QPixmap::fromImage(french_units_imageScaled));

    QLabel *flabel_text = new QLabel();
    flabel_text->setFrameShape(QFrame::Panel);
    flabel_text->setFrameShadow(QFrame::Raised);
    flabel_text->setLineWidth(2);
    flabel_text->setText("French units");

    QLabel *nlabel = new QLabel();
    nlabel->setPixmap(QPixmap::fromImage(neutral_units_imageScaled));

    QLabel *nlabel_text = new QLabel();
    nlabel_text->setFrameShape(QFrame::Panel);
    nlabel_text->setFrameShadow(QFrame::Raised);
    nlabel_text->setLineWidth(2);
    nlabel_text->setText("Neutral units");

    QVBoxLayout *layout = new QVBoxLayout();
    layout->addWidget(glabel_text);
    layout->addWidget(glabel);
    layout->addWidget(flabel_text);
    layout->addWidget(flabel);
    layout->addWidget(nlabel_text);
    layout->addWidget(nlabel);

    layout->addWidget(panel);//Button);

    setLayout(layout);
}

//#include "Lib.h";

/*
#include "fin.h"
#include "codes.h"
#include "shp.h"
#include "units.h"
*/

/*
void mapinfounitswindow::populate()
{
    QImage german_units_Image = QImage((Tilesize * 15),Tilesize * 10, QImage::Format_RGB16); //Create a new QImage object
    QImage french_units_Image = QImage((Tilesize * 15),Tilesize * 10, QImage::Format_RGB16); //Create a new QImage object

    german_units_Image.fill(QWidget::palette().color(QWidget::backgroundRole()));
    french_units_Image.fill(QWidget::palette().color(QWidget::backgroundRole()));

    int x_pos = 1;
    int y_pos = 5;
    processSide(0, x_pos, y_pos, german_units_Image);

    QImage german_units_processed = QImage(german_units_Image.width(), y_pos + (Tilesize * 2), german_units_Image.format());
    QPainter gpainter(&german_units_processed);
    gpainter.drawImage(0, 0, german_units_Image, 0, 0, german_units_Image.width(), y_pos + (Tilesize * 2));
    gpainter.end();

    int fx_pos = 1;
    int fy_pos = 5;
    processSide(1, fx_pos, fy_pos, french_units_Image);

    QImage french_units_processed = QImage(french_units_Image.width(), fy_pos + (Tilesize * 2), french_units_Image.format());
    QPainter fpainter(&french_units_processed);
    fpainter.drawImage(0, 0, french_units_Image, 0, 0, french_units_Image.width(), fy_pos + (Tilesize * 2));
    fpainter.end();

    mapinfounitswindow  *mapinfounits_window;

    mapinfounits_window = new mapinfounitswindow();
    mapinfounits_window->setWindowFlag(Qt::SubWindow);
    mapinfounits_window->setWindowFlags(Qt::WindowStaysOnTopHint | Qt::WindowTitleHint | Qt::CustomizeWindowHint);
    mapinfounits_window->setWindowTitle("Map units overview");


    QPushButton *Button = new QPushButton("Close");
    QObject::connect(Button, &QPushButton::clicked, [=]()
    {
        mapinfounits_window->close();
    });

    QImage german_units_imageScaled = german_units_processed.scaled(german_units_processed.width() * 2, german_units_processed.height() * 2);
    QImage french_units_imageScaled = french_units_processed.scaled(french_units_processed.width() * 2, french_units_processed.height() * 2);

    QLabel *glabel = new QLabel();
    glabel->setPixmap(QPixmap::fromImage(german_units_imageScaled));

    QLabel *glabel_text = new QLabel();
    glabel_text->setFrameShape(QFrame::Panel);
    glabel_text->setFrameShadow(QFrame::Raised);
    glabel_text->setLineWidth(2);
    glabel_text->setText("German units");

    QLabel *flabel = new QLabel();
    flabel->setPixmap(QPixmap::fromImage(french_units_imageScaled));

    QLabel *flabel_text = new QLabel();
    flabel_text->setFrameShape(QFrame::Panel);
    flabel_text->setFrameShadow(QFrame::Raised);
    flabel_text->setLineWidth(2);
    flabel_text->setText("French units");

    QVBoxLayout *layout = new QVBoxLayout();
    layout->addWidget(glabel_text);
    layout->addWidget(glabel);
    layout->addWidget(flabel_text);
    layout->addWidget(flabel);
    layout->addWidget(Button);

    mapinfounits_window->setLayout(layout);

    mapinfounits_window->show();
}
*/

/*
 0  ARMOURED CAR
 1  A7V TANK
 2  GOTHA BOMBER
 3  ZEPPELIN STAAKEN
 4  JUNKERS J4-10
 5  FOKKER E I.
 6  FOKKER E III.
 7  ALBATROS
 8  FOKKER DR I.
 9  FOKKER D VII
10  LIGHT ARTILLERY
11  MEDIUM ARTILLERY
12  HEAVY ARTILLERY
13  BUNKER
14  MOB AA-EMPLACEM.
15  STAT AA-EMPLACEM
16  CONSTR. UNIT
17  ELITE INFANTRY
18  INFANTRY
19  ANTI-TANK GUNS
20  SAPPERS
21  CAVALRY
22  SUPPLY CAR
23  STATIC BALLOON
24  TRAIN ARTILLERY
25  ARMOURED TRAIN
26  SUPPLY TRAIN
27  PATROL BOAT
28  TORPEDO BOAT
29  SUBMARINE
30  SUBMARINE
31  TRANSPORT SHIP
32  DESTROYER
33  DESTROYER
34  BATTLESHIP
35  BATTLESHIP
36  VOISIN III
37  HANDLEY PAGE
38  D.H.4
39  MORANE
40  D.H.2
41  NIEUPORT XVII
42  SPAD VII
43  S.E.5A
44  SPAD XIII
45  SOPWITH CAMEL
46  CHARRON
47  RENAULT FT 17
48  MARK I.
49  ST. CHAMMOND
50  MARK IV
 */
