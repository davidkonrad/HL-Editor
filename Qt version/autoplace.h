/*
 * HL Editor
 *
 * autoplace.h by David Konrad
 *
 * Place mountains, buildings etc correctly on the map
 *
 */

//only change part on map, preserve unit
void Change_Map_part(int x, int y, unsigned char part)
{
    if ((x < (Map.width-1)) && (x >= 0) && (y < (Map.height-1)) && (y >= 0)) {
        int offset = ((y * Map.width) + x) * 2;
        Map.data[offset] = part;
        Redraw_Field(x, y, part, Map.data[offset+1]);
    }
}

void place_mountain_on_map(QPoint h, int tile)
{
/*   Green mountain    brown mountain    green hillside
 *        67                71
 *      69  70            73  74                66
 *        68                72                65
 */
    bool eq = (h.x() % 2) == 0;

    // assume we clicked on a bottom tile, align starting point depending on the selected mountain tile type
    if (tile == 70 || tile == 74) {  //right
        h = h - (eq ? QPoint(1,0) : QPoint(1,-1));
    }
    if (tile == 69 || tile == 73) {  //left
        h = h + (eq ? QPoint(1,0) : QPoint(1,1));
    }
    if (tile == 67 || tile == 71) {  //top
        h = h + (eq ? QPoint(0,1) : QPoint(0,1));
    }

    QList<QPoint> n = mapPosNeighbors(h);

    //green mountain
    if (tile >= 67 && tile <= 70) {
        Change_Map_part(n[2].x(), n[2].y(), 67); //top
        Change_Map_part(n[3].x(), n[3].y(), 69); //left
        Change_Map_part(h.x(), h.y(), 68);       //bottom
        Change_Map_part(n[1].x(), n[1].y(), 70); //right
    }
    //green mountain
    if (tile >= 71 && tile <= 74) {
        Change_Map_part(n[2].x(), n[2].y(), 71); //top
        Change_Map_part(n[3].x(), n[3].y(), 73); //left
        Change_Map_part(h.x(), h.y(), 72);       //bottom
        Change_Map_part(n[1].x(), n[1].y(), 74); //right
    }

    //green hillside
    if (tile == 65) {
        Change_Map_part(h.x(), h.y(), 65);
        Change_Map_part(h.x()+1, eq ? h.y()-1 : h.y(), 66);
    }
    if (tile == 66) {
        Change_Map_part(h.x(), h.y(), 66);
        Change_Map_part(h.x()-1, eq ? h.y() : h.y()+1, 65);
    }
}

//entrance_tile should be either 1 or 2, specifying side (german, french)
void place_hq_on_map(QPoint h, int entrance_tile)
{
    Change_Mapdata(h.x(), h.y(), entrance_tile, 0xFF);
    Change_Mapdata(h.x()-1, h.y() + (h.x() % 2),0x05,0xFF);
    Change_Mapdata(h.x(), h.y() + 1, 0x03, 0xFF);
    Change_Mapdata(h.x()+1, h.y() + (h.x() % 2), 0x07, 0xFF);
    Change_Mapdata(h.x()-1, h.y()+(h.x() % 2) + 1, 0x06, 0xFF);
    Change_Mapdata(h.x(),h.y() + 2, 0x04, 0xFF);
    Change_Mapdata(h.x() + 1, h.y() + (h.x() % 2) + 1, 0x08, 0xFF);
}

void place_depot_on_map(QPoint h, int entrance)
{
    Change_Mapdata(h.x(), h.y(), entrance, 0xFF);
    Change_Mapdata(h.x()-1, h.y() + (h.x() % 2), 0x13, 0xFF);
    Change_Mapdata(h.x(), h.y() + 1, 0x12, 0xFF);
    Change_Mapdata(h.x() + 1, h.y() + (h.x() % 2), 0x14, 0xFF);
}

void place_under_construction_depot_on_map(QPoint h)
{
    Change_Mapdata(h.x(), h.y(), 21, 0xFF);
    Change_Mapdata(h.x()-1, h.y() + (h.x() % 2), 0x17, 0xFF);
    Change_Mapdata(h.x(), h.y() + 1, 0x16, 0xFF);
    Change_Mapdata(h.x()+1, h.y() + (h.x() % 2), 0x18, 0xFF);
}

void place_factory_on_map(QPoint h, int entrance)
{
    Change_Mapdata(h.x(), h.y(), entrance, 0xFF);
    if (h.x() % 2 == 1) {
        Change_Mapdata(h.x()-1, h.y(), 0x09, 0xFF);
        Change_Mapdata(h.x()-1, h.y()+1, 0x0A, 0xFF);
    } else {
        Change_Mapdata(h.x()-1, h.y()-1, 0x09, 0xFF);
        Change_Mapdata(h.x()-1, h.y(), 0x0A, 0xFF);
    }
    Change_Mapdata(h.x()-2, h.y(), 0x0B, 0xFF);
}
