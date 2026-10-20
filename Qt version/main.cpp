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
#include <QInputDialog>
#include <QScrollArea>
#include <QMouseEvent>
#include <QToolBar>
#include <QToolButton>
#include <QFileInfo>

#include "tilelist.h"
#include "unitlist.h"
#include "buildable.h"
#include "building.h"
#include "replace.h"
#include "mapinfounits.h"

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
QString          REG_AUTOGEN_PATH_TYPE = "autogenPathType";
QString          REG_AUTOGEN_PATH_STRAIGHT_DIST = "autogenPathStraightDist";

QString          Actual_Level = "";
int              Actual_Levelnum;

QImage           MapImage;                           // I use a QImage as Screenbuffer to draw the map
QImage           MapImageScaled;                     // Additional buffer for the scaled map image
QScrollArea      *scrollArea;

tilelistwindow   *tile_selection = NULL;            // and a widget for it (NOTE: was declared as QWidget)

QImage           UnitListImage;                     // Same for the Unit selection window
QImage           UnitListImageScaled;
QScrollArea      *unitscrollArea;
unitlistwindow   *unit_selection;                   //was decalred as type QWidget thus no 'custom methods' could be called

QRect            screenrect;                        // A QRect to save the screen geometry and position windows accordingly.
QLabel           *unit_name_text;                   // Text label to display unit name on mouse over

/*
 dadk
 this ended up being the solution for the 'right click problem'.
 In the original code from GitHub, any right click on child windows caused the program to crash,
 the line was unitlistScrollArea->setWidget() (also tilelist) even though the code is exact the same as in left click.
 It works if the current unitlist / tilelist scrollarea references are stored (the inserted QLabels)
 and we only refresh the existing QLabel, not creating and inserting a new one
*/
QLabel           *scrollArea_current_label = NULL;
QLabel           *unitscrollArea_current_label = NULL;

QAction          *lockWindowTilesizeAct; //!?
QAction          *restoreWindowPosAct;
QAction          *hideNativeMapsAct;
QAction          *resetSettingsAct;

QMenu            *menuRecentFiles;
//
bool             DRAG_UNIT_START = false; //user are about to drag a unit from one position to another
bool             DRAG_UNIT_PROGRESS = false;
int              drag_unit_number;
QPoint           drag_start;
QPoint           drag_end;

bool             SELECTION_MODE = false;
bool             SELECTING_TILES = false; //user are selecting an area on the map
QPoint           selection_start;
QPoint           selection_end;

bool             SELECTING_PATH = false;
int              selecting_path_target = 0; //1=dirtroad, 2=road, 3=railroad, 4=river
QPoint           selecting_path_last_pos;
QImage           autogenerateMapImage;

bool             DRAG_SELECTION_START = false; //user are about to drag the selection from one position to another
bool             DRAG_SELECTION_PROGRESS = false;
QPoint           selection_drag_from;

//perhaps this could be in some kind of struct or class?
double           Scale_factor = 2.0;                // Default scaling factor for the old VGA bitmaps is 2x
double           Scale_factor_max = 5;
double           Scale_factor_min = 0.5;
double           Scale_factor_locked = 0;           // dadk, If above 0, lock child window tile sizes to that number

unsigned char    selected_tile = 0xFF;              // Nu tile selected by default
unsigned char    selected_unit = 0xFF;              // No unit is selected by default
bool             Res_loaded = false;                // to check if bitmaps have already been loaded into memory
bool             summer = true;                     // set summer as default season for map ressources
bool             changes = false;
bool             already_saved = false;
bool             grid_enabled = false;
bool             coords_enabled = false;
bool             Player2 = true;
bool             Ocean = false;
bool             Update_Ressources;
QPoint           selected_pos;

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
#include "utils.h"
#include "unit_info.h"
#include "other.h"
#include "hexPath.h"
#include "selection.h"
#include "dragdrop.h"

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

bool isNativeMap(QString level, int code)
//test if the level code is a 'native' HL map
{
    QStringList native;
    native << "PULSE" << "CIVIL" <<  "MOUSE" <<  "VENOM" <<  "NOISE" <<  "RIGHT" <<  "ORKAN" <<  "FRONT" <<  "RATIO" <<  "PARTS" <<  "PLANE" <<  "FLAME" <<  "GOTHA" <<  "BALON" <<  "PAUSE" <<  "ELITE" <<  "INFRA" <<  "HILLS" <<  "COBRA" <<  "ATLAS" <<  "AMPER" <<  "RHEIN" <<  "CANDL" <<  "STERN" <<  "BATLE" <<  "GOOSE" <<  "SPORT" <<  "BIMBO" <<  "TEMPO" <<  "BARON" <<  "BUMMM" <<  "LEVEL" <<  "TOXIN" <<  "PRINC" <<  "CLEAN" <<  "XENON" <<  "SIGNS" <<  "HOUSE" <<  "SIGMA" <<  "SEVEN" <<  "ZOMBI" <<  "MOVES" <<  "BLADE" <<  "ZORRO" <<  "STONE" <<  "MOSEL" <<  "ORDER" <<  "SODOM" <<  "TRACK" <<  "HUSAR" <<  "BEAST" <<  "PLATE" <<  "LIGHT" <<  "SCROL" <<  "VIRUS" <<  "BISON" <<  "DRUCK" <<  "TROLL" <<  "UBOOT" <<  "DROID" <<  "GRAND" <<  "ROYAL" <<  "WATER" <<  "SKILL" <<  "SKULL" <<  "AUDIO" <<  "SPELL" <<  "CAMEL" <<  "FLAGS" <<  "STORY" <<  "SCOUT" <<  "GREEN";
    return native.contains(level) && code < 72;
}

QStringList get_filtered_level_codes()
//return a list of level codes, if 'Hide Native Maps' is checked, return only 'custom' maps
{
    QStringList levels;
    int i;
    if (hideNativeMapsAct->isChecked()) {
        for (i=0; i < Levelcode.Codelist.count(); i++) {
            if (isNativeMap(Levelcode.Codelist[i], i) == false) {
                levels << Levelcode.Codelist[i];
            }
        }
   } else {
        levels = Levelcode.Codelist;
   }
   return levels;
}


QPoint mouse_to_map(QPoint event_pos)
//return map coordinates from mouse position
{
    int pos_x = scrollArea->horizontalScrollBar()->value();
    int pos_y = scrollArea->verticalScrollBar()->value();
    QPoint mouse_pos = scrollArea->mapFromParent(event_pos);
    mouse_pos = mouse_pos + QPoint(pos_x, pos_y);
    return mouseToFieldPos(mouse_pos);
}

//=================== Main Window  ==========================

MainWindow::MainWindow()
//Creates Main Window and adds a scroll area to display maps
{
    createActions();
    createMenus();
    createToolbar();
    update_window_title();

    //initialize images and the scroll area
    MapImage = QImage();
    MapImageScaled = QImage();
    scrollArea = new(QScrollArea);
    scrollArea->setBackgroundRole(QPalette::Dark);
    scrollArea->setVisible(true);
    scrollArea->setMouseTracking(true);
    setCentralWidget(scrollArea);
}

