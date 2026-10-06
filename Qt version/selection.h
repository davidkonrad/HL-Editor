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
} Sel_Rect;


bool tile_is_road(int tile) {
    if (tile >= 85 && tile <= 104) return true; //railroad
    if (tile >= 146 && tile <= 164) return true; //dirtroad
    if (tile >= 127 && tile <= 145) return true; //highway
    return false;
}

bool tile_is_building(int tile) {
    if (tile >= 1 && tile <= 24) return true;
    return false;
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

    return r;
}

void Draw_SelHex(int x, int y, QImage *Image)
{
    double sf = Scale_factor;
    bool equal =  (x % 2 != 0);
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
    if ((selection_start == selection_end) ||
        (selection_start.isNull()) ||
        (selection_end.isNull())) return;

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


int get_part(Autogen_Part_Rec partRec[], int len) {
    int ran = QRandomGenerator::global()->bounded(1, 100);
    int num = 0;
    for (int i=0; i<len; i++) {
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
            int part = get_part(partRec, len);
            if (!tile_is_protected(field_pos)) {
                Map.data[field_pos*2] = part;
                Redraw_Field(x, y, part, Map.data[(field_pos*2)+1]);
            }
        }
    }
}

//forest
Autogen_Part_Rec Forest[] = {
    { 54, 20 }, //forest #1
    { 55, 20 },
    { 56, 20 },
    { 57, 19 },
    { 34, 7 }, //lake
    { 78, 7 }, //plain #2
    { 79, 7 } //"stones"
};

void autogenerate_Forest() {
    autogenerate_Selection(Forest, 7);
}

//grassland
Autogen_Part_Rec Grassland[] = {
    { 0, 40 }, //plain
    { 78, 18 }, //plain #2
    { 79, 18 }, //"stones"
    { 80, 18 }, //fields
    { 81, 6 }, //swamp like
};

void autogenerate_Grassland() {
    autogenerate_Selection(Grassland, 5);
}

//city area
Autogen_Part_Rec Cityarea[] = {
    { 49, 35 }, //house #1
    { 50, 35 }, //house #2
    { 51, 10 }, //large house
    { 79, 5 }, //"stones"
    { 80, 5 }, //plain
    { 34, 5 }, //lake
    { 54, 5 } //forest #1
};

void autogenerate_Cityarea() {
    autogenerate_Selection(Cityarea, 7);
}

//crater, battlefield
Autogen_Part_Rec Craterland[] = {
    { 81, 16 }, //swamp like
    { 82, 20 }, //swamp like #2
    { 83, 20 }, //small craters
    { 84, 20 }, //large crater
    { 79, 6 }, //"stones"
    { 34, 6 }, //lake
    { 167, 6 }, //round trench
    { 63, 6 }, //xx defense
};

void autogenerate_Craterland() {
    autogenerate_Selection(Craterland, 8);
}

//lake, there must certainly be a far clever way to do this :-)
void autogenerate_Lake() {
    Sel_Rect rect = get_sel_rect();

    for (int x = rect.left; x <= rect.right; x++) {
        for (int y = rect.top; y <= rect.bottom; y++) {
            int field_pos = (y * Map.width) + x;
            bool equal =  (x % 2 != 0);
            int part = 25;

            if (x == rect.left) {
               if (y == rect.bottom && equal) {
                  //keep the corner
                  part = Map.data[field_pos*2];
               } else {
                  part = 32;
               }
            }

            if (x == rect.left + 1 && !equal) {
               if (y == rect.bottom) {
                  part = 28;
               } else {
                   part = 25;
               }
            }

            if (x == rect.right) {
               if (y == rect.top && !equal) {
                   //keep the corner
                   part = Map.data[field_pos*2];
               } else {
                   part = 33;
               }
            }

            if (x == rect.right - 1) {
               if (y == rect.top && equal) {
                   part = 29;
               }
            }

            //upper part of lake
            if (y == rect.top && x != rect.left && x != rect.right) {
                  if (!equal) part = 31;
            }

            //lower part of lake
            if (y == rect.bottom && x != rect.left && x != rect.right) {
                  if (equal) part = 30;
            }

            if (!tile_is_protected(field_pos)) {
                Map.data[field_pos*2] = part;
                Redraw_Field(x, y, part, Map.data[(field_pos*2)+1]);
            }
        }
    }
}
