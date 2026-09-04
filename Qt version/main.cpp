/** HL Editor - v1.02
* by Jan Knipperts (Dragonsphere /DOSReloaded)
*
* A map editor for the game History Line 1914-1918 by BlueByte
*
*
*  Known issues:
*  - File access in the main program runs via Qt / QFile routines, but in some header files still via the standard C routines or fopen_s.
*    This is due to the project history (This project started with some small tools to modify the game files of HL1914-1918 (mostly under DOS),
*    then with the first attempt of a GUI editor for maps under DOS and WIN 32 API and then as the current project in Qt) and I was simply too lazy to adjust all the file accesses :)
*
*  - Capacity / weight is not yet taken into account when placing units in transport vehicles.
*    You can put a battleship in a truck if you want to. Of course, this is a bit impractical in the game....
*
*/

#include "mainwindow.h"
#include <QApplication>
#include <QtWidgets>
#include <QLineEdit>
#include <QMessageBox>
#include <QInputDialog>
#include <QScrollArea>
#include <QMouseEvent>
#include <QPushButton>

#include "tilelist.h"
#include "unitlist.h"
#include "buildable.h"
#include "building.h"
#include "replace.h"

//dadk
#include <QToolBar>
#include <QToolButton>

//Global variables and constants:
QString          Title = "History Line 1914-1918 Editor";
QString          Author = "by Jan Knipperts";
QString          Version = "v1.02";

QString          GameDir;                            // Path to History Line 1914-1918 (read from config file)
QString          Map_file;                           // String for user selected map file
QString          MapDir       = "/MAP";              // Maps should be in the MAP sub directory of the game
QString          Palette_name = "/00.PAL";           // Standard VGA Palette file of the game
QString          Code_name    = "/CODES.DAT";        // File with the levelcodes
QString          Partlib_S_name = "/LIB/PARTS.LIB";  // Game ressource files for summer tile graphics
QString          Partdat_S_name = "/LIB/PARTS.DAT";
QString          Partlib_W_name = "/LIB/PARTW.LIB";  // Game ressource files for winter tile graphics
QString          Partdat_W_name = "/LIB/PARTW.DAT";
QString          Unitlib_name = "/LIB/UNIT.LIB";
QString          Unitdat_name = "/LIB/UNIT.DAT";
QString          Unitdat2_name = "/UNIT.DAT";

QSettings        *Settings;                          // Our new config file :)
QString          REG_GAMEDIR = "GameDir";            // Constants to avoid confusion, each referenced multiple places
QString          REG_SHOW_WARNINGS = "ShowWarnings";
QString          REG_SCALE_FACTOR = "ScaleFactor";
QString          REG_SHOW_GRID = "ShowGrid";
QString          REG_LOCK_TILESIZE = "LockTileSize";
QString          REG_AUTOLOAD = "AutoLoad";
QString          REG_RECENT_MAP = "RecentMap";
QString          REG_RESTORE_WINDOWS = "RestoreWindows";
QString          REG_MAINWINDOW_POS = "MainWindowPos";
QString          REG_MAINWINDOW_SIZE = "MainWindowSize";
QString          REG_MAINWINDOW_SCROLL_POS = "MainWindowScrollPos";
QString          REG_TILELIST_GEO = "tilelistGeometry";
QString          REG_UNITLIST_GEO = "unitlistGeometry";

QString          Actual_Level = "";
int              Actual_Levelnum;

QImage           MapImage;                           // I use a QImage as Screenbuffer to draw the map
QImage           MapImageScaled;                     // Additional buffer for the scaled map image
QScrollArea      *scrollArea;

QImage           BasicTileListImage;                // Screenbuffers for the basic tile selection window
QImage           ExtTileListImage;
QImage           BasicTileListImageScaled;          // Additional buffers for the scaled images
QImage           ExtTileListImageScaled;

QScrollArea      *BasicTilescrollArea;              // ScrollArea for basic tiles
QScrollArea      *ExtTilescrollArea;                // ScrollArea for extended tiles
QLabel           *tile_selection_title1;            // label before basic tiles, previous local QLabel title
QLabel           *tile_selection_title2;            // label before extended tiles, previous local QLabel title2

tilelistwindow   *tile_selection;                   // and a widget for it (NOTE: was declared as QWidget)

QImage           UnitListImage;                     // Same for the Unit selection window
QImage           UnitListImageScaled;
QScrollArea      *unitscrollArea;
unitlistwindow   *unit_selection;                   //was decalred as type QWidget thus no 'custom methods' could be called

QImage           BuildableImage;                    //..and the child window to define buildable units
QImage           BuildableImageScaled;
QScrollArea      *buildablescrollArea;
QWidget          *buildable;

QImage           Building_Image;                    //..and the child window to define a buildings contents
QImage           Building_Image_Scaled;
QScrollArea      *Building_ScrollArea;
QWidget          *building_window;
QLineEdit*       RessourceEdit;

QWidget          *replacedlg;
QImage           tile_image1;
QLabel           *Tile1;
QImage           tile_image2;
QLabel           *Tile2;
unsigned char    r1,r2;

QRect            screenrect;                        // A QRect to save the screen geometry and position windows accordingly.
QLabel           *unit_name_text;                   // Text label to display unit name on mouse over
QLabel           *buildable_unitname;

/*
 dadk
 this ended up being the solution for the 'right click problem'.
 In the original code from GitHub, any right click on child windows caused the program to crash,
 the line was unitlistScrollArea->setWidget() (also tilelist) even though the code is exact the same as in left click.
 It works if the current unitlist / tilelist scrollarea references are stored (the inserted QLabels)
 and we only refresh the existing QLabel, not creating and inserting a new one
*/
QLabel                   *scrollArea_current_label = NULL;
QLabel                   *BasicTilescrollArea_current_label = NULL;
QLabel                   *ExtTilescrollArea_current_label = NULL;
QLabel                   *unitscrollArea_current_label = NULL;

QAction                  *lockWindowTilesizeAct; //!?
QAction                  *hideNativeMapsAct;
QAction                  *restoreWindowPosAct;
QAction                  *resetSettingsAct;

//perhaps this could be in some kind of struct or class?
double                   Scale_factor = 2.0;                // Default scaling factor for the old VGA bitmaps is 2x
double                   Scale_factor_locked = 0;           // dadk, If above 0, lock child window tile sizes to that number
unsigned char            selected_tile = 0x00;              // Define "Plains" as default tile
unsigned char            selected_unit = 0xFF;              // No unit is selected by default
int                      selected_building = -1;            // No building is selected by default
bool                     Res_loaded = false;                // to check if bitmaps have already been loaded into memory
bool                     summer = true;                     // set summer as default season for map ressources
bool                     no_tilechange = false;
bool                     changes = false;
bool                     already_saved = false;
bool                     replace_accepted = false;
bool                     grid_enabled = false;
bool                     Player2 = true;
bool                     Ocean = false;
bool                     Update_Ressources;

//SHA-1 Checksums of different .COM file types in HL 1914-1918 (packed and unpacked)

auto TypeI_checksum = QByteArray::fromHex("7101b53f49a9c4784625944cc210edd2798f26fa");  //Checksum for TPWM packed file
auto TypeI_checksum_up = QByteArray::fromHex("b02d7202be4c1a0b0f6b56200c231a72aea1b4e0");//Checksum for unpacked file
auto TypeII_checksum = QByteArray::fromHex("e0d664b4e2a1074f3fecb0f0e6e5f3e49631accc");
auto TypeII_checksum_up = QByteArray::fromHex("2104f458621bd2e3480d5ee05ae9e6ce30f112f7");
auto TypeIII_checksum = QByteArray::fromHex("76fd238fe293c61bed9c5e1ad7e741d15d93e706");
auto TypeIII_checksum_up = QByteArray::fromHex("ed34dbb1fc78e2c2de7036451508f26c51954e8b");
auto TypeIV_checksum = QByteArray::fromHex("a58db2cea96ad91674458f78d820208ef42fdbd1");
auto TypeIV_checksum_up = QByteArray::fromHex("923faf349491634b722a43c00160a10cf8418add");

//now include the code to load game ressources etc.

#include "Lib.h"
#include "fin.h"
#include "codes.h"
#include "shp.h"
#include "units.h"
#include "other.h"


//--------------------------------------
bool Check_levelcode(QString code)
{
    if (code.length() != 5)          //Wrong length for levelcode
        return false;

    int i;

    for (i = 0; i < code.length(); i++)
    {
        if (!code.at(i).isLetter())   //character is no letter
            return false;
        if (!code.at(i).toLatin1())   //character is no valid ASCII character
            return false;
    }

    return true;
}

/*
 * Calc field position from mouse coords
 * Thanks to Amit Patel for this elegant solution of a pixel coordinates to hexagon coordinates algorithm!
 * Source: http://www-cs-students.stanford.edu/~amitp/Articles/GridToHex.html
 * -- moved to own function to avoid redundancy
*/
QPoint mouseToFieldPos(QPoint mouse_pos)
{
    int halfsize = Tilesize/2;
    int mx = mouse_pos.x() / Scale_factor; //Let's leave out the scaling to make things easier
    int my = mouse_pos.y() / Scale_factor;
    int hy = my / halfsize;
    int hx = mx / (Tilesize - Tileshift);

    int diagonale[2][12] = {            //the x values of the diagonals of the hexagon
        {7,6,6,5,4,4,3,3,2,1,1,0},
        {0,1,1,2,3,3,4,4,5,6,6,7}
    };

    if( diagonale[(hy+hx)%2][my %halfsize] >= mx %(Tilesize - Tileshift) ) //We can use the y coordinate (modulo the half row height) as an index into the diagonal
      hx--;

    hy = ((hy-(hx%2))/2);

    if (hx < 0) hx = 0;  //Just to be save...
    if (hy < 0) hy = 0;

    return QPoint(hx, hy);
}


//--------------------------------------



//=================== Main Window  ==========================


MainWindow::MainWindow()
//Creates Main Window and adds a scroll area to display maps
{
    createActions();
    createMenus();
    createToolbar();
    update_window_title();

    //initialize images and the scroll area
    MapImage = QImage(); //Create a new QImage object for the map image
    MapImageScaled = QImage(); //100,100,QImage::Format_RGB16); //Create a scaled version of it  
    scrollArea = new(QScrollArea);
    scrollArea->setBackgroundRole(QPalette::Dark);
    scrollArea->setVisible(true);

    setCentralWidget(scrollArea);
}


void MainWindow::set_changes_state(bool state)
//Set new 'changes' state for menu, toolbar and title
{
    if (changes == state) return;
    changes = state;
    tb_save_changes->setEnabled(state);
    saveAct->setEnabled(state);
    update_window_title();
}


void MainWindow::update_window_title()
//Unified setWindowTitle, replace the various different setWindowTitle
{
    QString t;
    t = Title + " " + Author + " " + Version; //dadk, have skipped the +" - Version: "+

    //only maps loaded from outside the GameDir will show full path, otherwise xxxx.FIN
    if (!Map_file.isEmpty())
    {
        QString f = Map_file;
        t = t + "  ::  " + f.replace(GameDir + "/MAP/", "");
    }

    if (!Actual_Level.isEmpty())
    {
        QString l = Actual_Level;
        t = t + " [" + l.toUpper() + "]";
    }

    if (changes) t = t + " *";
    setWindowTitle(t);
}


void MainWindow::closeEvent(QCloseEvent *event)
//Own closeEvent handler, primary to make sure allocated memory will be properly released
{
    if ((Map.loaded == true) && (changes == true))
    {
        QMessageBox dlg;
        QMessageBox::StandardButton answer;
        dlg.setWindowFlags(dlg.windowFlags() | Qt::WindowStaysOnTopHint);

        answer = dlg.question(this,
                              "Save changes",
                              "There are unsaved changes to the map. Do you want to save them before quitting?",
                              QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel);

        if (answer == QMessageBox::Cancel) {
            event->ignore();
            return;
        }
        if (answer == QMessageBox::Yes) {
            Save();
        }
    }

    if (restoreWindowPosAct->isChecked()) saveWindowPos();

    Release_Buffers();
    event->accept();
}


void MainWindow::mouseDoubleClickEvent( QMouseEvent *event )
{
    //Double Click to delete unit on current field
/*
    if (event->button() == Qt::LeftButton)
    {
        int pos_x = scrollArea->horizontalScrollBar()->value();
        int pos_y = scrollArea->verticalScrollBar()->value();
        QPoint mouse_pos = scrollArea->mapFromParent(event->pos());
        mouse_pos = mouse_pos + QPoint(pos_x,pos_y);

        QPoint h = mouseToFieldPos(mouse_pos);

        if ((h.x() < (Map.width-1)) && (h.y() < (Map.height-1)))  //Is the field on the map?
        {
            int field_pos = (h.y()*Map.width)+h.x();

            //Transport unit?
            if ((Map.data[(field_pos*2)+1] == 0x2C) ||
                (Map.data[(field_pos*2)+1] == 0x2D) ||
                (Map.data[(field_pos*2)+1] == 0x34) ||
                (Map.data[(field_pos*2)+1] == 0x35) ||
                (Map.data[(field_pos*2)+1] == 0x3E) ||
                (Map.data[(field_pos*2)+1] == 0x3F))
            {
                 Map.data[(field_pos*2)+1] = 0xFF;
                 Update_building_record_from_map(); //Update building data record
            }
            else
                 Map.data[(field_pos*2)+1] = 0xFF;

            Redraw_Field(h.x(),h.y(),selected_tile,0xFF);
            MapImageScaled = MapImage.scaled(MapImage.width()*Scale_factor,MapImage.height()*Scale_factor); //Create a scaled version of it
            if(showgridAct->isChecked()) ShowGrid();  //redraw the grid if enabled
            Draw_Hexagon(h.x(),h.y(),QPen(Qt::red, 1), &MapImageScaled, true, false); //redraw the frame

            QLabel *imageLabel = new QLabel;     //Create a scroll area to display the map
            imageLabel->setPixmap(QPixmap::fromImage(MapImageScaled));
            scrollArea->setWidget(imageLabel);

            scrollArea->horizontalScrollBar()->setValue(pos_x); //Reset the scrollArea to last position
            scrollArea->verticalScrollBar()->setValue(pos_y);
            set_changes_state(true); //There are unsaved changes now
        }
    }
*/
}


