/* OTHER.H
 * Additional functions for the Hisotry Line Mapeditor. Draw hexagons, release memory, create child windows and dialogs, etc.
*/


//content for .TMP files,
typedef struct {
    int summer;
    int twoplayer;
} TMP_Rec;


void Release_Buffers()
//Memory cleanup...
{
    if (Map.data != NULL) free(Map.data);
    if (Partlib.data != NULL) free(Partlib.data);
    if (Unitlib.data != NULL) free(Unitlib.data);
    if (Building_info != NULL) free(Building_info);
    if (SHP.buildings != NULL) free(SHP.buildings);
    if (CODESDAT_buffer != NULL)free(CODESDAT_buffer);
    return;
}

void Check_used_tiles()
{
    /*
     * Unfortunately, the game only uses a 64 KB buffer for the terrain graphics.
     * Since all terrain graphics together are well over 64 KB in size, this results in a limitation when using the graphics for your own maps.
     * In addition, the first 25 graphics are always loaded into the memory, while others are only loaded if they are used on a map.
     *
     * 25*594 (unpacked size of a terrain tile) = 14.850 byte
     * 65.535 byte (64 K buffer) - 14.850 = 50.686
     * 50.686 / 594 = 85
     * So this results in 85 mathematically possible additional parts the game can handle.
     *
     * During testing, however, errors sometimes occurred a little earlier (perhaps the game also uses the memory area for some variables or similar).
     * Therefore we give a warning from 80 parts.
     */

    if (Settings->value(REG_SHOW_WARNINGS).toBool())
    {
        int upper_parts = 0;
        unsigned char used_parts[Num_Parts];
        memset(&used_parts,0,sizeof(used_parts));

        for (int offset = 0; offset < (Map.width*Map.height)*2; offset += 2) //Determine which tile graphics were used
        {
            if (Map.data[offset] != 0xAE)
            used_parts[Map.data[offset]] = 1;
        }

        for (int i = 0; i < Num_Parts; i++) //Check how many of these are extended graphics
        {
            if (used_parts[i] == 1)
            if (i > 24) upper_parts++;
        }

        if (upper_parts > 79)  //Issue a warning if 80 or more
        {
            show_warning("You have used more extended terrain tiles (tiles outside of grass level and building parts) than can fit in the game's memory buffer. This can lead to graphic errors and incorrect display of the map in the game.");
        }
    }
}


bool Check_for_game_files()
//Check whether game resources are available.
//For Linux compatibility, a separate check is also carried out for lower-case file names.
//dadk, why?
{
    QFile UnitdatFile(GameDir + Unitdat_name);
    QFile UnitlibFile(GameDir + Unitlib_name);
    QFile Partdat_S_File(GameDir + Partdat_S_name);
    QFile Partlib_S_File(GameDir + Partlib_S_name);
    QFile Partdat_W_File(GameDir + Partdat_W_name);
    QFile Partlib_W_File(GameDir + Partlib_W_name);
    QFile CodeFile = (GameDir + Code_name);

    QFile PalFile(GameDir + Palette_name);

    if (PalFile.exists() == false)
    {
        QFile PalFile(GameDir.toLower()+Palette_name.toLower());
        if (PalFile.exists() == false) return false;
    }
    if (UnitdatFile.exists() == false)
    {
        QFile UnitdatFile(GameDir.toLower()+Unitdat_name.toLower());
        if (UnitdatFile.exists() == false) return false;
    }
    if (UnitlibFile.exists() == false)
    {
        QFile UnitlibFile(GameDir.toLower()+Unitlib_name.toLower());
        if (UnitlibFile.exists() == false) return false;
    }
    if (Partdat_S_File.exists() == false)
    {
        QFile Partdat_S_File(GameDir.toLower()+Partdat_S_name.toLower());
        if (Partdat_S_File.exists() == false) return false;
    }
    if (Partlib_S_File.exists() == false)
    {
        QFile Partlib_S_File(GameDir.toLower()+Partlib_S_name.toLower());
        if (Partlib_S_File.exists() == false) return false;
    }
    if (Partdat_W_File.exists() == false)
    {
        QFile Partdat_W_File(GameDir.toLower()+Partdat_W_name.toLower());
        if (Partdat_W_File.exists() == false) return false;
    }
    if (Partlib_W_File.exists() == false)
    {
        QFile Partlib_W_File(GameDir.toLower()+Partlib_W_name.toLower());
        if (Partlib_W_File.exists() == false) return false;
    }
    if (CodeFile.exists() == false)
    {
        QFile CodeFile(GameDir.toLower()+Code_name.toLower());
        if (CodeFile.exists() == false) return false;
    }
   return true;
}


bool Read_Config()
//Reading the configuration file
{
    Settings = new QSettings("HL-Editor.INI", QSettings::IniFormat);

    GameDir = Settings->value(REG_GAMEDIR).toString();
    bool found = Check_for_game_files();
    if (found == true) MapDir = GameDir + MapDir;

    Scale_factor = Settings->value(REG_SCALE_FACTOR).toDouble();
    if (Scale_factor == 0) Scale_factor = 2;

    Scale_factor_locked = Settings->value(REG_LOCK_TILESIZE).toDouble();
    if (Scale_factor_locked > 0) lockWindowTilesizeAct->setChecked(true);

    restoreWindowPosAct->setChecked( Settings->value(REG_RESTORE_WINDOWS).toBool() );

    if (Settings->value(REG_AUTOLOAD).toBool() == true) {
          if (Settings->value(REG_RECENT_MAP).toString() != "") {
           Map_file = Settings->value(REG_RECENT_MAP).toString();
        }
    }

    return found;
}


