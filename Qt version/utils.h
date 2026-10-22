/*
 * HL Editor
 *
 * utils.h by David Konrad
 *
 * Helper functions commonly used by various parts of the application
 *
 */


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

bool tile_is_building_entrance(int tile) {
    return (tile == 0x01 || tile == 0x02 || tile == 0x01 || (tile >= 0x0C && tile <= 0x11));
}

bool tile_is_hq_entrance(int tile) {
    return (tile == 1 || tile == 2);
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
            (tile >= 25 && tile <= 33) || //coastline
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

int tile_at_pos(QPoint p) {
    int offset = ((p.y() * Map.width) + p.x()) * 2;
    return Map.data[offset];
}

int unit_at_pos(QPoint p) {
    int offset = ((p.y() * Map.width) + p.x()) * 2;
    return Map.data[offset+1];
}

//mouse, points ------------------------------------------------
/*
 * Calc field position from mouse coords
 * Thanks to Amit Patel for this elegant solution of a pixel coordinates to hexagon coordinates algorithm!
 * Source: http://www-cs-students.stanford.edu/~amitp/Articles/GridToHex.html
 * -- moved to own function to avoid redundancy
*/
QPoint mouseToFieldPos(QPoint mouse_pos)
{
    int halfsize = Tilesize/2;
    int mx = mouse_pos.x() / Scale_factor; //Let's leave out the scaling to make things easier
    int my = mouse_pos.y() / Scale_factor;
    int hy = my / halfsize;
    int hx = mx / (Tilesize - Tileshift);

    int diagonale[2][12] = {            //the x values of the diagonals of the hexagon
        {7,6,6,5,4,4,3,3,2,1,1,0},
        {0,1,1,2,3,3,4,4,5,6,6,7}
    };

    if( diagonale[(hy+hx)%2][my %halfsize] >= mx %(Tilesize - Tileshift) ) //We can use the y coordinate (modulo the half row height) as an index into the diagonal
      hx--;

    hy = ((hy-(hx%2))/2);

    if (hx < 0) hx = 0;  //Just to be save...
    if (hy < 0) hy = 0;

    return QPoint(hx, hy);
}



/* mapPosNeighbors
 * return all map pos(x,y) neighbors as a list of QPoints
 *         2
 *      3     1
 *         p
 *      4     0
 *         5
 */
QList<QPoint> mapPosNeighbors(QPoint p, bool include_center = false) {
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



//----------------------------------------------------

QImage grayscale_transparent_image(QImage image)
{
/* -- deprecated
    auto alphaChannel = image.alphaChannel();
    image = image.convertToFormat(QImage::Format_Grayscale8);
    image = image.convertToFormat(QImage::Format_ARGB32);
    image.setAlphaChannel(alphaChannel);
    return image;
*/
    // Moden grayscale + alpha (Qt 6)
    QImage alpha = image.convertToFormat(QImage::Format_Alpha8);
    QImage gray  = image.convertToFormat(QImage::Format_Grayscale8);

    QImage result(image.size(), QImage::Format_ARGB32);

    for (int y = 0; y < image.height(); ++y) {
        const uchar *aLine = alpha.constScanLine(y);
        const uchar *gLine = gray.constScanLine(y);
        QRgb *dst = reinterpret_cast<QRgb*>(result.scanLine(y));

        for (int x = 0; x < image.width(); ++x) {
            dst[x] = qRgba(gLine[x], gLine[x], gLine[x], aLine[x]);
        }
    }
    return result;
}
