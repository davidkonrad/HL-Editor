/*
 * HL Editor
 *
 * dragdrop.h by David Konrad
 *
 * Provides functions for handling unit and selection dragdrop on the map.
 *
 */


QImage get_unit_image(int unit, int direction, bool grayscale = false)
{
    QImage unitImg = QImage(Tilesize, Tilesize, QImage::Format_ARGB32_Premultiplied);
    unitImg.fill(Qt::transparent);

    Draw_Unit(0, 0, ((unit / 2) * 6) + direction, 1, &unitImg); //+1,2,3,4,5,6

    if (grayscale)
        unitImg = grayscale_transparent_image(unitImg);

    return unitImg;
}

QImage get_large_unit_image(int unit1, int unit2, int direction, bool grayscale = false)
{
    QImage unitImg = QImage(Tilesize + (Tileshift*2), Tilesize + (Tileshift*2), QImage::Format_ARGB32_Premultiplied);
    unitImg.fill(Qt::transparent);

    Draw_Unit(0, 0, (unit1 * 6) + direction, 3, &unitImg);
    Draw_Unit(Tileshift * 1.5, Tilesize - (Tileshift * 2), (unit2 * 6) + direction, 3, &unitImg);

    if (grayscale)
        unitImg = grayscale_transparent_image(unitImg);

    return unitImg;
}

QIcon get_unit_icon(int unit, int direction, bool grayscale = false)
{
    return QPixmap::fromImage( get_unit_image(unit, direction, grayscale));
}

bool DRAGGING_LARGE_SHIP = false;

int get_large_ship_counterpart() {
    int prior = ((drag_start.y()-1) * Map.width) + drag_start.x();
    int next = ((drag_start.y()+1) * Map.width) + drag_start.x();

    if (drag_unit_number/2 == 29) {
        return (Map.data[(next*2)+1]/2 == 30) ? next : 0;
    }
    if (drag_unit_number/2 == 30) {
        return (Map.data[(prior*2)+1]/2 == 29) ? prior : 0;
    }
    if (drag_unit_number/2 == 32) {
        return (Map.data[(next*2)+1]/2 == 33) ? next : 0;
    }
    if (drag_unit_number/2 == 33) {
        return (Map.data[(prior*2)+1]/2 == 32) ? prior : 0;
    }
    if (drag_unit_number/2 == 34) {
        return (Map.data[(next*2)+1]/2 == 35) ? next : 0;
    }
    if (drag_unit_number/2 == 35) {
        return (Map.data[(prior*2)+1]/2 == 34) ? prior : 0;
    }
    return -1;
}

void redraw_area(QPoint p) {
    int field_pos;
    QList<QPoint> np = mapPosNeighbors(p);
    for (int i=0; i<=5; i++) {
        field_pos = point_to_fieldpos(np[i]);
        Redraw_Field(np[i].x(), np[i].y(), Map.data[field_pos*2], Map.data[(field_pos*2)+1]);
    }
}

void drop_large_ship() {
    int counter_field_pos = get_large_ship_counterpart();
    int unit = Map.data[(counter_field_pos*2)+1];

    int prior = ((drag_end.y()-1) * Map.width) + drag_end.x();
    int next = ((drag_end.y()+1) * Map.width) + drag_end.x();

    if ((drag_unit_number/2 == 29) || (drag_unit_number/2 == 32) || (drag_unit_number/2 == 34)) {
        Map.data[(next*2)+1] = unit;
    } else {
        Map.data[(prior*2)+1] = unit;
    }

    Map.data[(counter_field_pos*2)+1] = 0xFF;

    redraw_area(drag_end); //
    redraw_area(drag_start);
}

QCursor get_dragdrop_unit_cursor()
{
    QImage unitImg;
    QPixmap pixmap;

    //if the unit is part of a large ship, and there is a matching unit nearby, we are dragging a large ship
    DRAGGING_LARGE_SHIP = unit_is_large_ship(drag_unit_number/2) && get_large_ship_counterpart() > 0;

    if (DRAGGING_LARGE_SHIP) {
        int starboard;
        int portside;
        unit_get_large_ship_parts(drag_unit_number/2, starboard, portside);
        unitImg = get_large_unit_image(starboard, portside, 5, true);
        //increase with *3 since the unit is angled
        pixmap = QPixmap::fromImage( unitImg.scaled((Tilesize + (Tileshift*3)) * Scale_factor, (Tilesize + (Tileshift*3)) * Scale_factor));
    } else {
        unitImg = get_unit_image(drag_unit_number, 5, true);
        pixmap = QPixmap::fromImage( unitImg.scaled(Tilesize * Scale_factor, Tilesize * Scale_factor));
    }

    QCursor cursor = QCursor(pixmap, -Tilesize, -Tilesize); //!?
    return cursor;
}