void MainWindow::mousePressEvent(QMouseEvent *event)
//Handle mouse events on the main window
{
    int pos_x = scrollArea->horizontalScrollBar()->value();
    int pos_y = scrollArea->verticalScrollBar()->value();
    QPoint mouse_pos = scrollArea->mapFromParent(event->pos());
    QPoint h;

    if (Map.loaded != true) return;

    mouse_pos = mouse_pos + QPoint(pos_x,pos_y);

    if (event->button() == Qt::LeftButton)
    {
        h = mouseToFieldPos(mouse_pos);

        if ((h.x() < (Map.width-1)) && (h.y() < (Map.height-1)))  //Is the field on the map?
        {
            int field_pos = (h.y() * Map.width) + h.x();
            unsigned char old_tile = Map.data[field_pos*2];
            unsigned char old_unit = Map.data[(field_pos*2)+1];

            if ((!no_tilechange) && (Map.data[field_pos*2] != selected_tile))
            {
                Map.data[field_pos*2] = (unsigned char) selected_tile;
                set_changes_state(true); //There are unsaved changes now

                if (Settings->value(REG_SHOW_WARNINGS).toBool())
                {
                    if (((selected_tile >= 0x12) && (selected_tile <= 0x14)) ||
                        ((selected_tile >= 0x09) && (selected_tile <= 0x0B)))
                    {
                        show_warning("Attention! Building parts of factories and depots that do not have an associated entrance and are not arranged as intended can still be opened in the game and then contain random garbage data.");
                    }
                }
            }

            if ((selected_unit != 0xFF) && (Map.data[(field_pos*2)+1] != selected_unit))
            {
                Map.data[(field_pos*2)+1] = (unsigned char) selected_unit;
                set_changes_state(true); //There are unsaved changes now

                if (Settings->value(REG_SHOW_WARNINGS).toBool())
                {
                    QString Partname = QString::fromStdString(char2string(Partdat.name[selected_tile],8));
                    bool valid_terrain = true;

                    int unit;
                    if (selected_unit != 0xFF)
                        unit = selected_unit / 2;
                    else
                        unit = Map.data[(field_pos*2)+1] / 2;

                    if (unit > Num_Units) unit = unit-Num_Units;

                    if (getbit(Unit_accessible_terrain[unit],0) == 0) //Deep Water is not accessible by unit
                        if (Partname.contains("SSSEA")) valid_terrain = false;

                    if (getbit(Unit_accessible_terrain[unit],1) == 0) //Railroad tracks are not accessible by unit
                        if (Partname.contains("SRAIL")) valid_terrain = false;

                    if (getbit(Unit_accessible_terrain[unit],2) == 0) //shallow water is not accessible by unit
                        if (Partname.contains("SCOAS")) valid_terrain = false;

                    if (getbit(Unit_accessible_terrain[unit],3) == 0) //Trenches are not accessible by unit
                        if (Partname.contains("SWALL")) valid_terrain = false;

                    if (getbit(Unit_accessible_terrain[unit],4) == 0) //Plains and road/bridge are not accessible by unit
                        if ((Partname.contains("SPLAI")) || (Partname.contains("SSTRE"))) valid_terrain = false;

                    if (getbit(Unit_accessible_terrain[unit],5) == 0) //Forest is not accessible by unit
                        if (Partname.contains("SFORE")) valid_terrain = false;

                    if (getbit(Unit_accessible_terrain[unit],6) == 0) //Mountains and narrow bridges are not accessible by unit
                        if (Partname.contains("SMOUN")) valid_terrain = false;

                    if ((((selected_tile == 0x01) || (selected_tile == 0x02)) ||
                         ((selected_tile >= 0x0C) && (selected_tile <= 0x11))) && (selected_unit != 0xFF))
                        valid_terrain = false;

                    if (!valid_terrain)
                    {
                        show_warning("You have placed a unit on terrain where the game does not provide for it. This can lead to glitches and errors when playing the map in game.");
                    }
                }
            }

            if (((old_tile == 0x01) || (old_tile == 0x02)) ||       //If a building has been set or modified....
                ((old_tile >= 0x0C) && (old_tile <= 0x11)) ||
                ((old_unit == 0x2C) ||
                 (old_unit  == 0x2D) ||
                 (old_unit  == 0x34) ||
                 (old_unit  == 0x35) ||
                 (old_unit  == 0x3E) ||
                 (old_unit  == 0x3F)) ||
                ((selected_tile == 0x01) || (selected_tile == 0x02)) ||
                ((selected_tile >= 0x0C) && (selected_tile <= 0x11)) ||
                ((selected_unit == 0x2C) ||
                 (selected_unit  == 0x2D) ||
                 (selected_unit  == 0x34) ||
                 (selected_unit  == 0x35) ||
                 (selected_unit  == 0x3E) ||
                 (selected_unit  == 0x3F)))
                Update_building_record_from_map(); //...Correct the building data record in memory

            Redraw_Field(h.x(), h.y(), Map.data[(field_pos*2)], Map.data[(field_pos*2)+1]);
            MapImageScaled = MapImage.scaled(MapImage.width() * Scale_factor,MapImage.height() * Scale_factor); //Create a scaled version of it
            if(showgridAct->isChecked()) ShowGrid();  //redraw the grid if enabled
            Draw_Hexagon(h.x(), h.y(), QPen(Qt::red, 1), &MapImageScaled, true, true); //redraw the frame

            QLabel *imageLabel = new QLabel;     //Create a scroll area to display the map
            imageLabel->setPixmap(QPixmap::fromImage(MapImageScaled));
            scrollArea_current_label = imageLabel;
            scrollArea->setWidget(imageLabel);

            scrollArea->horizontalScrollBar()->setValue(pos_x); //Reset the scrollArea to last position
            scrollArea->verticalScrollBar()->setValue(pos_y);
        }
    }

    if (event->button() == Qt::RightButton)
    {
        qDebug() << "right click";

        h = mouseToFieldPos(mouse_pos);

        if ((h.x() > (Map.width-1)) || (h.y() > (Map.height-1)))  //Is the field on the map?
            return;

        int field_pos = (h.y() * Map.width) + h.x();

        qDebug() << "selected" << selected_tile;
        //place mountain?
        if (selected_tile >= 0x43 && selected_tile <= 0x4A) //0x44 .. 0x4A
        {
            place_mountain_on_map(h);
        }

        if (((Map.data[field_pos*2] == 0x01) ||
             (Map.data[field_pos*2] == 0x02)) || // HQ
            ((Map.data[field_pos*2] >= 0x0C) &&
             (Map.data[field_pos*2] <= 0x11)) || //Fabrik, Depot
            ((Map.data[(field_pos*2)+1] == 0x2C) ||
             (Map.data[(field_pos*2)+1] == 0x2D) ||
             (Map.data[(field_pos*2)+1] == 0x34) ||
             (Map.data[(field_pos*2)+1] == 0x35) ||
             (Map.data[(field_pos*2)+1] == 0x3E) ||
             (Map.data[(field_pos*2)+1] == 0x3F))) //Transport
        {
            qDebug() << "right click building";
            selected_building = Get_Building_by_field(field_pos);

            qDebug() << "right click selected_building" << selected_building;
            qDebug() << "right click building_window" << building_window;

            if (building_window == NULL) {
                qDebug() << "right click create window";
                Create_building_configuration_window();
            } else {
                qDebug() << "right click close and create window";
                building_window->close();
                Create_building_configuration_window();
            }
        } else {
            if  (((selected_tile == 0x01) || (selected_tile == 0x02)) ||
                 ((selected_tile >= 0x0C) && (selected_tile <= 0x11)) ||
                (selected_tile == 0x15))
            {
                qDebug() << "right click, create a building";

                if ((selected_tile == 0x01) || (selected_tile == 0x02))
                {
                    Change_Mapdata(h.x(), h.y(),selected_tile, 0xFF);
                    Change_Mapdata(h.x()-1, h.y() + (h.x() % 2),0x05,0xFF);
                    Change_Mapdata(h.x(), h.y() + 1, 0x03, 0xFF);
                    Change_Mapdata(h.x()+1, h.y() + (h.x() % 2), 0x07, 0xFF);
                    Change_Mapdata(h.x()-1, h.y()+(h.x() % 2) + 1, 0x06, 0xFF);
                    Change_Mapdata(h.x(),h.y() + 2, 0x04, 0xFF);
                    Change_Mapdata(h.x() + 1, h.y() + (h.x() % 2) + 1, 0x08, 0xFF);
                }

                if ((selected_tile >= 0x0C) && (selected_tile <= 0x0E))
                {
                    Change_Mapdata(h.x(), h.y(), selected_tile, 0xFF);

                    if (h.x() % 2 == 1)
                    {
                        Change_Mapdata(h.x()-1, h.y(), 0x09, 0xFF);
                        Change_Mapdata(h.x()-1, h.y()+1, 0x0A, 0xFF);
                    }
                    else
                    {
                        Change_Mapdata(h.x()-1, h.y()-1, 0x09, 0xFF);
                        Change_Mapdata(h.x()-1, h.y(), 0x0A, 0xFF);
                    }
                    Change_Mapdata(h.x()-2, h.y(), 0x0B, 0xFF);
                }

                if ((selected_tile >= 0x0F) && (selected_tile <= 0x11))
                {
                    Change_Mapdata(h.x(), h.y(), selected_tile, 0xFF);
                    Change_Mapdata(h.x()-1, h.y() + (h.x() % 2), 0x13, 0xFF);
                    Change_Mapdata(h.x(), h.y() + 1, 0x12, 0xFF);
                    Change_Mapdata(h.x() + 1, h.y() + (h.x() % 2), 0x14, 0xFF);
                }

                if (selected_tile == 0x15)
                {
                    Change_Mapdata(h.x(), h.y(), selected_tile, 0xFF);
                    Change_Mapdata(h.x()-1, h.y() + (h.x() % 2), 0x17, 0xFF);
                    Change_Mapdata(h.x(), h.y() + 1, 0x16, 0xFF);
                    Change_Mapdata(h.x()+1, h.y() + (h.x() % 2), 0x18, 0xFF);
                }
                Update_building_record_from_map();    // ...Correct the building data record in memory
                set_changes_state(true);              // changess is made
                tile_selection->resetSelection(0x00); // set tile selection to grass
            }
        }

        /*
        qDebug() << "right click, in any case";
        MapImageScaled = MapImage.scaled(MapImage.width()*Scale_factor,MapImage.height()*Scale_factor); //Create a scaled version of it

        MapImageScaled = MapImage.scaled(MapImage.width()*Scale_factor,MapImage.height()*Scale_factor); //Create a scaled version of it
        if (showgridAct->isChecked()) ShowGrid();  //redraw the grid if enabled

        if (scrollArea_current_label)
            scrollArea_current_label->setPixmap(QPixmap::fromImage(MapImageScaled));
//

        QLabel *imageLabel = new QLabel;     //Create a scroll area to display the map
        imageLabel->setPixmap(QPixmap::fromImage(MapImageScaled));
        scrollArea->setWidget(imageLabel);
        scrollArea->horizontalScrollBar()->setValue(pos_x); //Reset the scrollArea to last position
        scrollArea->verticalScrollBar()->setValue(pos_y);
*/
        //Redraw_Field(h.x(), h.y(), Map.data[(field_pos*2)], Map.data[(field_pos*2)+1]);

        MapImageScaled = MapImage.scaled(MapImage.width() * Scale_factor, MapImage.height() * Scale_factor); //Create a scaled version of it
        if(showgridAct->isChecked()) ShowGrid();  //redraw the grid if enabled
        Draw_Hexagon(h.x(), h.y(), QPen(Qt::red, 1), &MapImageScaled, true, true); //redraw the frame

        if (scrollArea_current_label) {
            scrollArea_current_label->setPixmap(QPixmap::fromImage(MapImageScaled));
            scrollArea->horizontalScrollBar()->setValue(pos_x); //Reset the scrollArea to last position
            scrollArea->verticalScrollBar()->setValue(pos_y);
            scrollArea->setFocus(); //verticalScrollBar()->setValue(pos_y);
            scrollArea_current_label->setFocus();
        } else {
            qDebug() << "no scrollArea_current_label";
        }
/*
        scrollArea->setFocus();
        scrollArea->horizontalScrollBar()->setValue(pos_x); //Reset the scrollArea to last position
        scrollArea->verticalScrollBar()->setValue(pos_y);
*/
/*
        QLabel *imageLabel = new QLabel;     //Create a scroll area to display the map
        imageLabel->setPixmap(QPixmap::fromImage(MapImageScaled));
        scrollArea->setWidget(imageLabel);
        scrollArea->horizontalScrollBar()->setValue(pos_x); //Reset the scrollArea to last position
        scrollArea->verticalScrollBar()->setValue(pos_y);
*/

    }
}

