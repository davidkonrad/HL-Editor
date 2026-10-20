/*
 * HL Editor
 *
 * utils.h by David Konrad
 *
 * Helper functions commonly used by various parts of the application
 *
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

bool tile_is_mountain(int tile) {
    return (tile >= 67 && tile <= 74);
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

bool points_equal(QPoint a, QPoint b)
{
    if (a.isNull() || b.isNull()) return true;
    if (a.x() != b.x()) return false;
    if (a.y() != b.y()) return false;
    return true;
}

