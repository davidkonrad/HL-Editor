/*
 * HL Editor
 *
 * dragdrop.h by David Konrad
 *
 * Provides functions for handling unit and selection dragdrop on the map.
 *
 */


QCursor get_dragdrop_cursor()
{
    //create a transparent image for the unit being dragged
    QImage unitImg = QImage(Tilesize, Tilesize, QImage::Format_ARGB32_Premultiplied);
    unitImg.fill(Qt::transparent);

    Draw_Unit(0, 0, ((drag_unit_number / 2)*6)+5, 1, &unitImg); //+1,2,3,4,5,6

    //a little trick from https://stackoverflow.com/questions/42316844/convert-qimageicon-to-grayscale-format-while-keeping-background
    //otherwise it was impossible to keep transparency (?)
    auto alphaChannel = unitImg.alphaChannel();
    unitImg.convertTo(QImage::Format_Grayscale16);
    unitImg.convertTo(QImage::Format_ARGB32);
    unitImg.setAlphaChannel(alphaChannel);

    //use the image as mouse cursor
    QPixmap pixmap = QPixmap::fromImage( unitImg.scaled(Tilesize * Scale_factor, Tilesize * Scale_factor));
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
        bool is_supply_car = (current_unit == 0x2C || current_unit == 0x2D);
        bool is_transporter = (current_unit == 0x34 || current_unit == 0x35 || current_unit == 0x3E || current_unit == 0x3F);

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
            drag_cancelled = !ask_question("There are already a unit on this tile, replace?");//, this);
        }
    }

    //is target a building
    if (tile >= 0x01 && tile <= 0x14) {
        drag_cancelled = true; //disallow units on building tiles
        //is target a building entrance
        if (tile == 0x01 || tile == 0x02 || tile == 0x01 || (tile >= 0x0C && tile <= 0x11)) {
            int building_index = Get_Building_by_field(to_field_pos);
            //building and unit is same side
            if (building_index > -1) {
                if (side == Building_info[building_index].Properties->Owner) {
                    for (int i=0; i<7; i++) {
                        if (Building_info[building_index].Properties->Units[i] == 0xFF) {
                            Map.data[(from_field_pos*2)+1] = 0xFF;
                            Building_info[building_index].Properties->Units[i] = drag_unit_number/2;
                            Redraw_Field(drag_start.x(), drag_start.y(), Map.data[(from_field_pos*2)], 0xFF);
                            break;
                        }
                        if (i == 6) {
                            show_error("Building is full");//, this);
                        }
                    }
                }
            }
        }
    }

    if (!drag_cancelled) {
        //is the dragged unit a transporter?
        if ((drag_unit_number == 0x2C) ||
            (drag_unit_number == 0x2D) ||
            (drag_unit_number == 0x34) ||
            (drag_unit_number == 0x35) ||
            (drag_unit_number == 0x3E) ||
            (drag_unit_number == 0x3F)) {
            //simply just 're-field' Building_info[]
            int transporter_index = Get_Building_by_field(from_field_pos);
            if (transporter_index > -1) {
                Building_info[transporter_index].Field = to_field_pos;
            }
        }

        Map.data[(from_field_pos*2)+1] = 0xFF;
        Map.data[(to_field_pos*2)+1] = drag_unit_number;

        Redraw_Field(drag_start.x(), drag_start.y(), Map.data[(from_field_pos*2)], Map.data[(from_field_pos*2)+1]);
        Redraw_Field(drag_end.x(), drag_end.y(), Map.data[(to_field_pos*2)], Map.data[(to_field_pos*2)+1]);

    }

    return !drag_cancelled;

}
