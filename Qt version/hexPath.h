/*
 * HL Editor
 *
 * hexPath.h by David Konrad
 *
 * Structures and functions for generating paths on the map, e.g rivers, roads and so on
 *
 */

#include <queue>

struct NoCross_Terrain {
    bool Building = true;
    bool Mountain = true;
    bool Water = true;
    bool Hill = true;
    bool Reef = true;
    bool Bunker = true;
    bool Trench = false;
    bool Forest = true;
    bool River = false;
    bool Residentals = false;
    bool Dirtroad = false;
    bool Road = false;
    bool Railroad = false;
};

NoCross_Terrain  no_cross_terrain;

bool is_legal_tile(QPoint p) {
    int field_pos = point_to_fieldpos(p);
    int old_tile = Map.data[field_pos*2];

    if (no_cross_terrain.Building) {
        if (tile_is_building(old_tile)) {
            return false;
        }
    }
    if (no_cross_terrain.Mountain) {
        if (tile_is_mountain(old_tile)) {
            return false;
        }
    }
    if (no_cross_terrain.Water) {
        if (tile_is_water(old_tile)) {
            return false;
        }
    }
    if (no_cross_terrain.Hill) {
       if (tile_is_hill(old_tile)) {
            return false;
        }
    }
    if (no_cross_terrain.Reef) {
       if (tile_is_reef(old_tile)) {
            return false;
        }
    }
    if (no_cross_terrain.Bunker) {
        int unit = Map.data[(field_pos*2)+1];
        if (unit/2 == 13) {
            return false;
        }
    }
    if (no_cross_terrain.Residentals) {
       if (tile_is_residental(old_tile)) {
            return false;
        }
    }
    if (no_cross_terrain.River) {
       if (tile_is_river(old_tile)) {
            return false;
        }
    }
    if (no_cross_terrain.Forest) {
       if (tile_is_forest(old_tile)) {
            return false;
        }
    }
    if (no_cross_terrain.Trench) {
       if (tile_is_trench(old_tile)) {
            return false;
        }
    }

    if (no_cross_terrain.Dirtroad) {
       if (tile_is_dirtroad(old_tile)) {
            return false;
        }
    }
    if (no_cross_terrain.Road) {
       if (tile_is_road(old_tile)) {
            return false;
        }
    }
    if (no_cross_terrain.Railroad) {
       if (tile_is_railroad(old_tile)) {
            return false;
        }
    }
    return true;
}


//--------------------------------------------------------------------------
struct Cube {
    int x = 0, y = 0, z = 0;
    Cube() = default;
    Cube(int x, int y, int z) : x(x), y(y), z(z) {}
};

struct PathPoint {
    QPoint pos;
    int dir = -1;
};

