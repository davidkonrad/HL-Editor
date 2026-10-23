/*
 * HL Editor
 *
 * selection.h by David Konrad
 *
 * Provides functions and data structures for handling selections on the map.
 *
 */

struct Sel_Rect {
    int left;
    int right;
    int top;
    int bottom;
    int width;
    int height;
    //debug, will eventually be removed
    friend QDebug operator<<(QDebug dbg, const Sel_Rect &rect) {
         QDebugStateSaver saver(dbg);
         dbg.nospace() << "Sel_Rect("
                       << "left=" << rect.left
                       << ", right=" << rect.right
                       << ", top=" << rect.top
                       << ", bottom=" << rect.bottom
                       << ", width=" << rect.width
                       << ", height=" << rect.height
                       << ")";
         return dbg;
     }
};

struct Protected_Terrain {
    bool Buildings = true;
    bool Roads = true;
    bool Water = true;
    bool River = true;
};

Protected_Terrain protected_terrain; //global

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

QPainterPath selectionOutlinePath(QPoint start, QPoint end, bool scaled = true)
{
    QPainterPath path;

    int x1 = qMin(start.x(), end.x());
    int x2 = qMax(start.x(), end.x());
    int y1 = qMin(start.y(), end.y());
    int y2 = qMax(start.y(), end.y());

    double sf = scaled ? Scale_factor : 1;
    int selHeight = (y2 + 1) * Tilesize * sf;

    auto hexTopLeft = [&](int col, int row) -> QPointF {
        if (scaled) {
            double xp = col * (Tilesize - Tileshift) * sf;
            double yp = (row * Tilesize) * sf;
            if (col % 2 != 0)
                yp += (Tilesize / 2.0) * sf;
            return QPointF(xp, yp);
        } else {
            double xp = (col - x1 + 1) * (Tilesize - Tileshift) * sf;
            double yp = ((row - y1 + 1) * Tilesize) * sf;
            if ((col - x1) % 2 != 0)
                yp += (Tilesize / 2.0) * sf;
            return QPointF(xp, yp);
        }
    };

    const QPointF c[6] = {
        QPointF(0,                 Tilesize / 2.0),
        QPointF(Tileshift,         0),
        QPointF(Tileshift * 2,     0),
        QPointF(Tilesize,          Tilesize / 2.0),
        QPointF(Tileshift * 2,     Tilesize),
        QPointF(Tileshift,         Tilesize)
    };

    // TOP
    for (int col = x1; col <= x2; ++col) {
        QPointF tl = hexTopLeft(col, y1);
        path.moveTo(tl + c[1] * sf);
        path.lineTo(tl + c[2] * sf);
        if (col < x2) {
            path.lineTo(hexTopLeft(col + 1, y1) + c[1] * sf);
        }
    }

    // RIGHT
    for (int row = y1; row <= y2; ++row) {
        QPointF tl = hexTopLeft(x2, row);
        path.moveTo(tl + c[3] * sf);
        path.lineTo(tl + c[4] * sf);
        if (row > y1) {
            path.moveTo(tl + c[3] * sf);
            path.lineTo(tl + c[2] * sf);
        }
    }

    // BOTTOM
    for (int col = x2; col >= x1; --col) {
        QPointF tl = hexTopLeft(col, y2);
        path.moveTo(tl + c[4] * sf);
        path.lineTo(tl + c[5] * sf);

        // outer angles, not corners
        if (col > x1) {
            path.moveTo(tl + c[0] * sf);
            QPointF h =  tl + c[5] * sf;
            if (h.y() > selHeight) //prevent fram drawing inside the selection
                path.lineTo(tl + c[5] * sf);
        }

        if (col < x2) {
            path.moveTo(tl + c[3] * sf);
            QPointF h =  tl + c[5] * sf;
            if (h.y() > selHeight)
                path.lineTo(tl + c[4] * sf);
        }

    }

    // LEFT
    for (int row = y2; row >= y1; --row) {
        QPointF tl = hexTopLeft(x1, row);
        path.moveTo(tl + c[0] * sf);
        path.lineTo(tl + c[5] * sf);
        if (row > y1) {
            path.moveTo(tl + c[0] * sf);
            path.lineTo(tl + c[1] * sf);
        }
    }

    // corners
    {
        QPointF tl = hexTopLeft(x1, y1);
        path.moveTo(tl + c[0] * sf);
        path.lineTo(tl + c[1] * sf);
    }
    {
        QPointF tl = hexTopLeft(x2, y1);
        path.moveTo(tl + c[3] * sf);
        path.lineTo(tl + c[2] * sf);
    }

    return path;
}


