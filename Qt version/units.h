/*
UNITS.H by Jan Knipperts
Reads unit names from UNITS.DAT of History Line 1914-1918
Version 1.1 - 07.01.2024

Added loading of accessible fields for each unit.

Info:
The UNIT.DAT file in HL's main directory contains information of all units (name, group size, attack value etc).
It seems that there is a 69 Byte chunk for each unit. The unit's name is stored as 0 terminated string at offset 0x2C of each chunk and
the fields on which a unit can move is stored in one byte at offset 0x5.
The coding of the accessible fields is as follows:
Bit 0: Water
Bit 1: Railroad tracks
Bit 2: shallow water
Bit 3: Trenches/barbed wire entanglements
Bit 4: Plain, road/bridge
Bit 5: Forest
Bit 6: Mountains, narrow bridges
Bit 7 is not used.
*/


#ifndef UNITS_H
#define UNITS_H


inline QString Unit_Name[Num_Units];
inline unsigned char Unit_accessible_terrain[Num_Units];

extern int Get_unit_data(std::string unitfilename);

//dadk
struct Unit_Info_Rec {
    QString   name;
    int       index;
    int       weight;
    int       resources;
    bool      allowSupplyCar;
    bool      allowTrainShip;
};
extern struct Unit_Info_Rec Unit_Info[];


#endif // UNITS_H

