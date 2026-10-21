/*
 * HL Editor
 *
 * hexPath.h by David Konrad
 *
 * Some parts of this code is generated in collaboration with Grok AI.
 * I think it is obvious where, but have marked it with comments also.
 *
 * I simply could not figure out how to make the shortest path from A to B in this virtual hexagon
 * setup. My 'engine' would not work, or worked only with straight lines and a few steps aside.
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
    bool Forest = true;
    bool River = false;
    bool Residentals = false;
    bool Dirtroad = false;
    bool Road = false;
    bool Railroad = false;
};

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