int Load_Ressources()
//Loading game resources
{
   QString C_Filename1 = get_path(Unitlib_name);
   QString C_Filename2 = get_path(Unitdat_name);

   if (Load_Unit_files(C_Filename1.toStdString().data(), C_Filename2.toStdString().data()) != 0)
   {
     show_error("Error loading unit files!");
     Release_Buffers();
     return -1;
    }

    //Load part/terrain bitmaps
    if (summer == false)  //it's winter
    {
      C_Filename1 = get_path(Partlib_W_name);
      C_Filename2 = get_path(Partdat_W_name);
    }
    else  //it's summer
    {
      C_Filename1 = get_path(Partlib_S_name);
      C_Filename2 = get_path(Partdat_S_name);
    }

    if (Load_Part_files(C_Filename1.toStdString().data(), C_Filename2.toStdString().data()) != 0)
    {
      show_error("Error loading terrain files!");
      if (Partlib.data != NULL) free(Partlib.data);
      return -1;
    }

    //Load color palette
    C_Filename1 = get_path(Palette_name);
    if (Load_Palette(C_Filename1.toStdString().data()) != 0)
    {
      show_error("Error loading color palette!");
      Release_Buffers();
      return -1;
    }

    //Load levelcodes
    C_Filename1 = get_path(Code_name);
    if (Read_Codesdat(C_Filename1.toStdString().data()) != 0)
    {
      show_error("Error reading CODES.DAT!");
      Release_Buffers();
      return -1;
    }
    Get_Levelcodes();

    //Read unit names
    C_Filename1 = get_path(Unitdat2_name);
    if (Get_unit_data(C_Filename1.toStdString().data()) != 0)
    {
      show_error("Error reading unit data!");
      Release_Buffers();
      return -1;
    }
    Res_loaded = true;
    return 0;
}


bool Get_actual_map_options()
//Checks whether the actual map (by Map_file) is already integrated in the game
//and loads additional information if this is the case.
{
    if (!Res_loaded)
    {
        if (Load_Ressources() != 0) return false;
    }
    Actual_Level = "";
    Actual_Levelnum = -1;
    QFileInfo fileinfo(Map_file);

    if (fileinfo.path() == MapDir)  // We are in the game's map directory
    {

        QString name = fileinfo.baseName();
        bool name_valid;
        int filenumber = name.toInt(&name_valid,10); //Check whether the file name (without extension) contains only decimal numbers

        if (name_valid)
        {
            if (filenumber < Levelcode.Number_of_levels) //the filename contains a valid levelnumber
            {
               Actual_Level = Levelcode.Codelist[filenumber];
               Actual_Levelnum = filenumber;
               if (Get_Mapoptions(filenumber) == 0)  //There is data available
               {
                  return true;
               }
            }
        }
    }
    return false;
}


bool Save()
//Saves actual Mapdata
{
    FILE*        f;
    size_t       IO_result;

    if ((already_saved == false) || (Map_file == ""))
        Map_file = QFileDialog::getSaveFileName(0,"Save History Line 1914-1918 map file", MapDir, "HL map files (*.fin)");

    if (!Map_file.isEmpty() && !Map_file.isNull())
    {
        if ((!Map_file.contains(".fin")) && (!Map_file.contains(".FIN")))
            Map_file = Map_file+".FIN";

        if (Save_Mapdata(Map_file.toStdString().data()) != 0)
        {
            show_warning("Unable to save the map data in " + Map_file);
            return false;
        }
        QString SHPfile;
        SHPfile = Map_file;
        SHPfile.replace(".fin",".shp").replace(".FIN",".SHP");

        if (Create_shp(SHPfile.toStdString().data()) != 0)
        {
            show_warning("Cannot save the building data in " + SHPfile + "!");
            return false;
        }

        if (Get_actual_map_options()) //is this map already part of the game?
        {
            if (summer)
             CODESDAT_buffer[(Actual_Levelnum*10)+8] = 0;
            else
             CODESDAT_buffer[(Actual_Levelnum*10)+8] = 2;

            if (Player2)
             CODESDAT_buffer[(Actual_Levelnum*10)+6] = 2;
            else
             CODESDAT_buffer[(Actual_Levelnum*10)+6] = 1;

            QString C_Filename = get_path(Code_name);
            f = fopen(C_Filename.toStdString().data(), "wb");
            if (f == NULL)
            {
                show_error("Error writing to CODES.DAT!");
                return false;
            }

            IO_result = fwrite(CODESDAT_buffer, CODESDAT_size, 1, f); //Write buffer
            if (IO_result != 1)
            {
                fclose(f);
                show_error("Error writing to CODES.DAT!");
                return false; //Write Error
            }
            fclose(f);
        }
        else
        {
            TMP_Rec  tmprec;
            QString  TMPfile;

            tmprec.summer = summer ? 1 : 0;
            tmprec.twoplayer = Player2 ? 1 : 0;

            TMPfile  = Map_file;
            TMPfile.replace(".fin",".tmp").replace(".FIN",".TMP");
            f = fopen(TMPfile.toStdString().data(), "wb");
            if (f == NULL)
            {
               show_error("Error creating " + TMPfile);
               return false;
            }

            IO_result = fwrite(&tmprec, sizeof(TMP_Rec), 1, f);
            if (IO_result != 1)
            {
               fclose(f);
               show_error("Error writing to " + TMPfile);
               return false; //Write Error
            }

            fclose(f);
        }
        //changes = false; //All changes saved
        //already_saved = true;
        return true;
    }
    return false; //!?
}