void MainWindow::closeEvent(QCloseEvent *event)
//Own closeEvent handler, primary to make sure allocated memory will be properly released
{
    if ((Map.loaded == true) && (changes == true))
    {
        int answer = ask_cancelable_question("Save changes", "There are unsaved changes to the map. Do you want to save them before quitting?", this);
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

void MainWindow::update_changes_state()
//update gui to reflect changes, needed as slot
{
    tb_save_changes->setEnabled(changes);
    saveAct->setEnabled(changes);
    update_window_title();
}

void MainWindow::set_changes_state(bool state)
//set new changes state if differentfor menu, toolbar and title
{
    if (changes == state) return;
    changes = state;
    update_changes_state();
}

void MainWindow::update_deselect_button()
//set deselect button checked if there is no selected_tile, selected_unit, i.e. in "browse mode"
{
    tb_deselect->setChecked(selected_tile == 0xFF && selected_unit == 0xFF);
}

void MainWindow::Create_Tileselection_window()
{
    QRect tg;
    bool reload = false;

    if (tile_selection != NULL) {
        reload = true;
        tg = tile_selection->geometry();
        tile_selection->close();
    } else {
        tg = Settings->value(REG_TILELIST_GEO).toRect();
    }

    tile_selection = new tilelistwindow();
    tilelistwindow_populate(tile_selection);

    if (!tg.isNull()) {
        if (reload) {
            tile_selection->move(tg.topLeft());
        } else {
            tile_selection->setGeometry(tg);
        }
    }

    tile_selection->show();
    connect(tile_selection, SIGNAL(SelectionChanged()), this, SLOT(update_deselect_button()));
}

void MainWindow::init_selecting_path(int target = 0)
{
    if (target == 0) {
        scrollArea->setCursor(QCursor(Qt::ArrowCursor));
        SELECTING_PATH = false;
        selecting_path_target = target;
        Paint_Map(true);
        tb_autogen_dirtroad->setIcon( get_part_icon(147));
        tb_autogen_road->setIcon(get_part_icon(128));
        tb_autogen_railroad->setIcon(get_part_icon(87));
        tb_autogen_river->setIcon(get_part_icon(37));
        tb_autogen_trench->setIcon(get_part_icon(168));
        tb_autogen_coastRightTop->setIcon(get_part_icon(30));
        tb_autogen_coastLeftBottom->setIcon(get_part_icon(31));
        return;
    }

    //just need to refresh the cursor
    if (target == -1) {
        scrollArea->setCursor(get_autogenerate_path_cursor(selecting_path_target));
        return;

    }

    if (!selection_start.isNull()) {
        tb_autogen_fill->setEnabled(false);
        selection_start = QPoint();
        selection_end = QPoint();
        Paint_Map();
    }
    SELECTING_PATH = true;
    selecting_path_target = target;

    tb_autogen_dirtroad->setIcon( get_part_icon(147, true, true));
    tb_autogen_road->setIcon(get_part_icon(128, true, true));
    tb_autogen_railroad->setIcon(get_part_icon(87, true, true));
    tb_autogen_river->setIcon(get_part_icon(37, true, true));
    tb_autogen_trench->setIcon(get_part_icon(168, true, true));
    tb_autogen_coastRightTop->setIcon(get_part_icon(30, true, true));
    tb_autogen_coastLeftBottom->setIcon(get_part_icon(31, true, true));

    if (target == 1) tb_autogen_dirtroad->setIcon(QPixmap::fromImage(get_autogenerate_path_icon(target, true)));
    if (target == 2) tb_autogen_road->setIcon(QPixmap::fromImage(get_autogenerate_path_icon(target, true)));
    if (target == 3) tb_autogen_railroad->setIcon(QPixmap::fromImage(get_autogenerate_path_icon(target, true)));
    if (target == 4) tb_autogen_river->setIcon(QPixmap::fromImage(get_autogenerate_path_icon(target, true)));
    if (target == 5) tb_autogen_trench->setIcon(QPixmap::fromImage(get_autogenerate_path_icon(target, true)));
    if (target == 6) tb_autogen_coastRightTop->setIcon(QPixmap::fromImage(get_autogenerate_path_icon(target, true)));
    if (target == 7) tb_autogen_coastLeftBottom->setIcon(QPixmap::fromImage(get_autogenerate_path_icon(target, true)));

    scrollArea->setCursor(get_autogenerate_path_cursor(selecting_path_target));
}

void MainWindow::updateToolbarIcons()
{
    //if no map is loaded yet
    if (!Res_loaded) {
        //if GameDir is not set,  we are not able to show HL graphics as icons
        //fallback to the VERBOTEN glyph
        if (Load_Ressources() != 0) {
            show_error("Failed to load bitmaps from the game!", this);
            tb_autogen_dirtroad->setIcon(QIcon(":/images/VERBOTEN_color.PNG"));
            tb_autogen_road->setIcon(QIcon(":/images/VERBOTEN_color.PNG"));
            tb_autogen_railroad->setIcon(QIcon(":/images/VERBOTEN_color.PNG"));
            tb_autogen_river->setIcon(QIcon(":/images/VERBOTEN_color.PNG"));
            tb_autogen_trench->setIcon(QIcon(":/images/VERBOTEN_color.PNG"));
            tb_autogen_coastRightTop->setIcon(QIcon(":/images/VERBOTEN_color.PNG"));
            tb_autogen_coastLeftBottom->setIcon(QIcon(":/images/VERBOTEN_color.PNG"));
            tb_tile_window->setIcon(QIcon(":/images/VERBOTEN_color.PNG"));
            tb_unit_window->setIcon(QIcon(":/images/VERBOTEN_color.PNG"));
            updateActions();
            return;
        } else {
            updateActions();
            update_window_title();
        }
    }

    init_selecting_path(); //will refresh autogen buttons with dynamic icons
    autogenLightForestAct->setIcon( get_part_icon(54, true));
    autogenDenseForestAct->setIcon( get_part_icon(57, true));
    autogenResidentalAreaAct->setIcon( get_part_icon(50));
    autogenBattlefieldAct->setIcon( get_part_icon(84));
    autogenPlainsAct->setIcon( get_part_icon(78));
    autogenWastelandAct->setIcon( get_part_icon(75));
    autogenLakeAct->setIcon( get_part_icon(34));
    autogenIslandAct->setIcon(QIcon(":/images/island.png"));

    if (tb_tile_window->isChecked()) {
        tb_tile_window->setIcon( get_part_icon(54));
    } else {
        tb_tile_window->setIcon( get_part_icon(54, false, true));
    }

    if (tb_unit_window->isChecked()) {
       tb_unit_window->setIcon( get_unit_icon(22, 3));
    } else {
       tb_unit_window->setIcon( get_unit_icon(22, 3, true));
    }
}

void MainWindow::updateActions()
{
    newAct->setEnabled(Res_loaded);
    openAct->setEnabled(Res_loaded);
    openbyCodeAct->setEnabled(Res_loaded);
    removefromgameAct->setEnabled(Res_loaded);

    saveAct->setEnabled(Map.loaded);
    saveasAct->setEnabled(Map.loaded);
    saveImageAct->setEnabled(Map.loaded);
    addtogameAct->setEnabled(Map.loaded);
    closeAct->setEnabled(Map.loaded);

    showtilewindowAct->setEnabled(Map.loaded);
    showunitwindowAct->setEnabled(Map.loaded);

    mapresizeAct->setEnabled(Map.loaded);
    replaceAct->setEnabled(Map.loaded);
    maptypeAct->setEnabled(Map.loaded);
    changeseasonAct->setEnabled(Map.loaded);
    showgridAct->setEnabled(Map.loaded);
    showcoordsAct->setEnabled(Map.loaded);
    buildableunitsAct->setEnabled(Map.loaded);
    mapInfoGeneralAct->setEnabled(Map.loaded);
    mapInfoUnitsAct->setEnabled(Map.loaded);
    setScaleFactorAct->setEnabled(Map.loaded);
    warningAct->setEnabled(Map.loaded);
    hideNativeMapsAct->setEnabled(Map.loaded);

    tb_deselect->setEnabled(Map.loaded);
    tb_move_tl->setEnabled(Map.loaded);
    tb_move_tr->setEnabled(Map.loaded);
    tb_move_bl->setEnabled(Map.loaded);
    tb_move_br->setEnabled(Map.loaded);
    tb_zoom_in->setEnabled(Map.loaded);
    tb_zoom_out->setEnabled(Map.loaded);
    tb_map_info->setEnabled(Map.loaded);
    tb_replace_tile->setEnabled(Map.loaded);

    tb_tile_window->setEnabled(Map.loaded);
    tb_unit_window->setEnabled(Map.loaded);

    tb_selection_mode->setEnabled(Map.loaded);

    tb_autogen_dirtroad->setEnabled(Map.loaded);
    tb_autogen_road->setEnabled(Map.loaded);
    tb_autogen_railroad->setEnabled(Map.loaded);
    tb_autogen_river->setEnabled(Map.loaded);
    tb_autogen_trench->setEnabled(Map.loaded);
    tb_autogen_coastRightTop->setEnabled(Map.loaded);
    tb_autogen_coastLeftBottom->setEnabled(Map.loaded);
    tb_autogen_options->setEnabled(Map.loaded);

    lockWindowTilesizeAct->setEnabled(Map.loaded);
    autoloadAct->setChecked(Settings->value(REG_AUTOLOAD).toBool());
    restoreWindowPosAct->setChecked(Settings->value(REG_RESTORE_WINDOWS).toBool());
    showgridAct->setChecked(Settings->value(REG_SHOW_GRID).toBool());
    showcoordsAct->setChecked(Settings->value(REG_SHOW_COORDS).toBool());
    warningAct->setChecked(Settings->value(REG_SHOW_WARNINGS).toBool());

    if (Map.loaded) {
        update_deselect_button();
        if (Scale_factor <= Scale_factor_min) tb_zoom_out->setEnabled(false);
        if (Scale_factor >= Scale_factor_max) tb_zoom_in->setEnabled(false);
    }
}

void MainWindow::showEvent(QShowEvent *event)
{
    updateToolbarIcons();
    if (!Settings->contains(REG_AUTOLOAD)) {
        //we are probably on first run
        //always suggest autoload as default option
        Settings->setValue(REG_AUTOLOAD, true);
    }
}

void MainWindow::update_window_title()
//Unified setWindowTitle, replace the various different setWindowTitle
{
    QString t;
    t = Title + " " + Author + " " + Version; //dadk, have skipped the +" - Version: "+

    //only maps loaded from outside the GameDir will show full path, otherwise xxxx.FIN
    if (!Map_file.isEmpty()) {
        QString f = Map_file;
        t = t + "  ::  " + f.replace(GameDir + "/MAP/", "");
    } else {
        if (Map.loaded) {
            t = t + "  ::  (New Map)";
        } else {
            if (Res_loaded) {
                t = t + "  ::  (No Map)";
            } else {
                t = t + "  ::  (Resources not Found!)";
            }
        }
    }

    if (!Actual_Level.isEmpty()) {
        QString l = Actual_Level;
        t = t + " [" + l.toUpper() + "]";
    }

    if (changes) t = t + " *";
    setWindowTitle(t);
}

void MainWindow::Paint_Map(bool refresh)
{
    if (!Map.loaded) return;

    int pos_x = scrollArea->horizontalScrollBar()->value();
    int pos_y = scrollArea->verticalScrollBar()->value();

    if (refresh) {
       scrollArea_current_label = NULL;
       selected_pos = QPoint();
    }

    if (scrollArea_current_label == NULL) //first time or refresh
    {
        if (!MapImage.isNull()) MapImage = QImage(); //Release mem for the last used image
        if (!MapImageScaled.isNull()) MapImageScaled = QImage(); //Release mem for the last used scaled image

        MapImage = QImage(((Map.width/2) * Tilesize) + (((Map.width/2)-1) * Tileshift), ((Map.height-1) * Tilesize) + (Tilesize/2), QImage::Format_RGB16); //Create a new QImage object for the map image
        MapImage.fill(Qt::transparent);

        Draw_Map(); //and draw the map to it

        MapImageScaled = MapImage.scaled(MapImage.width()*Scale_factor,MapImage.height()*Scale_factor); //Create a scaled version of it
        if (showgridAct->isChecked()) ShowGrid();
        if (showcoordsAct->isChecked()) ShowCoords();

        if (!selected_pos.isNull())
            Draw_Hexagon(selected_pos.x(), selected_pos.y(), QPen(Qt::red, 1), &MapImageScaled, true, true);

        if (!selection_start.isNull()) {
            if (SELECTING_PATH) {
                Paint_Path_Selection(&MapImageScaled);
            } else {
                Paint_Selection(&MapImageScaled);
            }
        }

        QLabel *imageLabel = new QLabel;
        imageLabel->setPixmap(QPixmap::fromImage(MapImageScaled));
        imageLabel->setAttribute(Qt::WA_TransparentForMouseEvents); //so we can respond to mouse events on the map, not just click
        scrollArea_current_label = imageLabel;

        if (scrollArea == NULL) scrollArea = new(QScrollArea);
        scrollArea->setWidget(imageLabel);
   } else {
        MapImageScaled = MapImage.scaled(MapImage.width() * Scale_factor, MapImage.height() * Scale_factor);
        if (showgridAct->isChecked()) ShowGrid();
        if (showcoordsAct->isChecked()) ShowCoords();

        if (!selected_pos.isNull())
            Draw_Hexagon(selected_pos.x(), selected_pos.y(), QPen(Qt::red, 1), &MapImageScaled, true, true);

        if (!selection_start.isNull()) {
            if (SELECTING_PATH) {
                Paint_Path_Selection(&MapImageScaled);
            } else {
                Paint_Selection(&MapImageScaled);
            }
        }

        QLabel *imageLabel = new QLabel;
        imageLabel->setPixmap(QPixmap::fromImage(MapImageScaled));
        imageLabel->setAttribute(Qt::WA_TransparentForMouseEvents);
        scrollArea_current_label = imageLabel;
        scrollArea->setWidget(scrollArea_current_label);
    }
    scrollArea->horizontalScrollBar()->setValue(pos_x); //Reset the scrollArea to last position
    scrollArea->verticalScrollBar()->setValue(pos_y);
}

void MainWindow::update_map()
{
    Paint_Map(true);
}

void MainWindow::mouseDoubleClickEvent( QMouseEvent *event )
{
    //Double Click to delete unit on current field
    if (event->button() == Qt::LeftButton)
    {
        QPoint h = mouse_to_map(event->pos());
        if ((h.x() < (Map.width-1)) && (h.y() < (Map.height-1)))  //Is the field on the map?
        {
            int field_pos = (h.y()*Map.width)+h.x();

            if (unit_is_transporter(Map.data[(field_pos*2)+1])) {
                 Map.data[(field_pos*2)+1] = 0xFF;
                 Update_building_record_from_map(); //Update building data record
            } else {
                Map.data[(field_pos*2)+1] = 0xFF;
            }

            drag_unit_number = -1; //a drag session might be initiated
            Redraw_Field(h.x(), h.y(), selected_tile, 0xFF);
            Paint_Map();
            set_changes_state(true); //There are unsaved changes now
        }
    }
    if (event->button() == Qt::RightButton)
    {
        qDebug() << "right doubleclick";
    }
}

void MainWindow::keyPressEvent(QKeyEvent* event)
{
    if (event->key() == Qt::Key_Escape) {
        if (DRAG_SELECTION_PROGRESS) {
            scrollArea->setCursor(QCursor(Qt::ArrowCursor));
            DRAG_SELECTION_PROGRESS = false;
        }
        if (SELECTING_PATH) {
            selection_start = QPoint();
            selection_end = QPoint();
            Paint_Map(true);
            init_selecting_path(-1); //refresh current cursor
        }
    }
    if (event->key() == Qt::Key_Plus) {
        zoom(true);
    }
    if (event->key() == Qt::Key_Minus) {
        zoom(false);
    }
}

void MainWindow::mousePressEvent(QMouseEvent *event)
//Handle mouse events on the main window
{
    if (Map.loaded != true) return;

    QPoint h = mouse_to_map(event->pos());

    if (event->button() == Qt::LeftButton)
    {
        if ((h.x() < (Map.width-1)) && (h.y() < (Map.height-1)))  //Is the field on the map?
        {
            int field_pos = (h.y() * Map.width) + h.x();
            unsigned char old_tile = Map.data[field_pos*2];
            unsigned char old_unit = Map.data[(field_pos*2)+1];

            if (SELECTING_PATH) {
                if (selection_start.isNull()) {
                    selection_start = h;
                    selecting_path_last_pos = h;
                    autogenerate_Paint_Map(true);
                    scrollArea->setCursor(Qt::WhatsThisCursor);
                    return;
                }
            }

            if (point_in_selection(h)) {
                if (selected_tile == 0 || (selected_tile > 0x14 && selected_tile != 0xFF)) {
                    Fill_Selection(selected_tile);
                } else if (selected_tile == 0xFF) {
                    DRAG_SELECTION_START = true;
                    selection_drag_from = h;
                }
            }

            if (selected_tile != 0xFF && (Map.data[field_pos*2] != selected_tile))
            {
                //warn overwriting building tiles
                int answer = 0;
                if (tile_is_building(old_tile) && Settings->value(REG_SHOW_WARNINGS).toBool())
                {
                    answer = ask_cancelable_question("Replace tile?", "Overwriting building tiles is not recommended; Overwrite anyway?", this);
                    if (answer == QMessageBox::Cancel)
                        return;
                }

                if (answer == 0 || answer == QMessageBox::Yes)
                {
                    //warn placing building tile
                    if (((selected_tile >= 0x12) && (selected_tile <= 0x14)) ||
                        ((selected_tile >= 0x09) && (selected_tile <= 0x0B)))
                    {
                        answer = ask_cancelable_question("Place building tile?", "Building parts of factories and depots that do not have an associated entrance and are not arranged as intended can still be opened in the game and then contain random garbage data; Continue anyway?", this);
                        if (answer == QMessageBox::Cancel)
                            return;
                    }
                    if (answer == 0 || answer == QMessageBox::Yes)
                    {
                        Map.data[field_pos*2] = (unsigned char) selected_tile;
                        set_changes_state(true); //There are unsaved changes now
                    }
                }
            }

            //if click is on a unit, and the unit is not about to be replaced, begin drag session
            if ((Map.data[(field_pos*2)+1] != 0xFF && selected_unit == Map.data[(field_pos*2)+1]) ||
                (Map.data[(field_pos*2)+1] != 0xFF && selected_unit == 0xFF))
            {
                DRAG_UNIT_START = true;
                drag_unit_number = Map.data[(field_pos*2)+1];
                drag_start = h;

            } else {
                if (!point_in_selection(h) && SELECTION_MODE) {
                    tb_autogen_fill->setEnabled(false);
                    SELECTING_TILES = true; //begin select session
                    selection_end = h;
                    selection_start = h;
                }
            }

            if ((selected_unit != 0xFF) && (Map.data[(field_pos*2)+1] != selected_unit))
            {
                if (!tile_is_building(old_tile))
                {
                    Map.data[(field_pos*2)+1] = (unsigned char) selected_unit;
                    set_changes_state(true); //There are unsaved changes now
                }

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
                        show_warning("You have placed a unit on terrain where the game does not provide for it. This can lead to glitches and errors when playing the map in game.", this);
                    }
                }
            }

            if ((tile_is_building(old_tile)) ||
                (tile_is_building(selected_tile)) ||
                (unit_is_transporter(old_unit)) ||
                (unit_is_transporter(selected_unit))) {
                Update_building_record_from_map(); //...Correct the building data record in memory
            }

            selected_pos = h;
            Redraw_Field(h.x(), h.y(), Map.data[(field_pos*2)], Map.data[(field_pos*2)+1]);
            Paint_Map();
        }
    }

    if (event->button() == Qt::RightButton)
    {
        if ((h.x() > (Map.width-1)) || (h.y() > (Map.height-1)))  //Is the field on the map?
            return;

        int field_pos = (h.y() * Map.width) + h.x();

        if (SELECTING_PATH) {
            init_selecting_path();
        }
        //reset selection
        selection_start = QPoint();
        selection_end = QPoint();
        selected_pos = QPoint();
        tb_autogen_fill->setEnabled(false);

        //place mountain?
        if (selected_tile >= 0x43 && selected_tile <= 0x4A) //0x44 .. 0x4A
        {
            place_mountain_on_map(h, selected_tile);
        }

        if ((tile_is_building(Map.data[field_pos*2])) ||
            (unit_is_transporter(Map.data[(field_pos*2)+1])))
        {
            buildingwindow *building_window = new buildingwindow();
            building_window->selected_building = Get_Building_by_field(field_pos);
            connect(building_window, SIGNAL(BuildingChanged()), this, SLOT(update_map()));
            connect(building_window, SIGNAL(BuildingChanged()), this, SLOT(update_changes_state()));
            buildingwindow_populate(building_window);
        } else {
            if  (((selected_tile == 0x01) || (selected_tile == 0x02)) ||
                 ((selected_tile >= 0x0C) && (selected_tile <= 0x11)) ||
                (selected_tile == 0x15))
            {
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
                Update_building_record_from_map();  // ...Correct the building data record in memory
                set_changes_state(true);
            }
        }
        Paint_Map();
    }
}

