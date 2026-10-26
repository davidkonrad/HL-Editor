/*
 * HL Editor
 *
 * nativemap.cpp by David Konrad
 *
 * implementation for the nativemap dialog
 *
 */

#include "nativemap.h"

struct NativeMap {
    int id;
    QString Code;
    QString Side;
    QString Com;
    QString Season;
};

struct NativeMap NativeMaps[] = {
    { 0,  "PULSE", "Both",   "I",   "Summer" },
    { 1,  "CIVIL", "German", "I",   "Winter" },
    { 2,  "MOUSE", "German", "I",   "Winter" },
    { 3,  "VENOM", "German", "I",   "Summer" },
    { 4,  "NOISE", "German", "I",   "Summer" },
    { 5,  "RIGHT", "German", "I",   "Summer" },
    { 6,  "ORKAN", "German", "I",   "Summer" },
    { 7,  "FRONT", "German", "I",   "Summer" },
    { 8,  "RATIO", "German", "I",   "Winter" },
    { 9,  "PARTS", "German", "I",   "Winter" },
    { 10, "PLANE", "German", "I",   "Summer" },
    { 11, "FLAME", "German", "I",   "Summer" },
    { 12, "GOTHA", "German", "I",   "Summer" },
    { 13, "BALON", "German", "I",   "Winter" },
    { 14, "PAUSE", "German", "I",   "Winter" },
    { 15, "ELITE", "German", "I",   "Summer" },
    { 16, "INFRA", "German", "I",   "Summer" },
    { 17, "HILLS", "German", "I",   "Summer" },
    { 18, "COBRA", "German", "I",   "Summer" },
    { 19, "ATLAS", "German", "I",   "Winter" },
    { 20, "AMPER", "German", "I",   "Winter" },
    { 21, "RHEIN", "German", "II",  "Summer" },
    { 22, "CANDL", "German", "IV",  "Summer" },
    { 23, "STERN", "German", "II",  "Summer" },
    { 24, "BATLE", "Allies", "II",  "Summer" },
    { 25, "GOOSE", "Allies", "II",  "Winter" },
    { 26, "SPORT", "Allies", "II",  "Winter" },
    { 27, "BIMBO", "Allies", "II",  "Summer" },
    { 28, "TEMPO", "Allies", "II",  "Summer" },
    { 29, "BARON", "Allies", "II",  "Summer" },
    { 30, "BUMMM", "Allies", "II",  "Summer" },
    { 31, "LEVEL", "Allies", "II",  "Summer" },
    { 32, "TOXIN", "Allies", "II",  "Winter" },
    { 33, "PRINC", "Allies", "II",  "Winter" },
    { 34, "CLEAN", "Allies", "II",  "Summer" },
    { 35, "XENON", "Allies", "II",  "Summer" },
    { 36, "SIGNS", "Allies", "II",  "Summer" },
    { 37, "HOUSE", "Allies", "II",  "Winter" },
    { 38, "SIGMA", "Allies", "II",  "Winter" },
    { 39, "SEVEN", "Allies", "II",  "Summer" },
    { 40, "ZOMBI", "Allies", "II",  "Summer" },
    { 41, "MOVES", "Allies", "II",  "Summer" },
    { 42, "BLADE", "Allies", "II",  "Summer" },
    { 43, "ZORRO", "Allies", "II",  "Winter" },
    { 44, "STONE", "Allies", "II",  "Winter" },
    { 45, "MOSEL", "Allies", "III", "Summer" },
    { 46, "ORDER", "Allies", "II",  "Summer" },
    { 47, "SODOM", "Allies", "III", "Summer" }
};

nativemapwindow::nativemapwindow(): QWidget(nullptr)
{

}
