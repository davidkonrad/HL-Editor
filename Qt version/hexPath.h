/*
 * HL Editor
 *
 * hexPath.h by David Konrad
 *
 * Provides functions and data structures for handling autogeneration of roads, rivers and so on
 * ------------------------------------------------------------------------------------------------
 * Some parts of this code is generated in collaboration with Grok AI.
 * I think it is clearly where, but have marked it with comments also.
 *
 * I simply could not figure out how to make the shortest path from A to B in this virtual hexagon
 * setup. My engine would not work, or worked only with straight lines and a few steps aside.
 *
 * So I asked Grok AI, and it immediately understood the task and produced working code.
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
    bool Forest = false;
    bool River = false;
    bool Residentals = false;
    bool Dirtroad = false;
    bool Road = false;
    bool Railroad = false;
};

int AUTOGEN_PATH_TYPE = 2;
int AUTOGEN_STRAIGHT_DIST = 3;

NoCross_Terrain  no_cross_terrain;

bool is_legal_tile(QPoint p) {
    int field_pos = (p.y() * Map.width) + p.x();
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

bool is_legal_tile(QPoint p); // din funktion

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

// ---------- fælles A* (bruges af begge) ----------
QList<QPoint> astar(QPoint start, QPoint goal, int maxNodes = 10000) {
    if (start == goal) return is_legal_tile(start) ? QList<QPoint>{start} : QList<QPoint>{};
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

    int expanded = 0;

    while (!open.empty()) {
        if (++expanded > maxNodes)
            return {};               // giver op

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
            if (!is_legal_tile(n) || closed.contains(n)) continue;
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
            if (!is_legal_tile(n) || closed.contains(n)) continue;
            int tentative = gScore[cur.pos] + 1;
            if (!gScore.contains(n) || tentative < gScore[n]) {
                cameFrom[n] = cur.pos;
                gScore[n] = tentative;
                int h = hexDistance(n, goal) + deviation(n, startC, goalC);  // eller 2 * deviation(...)
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
1. hexPathWavy
Følger den lige linje, men tilføjer en blød sinus-bølge på tværs, så vejen/floden bugter sig jævnt.
God til floder eller naturlige stier.

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

// ========== 1. Wavy – blød sinus-bølge på tværs af linjen ==========
QList<PathPoint> hexPathWavy(QPoint start, QPoint goal, double amplitude = 2.5, double frequency = 0.35) {
    if (!is_legal_tile(start) || !is_legal_tile(goal)) return {};

    Cube a = toCube(start), b = toCube(goal);
    int n = hexDistance(a, b);
    if (n == 0) return {{start, -1}};

    // retning og en vinkelret vektor i cube-rum
    double dx = b.x - a.x, dy = b.y - a.y, dz = b.z - a.z;
    double len = std::sqrt(dx*dx + dy*dy + dz*dz);
    if (len < 1e-6) return {{start, -1}};
    // simpel vinkelret (ikke perfekt, men fin til bølge)
    double px = -dy, py = dx, pz = 0;
    double plen = std::sqrt(px*px + py*py + pz*pz);
    if (plen > 1e-6) { px /= plen; py /= plen; pz /= plen; }

    QList<QPoint> ideal;
    for (int i = 0; i <= n; ++i) {
        double t = double(i) / n;
        double wave = amplitude * std::sin(t * n * frequency * 2 * M_PI);
        double x = a.x + dx * t + px * wave;
        double y = a.y + dy * t + py * wave;
        double z = a.z + dz * t + pz * wave;
        ideal.append(toOffset(cubeRound(x, y, z)));
    }

    // følg ideal med korte A*-omveje omkring ulovlige felter (samme idé som før)
    QList<QPoint> path{start};
    for (int i = 1; i < ideal.size(); ++i) {
        if (is_legal_tile(ideal[i]) && hexNeighbors(path.last()).contains(ideal[i])) {
            path.append(ideal[i]);
        } else {
            auto det = astar(path.last(), ideal[i]);
            if (det.isEmpty()) return {};
            for (int k = 1; k < det.size(); ++k) path.append(det[k]);
        }
    }
    return toPathPoints(path);
}

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

//Her er en mere kontrolleret variant, hexPathGentleWave, der holder sig til gyldige nabo-skridt og kun laver bløde, sammenhængende buer.
//Den er bedre egnet til roads/rivers
QList<PathPoint> hexPathGentleWave(QPoint start, QPoint goal,
                                   double amplitude = 1.8,
                                   double waveLength = 12.0) {
    if (!is_legal_tile(start) || !is_legal_tile(goal)) return {};

    Cube a = toCube(start), b = toCube(goal);
    int n = hexDistance(a, b);
    if (n == 0) return {{start, -1}};

    // basis-retning
    double dx = b.x - a.x, dy = b.y - a.y, dz = b.z - a.z;
    double len = std::sqrt(dx*dx + dy*dy + dz*dz);
    if (len < 1e-6) return {{start, -1}};

    // en grov vinkelret retning
    double px = -dy, py = dx, pz = 0.0;
    double plen = std::sqrt(px*px + py*py);
    if (plen > 1e-6) { px /= plen; py /= plen; }

    QList<QPoint> path;
    path.append(start);
    QPoint cur = start;

    for (int i = 1; i <= n; ++i) {
        double t = double(i) / n;
        double wave = amplitude * std::sin(t * n / waveLength * 2.0 * M_PI);

        // ønsket cube-position
        double wx = a.x + dx * t + px * wave;
        double wy = a.y + dy * t + py * wave;
        double wz = a.z + dz * t + pz * wave;
        QPoint target = toOffset(cubeRound(wx, wy, wz));

        // tag ét skridt mod target, men kun via lovlig nabo
        QPoint best = cur;
        int bestDist = hexDistance(cur, target);
        for (const QPoint& nb : hexNeighbors(cur)) {
            if (!is_legal_tile(nb)) continue;
            int d = hexDistance(nb, target);
            if (d < bestDist) {
                bestDist = d;
                best = nb;
            }
        }
        if (best == cur) break;          // kan ikke komme videre
        cur = best;
        path.append(cur);
        if (cur == goal) break;
    }

    // sørg for at vi ender på goal hvis muligt
    if (path.last() != goal) {
        auto tail = astar(path.last(), goal, 2000);
        if (!tail.isEmpty()) {
            for (int k = 1; k < tail.size(); ++k)
                path.append(tail[k]);
        }
    }

    return toPathPoints(path);
}

//--------------------------------
QList<PathPoint> hexPathGentleWaveSmooth(QPoint start, QPoint goal) {
    // --- justerbare konstanter ---
    const double amplitude     = 2.2;   // hvor høje buerne er
    const double waveLength    = 9.0;   // grund-bølgelængde
    const double diagonalBias  = 0.65;  // ekstra bølge på diagonale/lodrette stræk
    const double noise         = 0.35;  // lille tilfældig variation
    // -----------------------------

    if (!is_legal_tile(start) || !is_legal_tile(goal)) return {};

    Cube a = toCube(start), b = toCube(goal);
    int n = hexDistance(a, b);
    if (n == 0) return {{start, -1}};

    double dx = b.x - a.x, dy = b.y - a.y, dz = b.z - a.z;
    double len = std::sqrt(dx*dx + dy*dy + dz*dz);
    if (len < 1e-6) return {{start, -1}};

    double px = -dy, py = dx, pz = 0.0;
    double plen = std::sqrt(px*px + py*py);
    if (plen > 1e-6) { px /= plen; py /= plen; }

    // er stien mest diagonal eller lodret?
    bool mostlyDiagOrVert = (std::abs(dx) < len * 0.7 && std::abs(dy) < len * 0.7) ||
                            (std::abs(dz) > len * 0.6);

    QList<QPoint> path;
    path.append(start);
    QPoint cur = start;
    auto* rng = QRandomGenerator::global();

    for (int i = 1; i <= n; ++i) {
        double t = double(i) / n;
        double amp = amplitude;
        if (mostlyDiagOrVert) amp += diagonalBias;

        double wave = amp * std::sin(t * n / waveLength * 2.0 * M_PI);
        wave += (rng->generateDouble() - 0.5) * noise * 2.0;

        double wx = a.x + dx * t + px * wave;
        double wy = a.y + dy * t + py * wave;
        double wz = a.z + dz * t;
        QPoint target = toOffset(cubeRound(wx, wy, wz));

        QPoint best = cur;
        int bestDist = hexDistance(cur, target);
        for (const QPoint& nb : hexNeighbors(cur)) {
            if (!is_legal_tile(nb)) continue;
            int d = hexDistance(nb, target);
            if (d < bestDist) {
                bestDist = d;
                best = nb;
            }
        }
        if (best == cur) break;
        cur = best;
        path.append(cur);
        if (cur == goal) break;
    }

    if (path.last() != goal) {
        auto tail = astar(path.last(), goal, 2000);
        if (!tail.isEmpty())
            for (int k = 1; k < tail.size(); ++k)
                path.append(tail[k]);
    }

    return toPathPoints(path);
}

QList<PathPoint> hexPathMeander(QPoint start, QPoint goal) {
    // --- justerbare konstanter ---
    const double amplitude   = 3.2;
    const double minSegment  = 5.0;
    const double maxSegment  = 13.0;
    const double noise       = 0.25;
    const int    turnCost    = 2;     // straf pr. "trin" man drejer
    // -----------------------------

    if (!is_legal_tile(start) || !is_legal_tile(goal)) return {};

    Cube a = toCube(start), b = toCube(goal);
    int n = hexDistance(a, b);
    if (n == 0) return {{start, -1}};

    double dx = b.x - a.x, dy = b.y - a.y, dz = b.z - a.z;
    double len = std::sqrt(dx*dx + dy*dy + dz*dz);
    if (len < 1e-6) return {{start, -1}};

    double px = -dy, py = dx;
    double plen = std::sqrt(px*px + py*py);
    if (plen > 1e-6) { px /= plen; py /= plen; }

    QList<QPoint> path;
    path.append(start);
    QPoint cur = start;
    auto* rng = QRandomGenerator::global();

    double side = (rng->generateDouble() < 0.5) ? 1.0 : -1.0;
    int nextSwitch = int(minSegment + rng->generateDouble() * (maxSegment - minSegment));

    // hjælper: find retningsindeks 0-5
    auto dirIndex = [](QPoint from, QPoint to) -> int {
        QPoint d = to - from;
        const QPoint* dirs = (from.x() & 1) ? kOddDirs : kEvenDirs;
        for (int i = 0; i < 6; ++i)
            if (dirs[i] == d) return i;
        return -1;
    };

    for (int step = 1; step <= n * 2; ++step) {
        if (step >= nextSwitch) {
            side = -side;
            nextSwitch = step + int(minSegment + rng->generateDouble() * (maxSegment - minSegment));
        }

        double t = std::min(1.0, double(path.size()) / double(n));
        double wave = side * amplitude + (rng->generateDouble() - 0.5) * noise * 2.0;
        double fade = std::sin(t * M_PI);
        wave *= 0.5 + 0.5 * fade;

        double wx = a.x + dx * t + px * wave;
        double wy = a.y + dy * t + py * wave;
        double wz = a.z + dz * t;
        QPoint target = toOffset(cubeRound(wx, wy, wz));

        QPoint best = cur;
        int bestScore = INT_MAX;

        int prevDir = -1;
        if (path.size() >= 2)
            prevDir = dirIndex(path[path.size()-2], cur);

        for (const QPoint& nb : hexNeighbors(cur)) {
            if (!is_legal_tile(nb)) continue;
            if (path.size() >= 2 && nb == path[path.size()-2]) continue;

            int d = hexDistance(nb, target);
            int newDir = dirIndex(cur, nb);

            int turnPenalty = 0;
            if (prevDir >= 0 && newDir >= 0) {
                int diff = std::abs(newDir - prevDir);
                if (diff > 3) diff = 6 - diff;          // korteste vej rundt
                turnPenalty = diff * turnCost;          // 0 = ligeud, 1 = 60°, 2 = 120° …
            }

            int score = d * 3 + turnPenalty;
            if (score < bestScore) {
                bestScore = score;
                best = nb;
            }
        }

        if (best == cur) break;
        cur = best;
        path.append(cur);
        if (cur == goal) break;
    }

    if (path.last() != goal) {
        auto tail = astar(path.last(), goal, 2000);
        if (!tail.isEmpty())
            for (int k = 1; k < tail.size(); ++k)
                path.append(tail[k]);
    }

    return toPathPoints(path);
}

//------------------------------------------------------------------
QList<PathPoint> hexPathStepped(QPoint start, QPoint goal) {
    // --- justerbare konstanter ---
    const int minStraight = 6;    // mindste lige stræk
    const int maxStraight = 14;   // længste lige stræk
    const int maxTurn     = 2;    // maks. retningsskift pr. knæk (1 = 60°, 2 = 120°)
    // -----------------------------

    if (!is_legal_tile(start) || !is_legal_tile(goal)) return {};

    Cube a = toCube(start), b = toCube(goal);
    int n = hexDistance(a, b);
    if (n == 0) return {{start, -1}};

    QList<QPoint> path;
    path.append(start);
    QPoint cur = start;
    auto* rng = QRandomGenerator::global();

    auto dirIndex = [](QPoint from, QPoint to) -> int {
        QPoint d = to - from;
        const QPoint* dirs = (from.x() & 1) ? kOddDirs : kEvenDirs;
        for (int i = 0; i < 6; ++i)
            if (dirs[i] == d) return i;
        return -1;
    };

    int currentDir = -1;
    int straightLeft = 0;

    while (cur != goal && path.size() < n * 3) {
        if (straightLeft <= 0) {
            // vælg ny retning tættere på målet
            int bestDir = -1;
            int bestDist = INT_MAX;
            auto neigh = hexNeighbors(cur);
            for (int i = 0; i < 6; ++i) {
                QPoint nb = neigh[i];
                if (!is_legal_tile(nb)) continue;
                int d = hexDistance(nb, goal);
                int turn = 0;
                if (currentDir >= 0) {
                    int diff = std::abs(i - currentDir);
                    if (diff > 3) diff = 6 - diff;
                    if (diff > maxTurn) continue;
                    turn = diff;
                }
                if (d + turn < bestDist) {
                    bestDist = d + turn;
                    bestDir = i;
                }
            }
            if (bestDir < 0) break;
            currentDir = bestDir;
            straightLeft = minStraight + rng->bounded(maxStraight - minStraight + 1);
        }

        // gå ét skridt i currentDir
        auto neigh = hexNeighbors(cur);
        QPoint next = cur + ((cur.x() & 1) ? kOddDirs[currentDir] : kEvenDirs[currentDir]);
        if (!is_legal_tile(next)) {
            straightLeft = 0;   // tving ny retning
            continue;
        }
        cur = next;
        path.append(cur);
        --straightLeft;
    }

    if (path.last() != goal) {
        auto tail = astar(path.last(), goal, 3000);
        if (!tail.isEmpty())
            for (int k = 1; k < tail.size(); ++k)
                path.append(tail[k]);
    }

    return toPathPoints(path);
}
//-----------------------------------------------------------------------------------
QList<PathPoint> hexPathOrganic(QPoint start, QPoint goal) {
    // --- justerbare konstanter ---
    const double amplitude  = 2.4;   // grund-bølge
    const double drift      = 0.8;   // tilfældig sideværts drift
    const double keepDir    = 1.6;   // hvor stærkt den foretrækker at fortsætte lige
    const double waveLength = 10.0;
    // -----------------------------

    if (!is_legal_tile(start) || !is_legal_tile(goal)) return {};

    Cube a = toCube(start), b = toCube(goal);
    int n = hexDistance(a, b);
    if (n == 0) return {{start, -1}};

    double dx = b.x - a.x, dy = b.y - a.y, dz = b.z - a.z;
    double len = std::sqrt(dx*dx + dy*dy + dz*dz);
    if (len < 1e-6) return {{start, -1}};

    double px = -dy, py = dx;
    double plen = std::sqrt(px*px + py*py);
    if (plen > 1e-6) { px /= plen; py /= plen; }

    QList<QPoint> path;
    path.append(start);
    QPoint cur = start;
    auto* rng = QRandomGenerator::global();

    auto dirIndex = [](QPoint from, QPoint to) -> int {
        QPoint d = to - from;
        const QPoint* dirs = (from.x() & 1) ? kOddDirs : kEvenDirs;
        for (int i = 0; i < 6; ++i)
            if (dirs[i] == d) return i;
        return -1;
    };

    for (int step = 1; step <= n * 2 && cur != goal; ++step) {
        double t = std::min(1.0, double(path.size()) / double(n));
        double wave = amplitude * std::sin(t * n / waveLength * 2.0 * M_PI);
        wave += (rng->generateDouble() - 0.5) * drift * 2.0;

        double wx = a.x + dx * t + px * wave;
        double wy = a.y + dy * t + py * wave;
        double wz = a.z + dz * t;
        QPoint target = toOffset(cubeRound(wx, wy, wz));

        int prevDir = (path.size() >= 2) ? dirIndex(path[path.size()-2], cur) : -1;

        QPoint best = cur;
        double bestScore = 1e9;

        for (const QPoint& nb : hexNeighbors(cur)) {
            if (!is_legal_tile(nb)) continue;
            if (path.size() >= 2 && nb == path[path.size()-2]) continue;

            int d = hexDistance(nb, target);
            int newDir = dirIndex(cur, nb);

            double turnPen = 0.0;
            if (prevDir >= 0 && newDir >= 0) {
                int diff = std::abs(newDir - prevDir);
                if (diff > 3) diff = 6 - diff;
                turnPen = diff * keepDir;
            }

            double score = d + turnPen;
            if (score < bestScore) {
                bestScore = score;
                best = nb;
            }
        }

        if (best == cur) break;
        cur = best;
        path.append(cur);
    }

    if (path.last() != goal) {
        auto tail = astar(path.last(), goal, 2500);
        if (!tail.isEmpty())
            for (int k = 1; k < tail.size(); ++k)
                path.append(tail[k]);
    }

    return toPathPoints(path);
}
//-------------------------------------------------------------------------------------
QList<PathPoint> hexPathZigzag(QPoint start, QPoint goal) {
    // --- justerbare konstanter ---
    const double amplitude = 2.2;
    const int    period    = 7;
    const int    minTurnLen = 3;   // mindst 3 tiles til et sving
    // -----------------------------

    if (!is_legal_tile(start) || !is_legal_tile(goal)) return {};

    Cube ca = toCube(start), cb = toCube(goal);
    int n = hexDistance(ca, cb);
    if (n == 0) return {{start, -1}};

    int pdx = -(cb.y - ca.y);
    int pdy =  (cb.x - ca.x);
    int plen = std::max(1, (std::abs(pdx) + std::abs(pdy)) / 2);
    pdx /= plen; pdy /= plen;

    QList<QPoint> path;
    path.append(start);
    QPoint cur = start;
    int stepsInCurrentDir = 0;
    int lockedDir = -1;

    auto dirIndex = [](QPoint from, QPoint to) -> int {
        const QPoint* dirs = (from.x() & 1) ? kOddDirs : kEvenDirs;
        QPoint d = to - from;
        for (int i = 0; i < 6; ++i)
            if (dirs[i] == d) return i;
        return -1;
    };

    for (int i = 1; i <= n * 2 && cur != goal; ++i) {
        double t = std::min(1.0, double(path.size()) / double(n));
        int side = ((i / period) % 2 == 0) ? 1 : -1;

        double wx = ca.x + (cb.x - ca.x) * t + pdx * side * amplitude;
        double wy = ca.y + (cb.y - ca.y) * t + pdy * side * amplitude;
        double wz = ca.z + (cb.z - ca.z) * t;
        QPoint target = toOffset(cubeRound(wx, wy, wz));

        int prevDir = (path.size() >= 2) ? dirIndex(path[path.size()-2], cur) : -1;

        // hvis vi er midt i et sving, så fortsæt i lockedDir indtil minTurnLen
        QPoint best = cur;
        if (lockedDir >= 0 && stepsInCurrentDir < minTurnLen) {
            auto neigh = hexNeighbors(cur);
            QPoint forced = cur + ((cur.x() & 1) ? kOddDirs[lockedDir] : kEvenDirs[lockedDir]);
            if (is_legal_tile(forced))
                best = forced;
            else
                lockedDir = -1;
        }

        if (best == cur) {
            int bestScore = INT_MAX;
            for (const QPoint& nb : hexNeighbors(cur)) {
                if (!is_legal_tile(nb)) continue;
                if (path.size() >= 2 && nb == path[path.size()-2]) continue;

                int d = hexDistance(nb, target);
                int newDir = dirIndex(cur, nb);
                int penalty = 0;
                if (prevDir >= 0 && newDir >= 0 && newDir != prevDir)
                    penalty = 4;   // blødt skift foretrækkes

                int score = d * 2 + penalty;
                if (score < bestScore) {
                    bestScore = score;
                    best = nb;
                    lockedDir = newDir;
                    stepsInCurrentDir = 0;
                }
            }
        }

        if (best == cur) break;
        cur = best;
        path.append(cur);
        ++stepsInCurrentDir;
        if (stepsInCurrentDir >= minTurnLen)
            lockedDir = -1;
    }

    if (path.last() != goal) {
        auto tail = astar(path.last(), goal, 2000);
        if (!tail.isEmpty())
            for (int k = 1; k < tail.size(); ++k)
                path.append(tail[k]);
    }

    return toPathPoints(path);
}
//--------------------------------------------------------------------------

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


QList<PathPoint> hexPathLongest(QPoint start, QPoint goal) {
    const int maxNodes = 15000;   // sikkerhedsloft
    const int lengthBonus = 3;    // hvor meget vi belønner længde

    if (!is_legal_tile(start) || !is_legal_tile(goal)) return {};
    if (start == goal) return {{start, -1}};

    struct Node {
        QPoint pos;
        int g;          // skridt indtil nu (vi vil maksimere)
        int f;          // g * bonus - afstand til mål
        bool operator<(const Node& o) const { return f < o.f; } // max-heap
    };

    std::priority_queue<Node> open;
    QHash<QPoint, int> bestG;     // længste kendte vej til punktet
    QHash<QPoint, QPoint> cameFrom;
    QSet<QPoint> closed;

    bestG[start] = 0;
    open.push({start, 0, 0});

    int expanded = 0;
    bool found = false;

    while (!open.empty() && expanded < maxNodes) {
        Node cur = open.top(); open.pop();
        if (closed.contains(cur.pos)) continue;
        closed.insert(cur.pos);
        ++expanded;

        if (cur.pos == goal) {
            found = true;
            break;
        }

        for (const QPoint& nb : hexNeighbors(cur.pos)) {
            if (!is_legal_tile(nb)) continue;

            int newG = cur.g + 1;
            if (!bestG.contains(nb) || newG > bestG[nb]) {
                bestG[nb] = newG;
                cameFrom[nb] = cur.pos;
                int f = newG * lengthBonus - hexDistance(nb, goal);
                open.push({nb, newG, f});
            }
        }
    }

    if (!found) return {};

    QList<QPoint> pts;
    for (QPoint p = goal; p != start; p = cameFrom.value(p))
        pts.prepend(p);
    pts.prepend(start);
    return toPathPoints(pts);
}

QList<PathPoint> hexPathBigArc(QPoint start, QPoint goal) {
    const double arcStrength = 2.2;   // lavere værdi til 64x48

    if (!is_legal_tile(start) || !is_legal_tile(goal)) return {};
    if (start == goal) return {{start, -1}};

    Cube a = toCube(start);
    Cube b = toCube(goal);
    int n = hexDistance(a, b);
    if (n < 4) return toPathPoints(astar(start, goal)); // for kort til bue

    double dx = b.x - a.x;
    double dy = b.y - a.y;
    double px = -dy, py = dx;
    double plen = std::sqrt(px*px + py*py);
    if (plen < 1e-6) return toPathPoints(astar(start, goal));
    px /= plen; py /= plen;

    int side = (goal.x() >= start.x()) ? 1 : -1;

    QList<QPoint> path{start};
    QPoint cur = start;

    for (int i = 1; i <= n; ++i) {
        double t = double(i) / n;
        double offset = side * arcStrength * 4.0 * t * (1.0 - t);

        double x = a.x + dx * t + px * offset;
        double y = a.y + dy * t + py * offset;
        double z = a.z + (b.z - a.z) * t;
        QPoint target = toOffset(cubeRound(x, y, z));

        // ét skridt ad gangen
        QPoint best = cur;
        int bestD = hexDistance(cur, target);
        for (const QPoint& nb : hexNeighbors(cur)) {
            if (!is_legal_tile(nb)) continue;
            int d = hexDistance(nb, target);
            if (d < bestD) { bestD = d; best = nb; }
        }
        if (best == cur) break;
        cur = best;
        path.append(cur);
        if (cur == goal) break;
    }

    if (path.last() != goal) {
        auto tail = astar(path.last(), goal, 2000);
        if (!tail.isEmpty())
            for (int k = 1; k < tail.size(); ++k)
                path.append(tail[k]);
    }
    return toPathPoints(path);
}

//---------------------------------------------------------------------------
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

    if (debug) qDebug() << "direction_to_tile" << dir << prior;

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
            return tiles.ARC_4; //DIAG_RIGHT;
        }
        if (prior == tiles.HORZ_UP) {
            return tiles.ARC_4; //ARC_1
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
        //new
/*
        if (prior == tiles.HORZ_DOWN) {
            return tiles.ARC_1;
        }
*/
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
        //new