void MainWindow::mouseReleaseEvent(QMouseEvent *event)
{
    QPoint h = mouse_to_map(event->pos());

    if (SELECTING_TILES) {
        SELECTING_TILES = false;
        tb_autogen_fill->setEnabled(selection_start != selection_end);
    }
    if (DRAG_SELECTION_PROGRESS) {
        scrollArea->setCursor(QCursor(Qt::ArrowCursor));
        if (point_in_selection(h)) {
            DRAG_SELECTION_PROGRESS = false;
            selection_drag_from = QPoint();
            drag_start = QPoint();
            drag_end = QPoint();
        } else {
            execute_selection_dragdrop(h);
            set_changes_state(true);
            Paint_Map();
        }
    }
    if (DRAG_UNIT_PROGRESS) {
        scrollArea->setCursor(QCursor(Qt::ArrowCursor));
        if (execute_unit_dragdrop()) {
            Paint_Map(DRAGGING_LARGE_SHIP);
            set_changes_state(true);
        }
        drag_start = QPoint();
        drag_end = QPoint();
        drag_unit_number = 0xFF;
        DRAG_UNIT_START = false;
        DRAG_UNIT_PROGRESS = false;
    }
    if (SELECTING_PATH) {
        if (!selection_start.isNull() && !points_equal(selection_start, selecting_path_last_pos)) {
            autogenerate_path(selection_start, selecting_path_last_pos, selecting_path_target, false);
            selection_start = QPoint();
            selection_end = QPoint();
            selecting_path_last_pos = QPoint();
            Paint_Map(true);
            set_changes_state(true);
            scrollArea->setCursor(get_autogenerate_path_cursor(selecting_path_target));
        }
    }
}

void MainWindow::mouseMoveEvent(QMouseEvent *event)
{
    QPoint h = mouse_to_map(event->pos());

    if (DRAG_UNIT_START && !DRAG_UNIT_PROGRESS) {
        scrollArea->setCursor( get_dragdrop_unit_cursor() );
        DRAG_UNIT_START = false;
        DRAG_UNIT_PROGRESS = true;
    } else if (DRAG_SELECTION_START) {
        scrollArea->setCursor( get_dragdrop_selection_cursor() );
        DRAG_SELECTION_PROGRESS = true;
        DRAG_SELECTION_START = false;
    } else if (DRAG_UNIT_PROGRESS) {
        drag_end = h;
    } else if (SELECTING_TILES) {
        selection_end = h;
        if (selection_start != selection_end) {
            Paint_Map();
        }
    }

    if (SELECTING_PATH) {
        if (!selection_start.isNull()) {
            if (!points_equal(selection_start, h)) {
                selecting_path_last_pos = h;
                autogenerate_Paint_Map(false);
            }
        }
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
    if (!Res_loaded) {
        if (Load_Ressources() != 0) {
            show_error("Failed to load bitmaps from the game!", this);
            return;
        }
    }

    if ((Map.loaded == true) && (changes == true)) {
        int answer = ask_cancelable_question("New map", "There are unsaved changes, do you want to save them?", this);

        if (answer == QMessageBox::Yes)
            Save();

        if (answer == QMessageBox::Cancel)
            return;
    }

    if (Map.data != NULL) free(Map.data);

    Map.width = 16; //Set default width and height
    Map.height = 16;
    Map.data_size = ((Map.width+1) * (Map.height+1)) * 2;

    if ((Map.data = (unsigned char*)malloc(Map.data_size)) == NULL) {
        show_error("Memory allocation error!", this);
        return;
    } else {
        unsigned char tile;
        if (ask_question("Do you want to create an Ocean map?") == true) {
            tile = 0x30;
            Ocean = true;
        } else {
            tile = 0x00;
            Ocean = false;
        }

        int x,y,o;
        o = 0;
        for (y = 1; y <= Map.height; y++) {
          for (x = 1; x <= Map.width; x++) {
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
        Map_file.clear();

        Paint_Map(true);
        updateActions();

        Building_stat.num_HQ = 0;
        Building_stat.num_F = 0;
        Building_stat.num_D = 0;
        Building_stat.num_T = 0;
        Building_stat.num_buildings = 0;
        if (Building_info != NULL) free(Building_info);
        if (SHP.buildings != NULL) free(SHP.buildings);
        Building_info = (Building_data_ext*) malloc(sizeof(Building_data_ext));
        memset(SHP.can_be_built,0,sizeof(SHP.can_be_built));

        //set bunker and large ships unbuildable as default
        SHP.can_be_built[13] = 1;
        SHP.can_be_built[29] = 1; //submarine
        SHP.can_be_built[30] = 1;
        SHP.can_be_built[32] = 1; //destroyer
        SHP.can_be_built[33] = 1;
        SHP.can_be_built[34] = 1; //battleship
        SHP.can_be_built[35] = 1;

        SHP.buildings = 0;
        Num_building_entries = 0;

        set_changes_state(true);
        already_saved = false;
        Actual_Level = "";
        Actual_Levelnum = -1;
        update_window_title();
        //always start as single player game
        Player2 = false;
        maptypeAct->setChecked(false);
    }
}


void MainWindow::Open_Map()
{
    QString      C_Filename1;
    QString      C_Filename2;
    TMP_Rec      load_res;

    if (!Map_file.isEmpty() && !Map_file.isNull()) {
        if (!Res_loaded) {
          if (Load_Ressources() != 0) {
              show_error("Failed to load bitmaps from the game!", this);
              return;
          }
        }

        load_res = Load_Map();
        if (load_res.summer == -100)
            return; //map file was not found

        if (load_res.summer > 0) {
            summer = true;
            C_Filename1 = get_path(Partlib_S_name);
            C_Filename2 = get_path(Partdat_S_name);
        } else {
            summer = false;
            C_Filename1 = get_path(Partlib_W_name);
            C_Filename2 = get_path(Partdat_W_name);
        }
        Load_Part_files(C_Filename1.toStdString().data(),C_Filename2.toStdString().data()); //Load correct season graphics

        if (load_res.twoplayer > 0) {
            Player2 = true;
            maptypeAct->setChecked(true);
        } else {
            Player2 = false;
            maptypeAct->setChecked(false);
        }

        Ocean = load_res.ocean == 1;

        Paint_Map(true);

        //recreate tile window to reflect update summer / winter
        Create_Tileselection_window();

        //only recreate unit_selection if it not exists, and that is upon startup
        if (!unit_selection) Create_Unitselection_window();

        set_changes_state(false);
        already_saved = true;
        Check_used_tiles();
        update_window_title();
        updateActions();

        if (autoloadAct->isChecked() == true) {
            // The global Map_file contains the full path, so maps outside /MAP can be autoloaded as well
            Settings->setValue(REG_RECENT_MAP, Map_file);
        }
        updateRecentFiles();
    }
}

void MainWindow::open_diag()
{
    if ((Map.loaded == true) && (changes == true))
    {
        int answer = ask_cancelable_question("Load map", "There are unsaved changes to the map. Do you want to save them?", this);

        if (answer == QMessageBox::Cancel)
            return;

        if (answer == QMessageBox::Yes)
            Save();
    }

    //hide child windows if visible
    if (tile_selection && showtilewindowAct->isChecked() == true) tile_selection->setVisible(false);
    if (unit_selection && showunitwindowAct->isChecked() == true) unit_selection->setVisible(false);

    //if Map_file already is set, open dialog from last path
    QFileInfo fi(Map_file);
    QString dir = !Map_file.isEmpty() ? fi.path() : MapDir;

    Map_file = open_file_dialog("Open map", "HL map files (*.fin *.FIN)", dir, this);

    if (tile_selection && showtilewindowAct->isChecked() == true) tile_selection->setVisible(true);
    if (unit_selection && showunitwindowAct->isChecked() == true) unit_selection->setVisible(true);

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
            show_error("Failed to load bitmaps from the game!", this);
            return;
        }
    }

    QString levelcode = get_item_dialog("Open map by levelcode:", "Please select a map:", get_filtered_level_codes(), "", this);

    if (!levelcode.isEmpty())
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
        if (Map_file.isNull()) {
            saveas_diag();
            return;
        }
        Save();
        set_changes_state(false);
    } else {
        show_error("There's nothing I could save.... Why don't you load a map first or create a new one?", this);
    }
}

