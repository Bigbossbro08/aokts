#ifndef AOKTS_ECEDIT_H
#define AOKTS_ECEDIT_H

/**
	AOK Trigger Studio (See aokts.cpp for legal conditions.)
	WINDOWS VERSION.
	ecedit.h -- defines user interface for condition & effect editing

	VIEW/CONTROLLER
**/

#include "../model/trigger.h"

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

/*
	Effects / Conditions

	GWL_USERDATA = pointer to current effect / condition.
*/

enum ECMsgs
{
	/*	EC_Closing: Sent to parent window when editor closes.
	 *	wParam: MAKELONG(update?, valid?)
	 *	lParam: reinterpret_cast<LPARAM>(EditEC*)
	 */
	EC_Closing = WM_APP + 0x100,

	/*	EC_Update: Sent to editor to update the stored index by adding operand.
		Parameters: long operand, 0
	*/
	EC_Update,

	/*	EC_RefreshTriggers: Sent to editor to refresh the trigger list. (Only used by effects.)
		Parameters: int trigindex (or -1 for no trigger deleted), 0
	*/
	EC_RefreshTriggers,
};

class EditEC
{
public:
	// TODO: unsigned?
	int trigindex;
	int index;
	void *user;     // user-specified data
	HWND parent;	// GetParent() returns the wrong HWND.
	HWND mapview;	// handle to parent's mapview for interaction

	/* Updates the original condition/effect to the editor's copy. */
	virtual void update(Trigger *t) = 0;
};

class EditEffect : public EditEC
{
public:
	EditEffect(Effect &source);	//copy constructor must be called

	Effect e;
	void (__cdecl *TrigCallback)(HWND, unsigned);

	void update(Trigger *t);
};

INT_PTR WINAPI EffectWndProc(HWND dialog, UINT msg, WPARAM wParam, LPARAM lParam);

class EditCondition : public EditEC
{
public:
	Condition c;

	void update(Trigger *t);
};

INT_PTR WINAPI CondWndProc(HWND dialog, UINT msg, WPARAM wParam, LPARAM lParam);

#endif // AOKTS_ECEDIT_H
