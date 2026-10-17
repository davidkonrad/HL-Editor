/*
 * HL Editor
 *
 * unit_info.h by David Konrad
 *
 * Provides data and functions for handling units rules, i.e weight, capacity for buildings,
 * cost in resources, allowed into transporters, or is transporter, and so on.
 *
 *
 */

struct Unit_Info_Rec {
    QString   name;
    int       index;
    int       weight;
    int       resources;
    bool      allowSupplyCar;
    bool      allowTrainShip;
};

//
bool tile_is_road_unified(int tile) {
    if (tile >= 85 && tile <= 104) return true; //railroad
    if (tile >= 146 && tile <= 164) return true; //dirtroad
    if (tile >= 127 && tile <= 145) return true; //highway
    return false;
}

bool tile_is_building(int tile) {
    return (tile >= 1 && tile <= 24);
}

bool tile_is_building_entrance(int tile) {
    return (tile == 0x01 || tile == 0x02 || tile == 0x01 || (tile >= 0x0C && tile <= 0x11));
}

bool tile_is_mountain(int tile) {
    return (tile >= 0x43 && tile <= 0x4A);
}

bool tile_is_hill(int tile) {
    return ((tile == 64) || //small green mountain
            (tile == 75) || //brown hill #1
            (tile == 76) || //brown hill #2
            (tile == 77) || //brown hill #3
            (tile == 65) || //green hill #1
            (tile == 66));  //green hill #1;
}

bool tile_is_water(int tile) {
    return ((tile == 47) ||
            (tile == 48) ||
            (tile >= 25 && tile <= 33) ||
            (tile == 34)); //lake
}

bool tile_is_reef(int tile) {
    return (tile == 60 || tile == 61 || tile == 62);
}

bool tile_is_trench(int tile) {
    return ((tile == 63) || //xxxx
            (tile >= 165 && tile <= 173));
}

bool tile_is_river(int tile) {
    return (tile >= 36 && tile <= 46);
}

bool tile_is_residental(int tile) {
    return (tile >= 49 && tile <= 52);
}

bool tile_is_forest(int tile) {
    return (tile >= 54 && tile <= 57);
}

bool tile_is_dirtroad(int tile) {
    return (tile >= 146 && tile <= 164);
}

bool tile_is_road(int tile) {
    return (tile >= 127 && tile <= 145);
}

bool tile_is_railroad(int tile) {
    return (tile >= 85 && tile <= 96);
}

bool tile_is_protected(int field_pos) {
    if (tile_is_road(Map.data[field_pos*2])) {
        return !autogenOverwriteRoadsAct->isChecked();
    }
    if (tile_is_building(Map.data[field_pos*2])) {
        return !autogenOverwriteBuildingsAct->isChecked();
    }
    return false;
}


//
bool point_inside_map(QPoint p) {
    return (p.x() > -1 && p.x() < Map.width-1 && p.y() > -1 && p.y() < Map.height-1);
}