void MainWindow::resizeEvent (QResizeEvent*)
{
    if (restoreWindowPosAct->isChecked())
        saveWindowPos();
}

void MainWindow::moveEvent (QMoveEvent*)
{
    if (restoreWindowPosAct->isChecked())
        saveWindowPos();
}

//========== Context menu ======================

#ifndef QT_NO_CONTEXTMENU
void MainWindow::contextMenuEvent(QContextMenuEvent *event)
{
    QMenu menu(this);
    menu.exec(event->globalPos());
}
#endif // QT_NO_CONTEXTMENU


void MainWindow::newFile_diag()
{
    if (!Res_loaded)
    {
        if (Load_Ressources() != 0)
        {
            show_error("Failed to load bitmaps from the game!");
            return;
        }
    }

    if ((Map.loaded == true) && (changes == true))
    {
        if (ask_question("There are unsaved changes to the map. Do you want to save them?") == true)
            Save();
    }

    if (Map.data != NULL) free(Map.data);

    Map.width = 16; //Set default width and height
    Map.height = 16;
    Map.data_size = ((Map.width+1) * (Map.height+1)) * 2;

    if ((Map.data = (unsigned char*)malloc(Map.data_size)) == NULL)
    {
        show_error("Memory allocation error!");
        return;
    }
    else
    {
        unsigned char tile;
        if (ask_question("Do you want to create an ocean map?") == true)
        {
            tile = 0x30;
            Ocean = true;
        }
        else
        {
            tile = 0x00;
            Ocean = false;
        }

        int x,y,o;
        o = 0;
        for (y = 1; y <= Map.height; y++)
        {
          for (x = 1; x <= Map.width; x++)
          {
              if ((x == Map.width) || (y == Map.height))
              {
                  Map.data[o] =  0xAE;
                  Map.data[o+1] =  0xFF;
              }
              else
              {
                  Map.data[o] =  tile;
                  Map.data[o+1] =  0xFF;
              }

              o = o + 2;
          }
        }

        Map.loaded = true; //Blank map loaded successfully ;)

        if (!MapImage.isNull()) MapImage = QImage(); //Release mem for the last used image
        if (!MapImageScaled.isNull()) MapImageScaled = QImage(); //Release mem for the last used scaled image
        MapImage = QImage(((Map.width/2)*Tilesize)+(((Map.width/2)-1)*Tileshift),((Map.height-1)*Tilesize)+(Tilesize/2), QImage::Format_RGB16); //Create a new QImage object for the map image
        MapImage.fill(Qt::transparent);   
        Draw_Map(); //and draw the map to it
        MapImageScaled = MapImage.scaled(MapImage.width()*Scale_factor,MapImage.height()*Scale_factor); //Create a scaled version of it        
        QLabel *imageLabel = new QLabel;     //Update the scrollArea
        imageLabel->setPixmap(QPixmap::fromImage(MapImageScaled));
        if (scrollArea == NULL) scrollArea = new(QScrollArea);
        scrollArea->setWidget(imageLabel);

        if (showtilewindowAct->isChecked() == true)
        {
          if (tile_selection == NULL)
              Create_Tileselection_window();
          else
              tile_selection->show();
        }

        if (showunitwindowAct->isChecked() == true)
        {
          if (unit_selection == NULL)
              Create_Unitselection_window();
          else
              unit_selection->show();
        }

        Building_stat.num_HQ = 0;
        Building_stat.num_F = 0;
        Building_stat.num_D = 0;
        Building_stat.num_T = 0;
        Building_stat.num_buildings = 0;
        if (Building_info != NULL) free(Building_info);
        if (SHP.buildings != NULL) free(SHP.buildings);
        Building_info = (Building_data_ext*) malloc(sizeof(Building_data_ext));
        memset(SHP.can_be_built,0,sizeof(SHP.can_be_built));
        SHP.buildings = 0;
        Num_building_entries = 0;

        set_changes_state(true);
        already_saved = false;
        Actual_Level = "";
        Actual_Levelnum = -1;
        //setWindowTitle(Title+" "+Author+" - Version: "+Version);
        update_window_title();
        Player2 = true;
        maptypeAct->setChecked(true);
    }
}


void MainWindow::Open_Map()
{
    QString    C_Filename1;
    QString    C_Filename2;

    if (!Map_file.isEmpty() && !Map_file.isNull())
    {
        if (!Res_loaded)
        {
          if (Load_Ressources() != 0)
          {
              show_error("Failed to load bitmaps from the game!");
              return;
          }
        }

        if (Load_Map() == 0) //success
        {                  
            if (!summer)
            {
                C_Filename1 = get_path(Partlib_W_name);
                C_Filename2 = get_path(Partdat_W_name);
            }
            else  //summer
            {
                C_Filename1 = get_path(Partlib_S_name);
                C_Filename2 = get_path(Partdat_S_name);

            }
            Load_Part_files(C_Filename1.toStdString().data(),C_Filename2.toStdString().data()); //Load correct season graphics

            if (Player2)
                maptypeAct->setChecked(true);
            else
                maptypeAct->setChecked(false);

            MapImage = QImage(((Map.width/2)*Tilesize)+(((Map.width/2)-1)*Tileshift),((Map.height-1)*Tilesize)+(Tilesize/2), QImage::Format_RGB16); //Create a new QImage object for the map image
            MapImage.fill(Qt::transparent);
            Draw_Map(); //and draw the map to it
            MapImageScaled = MapImage.scaled(MapImage.width()*Scale_factor,MapImage.height()*Scale_factor); //Create a scaled version of it
            Map.loaded = true;
            if(showgridAct->isChecked()) ShowGrid();  //redraw the grid if enabled
            QLabel *imageLabel = new QLabel;     //Create a scroll area to display the map
            imageLabel->setPixmap(QPixmap::fromImage(MapImageScaled));
            scrollArea->setWidget(imageLabel);
            if (showtilewindowAct->isChecked() == true)
            {
                if (tile_selection == NULL)
                    Create_Tileselection_window();
                else
                {
                    BasicTileListImageScaled = BasicTileListImage.scaled(BasicTileListImage.width()*Scale_factor,BasicTileListImage.height()*Scale_factor); //Restore original image for basic tiles
                    ExtTileListImageScaled = ExtTileListImage.scaled(ExtTileListImage.width()*Scale_factor,ExtTileListImage.height()*Scale_factor); //Restore original image for extanded tiles
                    Draw_Hexagon(0,0,QPen(Qt::red, 1),&BasicTileListImageScaled,false,true);

                    QLabel *label_b = new QLabel();                                     //Create labels
                    label_b->setPixmap(QPixmap::fromImage(BasicTileListImageScaled));
                    QLabel *label_e = new QLabel();
                    label_e->setPixmap(QPixmap::fromImage(ExtTileListImageScaled));

                    selected_tile = 0;   //no tile selected
                    no_tilechange = false;

                    BasicTilescrollArea->setWidget(label_b);
                    ExtTilescrollArea->setWidget(label_e);
                    tile_selection->update();
                }
            }

            if (showunitwindowAct->isChecked() == true)
            {
                if (unit_selection == NULL)
                    Create_Unitselection_window();
                else
                    unit_selection->show();
            }

            set_changes_state(false);
            already_saved = true;
            Check_used_tiles();
            update_window_title();

            //dadk, update toolbar buttons and more
            tb_move_tl->setEnabled(true);
            tb_move_tr->setEnabled(true);
            tb_move_bl->setEnabled(true);
            tb_move_br->setEnabled(true);
            tb_zoom_in->setEnabled(true);
            tb_zoom_out->setEnabled(true);
            tb_map_info->setEnabled(true);
            tb_replace_tile->setEnabled(true);
            lockWindowTilesizeAct->setEnabled(true);
            if (Scale_factor <= 1) tb_zoom_out->setEnabled(false);
            if (Scale_factor >= 3) tb_zoom_in->setEnabled(false);

            if (autoloadAct->isChecked() == true) {
                // The global Map_file contains the full path, so maps outside /MAP can be autoloaded as well
                Settings->setValue(REG_RECENT_MAP, Map_file);
            }
        }
    }
}

void MainWindow::open_diag()
{
    if ((Map.loaded == true) && (changes == true))
    {
        if (ask_question("There are unsaved changes to the map. Do you want to save them?") == true)
            Save();
    }
    //hide child windows if visible
    if (tile_selection && showtilewindowAct->isChecked() == true) tile_selection->hide();
    if (unit_selection && showunitwindowAct->isChecked() == true) unit_selection->hide();

    Map_file = QFileDialog::getOpenFileName(this, tr("Open History Line 1914-1918 map file"), MapDir, tr("HL map files (*.fin *.FIN)"));

    if (tile_selection && showtilewindowAct->isChecked() == true) tile_selection->show();
    if (unit_selection && showunitwindowAct->isChecked() == true) unit_selection->show();

    Open_Map();
}


void MainWindow::open_by_code_diag()
{
    if ((Map.loaded == true) && (changes == true))
    {
        if (ask_question("There are unsaved changes to the map. Do you want to save them?") == true)
          Save();
    }

    if (!Res_loaded)
    {
        if (Load_Ressources() != 0)
        {
            show_error("Failed to load bitmaps from the game!");
            return;
        }
    }

    bool ok;

    Qt::WindowFlags flags = windowFlags() | Qt::WindowStaysOnTopHint;
    Qt::WindowFlags helpFlag = Qt::WindowContextHelpButtonHint| Qt::WindowMinMaxButtonsHint;
    flags = flags & (~helpFlag);

    QString levelcode = QInputDialog::getItem(this, tr("Open map by levelcode:"),
                                              "Please select a map:", Levelcode.Codelist, 0, false, &ok,flags);

    if (ok && !levelcode.isEmpty())
    {
        if (!Check_levelcode(levelcode))
        {
            show_error("The selected levelcode is invalid! Are the game files corrupted?");
            return;
        }

        int i;
        int fnum;
        fnum = 0;

        for (i=0;i < Levelcode.Codelist.count(); i++)
        {
            if (QString::compare(Levelcode.Codelist[i], levelcode, Qt::CaseInsensitive) == 0)
            {
              fnum = i;
              break;
            }
        }

        if (fnum < 10)
            Map_file = "0"+QString::number(fnum);
        else
            Map_file = QString::number(fnum);

        Map_file = get_path(MapDir + "/" + Map_file + ".FIN"); //

        Open_Map();
    }
}


void MainWindow::save_diag()
{
    if (Map.loaded == true) {
        Save();
        set_changes_state(false);
    } else {
        show_error("There's nothing I could save.... Why don't you load a map first or create a new one?");
    }
}


void MainWindow::saveas_diag()
{
    if (Map.loaded == true)
    {
        already_saved = false;

        //hide child windows if visible
        if (tile_selection && showtilewindowAct->isChecked() == true) tile_selection->hide();
        if (unit_selection && showunitwindowAct->isChecked() == true) unit_selection->hide();

        if (Save()) {
            update_window_title();
            set_changes_state(false);
            Check_used_tiles();
        }

        if (tile_selection && showtilewindowAct->isChecked() == true) tile_selection->show();
        if (unit_selection && showunitwindowAct->isChecked() == true) unit_selection->show();

    }
    else
    {
        show_error("There's nothing I could save.... Why don't you load a map first or create a new one?");
    }
}


void MainWindow::saveimage_diag()
{
    if (Map.loaded == true)
    {
        QString fileName = QFileDialog::getSaveFileName(this, tr("Save Image File"),
                                                        QString(),
                                                        tr("Images (*.png)"));
        if (!fileName.isEmpty())
        {
            if (MapImage.save(fileName) != true)
            {
                show_error("Unfortunately I could not save the image file!");
            }
        }
    }
    else
    {
        show_error("There's nothing I could save to an image file.... Why don't you load a map first or create a new one?");
    }
}


void MainWindow::grid_diag()
{
    if(showgridAct->isChecked())
    {
        ShowGrid();  //Draw a Hexfield-Grid
        grid_enabled = true;
    }
    else
    {
        MapImageScaled = MapImage.scaled(MapImage.width()*Scale_factor,MapImage.height()*Scale_factor);
        grid_enabled = false;
    }

    QLabel *imageLabel = new QLabel;     //Update the scrollArea
    imageLabel->setPixmap(QPixmap::fromImage(MapImageScaled));
    if (scrollArea == NULL) scrollArea = new(QScrollArea);
    scrollArea->setWidget(imageLabel);

    Settings->setValue(REG_SHOW_GRID, showgridAct->isChecked());
}


