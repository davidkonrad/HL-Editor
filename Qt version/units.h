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

QString Unit_Name[Num_Units];
unsigned char Unit_accessible_terrain[Num_Units];

int Get_unit_data(std::string unitfilename)
{
    FILE*                   f;
    size_t                  IO_result;
    unsigned int            fsize;
    unsigned int			ID;
    unsigned int            i;
    unsigned int            offset;
    unsigned char           c;

    memset(&Unit_accessible_terrain,0,sizeof(Unit_accessible_terrain));

    //fopen_s(&f, unitfilename.data(), "rb");
    //if (!f) return -1; //File not found
    f = fopen(unitfilename.data(), "rb");
    if (f == NULL) return -1; //File not found

    IO_result = fread(&ID, sizeof(ID), 1, f); //Read ID

    if (IO_result != 1)
    {
        fclose(f);
        return -2; //Read Error
    }

    fseek(f, 0, SEEK_END);	//Get file size
    fsize = ftell(f);

    if (ID == 0x4D575054) //"TPWM" string - file is compressed
    {
        if (Unpack_file(f) != 0)
        {
            free(TPWM.unpacked_data);
            return -5;		//Decompressing failed!
        }
        fsize = TPWM.unpacked_size;

        if (fsize != (Num_Units*0x45)) return -6; //File is corrupt

        for (i = 0; i < Num_Units; i++)
        {
            //Get accessible fields for unit            
            Unit_accessible_terrain[i] = (unsigned char) TPWM.unpacked_data[(i*0x45)+5];

            //Get unit name string
            offset = (i*0x45)+0x2C;
            c = 1;
            Unit_Name[i] = "";
            while ((c > 0) && (offset <= (i*0x45)+0x45)) //Unit names are stored as 0 terminated strings in the file
            {
                c = (unsigned char) TPWM.unpacked_data[offset];

                switch (c)      //Handle german Umlaute...
                {
                case 0x8E:
                    Unit_Name[i] = Unit_Name[i] + "AE";
                    break;

                case 0x99:
                    Unit_Name[i] = Unit_Name[i] + "OE";
                    break;

                case 0x9A:
                    Unit_Name[i] = Unit_Name[i] + "UE";
                    break;

                default:
                    Unit_Name[i] = Unit_Name[i] + QChar::fromLatin1(c);
                    break;
                }

                offset++;
            }
        }

    }
    else
    {
        if (fsize != (Num_Units*0x45)) //File is corrupt
        {
            fclose(f);
            return -6;
        }

        for (i = 0; i < Num_Units; i++)
        {

            //Get accessible fields for unit
            fseek(f,(i*0x45)+5, SEEK_SET);
            IO_result = fread(&Unit_accessible_terrain[i],1, 1, f);
            if (IO_result != 1) //Read Error?
            {
            fclose(f);
            return -2;
            }

            //Get unit name
            fseek(f,(i*0x45)+0x2C, SEEK_SET);
            c = 1;
            Unit_Name[i] = "";
            while (c > 0)  //Unit names are stored as 0 terminated strings in the file
            {
                IO_result = fread(&c,1, 1, f);
                if (IO_result != 1) //Read Error?
                {
                    fclose(f);
                    return -2;
                }

                switch (c)
                {
                case 0x8E:
                    Unit_Name[i] = Unit_Name[i] + "AE";
                    break;

                case 0x99:
                    Unit_Name[i] = Unit_Name[i] + "OE";
                    break;

                default:
                    Unit_Name[i] = Unit_Name[i] + QChar::fromLatin1(c);
                    break;
                }

            }
        }
        fclose(f);
    }

    return 0;
}

//dadk
struct Unit_Info_Rec {
    QString   name;
    int       index;
    int       weight;
    int       resources;
    bool      allowSupplyCar;
    bool      allowTrainShip;
};

struct Unit_Info_Rec Unit_Info[] = {
    //name                 #no wgt res  supply trans
    { "ARMOURED CAR",       0,  3,  55, false, true  },
    { "A7V TANK",           1,  5,  90, false, true  },
    { "GOTHA BOMBER",       2,  0, 100, false, false },
    { "ZEPPELIN STAAKEN",   3,  0, 115, false, false },
    { "JUNKERS J4-10",      4,  0,  90, false, false },
    { "FOKKER E I.",        5,  0,  80, false, false },
    { "FOKKER E III.",      6,  0,  85, false, false },
    { "ALBATROS",           7,  0,  90, false, false },
    { "FOKKER DR I.",       8,  0,  92, false, false },
    { "FOKKER D VII",       9,  0,  95, false, false },
    { "LIGHT ARTILLERY",   10,  3,  55, true,  true  },
    { "MEDIUM ARTILLERY",  11,  4,  70, true,  true  },
    { "HEAVY ARTILLERY",   12,  5,  85, true,  true  },
    { "BUNKER",            13,  0, 255, false, false },
    { "MOB AA-EMPLACEM.",  14,  4,  62, false, true  },
    { "STAT AA-EMPLACEM",  15,  2,  50, true,  true  },
    { "CONSTR. UNIT",      16,  4, 105, false, true  },
    { "ELITE INFANTRY",    17,  1,  50, true,  true  },
    { "INFANTRY",          18,  1,  35, true,  true  },
    { "ANTI-TANK GUNS",    19,  2,  53, true,  true  },
    { "SAPPERS",           20,  1,  45, true,  true  },
    { "CAVALRY",           21,  2,  50, false, true  },
    { "SUPPLY CAR",        22,  1,  45, false, true  }, // 9/4 in manual = 9 capacity, max unit weight 4
    { "STATIC BALLOON",    23,  1,  30, false, false },
    { "TRAIN ARTILLERY",   24, 35, 125, false, false },
    { "ARMOURED TRAIN",    25, 40, 100, false, false },
    { "SUPPLY TRAIN",      26,  1,  90, false, false }, // 35/15
    { "PATROL BOAT",       27,  0, 255, false, false },
    { "TORPEDO BOAT",      28,  0, 255, false, false },
    { "SUBMARINE",         29,  0, 255, false, false },
    { "SUBMARINE",         30,  0, 255, false, false },
    { "TRANSPORT SHIP",    31,  0, 255, false, false },
    { "DESTROYER",         32,  0, 255, false, false },
    { "DESTROYER",         33,  0, 255, false, false },
    { "BATTLESHIP",        34,  0, 255, false, false },
    { "BATTLESHIP",        35,  0, 255, false, false },
    { "VOISIN III",        36,  0,  85, false, false },
    { "HANDLEY PAGE",      37,  0, 115, false, false },
    { "D.H.4",             38,  0,  90, false, false },
    { "MORANE",            39,  0,  80, false, false },
    { "D.H.2",             40,  0,  85, false, false },
    { "NIEUPORT XVII",     41,  0,  85, false, false },
    { "SPAD VII",          42,  0,  90, false, false },
    { "S.E.5A",            43,  0,  91, false, false },
    { "SPAD XIII",         44,  0,  92, false, false },
    { "SOPWITH CAMEL",     45,  0,  95, false, false },
    { "CHARRON",           46,  3,  56, false, true  },
    { "RENAULT FT 17",     47,  3,  51, false, true  },
    { "MARK I.",           48,  4,  85, false, true  },
    { "ST. CHAMMOND",      49,  5,  90, false, true  },
    { "MARK IV",           50,  5,  95, false, true  }
};