/*
        if (prior == tiles.ARC_1) {
            return tiles.ARC_3;
        }
*/
        return tiles.DIAG_LEFT;
    }

    if (dir == 5) {
        if (prior == tiles.DIAG_RIGHT) {
            return tiles.DIAG_RIGHT;
        }
        if (prior == tiles.ARC_3) {
            return tiles.ARC_1;
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
    171,   /* \  */
    170,   /* /  */
    167,   /* |  */  //63
    169,   /* _| */
    166,   /* ‾| */  //168
    167,   /* |‾ */
    168,   /* |_ */  //166
    173,   /* /\ */
    172    /* \/ */
};

const Autogen_Tiles CoastTopRight_Tiles = {
    28,   /* \  */
    26,   /* /  */
    33,   /* |  */
    26,   /* _| */
    30,   /* ‾| */
    26,   /* |‾ */
    33,   /* |_ */ //25
    25,   /* /\ */
    30    /* \/ */
};

const Autogen_Tiles CoastLeftBottom_Tiles = {
    29,   /* \  */
    27,   /* /  */
    32,   /* |  */
    27,   /* _| */
    25,   /* ‾| */ //29

    31,   /* |‾ */
    29,   /* |_ */

    31,   /* /\ */
    29    /* \/ */ //25S
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

QList<QPoint> hexNeighborsList(QPoint p) {
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
    result.reserve(6);
    for (int i = 0; i < 6; ++i)
        result.append(p + dirs[i]);
    return result;
}

//some 'manual' checks, see if we can place a bridge instead and so on, the list will grow
//HUSK IKKE RETTET IFT HORZ_UP etc
int process_special_tiles(int part, int old_tile, int target) {
    if (target == 1) { //dirtroad
        if (part == Dirtroad_Tiles.VERTICAL) {
            if (old_tile == River_Tiles.DIAG_RIGHT) {
                return 122; //vertical bridge
            }
        }
        if (part == Dirtroad_Tiles.VERTICAL) {
            if (old_tile == River_Tiles.DIAG_LEFT) {
                return 121; //vertical bridge
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


//------------------------------------------------------------------------
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
    return 0;
}


int scenicCallback(QPoint p) {
    int score = 1;               // basis
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

QList<PathPoint> getHexPath(QPoint start, QPoint end) {
    switch (AUTOGEN_PATH_TYPE) {
        case 1 :
            return hexPath(start, end); break;
        case 2 :
            return hexPathStraight(start, end); break;
        case 3 :
            return hexPathBigArc(start, end); break;
                    //hexPathLongest(start, end); break;
                    //pathEngine(start, end); break;
                    //hexPathZigzag(start, end); break;
                    //hexPathOrganic(start, end); break;
                    //hexPathStepped(start, end); break;
                    //hexPathMeander(start, end); break;
                    //hexPathGentleWaveSmooth(start, end); break;
                    //hexPathGentleWave(start, end); break;
                    //hexPathWavy(start, end); break;
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
    bool water_is_protected = no_cross_terrain.Water;
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

    prior = get_adjacent_tile(start, tiles);

    QList<PathPoint> path = getHexPath(start, end);

    for (const PathPoint& p : path) {
        if ((p.pos.x() > -1 && p.pos.x() < Map.width-1) && (p.pos.y() > -1 && p.pos.y() < Map.height-1)) {
            field_pos = (p.pos.y() * Map.width) + p.pos.x();

            if (p.dir > -1) {
                part = direction_to_tile(tiles, p.dir, prior, !draw_only);
                prior_dir = p.dir;
            } else {
                part = direction_to_tile(tiles, prior_dir, prior, !draw_only);
            }

            int old_tile = Map.data[field_pos*2];

            int alt_part = process_special_tiles(part, old_tile, target);

            if (!draw_only) {
                if (!tile_is_building(old_tile)) {
                    Map.data[field_pos*2] = alt_part;
                }
            }

            if (!tile_is_building(old_tile)) {
                Redraw_Field(p.pos.x(), p.pos.y(), alt_part, Map.data[(field_pos*2)+1]);
            }

            prior = part;
        }
        if (!draw_only) qDebug() << "---------------------------------";
    }

    if (target == 6 || target == 7) {
        no_cross_terrain.Water = water_is_protected;
    }

}