void MainWindow::statistics_diag()
{
    //Get number of used terrain tiles and units
    int parts = 0;
    int upper_parts = 0;
    int units = 0;

    int g_hq = 0;
    int g_f = 0;
    int g_d = 0;
    int g_res = 0;
    int g_units = 0;

    int f_hq = 0;
    int f_f = 0;
    int f_d = 0;
    int f_res = 0;
    int f_units = 0;

    int n_f = 0;
    int n_d = 0;
    int n_res = 0;
    int n_units = 0;


    unsigned char used_parts[Num_Parts];
    memset(&used_parts,0,sizeof(used_parts));
    unsigned char used_units[Num_Units];
    memset(&used_units,0,sizeof(used_units));

    for (int offset = 0; offset < (Map.width*Map.height)*2; offset += 2)
    {
        if (Map.data[offset] != 0xAE)
        {
            used_parts[Map.data[offset]] = 1;
        }
        if (Map.data[offset+1] != 0xFF)
        {
            if ((Map.data[offset+1] % 2) != 1)
              g_units++;
            else
              f_units++;
            used_units[Map.data[offset+1]/2] = 1;
        }
    }

    for (int i = 0; i < Num_Parts; i++)
        if (used_parts[i] == 1)
        {
            parts++;
            if (i > 24) upper_parts++;
        }

    for (int i = 0; i < Num_Units; i++)
        if (used_units[i] == 1) units++;

    for (int i = 0; i < Building_stat.num_buildings; i++)
    {
        if (Building_info[i].Properties->Owner == 0)
        {
            if (Building_info[i].Properties->Type == 0)
              g_hq++;
            if (Building_info[i].Properties->Type == 1)
              g_f++;
            if (Building_info[i].Properties->Type == 2)
              g_d++;

            g_res += Building_info[i].Properties->Resources;
            for (int i1 = 0; i1 < 6; i1++)
              if (Building_info[i].Properties->Units[i1] != 0xFF) g_units++;  //Add units inside buildings
        }
        if (Building_info[i].Properties->Owner == 1)
        {
            if (Building_info[i].Properties->Type == 0)
              f_hq++;
            if (Building_info[i].Properties->Type == 1)
              f_f++;
            if (Building_info[i].Properties->Type == 2)
              f_d++;

            f_res += Building_info[i].Properties->Resources;
            for (int i1 = 0; i1 < 6; i1++)
              if (Building_info[i].Properties->Units[i1] != 0xFF) f_units++;  //Add units inside buildings
        }
        if (Building_info[i].Properties->Owner == 2)
        {
            if (Building_info[i].Properties->Type == 1)
              n_f++;
            if (Building_info[i].Properties->Type == 2)
              n_d++;

            n_res += Building_info[i].Properties->Resources;
            for (int i1 = 0; i1 < 7; i1++)
              if (Building_info[i].Properties->Units[i1] != 0xFF) n_units++;  //Add units inside buildings
        }
    }

    QString maptype;

    if (Player2 == false)
    {
        QString com = Map_file;
        com.replace(".fin",".com").replace(".FIN",".COM");
        QByteArray Checksum = fileChecksum(com, QCryptographicHash::Sha1);

        if ((Checksum == TypeI_checksum) ||(Checksum == TypeI_checksum_up)) com = "Type I";
        if ((Checksum == TypeII_checksum) ||(Checksum == TypeII_checksum_up)) com = "Type II";
        if ((Checksum == TypeIII_checksum) ||(Checksum == TypeIII_checksum_up)) com = "Type III";
        if ((Checksum == TypeIV_checksum) ||(Checksum == TypeIV_checksum_up)) com = "Type IV";
        maptype = "Single Player "+com+"\n";
    }
    else
        maptype = "Two-player \n";


    QString numbersstr =
        "Map type: "+maptype+
        "Map size: "+QString::number(Map.width)+"x"+QString::number(Map.height)+"\n"+        
        "Different terrain tiles used: "+QString::number(parts)+"\n"+
        " thereof extended terrain tiles: "+QString::number(upper_parts)+"\n"+
        "Different units used: "+QString::number(units)+"\n\n"+

                         "Germany: \n"
                         "Units: "+QString::number(g_units)+"\n"+
                         "Resource income per turn: "+QString::number(g_res)+"\n"+
                         "Buildings: "+QString::number((g_hq+g_f+g_d))+"\n"+
                         " - Headquarters: "+QString::number(g_hq)+"\n"+
                         " - Factories: "+QString::number(g_f)+"\n"+
                         " - Depots: "+QString::number(g_d)+"\n"+
                         "\n"+
                         "France: \n"
                         "Units: "+QString::number(f_units)+"\n"+
                         "Resource income per turn: "+QString::number(f_res)+"\n"+
                         "Buildings: "+QString::number((f_hq+f_f+f_d))+"\n"+
                         " - Headquarters: "+QString::number(f_hq)+"\n"+
                         " - Factories: "+QString::number(f_f)+"\n"+
                         " - Depots: "+QString::number(f_d)+"\n"+
                         "\n"+
                         "Neutral: \n"
                         "Units: "+QString::number(n_units)+"\n"+
                         "Resources: "+QString::number(n_res)+"\n"+
                         "Buildings: "+QString::number(n_f+n_d)+"\n"+
                         " - Factories: "+QString::number(n_f)+"\n"+
                         " - Depots: "+QString::number(n_d)+"\n";

    QMessageBox  Info;

    /* none of the below seem to have affect
    Info.setGeometry(QStyle::alignedRect(
                            Qt::LeftToRight,
                            Qt::AlignCenter,
                            Info.size(),
                            screenrect));
    Info.setWindowFlags(Info.windowFlags() | Qt::WindowStaysOnTopHint);
    Info.raise();
    */

    Info.information(this, "Some informations about your map:", numbersstr);
    //Info.setFixedSize(500,200);
    Info.setGeometry(screenrect.width() / 2, screenrect.height() / 2, 500, 200 );
    Check_used_tiles();
}

void MainWindow::tilewindow_diag()
{
    if(showtilewindowAct->isChecked())
    {
        if (tile_selection == NULL) {
            Create_Tileselection_window();
        } else {
            //for some reason close / show moves the 'window' up; use saved window pos to restore the geometry
            QRect tg = Settings->value(REG_TILELIST_GEO).toRect();
            tile_selection->show();
            if (!tg.isNull()) tile_selection->setGeometry(tg);
        }
    }
    else
    {
        if (tile_selection != NULL) {
            saveWindowPos();
            tile_selection->close();
        }
    }
    //update toolbutton, as mentioned in createToolbar, weird they just cant 'connect', or more likely I misunderstand
    if (tb_tile_window->isChecked() != showtilewindowAct->isChecked()) tb_tile_window->setChecked(showtilewindowAct->isChecked());
}

void MainWindow::unitwindow_diag()
{
    if (showunitwindowAct->isChecked())
    {
        if (unit_selection == NULL) {
            Create_Unitselection_window();
       } else {
            //for some reason close / show moves the 'window' up; use saved window pos to restore the geometry
            QRect ug = Settings->value(REG_UNITLIST_GEO).toRect();
            unit_selection->show();
            if (!ug.isNull()) unit_selection->setGeometry(ug);
       }
    }
    else
    {
        if (unit_selection != NULL) {
            saveWindowPos();
            unit_selection->close();
        }
    }
    //update toolbutton, as mentioned in createToolbar, weird they just cant 'connect'
    if (tb_unit_window->isChecked() != showunitwindowAct->isChecked()) tb_unit_window->setChecked(showunitwindowAct->isChecked());

}


void MainWindow::setPath_diag()
{
    QDir           dir;

    GameDir = QFileDialog::getExistingDirectory(this, tr("Please select the directory of Historyline 1914-1918"),
                                                GameDir,
                                                QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);

    if (!GameDir.isEmpty() && !GameDir.isNull())
    {
        if (!Check_for_game_files())
        {
            show_error("I cannot find the required game files in the selected directory!");
        }
        else
        {
            MapDir = GameDir + MapDir;
            Settings->setValue(REG_GAMEDIR, GameDir);
        }
    }
}


void MainWindow::update_Scale_factor()
// execute Scale_factor change, update Settings, update map, update child windows
{
    Settings->setValue(REG_SCALE_FACTOR, Scale_factor);

    if (Map.loaded)
    {
        int pos_x = scrollArea->horizontalScrollBar()->value();
        int pos_y = scrollArea->verticalScrollBar()->value();

        MapImageScaled = MapImage.scaled(MapImage.width() * Scale_factor, MapImage.height() * Scale_factor);
        if(showgridAct->isChecked()) ShowGrid();  //redraw the grid if enabled

        QLabel *imageLabel = new QLabel;
        imageLabel->setPixmap(QPixmap::fromImage(MapImageScaled));
        scrollArea->setWidget(imageLabel);

        scrollArea->horizontalScrollBar()->setValue(pos_x); //Reset the scrollArea to last position
        scrollArea->verticalScrollBar()->setValue(pos_y);

        //Scale and update the child window contents

        if ( (showunitwindowAct->isChecked() == true) && (lockWindowTilesizeAct->isChecked() == false) )
        {
            UnitListImageScaled = UnitListImage.scaled(UnitListImage.width()*Scale_factor,UnitListImage.height()*Scale_factor);
            QLabel *imageLabel1 = new QLabel;
            imageLabel1->setPixmap(QPixmap::fromImage(UnitListImageScaled));
            unitscrollArea->setWidget(imageLabel1);
            unit_selection->setMaximumWidth(imageLabel1->width() + (Tilesize * 1));
            unit_selection->setMaximumHeight(imageLabel1->height() + (Tilesize * 2));
            unit_selection->resize((imageLabel1->width()), UnitListImageScaled.height());
            unit_selection->update();
        }

        if ( (showtilewindowAct->isChecked() == true) && (lockWindowTilesizeAct->isChecked() == false) )
        {
            BasicTileListImageScaled = BasicTileListImage.scaled(BasicTileListImage.width()*Scale_factor,BasicTileListImage.height()*Scale_factor); //Restore original image for basic tiles
            ExtTileListImageScaled = ExtTileListImage.scaled(ExtTileListImage.width()*Scale_factor,ExtTileListImage.height()*Scale_factor); //Restore original image for extanded tiles
            Draw_Hexagon(0,0,QPen(Qt::red, 1),&BasicTileListImageScaled,false,true);

            QLabel *label_b = new QLabel();                                     //Create labels
            label_b->setPixmap(QPixmap::fromImage(BasicTileListImageScaled));
            QLabel *label_e = new QLabel();
            label_e->setPixmap(QPixmap::fromImage(ExtTileListImageScaled));

            selected_tile = 0;   //no tile selected
            no_tilechange = false;

            BasicTilescrollArea->setWidget(label_b);
            BasicTilescrollArea->setMaximumHeight(BasicTileListImageScaled.height() + (Tilesize / 4)); //adjust max height
            BasicTilescrollArea->setMaximumWidth(label_b->width() + Tilesize);

            ExtTilescrollArea->setWidget(label_e);
            ExtTilescrollArea->setMaximumWidth(label_e->width() + Tilesize);
            ExtTilescrollArea->setMaximumHeight(label_e->height() + Tilesize);

            tile_selection->setMaximumWidth(label_b->width() + Tilesize);
            tile_selection->setMaximumHeight(tile_selection_title1->height() + label_b->height() + tile_selection_title2->height() + label_e->height() + (Tilesize * 2.5 ));
            tile_selection->resize(label_b->width(), tile_selection->height());
            tile_selection->update();
        }
    }
}

void MainWindow::setScale_diag()
{
    bool ok;

    Qt::WindowFlags flags = windowFlags();
    Qt::WindowFlags helpFlag =   Qt::WindowContextHelpButtonHint| Qt::WindowMinMaxButtonsHint;
    flags = flags & (~helpFlag);
    Scale_factor = QInputDialog::getDouble(
        this,
        tr("Scaling of VGA bitmaps:"),
        tr("Enter a factor for scaling"),
        Scale_factor,
        0,
        10,
        1,
        &ok,
        flags);

    if (ok)
    {
        if (Scale_factor < 1) Scale_factor = 1;
        update_Scale_factor();
    }
}


