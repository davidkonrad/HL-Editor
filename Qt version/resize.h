/*
 * HL Editor
 *
 * resize.h by David Konrad
 *
 * header for the resize dialog
 *
 */

#ifndef RESIZE_H
#define RESIZE_H

#include "qwidget.h"



QT_BEGIN_NAMESPACE
class QScrollArea;
class QScrollBar;
class QLabel;
class QComboBox;
QT_END_NAMESPACE


class resizewindow : public QWidget
{
    Q_OBJECT

public:
    explicit resizewindow(QImage *image, QPoint mapSize);
    QPoint getSize();
    QPoint getMapPosition();

signals:
    void ResizeMap();

private slots:
    void sizeChanged(int index);

protected:
    void keyPressEvent(QKeyEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void updateScrollArea();

private:
    int mapWidth;
    int mapHeight;
    int newWidth;
    int newHeight;
    int newLeft = 0;
    int newTop = 0;

    bool DRAGGING_MAP;
    QPoint drag_from;
    QPoint drag_pos;

    QImage       mapImage;
    QScrollArea  *scrollArea;
    QLabel       *scrollAreaLabel;
    QComboBox    *sizeCombo;

    bool point_inside_map(QPoint p);
    QPoint get_mouse_distance(QPoint p);

};

#endif // RESIZE_H