struct Unit_Info_Rec Unit_Info[] = { 
{
  "ARMOURED CAR",
  0,
  3,
  55,
  false,
  true
},
{
  "A7V TANK",
  1,
  5,
  90,
  false,
  true
},
{
  "GOTHA BOMBER",
  2,
  0,
  100,
  false,
  false
},
{
  "ZEPPELIN STAAKEN",
  3,
  0,
  115,
  false,
  false
},
{
  "JUNKERS J4-10",
  4,
  0,
  90,
  false,
  false
},
{
  "FOKKER E I.",
  5,
  0,
  80,
  false,
  false
},
{
  "FOKKER E III.",
  6,
  0,
  85,
  false,
  false
},
{
  "ALBATROS",
  7,
  0,
  90,
  false,
  false
},
{
  "FOKKER DR I.",
  8,
  0,
  92,
  false,
  false
},
{
  "FOKKER D VII",
  9,
  0,
  95,
  false,
  false
},
{
  "LIGHT ARTILLERY",
  10,
  3,
  55,
  true,
  true
},
{
  "MEDIUM ARTILLERY",
  11,
  4,
  70,
  true,
  true
},
{
  "HEAVY ARTILLERY",
  12,
  5,
  85,
  true,
  true
},
{
  "BUNKER",
  13,
  0,
  255,
  false,
  false
},
{
  "MOB AA-EMPLACEM.",
  14,
  4,
  62,
  false,
  true
},
{
  "STAT AA-EMPLACEM",
  15,
  2,
  50,
  true,
  true
},
{
  "CONSTR. UNIT",
  16,
  4,
  105,
  false,
  true
},
{
  "ELITE INFANTRY",
  17,
  1,
  50,
  true,
  true
},
{
  "INFANTRY",
  18,
  1,
  35,
  true,
  true
},
{
  "ANTI-TANK GUNS",
  19,
  2,
  53,
  true,
  true
},
{
  "SAPPERS",
  20,
  1,
  45,
  true,
  true
},
{
  "CAVALRY",
  21,
  2,
  50,
  false,
  true
},
{
  "SUPPLY CAR",
  22,
  1, // 9/4 in manual = 9 capacity, max unit weight 4
  45,
  false,
  true
},
{
  "STATIC BALLOON",
  23,
  1,
  30,
  false,
  false
},
{
  "TRAIN ARTILLERY",
  24,
  35,
  125,
  false,
  false
},
{
  "ARMOURED TRAIN",
  25,
  40,
  100,
  false,
  false
},
{
  "SUPPLY TRAIN",
  26,
  1, // 35/15
  90,
  false,
  false
},
{
  "PATROL BOAT",
  27,
  0,
  255,
  false,
  false
},
{
  "TORPEDO BOAT",
  28,
  0,
  255,
  false,
  false
},
{
  "SUBMARINE",
  29,
  0,
  255,
  false,
  false
},
{
  "SUBMARINE",
  30,
  0,
  255,
  false,
  false
},
{
  "TRANSPORT SHIP",
  31,
  0,
  255,
  false,
  false
},
{
  "DESTROYER",
  32,
  0,
  255,
  false,
  false
},
{
  "DESTROYER",
  33,
  0,
  255,
  false,
  false
},
{
  "BATTLESHIP",
  34,
  0,
  255,
  false,
  false
},
{
  "BATTLESHIP",
  35,
  0,
  255,
  false,
  false
},
{
  "VOISIN III",
  36,
  0,
  85,
  false,
  false
},
{
  "HANDLEY PAGE",
  37,
  0,
  115,
  false,
  false
},
{
  "D.H.4",
  38,
  0,
  90,
  false,
  false
},
{
  "MORANE",
  39,
  0,
  80,
  false,
  false
},
{
  "D.H.2",
  40,
  0,
  85,
  false,
  false
},
{
  "NIEUPORT XVII",
  41,
  0,
  85,
  false,
  false
},
{
  "SPAD VII",
  42,
  0,
  90,
  false,
  false
},
{
  "S.E.5A",
  43,
  0,
  91,
  false,
  false
},
{
  "SPAD XIII",
  44,
  0,
  92,
  false,
  false
},
{
  "SOPWITH CAMEL",
  45,
  0,
  95,
  false,
  false
},
{
  "CHARRON",
  46,
  3,
  56,
  false,
  true
},
{
  "RENAULT FT 17",
  47,
  3,
  51,
  false,
  true
},
{
  "MARK I.",
  48,
  4,
  85,
  false,
  true
},
{
  "ST. CHAMMOND",
  49,
  5,
  90,
  false,
  true
},
{
  "MARK IV",
  50,
  5,
  95,
  false,
  true
}

}; 

//
bool unit_allow_in_supply_car(int unit) {
    for (int i=0; i < Num_Units; i++) {
        if (Unit_Info[i].index == unit) {
            return Unit_Info[i].allowSupplyCar;
        }
    }
    return false;
}

bool unit_allow_in_transporter(int unit) {
    for (int i=0; i < Num_Units; i++) {
        if (Unit_Info[i].index == unit) {
            return Unit_Info[i].allowTrainShip;
        }
    }
    return false;
}

bool unit_is_supply_car(int unit) {
    return (unit == 0x2C || unit == 0x2D);
}

bool unit_is_transporter(int unit) {
    return (unit == 0x34 || unit == 0x35 || unit == 0x3E || unit == 0x3F);
}

bool unit_is_large_ship(int unit) {
    qDebug() << "unit_is_large_ship" << unit;
    return ((unit == 29) || (unit == 30) || //submarine
            (unit == 32) || (unit == 33) || //destroyer
            (unit == 34) || (unit == 35));  //battleship
}

bool unit_get_large_ship_parts(int unit, int &starboard, int &portside) {
    switch (unit) {
        case 29 :
        case 30 :
            starboard = 29;
            portside = 30;
            break;

        case 32 :
        case 33 :
            starboard = 32;
            portside = 33;
            break;

        case 34 :
        case 35 :
            starboard = 34;
            portside = 35;
            break;

         default :
             return false;
    }
    return true;
}

int unit_get_weight(int unit) {
    for (int i=0; i < Num_Units; i++) {
        if (Unit_Info[i].index == unit) {
            return Unit_Info[i].weight;
        }
    }
    return 0;
}

int unit_get_resources(int unit) {
    for (int i=0; i < Num_Units; i++) {
        if (Unit_Info[i].index == unit) {
            return Unit_Info[i].resources;
        }
    }
    return 0;
}

int unit_get_building_capacity(int index) {
    if ((Map.data[(Building_info[index].Field * 2) + 1] / 2) == 22)
        return 9; //supply car

    if ((Map.data[(Building_info[index].Field * 2) + 1] / 2) == 26)
        return 35; //supply train

    if ((Map.data[(Building_info[index].Field * 2) + 1] / 2) == 31)
        return 35; //transport ship

    return 255; //building
}

int unit_get_building_weight(int index) {
    int weight = 0;
    for (int i=0; i<7; i++) {
        int unit = Building_info[index].Properties->Units[i];
        if (unit != 0xFF)
            weight = weight + unit_get_weight(unit);
    }
    return weight;
}