void MainWindow::add_diag()
{
    QString Codefile,Comfile,Newfile;

    if (Map.loaded == true)
    {
        Check_used_tiles();

        bool ok;
        Qt::WindowFlags flags = windowFlags() | Qt::WindowStaysOnTopHint;
        Qt::WindowFlags helpFlag = Qt::WindowContextHelpButtonHint | Qt::WindowMinMaxButtonsHint;
        flags = flags & (~helpFlag);
        QString levelcode = QInputDialog::getText(this, tr("Add map to game"),
                                                  tr("Enter a levelcode for your map (5 letters):"), QLineEdit::Normal,
                                                  "", &ok,flags);
        if (ok && !levelcode.isEmpty())
        {
            if (!Check_levelcode(levelcode))
            {
                show_error("Code must be five letters to work with the game.");
                add_diag(); //?!
                return;
            }

            if (Levelcode_exists(levelcode))
            {
                if (ask_question("Level \"" + levelcode.toUpper() + "\" already exists. Do you want to update/overwrite that level?") == true)
                {
                    remove_level(levelcode);
                } else {
                    return;
                }
            }

            QDir Map_dir(MapDir);
            QStringList maps = Map_dir.entryList(QStringList() << "*.fin" << "*.FIN",QDir::Files);

            for(int i = maps.size()-1; i >= 0; i--)
            {
                if (!maps[i][0].isDigit())
                  maps.removeAt(i);
            }

            if (maps.size()-1 >= 99 )
            {
                show_error("There are too many maps in the directory. The game can handle a maximum of 99.");
                return;
            }

            int filenumber = QString((QString) maps[maps.size()-1][0]+maps[maps.size()-1][1]).toInt();


            if (Levelcode.Number_of_levels != (filenumber+1))
            {
                show_warning("There are " + QString::number((filenumber+1)) + " valid named map-files in the MAP subdirectory, but " +
                            QString::number(Levelcode.Number_of_levels)+ " maps stored in the game's Code.dat file. " +
                            "Please clean up the directory first.");
                return;
            }

            filenumber++;
            Map_file = QString::number(filenumber);
            Map_file = Map_file+".FIN" ;
            Map_file = MapDir+"/"+Map_file;
            Map_file.replace("/'", "\\'");
            if (Save_Mapdata(Map_file.toStdString().data()) != 0) //Create new .FIN file for this map.
            {
                show_error("Failed to create " + Map_file);
                return;
            }

            QString SHPfile;
            SHPfile = Map_file;
            SHPfile.replace(".fin",".shp").replace(".FIN",".SHP");
            if (Create_shp(SHPfile.toStdString().data()) != 0)
            {
                show_warning("I cannot save the building data in " + SHPfile);
            }

            Codefile = (GameDir + Code_name); //Create a C style filename for use of stdio
            Codefile.replace("/'", "\\'");

            if (Add_map(Codefile.toStdString().data(), levelcode) != 0)
            {
                show_error("Can't write data for the new map to CODES.DAT");
                return;
            }


            if (Player2 == false)
            {
                bool            ok;
                QStringList     items;
                QByteArray      Checksum;

                items << "Type I"
                      << "Type II"
                      << "Type III"
                      << "Type IV";

                Qt::WindowFlags flags = windowFlags() | Qt::WindowStaysOnTopHint;
                Qt::WindowFlags helpFlag =   Qt::WindowContextHelpButtonHint| Qt::WindowMinMaxButtonsHint;
                flags = flags & (~helpFlag);

                QString item = QInputDialog::getItem(this, tr("Type of computer opponent"),
                                                 tr("You have configured your map as a single player map. Please select the type of computer opponent (.COM file) for your map:"), items, 0, false, &ok,flags);
                if (ok && !item.isEmpty())
                {
                  QStringList coms = Map_dir.entryList(QStringList() << "*.com" << "*.COM",QDir::Files);

                  if (item == "Type I")
                  {
                        Comfile = "";
                        for(int i = coms.size()-1; i >= 0; i--)
                        {
                            Checksum = fileChecksum(MapDir+"/"+coms[i], QCryptographicHash::Sha1);
                            if ((Checksum == TypeI_checksum) || (Checksum == TypeI_checksum_up))
                             {
                                Comfile = MapDir+"/"+coms[i];
                            }
                        }
                        if (Comfile == "")
                        {
                            show_error("I could not find a type I .COM file in the maps folder that I could use for the new map.");
                        }
                  }
                  if (item == "Type II")
                  {
                        Comfile = "";
                        for(int i = coms.size()-1; i >= 0; i--)
                        {
                            Checksum = fileChecksum(MapDir+"/"+coms[i], QCryptographicHash::Sha1);
                            if ((Checksum == TypeII_checksum) || (Checksum == TypeII_checksum_up))
                            {
                                Comfile = MapDir+"/"+coms[i];
                            }
                        }
                        if (Comfile == "")
                        {
                            show_error("I could not find a type II .COM file in the maps folder that I could use for the new map.");
                        }
                  }
                  if (item == "Type III")
                  {
                        Comfile = "";
                        for(int i = coms.size()-1; i >= 0; i--)
                        {
                            Checksum = fileChecksum(MapDir+"/"+coms[i], QCryptographicHash::Sha1);
                            if ((Checksum == TypeIII_checksum) || (Checksum == TypeIII_checksum_up))
                            {
                                Comfile = MapDir+"/"+coms[i];
                            }
                        }
                        if (Comfile == "")
                        {
                            show_error("I could not find a type III .COM file in the maps folder that I could use for the new map.");
                        }
                  }
                  if (item == "Type IV")
                  {
                        Comfile = "";
                        for(int i = coms.size()-1; i >= 0; i--)
                        {
                            Checksum = fileChecksum(MapDir+"/"+coms[i], QCryptographicHash::Sha1);
                            if ((Checksum == TypeIV_checksum) || (Checksum == TypeIV_checksum_up))
                            {
                                Comfile = MapDir+"/"+coms[i];
                            }
                        }
                        if (Comfile == "")
                        {
                            show_error("I could not find a type IV .COM file in the maps folder that I could use for the new map.");
                        }
                  }

                    Newfile = Map_file;
                    Newfile.replace(".fin",".com").replace(".FIN",".COM");
                    QFile::copy(Comfile, Newfile);

                    if (!QFile::exists(Newfile))
                    {
                        show_error("Failed to create " + Newfile + "!");
                        return;
                    }
                }
            }
            Actual_Level = levelcode;
            set_changes_state(false);
            already_saved = true;
            update_window_title();
        }
    }
    else
    {
        show_warning("There's nothing I could add to the game.... Why don't you load a map first or create a new one?");
    }
}


void MainWindow::remove_level(QString R_levelcode)
// dadk, the level removement itself extracted out to its own function, so it can be executed from elsewhere
{
    QString R_SHPfile, R_Mapfile, R_Comfile, R_Codefile, R_Hifile;
    int old_maxlevel;

    if (!Check_levelcode(R_levelcode))
    {
        show_error("The selected levelcode is invalid! Are the game files corrupted?");
        return;
    }

    int i;
    int fnum;
    fnum = 0;

    for (i=0;i < Levelcode.Codelist.count(); i++)
    {
        if (QString::compare(Levelcode.Codelist[i], R_levelcode, Qt::CaseInsensitive) == 0)
        {
            fnum = i;
            break;
        }
    }
    R_Codefile = get_path(Code_name);

    old_maxlevel = Levelcode.Number_of_levels;

    if (ask_question("All references to the map " + R_levelcode + " will be removed from the game files and all files belonging to the map will be deleted. Are you sure?") == false)
        return;

    if (Remove_map(R_Codefile.toStdString().data(), R_levelcode) != 0)
    {
        show_error("Failed to update the CODES.DAT file!");
        return;
    }

    if (fnum < 10)
        R_Mapfile = "0"+QString::number(fnum);
    else
        R_Mapfile = QString::number(fnum);

    R_Mapfile = R_Mapfile + ".FIN" ;
    R_Mapfile = MapDir + "/" + R_Mapfile;
    R_Mapfile.replace("/'", "\\'");
    R_SHPfile = R_Mapfile;
    R_SHPfile.replace(".fin",".shp").replace(".FIN",".SHP");
    R_Comfile = R_Mapfile;
    R_Comfile.replace(".fin",".com").replace(".FIN",".COM");
    R_Hifile = R_Mapfile;
    R_Hifile.replace(".fin",".hi").replace(".FIN",".HI");

    if (QFile::exists(R_Mapfile))
        QFile::remove(R_Mapfile);
    if (QFile::exists(R_SHPfile))
        QFile::remove(R_SHPfile);
    if (QFile::exists(R_Comfile))
        QFile::remove(R_Comfile);
    if (QFile::exists(R_Hifile))
        QFile::remove(R_Hifile);


    //Alle umbenennen

    QString orig_file, new_file;

    if (fnum < old_maxlevel)
    {
        for (i = fnum+1; i <= old_maxlevel; i++)
        {
            if (i < 10)
                orig_file = "0"+QString::number(i);
            else
                orig_file = QString::number(i);

            if ((i-1) < 10)
                new_file = "0"+QString::number(i-1);
            else
                new_file = QString::number(i-1);


            orig_file = MapDir+"/"+orig_file;
            orig_file.replace("/'", "\\'");
            new_file = MapDir+"/"+new_file;
            new_file.replace("/'", "\\'");


            if (QFile::exists(orig_file+".FIN"))
            {
                QFile::rename(orig_file+".FIN", new_file+".FIN");
            }
            else
            {
                if (QFile::exists(orig_file+".fin"))
                    QFile::rename(orig_file+".fin", new_file+".fin");
            }

            if (QFile::exists(orig_file+".SHP"))
            {
                QFile::rename(orig_file+".SHP", new_file+".SHP");
            }
            else
            {
                if (QFile::exists(orig_file+".shp"))
                    QFile::rename(orig_file+".shp",new_file+".shp");
            }

            if (QFile::exists(orig_file+".COM"))
            {
                QFile::rename(orig_file+".COM",new_file+".COM");
            }
            else
            {
                if (QFile::exists(orig_file+".com"))
                    QFile::rename(orig_file+".com",new_file+".com");
            }

            if (QFile::exists(orig_file+".HI"))
            {
                QFile::rename(orig_file+".HI",new_file+".HI");
            }
            else
            {
                if (QFile::exists(orig_file+".hi"))
                    QFile::rename(orig_file+".hi",new_file+".hi");
            }
        }
    }

    if (Map.loaded == true)
    {
        if (Actual_Level == R_levelcode)
        {
            //setWindowTitle(Title+" "+Author+" - Version: "+Version);
            update_window_title();

            Actual_Level = "";
            set_changes_state(true);
            already_saved = false;
       }
    }
}

bool isNativeMap(QString level, int code)
//test if the level code is a 'native' HL map
{
    QStringList native;
    native << "PULSE" << "CIVIL" <<  "MOUSE" <<  "VENOM" <<  "NOISE" <<  "RIGHT" <<  "ORKAN" <<  "FRONT" <<  "RATIO" <<  "PARTS" <<  "PLANE" <<  "FLAME" <<  "GOTHA" <<  "BALON" <<  "PAUSE" <<  "ELITE" <<  "INFRA" <<  "HILLS" <<  "COBRA" <<  "ATLAS" <<  "AMPER" <<  "RHEIN" <<  "CANDL" <<  "STERN" <<  "BATLE" <<  "GOOSE" <<  "SPORT" <<  "BIMBO" <<  "TEMPO" <<  "BARON" <<  "BUMMM" <<  "LEVEL" <<  "TOXIN" <<  "PRINC" <<  "CLEAN" <<  "XENON" <<  "SIGNS" <<  "HOUSE" <<  "SIGMA" <<  "SEVEN" <<  "ZOMBI" <<  "MOVES" <<  "BLADE" <<  "ZORRO" <<  "STONE" <<  "MOSEL" <<  "ORDER" <<  "SODOM" <<  "TRACK" <<  "HUSAR" <<  "BEAST" <<  "PLATE" <<  "LIGHT" <<  "SCROL" <<  "VIRUS" <<  "BISON" <<  "DRUCK" <<  "TROLL" <<  "UBOOT" <<  "DROID" <<  "GRAND" <<  "ROYAL" <<  "WATER" <<  "SKILL" <<  "SKULL" <<  "AUDIO" <<  "SPELL" <<  "CAMEL" <<  "FLAGS" <<  "STORY" <<  "SCOUT" <<  "GREEN";
    return native.contains(level) && code < 72;
}

void MainWindow::remove_diag()
{
    bool ok;
    int i;

    Qt::WindowFlags flags = windowFlags() | Qt::WindowStaysOnTopHint;
    Qt::WindowFlags helpFlag = Qt::WindowContextHelpButtonHint | Qt::WindowMinMaxButtonsHint;
    flags = flags & (~helpFlag);

    QStringList levels;
    if (hideNativeMapsAct->isChecked())
    {
        for (i=0; i < Levelcode.Codelist.count(); i++)
        {
            if (isNativeMap(Levelcode.Codelist[i], i) == false)
            {
                levels << Levelcode.Codelist[i];
            }
        }
        if (levels.count() < 1)
        {
            show_info("No custom maps found. If you want to remove a native map, you must uncheck Settings -> Hide native maps");
            return;
        }
    } else {
        levels = Levelcode.Codelist;
    }

    QString R_levelcode = QInputDialog::getItem(this,
                                                tr("Remove map from game"),
                                                "Which map should be removed from the game?",
                                                levels, //Levelcode.Codelist,
                                                0,
                                                false,
                                                &ok,
                                                flags);

    if (ok && !R_levelcode.isEmpty())
        remove_level(R_levelcode);
}