TMP_Rec Load_Map()
//Load a map file
//Now returning TMP_Rec instead of int.  If the map is installed return season / twoplayer settings
//If not installed return content of .TMP file.
//If .TMP file is not found, return rec populated with -1 values, and by that defaults is set in the caller main.cpp Open_Map()
{
    QString      C_Filename;
    FILE*        f;
    size_t       IO_result;
    int          res;
    TMP_Rec      tmprec = {-1,-1};

    C_Filename = Map_file;
    res = Load_Mapdata(C_Filename.toStdString().data());
    if (res != 0)
    {
        show_error("Error loading map data!");
        Release_Buffers();
        return tmprec; //-1;
    }

    C_Filename.replace(".fin",".shp").replace(".FIN",".SHP");

    if (Read_shp_data(C_Filename.toStdString().data()) != 0)
    {
        show_warning("Cannot find building data for this map. " + C_Filename + "! corrupted or missing?");
        memset(SHP.can_be_built,0,sizeof(SHP.can_be_built));
        Building_info = NULL;
        Create_valid_building_record_from_map();
    }
    else
        Add_building_positions();


    if (Get_actual_map_options()) //Is this map already part of the game
    {
        if ((Mapoptions.season == 0) || (Mapoptions.season == 1))  //get season for this map from CODES.DAT
            tmprec.summer = 1;
        else
            tmprec.summer = 0;

        if (Mapoptions.map_type == 2) //get number of players
            tmprec.twoplayer = 1;
        else
            tmprec.twoplayer = 0;
    }
    else
    {
        C_Filename.replace(".shp",".tmp").replace(".SHP",".TMP");
        f = fopen(C_Filename.toStdString().data(), "rb");
        if (!f)
        {
            return tmprec; //{-1,-1}
        }
        IO_result = fread(&tmprec, sizeof(TMP_Rec), 1, f); //Read season etc
        fclose(f);
    }

    return tmprec;
}


void Draw_Hexagon(int x, int y, QPen Pen, QImage *Image, bool align, bool scaling, bool isChildWindow = false)
//dadk, added a isChildWindow flag, indicating if target is tilelist or unitlist
{
    double sf = isChildWindow && lockWindowTilesizeAct->isChecked() ? Settings->value(REG_LOCK_TILESIZE).toDouble() : Scale_factor;

    int xp,yp;
    QPainter painter(Image);
    painter.setPen(Pen);

    if (align)
    {
        xp = x * (Tilesize-Tileshift);
        if (x % 2 != 0)
            yp = (y * Tilesize) + (Tilesize / 2);
        else
            yp = (y * Tilesize) ;
    }
    else
    {
         xp = (x * Tilesize);
         yp = (y * Tilesize);
    }

    if (scaling)
    {
        xp = xp * sf;
        yp = yp * sf;

        painter.drawLine(xp,yp+((Tilesize / 2) * sf), xp + (Tileshift * sf), yp);
        painter.drawLine(xp+(Tileshift * sf), yp, xp + ((Tileshift *2 ) * sf), yp);
        painter.drawLine(xp+((Tileshift * 2) * sf),yp,xp+(Tilesize*sf),yp+((Tilesize/2)*sf));
        painter.drawLine(xp+(Tilesize*sf),yp+((Tilesize/2)*sf),xp+((Tileshift*2)*sf),yp+(Tilesize*sf));
        painter.drawLine(xp+(Tileshift*sf),yp+(Tilesize*sf),xp+((Tileshift*2)*sf),yp+(Tilesize*sf));
        painter.drawLine(xp,yp+((Tilesize/2)*sf),xp+(Tileshift*sf),yp+(Tilesize*sf));
    }
    else
    {
        painter.drawLine(xp,yp+(Tilesize/2),xp+Tileshift,yp);
        painter.drawLine(xp+Tileshift,yp,xp+(Tileshift*2),yp);
        painter.drawLine(xp+(Tileshift*2),yp,xp+Tilesize,yp+(Tilesize/2));
        painter.drawLine(xp+Tilesize,yp+(Tilesize/2),xp+(Tileshift*2),yp+Tilesize);
        painter.drawLine(xp+Tileshift,yp+Tilesize,xp+(Tileshift*2),yp+Tilesize);
        painter.drawLine(xp,yp+(Tilesize/2),xp+Tileshift,yp+Tilesize);
    }

    painter.end();
}