void Draw_SelHex(int x, int y, bool bordered, QImage *image)
{
    double sf = Scale_factor;
    int xp, yp;

    xp = x * (Tilesize-Tileshift);
    if (x % 2 != 0)
        yp = (y * Tilesize) + (Tilesize / 2);
    else
        yp = (y * Tilesize) ;

    xp = xp * sf;
    yp = yp * sf;

    QPainter painter(image);
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

    painter.fillPath(path, QBrush(QColor(100, 100, 100, 128)));
    painter.end();
}


void Paint_Selection_Drag_Border(QImage *image)
{
    QPainter painter(image);
    QPainterPath outline = selectionOutlinePath(selection_start, selection_end, false);

    QPen pen(Qt::yellow, 1);

    pen.setStyle(Qt::DotLine);
    pen.setJoinStyle(Qt::RoundJoin);
    pen.setCapStyle(Qt::RoundCap);
    painter.setPen(pen);

    painter.setPen(pen);
    painter.drawPath(outline);
}

void Paint_Selection_Border(QImage *image)
{
    QPainter painter(image);
    QPainterPath outline = selectionOutlinePath(selection_start, selection_end);

    // shadow
    painter.save();

    if (Scale_factor > 1.5) {
        painter.translate(2, 2); //
    }

    QPen shadowPen(QColor(0, 0, 0, 80), 3);
    shadowPen.setJoinStyle(Qt::RoundJoin);
    shadowPen.setCapStyle(Qt::RoundCap);

    painter.setPen(shadowPen);
    painter.drawPath(outline);
    painter.restore();

    // outer
    QPen outerPen(Qt::yellow, 3);
    outerPen.setJoinStyle(Qt::RoundJoin);
    outerPen.setCapStyle(Qt::RoundCap);
    painter.setPen(outerPen);
    painter.drawPath(outline);

    // inner
    QPen innerPen(QColor(128, 128, 0), 1);
    innerPen.setJoinStyle(Qt::RoundJoin);
    innerPen.setCapStyle(Qt::RoundCap);
    painter.setPen(innerPen);
    painter.drawPath(outline);
}

void Paint_Selection(QImage *image) {
    if (selection_start == selection_end) return;
    Sel_Rect rect = get_sel_rect();

    for (int x = rect.left; x <= rect.right; x++) {
        for (int y = rect.top; y <= rect.bottom; y++) {
            Draw_SelHex(x, y, true, image);
        }
    }
    Paint_Selection_Border(image);
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

bool field_is_protected(int field_pos) {
    int tile = Map.data[field_pos*2];
    if (protected_terrain.Buildings) {
        if (tile_is_building(tile)) return true;
    }
    if (protected_terrain.Roads) {
        if (tile_is_road(tile)) return true;
    }
    if (protected_terrain.Water) {
        if (tile_is_water(tile)) return true;
    }
    if (protected_terrain.River) {
        if (tile_is_river(tile)) return true;
    }
    return false;
}

void Fill_Selection(int part)
{
    Sel_Rect rect = get_sel_rect();

    for (int x = rect.left; x <= rect.right; x++) {
        for (int y = rect.top; y <= rect.bottom; y++) {
            int field_pos = (y * Map.width) + x;
            if (!field_is_protected(field_pos)) {
                Map.data[field_pos*2] = part;
                Redraw_Field(x, y, part, Map.data[(field_pos*2)+1]);
            }
        }
    }
}

//
void reset_selection()
{
    SELECTING_TILES = false;
    selection_start = QPoint(-1, -1);
    selection_end = QPoint(-1, -1);
}

bool has_selection()
{
    return (selection_start.x() > -1 && selection_end.x() > -1);
}

bool point_in_selection(QPoint h)
{
    Sel_Rect rect = get_sel_rect();
    if ((h.x() >= rect.left) && (h.x() <= rect.right))
        if ((h.y() >= rect.top) && (h.y() <= rect.bottom))
            return true;

    return false;
}

//
QImage get_part_image(int part, bool bordered = false, bool grayscale = false) {
    QImage partImg = QImage(Tilesize, Tilesize, QImage::Format_ARGB32_Premultiplied);
    partImg.fill(Qt::transparent);

    Draw_Part(0, 0, part, &partImg);

    if (grayscale)
        partImg = grayscale_transparent_image(partImg);

    if (bordered) {
        QPen pen = QPen(Qt::gray);
        pen.setJoinStyle(Qt::RoundJoin);
        pen.setWidth(2);
        Draw_Hexagon(0, 0, pen, &partImg, false, false);
    }
    return partImg;
}

QIcon get_part_icon(int part, bool bordered = false, bool grayscale = false) {
    return QPixmap::fromImage( get_part_image(part, true, grayscale));
}

QImage get_autogenerate_path_icon(int target, bool hexagon)
{
    int part;

    switch (target) {
        case 1 : part = 147; break;
        case 2 : part = 128; break;
        case 3 : part = 87; break;
        case 4 : part = 37; break;
        case 5 : part = 168; break;
        case 6 : part = 30; break;
        case 7 : part = 31; break;
    }

    QImage partImg = get_part_image(part);

    if (hexagon) {
        QPen pen = QPen(Qt::yellow);
        pen.setStyle(Qt::DotLine);
        pen.setJoinStyle(Qt::RoundJoin);
        pen.setCosmetic(true);
        pen.setWidth(1);
        Draw_Hexagon(0, 0, pen, &partImg, false, false);
    }

    return partImg;
}

QCursor get_autogenerate_path_cursor(int target)
{
    QImage partImg = get_autogenerate_path_icon(target, true);
    QPixmap pixmap = QPixmap::fromImage( partImg.scaled(Tilesize * Scale_factor, Tilesize * Scale_factor));
    QCursor cursor = QCursor(pixmap, -Tilesize, -Tilesize);
    return cursor;
}

struct Autogen_Part_Rec {
    int index;
    double chance;
};

bool lucky(int percentage)
{
    int ran = QRandomGenerator::global()->bounded(1, 100);
    return ran <= percentage;
}

QPoint random_pos() {
    Sel_Rect rect = get_sel_rect();
    int x = QRandomGenerator::global()->bounded(rect.left + 1, rect.right - 1);
    int y = QRandomGenerator::global()->bounded(rect.top + 1, rect.bottom);
    return QPoint(x, y);
}

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
            if (!field_is_protected(field_pos)) {
                Map.data[field_pos*2] = part;
                Redraw_Field(x, y, part, Map.data[(field_pos*2)+1]);
            }
        }
    }
}

