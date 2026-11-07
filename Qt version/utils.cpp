/*
 * HL Editor
 *
 * utils.cpp by David Konrad
 *
 * utils.h implementation
 *
 */

#include <QPoint>
#include <QImage>

#include "Lib.h"
#include "fin.h"
#include "shp.h"
#include "units.h"
#include "utils.h"

//tiles ---------------------------------------------------------
bool tile_is_road_unified(int tile) {
    if (tile >= 85 && tile <= 104) return true; //railroad
    if (tile >= 146 && tile <= 164) return true; //dirtroad
    if (tile >= 127 && tile <= 145) return true; //highway
    return false;
}

bool tile_is_building(int tile) {
    return (tile >= 1 && tile <= 24);
}

bool tile_is_hq_entrance(int tile) {
    return (tile == 1 || tile == 2);
}

bool tile_is_hq(int tile) {
    return (tile >= 1 && tile <=8);
}

bool tile_is_factory_entrance(int tile) {
    return (tile == 12 || tile == 13 || tile == 14);
}

bool tile_is_factory(int tile) {
    if (tile_is_factory_entrance(tile)) return true;
    return (tile == 9 || tile == 10 || tile == 11);
}

bool tile_is_depot_entrance(int tile) {
    return (tile == 15 || tile == 16 || tile == 17);
}

bool tile_is_depot(int tile) {
    if (tile_is_depot_entrance(tile)) return true;
    return (tile == 18 || tile == 19 || tile == 20);
}

bool tile_is_under_construction_depot_entrance(int tile) {
    return (tile == 21);
}

bool tile_is_building_entrance(int tile) {
    return ((tile_is_hq_entrance(tile)) ||
            (tile_is_depot_entrance(tile)) ||
            (tile_is_factory_entrance(tile)) ||
            (tile_is_under_construction_depot_entrance(tile)));
}

bool tile_is_under_construction_depot(int tile) {
    if (tile_is_under_construction_depot_entrance(tile)) return true;
    return (tile == 22 || tile == 23 || tile == 24);
}

bool tile_is_mountain(int tile) {
    return (tile >= 67 && tile <= 74);
}

bool tile_is_mountain_green(int tile) {
    return (tile >= 67 && tile <= 70);
}

bool tile_is_mountain_brown(int tile) {
    return (tile >= 71 && tile <= 74);
}

bool tile_is_hillside(int tile) {
    return (tile == 65 || tile == 66);
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
            (tile >= 25 && tile <= 33)); //shallow water + coastline
}

bool tile_is_lake(int tile) {
    return (tile == 34);
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

int tile_at_pos(QPoint p) {
    int offset = ((p.y() * Map.width) + p.x()) * 2;
    return Map.data[offset];
}

//units --------------------------------------------------------------------------
int unit_at_pos(QPoint p) {
    int offset = ((p.y() * Map.width) + p.x()) * 2;
    return Map.data[offset+1];
}

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

bool unit_is_supply_train(int unit) {
    return (unit/2 == 26);
}

bool unit_is_transport_ship(int unit) {
    return (unit/2 == 31);
}

bool unit_is_transporter(int unit) {
    return (unit == 0x34 || unit == 0x35 || unit == 0x3E || unit == 0x3F);
}

bool unit_is_large_ship(int unit) {
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


/* mapPosNeighbors
 * return all map pos(x,y) neighbors as a list of QPoints
 *         2
 *      3     1
 *         p
 *      4     0
 *         5
 */
QList<QPoint> mapPosNeighbors(QPoint p, bool include_center) {
    // odd-q: odd x lowered half hex
    static const QPoint evenCol[6] = {
        {+1,  0}, {+1, -1}, { 0, -1},
        {-1, -1}, {-1,  0}, { 0, +1}
    };
    static const QPoint oddCol[6] = {
        {+1, +1}, {+1,  0}, { 0, -1},
        {-1,  0}, {-1, +1}, { 0, +1}
    };

    const QPoint* dirs = (p.x() & 1) ? oddCol : evenCol;

    QList<QPoint> result;

    if (include_center) {
        result.reserve(7);
        result.append(p);
    } else {
       result.reserve(6);
    }

    for (int i = 0; i < 6; ++i) {
       result.append(p + dirs[i]);
    }

    return result;
}

bool points_equal(QPoint a, QPoint b)
{
    if (a.isNull() || b.isNull()) return true;
    if (a.x() != b.x()) return false;
    if (a.y() != b.y()) return false;
    return true;
}

bool point_inside_map(QPoint p) {
    return (p.x() > -1 && p.x() < Map.width-1 && p.y() > -1 && p.y() < Map.height-1);
}

int point_to_fieldpos(QPoint p) {
    return (p.y() * Map.width) + p.x();
}

