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
            border.lineTo(xp, yp + ((Tilesize / 2) * sf));
        } else {
            border.moveTo(xp + ((Tileshift*2) * sf), yp + (Tilesize*sf));
            border.lineTo(xp + (Tileshift*sf) + 1, yp + (Tilesize*sf));
        }
        painter.drawPath(border);
    }

    if (x == rect.right) {
        border.moveTo(xp + ((Tileshift*2) * sf), yp + 2);
        border.lineTo(xp + (Tilesize * sf), yp + ((Tilesize/2) * sf));
        border.lineTo(xp + ((Tileshift*2) * sf), yp + (Tilesize*sf));
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
            Map.data[field_pos*2] = part;
            Redraw_Field(x, y, part, Map.data[(field_pos*2)+1]);
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