void MainWindow::saveas_diag()
{
    if (Map.loaded == true)
    {
        //hide child windows if visible
        if (tile_selection->isVisible() && showtilewindowAct->isChecked() == true) tile_selection->setVisible(false);
        if (unit_selection->isVisible() && showunitwindowAct->isChecked() == true) unit_selection->setVisible(false);

        QString new_file;
        new_file = QFileDialog::getSaveFileName(this ,"Save History Line 1914-1918 map file", MapDir, "HL map files (*.FIN *.fin)");

        if (tile_selection && showtilewindowAct->isChecked() == true) tile_selection->setVisible(true);
        if (unit_selection && showunitwindowAct->isChecked() == true) unit_selection->setVisible(true);

        if (!new_file.isEmpty() && !new_file.isNull()) {
            Map_file = new_file;
            if (Save()) {
                already_saved = false;
                set_changes_state(false);
                update_window_title();
                Settings->setValue(REG_RECENT_MAP, Map_file);
                Check_used_tiles();
                updateRecentFiles();
            }
        }
    }
}

void MainWindow::saveimage_diag()
{
    QString fileName = QFileDialog::getSaveFileName(this, tr("Save Image File"), QString(), tr("Images (*.png)"));
    if (!fileName.isEmpty()) {
        //ensure .png extension
        if (!fileName.contains(".png", Qt::CaseInsensitive)) fileName = fileName + ".png";
        if (MapImage.save(fileName) != true) {
            show_error("Save error", "Something went wrong, map image could not be saved", this);
        } else {
            show_info("Image saved", "Image of the map succesfully saved as " + fileName, this);
        }
    }
}

void MainWindow::close_diag()
//close map and reset actions, controls
{
    if ((Map.loaded == true) && (changes == true)) {
        int answer = ask_cancelable_question("Close map", "There are unsaved changes, do you want to save them?", this);
        if (answer == QMessageBox::Yes) Save();
        if (answer == QMessageBox::Cancel) return;
    }

    tile_selection->close();
    unit_selection->close();

    scrollArea->widget()->deleteLater();
    MapImage = QImage();
    MapImageScaled = QImage();

    Map.loaded = false;
    Map_file = "";
    Actual_Level = "";
    changes = false;

    updateToolbarIcons();
    update_changes_state();
    updateActions();
}

void MainWindow::grid_diag()
{
    grid_enabled = showgridAct->isChecked();
    Paint_Map();
    Settings->setValue(REG_SHOW_GRID, showgridAct->isChecked());
}

void MainWindow::coords_diag()
{
    coords_enabled = showcoordsAct->isChecked();
    Paint_Map();
    Settings->setValue(REG_SHOW_COORDS, showcoordsAct->isChecked());
}

//---------------------------------
//mapinfounits
void mapinfounitswindow::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Escape)
        close();
}

void MainWindow::mapInfoUnits_diag()
{
    create_mapinfounits_window();
}
//---------------------------------


void MainWindow::mapInfoGeneral_diag()
{
    QString text = get_MapInfoGeneral_str();
    show_info("Map info", text);
    Check_used_tiles();
}

