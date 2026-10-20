#ifndef TILELIST_H
#define TILELIST_H

#include "qwidget.h"


QT_BEGIN_NAMESPACE
class QScrollArea;
class QScrollBar;
class QLabel;
QT_END_NAMESPACE


class tilelistwindow : public QWidget
{
    Q_OBJECT

public:
    QImage        BasicTileListImage;                // Screenbuffers for the basic tile selection window
    QImage        ExtTileListImage;
    QImage        BasicTileListImageScaled;          // Additional buffers for the scaled images
    QImage        ExtTileListImageScaled;

    QScrollArea   *BasicTilescrollArea;              // ScrollArea for basic tiles
    QScrollArea   *ExtTilescrollArea;                // ScrollArea for extended tiles
    QLabel        *tile_selection_title1;            // label before basic tiles, previous local QLabel title
    QLabel        *tile_selection_title2;            // label before extended tiles, previous local QLabel title2

    QLabel        *label_basic;
    QLabel        *label_ext;

    void resetSelection();

signals:
    void SelectionChanged();

protected:
    void mousePressEvent(QMouseEvent *event) override;

private:

};

#endif // TILELIST_H
