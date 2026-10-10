/*
 * HL Editor
 *
 * selection.h by David Konrad
 *
 * Provides functions and data structures for handling selections on the map.
 *
 */


typedef struct {
    int left;
    int right;
    int top;
    int bottom;
    int width;
    int height;
} Sel_Rect;


bool tile_is_road(int tile) {
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

bool tile_is_protected(int field_pos) {
    if (tile_is_road(Map.data[field_pos*2])) {
        return !autogenOverwriteRoadsAct->isChecked();
    }
    if (tile_is_building(Map.data[field_pos*2])) {
        return !autogenOverwriteBuildingsAct->isChecked();
    }
    return false;
}

/*
 * return Sel_Rect based on selection_start, selection_end
 * align selections if they break map boundaries
 */
Sel_Rect get_sel_rect()
{
    int max_width = Map.width - 2;
    int max_height = Map.height -2;
    Sel_Rect r;

    if (selection_start.x() <= selection_end.x()) {
        if (selection_start.x() < 0) selection_start.setX(0);
        if (selection_end.x() > max_width) selection_end.setX(max_width);
        r.left = selection_start.x();
        r.right = selection_end.x();
    } else {
        if (selection_start.x() > max_width) selection_start.setX(max_width);
        if (selection_end.x() < 0) selection_end.setX(0);
        r.left = selection_end.x();
        r.right = selection_start.x();
    }

    if (selection_start.y() <= selection_end.y()) {
        if (selection_start.y() < 0) selection_start.setY(0);
        if (selection_end.y() > max_height) selection_end.setY(max_height);
        r.top = selection_start.y();
        r.bottom = selection_end.y();
    } else {
        if (selection_end.y() < 0) selection_end.setY(0);
        if (selection_start.y() > max_height) selection_start.setY(max_height);
        r.top = selection_end.y();
        r.bottom = selection_start.y();
    }

    r.height = r.bottom - r.top;
    r.width = r.right - r.left;

    return r;
}

void Draw_SelHex(int x, int y, QImage *Image)
{
    double sf = Scale_factor;
    bool equal = (x % 2 != 0);
    int xp, yp;
    Sel_Rect rect = get_sel_rect();

    xp = x * (Tilesize-Tileshift);
    if (x % 2 != 0)
        yp = (y * Tilesize) + (Tilesize / 2);
    else
        yp = (y * Tilesize) ;

    xp = xp * sf;
    yp = yp * sf;

    QPainter painter(Image);
    painter.setRenderHint(QPainter::Antialiasing);

    QPainterPath path;
    QPainterPath border;
    border.setFillRule(Qt::WindingFill);

    path.moveTo(xp, yp +((Tilesize / 2) * sf));
    path.lineTo(xp + (Tileshift*sf), yp);
    path.lineTo(xp + ((Tileshift*2) * sf), yp);
    path.lineTo(xp + (Tilesize*sf), yp + ((Tilesize/2) * sf));
    path.lineTo(xp + ((Tileshift*2) * sf), yp + (Tilesize*sf));
    path.lineTo(xp + (Tileshift*sf), yp + (Tilesize*sf));
    path.lineTo(xp, yp + ((Tilesize / 2) * sf));

    QPen pen = QPen(Qt::yellow);
    pen.setStyle(Qt::SolidLine);
    pen.setJoinStyle(Qt::RoundJoin);
    pen.setCosmetic(true);
    pen.setWidthF(sf * 1.15);
    painter.setPen(pen);

    if (y == rect.top) {
        if (equal) {
            border.moveTo(xp + (Tileshift*sf) + 2, yp);
            border.lineTo(xp + ((Tileshift*2) * sf) - 2, yp);
          } else {
            border.moveTo(xp, yp +((Tilesize / 2) * sf) - 2);
            border.lineTo(xp + (Tileshift*sf), yp);
            border.lineTo(xp + ((Tileshift*2) * sf), yp);
            border.lineTo(xp + (Tilesize*sf), yp + ((Tilesize/2) * sf) - 2);
        }
        painter.drawPath(border);
    }

    if (y == rect.bottom) {
        if (equal) {
            border.moveTo(xp + (Tilesize * sf), yp + ((Tilesize/2) * sf));
            border.lineTo(xp + ((Tileshift*2) * sf), yp + (Tilesize * sf) + 2);
            border.lineTo(xp + (Tileshift*sf), yp + (Tilesize*sf));
            border.lineTo(xp, yp + ((Tilesize / 2) * sf) + 2);
        } else {
            border.moveTo(xp + ((Tileshift*2) * sf), yp + (Tilesize*sf));
            border.lineTo(xp + (Tileshift*sf) + 1, yp + (Tilesize*sf));
        }
        painter.drawPath(border);
    }

    if (x == rect.right) {
        border.moveTo(xp + ((Tileshift*2) * sf), yp + 2);
        border.lineTo(xp + (Tilesize * sf), yp + ((Tilesize/2) * sf));
        border.lineTo(xp + ((Tileshift*2) * sf) -1, yp + (Tilesize*sf) - 1);
        painter.drawPath(border);
    }

    if (x == rect.left) {
        if (y == rect.top && !equal) {
            border.moveTo(xp, yp +((Tilesize / 2) * sf));
            border.lineTo(xp + (Tileshift*sf), yp + (Tilesize*sf));
        } else if (y == rect.bottom && equal) {
            border.moveTo(xp + (Tileshift*sf), yp);
            border.lineTo(xp, yp +((Tilesize / 2) * sf));
        } else {
            border.moveTo(xp + (Tileshift*sf), yp + 2);
            border.lineTo(xp, yp +((Tilesize / 2) * sf));
            border.lineTo(xp + (Tileshift*sf), yp + (Tilesize*sf));
        }
        painter.drawPath(border);
    }

    painter.setPen(Qt::NoPen);
    painter.drawPath(path);
    painter.fillPath(path, QBrush(QColor(100, 100, 100, 128)));

    painter.end();
}

void Paint_Selection(QImage &image) {
    if (selection_start == selection_end) return;

    Sel_Rect rect = get_sel_rect();

    for (int x = rect.left; x <= rect.right; x++) {
        for (int y = rect.top; y <= rect.bottom; y++) {
            Draw_SelHex(x, y, &image);
        }
    }
}

void Fill_Selection(int part)
{
    Sel_Rect rect = get_sel_rect();

    for (int x = rect.left; x <= rect.right; x++) {
        for (int y = rect.top; y <= rect.bottom; y++) {
            int field_pos = (y * Map.width) + x;
            if (!tile_is_protected(field_pos)) {
                Map.data[field_pos*2] = part;
                Redraw_Field(x, y, part, Map.data[(field_pos*2)+1]);
            }
        }
    }
}

bool point_in_selection(QPoint h)
{
    Sel_Rect rect = get_sel_rect();

    if ((h.x() >= rect.left) && (h.x() <= rect.right))
        if ((h.y() >= rect.top) && (h.y() <= rect.bottom))
            return true;

    return false;
}

//autogenerate
struct Autogen_Part_Rec {
    int index;
    int chance;
};

int autogenerate_get_part(Autogen_Part_Rec partRec[], int len) {
    int ran = QRandomGenerator::global()->bounded(1, 100);
    int num = 0;
    for (int i=0; i<=len; i++) {
        if ((ran >= num) && (ran < (num + partRec[i].chance))) {
            return partRec[i].index;
        }
        num = num + partRec[i].chance;
    }
    return 0;
}

void autogenerate_Selection(Autogen_Part_Rec partRec[], int len) {
    Sel_Rect rect = get_sel_rect();

    for (int x = rect.left; x <= rect.right; x++) {
        for (int y = rect.top; y <= rect.bottom; y++) {
            int field_pos = (y * Map.width) + x;
            int part = autogenerate_get_part(partRec, len);
            if (!tile_is_protected(field_pos)) {
                Map.data[field_pos*2] = part;
                Redraw_Field(x, y, part, Map.data[(field_pos*2)+1]);
            }
        }
    }
}

//forest
Autogen_Part_Rec Forest[] = {
    { 54, 19 }, //forest #1
    { 55, 19 },
    { 56, 19 },
    { 57, 20 },
    { 34, 3 }, //lake
    { 78, 8 }, //plain #2
    { 79, 4 }, //"stones"
    { 64, 2 }, //small green mountain
    { 75, 1 }, //brown hill #1
    { 76, 1 }, //brown hill #2
    { 80, 4 } //fields
};

void autogenerate_Forest() {
/*
    int num = 0;
    for (int i=0; i<=11; i++) {
        num = num + Forest[i].chance;
    }
    qDebug() <<"forest" << num;
*/
    autogenerate_Selection(Forest, 11);
}

//grassland
Autogen_Part_Rec Grassland[] = {
    { 0, 39 }, //plain
    { 78, 20 }, //plain #2
    { 79, 16 }, //"stones"
    { 80, 20 }, //fields
    { 81, 1 }, //swamp like
    { 82, 1 }, //swamp like #2
    { 54, 1 }, //forest #1
    { 34, 1 }, //lake
    { 64, 1 }, //small green mountain
};

void autogenerate_Grassland() {
/*
    int num = 0;
    for (int i=0; i<=8; i++) {
        num = num + Grassland[i].chance;
    }
    qDebug() << "grassland" << num;
*/
    autogenerate_Selection(Grassland, 8);
}

//city area
Autogen_Part_Rec Cityarea[] = {
    { 49, 29 }, //house #1
    { 50, 29 }, //house #2
    { 51, 3 }, //large house #1
    { 52, 1 }, //large house #2
    { 79, 11 }, //"stones"
    { 80, 11 }, //plain
    { 34, 4 }, //lake
    { 54, 5 }, //forest #1
    { 64, 1 }, //small green mountain
    { 0, 4 }, //plains #1
    { 75, 1 }, //brown hill #1
    { 76, 1 }, //brown hill #2
};

void autogenerate_Cityarea() {
/*
    int num = 0;
    for (int i=0; i<=11; i++) {
        num = num + Cityarea[i].chance;
    }
    qDebug() << "cityarea" << num;
*/
    autogenerate_Selection(Cityarea, 11);
}

//crater, battlefield
Autogen_Part_Rec Craterland[] = {
    { 81, 19 }, //swamp like
    { 82, 22 }, //swamp like #2
    { 83, 21 }, //small craters
    { 84, 20 }, //large crater
    { 79, 5 }, //"stones"
    { 34, 4 }, //lake
    { 167, 3 }, //round trench
    { 63, 6 }, //xx defense
};

void autogenerate_Craterland() {
/*
    int num = 0;
    for (int i=0; i<=7; i++) {
        num = num + Craterland[i].chance;
    }
    qDebug() << "craterland" << num;
*/
    autogenerate_Selection(Craterland, 7);
}

//lake, there must certainly be a far clever way to do this :-)
Autogen_Part_Rec Lake[] = {
    { 25, 90 }, //low water
    { 47, 7 }, //medium water
    { 60, 1 }, //stone #1
    { 61, 1 }, //stone #2
    { 62, 1 }, //stone #3
};

void autogenerate_Lake() {
    int field_pos;
    int part;
    bool equal;
    Sel_Rect rect = get_sel_rect();

    //draw water, later on we can expand with various depths
    for (int x = rect.left; x <= rect.right; x++) {
        for (int y = rect.top; y <= rect.bottom; y++) {
            field_pos = (y * Map.width) + x;
            equal =  (x % 2 != 0);
            part = autogenerate_get_part(Lake, 5);

            //left coast
            if (x == rect.left) {
                if ((y == rect.top && !equal) || (y == rect.bottom && equal)) {
                    //left corners, preserve hex
                    part = Map.data[field_pos*2];
                } else {
                    part = 32; //coast right {
                }
            }

            //right coast
            if (x == rect.right) {
                if ((y == rect.top && !equal) || (y == rect.bottom && equal)) {
                    //left corners, preserve hex
                    part = Map.data[field_pos*2];
                } else {
                    part = 33; //coast left }
                }
            }

            //north coast
            if (y == rect.top && x != rect.left && x != rect.right) {
                if (x == rect.left + 1 && equal) {
                    part = 27; // coast /
                } else if (x == rect.right - 1 && equal) {
                    part = 29; // coast /
                } else {
                    if (!equal) part = 31;
                }
            }

            //south coast
            if (y == rect.bottom && x != rect.left && x != rect.right) {
                if (x == rect.left + 1 && !equal) {
                    part = 28; //coast up left
                } else if (x == rect.right - 1 && !equal) {
                    part = 26; //coast up right
                } else {
                    if (equal) part = 30;
                }
            }

            if (!tile_is_protected(field_pos)) {
                Map.data[field_pos*2] = part;
                Redraw_Field(x, y, part, Map.data[(field_pos*2)+1]);
            }
        }
    }

}