Cube toCube(QPoint p) {
    int col = p.x(), row = p.y();
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
int hexDistance(QPoint a, QPoint b) { return hexDistance(toCube(a), toCube(b)); }

Cube cubeRound(double x, double y, double z) {
    int rx = std::round(x), ry = std::round(y), rz = std::round(z);
    double xd = std::abs(rx - x), yd = std::abs(ry - y), zd = std::abs(rz - z);
    if (xd > yd && xd > zd) rx = -ry - rz;
    else if (yd > zd)       ry = -rx - rz;
    else                    rz = -rx - ry;
    return Cube(rx, ry, rz);
}

static const QPoint kEvenDirs[6] = {
    {+1,0},{+1,-1},{0,-1},{-1,-1},{-1,0},{0,+1}
};

static const QPoint kOddDirs[6] = {
    {+1,+1},{+1,0},{0,-1},{-1,0},{-1,+1},{0,+1}
};

QList<QPoint> hexNeighbors(QPoint p) {
    const QPoint* d = (p.x() & 1) ? kOddDirs : kEvenDirs;
    QList<QPoint> r;
    for (int i = 0; i < 6; ++i) r.append(p + d[i]);
    return r;
}

int directionTo(QPoint from, QPoint to) {
    const QPoint* d = (from.x() & 1) ? kOddDirs : kEvenDirs;
    QPoint delta = to - from;
    for (int i = 0; i < 6; ++i) if (d[i] == delta) return i;
    return -1;
}

inline uint qHash(const QPoint& p, uint seed = 0) {
    return qHash(qMakePair(p.x(), p.y()), seed);
}

QList<QPoint> astar(QPoint start, QPoint goal, int maxNodes = 10000) {
    if (start == goal) return is_legal_tile(start) ? QList<QPoint>{start} : QList<QPoint>{};
    if (!is_legal_tile(start) || !is_legal_tile(goal)) return {};

    // Bounds-tjek
    auto insideMap = [](QPoint p) {
        return p.x() >= 0 && p.x() < Map.width &&
               p.y() >= 0 && p.y() < Map.height;
    };
    if (!insideMap(start) || !insideMap(goal))
        return {};

    struct Node {
        QPoint pos;
        int g, f;
        bool operator>(const Node& o) const { return f > o.f; }
    };
    std::priority_queue<Node, std::vector<Node>, std::greater<Node>> open;
    QHash<QPoint, int> gScore;
    QHash<QPoint, QPoint> cameFrom;
    QSet<QPoint> closed;

    gScore[start] = 0;
    open.push({start, 0, hexDistance(start, goal)});

    int expanded = 0;

    while (!open.empty()) {
        if (++expanded > maxNodes)
            return {};

        Node cur = open.top();
        open.pop();
        if (closed.contains(cur.pos)) continue;
        closed.insert(cur.pos);

        if (cur.pos == goal) {
            QList<QPoint> path;
            for (QPoint p = goal; p != start; p = cameFrom[p])
                path.prepend(p);
            path.prepend(start);
            return path;
        }

        for (const QPoint& n : hexNeighbors(cur.pos)) {
            if (!insideMap(n) || !is_legal_tile(n) || closed.contains(n))
                continue;
            int tg = gScore[cur.pos] + 1;
            if (!gScore.contains(n) || tg < gScore[n]) {
                cameFrom[n] = cur.pos;
                gScore[n] = tg;
                open.push({n, tg, tg + hexDistance(n, goal)});
            }
        }
    }
    return {};
}

QList<PathPoint> toPathPoints(const QList<QPoint>& pts) {
    QList<PathPoint> out;
    for (int i = 0; i < pts.size(); ++i) {
        int dir = (i + 1 < pts.size()) ? directionTo(pts[i], pts[i+1]) : -1;
        out.append({pts[i], dir});
    }
    return out;
}

// Afvigelse fra den lige linje i cube-rum (billig og god nok)
int deviation(QPoint p, Cube startC, Cube goalC) {
    Cube c = toCube(p);
    // vektor start→goal og start→p
    int dx = goalC.x - startC.x;
    int dy = goalC.y - startC.y;
    int dz = goalC.z - startC.z;
    int px = c.x - startC.x;
    int py = c.y - startC.y;
    int pz = c.z - startC.z;
    // krydsprodukt-magnitude som mål for afstand til linjen
    int cross = std::abs(dy*pz - dz*py) + std::abs(dz*px - dx*pz) + std::abs(dx*py - dy*px);
    int len = std::abs(dx) + std::abs(dy) + std::abs(dz);
    return len ? cross / len : 0;
}

QList<QPoint> astarStraight(QPoint start, QPoint goal, int maxNodes = 10000) {
    if (start == goal) return is_legal_tile(start) ? QList<QPoint>{start} : QList<QPoint>{};
    if (!is_legal_tile(start) || !is_legal_tile(goal)) return {};

    // Bounds-tjek
    auto insideMap = [](QPoint p) {
        return p.x() >= 0 && p.x() < Map.width &&
               p.y() >= 0 && p.y() < Map.height;
    };
    if (!insideMap(start) || !insideMap(goal))
        return {};

    Cube startC = toCube(start);
    Cube goalC  = toCube(goal);

    struct Node {
        QPoint pos;
        int g, f;
        bool operator>(const Node& o) const { return f > o.f; }
    };
    std::priority_queue<Node, std::vector<Node>, std::greater<Node>> open;
    QHash<QPoint, int> gScore;
    QHash<QPoint, QPoint> cameFrom;
    QSet<QPoint> closed;

    gScore[start] = 0;
    open.push({start, 0, hexDistance(start, goal)});

    int expanded = 0;

    while (!open.empty()) {
        if (++expanded > maxNodes)
            return {};

        Node cur = open.top();
        open.pop();
        if (closed.contains(cur.pos)) continue;
        closed.insert(cur.pos);

        if (cur.pos == goal) {
            QList<QPoint> path;
            for (QPoint p = goal; p != start; p = cameFrom[p])
                path.prepend(p);
            path.prepend(start);
            return path;
        }

        for (const QPoint& n : hexNeighbors(cur.pos)) {
            if (!insideMap(n) || !is_legal_tile(n) || closed.contains(n))
                continue;
            int tentative = gScore[cur.pos] + 1;
            if (!gScore.contains(n) || tentative < gScore[n]) {
                cameFrom[n] = cur.pos;
                gScore[n] = tentative;
                int h = hexDistance(n, goal) + deviation(n, startC, goalC);
                open.push({n, tentative, tentative + h});
            }
        }
    }
    return {};
}


QList<PathPoint> hexPathStraight(QPoint start, QPoint goal) {
    return toPathPoints(astarStraight(start, goal));
}

// hexPath forbliver den frie version (uden deviation-straf)
QList<PathPoint> hexPath(QPoint start, QPoint goal) {
    return toPathPoints(astar(start, goal));  // den oprindelige astar
}

/*
2. hexPathPreferDirection
Foretrækker at fortsætte i samme retning så længe som muligt (færre knæk).
Giver mere “motorvejs-agtige” eller jernbane-lignende forløb.

3. hexPathScenic
Prøver at gå via felter med høj “interessanthed” (du kan give den en simpel scoring-funktion, fx tæt på vand, bjerge, byer osv.).
Stien bliver lidt længere, men mere spændende.

4. hexPathRandomDetour
Som den lige version, men med en lille chance for at tage en tilfældig omvej undervejs — selv når der ikke er forhindringer.
Giver mere organisk og uforudsigeligt resultat hver gang.
*/


// ========== 2. PreferDirection – færre knæk (motorvej/jernbane) ==========
QList<PathPoint> hexPathPreferDirection(QPoint start, QPoint goal) {
    if (!is_legal_tile(start) || !is_legal_tile(goal)) return {};

    struct Node {
        QPoint pos;
        int dir;          // retning vi kom fra (-1 = start)
        int g, f;
        bool operator>(const Node& o) const { return f > o.f; }
    };

    std::priority_queue<Node, std::vector<Node>, std::greater<Node>> open;
    // key = pos + dir vi ankom med
    QHash<QPair<QPoint,int>, int> gScore;
    QHash<QPair<QPoint,int>, QPair<QPoint,int>> cameFrom;
    QSet<QPair<QPoint,int>> closed;

    auto key = [](QPoint p, int d) { return qMakePair(p, d); };

    gScore[key(start, -1)] = 0;
    open.push({start, -1, 0, hexDistance(start, goal)});

    QPair<QPoint,int> goalKey;
    bool found = false;

    while (!open.empty()) {
        Node cur = open.top(); open.pop();
        auto ck = key(cur.pos, cur.dir);
        if (closed.contains(ck)) continue;
        closed.insert(ck);

        if (cur.pos == goal) {
            goalKey = ck;
            found = true;
            break;
        }

        auto neigh = hexNeighbors(cur.pos);
        for (int i = 0; i < neigh.size(); ++i) {
            QPoint n = neigh[i];
            if (!is_legal_tile(n)) continue;

            // billigere at fortsætte ligeud
            int stepCost = (i == cur.dir) ? 1 : 2;
            int tg = gScore[ck] + stepCost;
            auto nk = key(n, i);
            if (closed.contains(nk)) continue;
            if (!gScore.contains(nk) || tg < gScore[nk]) {
                cameFrom[nk] = ck;
                gScore[nk] = tg;
                open.push({n, i, tg, tg + hexDistance(n, goal)});
            }
        }
    }

    if (!found) return {};

    QList<QPoint> pts;
    auto cur = goalKey;
    while (cur.first != start || cur.second != -1) {
        pts.prepend(cur.first);
        cur = cameFrom[cur];
    }
    pts.prepend(start);
    return toPathPoints(pts);
}

// ========== 3. Scenic – foretrækker “interessante” felter ==========
// Du giver en funktion der returnerer højere tal for mere interessante hexer
// (fx tæt på flod, bjerg, by …). Højere = mere tillokkende.
using ScenicFunc = std::function<int(QPoint)>;

//Lav scenicWeight (1–2) → næsten lige vej, kun små afstikkere.
//Høj scenicWeight (5–10) → den tager gerne længere omveje forbi bygninger.
QList<PathPoint> hexPathScenic(QPoint start, QPoint goal, ScenicFunc scenic, int scenicWeight = 3) {
    if (!is_legal_tile(start) || !is_legal_tile(goal)) return {};

    struct Node {
        QPoint pos;
        int g, f;
        bool operator>(const Node& o) const { return f > o.f; }
    };
    std::priority_queue<Node, std::vector<Node>, std::greater<Node>> open;
    QHash<QPoint, int> gScore;
    QHash<QPoint, QPoint> cameFrom;
    QSet<QPoint> closed;

    gScore[start] = 0;
    open.push({start, 0, hexDistance(start, goal)});

    while (!open.empty()) {
        Node cur = open.top(); open.pop();
        if (closed.contains(cur.pos)) continue;
        closed.insert(cur.pos);
        if (cur.pos == goal) {
            QList<QPoint> pts;
            for (QPoint p = goal; p != start; p = cameFrom[p]) pts.prepend(p);
            pts.prepend(start);
            return toPathPoints(pts);
        }
        for (const QPoint& n : hexNeighbors(cur.pos)) {
            if (!is_legal_tile(n) || closed.contains(n)) continue;
            // lavere pris på interessante felter
            int step = std::max(1, 4 - scenic(n) / scenicWeight);
            int tg = gScore[cur.pos] + step;
            if (!gScore.contains(n) || tg < gScore[n]) {
                cameFrom[n] = cur.pos;
                gScore[n] = tg;
                open.push({n, tg, tg + hexDistance(n, goal)});
            }
        }
    }
    return {};
}

// ========== 4. RandomDetour – lige vej med tilfældige små omveje ==========
QList<PathPoint> hexPathRandomDetour(QPoint start, QPoint goal, double detourChance = 0.25) {
    auto base = hexPathStraight(start, goal);   // eller hexPathWavy
    if (base.size() < 3) return base;

    QList<QPoint> pts;
    for (const auto& pp : base) pts.append(pp.pos);

    auto* rng = QRandomGenerator::global();
    for (int i = 1; i < pts.size() - 1; ++i) {
        if (rng->generateDouble() > detourChance) continue;
        // prøv at skubbe punktet til en tilfældig nabo
        auto neigh = hexNeighbors(pts[i]);
        // behold kun lovlige som stadig er nabo til både forrige og næste
        QList<QPoint> candidates;
        for (const QPoint& n : neigh) {
            if (!is_legal_tile(n)) continue;
            if (hexNeighbors(pts[i-1]).contains(n) && hexNeighbors(n).contains(pts[i+1]))
                candidates.append(n);
        }
        if (!candidates.isEmpty())
            pts[i] = candidates[rng->bounded(candidates.size())];
    }
    return toPathPoints(pts);
}


/*
Path Engine – parametre
=========================================================================================================
Konstant           | Effekt                                              | Forslag       | Bemærkning
-------------------|-----------------------------------------------------|---------------|----------------------------------
straightness       | Hvor tæt stien holder sig på den lige linje         | 0.0 – 2.5     | 0 = fri, 1 = moderat, 2+ = meget lige
waviness           | Bløde, regelmæssige buer                            | 0.0 – 3.0     | 0 = ingen, 1.5–2.5 = pæne bølger
randomness         | Tilfældig uro / organisk udtryk                     | 0.0 – 1.5     | 0 = helt fast, 0.4–0.8 = levende
turnPenalty        | Straf for skarpe retningsskift                      | 0.2 – 1.2     | Højere = færre knæk
continueBonus      | Belønning for at fortsætte ligeud                   | 0.0 – 1.0     | Giver længere lige stræk
momentum           | Ekstra kraft til at holde den aktuelle retning      | 0.0 – 0.8     | Forstærker continueBonus
clearance          | Holder afstand til ulovlige felter                  | 0.0 – 1.5     | >0 = går lidt udenom forhindringer
horizontalBias     | Foretrækker vandrette/diagonale bevægelser          | 0.0 – 1.0     | 0 = neutral
verticalBias       | Foretrækker lodrette bevægelser                     | 0.0 – 1.0     | 0 = neutral
centerAvoidance    | Undgår at skære lige gennem midten af området       | 0.0 – 0.8     | Giver mere “omkring”-stier
maxNodes           | Maks. antal felter A* må undersøge                  | 5000 – 15000  | Sikkerhedsloft mod hæng
=========================================================================================================
Alle værdier kan stå på 0 (på nær maxNodes) for ren standard-A*.
*/
QList<PathPoint> pathEngine(QPoint start, QPoint goal) {
    // ===================== PATH ENGINE =====================
    // Grundlæggende
    const double straightness     = 1.0;   // 0 = fri, 2+ = tæt på linjen
    const double waviness         = 0.0;   // 0 = ingen, 1–3 = bløde buer
    const double randomness       = 0.0;   // 0 = fast, 0.3–1.5 = organisk

    // Retning og flow
    const double turnPenalty      = 0.5;   // højere = færre skarpe knæk
    const double continueBonus    = 0.4;   // højere = længere lige stræk
    const double momentum         = 0.3;   // ekstra belønning for at holde retning

    // Stil
    const double clearance        = 0.0;   // >0 = hold afstand til ulovlige felter
    const double horizontalBias   = 0.0;   // >0 = foretræk vandrette/diagonale bevægelser
    const double verticalBias     = 0.0;   // >0 = foretræk lodrette bevægelser
    const double centerAvoidance  = 0.0;   // >0 = undgå midten af åbne områder

    // Sikkerhed
    const int    maxNodes         = 10000;
    // =======================================================

    if (!is_legal_tile(start) || !is_legal_tile(goal)) return {};
    if (start == goal) return {{start, -1}};

    Cube startC = toCube(start);
    Cube goalC  = toCube(goal);
    auto* rng = QRandomGenerator::global();

    struct Node {
        QPoint pos;
        int g;
        double f;
        bool operator>(const Node& o) const { return f > o.f; }
    };

    std::priority_queue<Node, std::vector<Node>, std::greater<Node>> open;
    QHash<QPoint, int> gScore;
    QHash<QPoint, QPoint> cameFrom;
    QSet<QPoint> closed;

    gScore[start] = 0;
    open.push({start, 0, double(hexDistance(start, goal))});

    auto dirIndex = [](QPoint from, QPoint to) -> int {
        const QPoint* dirs = (from.x() & 1) ? kOddDirs : kEvenDirs;
        QPoint d = to - from;
        for (int i = 0; i < 6; ++i)
            if (dirs[i] == d) return i;
        return -1;
    };

    // simpel clearance: antal ulovlige naboer
    auto illegalNeighbors = [](QPoint p) {
        int c = 0;
        for (const QPoint& n : hexNeighbors(p))
            if (!is_legal_tile(n)) ++c;
        return c;
    };

    int expanded = 0;
    while (!open.empty() && expanded < maxNodes) {
        Node cur = open.top(); open.pop();
        if (closed.contains(cur.pos)) continue;
        closed.insert(cur.pos);
        ++expanded;

        if (cur.pos == goal) {
            QList<QPoint> pts;
            for (QPoint p = goal; p != start; p = cameFrom[p])
                pts.prepend(p);
            pts.prepend(start);
            return toPathPoints(pts);
        }

        int prevDir = -1;
        if (cameFrom.contains(cur.pos))
            prevDir = dirIndex(cameFrom[cur.pos], cur.pos);

        for (const QPoint& nb : hexNeighbors(cur.pos)) {
            if (!is_legal_tile(nb) || closed.contains(nb)) continue;

            int newDir = dirIndex(cur.pos, nb);
            double stepCost = 1.0;

            // turn / continue / momentum
            if (prevDir >= 0 && newDir >= 0) {
                int diff = std::abs(newDir - prevDir);
                if (diff > 3) diff = 6 - diff;
                stepCost += diff * turnPenalty;
                if (diff == 0) {
                    stepCost -= continueBonus;
                    stepCost -= momentum;
                }
            }

            // directional bias
            if (newDir == 0 || newDir == 3) stepCost -= horizontalBias;
            if (newDir == 2 || newDir == 5) stepCost -= verticalBias;

            // clearance
            if (clearance > 0.01)
                stepCost += illegalNeighbors(nb) * clearance;

            stepCost = std::max(0.2, stepCost);

            int tg = gScore[cur.pos] + int(stepCost * 10); // skaleret til int
            if (!gScore.contains(nb) || tg < gScore[nb]) {
                cameFrom[nb] = cur.pos;
                gScore[nb] = tg;

                double h = hexDistance(nb, goal);

                if (straightness > 0.01)
                    h += deviation(nb, startC, goalC) * straightness;

                if (waviness > 0.01) {
                    double t = 1.0 - double(hexDistance(nb, goal)) /
                               std::max(1, hexDistance(start, goal));
                    h += std::abs(std::sin(t * 6.283185 * 1.7)) * waviness;
                }

                if (centerAvoidance > 0.01) {
                    // simpel: straf for at være langt fra både start og mål (midten)
                    double midDist = (hexDistance(nb, start) + hexDistance(nb, goal)) * 0.5;
                    h += midDist * centerAvoidance * 0.1;
                }

                if (randomness > 0.01)
                    h += (rng->generateDouble() - 0.5) * randomness * 2.0;

                open.push({nb, tg, tg * 0.1 + h});
            }
        }
    }
    return {};
}


//---------------------------------------------------------------------------

int scenicAttractToBuildings(QPoint p) {
    int score = 1;   // basis
    const int dist = 4;

    for (int x = p.x() - dist; x <= p.x() + dist; ++x) {
        for (int y = p.y() - dist; y <= p.y() + dist; ++y) {
            if (x < 0 || y < 0 || x >= Map.width || y >= Map.height)
                continue;

            int field_pos = y * Map.width + x;
            int old_tile = Map.data[field_pos * 2];

            if (tile_is_building_entrance(old_tile)) {
                int vx = dist - std::abs(x - p.x());
                int vy = dist - std::abs(y - p.y());
                score += (vx * vy) * 30;   // jo tættere, jo højere score
            }

            if (tile_is_building(old_tile)) {
                int vx = dist - std::abs(x - p.x());
                int vy = dist - std::abs(y - p.y());
                score += (vx * vy) * 15;   // jo tættere, jo højere score
            }
        }
    }
    return score;
}

