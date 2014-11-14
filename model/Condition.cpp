#include "Condition.h"
#include <sstream>
#include "TriggerVisitor.h"

#include "../util/utilio.h"
#include "../util/Buffer.h"

#pragma pack(push, 4)
// An condition as stored in in scenario
struct Genie_Condition
{
	long type;
	long check;
	long amount;
	long resource_type;
	long uid_object;
	long uid_location;
	long unit_const;
	long player;
	long technology;
	long timer;
	long reserved;
	AOKRECT area;
	long unit_group;
	long unit_type;
	long ai_signal;
};
#pragma pack(pop)

Condition::Condition()
:	ECBase(CONDITION),
	amount(-1),
	res_type(-1),
	object(-1),
	u_loc(-1),
	pUnit(NULL),
	player(-1),
	pTech(NULL),
	timer(-1),
	reserved(-1),
	// AOKRECT default constructor OK
	group(-1),
	utype(-1),
	ai_signal(-1)
{
}

Condition::Condition(Buffer& b)
:	ECBase(CONDITION)
{
	// read flat data
	Genie_Condition genie;
	b.read(&genie, sizeof(genie));
	std::swap(genie.type, genie.check); // HACK: un-swap type, check
	fromGenie(genie);
}

std::string Condition::getName() const
{
	return (type < NUM_CONDS) ? types[type] : "Unknown!";
}

std::string Condition::getNameTip() const
{
    std::string stype = std::string((type < NUM_CONDS) ? types_short[type] : "Unknown!");
    if (type == 10) {
        stype.append(": ");
        std::ostringstream convert;
        convert << timer;
        stype.append(convert.str());
    }
	return stype;
}

int Condition::getPlayer() const
{
	return player;
}

void Condition::setPlayer(int pplayer)
{
	player = pplayer;
}

bool Condition::check() const
{
	switch (type)
	{
	case CONDITION_BringObjectToArea:
		return (object >= 0 && area.left >= 0);

	case CONDITION_BringObjectToObject:
		return (object >= 0 && u_loc >= 0);

	case CONDITION_OwnObjects:
	case CONDITION_OwnFewerObjects:
	case CONDITION_OwnFewerFoundations:
		return (player >= 0 && amount >= 0 && !(area.top == 0 && area.left == 0 && area.bottom == 0 && area.right == 0));

	case CONDITION_ObjectsInArea:
	case CONDITION_SelectedObjectsInArea:
	case CONDITION_PoweredObjectsInArea:
		return (area.left >= 0 && amount >= 0);

	case CONDITION_DestroyObject:
		return (object >= 0);

	case CONDITION_CaptureObject:
		return (object >= 0 && player >= 0);

	case CONDITION_AccumulateAttribute:
		return (player >= 0 && res_type >= 0);

	case CONDITION_ResearchTehcnology:
	case CONDITION_ResearchingTechnology:
		return (player >= 0 && pTech->id() >= 0);

	case CONDITION_Timer:
		return (timer >= 0);

	case CONDITION_ObjectSelected:
		return (object >= 0);

	case CONDITION_AISignal:
		return (ai_signal >= 0);

	case CONDITION_PlayerDefeated:
	case CONDITION_UnitsQueuedPastPopCap:
		return (player >= 0);

	case CONDITION_ObjectHasTarget:
		return (object >= 0 && u_loc >= 0);

	case CONDITION_ObjectVisible:
	case CONDITION_ObjectNotVisible:
		return (object >= 0);

	//CONDITION_ResearchingTechnology above

	case CONDITION_UnitsGarrisoned:
		return (object >= 0 && amount >= 0);

	case CONDITION_DifficultyLevel:
		return (amount >= 0);

	//CONDITION_OwnFewerFoundations shares with OwnObjects above
	//CONDITION_SelectedObjectsInArea shares with ObjectsInArea above
	//CONDITION_PoweredObjectsInArea shares with ObjectsInArea above
	//CONDITION_UnitsQueuedPastPopCap shares with PlayerDefeated above

	default:
		return false;
	}
}

void Condition::read(FILE *in)
{
	Genie_Condition genie;

	readbin(in, &genie);
	fromGenie(genie);
}

void Condition::write(FILE *out)
{
	Genie_Condition genie = toGenie();
	writebin(out, &genie);
}

void Condition::tobuffer(Buffer &b) const
{
	/* Even though the Genie format sucks, we use it for Buffer operations
	 * (i.e., copy & paste) since it's easier to maintain one sucky format than
	 * one sucky and one slightly-less-sucky format.
	 */

	// write flat data
	Genie_Condition genie = toGenie();
	std::swap(genie.type, genie.check); // HACK: swap type, check
	b.write(&genie, sizeof(genie));
}

void Condition::accept(TriggerVisitor& tv)
{
	tv.visit(*this);
}

void Condition::fromGenie(const Genie_Condition& genie)
{
	if (genie.check != CONDITION)
		throw bad_data_error("Condition has incorrect check value.");

	type = genie.type;
	ttype = static_cast<TType>(genie.check);
	res_type = genie.resource_type;
	amount = genie.amount;
	object = genie.uid_object;
	u_loc = genie.uid_location;
	pUnit = esdata.units.getByIdSafe(genie.unit_const);
	player = genie.player;
	pTech = esdata.techs.getByIdSafe(genie.technology);
	timer = genie.timer;
	reserved = genie.reserved;
	area = genie.area;
	group = genie.unit_group;
	utype = genie.unit_type;
	ai_signal = genie.ai_signal;
}

Genie_Condition Condition::toGenie() const
{
	Genie_Condition ret =
	{
		type,
		ttype,
		amount,
		res_type,
		object,
		u_loc,
		pUnit ? pUnit->id() : -1,
		player,
		pTech ? pTech->id() : -1,
		timer,
		reserved,
		area,
		group,
		utype,
		ai_signal
	};

	return ret;
}