void autogenerate_Hillside(int attempts = 3)
{
    for (int i=0; i < attempts; i++) {
        if (lucky(15)) {
            QPoint p = random_pos();
            QPoint p2 = p + QPoint(1,0);

            //do not accidently overwrite eexisting mountain or building
            if ((!tile_is_building(tile_at_pos(p)) && !tile_is_mountain(tile_at_pos(p))) &&
                (!tile_is_building(tile_at_pos(p2)) && !tile_is_mountain(tile_at_pos(p2)))) {

                int field_pos = (p.y() * Map.width) + p.x();
                Map.data[field_pos*2] = 65;
                Redraw_Field(p.x(), p.y(), 65, Map.data[(field_pos*2)+1]);

                field_pos = (p2.y() * Map.width) + p2.x();
                Map.data[field_pos*2] = 66;
                Redraw_Field(p2.x(), p2.y(), 66, Map.data[(field_pos*2)+1]);
            }
        }
    }
}

void autogenerate_Mountains(int attempts = 3)
{
    for (int i=0; i < attempts; i++) {
        if (lucky(25)) {
            QPoint p = random_pos();

            //do not accidently overwrite eexisting mountain or building
            if (!tile_is_building(tile_at_pos(p)) && !tile_is_mountain(tile_at_pos(p))) {

                QList<QPoint> nb = mapPosNeighbors(p);
                for (int i=1; i<=4; i++) {
                   if (tile_is_building(tile_at_pos(nb[i])) || tile_is_mountain(tile_at_pos(nb[i]))) return;
                }

                if (lucky(50)) {
                    place_mountain_on_map(p, 68); //green bottom
                } else {
                    place_mountain_on_map(p, 72); //brown bottom
                }
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
    { 34, 1 }, //lake
    { 78, 8 }, //plain #2
    { 79, 4 }, //"stones"
    { 64, 2 }, //small green mountain
    { 75, 1 }, //brown hill #1
    { 76, 1 }, //brown hill #2
    { 77, 1 }, //brown hill #3
    { 80, 4 } //fields
};

Autogen_Part_Rec LightForest[] = {
    { 54, 8 },  //forest #1
    { 55, 8 },
    { 56, 8 },
    { 57, 8 },
    { 34, 1 },  //lake
    { 0, 19 },  //plain
    { 78, 19 }, //plain #2
    { 79, 8 },  //"stones"
    { 64, 1 },  //small green mountain
    { 75, 1 },  //brown hill #1
    { 76, 1 },  //brown hill #2
    { 77, 1 },  //brown hill #3
    { 49, 1 },  //house #1
    { 50, 1 },  //house #2
    { 80, 15 }  //fields
};

void autogenerate_Forest(bool dense) {
    if (dense) {
        autogenerate_Selection(DenseForest, 11);
    } else {
        autogenerate_Selection(LightForest, 15);
    }
}

//plains
Autogen_Part_Rec Plains[] = {
    { 0, 34 },  //plain
    { 78, 23 }, //plain #2
    { 79, 16 }, //"stones"
    { 80, 22 }, //fields
    { 81, 1 },  //swamp like
    { 54, 1 },  //forest #1
    { 57, 1 },  //forest #4
    { 34, 1 },  //lake
    { 64, 1 },  //small green mountain
};

void autogenerate_Plains() {
    int num = 0;
    for (int i=0; i<=9; i++) {
        num = num + Plains[i].chance;
    }
    //qDebug() << "plains" << num;
    autogenerate_Selection(Plains, 9);
}

//wasteland
Autogen_Part_Rec Wasteland[] = {
    { 0, 17 }, //plain
    { 78, 17 }, //plain #2
    { 79, 16 }, //"stones"
    { 80, 17 }, //fields
    { 81, 2 }, //swamp like
    { 82, 2 }, //swamp like #2
    { 54, 6 }, //forest #1
    { 55, 6 }, //forest #2
    { 56, 6 }, //forest #3
    { 57, 6 }, //forest #4
    { 34, 1 }, //lake
    { 64, 1 }, //small green mountain
    { 75, 1 }, //brown hill #1
    { 76, 1 }, //brown hill #2
    { 77, 1 }, //brown hill #3
};

void autogenerate_Wasteland() {
    Sel_Rect rect = get_sel_rect();
    int ha = (rect.width * rect.height) / 4; //larger selection = more attempts
    int ma = (rect.width * rect.height) * 3;
    autogenerate_Selection(Wasteland, 15);
    autogenerate_Mountains(ma);
    autogenerate_Hillside(ha);
}


//residental area
Autogen_Part_Rec Residentalarea[] = {
    { 49, 25 }, //house #1
    { 50, 25 }, //house #2
    { 51, 1 }, //large house #1
    { 52, 1 }, //large house #2
    { 79, 15 }, //"stones"
    { 80, 15 }, //plain
    { 34, 1 }, //lake
    { 54, 1 }, //forest #1
    { 55, 1 }, //forest #2
    { 56, 1 }, //forest #3
    { 57, 1 }, //forest #4
    { 64, 1 }, //small green mountain
    { 0, 10 }, //plains #1
    { 75, 1 }, //brown hill #1
    { 76, 1 }, //brown hill #2
};

void autogenerate_Residentalarea() {
    int num = 0;
    for (int i=0; i<=15; i++) {
        num = num + Residentalarea[i].chance;
    }
    //qDebug() << "Residentalarea" << num;
    autogenerate_Selection(Residentalarea, 15);
}

//battlefield
Autogen_Part_Rec Battlefield[] = {
    { 81, 19 }, //swamp like
    { 82, 22 }, //swamp like #2
    { 83, 21 }, //small craters
    { 84, 20 }, //large crater
    { 79, 5 }, //"stones"
    { 34, 4 }, //lake
    { 167, 3 }, //round trench
    { 63, 6 }, //xx defense
};

void autogenerate_Battlefield() {
    autogenerate_Selection(Battlefield, 7);
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
    bool water_protected = protected_terrain.Water;
    Sel_Rect rect = get_sel_rect();

    protected_terrain.Water = false; //temp allow overwrite water

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

            if (!field_is_protected(field_pos)) {
                Map.data[field_pos*2] = part;
                Redraw_Field(x, y, part, Map.data[(field_pos*2)+1]);
            }
        }
    }
    protected_terrain.Water = water_protected;
}

void autogenerate_Island() {
    int field_pos;
    int part;
    bool equal;
    bool water_protected = protected_terrain.Water;
    Sel_Rect rect = get_sel_rect();

    protected_terrain.Water = false; //temp allow overwrite water

    for (int x = rect.left; x <= rect.right; x++) {
        for (int y = rect.top; y <= rect.bottom; y++) {
            field_pos = (y * Map.width) + x;
            equal =  (x % 2 != 0);
            part = autogenerate_get_part(Plains, 9);

            //left coast
            if (x == rect.left) {
                if ((y == rect.top && !equal) || (y == rect.bottom && equal)) {
                    //left corners, preserve hex
                    part = Map.data[field_pos*2];
                } else {
                    part = 33; //coast left {
                }
            }

            //right coast
            if (x == rect.right) {
                if ((y == rect.top && !equal) || (y == rect.bottom && equal)) {
                    //left corners, preserve hex
                    part = Map.data[field_pos*2];
                } else {
                    part = 32; //coast right }
                }

            }

            //north coast
            if (y == rect.top && x != rect.left && x != rect.right) {
                if (x == rect.left + 1 && !equal) {
                    part = 28; //coast up left
                } else if (x == rect.right - 1 && !equal) {
                    part = 26; //coast up right
                } else if (equal) {
                    part = 30;
                } else {
                    part = Map.data[field_pos*2];
                }
            }

            //south coast
            if (y == rect.bottom && x != rect.left && x != rect.right) {
                if (x == rect.left + 1 && equal) {
                    part = 27; // coast /
                } else if (x == rect.right - 1 && equal) {
                    part = 29; // coast /
                } else if (!equal) {
                    part = 31;
                } else {
                    part = Map.data[field_pos*2];
                }
            }

            if (!field_is_protected(field_pos)) {
                Map.data[field_pos*2] = part;
                Redraw_Field(x, y, part, Map.data[(field_pos*2)+1]);
            }
        }
    }
    protected_terrain.Water = water_protected;
}


void autogenerate_Paint_Map(bool refresh)
{
    int pos_x = scrollArea->horizontalScrollBar()->value();
    int pos_y = scrollArea->verticalScrollBar()->value();

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

        for (int x = -1; x <= Map.width-1; x++) {
            for (int y = -1; y <= Map.height-1; y++) {
                Draw_SelHex(x, y, false, &MapImage);
            }
        }

        autogenerateMapImage = MapImage.copy();

        if (!selection_start.isNull() && !selecting_path_last_pos.isNull()) {
            autogenerate_path(selection_start, selecting_path_last_pos, selecting_path_target, true);
        }

        MapImageScaled = MapImage.scaled(MapImage.width()*Scale_factor,MapImage.height()*Scale_factor); //Create a scaled version of it

        if (grid_enabled) ShowGrid();
        if (coords_enabled) ShowCoords();

        if (!selection_start.isNull()) {
            if (SELECTING_PATH) {
                Paint_Path_Selection(&MapImageScaled);
            }
        }

        if (!selected_pos.isNull()) {
            Draw_Hexagon(selected_pos.x(), selected_pos.y(), QPen(Qt::red, 1), &MapImageScaled, true, true);
        }

        QLabel *imageLabel = new QLabel;
        imageLabel->setPixmap(QPixmap::fromImage(MapImageScaled));
        imageLabel->setAttribute(Qt::WA_TransparentForMouseEvents); //so we can respond to mouse events on the map, not just click
        scrollArea_current_label = imageLabel;

        if (scrollArea == NULL) scrollArea = new(QScrollArea);
        scrollArea->setWidget(imageLabel);
   } else {
        MapImage = autogenerateMapImage.copy();

        if (!selection_start.isNull() && !selecting_path_last_pos.isNull()) {
            autogenerate_path(selection_start, selecting_path_last_pos, selecting_path_target, true);
        }

        MapImageScaled = MapImage.scaled(MapImage.width() * Scale_factor, MapImage.height() * Scale_factor);

        if (grid_enabled) ShowGrid();
        if (coords_enabled) ShowCoords();

        if (!selection_start.isNull()) {
            if (SELECTING_PATH) {
                Paint_Path_Selection(&MapImageScaled);
            }
        }

        if (!selected_pos.isNull()) {
            Draw_Hexagon(selected_pos.x(), selected_pos.y(), QPen(Qt::red, 1), &MapImageScaled, true, true);
        }

        QLabel *imageLabel = new QLabel;
        imageLabel->setPixmap(QPixmap::fromImage(MapImageScaled));
        imageLabel->setAttribute(Qt::WA_TransparentForMouseEvents);
        scrollArea_current_label = imageLabel;
        scrollArea->setWidget(scrollArea_current_label);
   }

   scrollArea->horizontalScrollBar()->setValue(pos_x); //Reset the scrollArea to last position
   scrollArea->verticalScrollBar()->setValue(pos_y);
}