void Redraw_Field(int x, int y,int part, int unit)
{
    int xp,yp,side;

    xp = x * (Tilesize-Tileshift);

    if (x % 2 != 0)
        yp = (y * Tilesize) + (Tilesize / 2);
    else
        yp = (y * Tilesize) ;

    Draw_Part(xp,yp,part,&MapImage);

    if (unit != 0xFF)
    {
        if (unit % 2 == 0) side = 1; else side = 2;
        unit = (unit / 2) * 6;
        if (side == 1) unit = unit + 3;
        Draw_Unit(xp,yp, unit, side, &MapImage);
    }
}


void Change_Mapdata(int x, int y, unsigned char part, unsigned char unit)
{
    if ((x < (Map.width-1)) && (x >= 0) && (y < (Map.height-1)) && (y >= 0))  //Is the field on the map?
    {
        int offset;
        offset = ((y*Map.width)+x)*2;
        Map.data[offset] = part;
        Map.data[offset+1] = unit;
        Redraw_Field(x,y,part,unit);
    }
}


void ShowGrid()
// Draws a frame around each hex field to make them more visible
{
    if (Map.loaded == true)
    {
        int x,y;

        for (y = 0; y < (Map.height-1); y++)
        {
            for (x = 0; x < (Map.width-1); x++)
            {
                if (summer)
                Draw_Hexagon(x, y, QPen(Qt::white, 1), &MapImageScaled, true, true, false);
                else
                Draw_Hexagon(x, y, QPen(Qt::black, 1), &MapImageScaled, true, true, false);
            }
        }    
    }
}


void Create_Tileselection_window()
{
    double sf = lockWindowTilesizeAct->isChecked() ? Settings->value(REG_LOCK_TILESIZE).toDouble() : Scale_factor;

    if (Map.loaded == true)
    {
        QRect current_geometry; //if window is already visible, store the position and size
        if (tile_selection) current_geometry = tile_selection->geometry();

        tile_selection = new tilelistwindow();
        tile_selection->setWindowFlag(Qt::SubWindow);
        tile_selection->setWindowFlags(Qt::WindowStaysOnTopHint | Qt::WindowTitleHint | Qt::CustomizeWindowHint);
        tile_selection->setWindowTitle("Tile selection");

        tile_selection_title1 = new QLabel();
        tile_selection_title1->setMaximumHeight(20);
        tile_selection_title1->setText("Basic tiles:");

        BasicTileListImage = QImage((10*Tilesize),3*Tilesize, QImage::Format_RGB16); //Create a new QImage object for the tile list
        BasicTileListImage.fill(Qt::transparent);

        int tx = 0;
        int ty = 0;

        for (int tc = 0; tc < 25; tc++)
        {
            Draw_Part(tx*Tilesize, ty*Tilesize, tc, &BasicTileListImage); //Draw the bitmap
            tx++;
            if (tx == 10)
            {
                tx = 0;
                ty++;
            }
        }

        tile_selection_title2 = new QLabel();
        tile_selection_title2->setMaximumHeight(30);
        tile_selection_title2->setText("Extended tiles (only about 78 different ones can be used):"); //dadk, changed from 80 to 78
        tile_selection_title2->setWordWrap(true);
        ExtTileListImage = QImage((10*Tilesize),((Num_Parts-25)/10)*Tilesize, QImage::Format_RGB16); //Create a new QImage object for the tile list
        ExtTileListImage.fill(Qt::transparent);

        tx = 0;
        ty = 0;

        for (int tc = 25; tc < Num_Parts; tc++)
        {
// dadk, get potential glyphs for buttons
/* will be removed
            QImage color;
            color = QImage(Tilesize, Tilesize, QImage::Format_ARGB32_Premultiplied);
            color.fill(Qt::transparent);
            Draw_Part(0, 0, tc, &color);
            QString name = QString::fromLatin1(Partlib.Index[tc].RES_Name);
            name = name.remove(QRegExp("[^a-zA-Z\\d\\s]"));
            color.save("glyphs/tiles/" + name + "_color.PNG", "png", 100);

            QImage gs;
            gs = QImage(Tilesize, Tilesize, QImage::Format_ARGB32_Premultiplied);
            gs.fill(Qt::transparent);
            Draw_Part(0, 0, tc, &gs);
            //a little trick from https://stackoverflow.com/questions/42316844/convert-qimageicon-to-grayscale-format-while-keeping-background
            //otherwise it was impossible to keep transparency
            auto alphaChannel = gs.alphaChannel();
            gs.convertTo(QImage::Format_Grayscale16);
            gs.convertTo(QImage::Format_ARGB32);
            gs.setAlphaChannel(alphaChannel);
            gs.save("glyphs/tiles/" + name + "_grayscale.PNG", "png", 100);
*/
//org code from here

            Draw_Part(tx*Tilesize,ty*Tilesize,tc,&ExtTileListImage); //Draw the bitmap
            tx++;
            if (tx == 10)
            {
                tx = 0;
                ty++;
            }
        }

        BasicTileListImageScaled = BasicTileListImage.scaled(BasicTileListImage.width()*sf,BasicTileListImage.height()*sf); //Create a scaled version of the images
        ExtTileListImageScaled = ExtTileListImage.scaled(ExtTileListImage.width()*sf,ExtTileListImage.height()*sf); //Create a scaled version of it

        //Preselect first tile
        Draw_Hexagon(0, 0, QPen(Qt::red, 1), &BasicTileListImageScaled, false, true, true);
        selected_tile = 0;

        QVBoxLayout *layout = new QVBoxLayout();

        QLabel *label_basic = new QLabel();
        label_basic->setPixmap(QPixmap::fromImage(BasicTileListImageScaled));

        QLabel *label_ext = new QLabel();
        label_ext->setPixmap(QPixmap::fromImage(ExtTileListImageScaled));

        layout->addWidget(tile_selection_title1);

        BasicTilescrollArea = new(QScrollArea);
        BasicTilescrollArea->setBackgroundRole(QPalette::Dark);
        BasicTilescrollArea->setWidget(label_basic);
        BasicTilescrollArea->setVisible(true);
        BasicTilescrollArea->setMaximumHeight(BasicTileListImageScaled.height() + 4); //adjust max height

        layout->addWidget(BasicTilescrollArea);

        layout->addWidget(tile_selection_title2);

        ExtTilescrollArea = new(QScrollArea);
        ExtTilescrollArea->setBackgroundRole(QPalette::Dark);
        ExtTilescrollArea->setWidget(label_ext);
        ExtTilescrollArea->setVisible(true);
        layout->addWidget(ExtTilescrollArea);

        tile_selection->setLayout(layout);

        tile_selection->setMaximumWidth(label_ext->width() + (Tilesize * 1.3));
        tile_selection->setMaximumHeight(tile_selection_title1->height() +
                                         label_basic->height() +
                                         tile_selection_title2->height() +
                                         label_ext->height() +
                                         (Tilesize * 1.7));

        //restore geometry or position window as originally
        if (!current_geometry.isNull()) {
            tile_selection->setGeometry(current_geometry);
        } else {
            tile_selection->move(screenrect.width()/2, screenrect.top());
        }

        tile_selection->show();
    }
}


