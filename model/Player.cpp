/*
	Player.cpp: contains definitions for the Player class
	
	MODEL?
*/

#include "Player.h"

#include "../util/utilio.h"
#include <algorithm>

using std::vector;

int Player::num_players = 9;

const char *Player::names[] =
{
	"Player 1", "Player 2", "Player 3", "Player 4", "Player 5", "Player 6", "Player 7", "Player 8",
	"GAIA", "Player 10", "Player 11", "Player 12", "Player 13", "Player 14", "Player 15", "Player 16"
};

Player::Player()
:	stable(-1)
{
}

void Player::reset()
{
	memset(name, 0, 30);
	stable = -1;
	enable = human = false;
	civ = 1;	//TODO: ini var
	strcpy(ai, "RandomGame");
	aimode = AI_standard;
	aifile.erase();
	memset(resources, 0, sizeof(resources));
	ndis_t = ndis_b = ndis_u = 0;
	memset(dis_tech, 0, sizeof(dis_tech));
	memset(dis_unit, 0, sizeof(dis_unit));
	memset(dis_bldg, 0, sizeof(dis_bldg));
	age = 0;
	pop = 75;
	camera[0] = 0;
	camera[1] = 0;
	avictory = false;
	memset(diplomacy, 0, sizeof(diplomacy));
	color = 0;
	ucount = 2;
	units.clear();
}

// functor for finding by UID
class uid_equals
{
public:
	uid_equals(UID uid)
		: _uid(uid)
	{
	}

	bool operator()(const Unit& u)
	{
		return (u.ident == _uid);
	}

private:
	UID _uid;
};

vector<Unit>::size_type Player::find_unit(UID uid) const
{
	vector<Unit>::const_iterator iter =
		std::find_if(units.begin(), units.end(), uid_equals(uid));

	// This will work for end() as well
	return (iter - units.begin());
}

void Player::erase_unit(vector<Unit>::size_type index)
{
	units.erase(units.begin() + index);
}

bool Player::read_aifile(FILE *in)
{
	readunk<long>(in, 0, "AI file unk 1", true);
	readunk<long>(in, 0, "AI file unk 2", true);
	aifile.read(in, sizeof(unsigned long));

	return true;
}

void Player::read_header_name(FILE * in)
{
	readbin(in, name, sizeof(name));
	//jump to the end of the 256-byte name area
	 if (fseek(in, 0x100 - sizeof(name), SEEK_CUR))
		 throw std::runtime_error("Seek error around player header name.");
}

void Player::read_header_stable(FILE * in)
{
	readbin(in, &stable);
}

struct PD1
{
	long enable; // boolean
	long human;  // boolean
	long civ;
	long unk;
};

void Player::read_data1(FILE * in)
{
	PD1 pd1;

	readbin(in, &pd1);

	enable = (pd1.enable != 0); // convert to bool
	human = (pd1.human != 0);   // convert to bool
	civ = pd1.civ;
	check<long>(pd1.unk, 4, "PlayerData1 unknown");
}

void Player::write_data1(FILE * out)
{
	PD1 pd1 =
	{
		enable, human, civ, 4
	};

	writebin(out, &pd1);
}

void Player::read_aimode(FILE * in)
{
	readbin(in, &aimode);
}

void Player::read_resources(FILE * in)
{
	readbin(in, resources, 6);
}

void Player::read_diplomacy(FILE * in)
{
	readbin(in, diplomacy, NUM_PLAYERS);
}

void Player::read_ndis_techs(FILE * in)
{
	readbin(in, &ndis_t);
}

void Player::read_dis_techs(FILE * in, const PerVersion& pv)
{
	readbin(in, dis_tech, pv.max_disables1);
}

void Player::read_ndis_units(FILE * in)
{
	readbin(in, &ndis_u);
}

void Player::read_dis_units(FILE * in, const PerVersion& pv)
{
	readbin(in, dis_unit, pv.max_disables1);
}

void Player::read_ndis_bldgs(FILE * in)
{
	readbin(in, &ndis_b);
}

void Player::read_dis_bldgs(FILE * in, const PerVersion& pv)
{
	readbin(in, dis_bldg, pv.max_disables2);
}

void Player::read_age(FILE * in)
{
	readbin(in, &age);
}

void Player::read_camera_longs(FILE * in)
{
	long value;

	// Player 1 has camera stored as longs, in Y, X order
	readbin(in, &value);
	camera[1] = static_cast<float>(value);
	readbin(in, &value);
	camera[0] = static_cast<float>(value);
}

void Player::read_data4(FILE * in, ScenVersion version)
{
	// Read and check duplicate copies of resources.
	readunk(in, static_cast<float>(resources[2]), "food float");
	readunk(in, static_cast<float>(resources[1]), "wood float");
	readunk(in, static_cast<float>(resources[0]), "gold float");
	readunk(in, static_cast<float>(resources[3]), "stone float");
	readunk(in, static_cast<float>(resources[4]), "orex float");
#if (GAME == 1)
	// unforunately it's the game, and not the scenario version
	readunk(in, static_cast<float>(resources[5]), "?? res float");
#endif

	if (version >= SVER_AOE2TC)
		readbin(in, &pop);
}