void MainWindow::tilewindow_diag()
{
    if (showtilewindowAct->isChecked()) {
        Create_Tileselection_window();
    } else {
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
    QDir   dir;

    GameDir = QFileDialog::getExistingDirectory(this, tr("Please select the directory of Historyline 1914-1918"),
                                                GameDir,
                                                QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);

    if (!GameDir.isEmpty() && !GameDir.isNull()) {
        if (!Check_for_game_files()) {
            show_error("I cannot find the required game files in the selected directory!", this);
        } else {
            MapDir = GameDir + MapDir;
            Settings->setValue(REG_GAMEDIR, GameDir);
            Settings->sync();//if further errors occur, at least we manage to save the GameDir

            //tb_file_window (and others) is only disabled if the program is started up
            //without proper GameDir, leading to default glyphs and disabled toobuttons
            if (!tb_tile_window->isEnabled()) {
                init_selecting_path();
                updateToolbarIcons();
                updateActions();
            }
        }
    }
}


void MainWindow::update_Scale_factor()
// execute Scale_factor change, update Settings, update map, update child windows
{
    Settings->setValue(REG_SCALE_FACTOR, Scale_factor);

    if (Map.loaded)
    {
        Paint_Map();

        if ( (showunitwindowAct->isChecked() == true) && (lockWindowTilesizeAct->isChecked() == false) )
        {
            if (!UnitListImageScaled.isNull()) UnitListImageScaled = QImage(); //Release mem for the last used image
            UnitListImageScaled = UnitListImage.scaled(UnitListImage.width()*Scale_factor,UnitListImage.height()*Scale_factor);
            QLabel *imageLabel1 = new QLabel;
            imageLabel1->setPixmap(QPixmap::fromImage(UnitListImageScaled));
            unitscrollArea->setWidget(imageLabel1);
            unit_selection->setMaximumWidth(imageLabel1->width() + (Tilesize * 1));
            unit_selection->setMaximumHeight(imageLabel1->height() + (Tilesize * 2));
            unit_selection->resize((imageLabel1->width()), UnitListImageScaled.height());
            unit_selection->update();
        }

        if ( (showtilewindowAct->isChecked() == true) && (lockWindowTilesizeAct->isChecked() == false) ) {
            Create_Tileselection_window();
        }
    }
}

void MainWindow::setScale_diag()
//fixed with min, max and step. Now more or less obsolete with zoom buttons
{
    QInputDialog dlg(this);
    dlg.setWindowTitle("Scaling of VGA bitmaps:");
    dlg.setLabelText("Enter a factor for scaling");
    dlg.setWindowFlags(dlg.windowFlags() | Qt::WindowStaysOnTopHint);
    dlg.setDoubleValue(Scale_factor);
    dlg.setDoubleStep(0.5);
    dlg.setDoubleMinimum(Scale_factor_min);
    dlg.setDoubleMaximum(Scale_factor_max);
    if (dlg.exec())
    {
        Scale_factor = dlg.doubleValue();
        if (Scale_factor < Scale_factor_min || Scale_factor > Scale_factor_max) {
            Scale_factor = 2; //
        }
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
        QInputDialog dlg(this);
        dlg.setWindowTitle("Add map to game");
        dlg.setLabelText("Enter a levelcode for your map (5 letters):");
        dlg.setWindowFlags(dlg.windowFlags() | Qt::WindowStaysOnTopHint);

        //force max 5 chars, upcase automatically, disallow anything but a-Z
        connect(&dlg, &QInputDialog::textValueChanged, this, [&dlg](const QString text) {
            QString t = text;
            t = t.replace(QRegularExpression("[^0-9a-zA-Z]+"), "");
            if (t.length() == 6) {
                dlg.setTextValue(t.left(5));
            } else {
                dlg.setTextValue(t.toUpper());
            }
        });

        ok = dlg.exec();
        QString levelcode = dlg.textValue();

        if (ok && !levelcode.isEmpty())
        {
            if (!Check_levelcode(levelcode))
            {
                show_error("Code must be five letters to work with the game.", this);
                add_diag(); //start over
                return;
            }

            if (Levelcode_exists(levelcode))
            {
                if (ask_question("Level \"" + levelcode.toUpper() + "\" already exists. Do you want to update/overwrite that level?", this) == true)
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
                show_error("There are too many maps in the directory. The game can handle a maximum of 99.", this);
                return;
            }

            int filenumber = QString((QString) maps[maps.size()-1][0]+maps[maps.size()-1][1]).toInt();

            if (Levelcode.Number_of_levels != (filenumber+1))
            {
                show_warning("There are " + QString::number((filenumber+1)) + " valid named map-files in the MAP subdirectory, but " +
                            QString::number(Levelcode.Number_of_levels)+ " maps stored in the game's Code.dat file. " +
                            "Please clean up the directory first.", this);
                return;
            }

            filenumber++;
            Map_file = QString::number(filenumber);
            Map_file = Map_file+".FIN" ;
            Map_file = MapDir+"/"+Map_file;
            Map_file.replace("/'", "\\'");
            if (Save_Mapdata(Map_file.toStdString().data()) != 0) //Create new .FIN file for this map.
            {
                show_error("Failed to create " + Map_file, this);
                return;
            }

            QString SHPfile;
            SHPfile = Map_file;
            SHPfile.replace(".fin",".shp").replace(".FIN",".SHP");
            if (Create_shp(SHPfile.toStdString().data()) != 0)
            {
                show_warning("I cannot save the building data in " + SHPfile, this);
            }

            Codefile = (GameDir + Code_name); //Create a C style filename for use of stdio
            Codefile.replace("/'", "\\'");

            if (Add_map(Codefile.toStdString().data(), levelcode) != 0)
            {
                show_error("Can't write data for the new map to CODES.DAT", this);
                return;
            }


            if (Player2 == false)
            {
                QStringList     items;
                QByteArray      Checksum;

                items << "Type I"
                      << "Type II"
                      << "Type III"
                      << "Type IV";

                QString item = get_item_dialog("Type of computer opponent",
                                               "You have configured your map as a single player map.\nPlease select the type of computer opponent (.COM file) for your map:",
                                               items,
                                               "",
                                               this);

                //fix: remove the newly installed level if the user cancel COM type selection
                if (item.isEmpty()) {
                    remove_level(levelcode, true);
                    return;
                } else {

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
                            show_error("I could not find a type I .COM file in the maps folder that I could use for the new map.", this);
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
                            show_error("I could not find a type II .COM file in the maps folder that I could use for the new map.", this);
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
                            show_error("I could not find a type III .COM file in the maps folder that I could use for the new map.", this);
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
                            show_error("I could not find a type IV .COM file in the maps folder that I could use for the new map.", this);
                        }
                  }

                    Newfile = Map_file;
                    Newfile.replace(".fin",".com").replace(".FIN",".COM");
                    QFile::copy(Comfile, Newfile);

                    if (!QFile::exists(Newfile))
                    {
                        show_error("Failed to create " + Newfile + "!", this);
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
        show_warning("There's nothing I could add to the game.... Why don't you load a map first or create a new one?", this);
    }
}


bool MainWindow::remove_level(QString R_levelcode, bool silent /* = false*/)
// dadk, the level removement itself extracted out to its own function, so it can be executed from elsewhere
// a silent flag indicate whether the user need to accept the operation or not
{
    QString R_SHPfile, R_Mapfile, R_Comfile, R_Codefile, R_Hifile;
    int old_maxlevel;

    if (!Check_levelcode(R_levelcode))
    {
        show_error("The selected levelcode is invalid! Are the game files corrupted?", this);
        return false;
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

    //Ask user for permission
    if (silent == false) {
        if (ask_question("All references to the map " + R_levelcode + " will be removed from the game files and all files belonging to the map will be deleted. Are you sure?", this) == false)
          return false;
    }

    if (Remove_map(R_Codefile.toStdString().data(), R_levelcode) != 0)
    {
        show_error("Failed to update the CODES.DAT file!", this);
        return false;
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
            update_window_title();
            Actual_Level = "";
            set_changes_state(true);
            already_saved = false;
       }
    }

    //notify about the deletion
    if (silent == false) {
        show_info("The level " + R_levelcode + " is now removed!", this);
    }

    return true;
}

void MainWindow::remove_diag()
{
    QStringList levels = get_filtered_level_codes();

    if (levels.count() < 1) {
        show_info("No custom maps found. If you want to remove a native map, you must uncheck Settings -> Hide native maps", this);
        return;
    }

    QString R_levelcode = get_item_dialog("Remove map from game",
                                          "Which map should be removed from the game?",
                                          levels,
                                          "",
                                          this);

    if (!R_levelcode.isEmpty()) {
        if (remove_level(R_levelcode)) {
            if (ask_question("Remove another level?", this)) {
                remove_diag(); //start over
            }
        }
    }
}


void MainWindow::map_resize_diag()
{
    if  (Map.loaded == true)
    {
        int             current, width, height,x,y,o,o1;
        unsigned char*  data;
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

        QString item = get_item_dialog(tr("Select new map size"), "Current size: " + items[current], items, items[current], this);

        if (!item.isEmpty())
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
                show_error("Memory allocation error!", this);
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

            Paint_Map(true);
        }
    }
    else
    {
       show_warning("Please load or create a map first.", this);
    }
}

void MainWindow::season_diag()
{
    QString      C_Filename1;
    QString      C_Filename2;

    summer = !summer;

    if (!summer) {  //Change to winter
        C_Filename1 = get_path(Partlib_W_name);
        C_Filename2 = get_path(Partdat_W_name);
    } else {
        C_Filename1 = get_path(Partlib_S_name);
        C_Filename2 = get_path(Partdat_S_name);
    }

    if (Partlib.data != NULL) free(Partlib.data);

    if (Load_Part_files(C_Filename1.toStdString().data(), C_Filename2.toStdString().data()) != 0) {
        show_error("Failed to load summer/winter graphics from the game!", this);
        return;
    }

    Create_Tileselection_window();
    Paint_Map(true);

    set_changes_state(true);
    updateToolbarIcons();
}

void MainWindow::maptype_diag()
{
    Player2 = maptypeAct->isChecked();
    set_changes_state(true);
}

void MainWindow::replace_diag()
{
    replacewindow *replacedlg = new replacewindow();
    connect(replacedlg, SIGNAL(TilesReplaced()), this, SLOT(update_map()));
    connect(replacedlg, SIGNAL(TilesReplaced()), this, SLOT(update_changes_state()));
    replacewindow_populate(replacedlg);
}

void MainWindow::buildable_units_diag()
{
    buildablewindow *buildable = new buildablewindow();
    connect(buildable, SIGNAL(BuildableChanged()), this, SLOT(update_changes_state()));
    buildablewindow_populate(buildable);
}

void MainWindow::warning_diag()
{
    Settings->setValue(REG_SHOW_WARNINGS, warningAct->isChecked());
}

void MainWindow::zoom(bool in) {
    if (in == true) {
        if (Scale_factor < Scale_factor_max) {
            Scale_factor = Scale_factor + 0.5;
         } else {
            tb_zoom_in->setEnabled(false);
         }
         tb_zoom_out->setEnabled(true);
    }
    if (in == false) {
        if (Scale_factor > Scale_factor_min) {
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

QAction* MainWindow::initAction(QString caption, QString toolTip, bool isCheckable, bool isChecked, bool isEnabled)
{
    QAction *res = new QAction(caption, this);
    res->setCheckable(isCheckable);
    res->setChecked(isChecked);
    res->setEnabled(isEnabled);
    res->setToolTip(toolTip);
    return res;
}

void MainWindow::createActions()
{
    newAct = initAction("&New", "Create a new map", false, false, false);
    newAct->setShortcuts(QKeySequence::New);
    connect(newAct, &QAction::triggered, this, &MainWindow::newFile_diag);

    openAct = initAction("&Open map by file", "Open an existing map", false, false, false);
    openAct->setShortcuts(QKeySequence::Open);
    connect(openAct, &QAction::triggered, this, &MainWindow::open_diag);

    openbyCodeAct = initAction("Open map by levelcode", "Open an existing map by its ingame levelcode", false, false, false);
    connect(openbyCodeAct, &QAction::triggered, this, &MainWindow::open_by_code_diag);

    saveAct = initAction("&Save", "Save the map to disk", false, false, false);
    saveAct->setShortcuts(QKeySequence::Save);
    connect(saveAct, &QAction::triggered, this, &MainWindow::save_diag);

    saveasAct = initAction("Save map as...", "Save the map to a new file", false, false, false);
    connect(saveasAct, &QAction::triggered, this, &MainWindow::saveas_diag);

    saveImageAct = initAction("Save an image of the map", "Save an image of your map", false, false, false);
    connect(saveImageAct, &QAction::triggered, this, &MainWindow::saveimage_diag);

    addtogameAct = initAction("&Add map to game", "Adds your map to the game", false, false, false);
    connect(addtogameAct, &QAction::triggered, this, &MainWindow::add_diag);

    removefromgameAct = initAction("&Remove map from game", "Removes a map from the game", false, false, false);
    connect(removefromgameAct, &QAction::triggered, this, &MainWindow::remove_diag);

    exitAct = initAction("E&xit", "Exit HL Editor", false, false, true);
    exitAct->setShortcuts(QKeySequence::Quit);
    connect(exitAct, &QAction::triggered, this, &QWidget::close);

    closeAct = initAction("Close map", "Close current map", false, false, false);
    connect(closeAct, &QAction::triggered, this, &MainWindow::close_diag);

    showgridAct = initAction("Show grid", "Show or hide grid", true, false, false);
    showgridAct->setShortcut(Qt::CTRL + Qt::Key_G);
    connect(showgridAct, &QAction::triggered, this, &MainWindow::grid_diag);

    showcoordsAct = initAction("Show coords", "Show or hide tile coords", true, false, false);
    showcoordsAct->setShortcut(Qt::CTRL + Qt::Key_D);
    connect(showcoordsAct, &QAction::triggered, this, &MainWindow::coords_diag);

    showtilewindowAct = initAction("Toggle Tile selection window", "Show or hide the tile selection window", true, true, false);
    showtilewindowAct->setShortcut(Qt::CTRL + Qt::Key_T);
    connect(showtilewindowAct, &QAction::triggered, this, &MainWindow::tilewindow_diag);

    showunitwindowAct = initAction("Toggle Unit selection window", "Show or hide the unit selection window", true, true, false);
    showunitwindowAct->setShortcut(Qt::CTRL + Qt::Key_U);
    connect(showunitwindowAct, &QAction::triggered, this, &MainWindow::unitwindow_diag);

    mapresizeAct = initAction("&Resize map", "Change the size of the map", false, false, false);
    connect(mapresizeAct, &QAction::triggered, this, &MainWindow::map_resize_diag);

    changeseasonAct = initAction("Toggle summer/winter", "Map plays in summer or in winter", false, false, false);
    connect(changeseasonAct, &QAction::triggered, this, &MainWindow::season_diag);

    replaceAct = initAction("Replace tile", "Replaces one tile with another", false, false, false);
    replaceAct->setIcon(QIcon(":/images/replace.png"));
    connect(replaceAct, &QAction::triggered, this, &MainWindow::replace_diag);

    maptypeAct = initAction("Two-Player map", "Sets Map type to single or two player map", true, true, false); //!?
    connect(maptypeAct, &QAction::triggered, this, &MainWindow::maptype_diag);

    mapInfoGeneralAct = initAction("Map info", "Show info about the map", false, false, false);
    connect(mapInfoGeneralAct, &QAction::triggered, this, &MainWindow::mapInfoGeneral_diag);

    mapInfoUnitsAct = initAction("Units info", "Shows overview over units for both sides", false, false, false);
    connect(mapInfoUnitsAct, &QAction::triggered, this, &MainWindow::mapInfoUnits_diag);

    buildableunitsAct = initAction("Set buildable units", "Which units can be built in factories?", false, false, false);
    connect(buildableunitsAct, &QAction::triggered, this, &MainWindow::buildable_units_diag);

    setPathAct = initAction("&Game path", "Set path to game resources", false, false, true);
    connect(setPathAct,&QAction::triggered, this, &MainWindow::setPath_diag);

    setScaleFactorAct = initAction("&Scale factor", "Scaler used to enlarge/enhance low resolution bitmaps.", false, false, false);
    connect(setScaleFactorAct,&QAction::triggered, this, &MainWindow::setScale_diag);

    warningAct = initAction("Show warnings", "Issue a warning if the map cannot be displayed correctly in the game or could lead to errors in the game.", true, false, false);
    connect(warningAct, &QAction::triggered, this, &MainWindow::warning_diag);

    hideNativeMapsAct = initAction("Hide native maps", "Hide 'native' maps when you load or remove maps by level code", true, true, false);

    lockWindowTilesizeAct = initAction("Lock Window Tile Sizes", "Lock child window tile sizes to current scale factor", true, false, false);
    connect(lockWindowTilesizeAct, &QAction::triggered, [this] {
        if (lockWindowTilesizeAct->isChecked()) {
           Settings->setValue(REG_LOCK_TILESIZE, Scale_factor);
        } else {
           Settings->remove(REG_LOCK_TILESIZE);
        }
    });

    autoloadAct = initAction("Autoload recent map", "Load recent map on restart", true, true, true);
    connect(autoloadAct,&QAction::triggered, [this] {
        Settings->setValue(REG_AUTOLOAD, autoloadAct->isChecked());
    });

    restoreWindowPosAct = initAction("Restore Window positions", "Automatically restore windows position and size on startup", true, true, true);
    connect(restoreWindowPosAct,&QAction::triggered, [this] {
        Settings->setValue(REG_RESTORE_WINDOWS, restoreWindowPosAct->isChecked());
        if (restoreWindowPosAct->isChecked())
            saveWindowPos();
    });

    resetSettingsAct = initAction("Reset All settings", "Reset all current settings (except the game path to HistoryLine)", false, false, true);
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

    autogenLightForestAct = new QAction("Light Forest", this);
    connect(autogenLightForestAct, &QAction::triggered, [this] {
        autogenerate_Forest(false);
        set_changes_state(true);
        Paint_Map();
    });

    autogenDenseForestAct = new QAction("Dense Forest", this);
    connect(autogenDenseForestAct, &QAction::triggered, [this] {
        autogenerate_Forest(true);
        set_changes_state(true);
        Paint_Map();
    });

    autogenPlainsAct = new QAction("Plains", this);
    connect(autogenPlainsAct, &QAction::triggered, [this] {
        autogenerate_Plains();
        set_changes_state(true);
        Paint_Map();
    });

    autogenWastelandAct = new QAction("Wasteland", this);
    connect(autogenWastelandAct, &QAction::triggered, [this] {
        autogenerate_Wasteland();
        set_changes_state(true);
        Paint_Map();
    });

    autogenResidentalAreaAct = new QAction("Residentals", this);
    connect(autogenResidentalAreaAct, &QAction::triggered, [this] {
        autogenerate_Residentalarea();
        set_changes_state(true);
        Paint_Map();
    });

    autogenBattlefieldAct = new QAction("Battlefield", this);
    connect(autogenBattlefieldAct, &QAction::triggered, [this] {
        autogenerate_Battlefield();
        set_changes_state(true);
        Paint_Map();
    });

    autogenLakeAct = new QAction("Lake", this);
    connect(autogenLakeAct, &QAction::triggered, [this] {
        autogenerate_Lake();
        set_changes_state(true);
        Paint_Map();
    });

    autogenIslandAct = new QAction("Island", this);
    connect(autogenIslandAct, &QAction::triggered, [this] {
        autogenerate_Island();
        set_changes_state(true);
        Paint_Map();
    });

    protectBuildingsAct = initAction("Protect Building tiles (dont overwrite)", "", true, protected_terrain.Buildings, true);
    connect(protectBuildingsAct, &QAction::triggered, [this] {
        protected_terrain.Buildings = protectBuildingsAct->isChecked();
    });

    protectRoadsAct = initAction("Protect Road tiles (of any kind)", "", true, protected_terrain.Roads, true);
    connect(protectRoadsAct, &QAction::triggered, [this] {
        protected_terrain.Roads = protectRoadsAct->isChecked();
    });

    protectWaterAct = initAction("Protect Water tiles (including coast lines)", "", true, protected_terrain.Water, true);
    connect(protectWaterAct, &QAction::triggered, [this] {
        protected_terrain.Water = protectWaterAct->isChecked();
    });

    protectRiverAct = initAction("Protect River tiles", "", true, protected_terrain.River, true);
    connect(protectRiverAct, &QAction::triggered, [this] {
        protected_terrain.River = protectRiverAct->isChecked();
    });


//-- options menu
    QActionGroup *pathGroup = new QActionGroup(this);
    autogenPathNormalAct = new QAction("Classic", pathGroup);
    autogenPathNormalAct->setCheckable(true);
    autogenPathNormalAct->setChecked(AUTOGEN_PATH_TYPE == 1);
    connect(autogenPathNormalAct, &QAction::triggered, [this] {
        AUTOGEN_PATH_TYPE = 1;
        Settings->setValue(REG_AUTOGEN_PATH_TYPE, AUTOGEN_PATH_TYPE);
    });

    autogenPathStraightAct = new QAction("Straight", pathGroup);
    autogenPathStraightAct->setCheckable(true);
    autogenPathStraightAct->setChecked(AUTOGEN_PATH_TYPE == 2);
    connect(autogenPathStraightAct, &QAction::triggered, [this] {
        AUTOGEN_PATH_TYPE = 2;
        Settings->setValue(REG_AUTOGEN_PATH_TYPE, AUTOGEN_PATH_TYPE);
    });

    autogenPathWavyAct = new QAction("Wavy path", pathGroup);
    autogenPathWavyAct->setCheckable(true);
    autogenPathWavyAct->setChecked(AUTOGEN_PATH_TYPE == 3);
    connect(autogenPathWavyAct, &QAction::triggered, [this] {
        AUTOGEN_PATH_TYPE = 3;
        Settings->setValue(REG_AUTOGEN_PATH_TYPE, AUTOGEN_PATH_TYPE);
    });

    autogenPathPreferDirectionAct = new QAction("Autobahn", pathGroup);
    autogenPathPreferDirectionAct->setCheckable(true);
    autogenPathPreferDirectionAct->setChecked(AUTOGEN_PATH_TYPE == 4);
    autogenPathPreferDirectionAct->setToolTip("Prefers to continue in the same direction for as long as possible (fewer bends). Results in a more ‘motorway-like’ or railway-like route.");
    connect(autogenPathPreferDirectionAct, &QAction::triggered, [this] {
        AUTOGEN_PATH_TYPE = 4;
        Settings->setValue(REG_AUTOGEN_PATH_TYPE, AUTOGEN_PATH_TYPE);
    });

    autogenPathScenicAct = new QAction("Seek buildings", pathGroup);
    autogenPathScenicAct->setCheckable(true);
    autogenPathScenicAct->setChecked(AUTOGEN_PATH_TYPE == 5);
    connect(autogenPathScenicAct, &QAction::triggered, [this] {
        AUTOGEN_PATH_TYPE = 5;
        Settings->setValue(REG_AUTOGEN_PATH_TYPE, AUTOGEN_PATH_TYPE);
    });

    autogenPathRandomDetourAct = new QAction("Random", pathGroup);
    autogenPathRandomDetourAct->setCheckable(true);
    autogenPathRandomDetourAct->setChecked(AUTOGEN_PATH_TYPE == 6);
    connect(autogenPathRandomDetourAct, &QAction::triggered, [this] {
        AUTOGEN_PATH_TYPE = 6;
        Settings->setValue(REG_AUTOGEN_PATH_TYPE, AUTOGEN_PATH_TYPE);
    });

    QActionGroup *straightGroup = new QActionGroup(this);

    autogenPathStraight0Act = new QAction("Straight 0", straightGroup);
    autogenPathStraight0Act->setCheckable(true);
    connect(autogenPathStraight0Act, &QAction::triggered, [this] {
        AUTOGEN_STRAIGHT_DIST = 0;
        Settings->setValue(REG_AUTOGEN_PATH_STRAIGHT_DIST, AUTOGEN_STRAIGHT_DIST);
    });

    autogenPathStraight2Act = new QAction("Straight 2", straightGroup);
    autogenPathStraight2Act->setCheckable(true);
    connect(autogenPathStraight2Act, &QAction::triggered, [this] {
        AUTOGEN_STRAIGHT_DIST = 2;
        Settings->setValue(REG_AUTOGEN_PATH_STRAIGHT_DIST, AUTOGEN_STRAIGHT_DIST);
    });

    autogenPathStraight3Act = new QAction("Straight 3", straightGroup);
    autogenPathStraight3Act->setCheckable(true);
    connect(autogenPathStraight3Act, &QAction::triggered, [this] {
        AUTOGEN_STRAIGHT_DIST = 3;
        Settings->setValue(REG_AUTOGEN_PATH_STRAIGHT_DIST, AUTOGEN_STRAIGHT_DIST);
    });
//

    nocrossBuildingAct = initAction("Buildings", "Wind path around buildings", true, no_cross_terrain.Building, true);
    connect(nocrossBuildingAct, &QAction::triggered, [this] {
        no_cross_terrain.Building = nocrossBuildingAct->isChecked();
    });

    nocrossMountainAct = initAction("Mountains", "Wind path around mountains", true, no_cross_terrain.Mountain, true);
    connect(nocrossMountainAct, &QAction::triggered, [this] {
        no_cross_terrain.Mountain = nocrossMountainAct->isChecked();
    });

    nocrossWaterAct = initAction("Water", "Do not cross water tiles", true, no_cross_terrain.Water, true);
    connect(nocrossWaterAct, &QAction::triggered, [this] {
        no_cross_terrain.Water = nocrossWaterAct->isChecked();
    });

    nocrossHillAct = initAction("Hills", "Wind path around hills", true, no_cross_terrain.Hill, true);
    connect(nocrossHillAct, &QAction::triggered, [this] {
        no_cross_terrain.Hill = nocrossHillAct->isChecked();
    });

    nocrossReefAct = initAction("Reef", "Do not enter reef tiles", true, no_cross_terrain.Reef, true);
    connect(nocrossReefAct, &QAction::triggered, [this] {
        no_cross_terrain.Reef = nocrossReefAct->isChecked();
    });

    nocrossBunkerAct = initAction("Bunkers", "Wind path around bunkers", true, no_cross_terrain.Bunker, true);
    connect(nocrossBunkerAct, &QAction::triggered, [this] {
        no_cross_terrain.Bunker = nocrossBunkerAct->isChecked();
    });

    nocrossTrenchAct = initAction("Trenches", "Do not cross trenches", true, no_cross_terrain.Trench, true);
    connect(nocrossTrenchAct, &QAction::triggered, [this] {
        no_cross_terrain.Trench = nocrossTrenchAct->isChecked();
    });

    nocrossResidentalsAct = initAction("Residentals", "Wind path around residental tiles", true, no_cross_terrain.Residentals, true);
    connect(nocrossResidentalsAct, &QAction::triggered, [this] {
        no_cross_terrain.Residentals = nocrossResidentalsAct->isChecked();
    });

    nocrossForestAct = initAction("Forest", "Wind path around forest tiles", true, no_cross_terrain.Forest, true);
    connect(nocrossForestAct, &QAction::triggered, [this] {
        no_cross_terrain.Forest = nocrossForestAct->isChecked();
    });

    nocrossRiverAct = initAction("River", "Do not cross river tiles (except bridges are available)", true, no_cross_terrain.River, true);
    connect(nocrossRiverAct, &QAction::triggered, [this] {
        no_cross_terrain.River = nocrossRiverAct->isChecked();
    });

    nocrossDirtroadAct = initAction("Dirtroad", "Do not cross dirt roads (except where bridges or crossings are available)", true, no_cross_terrain.Dirtroad, true);
    connect(nocrossDirtroadAct, &QAction::triggered, [this] {
        no_cross_terrain.Dirtroad = nocrossDirtroadAct->isChecked();
    });

    nocrossRoadAct = initAction("Road", "Do not cross roads (except where crossings or bridges are available)", true, no_cross_terrain.Road, true);
    connect(nocrossRoadAct, &QAction::triggered, [this] {
        no_cross_terrain.Road = nocrossRoadAct->isChecked();
    });

    nocrossRailroadAct = initAction("Railroad", "Do not cross railroads (except where crossings or bridges are available)", true, no_cross_terrain.Railroad, true);
    connect(nocrossRailroadAct, &QAction::triggered, [this] {
        no_cross_terrain.Railroad = nocrossRailroadAct->isChecked();
    });

}

QToolButton* MainWindow::initToolButton(bool isCheckable, bool isChecked, bool isEnabled, QString toolTip, QIcon icon)
{
    QToolButton *res = new QToolButton(this);
    res->setCheckable(isCheckable);
    res->setChecked(isChecked);
    res->setEnabled(isEnabled);
    res->setToolTip(toolTip);
    res->setIcon(icon);
    return res;
}

void MainWindow::createToolbar()
{
    QToolBar *toolbar;
    toolbar = addToolBar(""); //??
    toolbar->setStyleSheet("QToolBar {border-left:1px dotted rgb(120,120,120);} ::separator{background:#ddd;padding:1rem; };");

    tb_deselect = initToolButton(true, false, false, "'Browse mode', reset tile and unit selections", QIcon(":/images/cursor-default-outline.png"));
    connect(tb_deselect, &QToolButton::clicked, [this]() {
        tile_selection->resetSelection();
        unit_selection->resetSelection();
        update_deselect_button();
    });
    toolbar->addWidget(tb_deselect);

    toolbar->addSeparator();

    tb_open_file = initToolButton(false, false, true, "Open map from file, Ctrl+O", QIcon(":/images/folder-open-o.png"));
    tb_open_file->setAutoRaise(false);
    connect(tb_open_file, &QToolButton::clicked, this, &MainWindow::open_diag);
    toolbar->addWidget(tb_open_file);

    toolbar->addSeparator();

    tb_save_changes = initToolButton(false, false, false, "Save changes, Ctrl+S", QIcon(":/images/floppy-green.png"));
    connect(tb_save_changes, &QToolButton::clicked, this, &MainWindow::save_diag);
    toolbar->addWidget(tb_save_changes);

    toolbar->addSeparator();

    tb_replace_tile = initToolButton(false, false, false, "Replace tiles", QIcon(":/images/replace.png"));
    connect(tb_replace_tile, &QToolButton::clicked, this, &MainWindow::replace_diag);
    toolbar->addWidget(tb_replace_tile);

    toolbar->addSeparator();

    tb_zoom_in = initToolButton(false, false, false, "Zoom in", QIcon(":/images/zoom-in-zero.png"));
    connect(tb_zoom_in, &QToolButton::clicked, [this]() { zoom(true); });
    toolbar->addWidget(tb_zoom_in);

    tb_zoom_out = initToolButton(false, false, false, "Zoom out", QIcon(":/images/zoom-out-zero.png"));
    connect(tb_zoom_out, &QToolButton::clicked, [this]() { zoom(false); });
    toolbar->addWidget(tb_zoom_out);

    toolbar->addSeparator();

    tb_move_tl = initToolButton(false, false, false, "Move to top left", QIcon(":/images/rectangle-1-gray.png"));
    connect(tb_move_tl, &QToolButton::clicked, [this]() {
        scrollArea->verticalScrollBar()->setValue(0);
        scrollArea->horizontalScrollBar()->setValue(0);
    });
    toolbar->addWidget(tb_move_tl);

    tb_move_bl = initToolButton(false, false, false, "Move to bottom left", QIcon(":/images/rectangle-3-gray.png"));
    connect(tb_move_bl, &QToolButton::clicked, [this]() {
        scrollArea->verticalScrollBar()->setValue(scrollArea->maximumWidth());
        scrollArea->horizontalScrollBar()->setValue(0);
    });
    toolbar->addWidget(tb_move_bl);

    tb_move_tr = initToolButton(false, false, false, "Move to top right", QIcon(":/images/rectangle-2-gray.png"));
    connect(tb_move_tr, &QToolButton::clicked, [this]() {
        scrollArea->verticalScrollBar()->setValue(0);
        scrollArea->horizontalScrollBar()->setValue(scrollArea->maximumWidth());
    });
    toolbar->addWidget(tb_move_tr);

    tb_move_br = initToolButton(false, false, false, "Move to bottom right", QIcon(":/images/rectangle-4-gray.png"));
    connect(tb_move_br, &QToolButton::clicked, [this]() {
        scrollArea->verticalScrollBar()->setValue(scrollArea->maximumHeight());
        scrollArea->horizontalScrollBar()->setValue(scrollArea->maximumWidth());
    });
    toolbar->addWidget(tb_move_br);

    toolbar->addSeparator();

    tb_map_info = initToolButton(false, false, false, "Show map information", QIcon(":/images/info-gray.png"));
    QMenu *infomenu = new QMenu();
    infomenu->addAction(mapInfoGeneralAct);
    infomenu->addAction(mapInfoUnitsAct);
    tb_map_info->setMenu(infomenu);
    tb_map_info->setPopupMode(QToolButton::InstantPopup);
    toolbar->addWidget(tb_map_info);

    toolbar->addSeparator();

    // dadk, Here I would have assumed you could just 'connect' to showtilewindowAct/showunitwindowAct
    // and by that inherit 'checked' status and so on to the button. But apparently I do not understand Qt
    tb_tile_window = initToolButton(true, true, false, "Toggle Tile selection", QIcon(":/images/VERBOTEN_color.PNG"));
    connect(tb_tile_window, &QToolButton::clicked, [this]() {
        showtilewindowAct->setChecked(tb_tile_window->isChecked());
        if (tb_tile_window->isChecked()) {
            tb_tile_window->setIcon( get_part_icon(79));
        } else {
            tb_tile_window->setIcon( get_part_icon(79, false, true));
        }
        tilewindow_diag();
    });
    toolbar->addWidget(tb_tile_window);

    tb_unit_window = initToolButton(true, true, false, "Toggle unit selection", QIcon(":/images/VERBOTEN_color.PNG"));
    connect(tb_unit_window, &QToolButton::clicked, [this]() {
        showunitwindowAct->setChecked(tb_unit_window->isChecked());
        if (tb_unit_window->isChecked()) {
           tb_unit_window->setIcon( get_unit_icon(22, 3));
        } else {
           tb_unit_window->setIcon( get_unit_icon(22, 3, true));
        }
        unitwindow_diag();
    });
    toolbar->addWidget(tb_unit_window);

    toolbar->addSeparator();

    tb_selection_mode = initToolButton(true, SELECTION_MODE, false, "Toggle selection mode", QIcon(":/images/select.png"));
    connect(tb_selection_mode, &QToolButton::clicked, [this]() {
        SELECTION_MODE = tb_selection_mode->isChecked();
    });
    toolbar->addWidget(tb_selection_mode);

    QMenu *autogenMenu = new QMenu();
    autogenMenu->setToolTipsVisible(true);
    autogenMenu->addAction(autogenLightForestAct);
    autogenMenu->addAction(autogenDenseForestAct);
    autogenMenu->addSeparator();
    autogenMenu->addAction(autogenPlainsAct);
    autogenMenu->addAction(autogenWastelandAct);
    autogenMenu->addAction(autogenResidentalAreaAct);
    autogenMenu->addAction(autogenBattlefieldAct);
    autogenMenu->addAction(autogenLakeAct);
    autogenMenu->addAction(autogenIslandAct);

    autogenMenu->addSeparator();
    autogenMenu->addAction(protectBuildingsAct);
    autogenMenu->addAction(protectRoadsAct);
    autogenMenu->addAction(protectWaterAct);
    autogenMenu->addAction(protectRiverAct);

    tb_autogen_fill = initToolButton(false, false, false, "Autogenerate", QIcon(":/images/grok-sel-hex.png"));
    tb_autogen_fill->setMenu(autogenMenu);
    tb_autogen_fill->setPopupMode(QToolButton::InstantPopup);
    toolbar->addWidget(tb_autogen_fill);

    toolbar->addSeparator();

    tb_autogen_dirtroad = initToolButton(false, false, false, "Autogenerate", QIcon(":/images/VERBOTEN_color.PNG"));
    tb_autogen_dirtroad->setAutoRaise(true);
        connect(tb_autogen_dirtroad, &QToolButton::clicked, [this]() {
        init_selecting_path(1);
    });
    toolbar->addWidget(tb_autogen_dirtroad);

    tb_autogen_road = initToolButton(false, false, false, "Generate Road", QIcon(":/images/VERBOTEN_color.PNG"));
    tb_autogen_road->setAutoRaise(true);
    connect(tb_autogen_road, &QToolButton::clicked, [this]() {
        init_selecting_path(2);
    });
    toolbar->addWidget(tb_autogen_road);

    tb_autogen_railroad = initToolButton(false, false, false, "Generate Railroad", QIcon(":/images/VERBOTEN_color.PNG"));
    tb_autogen_railroad->setAutoRaise(true);
    connect(tb_autogen_railroad, &QToolButton::clicked, [this]() {
        init_selecting_path(3);
    });
    toolbar->addWidget(tb_autogen_railroad);

    tb_autogen_river = initToolButton(false, false, false, "Generate River", QIcon(":/images/VERBOTEN_color.PNG"));
    tb_autogen_river->setAutoRaise(true);
    connect(tb_autogen_river, &QToolButton::clicked, [this]() {
        init_selecting_path(4);
    });
    toolbar->addWidget(tb_autogen_river);

    tb_autogen_trench = initToolButton(false, false, false, "Generate Trench", QIcon(":/images/VERBOTEN_color.PNG"));
    tb_autogen_trench->setAutoRaise(true);
    connect(tb_autogen_trench, &QToolButton::clicked, [this]() {
        init_selecting_path(5);
    });
    toolbar->addWidget(tb_autogen_trench);

    tb_autogen_coastRightTop = initToolButton(false, false, false, "Generate right / top Coastline", QIcon(":/images/VERBOTEN_color.PNG"));
    tb_autogen_coastRightTop->setAutoRaise(true);
    connect(tb_autogen_coastRightTop, &QToolButton::clicked, [this]() {
        init_selecting_path(6);
    });
    toolbar->addWidget(tb_autogen_coastRightTop);

    tb_autogen_coastLeftBottom = initToolButton(false, false, false, "Generate left / bottom Coastline", QIcon(":/images/VERBOTEN_color.PNG"));
    tb_autogen_coastLeftBottom->setAutoRaise(true);
    connect(tb_autogen_coastLeftBottom, &QToolButton::clicked, [this]() {
        init_selecting_path(7);
    });
    toolbar->addWidget(tb_autogen_coastLeftBottom);

    toolbar->addSeparator();

    tb_autogen_options = initToolButton(false, false, true, "Autogenerate options", QIcon(":/images/hex-connect.png"));

    QMenu *autogenOptMenu = new QMenu();
    autogenOptMenu->setToolTipsVisible(true);

    autogenOptMenu->addSection("Path");
    autogenOptMenu->addAction(autogenPathStraightAct);
    autogenOptMenu->addAction(autogenPathNormalAct);
    autogenOptMenu->addAction(autogenPathWavyAct);
    autogenOptMenu->addAction(autogenPathPreferDirectionAct);
    autogenOptMenu->addAction(autogenPathRandomDetourAct);
    autogenOptMenu->addAction(autogenPathScenicAct);

/*
    autogenOptMenu->addSection("Straight mode");
    autogenOptMenu->addAction(autogenPathStraight0Act);
    autogenOptMenu->addAction(autogenPathStraight2Act);
    autogenOptMenu->addAction(autogenPathStraight3Act);
*/

    autogenOptMenu->addSection("Wind path around");
    autogenOptMenu->addAction(nocrossBuildingAct);
    autogenOptMenu->addAction(nocrossMountainAct);
    autogenOptMenu->addAction(nocrossHillAct);
    autogenOptMenu->addAction(nocrossWaterAct);
    autogenOptMenu->addAction(nocrossReefAct);
    autogenOptMenu->addAction(nocrossRiverAct);
    autogenOptMenu->addAction(nocrossForestAct);
    autogenOptMenu->addAction(nocrossDirtroadAct);
    autogenOptMenu->addAction(nocrossRoadAct);
    autogenOptMenu->addAction(nocrossRailroadAct);
    autogenOptMenu->addAction(nocrossResidentalsAct);
    autogenOptMenu->addAction(nocrossBunkerAct);
    autogenOptMenu->addAction(nocrossTrenchAct);

    tb_autogen_options->setMenu(autogenOptMenu);
    tb_autogen_options->setPopupMode(QToolButton::InstantPopup);

    toolbar->addWidget(tb_autogen_options);
}

void MainWindow::createMenus()
{
    fileMenu = menuBar()->addMenu(tr("&File"));
    fileMenu->addAction(newAct);
    fileMenu->addAction(openAct);
    fileMenu->addAction(openbyCodeAct);

    fileMenu->addSeparator();
    menuRecentFiles = fileMenu->addMenu("Recent ...");
    menuRecentFiles->setEnabled(false);
    fileMenu->addSeparator();

    fileMenu->addAction(saveAct);
    fileMenu->addAction(saveasAct);
    fileMenu->addSeparator();
    fileMenu->addAction(closeAct);
    fileMenu->addSeparator();
    fileMenu->addAction(saveImageAct);
    fileMenu->addSeparator();
    fileMenu->addAction(addtogameAct);
    fileMenu->addAction(removefromgameAct);
    fileMenu->addSeparator();
    fileMenu->addAction(exitAct);

    editMenu = menuBar()->addMenu(tr("&Edit"));
    editMenu->addAction(mapresizeAct);
    editMenu->addAction(changeseasonAct);
    editMenu->addAction(maptypeAct);
    editMenu->addAction(replaceAct);
    editMenu->addAction(buildableunitsAct);
    editMenu->addSeparator();
    editMenu->addAction(showgridAct);
    editMenu->addAction(showcoordsAct);
    editMenu->addSeparator();
    editMenu->addAction(showtilewindowAct);
    editMenu->addAction(showunitwindowAct);
    editMenu->addAction(mapInfoGeneralAct);

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

void MainWindow::updateRecentFiles()
{
    QStringList recent = Settings->value(REG_RECENT_FILES).toString().split(";");
    QStringList fixed;

    QString name = Map_file;
    name.replace(GameDir + "/MAP/", ""); //strip path for maps located in MAP/
    recent.prepend(name);

    for (int i=0; i<recent.count(); i++) {
        if (!recent[i].isEmpty())
            if (!fixed.contains(recent[i]))
                fixed.append(recent[i]);
        if (fixed.count() > 4)
            break;
    }

    Settings->setValue(REG_RECENT_FILES, fixed.join(";"));

    if (fixed.count()) {
        menuRecentFiles->setEnabled(true);
        menuRecentFiles->clear();
        for (int i=0; i<fixed.count(); i++) {
            QAction *m = new QAction(fixed[i]);
            QObject::connect(m, &QAction::triggered, [m, this]() {
                QString f = m->text();
                if (!f.contains("/")) f = GameDir + "/MAP/" + f;
                if (QFileInfo::exists(f)) {
                    Map_file = f;
                    Open_Map();
                } else {
                    show_error("The file '" + (m->text()) + "' could not be found", this);
                }
            });
            menuRecentFiles->addAction(m);
        }
    } else {
        menuRecentFiles->setEnabled(false);
    }
}

//========================== Event handling for child windows =================================
void tilelistwindow::mousePressEvent(QMouseEvent *event)
{
    double sf = lockWindowTilesizeAct->isChecked() ? Settings->value(REG_LOCK_TILESIZE).toDouble() : Scale_factor;

    if (event->button() == Qt::LeftButton) {
        QRect b_widgetRect = BasicTilescrollArea->geometry();
        QRect e_widgetRect = ExtTilescrollArea->geometry();

        if (b_widgetRect.contains(event->pos())) {
            int b_pos_x = BasicTilescrollArea->horizontalScrollBar()->value();
            int b_pos_y = BasicTilescrollArea->verticalScrollBar()->value();

            int b_fx = (event->pos().x()-b_widgetRect.left()+b_pos_x) / (Tilesize * sf); // Calc field position from mouse cords
            int b_fy = (event->pos().y()-b_widgetRect.top()+b_pos_y) / (Tilesize * sf);

            if (((b_fy * 10) + b_fx) < 25) {
                selected_tile = ((b_fy * 10) + b_fx);
                emit SelectionChanged();
                tilelistwindow_update(this, b_fx, b_fy);
            }
        } else {
            if (e_widgetRect.contains(event->pos())) {
                int e_pos_x = ExtTilescrollArea->horizontalScrollBar()->value();
                int e_pos_y = ExtTilescrollArea->verticalScrollBar()->value();

                int e_fx = (event->pos().x()-e_widgetRect.left() + e_pos_x) / (Tilesize * sf); // Calc field position from mouse cords
                int e_fy = (event->pos().y()-e_widgetRect.top() + e_pos_y) / (Tilesize * sf);

                if (((e_fy*10)+e_fx) < Num_Parts-1) {
                    selected_tile = ((e_fy*10)+e_fx)+25;
                    emit SelectionChanged();
                    tilelistwindow_update(this, e_fx, e_fy);
                }
            }
        }
    }
    if (event->button() == Qt::RightButton) {
        resetSelection();
    }
}


void tilelistwindow::resetSelection()
{
    selected_tile = 0xFF;
    emit SelectionChanged();
    tilelistwindow_update(this);
}

void tilelistwindow::mouseDoubleClickEvent (QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        if (tile_selection->height() >= tile_selection->maximumHeight()) {
            tile_selection->resize(tile_selection->width(), tile_selection->maximumHeight() / 2); // half the height
        } else {
            tile_selection->resize(tile_selection->maximumWidth(), tile_selection->maximumHeight()); //set maximum size
        }
    }
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

    if (event->button() == Qt::RightButton) {
        if (unitscrollArea->rect().contains(event->pos())) {
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
}

void unitlistwindow::mouseDoubleClickEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        if (unit_selection->height() >= unit_selection->maximumHeight()) {
            unit_selection->resize(unit_selection->width(), unit_selection->maximumHeight() / 2); // half the height
        } else {
            unit_selection->resize(unit_selection->maximumWidth(), unit_selection->maximumHeight()); //set maximum size
        }
    }
}

//--------------------------------------------

void buildablewindow::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        if (buildablescrollArea->rect().contains(event->pos())) {
            double sf = lockWindowTilesizeAct->isChecked() ? Settings->value(REG_LOCK_TILESIZE).toDouble() : Scale_factor;

            int pos_x = buildablescrollArea->horizontalScrollBar()->value();
            int pos_y = buildablescrollArea->verticalScrollBar()->value();
            QRect widgetRect = buildablescrollArea->geometry();
            int fx = (event->pos().x()-widgetRect.left()+pos_x) / (Tilesize*sf); // Calc field position from mouse cords
            int fy = (event->pos().y()-widgetRect.top()+pos_y) / (Tilesize*sf);

            int unit = ((fy*10)+fx);     //Calc correct unit number

            buildable_unitname->setText(Unit_Name[unit]);

            if (unit <= Num_Units) {
               if (SHP.can_be_built[unit] == 0) {
                   SHP.can_be_built[unit] = 1;
               } else {
                   SHP.can_be_built[unit] = 0;
               }
               buildablewindow_update(this);
               changes = true;
               emit BuildableChanged();
            }
        }
    }
}

//buildingwindow
void buildingwindow::mousePressEvent(QMouseEvent *event)
{
    QRect widgetRect = Building_ScrollArea->geometry();
    double sf = lockWindowTilesizeAct->isChecked() ? Settings->value(REG_LOCK_TILESIZE).toDouble() : Scale_factor;

    if ((event->button() == Qt::LeftButton) && (selected_building != -1)) {
        if (widgetRect.contains(event->pos())) {
            int fx = ((event->pos().x()-widgetRect.left()) / sf) / Tilesize;
            if (selected_unit != 0xFF) {
                if (selected_unit/2 == 13) {
                    return; //bunkers cannot move out from any transporter or building, so meaningless to allow them inside
                }

                if (unit_get_building_capacity(selected_building) == 9 && !unit_allow_in_supply_car(selected_unit/2)) {
                    return; //unit not allowed in supply cars
                }

                if (unit_get_building_capacity(selected_building) == 35 && !unit_allow_in_transporter(selected_unit/2)) {
                    return; //unit not allowed in supply train, transport ship
                }

                //already full?
                if (unit_get_building_weight(selected_building) >= unit_get_building_capacity(selected_building)) {
                    QString msg = Building_info[selected_building].Properties->Type == 3 ? "Transporter" : "Building";
                    msg = msg + " is full";
                    show_error(msg, this);
                    return;
                }

                //will exceed capacity?
                if ((unit_get_building_weight(selected_building) + unit_get_weight(selected_unit)) > unit_get_building_capacity(selected_building)) {
                    QString msg = Building_info[selected_building].Properties->Type == 3 ? "transporters'" : "buildings'";
                    msg = "Adding this unit will exceed the " + msg + " capacity";
                    show_error(msg, this);
                    return;
                }
            }
            Building_info[selected_building].Properties->Units[fx] = selected_unit != 0xFF ? selected_unit/2 : 0xFF;
            buildingwindow_update_tiles(this);
            changes = true;
            emit BuildingChanged();
       }
    }

    if ((event->button() == Qt::RightButton) && (selected_building != -1)) {
        if (widgetRect.contains(event->pos())) {
            int fx = ((event->pos().x()-widgetRect.left()) / sf) / Tilesize;
            Building_info[selected_building].Properties->Units[fx] = 0xFF;
            buildingwindow_update_tiles(this);
            changes = true;
            emit BuildingChanged();
        }
    }
}

//replacewindow
void replacewindow::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        QRect widgetRect = Tile1->geometry();
        if (widgetRect.contains(event->pos())) {
            r1 = selected_tile;
            replacewindow_update(this);
        }
        widgetRect = Tile2->geometry();
        if (widgetRect.contains(event->pos())) {
            r2 = selected_tile;
            replacewindow_update(this);
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
        changes = true; //update global

        for (y = 0; y < Map.height; y++) {
            for (x = 0; x < Map.width; x++) {
                memcpy(&fielddata, Map.data + offset, sizeof(fielddata));
                if (fielddata.Part == r1) {
                    fielddata.Part = r2;
                    memcpy(Map.data + offset,&fielddata, sizeof(fielddata));
                }
                offset = offset + sizeof(Field);
            }
        }
        emit TilesReplaced(); //will trigger paint and changes update
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
      window.updateActions();
      if (Settings->value(REG_AUTOLOAD).toBool() == true) {
          window.Open_Map();
      }
    }

    if (restoreWindowPosAct->isChecked()) {
        window.restoreWindowPos();
    } else {
        //default as original
        window.resize(screenrect.width() / 2, screenrect.height() / 2);
        window.move(screenrect.left(), screenrect.top());
    }
    window.show();

    return app.exec();
}
