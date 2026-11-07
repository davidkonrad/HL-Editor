/*
 * HL Editor
 *
 * utils.h by David Konrad
 *
 * Helper functions commonly used by various parts of the application
 *
 */

#ifndef UTILS_H
#define UTILS_H


extern bool tile_is_road_unified(int tile);
extern bool tile_is_building(int tile);
extern bool tile_is_hq_entrance(int tile);
extern bool tile_is_hq(int tile);
extern bool tile_is_factory_entrance(int tile);
extern bool tile_is_factory(int tile);
extern bool tile_is_depot_entrance(int tile);
extern bool tile_is_depot(int tile);
extern bool tile_is_under_construction_depot_entrance(int tile);
extern bool tile_is_building_entrance(int tile);
extern bool tile_is_under_construction_depot(int tile);
extern bool tile_is_mountain(int tile);
extern bool tile_is_mountain_green(int tile);
extern bool tile_is_mountain_brown(int tile);
extern bool tile_is_hillside(int tile);
extern bool tile_is_hill(int tile);
extern bool tile_is_water(int tile);
extern bool tile_is_lake(int tile);
extern bool tile_is_reef(int tile);
extern bool tile_is_trench(int tile);
extern bool tile_is_river(int tile);
extern bool tile_is_residental(int tile);
extern bool tile_is_forest(int tile);
extern bool tile_is_dirtroad(int tile);
extern bool tile_is_road(int tile);
extern bool tile_is_railroad(int tile);

extern int tile_at_pos(QPoint p);
extern int unit_at_pos(QPoint p);

extern bool unit_allow_in_supply_car(int unit);
extern bool unit_allow_in_transporter(int unit);
extern bool unit_is_supply_car(int unit);
extern bool unit_is_supply_train(int unit);
extern bool unit_is_transport_ship(int unit);
extern bool unit_is_transporter(int unit);
extern bool unit_is_large_ship(int unit);
extern bool unit_get_large_ship_parts(int unit, int &starboard, int &portside);

extern int unit_get_building_capacity(int index);
extern int unit_get_building_weight(int index);

extern QList<QPoint> mapPosNeighbors(QPoint p, bool include_center = false);

extern bool points_equal(QPoint a, QPoint b);
extern bool point_inside_map(QPoint p);
extern int point_to_fieldpos(QPoint p);

extern void Change_Map_part(int x, int y, unsigned char part);

extern bool terrain_hq_sanity_test(QPoint p);
extern bool terrain_sanity_check(QPoint p);
extern void terrain_clear_entity_area(QPoint p);

extern QList<QPoint> terrain_get_entity_tiles(QPoint p);
extern QList<QPoint> terrain_get_hq_tiles(QPoint p);


#endif // UTILS_H