void Player::read_units(FILE *in)
{
	unsigned long num;
	int i;
	Unit u;

	readbin(in, &num);
	if (num)
	{
		// Reserve in advance for efficiency.
		units.reserve(num);

		i = num;
		while (i--)
		{
			u.read(in);
			units.push_back(u);
		}
	}
}

void Player::read_data3(FILE *in, float *view)
{
	short nlen;
	short ndiplomacy;
	long end;
	char unk;

	// just skip the constant name, dunno why it's there
	readbin(in, &nlen);
	SKIP(in, nlen);

	// read camera, or into view if calling func provided
	readbin(in, view ? view : camera, 2);

	readbin(in, &u1);
	readbin(in, &u2);
	readbin(in, &avictory);

	// skip diplomacy stuff
	readbin(in, &ndiplomacy);
	check<short>(ndiplomacy, 9, "PD3 diplomacy count");
	SKIP(in, sizeof(char) * ndiplomacy);	//diplomacy
	SKIP(in, sizeof(long) * ndiplomacy);	//diplomacy2

	readbin(in, &color);

	// now this is the weird stuff
	readbin(in, &ucount);

	if (ucount != 1.0F && ucount != 2.0F)
	{
		printf("Unknown PlayerData3 float value: %f\n", ucount);
		throw bad_data_error("Unknown PlayerData3 float value");
	}
	// printf("PD3 ucount was %f\n", ucount);

	readbin(in, &unk);
	check<char>(unk, 0, "PD3 char 1");
	readunk<char>(in, 0, "PD3 char 2");
	readunk<char>(in, 0, "PD3 char 3");
	readunk<char>(in, 0, "PD3 char 4");
	readunk<char>(in, 0, "PD3 char 5");
	readunk<char>(in, 0, "PD3 char 6");
	readunk<char>(in, 0, "PD3 char 7");
	readunk<char>(in, 0, "PD3 char 8");
	readunk<char>(in, 0, "PD3 char 9");

	if (unk)
	{
		printf("Skipping pd3 section, GTE hack.\n");
		SKIP(in, 44); // for Grand Theft Empires
	}

	if (ucount == 2.0F)
	{
		readunk<char>(in, 0, "PD3 char-2 1");
		readunk<char>(in, 0, "PD3 char-2 2");
		readunk<char>(in, 0, "PD3 char-2 3");
		readunk<char>(in, 0, "PD3 char-2 4");
		readunk<char>(in, 0, "PD3 char-2 5");
		readunk<char>(in, 0, "PD3 char-2 6");
		readunk<char>(in, 0, "PD3 char-2 7");
		readunk<char>(in, 0, "PD3 char-2 8");
	}

	readbin(in, &end);
	printf("End was %d\n", end);
}

void Player::write_header_name(FILE * out)
{
	size_t len = strlen(name);
	fwrite(name, sizeof(char), len, out);
	NULLS(out, 0x100 - len);
}

void Player::write_header_stable(FILE * out)
{
	writebin(out, &stable);
}

void Player::write_units(FILE *out)
{
	// Write count
	unsigned long num = units.size();
	fwrite(&num, sizeof(long), 1, out);

	// Write units
	for (std::vector<Unit>::const_iterator iter = units.begin();
		iter != units.end(); ++iter)
		fwrite(&*iter, Unit::size, 1, out);
}

void Player::write_data3(FILE *out, int me, float *view)
{
	int d_convert[4] = { 2, 3, 1, 4 };
	int i;

	writecs<unsigned short>(out, Player::names[me]);

	fwrite(
		view ? view : camera,	//let calling func determine view if it wants to
		sizeof(float), 2, out);	//camera[]

	fwrite(&u1, sizeof(short), 2, out);	//u1, u2

	fputc(avictory, out);
	fwrite(&num_players, sizeof(short), 1, out);

#ifndef NOWRITE_D2
	putc(diplomacy[GAIA_INDEX], out);	//GAIA first
	for (i = 0; i < num_players - 1; i++)
		putc(diplomacy[i], out);
#else
	NULLS(out, num_players);
#endif

#ifndef NOWRITE_D3
	NULLS(out, 4);	//gaia diplomacy
	for (i = 0; i < num_players - 1; i++)
	{
		if (i == me)
			fwrite(&d_convert[2], sizeof(long), 1, out);
		else
			fwrite(&d_convert[diplomacy[i]], sizeof(long), 1, out);
	}
#else
	NULLS(out, num_players * sizeof(long));
#endif

	fwrite(&color, 4, 2, out);	//color, ucount
	NULLS(out, 9);
	if (ucount == 1)
	{
		NULLS(out, 4);
	}
	else
	{
		NULLS(out, 8);
		writeval(out, (long)-1);
	}
}

bool Player::import_ai(const char *path)
{
	size_t ai_len = fsize(path);
	char * ai_buf;

	ai_buf = aifile.unlock(ai_len + 1);

	AutoFile ai_in(path, "rb");
	readbin(ai_in.get(), ai_buf, ai_len);
	ai_buf[ai_len] = '\0';	//null-terminate it

	aifile.lock();

	return true;
}


bool Player::export_ai(const char *path)
{
	FILE *ai_out;

	//check that player actually has an ai script.
	if (aifile.length() == 0)
		return false;

	ai_out = fopen(path, "wb");
	if (!ai_out)
		return false;

	fputs(aifile.c_str(), ai_out);

	fclose(ai_out);
	return true;
}