void Create_Unitselection_window()
{
    double sf = lockWindowTilesizeAct->isChecked() ? Settings->value(REG_LOCK_TILESIZE).toDouble() : Scale_factor;

    if (Map.loaded == true)
    {
        unit_selection = new unitlistwindow();
        unit_selection->setWindowFlag(Qt::SubWindow);
        unit_selection ->setWindowFlags(Qt::WindowStaysOnTopHint | Qt::WindowTitleHint | Qt::CustomizeWindowHint);

        unit_selection->resize(((11*Tilesize)*sf), (2*((Num_Units/10)+1)*Tilesize)*sf);
        unit_selection->setWindowTitle("Unit selection");
        unit_selection->setMouseTracking(true);

        unit_name_text = new QLabel();
        unit_name_text->setText("");

        UnitListImage = QImage((10*Tilesize), 2*((Num_Units/10)+1) * Tilesize, QImage::Format_RGB16); //Create a new QImage object for the tile list
        UnitListImage.fill(Qt::transparent);

        int tx = 0;
        int ty = 0;

        for (int tc = 0; tc < Num_Units; tc++)
        {
// dadk, get potential glyphs for buttons
/* will be removed
            QImage color;
            color = QImage(Tilesize, Tilesize, QImage::Format_ARGB32_Premultiplied);
            color.fill(Qt::transparent);
            Draw_Unit(0, 0, (tc*6)+5, 1, &color);
            QString name = Unit_Name[tc];
            name.replace(QChar('\0'), "");
            color.save("glyphs/units/" + name + "_color.PNG", "png", 100);

            QImage gs;
            gs = QImage(Tilesize, Tilesize, QImage::Format_ARGB32_Premultiplied);
            gs.fill(Qt::transparent);
            Draw_Unit(0, 0, (tc*6)+5, 1, &gs);
            auto alphaChannel = gs.alphaChannel();
            gs.convertTo(QImage::Format_Grayscale16);
            gs.convertTo(QImage::Format_ARGB32);
            gs.setAlphaChannel(alphaChannel);
            gs.save("glyphs/units/" + name + "_grayscale.PNG", "png", 100);
*/
//org code from here
            Draw_Unit(tx*Tilesize, ty*Tilesize, (tc*6)+3, 1, &UnitListImage); //+1,2,3,4,5,6

            tx++;
            if (tx == 10)
            {
                tx = 0;
                ty++;
            }
        }
        ty++;
        tx=0;
        for (int tc = 0; tc < Num_Units; tc++)
        {
            Draw_Unit(tx*Tilesize, ty*Tilesize, tc*6, 2, &UnitListImage);
            tx++;
            if (tx == 10)
            {
                tx = 0;
                ty++;
            }
        }

        UnitListImageScaled = UnitListImage.scaled(UnitListImage.width()*sf,UnitListImage.height()*sf); //Create a scaled version of it

        QLabel *label = new QLabel();
        label->setPixmap(QPixmap::fromImage(UnitListImageScaled));

        unitscrollArea = new(QScrollArea);
        unitscrollArea->setBackgroundRole(QPalette::Dark);
        unitscrollArea->setWidget(label);
        unitscrollArea->setVisible(true);

        QVBoxLayout *layout = new QVBoxLayout();
        layout->addWidget(unitscrollArea);
        layout->addWidget(unit_name_text);
        unit_selection->setLayout(layout);

        unit_selection->setMaximumHeight(label->height() + (Tilesize * 2));
        unit_selection->setMaximumWidth(label->width() + (Tilesize * 1.2));

        unit_selection->move(screenrect.width()/2, screenrect.bottom()/2);
        unit_selection->show();
    }
}


