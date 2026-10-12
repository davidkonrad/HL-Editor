#include <QAction>
#include <QSettings>
#include <QScrollArea>
#include <QLabel>
#include <QPainter>

QAction          *lockWindowTilesizeAct; //!?

QSettings        *Settings;                          // Our new config file :)
QString          REG_GAMEDIR = "GameDir";            // Constants to avoid confusion, each referenced multiple places
QString          REG_SHOW_WARNINGS = "ShowWarnings";
QString          REG_SCALE_FACTOR = "ScaleFactor";
QString          REG_SHOW_GRID = "ShowGrid";
QString          REG_SHOW_COORDS = "ShowCoords";
QString          REG_LOCK_TILESIZE = "LockTileSize";
QString          REG_AUTOLOAD = "AutoLoad";
QString          REG_RECENT_MAP = "RecentMap";
QString          REG_RECENT_FILES = "recentFiles";
QString          REG_RESTORE_WINDOWS = "RestoreWindows";
QString          REG_MAINWINDOW_POS = "MainWindowPos";
QString          REG_MAINWINDOW_SIZE = "MainWindowSize";
QString          REG_MAINWINDOW_SCROLL_POS = "MainWindowScrollPos";
QString          REG_TILELIST_GEO = "tilelistGeometry";
QString          REG_UNITLIST_GEO = "unitlistGeometry";
QString          REG_AUTO_OVERWRITE_BUILDINGS = "autogenOverwriteBuildings";
QString          REG_AUTO_OVERWRITE_ROADS = "autogenOverwriteRoads";

double           Scale_factor = 2.0;                // Default scaling factor for the old VGA bitmaps is 2x
double           Scale_factor_max = 5;
double           Scale_factor_min = 0.5;
double           Scale_factor_locked = 0;           // dadk, If above 0, lock child window tile sizes to that number

QImage           BasicTileListImage;                // Screenbuffers for the basic tile selection window
QImage           ExtTileListImage;
QImage           BasicTileListImageScaled;          // Additional buffers for the scaled images
QImage           ExtTileListImageScaled;

QScrollArea      *BasicTilescrollArea;              // ScrollArea for basic tiles
QScrollArea      *ExtTilescrollArea;                // ScrollArea for extended tiles
QLabel           *BasicTilescrollArea_current_label = NULL;
QLabel           *tile_selection_title1;            // label before basic tiles, previous local QLabel title
QLabel           *tile_selection_title2;            // label before extended tiles, previous local QLabel title2

QLabel           *ExtTilescrollArea_current_label   = NULL;

unsigned char    selected_tile = 0x00;              // Define "Plains" as default tile
bool             no_tilechange = false;

//copy, duplicate
const int Tilesize_ = 24;
const int Tileshift_ = 8;
const int Num_Parts_ = 175;

void Draw_Hexagon(int x, int y, QPen Pen, QImage *Image, bool align, bool scaling, bool isChildWindow = false)
//dadk, added a isChildWindow flag, indicating if target is tilelist or unitlist
{
    double sf = isChildWindow && lockWindowTilesizeAct->isChecked() ? Settings->value(REG_LOCK_TILESIZE).toDouble() : Scale_factor;

    int xp,yp;
    QPainter painter(Image);
    painter.setPen(Pen);

    if (align)
    {
        xp = x * (Tilesize_ - Tileshift_);
        if (x % 2 != 0)
            yp = (y * Tilesize_) + (Tilesize_ / 2);
        else
            yp = (y * Tilesize_) ;
    }
    else
    {
         xp = (x * Tilesize_);
         yp = (y * Tilesize_);
    }

    if (scaling)
    {
        xp = xp * sf;
        yp = yp * sf;

        painter.drawLine(xp,yp+((Tilesize_ / 2) * sf), xp + (Tileshift_ * sf), yp);
        painter.drawLine(xp+(Tileshift_ * sf), yp, xp + ((Tileshift_ *2 ) * sf), yp);
        painter.drawLine(xp+((Tileshift_ * 2) * sf), yp, xp+(Tilesize_*sf), yp + ((Tilesize_/2)*sf));
        painter.drawLine(xp+(Tilesize_ * sf), yp + ((Tilesize_/2)*sf),xp+((Tileshift_*2)*sf),yp+(Tilesize_*sf));
        painter.drawLine(xp+(Tileshift_*sf),yp+(Tilesize_*sf),xp+((Tileshift_*2)*sf),yp+(Tilesize_*sf));
        painter.drawLine(xp,yp+((Tilesize_/2)*sf),xp+(Tileshift_*sf),yp+(Tilesize_*sf));
    }
    else
    {
        painter.drawLine(xp,yp+(Tilesize_/2),xp+Tileshift_,yp);
        painter.drawLine(xp+Tileshift_,yp,xp+(Tileshift_*2),yp);
        painter.drawLine(xp+(Tileshift_*2),yp,xp+Tilesize_,yp+(Tilesize_/2));
        painter.drawLine(xp+Tilesize_,yp+(Tilesize_/2),xp+(Tileshift_*2),yp+Tilesize_);
        painter.drawLine(xp+Tileshift_,yp+Tilesize_,xp+(Tileshift_*2),yp+Tilesize_);
        painter.drawLine(xp,yp+(Tilesize_/2),xp+Tileshift_,yp+Tilesize_);
    }

    painter.end();
}
