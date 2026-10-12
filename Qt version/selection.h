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

void Paint_Selection(QImage *image) {
    if (selection_start == selection_end) return;

    Sel_Rect rect = get_sel_rect();

    for (int x = rect.left; x <= rect.right; x++) {
        for (int y = rect.top; y <= rect.bottom; y++) {
            Draw_SelHex(x, y, image);
        }
    }
}

void Paint_Path_Selection(QImage *image) {
    if (selection_start == selection_end) return;

    QPen pen = QPen(Qt::yellow);
    pen.setStyle(Qt::DotLine);
    pen.setJoinStyle(Qt::RoundJoin);
    pen.setCosmetic(true);
    pen.setWidthF(Scale_factor * 1.15);

    Draw_Hexagon(selection_start.x(), selection_start.y(), pen, image, true, true);
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

QImage get_autogenerate_path_icon(int target)
{
    int part;

    switch (target) {
        case 1 : part = 147; break;
        case 2 : part = 128; break;
        case 3 : part = 87; break;
        case 4 : part = 37; break;
    }

    QPen pen = QPen(Qt::yellow);
    pen.setStyle(Qt::DotLine);
    pen.setJoinStyle(Qt::RoundJoin);
    pen.setCosmetic(true);
    pen.setWidth(1);

    //create a transparent image for the unit being dragged
    QImage partImg = QImage(Tilesize, Tilesize, QImage::Format_ARGB32_Premultiplied);
    partImg.fill(Qt::transparent);

    Draw_Part(0, 0, part, &partImg);
    Draw_Hexagon(0, 0, pen, &partImg, false, false);

    return partImg;
}

QCursor get_autogenerate_path_cursor(int target)
{
    QImage partImg = get_autogenerate_path_icon(target);
    QPixmap pixmap = QPixmap::fromImage( partImg.scaled(Tilesize * Scale_factor, Tilesize * Scale_factor));
    QCursor cursor = QCursor(pixmap, -Tilesize, -Tilesize);
    return cursor;
}

struct Autogen_Part_Rec {
    int index;
    double chance;
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
Autogen_Part_Rec DenseForest[] = {
    { 54, 20 }, //forest #1
    { 55, 19 },
    { 56, 19 },
    { 57, 20 },
    { 34, 2 }, //lake
    { 78, 8 }, //plain #2
    { 79, 4 }, //"stones"
    { 64, 2 }, //small green mountain
    { 75, 1 }, //brown hill #1
    { 76, 1 }, //brown hill #2
    { 80, 4 } //fields
};

Autogen_Part_Rec LightForest[] = {
    { 54, 8 }, //forest #1
    { 55, 8 },
    { 56, 8 },
    { 57, 8 },
    { 34, 1 }, //lake
    { 0, 19 }, //plain
    { 78, 19 }, //plain #2
    { 79, 8 }, //"stones"
    { 64, 1 }, //small green mountain
    { 75, 1 }, //brown hill #1
    { 76, 1 }, //brown hill #2
    { 77, 1 }, //brown hill #3
    { 49, 1 }, //house #1
    { 50, 1 }, //house #2
    { 80, 15 } //fields
};

void autogenerate_Forest(bool dense) {
    if (dense) {
        autogenerate_Selection(DenseForest, 11);
    } else {
        autogenerate_Selection(LightForest, 15);
    }
}

//grassland
Autogen_Part_Rec Grassland[] = {
    { 0, 34 }, //plain
    { 78, 21 }, //plain #2
    { 79, 16 }, //"stones"
    { 80, 22 }, //fields
    { 81, 1 }, //swamp like
    { 82, 1 }, //swamp like #2
    { 54, 1 }, //forest #1
    { 34, 1 }, //lake
    { 64, 1 }, //small green mountain
    { 75, 1 }, //brown hill #1
    { 76, 1 }, //brown hill #2
};

void autogenerate_Grassland() {
    int num = 0;
    for (int i=0; i<=11; i++) {
        num = num + Grassland[i].chance;
    }
    qDebug() << "grassland area" << num;

    autogenerate_Selection(Grassland, 11);
}

//city area
Autogen_Part_Rec Residentalarea[] = {
    { 49, 29 }, //house #1
    { 50, 29 }, //house #2
    { 51, 3 }, //large house #1
    { 52, 1 }, //large house #2
    { 79, 13 }, //"stones"
    { 80, 13 }, //plain
    { 34, 1 }, //lake
    { 54, 1 }, //forest #1
    { 55, 1 }, //forest #2
    { 56, 1 }, //forest #3
    { 57, 1 }, //forest #4
    { 64, 1 }, //small green mountain
    { 0, 4 }, //plains #1
    { 75, 1 }, //brown hill #1
    { 76, 1 }, //brown hill #2
};

void autogenerate_Residentalarea() {
    autogenerate_Selection(Residentalarea, 15);
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

// path, code for autogenerating roads, railroads and rivers
// hexPath (but only hexPath!) made with Grok AI,- could simply not figure it out my self :-(
struct Cube {
    int x = 0, y = 0, z = 0;
    Cube() = default;
    Cube(int x, int y, int z) : x(x), y(y), z(z) {}
};

struct PathPoint {
    QPoint pos;
    int dir = -1;    // 0–5 = direction to the next point, -1 = last point
};

// odd-q
Cube toCube(QPoint p) {
    int col = p.x();
    int row = p.y();
    int x = col;
    int z = row - (col - (col & 1)) / 2;
    int y = -x - z;
    return Cube(x, y, z);
}

QPoint toOffset(Cube c) {
    int col = c.x;
    int row = c.z + (c.x - (c.x & 1)) / 2;
    return QPoint(col, row);
}

int hexDistance(Cube a, Cube b) {
    return (std::abs(a.x - b.x) + std::abs(a.y - b.y) + std::abs(a.z - b.z)) / 2;
}

Cube cubeRound(double x, double y, double z) {
    int rx = std::round(x);
    int ry = std::round(y);
    int rz = std::round(z);
    double xDiff = std::abs(rx - x);
    double yDiff = std::abs(ry - y);
    double zDiff = std::abs(rz - z);
    if (xDiff > yDiff && xDiff > zDiff)
        rx = -ry - rz;
    else if (yDiff > zDiff)
        ry = -rx - rz;
    else
        rz = -rx - ry;
    return Cube(rx, ry, rz);
}

// The 6 directions for odd-q (same order as previous neighbours)
// 0: +1, 0    1: +1, -1    2:  0, -1
// 3: -1, -1    4: -1, 0    5:  0, +1   (for even columns; odd columns are offset)
static const QPoint kEvenDirs[6] = {
    {+1,  0}, {+1, -1}, { 0, -1},
    {-1, -1}, {-1,  0}, { 0, +1}
};
static const QPoint kOddDirs[6] = {
    {+1, +1}, {+1,  0}, { 0, -1},
    {-1,  0}, {-1, +1}, { 0, +1}
};

int directionTo(QPoint from, QPoint to) {
    const QPoint* dirs = (from.x() & 1) ? kOddDirs : kEvenDirs;
    QPoint delta = to - from;
    for (int i = 0; i < 6; ++i) {
        if (dirs[i] == delta)
            return i;
    }
    return -1; // should never happen on a 'legal' path
}

QList<PathPoint> hexPath(QPoint start, QPoint goal) {
    QList<PathPoint> path;
    Cube a = toCube(start);
    Cube b = toCube(goal);
    int n = hexDistance(a, b);
    if (n == 0) {
        path.append({start, -1});
        return path;
    }

    QList<QPoint> points;
    for (int i = 0; i <= n; ++i) {
        double t = static_cast<double>(i) / n;
        double x = a.x + (b.x - a.x) * t;
        double y = a.y + (b.y - a.y) * t;
        double z = a.z + (b.z - a.z) * t;
        points.append(toOffset(cubeRound(x, y, z)));
    }

    for (int i = 0; i < points.size(); ++i) {
        int dir = (i + 1 < points.size()) ? directionTo(points[i], points[i + 1]) : -1;
        path.append({points[i], dir});
    }
    return path;
}

struct Autogen_Tiles {
    int DIAG_RIGHT;  /* \  */
    int DIAG_LEFT;   /* /  */
    int VERTICAL;    /* |  */
    int ARC_1;       /* _| */
    int ARC_2;       /* ‾| */
    int ARC_3;       /* |‾ */
    int ARC_4;       /* |_ */
    int HORZ_DOWN;   /* /\ */
    int HORZ_UP;     /* \/ */
};

int direction_to_tile(Autogen_Tiles tiles, int dir, int prior) {
    if (dir == -1) dir = 5;//return tiles.ARC_3;

    qDebug() << "direction_to_tile" << dir << prior;

    if (dir == 0) {
        if (prior == tiles.ARC_2) {
            return tiles.ARC_4;
        }
        if (prior == tiles.ARC_3) {
            return tiles.ARC_4;
        }
        if (prior == tiles.HORZ_DOWN) {
            return tiles.DIAG_RIGHT;
        }
        if (prior == tiles.HORZ_UP) {
            return tiles.HORZ_DOWN;
        }
        if (prior == tiles.VERTICAL) {
            return tiles.ARC_4;
        }
        if (prior == tiles.DIAG_LEFT) {
            return tiles.HORZ_DOWN;
        }

        return tiles.DIAG_RIGHT;
    }

    if (dir == 1) {
        if (prior == tiles.HORZ_UP) {
            return tiles.DIAG_LEFT;
        }
        if (prior == tiles.DIAG_LEFT) {
            return tiles.DIAG_LEFT;
        }
        if (prior == tiles.ARC_1) {
            return tiles.ARC_3;
        }
        if (prior == tiles.VERTICAL) {
            return tiles.ARC_3;
        }
        if (prior == tiles.ARC_3) {
            return tiles.DIAG_LEFT;
        }
        return tiles.HORZ_UP;
    }

    if (dir == 2) {
        if (prior == tiles.ARC_1) {
            return tiles.VERTICAL;
        }
        if (prior == tiles.ARC_2) {
            return tiles.ARC_4;
        }
        if (prior == tiles.ARC_4) {
            return tiles.VERTICAL;
        }
        if (prior == tiles.VERTICAL) {
            return tiles.VERTICAL;
        }
        if (prior == tiles.DIAG_RIGHT) {
            return tiles.DIAG_RIGHT;
        }
        if (prior == tiles.HORZ_UP) {
            return tiles.ARC_1;
        }
    }

    if (dir == 3) {
        if (prior == tiles.HORZ_UP) {
            return tiles.DIAG_RIGHT;
        }
        if (prior == tiles.ARC_1) {
            return tiles.ARC_2;
        }
        if (prior == tiles.ARC_2) {
            return tiles.DIAG_RIGHT;
        }
        if (prior == tiles.ARC_4) {
            return tiles.ARC_2;
        }
        if (prior == tiles.DIAG_RIGHT) {
            return tiles.DIAG_RIGHT; //ARC_4
        }
        if (prior == tiles.VERTICAL) {
            return tiles.ARC_2;
        }
        return tiles.HORZ_UP;
    }

    if (dir == 4) {
        if (prior == tiles.ARC_3) {
            return tiles.ARC_1;
        }
        if (prior == tiles.VERTICAL) {
            return tiles.ARC_1;
        }
        if (prior == tiles.HORZ_UP) {
            return tiles.HORZ_DOWN;
        }
        if (prior == tiles.DIAG_RIGHT) {
            return tiles.HORZ_DOWN;
        }
        return tiles.DIAG_LEFT;
    }

    if (dir == 5) {
        if (prior == tiles.DIAG_RIGHT) {
            return tiles.DIAG_RIGHT; //ARC_2;
        }
        if (prior == tiles.ARC_3) {
            return tiles.VERTICAL;
        }
        if (prior == tiles.VERTICAL) {
            return tiles.VERTICAL;
        }
        if (prior == tiles.ARC_2) {
            return tiles.VERTICAL;
        }
        if (prior == tiles.ARC_4) {
            return tiles.ARC_2;
        }
        if (prior == 0) {
            return tiles.DIAG_RIGHT;
        }

        return tiles.ARC_3;
    }

    return tiles.ARC_1;
}

const Autogen_Tiles Dirtroad_Tiles = {
    148,   /* \  */
    147,   /* /  */
    146,   /* |  */
    152,   /* _| */
    151,   /* ‾| */
    149,   /* |‾ */
    150,   /* |_ */
    154,   /* /\ */
    153    /* \/ */
};

const Autogen_Tiles Road_Tiles = {
    129,   /* \  */
    128,   /* /  */
    127,   /* |  */
    133,   /* _| */
    132,   /* ‾| */
    130,   /* |‾ */
    131,   /* |_ */
    135,   /* /\ */
    134    /* \/ */
};

const Autogen_Tiles Railroad_Tiles = {
    86,   /* \  */
    87,   /* /  */
    85,   /* |  */
    88,   /* _| */
    89,   /* ‾| */
    90,   /* |‾ */
    91,   /* |_ */
    93,   /* /\ */
    92    /* \/ */
};

const Autogen_Tiles River_Tiles = {
    36,   /* \  */
    37,   /* /  */
    38,   /* |  */
    39,   /* _| */
    40,   /* ‾| */
    41,   /* |‾ */
    42,   /* |_ */
    43,   /* /\ */
    44    /* \/ */
};

void autogenerate_path(QPoint from, QPoint to, int target, bool draw_only) {
    int prior = 0;
    int prior_dir;
    int field_pos;
    int part;
    Autogen_Tiles tiles;
    QPoint start;
    QPoint end;

    int f = from.x() * from.y();
    int t = to.x() * to.y();

    if (f > t) {
        start = from;
        end = to;
    } else {
        start = to;
        end = from;
    }

    qDebug() << "path" << from << to;

    switch (target) {
        case 1: tiles = Dirtroad_Tiles; break;
        case 2: tiles = Road_Tiles; break;
        case 3: tiles = Railroad_Tiles; break;
        case 4: tiles = River_Tiles; break;
        default: return; break;
    }

    QList<PathPoint> path = hexPath(start, end);
    for (const PathPoint& p : path) {
        field_pos = (p.pos.y() * Map.width) + p.pos.x();

        if (p.dir > 0) {
            part = direction_to_tile(tiles, p.dir, prior);
            prior_dir = p.dir;
        } else {
            part = direction_to_tile(tiles, prior_dir, prior);
        }

        if (!draw_only) Map.data[field_pos*2] = part;

        Redraw_Field(p.pos.x(), p.pos.y(), part, Map.data[(field_pos*2)+1]);

        qDebug() << p.pos << p.dir << prior << part;
        prior = part;

    }
    qDebug() << "------------------------------";
}


void autogenerate_Paint_Map(bool refresh)
{
    if (refresh) {
       scrollArea_current_label = NULL;
       selected_pos = QPoint();
    }

    if (scrollArea_current_label == NULL) //first time or refresh
    {
        if (!MapImage.isNull()) MapImage = QImage(); //Release mem for the last used image
        if (!MapImageScaled.isNull()) MapImageScaled = QImage(); //Release mem for the last used scaled image

        MapImage = QImage(((Map.width/2) * Tilesize) + (((Map.width/2)-1) * Tileshift), ((Map.height-1) * Tilesize) + (Tilesize/2), QImage::Format_RGB16); //Create a new QImage object for the map image
        MapImage.fill(Qt::transparent);

        Draw_Map(); //and draw the map to it

        MapImage = MapImage.convertToFormat(QImage::Format_Grayscale8);
        MapImage = MapImage.convertToFormat(QImage::Format_ARGB32);

        if (!selection_start.isNull() && !selecting_path_last_pos.isNull()) {
            autogenerate_path(selection_start, selecting_path_last_pos, selecting_path_target, true);
        }

        MapImageScaled = MapImage.scaled(MapImage.width()*Scale_factor,MapImage.height()*Scale_factor); //Create a scaled version of it
        if (grid_enabled) ShowGrid();
        if (coords_enabled) ShowCoords();

        if (!selected_pos.isNull())
            Draw_Hexagon(selected_pos.x(), selected_pos.y(), QPen(Qt::red, 1), &MapImageScaled, true, true);

        if (!selection_start.isNull()) {
            if (SELECTING_PATH) {
                Paint_Path_Selection(&MapImageScaled);
            } else {
                Paint_Selection(&MapImageScaled);
            }
        }

        QLabel *imageLabel = new QLabel;
        imageLabel->setPixmap(QPixmap::fromImage(MapImageScaled));
        imageLabel->setAttribute(Qt::WA_TransparentForMouseEvents); //so we can respond to mouse events on the map, not just click
        scrollArea_current_label = imageLabel;

        if (scrollArea == NULL) scrollArea = new(QScrollArea);
        scrollArea->setWidget(imageLabel);
   } else {
        int pos_x = scrollArea->horizontalScrollBar()->value();
        int pos_y = scrollArea->verticalScrollBar()->value();

        MapImageScaled = MapImage.scaled(MapImage.width() * Scale_factor, MapImage.height() * Scale_factor);
        if (grid_enabled) ShowGrid();
        if (coords_enabled) ShowCoords();

        if (!selected_pos.isNull())
            Draw_Hexagon(selected_pos.x(), selected_pos.y(), QPen(Qt::red, 1), &MapImageScaled, true, true);

        if (!selection_start.isNull()) {
            if (SELECTING_PATH) {
                Paint_Path_Selection(&MapImageScaled);
            } else {
                Paint_Selection(&MapImageScaled);
            }
        }

        if (!selection_start.isNull() && !selecting_path_last_pos.isNull()) {
            autogenerate_path(selection_start, selecting_path_last_pos, selecting_path_target, true);
        }

        QLabel *imageLabel = new QLabel;
        imageLabel->setPixmap(QPixmap::fromImage(MapImageScaled));
        imageLabel->setAttribute(Qt::WA_TransparentForMouseEvents);
        scrollArea_current_label = imageLabel;
        scrollArea->setWidget(scrollArea_current_label);

        scrollArea->horizontalScrollBar()->setValue(pos_x); //Reset the scrollArea to last position
        scrollArea->verticalScrollBar()->setValue(pos_y);
   }
   scrollArea->setMouseTracking(true);
   scrollArea_current_label->setMouseTracking(true);
}