bool execute_unit_dragdrop()
{
    int from_field_pos = (drag_start.y() * Map.width) + drag_start.x();
    int to_field_pos = (drag_end.y() * Map.width) + drag_end.x();
    int side = (drag_unit_number % 2 == 0) ? 0 : 1;
    bool drag_cancelled = false;
    unsigned char tile = Map.data[to_field_pos*2];
    unsigned char current_unit = Map.data[(to_field_pos*2)+1];

    //drag drop within same field
    if (from_field_pos == to_field_pos) {
        DRAG_UNIT_START = false;
        DRAG_UNIT_PROGRESS = false;
        return false;
    }

    //warn if overwrite / replace existing unit, or place inside transporter
    if (current_unit != 0xFF) {
        bool is_supply_car = unit_is_supply_car(current_unit);
        bool is_transporter = unit_is_transporter(current_unit);

        if ((is_supply_car && unit_allow_in_supply_car(drag_unit_number/2)) ||
            (is_transporter && unit_allow_in_transporter(drag_unit_number/2))) {

            int transporter_index = Get_Building_by_field(to_field_pos);
            if (transporter_index > -1) {
                //transporter and unit is same side
                if (side == Building_info[transporter_index].Properties->Owner) {
                    int max_weight = is_supply_car ? 9 : 35; //35 is train, dont know ship max (yet)
                    //is there room for the unit?
                    if (max_weight >= (unit_get_building_weight(transporter_index) + unit_get_weight(drag_unit_number/2))) {
                        for (int i=0; i<7; i++) {
                            if (Building_info[transporter_index].Properties->Units[i] == 0xFF) {
                                Map.data[(from_field_pos*2)+1] = 0xFF;
                                Building_info[transporter_index].Properties->Units[i] = drag_unit_number/2;
                                Redraw_Field(drag_start.x(), drag_start.y(), Map.data[(from_field_pos*2)], 0xFF);
                                drag_cancelled = true;
                                break;
                            }
                            if (i == 6) {
                                show_error("Transporter is full"); //, this);
                                drag_cancelled = true;
                            }
                        }
                    } else {
                        show_error("Transporter is full");//, this);
                        drag_cancelled = true;
                    }
                } else {
                    drag_cancelled = true;
                }
            }
        } else if (is_supply_car || is_transporter) {
            drag_cancelled = true; //unit not allowed in supply car or transporter
        } else if (Settings->value(REG_SHOW_WARNINGS).toBool()) {
            drag_cancelled = !ask_question("There are already a unit on this tile, replace?");
        } else {
            drag_cancelled = true;
        }
    }

    //is target a building
    if (tile_is_building(tile)) {
        drag_cancelled = true; //disallow units on building tiles

        //is target a building entrance
        if (tile_is_building_entrance(tile)) {
            if (drag_unit_number/2 == 13) { //ignore bunkers
                return false;
            }

            int building_index = Get_Building_by_field(to_field_pos);
            //building and unit is same side
            if (building_index > -1) {
                if (side == Building_info[building_index].Properties->Owner) {
                    for (int i=0; i<7; i++) {
                        if (Building_info[building_index].Properties->Units[i] == 0xFF) {
                            Map.data[(from_field_pos*2)+1] = 0xFF;
                            Building_info[building_index].Properties->Units[i] = drag_unit_number/2;
                            Redraw_Field(drag_start.x(), drag_start.y(), Map.data[(from_field_pos*2)], 0xFF);
                            return true;
                        }
                        if (i == 6) {
                            show_error("Building is full");
                        }
                    }
                }
            }
        }
    }

    if (!drag_cancelled) {
        //is the dragged unit a transporter?
        if (unit_is_transporter(drag_unit_number)) {
            //simply just 're-field' Building_info[]
            int transporter_index = Get_Building_by_field(from_field_pos);
            if (transporter_index > -1) {
                Building_info[transporter_index].Field = to_field_pos;
            }
        } else {
            Change_Mapdata( drag_start.x(), drag_start.y(), Map.data[from_field_pos*2], 0xFF);
            Change_Mapdata( drag_end.x(), drag_end.y(), Map.data[to_field_pos*2], drag_unit_number);
            if (DRAGGING_LARGE_SHIP) drop_large_ship(); //dragdrop the second part of the ship
        }
    }

    return !drag_cancelled;
}

