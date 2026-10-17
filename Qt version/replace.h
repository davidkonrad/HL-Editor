#ifndef REPLACE_H
#define REPLACE_H

#include "qwidget.h"

QT_BEGIN_NAMESPACE
class QScrollArea;
class QScrollBar;
class QLabel;
QT_END_NAMESPACE


class replacewindow : public QWidget
{
    Q_OBJECT

private:

signals:
    void TilesReplaced();

public:
    QLabel        *Tile1;
    QLabel        *Tile2;
    QImage        tile_image1;
    QImage        tile_image2;
    unsigned char r1;
    unsigned char r2;
    bool          replace_accepted = false;

protected:
    void mousePressEvent (QMouseEvent *event) override;
    void closeEvent (QCloseEvent *event) override;

};

#endif // REPLACE_H