void Create_buildable_units_window()
{
    double sf = lockWindowTilesizeAct->isChecked() ? Settings->value(REG_LOCK_TILESIZE).toDouble() : Scale_factor;

    buildable = new buildablewindow();
    buildable->setWindowFlag(Qt::SubWindow);
    buildable->setWindowFlags(Qt::WindowStaysOnTopHint);
    buildable->resize(((11*Tilesize)*sf)+10, (((Num_Units/10)+1)*Tilesize)*sf);
    buildable->setWindowTitle("Units buildable in factories");

    BuildableImage = QImage((10*Tilesize),((Num_Units/10)+1)*Tilesize, QImage::Format_RGB16); //Create a new QImage object
    BuildableImage.fill(Qt::transparent);


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

    BuildableImageScaled = BuildableImage.scaled(BuildableImage.width() * sf, BuildableImage.height() * sf); //Create a scaled version of it

    QLabel *label = new QLabel();
    label->setPixmap(QPixmap::fromImage(BuildableImageScaled));

    buildablescrollArea = new(QScrollArea);
    buildablescrollArea->setBackgroundRole(QPalette::Dark);
    buildablescrollArea->setWidget(label);
    buildablescrollArea->setVisible(true);

    buildable_unitname = new QLabel();
    buildable_unitname->setText("");

    QVBoxLayout *layout = new QVBoxLayout();
    layout->addWidget(buildablescrollArea);
    layout->addWidget(buildable_unitname);

    buildable->setLayout(layout);

    buildable->show();
}


void Create_building_configuration_window()
{
    if (selected_building == -1)
    {
        show_error("There is no data record for this building!");
        return;
    }
    else
    {
        building_window = new buildingwindow();
        building_window->setWindowFlag(Qt::SubWindow);
        building_window->setWindowFlags(Qt::WindowStaysOnTopHint);

        QString Building_title = "Properties of ";
        switch (Building_info[selected_building].Properties->Owner)
        {
            case 0:
            Building_title += "German ";
            break;

            case 1:
            Building_title += "French ";
            break;

            case 2:
            Building_title += "Neutral ";
            break;
        }

        switch (Building_info[selected_building].Properties->Type)
        {
            case 0:
            Building_title += "Headquarter";
            break;

            case 1:
            Building_title += "Factory";
             break;

            case 2:
            Building_title += "Depot ";
            break;

            case 3:
            Building_title += "Transport Unit ";
            break;
        }

        building_window->setWindowTitle(Building_title);
        Building_Image = QImage((7*Tilesize),Tilesize+1, QImage::Format_RGB16); //Create a new QImage object
        Building_Image.fill(Qt::transparent);

        for (int i = 0; i < 7; i++)
        {
            if (Building_info[selected_building].Properties->Units[i] != 0xFF)
                Draw_Unit(i * Tilesize, 0,(Building_info[selected_building].Properties->Units[i] * 6) , Building_info[selected_building].Properties->Owner+1, &Building_Image);
        }

        Building_Image_Scaled = Building_Image.scaled(Building_Image.width() * Scale_factor, Building_Image.height() * Scale_factor); //Create a scaled version of it

        for (int i = 0; i < 7; i++)
        {
            QPainter painter(&Building_Image_Scaled);
            QPen pen;
            pen.setWidth(1);
            pen.setColor(Qt::white);
            painter.setPen(pen);
            QRect R((i * Tilesize) * Scale_factor, 0, ((i * Tilesize) + Tilesize) * Scale_factor, Building_Image_Scaled.height()-1);
            painter.drawRect(R);
            painter.end();
        }

        QLabel *textlabel1 = new QLabel();
        textlabel1->setText("Units in the building:");
        QLabel *textlabel2 = new QLabel();
        textlabel2->setText("Resources generated by this building per turn:");

        QLabel *bitmaplabel = new QLabel();
        bitmaplabel->setPixmap(QPixmap::fromImage(Building_Image_Scaled));

        Building_ScrollArea = new(QScrollArea);
        Building_ScrollArea->setBackgroundRole(QPalette::Dark);
        Building_ScrollArea->setWidget(bitmaplabel);
        Building_ScrollArea->resize(Building_Image_Scaled.width(),Building_Image_Scaled.height());
        Building_ScrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        Building_ScrollArea->verticalScrollBar()->hide();
        Building_ScrollArea->verticalScrollBar()->resize(0, 0);
        Building_ScrollArea->setVisible(true);

        RessourceEdit = new QLineEdit;
        RessourceEdit->clear();
        RessourceEdit->setValidator( new QIntValidator(0, 255) );
        RessourceEdit->setText(QString::number(Building_info[selected_building].Properties->Resources));  //Bisherigen wert anzeigen
        Update_Ressources = true;

        QPushButton *Button = new QPushButton("Save changes");
        Button->setGeometry(QRect(QPoint(100, 100), QSize(200, 50)));
        Button->setStyleSheet("background-color: rgb(240,240,240)");
        QObject::connect(Button, &QPushButton::clicked, [=]()
        {
            if (selected_building != -1)
            {
                if ((RessourceEdit->text().toInt() != Building_info[selected_building].Properties->Resources) && (Update_Ressources))
                    Building_info[selected_building].Properties->Resources = RessourceEdit->text().toInt();
            }
            building_window->close();
        });

        QVBoxLayout *layout = new QVBoxLayout();
        layout->addWidget(textlabel1);
        layout->addWidget(Building_ScrollArea);
        layout->addWidget(textlabel2);
        layout->addWidget(RessourceEdit);
        layout->addWidget(Button);

        building_window->setLayout(layout);
        building_window->show();
    }
}