//selection ------------------------------------
QCursor get_dragdrop_selection_cursor()
{
    Sel_Rect rect = get_sel_rect();
    int width = (rect.right - rect.left);
    int height = (rect.bottom - rect.top);
    int imageWidth;
    int imageHeight;

    if (((width % 2 == 0) && (height % 2 == 0)) ||
        ((width % 2 == 0) && !(height % 2 == 0)))
    {
       imageWidth = width + 4;
       imageHeight = height + 3;
    } else {
        imageWidth = width + 3;
        imageHeight = height + 2;
    }

    QImage image = QImage(((imageWidth/2) * Tilesize) + (((imageWidth/2)-1) * Tileshift), (imageHeight * Tilesize) + (Tilesize/2), QImage::Format_ARGB32_Premultiplied);
    image.fill(Qt::transparent);

    for (int x = rect.left; x <= rect.right; x++) {
        for (int y = rect.top; y <= rect.bottom; y++) {
            int side;
            int field_pos = (y * Map.width) + x;
            int unit = Map.data[(field_pos*2)+1];
            int xx = (x - rect.left) + 1;
            int yy = (y - rect.top) + 1;

            if (x % 2 != 0)
            {
                Draw_Part((xx * (Tilesize - Tileshift)), (yy * Tilesize) + (Tilesize / 2), Map.data[field_pos*2], &image);
                if (unit != 0xFF) {
                    if (unit % 2 == 0) side = 1; else side = 2;
                    int drawunit = (unit / 2) * 6;
                    if (side == 1) drawunit = drawunit + 3;
                    Draw_Unit((xx * (Tilesize - Tileshift)), (yy * Tilesize) + (Tilesize / 2), drawunit, side, &image);
                }
            }
            else
            {
                Draw_Part((xx * (Tilesize - Tileshift)), (yy * Tilesize), Map.data[field_pos*2], &image);
                if (unit != 0xFF)
                {
                    if (unit % 2 == 0) side = 1; else side = 2;
                    int drawunit = (unit / 2) * 6;
                    if (side == 1) unit = unit + 3;
                    Draw_Unit((xx * (Tilesize - Tileshift)), (yy * Tilesize), drawunit, side, &image);
                }
            }

        }
    }

    image = grayscale_transparent_image(image);

    //use the image as mouse cursor
    QPixmap pixmap = QPixmap::fromImage( image.scaled(image.width()*Scale_factor, image.height()*Scale_factor) );
    QCursor cursor = QCursor(pixmap, -Tilesize, -Tilesize); //!?

    return cursor;
}

