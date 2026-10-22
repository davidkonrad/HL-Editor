/*
 * HL Editor
 *
 * autopath.h by David Konrad
 *
 * Provides data structures and functions for handling autogeneration of roads, rivers and so on
 *
 */


int AUTOGEN_PATH_TYPE = 2;
int AUTOGEN_STRAIGHT_DIST = 3;

bool AUTOGEN_DIRECTION_EAST = false;
bool AUTOGEN_DIRECTION_WEST = false;
bool AUTOGEN_DIRECTION_NORTH = false;
bool AUTOGEN_DIRECTION_SOUTH = false;

bool AUTOGEN_MOVE_UP = false;
bool AUTOGEN_MOVE_DOWN = false;
bool AUTOGEN_MOVE_LEFT = false;
bool AUTOGEN_MOVE_RIGHT = false;

struct Autogen_Tiles {
    int DIAG_RIGHT;  /* \  */
    int DIAG_LEFT;   /* /  */
    int VERTICAL;    /* |  */
    int ARC_1;       /* _| */
    int ARC_2;       /* ‾| */
    int ARC_3;       /* |‾ */
    int ARC_4;       /* |_ */
    int HORZ_UP;     /* /\ */
    int HORZ_DOWN;   /* \/ */
};

int direction_to_tile(Autogen_Tiles tiles, int dir, int prior, bool debug) {
    if (dir == -1) dir = 5;//return tiles.ARC_3;

    QString sp;
    if (prior == tiles.DIAG_RIGHT) sp = "DIAG_RIGHT";
    if (prior == tiles.DIAG_LEFT) sp = "DIAG_LEFT";
    if (prior == tiles.VERTICAL) sp = "VERTICAL";
    if (prior == tiles.ARC_1) sp = "ARC_1";
    if (prior == tiles.ARC_2) sp = "ARC_2";
    if (prior == tiles.ARC_3) sp = "ARC_3";
    if (prior == tiles.ARC_4) sp = "ARC_4";
    if (prior == tiles.HORZ_UP) sp = "HORZ_UP";
    if (prior == tiles.HORZ_DOWN) sp = "HORZ_DOWN";


    QString d1;
    if (AUTOGEN_DIRECTION_EAST) d1 = d1 + "east ";
    if (AUTOGEN_DIRECTION_WEST) d1 = d1 + "west ";
    if (AUTOGEN_DIRECTION_NORTH) d1 = d1 + "north ";
    if (AUTOGEN_DIRECTION_SOUTH) d1 = d1 + "south ";

    QString d2;
    if (AUTOGEN_MOVE_UP) d2 = d2 + "UP ";
    if (AUTOGEN_MOVE_DOWN) d2 = d2 + "DOWN ";
    if (AUTOGEN_MOVE_LEFT) d2 = d2 + "LEFT ";
    if (AUTOGEN_MOVE_RIGHT) d2 = d2 + "RIGHT ";

    if (debug) qDebug() << "direction_to_tile" << dir << sp << d1 << d2;

    if (dir == 0) {
        if (prior == tiles.ARC_2) {
            return tiles.ARC_4;
        }
        if (prior == tiles.ARC_3) {
            if (AUTOGEN_MOVE_DOWN) return tiles.ARC_4;
            return tiles.HORZ_DOWN;
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
         if (prior == tiles.DIAG_RIGHT) {
            if (AUTOGEN_DIRECTION_WEST && AUTOGEN_DIRECTION_NORTH && AUTOGEN_MOVE_RIGHT) return tiles.DIAG_RIGHT;
            if (AUTOGEN_DIRECTION_WEST && AUTOGEN_DIRECTION_SOUTH && AUTOGEN_MOVE_RIGHT) return tiles.DIAG_RIGHT;
            if (AUTOGEN_MOVE_DOWN && AUTOGEN_MOVE_RIGHT) return tiles.DIAG_RIGHT;
            if (AUTOGEN_DIRECTION_EAST) return tiles.DIAG_RIGHT;
            return tiles.ARC_2;
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
        //
        if (prior == tiles.ARC_4) {
            if (AUTOGEN_DIRECTION_EAST && AUTOGEN_DIRECTION_NORTH && AUTOGEN_MOVE_UP) return tiles.ARC_3;
            if (AUTOGEN_DIRECTION_EAST && AUTOGEN_DIRECTION_NORTH) return tiles.HORZ_UP; //HORZ_DOWN;
            return tiles.ARC_3;
        }
        return tiles.HORZ_UP; //VERTICAL
    }

    if (dir == 2) {
        if (prior == tiles.ARC_1) {
            return tiles.VERTICAL;
        }
        if (prior == tiles.ARC_2) {
            if (AUTOGEN_DIRECTION_WEST && AUTOGEN_DIRECTION_SOUTH && AUTOGEN_MOVE_LEFT) return tiles.ARC_4;

            if (AUTOGEN_MOVE_LEFT && AUTOGEN_MOVE_UP) return tiles.ARC_4;
            if (AUTOGEN_DIRECTION_NORTH && AUTOGEN_MOVE_LEFT) return tiles.ARC_4;
            if (AUTOGEN_MOVE_LEFT) return tiles.HORZ_UP;
            return tiles.ARC_4;
        }
        if (prior == tiles.ARC_4) {
            return tiles.VERTICAL;
        }
        if (prior == tiles.VERTICAL) {
            return tiles.VERTICAL;
        }
        if (prior == tiles.DIAG_RIGHT) {
            return tiles.ARC_4; //DIAG_RIGHT;
        }
        if (prior == tiles.HORZ_UP) {
            if (AUTOGEN_DIRECTION_WEST && AUTOGEN_DIRECTION_NORTH && AUTOGEN_MOVE_RIGHT) return tiles.ARC_1;
            if (AUTOGEN_DIRECTION_WEST && AUTOGEN_MOVE_LEFT) return tiles.ARC_4;
            if (AUTOGEN_DIRECTION_NORTH && AUTOGEN_DIRECTION_WEST) return tiles.ARC_4;
            if (AUTOGEN_MOVE_UP && AUTOGEN_MOVE_LEFT) return tiles.ARC_4;
            if (AUTOGEN_DIRECTION_NORTH && AUTOGEN_DIRECTION_EAST) return tiles.ARC_1;
        }
        if (prior == tiles.HORZ_DOWN) {
            if (AUTOGEN_DIRECTION_NORTH && AUTOGEN_DIRECTION_WEST) return tiles.ARC_4;
            if (AUTOGEN_DIRECTION_NORTH && AUTOGEN_DIRECTION_EAST) return tiles.ARC_1;
        }
    }

    if (dir == 3) {
        if (prior == tiles.HORZ_UP) {
            return tiles.DIAG_RIGHT;
        }
        if (prior == tiles.ARC_1) {
            if (AUTOGEN_DIRECTION_NORTH && AUTOGEN_DIRECTION_WEST && AUTOGEN_MOVE_UP) return tiles.ARC_2;
            if (AUTOGEN_DIRECTION_NORTH && AUTOGEN_DIRECTION_EAST && AUTOGEN_MOVE_UP) return tiles.ARC_2;

            if (AUTOGEN_DIRECTION_NORTH && AUTOGEN_DIRECTION_WEST && AUTOGEN_MOVE_LEFT && AUTOGEN_MOVE_DOWN) return tiles.HORZ_UP;
            if (AUTOGEN_DIRECTION_NORTH && AUTOGEN_DIRECTION_WEST && AUTOGEN_MOVE_LEFT) return tiles.HORZ_UP;

            if (AUTOGEN_DIRECTION_NORTH && AUTOGEN_DIRECTION_EAST) return tiles.HORZ_UP;
            if (AUTOGEN_DIRECTION_NORTH && AUTOGEN_DIRECTION_WEST && AUTOGEN_MOVE_LEFT) return tiles.ARC_2;

            if (AUTOGEN_DIRECTION_NORTH && AUTOGEN_MOVE_UP) return tiles.ARC_2;

            if (AUTOGEN_DIRECTION_WEST && AUTOGEN_MOVE_UP) return tiles.ARC_2;

            if (AUTOGEN_DIRECTION_WEST) return tiles.HORZ_UP;
            return tiles.HORZ_DOWN;
        }
        if (prior == tiles.ARC_2) {
            return tiles.DIAG_RIGHT;
        }
        if (prior == tiles.ARC_4) {
            return tiles.ARC_2;
        }
        if (prior == tiles.DIAG_RIGHT) {
            return tiles.DIAG_RIGHT;
        }
        if (prior == tiles.VERTICAL) {
            return tiles.ARC_2;
        }
        if (prior == tiles.HORZ_DOWN) {
            return tiles.HORZ_UP;
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
        if (prior == tiles.ARC_2) {
            if (AUTOGEN_DIRECTION_WEST && AUTOGEN_DIRECTION_SOUTH && AUTOGEN_MOVE_LEFT) return tiles.HORZ_DOWN;
            if (AUTOGEN_DIRECTION_WEST && AUTOGEN_DIRECTION_SOUTH) return tiles.ARC_1;
            if (AUTOGEN_DIRECTION_WEST && AUTOGEN_DIRECTION_NORTH && AUTOGEN_MOVE_DOWN) return tiles.ARC_1;
            return tiles.HORZ_DOWN; //UP?
        }

        return tiles.DIAG_LEFT;
    }

    if (dir == 5) {
        if (prior == tiles.DIAG_RIGHT) {
            if (AUTOGEN_DIRECTION_EAST && AUTOGEN_DIRECTION_NORTH && AUTOGEN_MOVE_RIGHT) return tiles.ARC_2;

            if (AUTOGEN_DIRECTION_WEST && AUTOGEN_DIRECTION_SOUTH) return tiles.ARC_2;
            if (AUTOGEN_MOVE_DOWN && AUTOGEN_MOVE_RIGHT) return tiles.ARC_2;
            return tiles.DIAG_RIGHT;
        }
        if (prior == tiles.ARC_3) {
            return tiles.VERTICAL; //ARC_1
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
            return tiles.VERTICAL;
        }
        //
        if (prior == tiles.HORZ_UP) {
            if (AUTOGEN_DIRECTION_EAST) {
                return tiles.ARC_2;
            }
        }
        if (prior == tiles.HORZ_DOWN) {
            if (AUTOGEN_MOVE_LEFT) return tiles.ARC_3;
            return tiles.ARC_2;
        }

        return tiles.ARC_3;
    }

    return tiles.ARC_1;
}

const Autogen_Tiles Road_Tiles = {
    129,   /* \  */
    128,   /* /  */
    127,   /* |  */
    133,   /* _| */
    132,   /* ‾| */
    130,   /* |‾ */
    131,   /* |_ */
    134,   /* /\ */
    135    /* \/ */
};

const Autogen_Tiles Dirtroad_Tiles = {
    148,   /* \  */
    147,   /* /  */
    146,   /* |  */
    152,   /* _| */
    151,   /* ‾| */
    149,   /* |‾ */
    150,   /* |_ */
    153,   /* /\ */
    154    /* \/ */
};

const Autogen_Tiles Railroad_Tiles = {
    86,   /* \  */
    87,   /* /  */
    85,   /* |  */
    88,   /* _| */
    89,   /* ‾| */
    90,   /* |‾ */
    91,   /* |_ */
    92,   /* /\ */
    93    /* \/ */
};

const Autogen_Tiles River_Tiles = {
    36,   /* \  */
    37,   /* /  */
    38,   /* |  */
    39,   /* _| */
    40,   /* ‾| */
    41,   /* |‾ */
    42,   /* |_ */
    44,   /* /\ */
    43    /* \/ */
};

const Autogen_Tiles Trench_Tiles = {
    171,  /* \  */
    170,  /* /  */
    167,  /* |  */
    169,  /* _| */
    166,  /* ‾| */
    167,  /* |‾ */
    168,  /* |_ */
    173,  /* /\ */
    172   /* \/ */
};

const Autogen_Tiles CoastTopRight_Tiles = {
    28,   /* \  */
    26,   /* /  */
    33,   /* |  */
    26,   /* _| */
    33,   /* ‾| */
    26,   /* |‾ */
    33,   /* |_ */ //25
    -1,   /* /\ */
    30    /* \/ */
};

const Autogen_Tiles CoastLeftBottom_Tiles = {
    29,   /* \  */
    27,   /* /  */
    32,   /* |  */
    27,   /* _| */
    29,   /* ‾| */
    27,   /* |‾ */
    29,   /* |_ */
    31,   /* /\ */
    -1    /* \/ */
};

//see if there is an existing nearby tile we can connect the road, river etc to
bool tile_in_Autogen_Tiles(int tile, Autogen_Tiles tiles) {
    if (tile == tiles.DIAG_RIGHT) return true;
    if (tile == tiles.DIAG_LEFT) return true;
    if (tile == tiles.VERTICAL) return true;
    if (tile == tiles.ARC_1) return true;
    if (tile == tiles.ARC_2) return true;
    if (tile == tiles.ARC_3) return true;
    if (tile == tiles.ARC_4) return true;
    if (tile == tiles.HORZ_DOWN) return true;
    if (tile == tiles.HORZ_UP) return true;
    return false;
}

//some 'manual' checks, see if we can place a bridge instead and so on, the list will grow
int process_special_tiles(int part, int old_tile, int target) {
    if (target == 1) { //dirtroad
        if (part == Dirtroad_Tiles.VERTICAL) {
            if (old_tile == River_Tiles.DIAG_RIGHT) {
                return 122; //vertical bridge
            }
            if (old_tile == River_Tiles.DIAG_LEFT) {
                return 121; //vertical bridge
            }
            if (old_tile == Railroad_Tiles.DIAG_LEFT) {
                return 99; //railroad crossing
            }
        }
        if (part == Dirtroad_Tiles.HORZ_UP || part == Dirtroad_Tiles.HORZ_DOWN) {
            if (old_tile == River_Tiles.VERTICAL) {
                return 59; //horizontal bridge
            }
        }
        if (part == Dirtroad_Tiles.DIAG_RIGHT) {
            if (old_tile == Dirtroad_Tiles.DIAG_LEFT) {
                return 163; //dirtroad X-road
            }
            if (old_tile == Road_Tiles.DIAG_LEFT) {
                return 144; //road X-road
            }
            if (old_tile == River_Tiles.DIAG_LEFT) {
                return 58; //railroad crossing
            }
        }
        if (part == Dirtroad_Tiles.DIAG_LEFT) {
            if (old_tile == Dirtroad_Tiles.DIAG_RIGHT) {
                return 163; //dirtroad X-road
            }
            if (old_tile == Road_Tiles.DIAG_RIGHT) {
                return 144; //road X-road
            }
        }
    }

    if (target == 2) { //road
        if (part == Road_Tiles.DIAG_RIGHT) {
            if (old_tile == Dirtroad_Tiles.DIAG_LEFT) {
                return 163; //dirtroad X-road
            }
            if (old_tile == Road_Tiles.DIAG_LEFT) {
                return 144; //road X-road
            }
            if (old_tile == Railroad_Tiles.VERTICAL) {
                return 101; //railroad crossing
            }
            if (old_tile == River_Tiles.DIAG_LEFT) {
                return 58; //railroad crossing
            }
            if (old_tile == River_Tiles.VERTICAL) {
                return 118; //heavy bridge
            }
        }
        if (part == Road_Tiles.DIAG_LEFT) {
            if (old_tile == Dirtroad_Tiles.DIAG_RIGHT) {
                return 163; //dirtroad X-road
            }
            if (old_tile == Road_Tiles.DIAG_RIGHT) {
                return 144; //road X-road
            }
            if (old_tile == Railroad_Tiles.VERTICAL) {
                return 104; //railroad crossing
            }
        }
        if (part == Road_Tiles.VERTICAL) {
            if (old_tile == Railroad_Tiles.DIAG_RIGHT) {
                return 102; //railroad crossing
            }
            if (old_tile == Railroad_Tiles.DIAG_LEFT) {
                return 103; //railroad crossing
            }
            if (old_tile == River_Tiles.DIAG_RIGHT) {
                return 116; //heavy bridge
            }
            if (old_tile == River_Tiles.DIAG_LEFT) {
                return 117; //heavy bridge
            }
        }
        if (part == Road_Tiles.HORZ_DOWN) {
            if (old_tile == River_Tiles.VERTICAL) {
                return 59; //bridge
            }
        }
        if (part == Road_Tiles.HORZ_UP) {
            if (old_tile == River_Tiles.VERTICAL) {
                return 59; //bridge
            }
        }
    }

    if (target == 3) { //railroad
        if (part == Railroad_Tiles.DIAG_LEFT) {
            if (old_tile == Dirtroad_Tiles.VERTICAL) {
                return 99; //ralroad crossing
            }
            if (old_tile == Road_Tiles.VERTICAL) {
                return 103; //ralroad crossing
            }
        }
        if (part == Railroad_Tiles.VERTICAL) {
            if (old_tile == Dirtroad_Tiles.DIAG_LEFT) {
                return 100; //railroad crossing
            }
            if (old_tile == Dirtroad_Tiles.DIAG_RIGHT) {
                return 101; //railroad crossing
            }
            if (old_tile == Road_Tiles.DIAG_RIGHT) {
                return 101; //railroad crossing
            }
            if (old_tile == Road_Tiles.DIAG_LEFT) {
                return 104; //railroad crossing
            }
            if (old_tile == River_Tiles.DIAG_RIGHT) {
                return 97; //railroad bridge
            }
            if (old_tile == River_Tiles.DIAG_LEFT) {
                return 98; //railroad bridge
            }
        }
        if (part == Railroad_Tiles.DIAG_RIGHT) {
            if (old_tile == Road_Tiles.VERTICAL) {
                return 102; //ralroad crossing
            }
            if (old_tile == Dirtroad_Tiles.VERTICAL) {
                return 102; //ralroad crossing
            }
        }
    }

    if (target == 4) { //river
        if (part == River_Tiles.DIAG_LEFT) {
            if (old_tile == Road_Tiles.VERTICAL) {
                return 117; //vertical bridge
            }
            if ((old_tile == Road_Tiles.DIAG_RIGHT) ||
                (old_tile == Dirtroad_Tiles.DIAG_RIGHT)) {
                return 58; //diag wood brigde
            }
        }
        if (part == River_Tiles.DIAG_RIGHT) {
            if (old_tile == Road_Tiles.VERTICAL) {
                return 116; //vertical bridge
            }
        }
        if (part == River_Tiles.VERTICAL) {
            if (old_tile == Road_Tiles.DIAG_RIGHT) {
                return 118; //diag right bridge
            }
        }
        if (part == River_Tiles.DIAG_RIGHT) {
            if (old_tile == Railroad_Tiles.VERTICAL) {
                return 97; //railroad bridge
            }
        }
        if (part == River_Tiles.DIAG_LEFT) {
            if (old_tile == Railroad_Tiles.VERTICAL) {
                return 98; //railroad bridge
            }
        }
    }
    return part;
}


/* Look for a starting point for the path
 *   1. Nearby tile of the same type, or
 *   2. Building entrance
 */
int get_adjacent_tile(QPoint from, Autogen_Tiles tiles) {
    int field_pos;
    int tile;
    for (const QPoint& p : hexNeighbors(from)) {
        field_pos = (p.y() * Map.width) + p.x();
        tile = Map.data[field_pos*2];
        if (tile_in_Autogen_Tiles(tile, tiles)) {
            return tile;
            break;
        }
    }
    for (const QPoint& p : hexNeighbors(from)) {
        field_pos = (p.y() * Map.width) + p.x();
        tile = Map.data[field_pos*2];
        if (tile_is_factory_entrance(tile)) {
            return tiles.DIAG_LEFT;
            break;
        }
        if (tile_is_hq_entrance(tile) || tile_is_depot_entrance(tile) || tile_is_under_construction_depot_entrance(tile)) {
            return tiles.VERTICAL;
            break;
        }
    }
    return 0;
}

QList<PathPoint> getHexPath(QPoint start, QPoint end) {
    switch (AUTOGEN_PATH_TYPE) {
        case 1 :
            return hexPath(start, end); break;
        case 2 :
            return hexPathStraight(start, end); break;
        case 3 :
            return  hexPathRandomDetour(start, end, 0.50); break; //double detourChance = 0.25
                    //hexPathPreferDirection(start, end); break;
                    //hexPathGentleWave(start, end); break;
                    //hexPathOrganic(start, end); break;
                    //pathEngine(start, end); break;

        case 4 :
            return hexPathPreferDirection(start, end); break;
        case 5 :
            return hexPathScenic(start, end, scenicCallback, 9); break;
        case 6 :
            return hexPathRandomDetour(start, end, 0.50); break; //double detourChance = 0.25

        default: break; //should never happen
   }
}

void autogenerate_path(QPoint from, QPoint to, int target, bool draw_only) {
    int prior;
    int prior_dir;
    int field_pos;
    int part;
    int connect_from;
    bool water_is_protected = no_cross_terrain.Water;
    Autogen_Tiles tiles;
    QPoint start;
    QPoint end;
    QPoint prior_pos;

    int f = from.x() * from.y();
    int t = to.x() * to.y();

    if (f > t) {
        start = from;
        end = to;
    } else {
        start = to;
        end = from;
    }

    switch (target) {
        case 1: tiles = Dirtroad_Tiles; break;
        case 2: tiles = Road_Tiles; break;
        case 3: tiles = Railroad_Tiles; break;
        case 4: tiles = River_Tiles; break;
        case 5: tiles = Trench_Tiles; break;
        case 6: tiles = CoastTopRight_Tiles; break;
        case 7: tiles = CoastLeftBottom_Tiles; break;
        default: return; break;
    }

    //coast line tiles should not fear water
    if (target == 6 || target == 7) {
        no_cross_terrain.Water = false;
    }

    if (!draw_only) qDebug() << "autogen" << start << end;

    AUTOGEN_DIRECTION_EAST = end.x() > start.x();
    AUTOGEN_DIRECTION_WEST = end.x() < start.x();
    AUTOGEN_DIRECTION_NORTH = end.y() < start.y();
    AUTOGEN_DIRECTION_SOUTH = end.y() > start.y();

    connect_from = get_adjacent_tile(start, tiles);
    prior = connect_from;
    //qDebug() << "connect_from" << connect_from;

    QList<PathPoint> path = getHexPath(start, end);

    for (const PathPoint& p : path) {
        if ((p.pos.x() > -1 && p.pos.x() < Map.width-1) && (p.pos.y() > -1 && p.pos.y() < Map.height-1)) {
            field_pos = (p.pos.y() * Map.width) + p.pos.x();

            AUTOGEN_MOVE_DOWN = prior_pos.y() < p.pos.y();
            AUTOGEN_MOVE_UP = prior_pos.y() > p.pos.y();
            AUTOGEN_MOVE_LEFT = prior_pos.x() > p.pos.x();
            AUTOGEN_MOVE_RIGHT = prior_pos.x() < p.pos.x();
            prior_pos = p.pos;

            if (p.dir > -1) {
                part = direction_to_tile(tiles, p.dir, prior, !draw_only);
                prior_dir = p.dir;
            } else {
                part = direction_to_tile(tiles, prior_dir, prior, !draw_only);
            }

            if (part != -1) { //'transparent'
                int old_tile = Map.data[field_pos*2];

                int alt_part = process_special_tiles(part, old_tile, target);

                if (!draw_only) {
                    if (!tile_is_building(old_tile)) {
                        Map.data[field_pos*2] = alt_part;
                    }
                }

//                if (!tile_is_building(old_tile)) {
                    Redraw_Field(p.pos.x(), p.pos.y(), alt_part, Map.data[(field_pos*2)+1]);
//                }
            }

            prior = part;
        }
        if (!draw_only) qDebug() << "---------------------------------";
    }

    if (target == 6 || target == 7) {
        no_cross_terrain.Water = water_is_protected;
    }

}
