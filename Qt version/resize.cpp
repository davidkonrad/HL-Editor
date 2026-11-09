/*
 * HL Editor
 *
 * resize.cpp by David Konrad
 *
 * resizewindow implementation
 *
 */


#include <QComboBox>
#include <QScrollArea>
#include <QtWidgets>

#include "resize.h"

int tileSize_x = 5; //
int tileSize_y = 6; //

QPoint xstr_to_point(QString str)
{
    QStringList size = str.split("x");
    return QPoint(size[0].toInt(), size[1].toInt());
}

QString point_to_xstr(QPoint p)
{
    return QString::number(p.x()) + "x" + QString::number(p.y());
}

QPoint mouse_to_coord(QPoint mp)
{
    QPoint res;
    res.setX(mp.x()/tileSize_x);
    res.setY(mp.y()/tileSize_y);
    return res;
}

//---------------------------
void resizewindow::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Escape)
        close();
}

QPoint resizewindow::get_mouse_distance(QPoint p)
{
    p = scrollArea->mapFromParent(p);
    return QPoint(p.x()+2, p.y()+2)    ;
}


bool resizewindow::point_inside_map(QPoint p)
{
    p = scrollArea->mapFromParent(p);
/*
    int left = newLeft * tileSize_x;
    int top = newTop * tileSize_y;
    int width = mapWidth * tileSize_x;
    int height = mapHeight * tileSize_y;
*/
    int left = 1;
    int top = 1;
    int width = mapWidth * tileSize_x;
    int height = mapHeight * tileSize_y;

    if ((p.x() >= left && p.x() <= (left + width)) &&
        (p.y() >= top && p.y() <= (top + height)))
        return true;

    return false;
}

QPoint resizewindow::getSize()
{
    return QPoint(newWidth, newHeight);
}

QPoint resizewindow::getMapPosition()
{
    return QPoint(newLeft, newTop);
}

void resizewindow::mousePressEvent(QMouseEvent *event)
{
    if (point_inside_map(event->pos())) {
        DRAGGING_MAP = true;
        QPoint p = scrollArea->mapFromParent(event->pos());
        drag_from = get_mouse_distance(event->pos());
        drag_pos = mouse_to_coord(p);
    } else {
        DRAGGING_MAP = false;
    }
}

void resizewindow::mouseMoveEvent(QMouseEvent *event)
{
    if (DRAGGING_MAP) {
        QPoint p = scrollArea->mapFromParent(event->pos());
        drag_pos = mouse_to_coord(p);
        updateScrollArea();
    }
}

void resizewindow::updateScrollArea()
{
    QImage scrollImage = QImage(66*tileSize_x, 65*tileSize_y, QImage::Format_RGB16);
    scrollImage.fill(Qt::lightGray);

    QPainter painter(&scrollImage);

    //qDebug() << "drag_pos" << drag_pos;

    int xp = 3 + (drag_pos.x()*tileSize_x) - drag_from.x();
    int yp = 3 + (drag_pos.y()*tileSize_y) - drag_from.y();

    newLeft = xp/tileSize_x;
    newTop = yp/tileSize_y;

    //qDebug() << xp << yp << xp/tileSize_x << yp/tileSize_y;

    painter.drawImage(xp, yp, mapImage);

    infoLabel->setText("Relative: " + QString::number(newLeft) + "," + QString::number(newTop));

    QPen pen(Qt::yellow, 3);
    pen.setJoinStyle(Qt::RoundJoin);
    pen.setCapStyle(Qt::RoundCap);
    painter.setPen(pen);

    painter.drawRect(3, 3, (newWidth+1)*tileSize_x, newHeight*tileSize_y); //2,2

    scrollAreaLabel->setPixmap(QPixmap::fromImage(scrollImage));
}

void resizewindow::sizeChanged(int index)
{
    QString xstr = sizeCombo->itemText(index);
    QPoint p = xstr_to_point(xstr);
    newWidth = p.x();
    newHeight = p.y();
    updateScrollArea();
}

resizewindow::resizewindow(QImage *image, QPoint mapSize): QWidget(nullptr)
{
    setWindowFlag(Qt::Dialog);
    setWindowFlags(Qt::Dialog | Qt::MSWindowsFixedSizeDialogHint | Qt::WindowStaysOnTopHint);
    setAttribute(Qt::WA_DeleteOnClose);

    //local tileSize 6, global 24
    mapImage = image->scaled(image->width()/3, image->height()/4);

    mapWidth = mapSize.x();
    mapHeight = mapSize.y();
    newWidth = mapWidth;
    newHeight = mapHeight;

    QString mapSizeStr = point_to_xstr(mapSize);

    QStringList defaultSizes;
    defaultSizes << "16x16" << "16x24" << "16x32" << "16x40" << "24x24" << "24x32" << "32x24" << "32x32"
                 << "32x48" << "40x32" << "40x40" << "48x64" << "64x24" << "64x32" << "64x48";


    infoLabel = new QLabel(this);
    infoLabel->setAlignment(Qt::AlignRight);;
    infoLabel->setText("");

    sizeCombo = new QComboBox(this);
    sizeCombo->addItems(defaultSizes);
    sizeCombo->setCurrentIndex( defaultSizes.indexOf(mapSizeStr) );
    connect(sizeCombo, SIGNAL(currentIndexChanged(int)), this, SLOT(sizeChanged(int)));

    scrollArea = new QScrollArea(this);
    scrollAreaLabel = new QLabel(this);
    updateScrollArea();
    scrollArea->setWidget(scrollAreaLabel);

    QScrollArea *panel = new QScrollArea();
    panel->setFrameShape(QFrame::NoFrame);
    panel->setMaximumHeight(40);

    QPushButton *button = new QPushButton("Change");
    button->setIcon(QApplication::style()->standardIcon(QStyle::SP_DialogApplyButton));
    button->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Ignored);
    QObject::connect(button, &QPushButton::clicked, [=]() {
        emit ResizeMap();
    });

    QHBoxLayout* panelLayout = new QHBoxLayout(this);
    panelLayout->addWidget(button, 0, Qt::AlignRight);
    panel->setLayout(panelLayout);

    QVBoxLayout *layout = new QVBoxLayout();
    layout->addWidget(sizeCombo);
    layout->addWidget(infoLabel);
    layout->addWidget(scrollArea);
    layout->addWidget(panel);

    setLayout(layout);
    setWindowTitle("Resize Map");
}