bool execute_selection_dragdrop(QPoint pos)
{
    Sel_Rect rect = get_sel_rect();
    int unit;
    int part;
    int field_pos;
    int tx;
    int ty;
    int building_index;
    QPoint offset = selection_drag_from - selection_start;
    pos = pos - offset;

    //adjust to map boundaries
    int shift = (pos.x() % 2) ? 0 : 1;
    if ((pos.x() + rect.width + shift) >= Map.width-1) {
        int dif = pos.x() + rect.width + 2 + shift;
        dif = abs((Map.width-1) - dif);
        pos.setX(pos.x() - dif);
    }
    if (pos.x() - offset.x() < 0) {
        pos.setX(0);//
    }

    shift = (pos.y() % 2) ? 1 : 2;
    if ((pos.y() + rect.height + shift) >= Map.height-1) {
        int dif = pos.y() + rect.height + 2 + shift;
        dif = abs((Map.height-1) - dif);
        pos.setY(pos.y() - dif);
    }
    if (pos.y() - offset.y() < 0) {
        pos.setY(0);//
    }

    //
    if ((selection_start.x() % 2 == 1) && (pos.x() % 2 == 0)) {
        pos.setX(pos.x() - 1);
    }

    if ((selection_start.x() % 2 == 0) && (pos.x() % 2 == 1)) {
        pos.setX(pos.x() - 1);
    }

    if ((selection_start.y() % 2 == 1) && (pos.y() % 2 == 0)) {
        pos.setY(pos.y() -1);
    }

    if ((selection_start.y() % 2 == 0) && (pos.y() % 2 == 1)) {
        pos.setY(pos.y() +1);
    }

    //qDebug() << rect;

    for (int x = rect.left; x <= rect.right; x++) {
        for (int y = rect.top; y <= rect.bottom; y++) {
            field_pos = (y * Map.width) + x;
            part = Map.data[field_pos*2];
            unit = Map.data[(field_pos*2)+1];

            if (tile_is_building_entrance(part) || unit_is_transporter(unit)) {
                building_index = Get_Building_by_field(field_pos);
            } else {
                building_index = -1;
            }

            Change_Mapdata(x, y, ((Ocean == true) ? 48 : 0), 0xFF); //set deep water if Ocean
            Redraw_Field(x, y, ((Ocean == true) ? 48 : 0), 0xFF);

            tx = pos.x() + (x - rect.left);
            ty = pos.y() + (y - rect.top);

            if ((tx > -1 && tx < Map.width-1) && (ty > -1 && ty < Map.height-1)) {
                Change_Mapdata(tx, ty, part, unit);
                Redraw_Field(tx, ty, part, unit);

                if (building_index > -1) {
                    Building_info[building_index].Field = (ty * Map.width) + tx;
                }
            }
        }
    }
    selection_start = pos;
    selection_end = QPoint(pos.x() + (rect.right - rect.left), pos.y() + (rect.bottom - rect.top));

    return true;
}


//terrain ----------------------------------------------------

/* return four squared entity, i.e mountain, building
 *
 *      part1
 *   part2  part3
 *      part4
 *
 */
QImage get_part_combined_image(int part1, int part2, int part3, int part4, bool grayscale = false)
{
    QImage partImg = QImage((Tilesize * 2) + Tileshift , (Tilesize * 2) + Tileshift, QImage::Format_ARGB32_Premultiplied);
    partImg.fill(Qt::transparent);

    Draw_Part(Tileshift * 2, 0, part1, &partImg);
    Draw_Part(0, Tilesize - (Tileshift * 1.5), part2, &partImg);
    Draw_Part(Tilesize + Tileshift, Tilesize - (Tileshift * 1.5), part3, &partImg);
    Draw_Part(Tileshift * 2, Tilesize, part4, &partImg);

    if (grayscale)
        partImg = grayscale_transparent_image(partImg);

/* we must soon figure out a final solution for borders
    QPen pen(Qt::yellow, 1);
    Draw_Hexagon(1, 0, pen, &partImg, true, false);
    Draw_Hexagon(0, 1, pen, &partImg, true, false);
*/

    return partImg;
}

QCursor get_part_combined_cursor(int part1, int part2, int part3, int part4)
{
    QImage image = get_part_combined_image(part1, part2, part3, part4, true);
    QPixmap pixmap = QPixmap::fromImage( image.scaled(Tilesize * Scale_factor, Tilesize * Scale_factor));
    QCursor cursor = QCursor(pixmap, -Tilesize, -Tilesize); //!?
    return cursor;
}

QCursor get_dragdrop_factory_cursor()
{
    QImage image = get_part_combined_image(9, 11, 12, 10, true);
    QPixmap pixmap = QPixmap::fromImage( image.scaled(image.width() * Scale_factor, image.height() * Scale_factor) );
    QCursor cursor = QCursor(pixmap, -Tilesize, -Tilesize); //!?
    return cursor;
}

QCursor get_dragdrop_depot_cursor()
{
    QImage image = get_part_combined_image(15, 19, 20, 18, true);
    QPixmap pixmap = QPixmap::fromImage( image.scaled(image.width() * Scale_factor, image.height() * Scale_factor) );
    QCursor cursor = QCursor(pixmap, -Tilesize, -Tilesize); //!?
    return cursor;
}