void Create_replace_tile_diag()
{
    double sf = lockWindowTilesizeAct->isChecked() ? Settings->value(REG_LOCK_TILESIZE).toDouble() : Scale_factor;

    replace_accepted = false;
    r1 = selected_tile;
    r2 = 0;

    replacedlg = new replacewindow();
    replacedlg->setWindowFlag(Qt::SubWindow);
    replacedlg->setWindowFlags(Qt::WindowStaysOnTopHint);
    replacedlg->resize(((2*Tilesize) * sf) + 20, Tilesize*sf);
    replacedlg->setWindowTitle("Replace tiles");

    QLabel *textlabel1 = new QLabel();
    textlabel1->setText("Replace");
    QLabel *textlabel2 = new QLabel();
    textlabel2->setText("with");

    tile_image1 = QImage(Tilesize,Tilesize, QImage::Format_RGB16); //Create a new QImage object
    tile_image1.fill(Qt::transparent);
    Draw_Part(0,0,r1,&tile_image1);
    tile_image2 = QImage(Tilesize,Tilesize, QImage::Format_RGB16); //Create a new QImage object
    tile_image2.fill(Qt::transparent);
    Draw_Part(0,0,r2,&tile_image2);

    tile_image1 = tile_image1.scaled(tile_image1.width()*sf,tile_image1.height()*sf); //scale it
    tile_image2 = tile_image2.scaled(tile_image2.width()*sf,tile_image2.height()*sf); //scale it

    Tile1 = new QLabel();
    Tile1->setPixmap(QPixmap::fromImage(tile_image1));
    Tile2 = new QLabel();
    Tile2->setPixmap(QPixmap::fromImage(tile_image2));

    QPushButton *okButton = new QPushButton("OK");
    QObject::connect(okButton, &QPushButton::clicked, [=]()
                     {
                         replace_accepted = true;
                         replacedlg->close();
                     });

    QHBoxLayout *layout = new QHBoxLayout();
    layout->addWidget(textlabel1);
    layout->addWidget(Tile1);
    layout->addWidget(textlabel2);
    layout->addWidget(Tile2);
    layout->addWidget(okButton);

    replacedlg->setLayout(layout);
    replacedlg->show();
}


// Returns Checksum of a file or empty QByteArray() on failure.
QByteArray fileChecksum(const QString &fileName,
                        QCryptographicHash::Algorithm hashAlgorithm)
{
    QFile f(fileName);
    if (f.open(QFile::ReadOnly))
    {
        QCryptographicHash hash(hashAlgorithm);
        if (hash.addData(&f))
        {
            return hash.result();
        }
    }
    return QByteArray();
}

