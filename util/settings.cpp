/**
	AOK Trigger Studio (See aokts.cpp for legal conditions.)
	WINDOWS VERSION.

	settings.cpp: Code for loading/manipulating settings.h values.

	VIEW/CONTROLLER
**/

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include "settings.h"
#include <stdio.h>

const char *regkey_swgb = "SOFTWARE\\LucasArts Entertainment Company LLC\\Star Wars Galactic Battlegrounds\\1.0";
const char *regval_swgb = "Install Path";

const char *regkey_aok = "SOFTWARE\\Microsoft\\Microsoft Games\\Age of Empires\\2.0";
const char *regval_aok = "InstallationDirectory";

Setts::Setts()
:	recent_used(0)
{
}

Setts::~Setts()
{
	write_recent(path);
}

bool Setts::load()
{
	LONG result;
	//HKEY key;
	DWORD size = sizeof(BasePath);
	char buffer[_MAX_PATH];

	GetCurrentDirectory(_MAX_PATH, path);
	strcat(path, "\\ts.ini");

	/* result = RegOpenKeyEx(HKEY_LOCAL_MACHINE, regkey, 0, KEY_READ, &key);
	if (result == ERROR_SUCCESS)
	{
		result = RegQueryValueEx(key, regval, NULL, NULL, (BYTE*)BasePath, &size);
		RegCloseKey(key);
		strcat(BasePath, "\\"); // aok
		//strcat(BasePath, "\\Game\\"); //swgb
	} */
    result = ERROR_SUCCESS;

	/* [Decompressed] */
	GetCurrentDirectory(_MAX_PATH, TempPath);
	GetPrivateProfileString("Decompressed", "Path", "scendata.tmp", buffer, sizeof(buffer), path);
	strcat(TempPath, "\\");
	strcat(TempPath, buffer);
	DelTempOnExit = GetPrivateProfileInt("Decompressed", "DelOnExit", 1, path) == 1;

	/* [Warnings] */
	warnbits = 0;
	if (GetPrivateProfileInt("Warnings", "WeirdResource", 1, path) != 0)
		warnbits |= WARN_WEIRDRESOURCE;

	editall = GetPrivateProfileInt("Advanced", "EditAll", 0, path) != 0;

	/* [Log] */
	GetPrivateProfileString("Log", "Name", "ts.log", logname, sizeof(logname), path);
	intense = GetPrivateProfileInt("Log", "Intense", 0, path) != 0;

	/* [Minimap] */
	zoom = (char)GetPrivateProfileInt("Minimap", "Zoom", 1, path);

	/* [Recent#] */
	read_recent(path);

	return (result == ERROR_SUCCESS);
}

void Setts::read_recent(const char *path)
{
	char section[10] = "Recent1";	//section[6] = '1'
	RecentFile *r_parse = recent;
	RecentFile **p_parse;
	int count = MAX_RECENT;

	while (count--)
	{
		GetPrivateProfileString(section, "Name", "",
			r_parse->display, sizeof(r_parse->display),
			path);

		GetPrivateProfileString(section, "Path", "",
			r_parse->path, sizeof(r_parse->path),
			path);

		r_parse++;
		section[6]++;	//increment the number
	}

	/* Now set up the linked-list pointers. */
	count = MAX_RECENT;
	r_parse = recent;
	p_parse = &recent_first;
	while (count-- && *r_parse->display)
	{
		*p_parse = r_parse;
		p_parse = &r_parse->next;	//oooh this algorithm works nicely :-D
		r_parse++;
		recent_used++;
	}
}

void Setts::write_recent(const char *path)
{
	RecentFile *r_parse = recent_first;
	char section[10] = "Recent1";	//section[6] = '1'

	while (r_parse && recent_used--)	//we can trash recent_used by now
	{
		WritePrivateProfileString(section, "Name", r_parse->display, path);
		WritePrivateProfileString(section, "Path", r_parse->path, path);

		r_parse = r_parse->next;
		section[6]++;
	}
}

/* Find the next available RecentFile */
struct RecentFile * Setts::recent_getnext()
{
	struct RecentFile *ret;

	if (recent_used == MAX_RECENT)
	{
		/* Get the end of the chain. */
		ret = recent_first;
		while (ret->next)
			ret = ret->next;
	}
	else
		/* There's an empty struct, so use it. */
		ret = recent + recent_used++;

	return ret;
}

/*
	Setts::recent_push: pushes a RecentFile to the top of its "stack"
*/
void Setts::recent_push(RecentFile *target)
{
	struct RecentFile *previous_top;
	struct RecentFile *r_parse;

	if (recent_first != target)	/* Nothing needs to be done. */
	{
		/* Put target on the top of the stack. */
		previous_top = recent_first;
		recent_first = target;

		/* Parse through links, finding and removing target link. */
		r_parse = previous_top;	//skip target because target->next isn't set properly yet.
		while (r_parse)
		{
			if (r_parse->next == target)
			{
				r_parse->next = target->next;
				break;
			}

			r_parse = r_parse->next;
		}

		/* Set target's next pointer to the previous top o' the stack. */
		target->next = previous_top;
	}
}
