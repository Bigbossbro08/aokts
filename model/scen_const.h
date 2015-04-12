/* MODEL */

#ifndef SCEN_CONST_H
#define SCEN_CONST_H

#include "esdata.h"

/* Typedefs */

typedef long UID;
typedef short UCNST;

/**
 * This is not saved in the scenario but is used internally by many data
 * reading functions.
 */
enum Game
{
	UNKNOWN = 0,
	AOE     = 1,
	ROR     = 2,
	AOK     = 3,
	AOC     = 4,
	AOHD    = 5,
	AOF     = 6,
	SWGB    = 7,
	SWGBCC  = 8,
	NOCONV  = 100
};

enum ScenVersion1
{
	SV1_AOE1		= 10,
	SV1_AOK		= 18,
	SV1_AOC_SWGB	= 21
};

enum ScenVersion2
{
	SV2_AOE1		= 15,
	SV2_AOK		    = 18,
	SV2_AOC_SWGB	= 22,
	SV2_AOHD_AOF	= 23,// HD / The Forgotten
	SV2_SWGBCC		= 24 // SWGB
};

/**
 * This is not saved in the scenario, but is used internally by AOKTS to
 * encapsulate differences in versions of the scenario format.
 */
struct PerVersion
{
	int messages_count;
	bool mstrings;
	int max_disables1; // max disable tech and unit
	int max_disables2; // max disable buildings
};

struct PerGame
{
	UCNST max_unit; // max unit types
	int max_research; // max research
	int max_tech; // max tech
	int max_terrains; // max tech
};

/* Internal Constants */

const double PI = 3.1415927;	//for radians

enum AOKTS_ERROR
{
	ERR_unknown = -11,	//unknown error
	ERR_outofbounds = -10,	//something went out of bounds, depends on context
	ERR_overlap = -9,	//used for map copy, target overlaps source
	ERR_data = -8,		//decompressed data error
	ERR_compr = -7,		//zlib negative return
	ERR_mem = -6,		//allocation failure (usually a bug)
	ERR_zver = -5,		//zlib DLL mismatch
	ERR_digit = -4,		//my mistake
	ERR_bitmap = -3,	//bitmap incomplete
	ERR_noscen = -2,	//not a scenario
	ERR_sver = -1,		//unrecognized scenario version
	ERR_none = 0,		//the only good error is no error
};

/** External constants **/

enum Diplomacy
{
	DIP_ally, DIP_neutral, DIP_unknown, DIP_enemy
};

extern size_t MapSizes[8]; //conversion from standard sizes to tiles (see scen.cpp for values)
extern size_t Elevations[9]; //conversion from standard sizes to tiles (see scen.cpp for values)

#define NUM_STYPES 4		//number of stockpile resource types

#define NUM_MSGS		6
#define NUM_CINEM		4

extern const char* message_names[NUM_MSGS];
extern const char* cinem_names[NUM_CINEM];

#define NUM_DIFFICULTIES 5
extern const char* difficulties_18[NUM_DIFFICULTIES];
extern const char* difficulties_21[NUM_DIFFICULTIES];

#define NUM_AGES 6
extern const struct PAIR ages[NUM_AGES];

#define NUM_GROUPS 63
extern const struct PAIR groups[NUM_GROUPS];

#define NUM_UTYPES 5
extern const struct PAIR utypes[NUM_UTYPES];

#endif //SCEN_CONST_H
