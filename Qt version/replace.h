#ifndef REPLACE_H
#define REPLACE_H

#include "qwidget.h"
#include "tilelist.h"

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

    explicit replacewindow(double Scale_factor, tilelistwindow *tw);

protected:
    double sf;
    tilelistwindow *tile_window;
    void mousePressEvent (QMouseEvent *event) override;
    //void closeEvent (QCloseEvent *event) override;
    void update_tiles();
    void replace();
};

#endif // REPLACE_H
