/*
 * HL Editor
 *
 * new.h by David Konrad
 *
 * header for the new map dialog
 *
 */


#ifndef NEW_H
#define NEW_H

#include "qwidget.h"

QT_BEGIN_NAMESPACE
class QCheckBox;
class QComboBox;
QT_END_NAMESPACE


class newwindow : public QWidget
{
    Q_OBJECT

public:
    explicit newwindow();
    QPoint getSize();
    bool getOcean();
    bool getTwoPlayer();
    int getFill();

signals:
    void CreateNewMap();

private slots:

protected:
    void keyPressEvent(QKeyEvent *event) override;

private:
    QComboBox  *sizeCombo;
    QComboBox  *fillCombo;
    QCheckBox  *oceanCheck;
    QCheckBox  *twoplayerCheck;

};

#endif // NEW_H