void MainWindow::map_resize_diag()
{
    if  (Map.loaded == true)
    {
        int             current, width, height,x,y,o,o1;
        unsigned char*  data;
        bool            ok;
        QStringList     items;

        //we must investigate alternative or 'custom' map formats
        items << "16x16"
            << "16x24"
            << "16x32"
            << "16x40"
            << "24x24"
            << "24x32"
            << "32x24"
            << "32x32"
            << "32x48"
            << "40x32"
            << "40x40"
            << "48x64"
            << "64x24"
            << "64x32"
            << "64x48";


        if ((Map.width == 16) && (Map.height == 16)) current = 0;
        if ((Map.width == 16) && (Map.height == 24)) current = 1;
        if ((Map.width == 16) && (Map.height == 32)) current = 2;
        if ((Map.width == 16) && (Map.height == 40)) current = 3;

        if ((Map.width == 24) && (Map.height == 24)) current = 4;
        if ((Map.width == 24) && (Map.height == 32)) current = 5;

        if ((Map.width == 32) && (Map.height == 24)) current = 6;
        if ((Map.width == 32) && (Map.height == 32)) current = 7;
        if ((Map.width == 32) && (Map.height == 48)) current = 8;

        if ((Map.width == 40) && (Map.height == 32)) current = 9;
        if ((Map.width == 40) && (Map.height == 40)) current = 10;

        if ((Map.width == 48) && (Map.height == 64)) current = 11;

        if ((Map.width == 64) && (Map.height == 24)) current = 12;
        if ((Map.width == 64) && (Map.height == 32)) current = 13;
        if ((Map.width == 64) && (Map.height == 48)) current = 14;

        Qt::WindowFlags flags = windowFlags();
        Qt::WindowFlags helpFlag =   Qt::WindowContextHelpButtonHint| Qt::WindowMinMaxButtonsHint;
        flags = flags & (~helpFlag);

        QString item = QInputDialog::getItem(this, tr("Select map size"),
                                         tr("Map width x height = "), items, current, false, &ok,flags);
        if (ok && !item.isEmpty())
        {
            if (item == "16x16"){ width = 16;height = 16;}
            if (item == "16x24"){ width = 16;height = 24;}
            if (item == "16x32"){ width = 16;height = 32;}
            if (item == "16x40"){ width = 16;height = 40;}
            if (item == "24x24"){ width = 24;height = 24;}
            if (item == "24x32"){ width = 24;height = 32;}
            if (item == "32x24"){ width = 32;height = 24;}
            if (item == "32x32"){ width = 32;height = 32;}
            if (item == "32x48"){ width = 32;height = 48;}
            if (item == "40x32"){ width = 40;height = 32;}
            if (item == "40x40"){ width = 40;height = 40;}
            if (item == "48x64"){ width = 48;height = 64;}
            if (item == "64x24"){ width = 64;height = 24;}
            if (item == "64x32"){ width = 64;height = 32;}
            if (item == "64x48"){ width = 64;height = 48;}

            if ((data = (unsigned char*)malloc(((width+1) * (height+1) * 2))) == NULL)
            {
                show_error("Memory allocation error!");
                return;
            }
            else
            {
                o = 0;
                o1 = 0;
                for (y = 1; y <= height; y++)
                {
                    o1 = ((y-1)*Map.width)*2;
                    for (x = 1; x <= width; x++)
                    {
                        if ((x == width) || (y == height))
                        {
                            data[o] =  (unsigned char) 0xAE;
                            data[o+1] =  (unsigned char) 0xFF;
                        }
                        else
                        {
                            if  ((x >= Map.width) || (y >= Map.height))
                            {
                                if (!Ocean)
                                    data[o] =  (unsigned char) 0x00;
                                else
                                    data[o] =  (unsigned char) 0x30;

                                data[o+1] =  (unsigned char) 0xFF;
                            }
                            else
                            {
                                data[o] = (unsigned char) Map.data[o1];
                                data[o+1] = (unsigned char) Map.data[o1+1];
                                o1 = o1 + 2;
                            }
                        }
                        o = o + 2;
                    }
                }
            }

            if  (Map.data != NULL) free(Map.data); //Release old data
            Map.width = width; //Set new width, height and size
            Map.height = height;
            Map.data_size = ((Map.width+1) * (Map.height+1)) * 2;
            Map.data = data; //Let Map.data point to new buffer

            Add_building_positions(); //Correct building positions

            MapImage = QImage(((Map.width/2)*Tilesize)+(((Map.width/2)-1)*Tileshift),((Map.height-1)*Tilesize)+(Tilesize/2), QImage::Format_RGB16); //Create a new QImage object for the map image
            MapImage.fill(Qt::transparent);
            Draw_Map(); //and draw the map to it
            MapImageScaled = MapImage.scaled(MapImage.width()*Scale_factor,MapImage.height()*Scale_factor); //Create a scaled version of it
            Map.loaded = true;
            if(showgridAct->isChecked()) ShowGrid();  //redraw the grid if enabled

            QLabel *imageLabel = new QLabel;     //Create a scroll area to display the map
            imageLabel->setPixmap(QPixmap::fromImage(MapImageScaled));
            scrollArea->setWidget(imageLabel);

            if (showtilewindowAct->isChecked() == true)
            {
                if (tile_selection == NULL)
                    Create_Tileselection_window();
                else
                    tile_selection->show();
            }

            if (showunitwindowAct->isChecked() == true)
            {
                if (unit_selection == NULL)
                    Create_Unitselection_window();
                else
                    unit_selection->show();
            }
        }
    }
    else
    {
       show_warning("Please load or create a map first.");
    }
}

void MainWindow::season_diag()
{
    QString                  C_Filename1;
    QString                  C_Filename2;

    if (summer == true)   //Change to winter
    {
        C_Filename1 = get_path(Partlib_W_name);
        C_Filename2 = get_path(Partdat_W_name);
        summer = false;
    }
    else
    {
        C_Filename1 = get_path(Partlib_S_name);
        C_Filename2 = get_path(Partdat_S_name);
        summer = true;
    }

    if (Partlib.data != NULL) free(Partlib.data);

    if (Load_Part_files(C_Filename1.toStdString().data(),C_Filename2.toStdString().data()) != 0)
    {
        show_error("Faild to load summer/winter graphics from the game!");
        return;
    }

    if (Map.loaded == true)
    {
        MapImage.fill(Qt::transparent);
        Draw_Map(); //redraw the mapimage

        MapImageScaled = MapImage.scaled(MapImage.width()*Scale_factor,MapImage.height()*Scale_factor); //Create a scaled version of it
        if(showgridAct->isChecked()) ShowGrid();  //redraw the grid if enabled
        QLabel *imageLabel = new QLabel;     //Create a scroll area to display the map
        imageLabel->setPixmap(QPixmap::fromImage(MapImageScaled));
        scrollArea->setWidget(imageLabel);

        if (showtilewindowAct->isChecked() == true)  //Update Tile selection
        {            
            BasicTileListImageScaled = BasicTileListImage.scaled(BasicTileListImage.width()*Scale_factor,BasicTileListImage.height()*Scale_factor); //Restore original image for basic tiles
            ExtTileListImageScaled = ExtTileListImage.scaled(ExtTileListImage.width()*Scale_factor,ExtTileListImage.height()*Scale_factor); //Restore original image for extanded tiles
            Draw_Hexagon(0,0,QPen(Qt::red, 1),&BasicTileListImageScaled,false,true);

            QLabel *label_b = new QLabel();                                     //Create labels
            label_b->setPixmap(QPixmap::fromImage(BasicTileListImageScaled));
            QLabel *label_e = new QLabel();
            label_e->setPixmap(QPixmap::fromImage(ExtTileListImageScaled));

            selected_tile = 0;   //no tile selected
            no_tilechange = false;

            BasicTilescrollArea->setWidget(label_b);
            ExtTilescrollArea->setWidget(label_e);
            tile_selection->update();
        }
    }
}

void MainWindow::maptype_diag()
{
    if(maptypeAct->isChecked())
    {
        Player2 = true;
    }
    else
    {
        Player2 = false;
    }
}

void MainWindow::replace_diag()
{
    if (Map.loaded == true)
    {
        Create_replace_tile_diag();
    }
    else
    {
        show_warning("Please load or create a map first.");
    }
}

void MainWindow::buildable_units_diag()
{
    if (Map.loaded == true)
    {
        if (buildable == NULL)
            Create_buildable_units_window();
        else
            buildable->show();
    }
    else
    {
        show_warning("Please load or create a map first.");
    }
}


void MainWindow::warning_diag()
{
    Settings->setValue(REG_SHOW_WARNINGS, warningAct->isChecked());
}


void MainWindow::createActions()
{
    newAct = new QAction(tr("&New"), this);
    newAct->setShortcuts(QKeySequence::New);
    newAct->setStatusTip(tr("Create a new map"));
    connect(newAct, &QAction::triggered, this, &MainWindow::newFile_diag);

    openAct = new QAction(tr("&Open map by file"), this);
    openAct->setShortcuts(QKeySequence::Open);
    openAct->setStatusTip(tr("Open an existing map"));
    connect(openAct, &QAction::triggered, this, &MainWindow::open_diag);

    openbyCodeAct = new QAction(tr("Open map by levelcode"),this);
    openbyCodeAct->setStatusTip(tr("Open an existing map by its ingame levelcode"));
    connect(openbyCodeAct, &QAction::triggered, this, &MainWindow::open_by_code_diag);

    saveAct = new QAction(tr("&Save"), this);
    saveAct->setShortcuts(QKeySequence::Save);
    saveAct->setStatusTip(tr("Save the map to disk"));
    connect(saveAct, &QAction::triggered, this, &MainWindow::save_diag);

    saveasAct = new QAction(tr("Save map as..."), this);
    saveasAct->setStatusTip(tr("Save the map to a new file"));
    connect(saveasAct, &QAction::triggered, this, &MainWindow::saveas_diag);

    saveImageAct = new QAction(tr("Save an image of the map"), this);
    saveImageAct->setStatusTip(tr("Save an image of your map"));
    connect(saveImageAct, &QAction::triggered, this, &MainWindow::saveimage_diag);

    addtogameAct = new QAction(tr("&Add map to game"), this);
    addtogameAct->setStatusTip(tr("Adds your map to the game"));
    connect(addtogameAct, &QAction::triggered, this, &MainWindow::add_diag);

    removefromgameAct = new QAction(tr("&Remove map from game"), this);
    removefromgameAct->setStatusTip(tr("Removes a map from the game"));
    connect(removefromgameAct, &QAction::triggered, this, &MainWindow::remove_diag);

    exitAct = new QAction(tr("E&xit"), this);
    exitAct->setShortcuts(QKeySequence::Quit);
    exitAct->setStatusTip(tr("Exit HL Editor"));
    connect(exitAct, &QAction::triggered, this, &QWidget::close);

    showgridAct = new QAction(tr("Show grid"), this);
    showgridAct->setCheckable(true);
    showgridAct->setChecked(false);
    showgridAct->setStatusTip(tr("Show/Hide the grid"));
    connect(showgridAct,&QAction::triggered,this,&MainWindow::grid_diag);

    showtilewindowAct = new QAction(tr("Show tile selection window"), this);
    showtilewindowAct->setCheckable(true);
    showtilewindowAct->setChecked(true);
    showtilewindowAct->setStatusTip(tr("Show/Hide the tile selection window"));
    connect(showtilewindowAct,&QAction::triggered,this,&MainWindow::tilewindow_diag);

    showunitwindowAct = new QAction(tr("Show unit selection window"), this);
    showunitwindowAct->setCheckable(true);
    showunitwindowAct->setChecked(true);
    showunitwindowAct->setStatusTip(tr("Show/Hide the unit selection window"));
    connect(showunitwindowAct,&QAction::triggered,this,&MainWindow::unitwindow_diag);

    mapresizeAct = new QAction(tr("&Resize map"),this);
    mapresizeAct->setStatusTip(tr("Change the size of the map"));
    connect(mapresizeAct,&QAction::triggered,this,&MainWindow::map_resize_diag);

    changeseasonAct = new QAction(tr("Toggle summer/winter"),this);
    changeseasonAct->setStatusTip(tr("Map plays in summer or in winter"));
    connect(changeseasonAct,&QAction::triggered,this,&MainWindow::season_diag);

    replaceAct = new QAction(tr("Replace tile"),this);
    replaceAct->setStatusTip(tr("Replaces one tile with another"));
    connect(replaceAct,&QAction::triggered,this,&MainWindow::replace_diag);

    maptypeAct = new QAction(tr("Two-Player map"),this);
    maptypeAct->setStatusTip(tr("Sets Map type to single or two player map"));
    maptypeAct->setCheckable(true);
    maptypeAct->setChecked(true);
    connect(maptypeAct,&QAction::triggered,this,&MainWindow::maptype_diag);

    statisticsAct = new QAction(tr("Map info"),this);
    statisticsAct->setStatusTip(tr("Shows map statistics"));
    connect(statisticsAct,&QAction::triggered,this,&MainWindow::statistics_diag);

    buildableunitsAct = new QAction(tr("Set buildable units"),this);
    buildableunitsAct->setStatusTip(tr("Which units can be built in factories?"));
    connect(buildableunitsAct,&QAction::triggered,this,&MainWindow::buildable_units_diag);

    setPathAct = new QAction(tr("&Game path"),this);
    setPathAct->setStatusTip(tr("Set path to game resources"));
    connect(setPathAct,&QAction::triggered,this,&MainWindow::setPath_diag);

    setScaleFactorAct = new QAction(tr("&Scale factor"),this);
    setScaleFactorAct->setStatusTip(tr("Scaler used to enlarge/enhance low resolution bitmaps."));
    connect(setScaleFactorAct,&QAction::triggered,this,&MainWindow::setScale_diag);

    warningAct = new QAction(tr("Show warnings"), this);
    warningAct->setCheckable(true);
    //warningAct->setChecked(Settings->value(REG_SHOW_WARNINGS).toBool());
    warningAct->setStatusTip(tr("Issue a warning if the map cannot be displayed correctly in the game or could lead to errors in the game."));
    connect(warningAct,&QAction::triggered,this,&MainWindow::warning_diag);

    //dadk
    hideNativeMapsAct = new QAction(tr("Hide native maps"), this);
    hideNativeMapsAct->setCheckable(true);
    hideNativeMapsAct->setChecked(true);
    hideNativeMapsAct->setStatusTip(tr("Hide native levels from the 'Remove map from game' dialog"));

    lockWindowTilesizeAct = new QAction(tr("Lock Window Tile Sizes"), this);
    lockWindowTilesizeAct->setCheckable(true);
    lockWindowTilesizeAct->setChecked(false);
    lockWindowTilesizeAct->setEnabled(false);
    lockWindowTilesizeAct->setStatusTip(tr("Lock child window Tile sizes to current"));
    connect(lockWindowTilesizeAct,&QAction::triggered, [this] {
        if (lockWindowTilesizeAct->isChecked()) {
           Settings->setValue(REG_LOCK_TILESIZE, Scale_factor);
        } else {
           Settings->remove(REG_LOCK_TILESIZE);
        }
    });

    autoloadAct = new QAction(tr("Autoload recent map"), this);
    autoloadAct->setCheckable(true);
    autoloadAct->setChecked(false);
    autoloadAct->setStatusTip(tr("Autoload recent loaded map"));
    connect(autoloadAct,&QAction::triggered, [this] {
        Settings->setValue(REG_AUTOLOAD, autoloadAct->isChecked());
    });

    restoreWindowPosAct = new QAction(tr("Restore Window positions"), this);
    restoreWindowPosAct->setCheckable(true);
    restoreWindowPosAct->setChecked(false);
    restoreWindowPosAct->setStatusTip(tr("Automatically restore the windows position and size at startup"));
    connect(restoreWindowPosAct,&QAction::triggered, [this] {
        Settings->setValue(REG_RESTORE_WINDOWS, restoreWindowPosAct->isChecked());
        if (restoreWindowPosAct->isChecked())
            saveWindowPos();
    });

    resetSettingsAct = new QAction(tr("Reset All settings"), this);
    resetSettingsAct->setStatusTip(tr("Reset all current settings (except the game path to HistoryLine)"));
    connect(resetSettingsAct,&QAction::triggered, [this] {
        Settings->remove(REG_RESTORE_WINDOWS);
        Settings->remove(REG_MAINWINDOW_POS);
        Settings->remove(REG_MAINWINDOW_SIZE);
        Settings->remove(REG_TILELIST_GEO);
        Settings->remove(REG_UNITLIST_GEO);
        restoreWindowPosAct->setChecked(false);

        Settings->remove(REG_LOCK_TILESIZE);
        lockWindowTilesizeAct->setChecked(false);

        Settings->remove(REG_AUTOLOAD);
        autoloadAct->setChecked(false);

        Settings->remove(REG_SHOW_WARNINGS);
        warningAct->setChecked(false);

    });
}

