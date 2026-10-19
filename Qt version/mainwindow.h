
#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

QT_BEGIN_NAMESPACE
class QScrollArea;
class QScrollBar;
class QLabel;
class QToolButton;
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow();

    void Open_Map();
    void setPath_diag();
    void zoom(bool in = NULL);
    void restoreWindowPos();
    void Paint_Map(bool refresh = false);
    void Repaint_Map(QPoint selection);
    void updateRecentFiles();
    void updateActions();

public slots:
    void update_map(); //update the map from other windows
    void update_changes_state(); //update changes state from other windows
    void update_deselect_button(); //update changes state from other windows

protected:
#ifndef QT_NO_CONTEXTMENU
    void contextMenuEvent(QContextMenuEvent *event) override;
#endif // QT_NO_CONTEXTMENU
    void showEvent (QShowEvent *event) override;
    void closeEvent (QCloseEvent *event) override;
    void mousePressEvent (QMouseEvent *event) override;
    void mouseMoveEvent (QMouseEvent *event) override;
    void mouseReleaseEvent (QMouseEvent *event) override;
    void mouseDoubleClickEvent (QMouseEvent *event) override;
    void resizeEvent (QResizeEvent*) override;
    void moveEvent (QMoveEvent*) override;
    void keyPressEvent(QKeyEvent* event) override;

private slots:
    void newFile_diag();
    void open_diag();
    void open_by_code_diag();
    void save_diag();
    void saveas_diag();
    void saveimage_diag();
    void add_diag();
    void remove_diag();
    void close_diag();
    void grid_diag();
    void coords_diag();
    void tilewindow_diag();
    void unitwindow_diag();
    void map_resize_diag();
    void season_diag();
    void replace_diag();
    void buildable_units_diag();
    void setScale_diag();
    void mapInfoGeneral_diag(); //renamed, was statistics_diag
    void mapInfoUnits_diag();
    void warning_diag();
    void maptype_diag();

private:
    void createActions();
    void createMenus();
    void adjustScrollBar(QScrollBar *scrollBar, double factor); //???

    void createToolbar();
    void updateToolbarIcons();
    void Create_Tileselection_window();
    void update_Scale_factor();
    void remove_level(QString R_levelcode, bool silent = false);
    void update_window_title();
    void saveWindowPos();
    void init_selecting_path(int target);
    void set_changes_state(bool state);

    QMenu *fileMenu;
    QMenu *editMenu;
    QMenu *configMenu;
    QAction *newAct;
    QAction *openAct;
    QAction *openbyCodeAct;
    QAction *saveAct;
    QAction *saveasAct;
    QAction *saveImageAct;
    QAction *addtogameAct;
    QAction *removefromgameAct;
    QAction *closeAct;
    QAction *exitAct;
    QAction *setPathAct;
    QAction *setScaleFactorAct;
    QAction *showtilewindowAct;
    QAction *showunitwindowAct;
    QAction *mapresizeAct;
    QAction *changeseasonAct;
    QAction *replaceAct;
    QAction *buildableunitsAct;
    QAction *mapInfoGeneralAct; //renamed, was statisticsAct
    QAction *mapInfoUnitsAct;
    QAction *maptypeAct;
    QLabel  *infoLabel;

    QAction *autoloadAct;
    QAction *warningAct;
    QAction *showgridAct;
    QAction *showcoordsAct;

    QAction *selectionModeAct;

    QAction *autogenDenseForestAct;
    QAction *autogenLightForestAct;
    QAction *autogenPlainsAct;
    QAction *autogenWastelandAct;
    QAction *autogenResidentalAreaAct;
    QAction *autogenBattlefieldAct;
    QAction *autogenLakeAct;
    QAction *autogenIslandAct;

    QAction *autogenPathNormalAct; //hexPath
    QAction *autogenPathStraightAct;
    QAction *autogenPathWavyAct;
    QAction *autogenPathPreferDirectionAct;
    QAction *autogenPathScenicAct;
    QAction *autogenPathRandomDetourAct;

    QAction *autogenPathStraight0Act;
    QAction *autogenPathStraight2Act;
    QAction *autogenPathStraight3Act;

    QAction *nocrossBuildingAct;
    QAction *nocrossMountainAct;
    QAction *nocrossWaterAct;
    QAction *nocrossHillAct;
    QAction *nocrossReefAct;
    QAction *nocrossBunkerAct;
    QAction *nocrossTrenchAct;
    QAction *nocrossForestAct;
    QAction *nocrossRiverAct;
    QAction *nocrossResidentalsAct;
    QAction *nocrossDirtroadAct;
    QAction *nocrossRoadAct;
    QAction *nocrossRailroadAct;

    //Toolbar
    QToolButton *tb_deselect;
    QToolButton *tb_open_file;
    QToolButton *tb_save_changes;
    QToolButton *tb_map_info;
    QToolButton *tb_replace_tile;
    QToolButton *tb_zoom_in;
    QToolButton *tb_zoom_out;

    QToolButton *tb_move_tl;
    QToolButton *tb_move_tr;
    QToolButton *tb_move_bl;
    QToolButton *tb_move_br;

    QToolButton *tb_tile_window;
    QToolButton *tb_unit_window;

    QToolButton *tb_selection_mode;
    QToolButton *tb_autogen_fill;
    QToolButton *tb_autogen_path;

    QToolButton *tb_autogen_dirtroad;
    QToolButton *tb_autogen_road;
    QToolButton *tb_autogen_railroad;
    QToolButton *tb_autogen_river;
    QToolButton *tb_autogen_trench;
    QToolButton *tb_autogen_coastRightTop;
    QToolButton *tb_autogen_coastLeftBottom;

    QToolButton *tb_autogen_options;
};

#endif
