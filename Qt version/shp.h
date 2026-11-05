/* SHP.H by Jan Knipperts
Header file to read and write History Line 1914-1918 map files (*.shp)

27.11.2023
- Fixed handling of not saved transport units in the original game files

19.02.2024
- Added the function "Correct_building_record_from_map()" to be able to record new / changed buildings correctly.

30.07.2024
- Renamed some functions
- Code optimized in some places
- Fixed a bug that corrupted the contents of buildings when new or many buildings were placed

*/


#ifndef SHP_H
#define SHP_H

#include "Lib.h"

struct Building_data //Date structure in shp file
{
    unsigned char  Owner;     // 	To which side the building belongs (0 = German; 1 = French, 2 = Neutral)
    unsigned char  Type;      //	Type of building (0=HQ; 1 = Factory; 2 = Depot; 3 = Transport unit)
    unsigned char  Index;     //	xth unit/building of this type
    unsigned char  Resources; //    number of resources this building generates per turn
    unsigned char  Unknown;   //    The meaning of this byte still has to be found out.
    unsigned char  Units[7];  //	Units inside the building/transport unit
};

struct  Building_data_ext  //Extanded structure for our use
{
    int             Field;
    Building_data*  Properties;
};

struct  SHP_struct
{
    unsigned char   can_be_built[Num_Units];
    unsigned char   num_buildings;
    Building_data*  buildings;
};

struct Building_statistics
{
    unsigned char   num_HQ; //number of HQ    
    unsigned char   num_F;  //number of fabrications  
    unsigned char   num_D; //number of depots    
    unsigned char   num_T; //number of transport units    
    unsigned char   num_buildings; //total number of buildings    
};

struct Unit_backup
{
    unsigned char  Units[7];  //	Units inside the building/transport unit
    unsigned int   Field;
};

extern SHP_struct          SHP;
extern Building_data_ext*  Building_info;
extern Building_statistics Building_stat;
extern Building_data       Bdata;
extern Building_data_ext*  Building_backup;
extern int                 Num_building_entries;


extern int Read_shp_data(std::string shpname);
extern void Add_building_positions();
extern int Create_shp(std::string shpname);
extern void Create_valid_building_record_from_map();
extern int Get_Building_by_field(int fieldno);
extern void Update_building_record_from_map();


#endif // SHP_H