//dadk
void MainWindow::zoom(bool in) {
    if (in == true) {
        if (Scale_factor < 3) {
            Scale_factor = Scale_factor + 0.5;
         } else {
            tb_zoom_in->setEnabled(false);
         }
         tb_zoom_out->setEnabled(true);
    }
    if (in == false) {
        if (Scale_factor > 0.5) {
            Scale_factor = Scale_factor - 0.5;
        } else {
            tb_zoom_out->setEnabled(false);
        }
        tb_zoom_in->setEnabled(true);
    }
    update_Scale_factor();
}

void MainWindow::saveWindowPos()
//save positions and size of MainWindow, tilelist and unitlist
{
    Settings->setValue(REG_MAINWINDOW_POS, this->pos());
    Settings->setValue(REG_MAINWINDOW_SIZE, this->size());
    Settings->setValue(REG_MAINWINDOW_SCROLL_POS, QPoint(scrollArea->verticalScrollBar()->value(), scrollArea->horizontalScrollBar()->value()));
    if (tile_selection) Settings->setValue(REG_TILELIST_GEO, tile_selection->geometry());
    if (unit_selection) Settings->setValue(REG_UNITLIST_GEO, unit_selection->geometry());
}

void MainWindow::restoreWindowPos()
//restore MainWindow, tilelist and unitlist sizes and positions
{
    QPoint mp = Settings->value(REG_MAINWINDOW_POS).toPoint();
    QSize ms = Settings->value(REG_MAINWINDOW_SIZE).toSize();
    QPoint sp = Settings->value(REG_MAINWINDOW_SCROLL_POS).toPoint();
    QRect tg = Settings->value(REG_TILELIST_GEO).toRect();
    QRect ug = Settings->value(REG_UNITLIST_GEO).toRect();

    this->resize(ms.width(), ms.height());
    this->move(mp.x(), mp.y());

    if (tile_selection) tile_selection->setGeometry(tg);
    if (unit_selection) unit_selection->setGeometry(ug);

    if (Map.loaded) {
        scrollArea->verticalScrollBar()->setValue(sp.x());
        scrollArea->horizontalScrollBar()->setValue(sp.y());
    }
}

void MainWindow::createToolbar()
{
    QToolBar *toolbar;
    toolbar = addToolBar(""); //??
    toolbar->setStyleSheet("QToolBar {border-left:1px dotted rgb(120,120,120);} ::separator{background:#ddd;padding:1rem; };");

    //deselect, unitlist, tilelist resetSelection
    tb_deselect = new QToolButton(this);
    tb_deselect->setIcon(QIcon(":/images/cursor-default-outline.png"));
    tb_deselect->setToolTip("Reset tilelist and unitlist selections");
    connect(tb_deselect, &QToolButton::clicked, [this]() {
        tile_selection-> resetSelection();
        unit_selection->resetSelection();
    });
    toolbar->addWidget(tb_deselect);

    toolbar->addSeparator();

    tb_open_file = new QToolButton(this);
    tb_open_file->setIcon(QIcon(":/images/folder-open-o.png"));
    tb_open_file->setToolTip("Open map from file, Ctrl+O");
    tb_open_file->setAutoRaise(false);
    connect(tb_open_file, &QToolButton::clicked, this, &MainWindow::open_diag);
    toolbar->addWidget(tb_open_file);

    toolbar->addSeparator();

    tb_save_changes = new QToolButton(this);
    tb_save_changes->setIcon(QIcon(":/images/floppy-disk.png"));
    tb_save_changes->setToolTip("Save changes, Ctrl+S");
    tb_save_changes->setEnabled(false);
    connect(tb_save_changes, &QToolButton::clicked, this, &MainWindow::save_diag);
    toolbar->addWidget(tb_save_changes);

    toolbar->addSeparator();

    tb_map_info = new QToolButton(this);
    tb_map_info->setIcon(QIcon(":/images/info-circle.png"));
    tb_map_info->setToolTip("Show map information");
    tb_map_info->setEnabled(false);
    connect(tb_map_info, &QToolButton::clicked, this, &MainWindow::statistics_diag);
    toolbar->addWidget(tb_map_info);

    tb_replace_tile = new QToolButton(this);
    tb_replace_tile->setIcon(QIcon(":/images/move-up.png"));
    tb_replace_tile->setToolTip("Replace tiles");
    tb_replace_tile->setEnabled(false);
    connect(tb_replace_tile, &QToolButton::clicked, this, &MainWindow::replace_diag);
    toolbar->addWidget(tb_replace_tile);

    toolbar->addSeparator();

    tb_zoom_in = new QToolButton(this);
    tb_zoom_in->setIcon(QIcon(":/images/zoom-in-zero.png"));
    tb_zoom_in->setIconSize(QSize(64, 64));
    tb_zoom_in->setEnabled(false);
    tb_zoom_in->setToolTip("Zoom in");
    connect(tb_zoom_in, &QToolButton::clicked, [this]() { zoom(true); });
    toolbar->addWidget(tb_zoom_in);

    tb_zoom_out = new QToolButton(this);
    tb_zoom_out->setIcon(QIcon(":/images/zoom-out-zero.png"));
    tb_zoom_out->setEnabled(false);
    tb_zoom_out->setToolTip("Zoom out");
    connect(tb_zoom_out, &QToolButton::clicked, [this]() { zoom(false); });
    toolbar->addWidget(tb_zoom_out);

    toolbar->addSeparator();

    tb_move_tl = new QToolButton(this);
    tb_move_tl->setIcon(QIcon(":/images/rectangle-1.png"));
    tb_move_tl->setEnabled(false);
    tb_move_tl->setToolTip("Move to top left");
    connect(tb_move_tl, &QToolButton::clicked, [this]() {
        scrollArea->verticalScrollBar()->setValue(0);
        scrollArea->horizontalScrollBar()->setValue(0);
    });
    toolbar->addWidget(tb_move_tl);

    tb_move_bl = new QToolButton(this);
    tb_move_bl->setIcon(QIcon(":/images/rectangle-3.png"));
    tb_move_bl->setEnabled(false);
    tb_move_bl->setToolTip("Move to bottom left");
    connect(tb_move_bl, &QToolButton::clicked, [this]() {
        scrollArea->verticalScrollBar()->setValue(scrollArea->maximumWidth());
        scrollArea->horizontalScrollBar()->setValue(0);
    });
    toolbar->addWidget(tb_move_bl);

    tb_move_tr = new QToolButton(this);
    tb_move_tr->setIcon(QIcon(":/images/rectangle-2.png"));
    tb_move_tr->setEnabled(false);
    tb_move_tr->setToolTip("Move to top right");
    connect(tb_move_tr, &QToolButton::clicked, [this]() {
        scrollArea->verticalScrollBar()->setValue(0);
        scrollArea->horizontalScrollBar()->setValue(scrollArea->maximumWidth());
    });
    toolbar->addWidget(tb_move_tr);

    tb_move_br = new QToolButton(this);
    tb_move_br->setIcon(QIcon(":/images/rectangle-4.png"));
    tb_move_br->setEnabled(false);
    tb_move_br->setToolTip("Move to bottom right");
    connect(tb_move_br, &QToolButton::clicked, [this]() {
        scrollArea->verticalScrollBar()->setValue(scrollArea->maximumHeight());
        scrollArea->horizontalScrollBar()->setValue(scrollArea->maximumWidth());
    });
    toolbar->addWidget(tb_move_br);

    toolbar->addSeparator();

    /*
       dadk, Here I would have assumed you could just 'connect' to showtilewindowAct/showunitwindowAct
       and by that inherit 'checked' status and so on to the button. But apparently I do not understand Qt in details
    */
    tb_tile_window = new QToolButton(this);
    tb_tile_window->setIcon(QIcon(":/images/SSTRE110_color.PNG"));
    tb_tile_window->setToolTip("Toggle Tile selection");
    tb_tile_window->setCheckable(true);
    tb_tile_window->setChecked(true);
    connect(tb_tile_window, &QToolButton::clicked, [this]() {
        showtilewindowAct->setChecked(tb_tile_window->isChecked());

        if (tb_tile_window->isChecked())
           tb_tile_window->setIcon(QIcon(":/images/SSTRE110_color.PNG"));
        else
           tb_tile_window->setIcon(QIcon(":/images/SSTRE110_grayscale.PNG"));

        tilewindow_diag();
    });
    toolbar->addWidget(tb_tile_window);

    tb_unit_window = new QToolButton(this);
    tb_unit_window->setIcon(QIcon(":/images/HEAVY ARTILLERY_color.PNG"));
    tb_unit_window->setToolTip("Toggle Unit selection");
    tb_unit_window->setCheckable(true);
    tb_unit_window->setChecked(true);
    connect(tb_unit_window, &QToolButton::clicked, [this]() {
        showunitwindowAct->setChecked(tb_unit_window->isChecked());

        if (tb_unit_window->isChecked())
           tb_unit_window->setIcon(QIcon(":/images/HEAVY ARTILLERY_color.PNG"));
        else
           tb_unit_window->setIcon(QIcon(":/images/HEAVY ARTILLERY_grayscale.PNG"));

        unitwindow_diag();
    });
    toolbar->addWidget(tb_unit_window);

    tb_child_windows_left = new QToolButton(this);
    tb_child_windows_left->setIcon(QIcon(":/images/box-align-left.png"));
    tb_child_windows_left->setToolTip("Order child windows to the left");
    toolbar->addWidget(tb_child_windows_left);

    tb_child_windows_right = new QToolButton(this);
    tb_child_windows_right->setIcon(QIcon(":/images/box-align-right.png"));
    tb_child_windows_right->setToolTip("Order child windows to the right");
    toolbar->addWidget(tb_child_windows_right);

}

void MainWindow::createMenus()
{
    fileMenu = menuBar()->addMenu(tr("&File"));
    fileMenu->addAction(newAct);
    fileMenu->addAction(openAct);
    fileMenu->addAction(openbyCodeAct);
    fileMenu->addSeparator();
    fileMenu->addAction(saveAct);
    fileMenu->addAction(saveasAct);
    fileMenu->addSeparator();
    fileMenu->addAction(saveImageAct);
    fileMenu->addSeparator();
    fileMenu->addAction(addtogameAct);
    fileMenu->addAction(removefromgameAct);
    fileMenu->addSeparator();
    fileMenu->addAction(exitAct);

    editMenu =  menuBar()->addMenu(tr("&Edit"));
    editMenu->addAction(mapresizeAct);
    editMenu->addAction(changeseasonAct);
    editMenu->addAction(maptypeAct);
    editMenu->addAction(replaceAct);
    editMenu->addAction(buildableunitsAct);
    editMenu->addSeparator();
    editMenu->addAction(showgridAct);
    editMenu->addAction(showtilewindowAct);
    editMenu->addAction(showunitwindowAct);
    editMenu->addAction(statisticsAct);

    configMenu = menuBar()->addMenu(tr("&Settings"));
    configMenu->addAction(setPathAct);
    configMenu->addAction(setScaleFactorAct);
    configMenu->addAction(warningAct);

    //dadk
    configMenu->addSeparator();
    configMenu->addAction(hideNativeMapsAct);
    configMenu->addAction(lockWindowTilesizeAct);
    configMenu->addSeparator();
    configMenu->addAction(autoloadAct);
    configMenu->addAction(restoreWindowPosAct);
    configMenu->addSeparator();
    configMenu->addAction(resetSettingsAct);
}


