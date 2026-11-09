/*
 * HL Editor
 *
 * autoplace.h by David Konrad
 *
 * Place mountains, buildings etc correctly on the map
 *
 */

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

/*
 * I know there is a smarter way to do this, but that will do for now
 */
void place_lake_on_map(QPoint p, int type) {
/*
  1  /
    {   }
    { * }
       /
  2   ︿
    /   }
   { * /
     ﹀
  3  ︿
   { * }
     ﹀

  4  ︿
   / * }
   {   }
      /

  5  \
   { * }
   {  /

  6
   {* \
   {  }
    ﹀
  7  ︿
   {   }
   { * }
     ﹀
  8  ︿
   /   }
   \ * }
     ﹀
  9   \
    {   }
    { * }
     \

 10  ︿
   {   \
   { * /
     ﹀

*/

/*
  /  26 (lo)
  /  27 (hi)
  \  28 (lo)
  \  29 (hi)
  {  32
  }  33
  ﹀ 30
  ︿ 31
*/

   QList<QPoint> n = mapPosNeighbors(p);

   /*
    *         2
    *      3     1
    *         p
    *      4     0
    *         5
    */

   if (type == 1) {
        Change_Map_part(p.x(), p.y(), 47);
        Change_Map_part(n[2].x(), n[2].y(), 27);
        Change_Map_part(n[3].x(), n[3].y(), 32);
        Change_Map_part(n[4].x(), n[4].y(), 32);
        Change_Map_part(n[1].x(), n[1].y(), 33);
        Change_Map_part(n[0].x(), n[0].y(), 33);
        Change_Map_part(n[5].x(), n[5].y(), 26);
   }
   if (type == 2) {
       Change_Map_part(p.x(), p.y(), 47);
       Change_Map_part(n[3].x(), n[3].y(), 27);
       Change_Map_part(n[2].x(), n[2].y(), 31);
       Change_Map_part(n[1].x(), n[1].y(), 33);
       Change_Map_part(n[0].x(), n[0].y(), 26);
       Change_Map_part(n[5].x(), n[5].y(), 30);
       Change_Map_part(n[4].x(), n[4].y(), 32);
   }
   if (type == 3) {
       Change_Map_part(p.x(), p.y(), 30);
       Change_Map_part(n[2].x(), n[2].y(), 31);
       Change_Map_part(n[3].x(), n[3].y(), 32);
       Change_Map_part(n[1].x(), n[1].y(), 33);
   }
   if (type == 4) {
       Change_Map_part(p.x(), p.y(), 25);
       Change_Map_part(n[3].x(), n[3].y(), 27);
       Change_Map_part(n[2].x(), n[2].y(), 31);
       Change_Map_part(n[1].x(), n[1].y(), 33);
       Change_Map_part(n[0].x(), n[0].y(), 33);
       Change_Map_part(n[4].x(), n[4].y(), 32);
       Change_Map_part(n[5].x(), n[5].y(), 26);
   }
   if (type == 5) {
       Change_Map_part(p.x(), p.y(), 26);
       Change_Map_part(n[1].x(), n[1].y(), 33);
       Change_Map_part(n[2].x(), n[2].y(), 29);
       Change_Map_part(n[3].x(), n[3].y(), 32);
       Change_Map_part(n[4].x(), n[4].y(), 32);
   }
   if (type == 6) {
       Change_Map_part(p.x(), p.y(), 29);
       Change_Map_part(n[3].x(), n[3].y(), 32);
       Change_Map_part(n[4].x(), n[4].y(), 32);
       Change_Map_part(n[5].x(), n[5].y(), 30);
       Change_Map_part(n[0].x(), n[0].y(), 33);
   }
   if (type == 7) {
       Change_Map_part(p.x(), p.y(), 47);
       Change_Map_part(n[3].x(), n[3].y(), 32);
       Change_Map_part(n[4].x(), n[4].y(), 32);
       Change_Map_part(n[2].x(), n[2].y(), 31);
       Change_Map_part(n[1].x(), n[1].y(), 33);
       Change_Map_part(n[0].x(), n[0].y(), 33);
       Change_Map_part(n[5].x(), n[5].y(), 30);
   }
   if (type == 8) {
       Change_Map_part(p.x(), p.y(), 47);
       Change_Map_part(n[3].x(), n[3].y(), 27);
       Change_Map_part(n[4].x(), n[4].y(), 28);
       Change_Map_part(n[2].x(), n[2].y(), 31);
       Change_Map_part(n[1].x(), n[1].y(), 33);
       Change_Map_part(n[0].x(), n[0].y(), 33);
       Change_Map_part(n[5].x(), n[5].y(), 30);
   }
   if (type == 9) {
        Change_Map_part(p.x(), p.y(), 47);
        Change_Map_part(n[2].x(), n[2].y(), 29);
        Change_Map_part(n[3].x(), n[3].y(), 32);
        Change_Map_part(n[4].x(), n[4].y(), 32);
        Change_Map_part(n[1].x(), n[1].y(), 33);
        Change_Map_part(n[0].x(), n[0].y(), 33);
        Change_Map_part(n[5].x(), n[5].y(), 28);
   }
   if (type == 10) {
       Change_Map_part(p.x(), p.y(), 47);
       Change_Map_part(n[3].x(), n[3].y(), 32);
       Change_Map_part(n[4].x(), n[4].y(), 32);
       Change_Map_part(n[2].x(), n[2].y(), 31);
       Change_Map_part(n[1].x(), n[1].y(), 29);
       Change_Map_part(n[0].x(), n[0].y(), 26);
       Change_Map_part(n[5].x(), n[5].y(), 30);
   }

}
