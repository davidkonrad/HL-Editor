/*
 * HL Editor
 *
 * new.cpp by David Konrad
 *
 * implementation for new mapp dialog
 *
 */

#include <QComboBox>
#include <QScrollArea>
#include <QtWidgets>

#include "Lib.h"
#include "new.h"


QPoint newwindow::getSize()
{
    QStringList size = sizeCombo->currentText().split("x");
    return QPoint(size[0].toInt(), size[1].toInt());
}

bool newwindow::getOcean()
{
    return oceanCheck->isChecked();
}

bool newwindow::getTwoPlayer()
{
    return twoplayerCheck->isChecked();
}

int newwindow::getFill()
{
    return fillCombo->currentIndex();
}

void newwindow::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Escape)
        close();
}

newwindow::newwindow(): QWidget(nullptr)
{
    setWindowFlag(Qt::Dialog);
    setWindowFlags(Qt::Dialog | Qt::MSWindowsFixedSizeDialogHint | Qt::WindowStaysOnTopHint);
    setAttribute(Qt::WA_DeleteOnClose);

    QStringList defaultSizes;
    defaultSizes << "16x16" << "16x24" << "16x32" << "16x40" << "24x24" << "24x32" << "32x24" << "32x32"
                 << "32x48" << "40x32" << "40x40" << "48x64" << "64x24" << "64x32" << "64x48";

    QLabel *infoLabel = new QLabel(this);
    infoLabel->setText("Map size");

    sizeCombo = new QComboBox(this);
    sizeCombo->addItems(defaultSizes);

    QLabel *fillLabel = new QLabel(this);
    fillLabel->setText("Fill");

    fillCombo = new QComboBox(this);
    fillCombo->addItem(get_part_icon(0), "Plain");
    fillCombo->addItem(get_part_icon(25), "Ocean");
    fillCombo->insertSeparator(2);
    fillCombo->addItem(get_part_icon(54), "Light Forest");
    fillCombo->addItem(get_part_icon(57), "Dense Forest");
    fillCombo->addItem(get_part_icon(78), "Plains");
    fillCombo->addItem(get_part_icon(75), "Wasteland");
    fillCombo->addItem(get_part_icon(84), "Battlefield");
    fillCombo->addItem(get_part_icon(50), "Residental area");

    QScrollArea *options = new QScrollArea();
    options->setMaximumHeight(30);
    options->setFrameShape(QFrame::NoFrame);

    oceanCheck = new QCheckBox("Ocean map");
    QObject::connect(oceanCheck, &QCheckBox::clicked, [=](){
        if (oceanCheck->isChecked())
            fillCombo->setCurrentIndex(1);
    });

    twoplayerCheck = new QCheckBox("Two player");

    QHBoxLayout* optionsLayout = new QHBoxLayout(this);
    optionsLayout->setContentsMargins(0, 0, 0, 0);
    optionsLayout->addWidget(oceanCheck);
    optionsLayout->addWidget(twoplayerCheck);
    options->setLayout(optionsLayout);

    QScrollArea *panel = new QScrollArea();
    panel->setFrameShape(QFrame::NoFrame);
    panel->setMaximumHeight(40);

    QPushButton *button = new QPushButton("Create Map");
    button->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Ignored);
    QObject::connect(button, &QPushButton::clicked, [=]() {
        emit CreateNewMap();
    });

    QHBoxLayout* panelLayout = new QHBoxLayout(this);
    panelLayout->addWidget(button, 0, Qt::AlignRight);
    panel->setLayout(panelLayout);

    QVBoxLayout *layout = new QVBoxLayout();
    layout->addWidget(infoLabel);
    layout->addWidget(sizeCombo);
    layout->addWidget(options);
    layout->addWidget(fillLabel);
    layout->addWidget(fillCombo);
    layout->addWidget(panel);

    setLayout(layout);
    setWindowTitle("Create new Map");

}