QCursor get_dragdrop_under_construction_depot_cursor()
{
    QImage image = get_part_combined_image(21, 23, 24, 22, true);
    QPixmap pixmap = QPixmap::fromImage( image.scaled(image.width() * Scale_factor, image.height() * Scale_factor) );
    QCursor cursor = QCursor(pixmap, -Tilesize, -Tilesize); //!?
    return cursor;
}

QCursor get_dragdrop_mountain_green_cursor()
{
    QImage image = get_part_combined_image(67, 69, 70, 68, true);
    QPixmap pixmap = QPixmap::fromImage( image.scaled(image.width() * Scale_factor, image.height() * Scale_factor) );
    QCursor cursor = QCursor(pixmap, -Tilesize, -Tilesize); //!?
    return cursor;
}

QCursor get_dragdrop_mountain_brown_cursor()
{
    QImage image = get_part_combined_image(71, 73, 74, 72, true);
    QPixmap pixmap = QPixmap::fromImage( image.scaled(image.width() * Scale_factor, image.height() * Scale_factor) );
    QCursor cursor = QCursor(pixmap, -Tilesize, -Tilesize); //!?
    return cursor;
}


QCursor get_dragdrop_terrain_cursor(int tile) {
    if (tile_is_mountain_green(tile)) return get_dragdrop_mountain_green_cursor();
    if (tile_is_mountain_brown(tile)) return get_dragdrop_mountain_brown_cursor();
    return QCursor(Qt::ArrowCursor);
}

int get_clear_tile() {
    return lucky(50) ? 78 : 79; //plain #2, stones
}

QList<QPoint> terrain_get_drag_tiles(QPoint p)
{
    QList<QPoint> result;
    int bottom_tile;

    if (tile_is_mountain_green(tile_at_pos(p))) bottom_tile = 68;
    if (tile_is_mountain_brown(tile_at_pos(p))) bottom_tile = 72;

 /* find bottom
 *      2
 *   3     1
 *      p
 */
    QList<QPoint> nb = mapPosNeighbors(p, true);
    for (int i=0; i<=6; i++) {
        if (tile_at_pos(nb[i]) == bottom_tile) {
            QList<QPoint> rr = mapPosNeighbors(nb[i]);
            result.append(nb[i]);
            result.append(rr[1]);
            result.append(rr[2]);
            result.append(rr[3]);
            break;
        }
    }
    return result;
}

//clear four squared drag terrain area, always with base of the bottom tile
void terrain_clear_drag_area()
{
    QList<QPoint> nb = terrain_get_drag_tiles(drag_start);
    for (int i=0; i<=3; i++) {
        Change_Map_part(nb[i].x(), nb[i].y(), get_clear_tile());
    }
}

/*
 * test if the dragged terrain type is 'legal'
 * i.e it contains all the correct tiles in the correct order
 */
bool terrain_dragdrop_sanity_check(QPoint p)
{
    QList<QPoint> sp = terrain_get_drag_tiles(p);
    if (sp.count() == 0) return false;

    if (tile_is_mountain_green(tile_at_pos(p))) {
        if (tile_at_pos(sp[0]) != 68) return false;
        if (tile_at_pos(sp[1]) != 70) return false;
        if (tile_at_pos(sp[2]) != 67) return false;
        if (tile_at_pos(sp[3]) != 69) return false;
        return true;
    }

    if (tile_is_mountain_brown(tile_at_pos(p))) {
        if (tile_at_pos(sp[0]) != 72) return false;
        if (tile_at_pos(sp[1]) != 74) return false;
        if (tile_at_pos(sp[2]) != 71) return false;
        if (tile_at_pos(sp[3]) != 73) return false;
        return true;
    }

    return false;
}

bool execute_terrain_dragdrop()
{
    terrain_clear_drag_area();

    if (tile_is_mountain_green(drag_terrain_tile)) {
        place_mountain_on_map(drag_end, 68);
    }

    if (tile_is_mountain_brown(drag_terrain_tile)) {
        place_mountain_on_map(drag_end, 72);
    }

    QList<QPoint> de = mapPosNeighbors(drag_end, true);
    for (int i=0; i<=6; i++) {
        Redraw_Field(de[i].x(), de[i].y());
    }

    return true;

}