//========================== Event handling for child windows =================================

void tilelistwindow::mousePressEvent(QMouseEvent *event)
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
}

void tilelistwindow::resetSelection(unsigned char newsel /* = 255*/)
//basically tilelistWindow right click, can now be called from elsewhere
//the repaint is refactored to reuse the already existing QLabel
{
    double sf = lockWindowTilesizeAct->isChecked() ? Settings->value(REG_LOCK_TILESIZE).toDouble() : Scale_factor;

    BasicTileListImageScaled = BasicTileListImage.scaled(BasicTileListImage.width() * sf, BasicTileListImage.height() * sf); //Restore original image for basic tiles
    ExtTileListImageScaled = ExtTileListImage.scaled(ExtTileListImage.width() * sf, ExtTileListImage.height() * sf); //Restore original image for extanded tiles

    selected_tile = newsel;   //no tile selcted, default 0xFF
    no_tilechange = true;

    if (BasicTilescrollArea_current_label)
        BasicTilescrollArea_current_label->setPixmap(QPixmap::fromImage(BasicTileListImageScaled));  //update the image

    if (ExtTilescrollArea_current_label)
        ExtTilescrollArea_current_label->setPixmap(QPixmap::fromImage(ExtTileListImageScaled));  //update the image

    tile_selection->update();
}

void tilelistwindow::mouseDoubleClickEvent (QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton)
        tile_selection->adjustSize();
}


//------------------------------------------------------------------------------

void unitlistwindow::mousePressEvent(QMouseEvent *event)
{
    double sf = lockWindowTilesizeAct->isChecked() ? Settings->value(REG_LOCK_TILESIZE).toDouble() : Scale_factor;

    if (event->button() == Qt::LeftButton)
    {
        if (unitscrollArea->rect().contains(event->pos()))
        {
            int pos_x = unitscrollArea->horizontalScrollBar()->value();
            int pos_y = unitscrollArea->verticalScrollBar()->value();
            UnitListImageScaled = UnitListImage.scaled(UnitListImage.width() * sf, UnitListImage.height() * sf); //Restore original image
            QRect widgetRect = unitscrollArea->geometry();
            int fx = (event->pos().x()-widgetRect.left()+pos_x) / (Tilesize * sf); // Calc field position from mouse cords
            int fy = (event->pos().y()-widgetRect.top()+pos_y)/ (Tilesize * sf);
            int os = selected_unit;

            selected_unit = ((fy * 10) + fx ) * 2;

            if (fy > 5)
              selected_unit = selected_unit - 119;     //Calc correct unit number for french units

            if (selected_unit < Num_Units * 2)
            {
              unit_name_text->setText(Unit_Name[selected_unit / 2]);
              Draw_Hexagon(fx, fy, QPen(Qt::red, 1), &UnitListImageScaled, false, true, true);
            }
            else
            {
              selected_unit = os;
              return;
            }

            QLabel *label = new QLabel();
            label->setPixmap(QPixmap::fromImage(UnitListImageScaled));  //update the image
            unitscrollArea_current_label = label;

            unitscrollArea->setWidget(label);
            unitscrollArea->horizontalScrollBar()->setValue(pos_x); //Reset the scrollArea to last position
            unitscrollArea->verticalScrollBar()->setValue(pos_y);

            unit_selection->update();                           //Update the window contents
        }
    }

    if (event->button() == Qt::RightButton)
    {
        if (unitscrollArea->rect().contains(event->pos()))
        {
            resetSelection();
        }
    }
}

void unitlistwindow::resetSelection()
//basically unitlistWindow right click, can now be called from elsewhere
//the repaint is refactored to reuse the already existing QLabel
{
    double sf = lockWindowTilesizeAct->isChecked() ? Settings->value(REG_LOCK_TILESIZE).toDouble() : Scale_factor;

    UnitListImageScaled = UnitListImage.scaled(UnitListImage.width() * sf, UnitListImage.height() * sf); //Restore original image
    selected_unit = 0xFF;     //No unit selected

    if (unitscrollArea_current_label)
        unitscrollArea_current_label->setPixmap(QPixmap::fromImage(UnitListImageScaled));  //update the image

    unit_name_text->setText("");
    unit_selection->update();
}


void unitlistwindow::mouseDoubleClickEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton)
        unit_selection->adjustSize();
}


//--------------------------------------------

void buildablewindow::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton)
    {
       if (buildablescrollArea->rect().contains(event->pos()))
       {
         int pos_x = buildablescrollArea->horizontalScrollBar()->value();
         int pos_y = buildablescrollArea->verticalScrollBar()->value();
         QRect widgetRect = buildablescrollArea->geometry();
         int fx = (event->pos().x()-widgetRect.left()+pos_x) / (Tilesize*Scale_factor); // Calc field position from mouse cords
         int fy = (event->pos().y()-widgetRect.top()+pos_y) / (Tilesize*Scale_factor);

         int unit = ((fy*10)+fx);     //Calc correct unit number
         buildable_unitname->setText(Unit_Name[unit]);

         if (unit <= Num_Units)
         {
            if (SHP.can_be_built[unit] == 0)
                SHP.can_be_built[unit] = 1;
            else
                SHP.can_be_built[unit] = 0;

         }

         int tx = 0;
         int ty = 0;

         for (int tc = 0; tc < Num_Units; tc++)
         {
            if (SHP.can_be_built[tc] == 0)
                Draw_Unit(tx*Tilesize,ty*Tilesize,tc*6,1,&BuildableImage);
            else
                Draw_Unit(tx*Tilesize,ty*Tilesize,tc*6,3,&BuildableImage);

            tx++;
            if (tx == 10)
            {
                tx = 0;
                ty++;
            }
        }

        BuildableImageScaled = BuildableImage.scaled(BuildableImage.width()*Scale_factor,BuildableImage.height()*Scale_factor); //Create a scaled version of it

        QLabel *label = new QLabel();
        label->setPixmap(QPixmap::fromImage( BuildableImageScaled));  //update the image

        buildablescrollArea->setWidget(label);
        buildable->update();                           //Update the window contents
        buildablescrollArea->horizontalScrollBar()->setValue(pos_x); //Reset the scrollArea to last position
        buildablescrollArea->verticalScrollBar()->setValue(pos_y);
        //window->set_changes_state(true); //Now there are unsaved changes
       }
    }
}


void buildingwindow::mousePressEvent(QMouseEvent *event)
{
    QRect widgetRect = Building_ScrollArea->geometry();

    if ((event->button() == Qt::LeftButton) && (selected_building != -1))
    {
       //Is the mouse on the ScrollArea (The list of units in the building?)
       if (widgetRect.contains(event->pos()))
       {
       // QPoint mouse_pos = Building_ScrollArea->mapFromParent(event->pos());

        int fx = ((event->pos().x()-widgetRect.left()) / Scale_factor) / Tilesize;

        Building_info[selected_building].Properties->Units[fx] = selected_unit/2;

        QPainter painter(&Building_Image);
        QPen pen;

        pen.setWidth(Tilesize);
        pen.setColor(Qt::black);
        painter.setPen(pen);
        painter.drawPoint((fx*Tilesize)+(Tilesize/2),(Tilesize/2));
        painter.end();

        Draw_Unit(fx*Tilesize, 0,(Building_info[selected_building].Properties->Units[fx] * 6) , Building_info[selected_building].Properties->Owner+1, &Building_Image);
        Building_Image_Scaled = Building_Image.scaled(Building_Image.width()*Scale_factor,Building_Image.height()*Scale_factor); //Create a scaled version of it

        for (int i = 0; i < 7; i++)
        {
            QPainter painter(&Building_Image_Scaled);
            pen.setWidth(1);
            pen.setColor(Qt::white);
            painter.setPen(pen);
            QRect R((i*Tilesize)*Scale_factor,0,((i*Tilesize)+Tilesize)*Scale_factor,Building_Image_Scaled.height()-1);
            painter.drawRect(R);
            painter.end();
        }


        QLabel *bitmaplabel = new QLabel();
        bitmaplabel->setPixmap(QPixmap::fromImage(Building_Image_Scaled));
        Building_ScrollArea->setWidget(bitmaplabel);
        Building_ScrollArea->update();

        building_window->update();
        //set_changes_state(true); //Now there are unsaved changes
       }
    }

    if ((event->button() == Qt::RightButton) && (selected_building != -1))
    {
       //Is the mouse on the ScrollArea (The list of units in the building?)
       if (widgetRect.contains(event->pos()))
       {
        int fx = ((event->pos().x()-widgetRect.left()) / Scale_factor) / Tilesize;

        Building_info[selected_building].Properties->Units[fx] = 0xFF;
        QPainter painter(&Building_Image);
        QPen pen;
        pen.setWidth(Tilesize);
        pen.setColor(Qt::black);
        painter.setPen(pen);
        painter.drawPoint((fx*Tilesize)+(Tilesize/2),(Tilesize/2));
        painter.end();

        Building_Image_Scaled = Building_Image.scaled(Building_Image.width()*Scale_factor,Building_Image.height()*Scale_factor); //Create a scaled version of it
        for (int i = 0; i < 7; i++)
        {
            QPainter painter(&Building_Image_Scaled);
            QPen pen;
            pen.setWidth(1);
            pen.setColor(Qt::white);
            painter.setPen(pen);
            QRect R((i*Tilesize)*Scale_factor,0,((i*Tilesize)+Tilesize)*Scale_factor,Building_Image_Scaled.height()-1);
            painter.drawRect(R);
            painter.end();
        }

        QLabel *bitmaplabel = new QLabel();
        bitmaplabel->setPixmap(QPixmap::fromImage(Building_Image_Scaled));
        Building_ScrollArea->setWidget(bitmaplabel);
        Building_ScrollArea->update();
        building_window->update();
        //set_changes_state(true); //Now there are unsaved changes
       }
    }
}


void update_replacewindow()
{
    tile_image1 = QImage(Tilesize,Tilesize, QImage::Format_RGB16); //Create a new QImage object
    tile_image1.fill(Qt::transparent);
    Draw_Part(0,0,r1,&tile_image1);
    tile_image2 = QImage(Tilesize,Tilesize, QImage::Format_RGB16); //Create a new QImage object
    tile_image2.fill(Qt::transparent);
    Draw_Part(0,0,r2,&tile_image2);

    tile_image1 = tile_image1.scaled(tile_image1.width()*Scale_factor,tile_image1.height()*Scale_factor); //scale it
    tile_image2 = tile_image2.scaled(tile_image2.width()*Scale_factor,tile_image2.height()*Scale_factor); //scale i
    Tile1->setPixmap(QPixmap::fromImage(tile_image1));
    Tile2->setPixmap(QPixmap::fromImage(tile_image2));
    replacedlg->update();
}

void replacewindow::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton)
    {
        QRect widgetRect = Tile1->geometry();

        if (widgetRect.contains(event->pos()))
        {
            r1 = selected_tile;
            update_replacewindow();
        }

        widgetRect = Tile2->geometry();

        if (widgetRect.contains(event->pos()))
        {
            r2 = selected_tile;
            update_replacewindow();
        }
    }
}


void replacewindow::closeEvent(QCloseEvent *event)
{
    if ((replace_accepted) && (r1 != r2))
    {
        int                      x,y,offset;
        field_info               fielddata;

        offset = 0;
        for (y = 0; y < Map.height; y++)
        {
            for (x = 0; x < Map.width; x++)
            {
                memcpy(&fielddata, Map.data + offset, sizeof(fielddata));
                if (fielddata.Part == r1)
                {
                    fielddata.Part = r2;
                    memcpy(Map.data + offset,&fielddata, sizeof(fielddata));
                }
                offset = offset + sizeof(Field);
            }
        }

        //set_changes_state(true);
        MapImage.fill(Qt::transparent);
        Draw_Map(); //redraw the mapimage
        MapImageScaled = MapImage.scaled(MapImage.width()*Scale_factor,MapImage.height()*Scale_factor); //Create a scaled version of it
        if (grid_enabled == true) ShowGrid();  //redraw the grid if enabled
        QLabel *imageLabel = new QLabel;     //Create a scroll area to display the map
        imageLabel->setPixmap(QPixmap::fromImage(MapImageScaled));
        scrollArea->setWidget(imageLabel);
    }

    event->accept();
}

//========================== Main program ================================

int main(int argc, char *argv[])
{
    QApplication    app(argc, argv);
    MainWindow      window;

    screenrect = app.primaryScreen()->geometry();   //Save screen geometry for window positioning

    if ((!Read_Config()) || (!Check_for_game_files()))  //Check for config and game files first
    {
       show_error("The path to HistoryLine is not set, or HistoryLine cannot be found.\n\nPlease reconfigure the path to the Game in Settings -> Game path");
       window.setPath_diag();
    } else {
      window.showgridAct->setChecked(Settings->value(REG_SHOW_GRID).toBool());

      if (Settings->value(REG_AUTOLOAD).toBool() == true) {
          window.Open_Map();
          window.autoloadAct->setChecked(true);
      }
      window.warningAct->setChecked(Settings->value(REG_SHOW_WARNINGS).toBool());
    }

    if (restoreWindowPosAct->isChecked()) {
        window.restoreWindowPos();
    } else {
        //as originally
        window.resize(screenrect.width() / 2, screenrect.height() / 2);
        window.move(screenrect.left(), screenrect.top());
    }
    window.show();

    return app.exec();
}

