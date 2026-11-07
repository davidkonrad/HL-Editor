
/*
LIB.H by Jan Knipperts, David Konrad
Provides all the functions to load bitmaps from the ressurce files of the game Historyline 1914-1918.
Version 1.3 - 11.12.2023
- Fixed a bug in TPWM_unpack that caused writing outside the buffer in some cases.

The main features:

TPWM_Unpack / Unpack_file
Unpacks data that is compressed with Turbo Packer

Load_Palette
Loads Hisotry Line 1914-1918 palette files to "HL_Palette"

Load_Part_files
Loads all terrain and building data from the game files into memory

Load_Unit_files
Loads all unit data from game files into memory

Draw_Part
Draws the given unit to a QImage named "MapImage" that is used as screen buffer

Draw_Unit
Draws the given unit to a QImage named "MapImage" that is used as screen buffer


*/
#ifndef LIB_H
#define LIB_H


#include <iostream>
#include <string>
#include <sstream>
#include <QImage>
#include <dialogs.h>


inline const int           Num_Parts = 175; //Number of HL terrain parts
inline const int           Num_Units = 51;  // Number of HL Units

inline const int           Tilesize = 24;	//Ein Hexfeld hat einen Durchmesser von 24 Pixeln
inline const int           Tileshift = 8;	//Versatz der Hexfelder

struct Index_Entry
{
    char                    RES_Name[8];
    unsigned int            Offset;
};

struct Bitmap_Header
{
    char                    id[8];
    unsigned char			transparent;
    unsigned char			type;
    unsigned short int		pad_word1;  //Seems to be always 0
    unsigned short int		pad_word2;  //Seems to be always 0
    unsigned short int		width;
    unsigned short int		height;
};

struct Palette_Entry
{
    unsigned char           Red;
    unsigned char           Green;
    unsigned char           Blue;
};

struct Partlib_Infos
{
    unsigned int			data_size;
    unsigned char*			data;
    unsigned int 			index_offset;
    Index_Entry             Index[Num_Parts];
};

struct Unitlib_Infos
{
    unsigned int			data_size;
    unsigned char*			data;
    unsigned int 			index_offset;
    Index_Entry             Index[Num_Units*6];//6 bitmaps of unit in different angels
};

struct TPWM_Infos
{
    unsigned int			unpacked_size;
    unsigned int			packed_size;
    unsigned char*			packed_data;
    unsigned char*			unpacked_data;
};

struct Partdat_Infos
{
    char                    name[Num_Parts][8];
};

struct Unitdat_Infos
{
    char                    name[Num_Units*6][8]; //6 bitmaps of unit in different angels
};

extern Partlib_Infos               Partlib;
extern Unitlib_Infos               Unitlib;
extern Partdat_Infos               Partdat;
extern Unitdat_Infos               Unitdat;
extern TPWM_Infos                  TPWM;
extern unsigned int				 ID;

extern int Unpack_file(FILE* f);

extern int Draw_Part(int xpos, int ypos, int part_num, QImage *Image);
extern int Draw_Unit(int xpos, int ypos, int unit_num, int side, QImage *Image);
extern void Draw_Hexagon(int x, int y, QPen Pen, QImage *Image, bool align, bool scaling, double sf);

extern int Load_Unit_files(std::string unitlib_filename,std::string unitdat_filename);
extern int Load_Part_files(std::string partlib_filename,std::string partdat_filename);
extern int Load_Palette(std::string pal_filename);

extern int getbit(char n, int bitwanted);
extern std::string char2string(char* a, int size);

extern QImage grayscale_transparent_image(QImage image); //not really related, but best place for the function
extern QImage get_part_image(int part, bool grayscale = false);
extern QIcon get_part_icon(int part, bool grayscale = false);

#endif // LIB_H