//dadk
void place_mountain_on_map(QPoint h)
{
/*   Green mountain    brown mountain
 *       0x43              0x47
 *    0x45  0x46        0x49  0x4A
 *       0x44              0x48
 */
    switch ((unsigned char)selected_tile) {
      //green mountain
      case 0x43 : //67, green mountain top
        if (h.x() % 2 == 0) {
          Change_Mapdata(h.x(), h.y(), 0x43, 0xFF);
          Change_Mapdata(h.x()-1, h.y(), 0x45, 0xFF);
          Change_Mapdata(h.x()+1, h.y(), 0x46, 0xFF);
          Change_Mapdata(h.x(), h.y()+1, 0x44, 0xFF);
        } else {
          Change_Mapdata(h.x(), h.y(), 0x43, 0xFF);
          Change_Mapdata(h.x()-1, h.y()+1, 0x45, 0xFF);
          Change_Mapdata(h.x()+1, h.y()+1, 0x46, 0xFF);
          Change_Mapdata(h.x(), h.y()+1, 0x44, 0xFF);
        }
        break;
      case 0x44 : //68, green mountain bottom
        if (h.x() % 2 == 0) {
          Change_Mapdata(h.x(), h.y(), 0x44, 0xFF);
          Change_Mapdata(h.x()-1, h.y()-1, 0x45, 0xFF);
          Change_Mapdata(h.x()+1, h.y()-1, 0x46, 0xFF);
          Change_Mapdata(h.x(), h.y()-1, 0x43, 0xFF);
        } else {
          Change_Mapdata(h.x(), h.y(), 0x44, 0xFF);
          Change_Mapdata(h.x()-1, h.y(), 0x45, 0xFF);
          Change_Mapdata(h.x()+1, h.y(), 0x46, 0xFF);
          Change_Mapdata(h.x(), h.y()-1, 0x43, 0xFF);
        }
        break;
      case 0x45 : //69, green mountain left
        if (h.x() % 2 == 0) {
          Change_Mapdata(h.x(), h.y(), 0x45, 0xFF);
          Change_Mapdata(h.x()+1, h.y()-1, 0x43, 0xFF);
          Change_Mapdata(h.x()+1, h.y(), 0x44, 0xFF);
          Change_Mapdata(h.x()+2, h.y(), 0x46, 0xFF);
        } else {
          Change_Mapdata(h.x(), h.y(), 0x45, 0xFF);
          Change_Mapdata(h.x()+1, h.y(), 0x43, 0xFF);
          Change_Mapdata(h.x()+1, h.y()+1, 0x44, 0xFF);
          Change_Mapdata(h.x()+2, h.y(), 0x46, 0xFF);
        }
        break;
      case 0x46 : //70, green mountain right
        if (h.x() % 2 == 0) {
          Change_Mapdata(h.x(), h.y(), 0x46, 0xFF);
          Change_Mapdata(h.x()-1, h.y(), 0x44, 0xFF);
          Change_Mapdata(h.x()-2, h.y(), 0x45, 0xFF);
          Change_Mapdata(h.x()-1, h.y()-1, 0x43, 0xFF);
        } else {
          Change_Mapdata(h.x(), h.y(), 0x46, 0xFF);
          Change_Mapdata(h.x()-1, h.y(), 0x43, 0xFF);
          Change_Mapdata(h.x()-1, h.y()+1, 0x44, 0xFF);
          Change_Mapdata(h.x()-2, h.y(), 0x45, 0xFF);
        }
       break;
    // brown mountain
    case 0x47 : //71, brown mountain left
       if (h.x() % 2 == 0) {
          Change_Mapdata(h.x(), h.y(), 0x47, 0xFF);
          Change_Mapdata(h.x()-1, h.y(), 0x49, 0xFF);
          Change_Mapdata(h.x()+1, h.y(), 0x4A, 0xFF);
          Change_Mapdata(h.x(), h.y()+1, 0x48, 0xFF);
       } else {
          Change_Mapdata(h.x(), h.y(), 0x47, 0xFF);
          Change_Mapdata(h.x()-1, h.y()+1, 0x49, 0xFF);
          Change_Mapdata(h.x()+1, h.y()+1, 0x4A, 0xFF);
          Change_Mapdata(h.x(), h.y()+1, 0x48, 0xFF);
       }
       break;
    case 0x48 : //72, brown mountain bottom
      if (h.x() % 2 == 0) {
          Change_Mapdata(h.x(), h.y(), 0x48, 0xFF);
          Change_Mapdata(h.x()-1, h.y()-1, 0x49, 0xFF);
          Change_Mapdata(h.x()+1, h.y()-1, 0x4A, 0xFF);
          Change_Mapdata(h.x(), h.y()-1, 0x47, 0xFF);
      } else {
          Change_Mapdata(h.x(), h.y(), 0x48, 0xFF);
          Change_Mapdata(h.x()-1, h.y(), 0x49, 0xFF);
          Change_Mapdata(h.x()+1, h.y(), 0x4A, 0xFF);
          Change_Mapdata(h.x(), h.y()-1, 0x47, 0xFF);
      }
      break;
    case 0x49 : //73, brown mountain left
      if (h.x() % 2 == 0) {
          Change_Mapdata(h.x(), h.y(), 0x49, 0xFF);
          Change_Mapdata(h.x()+1, h.y()-1, 0x47, 0xFF);
          Change_Mapdata(h.x()+1, h.y(), 0x48, 0xFF);
          Change_Mapdata(h.x()+2, h.y(), 0x4A, 0xFF);
      } else {
          Change_Mapdata(h.x(), h.y(), 0x49, 0xFF);
          Change_Mapdata(h.x()+1, h.y(), 0x47, 0xFF);
          Change_Mapdata(h.x()+1, h.y()+1, 0x48, 0xFF);
          Change_Mapdata(h.x()+2, h.y(), 0x4A, 0xFF);
      }
      break;
    case 0x4A : //74, brown mountain right
       if (h.x() % 2 == 0) {
          Change_Mapdata(h.x(), h.y(), 0x4A, 0xFF);
          Change_Mapdata(h.x()-1, h.y(), 0x48, 0xFF);
          Change_Mapdata(h.x()-2, h.y(), 0x49, 0xFF);
          Change_Mapdata(h.x()-1, h.y()-1, 0x47, 0xFF);
       } else {
          Change_Mapdata(h.x(), h.y(), 0x4A, 0xFF);
          Change_Mapdata(h.x()-1, h.y(), 0x47, 0xFF);
          Change_Mapdata(h.x()-1, h.y()+1, 0x48, 0xFF);
          Change_Mapdata(h.x()-2, h.y(), 0x49, 0xFF);
       }
      break;

    default :
      break;
   }
}
