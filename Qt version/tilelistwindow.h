#ifndef TILELISTWINDOW_H
#define TILELISTWINDOW_H

/*
#include <QMainWindow>
#include <QObject>
*/
#include <QWidget>


class tilelistwindow : public QWidget
{
    Q_OBJECT

public:
    void resetSelection(unsigned char newsel = 0xFF);

protected:
    void mousePressEvent(QMouseEvent *event) override;
    void mouseDoubleClickEvent (QMouseEvent *event) override;

private:

};

#endif // TILELISTWINDOW_H

inline void tilelistwindow::mousePressEvent(QMouseEvent *event)
{
    double sf = lockWindowTilesizeAct->isChecked() ? Settings->value(REG_LOCK_TILESIZE).toDouble() : Scale_factor;

    if (event->button() == Qt::LeftButton)
    {
        QRect b_widgetRect = BasicTilescrollArea->geometry();
        QRect e_widgetRect = ExtTilescrollArea->geometry();

        if (b_widgetRect.contains(event->pos()))
        {
            int e_pos_x = ExtTilescrollArea->horizontalScrollBar()->value();
            int e_pos_y = ExtTilescrollArea->verticalScrollBar()->value();

            ExtTileListImageScaled = ExtTileListImage.scaled(ExtTileListImage.width() * sf, ExtTileListImage.height() * sf); //Restore original image
            QLabel *label_e = new QLabel();
            label_e->setPixmap(QPixmap::fromImage(ExtTileListImageScaled));
            ExtTilescrollArea_current_label = label_e;
            ExtTilescrollArea->setWidget(label_e);
            ExtTilescrollArea->horizontalScrollBar()->setValue(e_pos_x); //Reset the scrollArea to last position
            ExtTilescrollArea->verticalScrollBar()->setValue(e_pos_y);

            int b_pos_x = BasicTilescrollArea->horizontalScrollBar()->value();
            int b_pos_y = BasicTilescrollArea->verticalScrollBar()->value();

            int b_fx = (event->pos().x()-b_widgetRect.left()+b_pos_x) / (Tilesize * sf); // Calc field position from mouse cords
            int b_fy = (event->pos().y()-b_widgetRect.top()+b_pos_y) / (Tilesize * sf);

            if (((b_fy * 10) + b_fx) < 25)
            {
                selected_tile = ((b_fy * 10) + b_fx);
                BasicTileListImageScaled = BasicTileListImage.scaled(BasicTileListImage.width() * sf, BasicTileListImage.height() * sf); //Restore original image
                Draw_Hexagon(b_fx, b_fy, QPen(Qt::red, 1), &BasicTileListImageScaled, false, true, true); //Draw the frame
                no_tilechange = false;
                QLabel *label_b = new QLabel();
                BasicTilescrollArea_current_label = label_b; //store current label
                label_b->setPixmap(QPixmap::fromImage(BasicTileListImageScaled));
                BasicTilescrollArea->setWidget(label_b);
                BasicTilescrollArea->horizontalScrollBar()->setValue(b_pos_x); //Reset the scrollArea to last position
                BasicTilescrollArea->verticalScrollBar()->setValue(b_pos_y);
            }
        }
        else
        {
            if (e_widgetRect.contains(event->pos()))
            {
                int b_pos_x = BasicTilescrollArea->horizontalScrollBar()->value();
                int b_pos_y = BasicTilescrollArea->verticalScrollBar()->value();
                BasicTileListImageScaled = BasicTileListImage.scaled(BasicTileListImage.width() * sf, BasicTileListImage.height() * sf); //Restore original image
                QLabel *label_b = new QLabel();
                label_b->setPixmap(QPixmap::fromImage(BasicTileListImageScaled));
                BasicTilescrollArea_current_label = label_b; //store current label
                BasicTilescrollArea->setWidget(label_b);
                BasicTilescrollArea->horizontalScrollBar()->setValue(b_pos_x); //Reset the scrollArea to last position
                BasicTilescrollArea->verticalScrollBar()->setValue(b_pos_y);

                int e_pos_x = ExtTilescrollArea->horizontalScrollBar()->value();
                int e_pos_y = ExtTilescrollArea->verticalScrollBar()->value();
                ExtTileListImageScaled = ExtTileListImage.scaled(ExtTileListImage.width() * sf, ExtTileListImage.height() * sf); //Restore original image

                int e_fx = (event->pos().x()-e_widgetRect.left() + e_pos_x) / (Tilesize * sf); // Calc field position from mouse cords
                int e_fy = (event->pos().y()-e_widgetRect.top() + e_pos_y) / (Tilesize * sf);

                if (((e_fy*10)+e_fx) < Num_Parts-1)
                {
                    selected_tile = ((e_fy*10)+e_fx)+25;
                    Draw_Hexagon(e_fx, e_fy, QPen(Qt::red, 1), &ExtTileListImageScaled, false, true, true); //Draw the frame
                    no_tilechange = false;
                    QLabel *label_e = new QLabel();
                    label_e->setPixmap(QPixmap::fromImage(ExtTileListImageScaled));
                    ExtTilescrollArea_current_label = label_e;
                    ExtTilescrollArea->setWidget(label_e);
                    ExtTilescrollArea->horizontalScrollBar()->setValue(e_pos_x); //Reset the scrollArea to last position
                    ExtTilescrollArea->verticalScrollBar()->setValue(e_pos_y);
                }
            }
        }
        tile_selection->update();
    }

    if (event->button() == Qt::RightButton)
    {
        resetSelection();
    }

    tb_deselect_update();
}

