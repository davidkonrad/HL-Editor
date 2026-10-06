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
  0,
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
  225,
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
  225,
  false,
  false
},
{
  "TORPEDO BOAT",
  28,
  0,
  225,
  false,
  false
},
{
  "SUBMARINE",
  29,
  0,
  225,
  false,
  false
},
{
  "SUBMARINE",
  30,
  0,
  225,
  false,
  false
},
{
  "TRANSPORT SHIP",
  31,
  0,
  225,
  false,
  false
},
{
  "DESTROYER",
  32,
  0,
  225,
  false,
  false
},
{
  "DESTROYER",
  33,
  0,
  225,
  false,
  false
},
{
  "BATTLESHIP",
  34,
  0,
  225,
  false,
  false
},
{
  "BATTLESHIP",
  35,
  0,
  225,
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
  00,
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

bool unit_is_transporter(int unit) {
    return (unit == 0x2C || unit == 0x2D || unit == 0x34 || unit == 0x35 || unit == 0x3E || unit == 0x3F);
}

int unit_get_weight(int unit) {
    for (int i=0; i < Num_Units; i++) {
        if (Unit_Info[i].index == unit) {
            return Unit_Info[i].weight;
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

    return 255;
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
