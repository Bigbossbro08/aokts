/**
	AOK Trigger Studio (See aokts.cpp for legal conditions.)
	WINDOWS VERSION
	playeredit.cpp -- Defines functions for Players editor.

	VIEW/CONTROLLER
**/

#include "editors.h"
#include "../res/resource.h"
#include "utilui.h"
#include "../util/settings.h"
#include "LCombo.h"
#include "mapview.h"
#include "../util/winugly.h"

#include <commdlg.h>

/* Players */

//these convert from enum Diplomacy to Win32 Checkbox states and vice-versa
WPARAM d_to_b[4] = { BST_CHECKED, BST_INDETERMINATE, BST_UNCHECKED, BST_UNCHECKED };
enum Diplomacy b_to_d[3] = { DIP_enemy, DIP_ally, DIP_neutral };

//These controls are disabled when user selects GAIA.
int gaia_disables[] =
{
	IDC_P_NAME, IDC_P_STABLE, IDC_P_HUMAN, IDC_P_COLOR, IDC_P_SPDIP, IDC_P_ACTIVE,
	IDC_P_AV, IDC_P_X, IDC_P_Y,
	IDC_P_AI, IDC_P_EXAI, IDC_P_IMAI, IDC_P_CLEARAI, IDC_P_AISCRIPT,
	IDC_P_VC, IDC_P_EXVC, IDC_P_IMVC, IDC_P_CLEARVC, IDC_P_VCSCRIPT,
	IDC_P_CTY, IDC_P_EXCTY, IDC_P_IMCTY, IDC_P_CLEARCTY, IDC_P_CTYSCRIPT,
	IDC_P_AIMODE, IDC_P_RANDOMGAME, IDC_P_PROMISORY
};

void LoadPlayer(HWND dialog)
{
	Player *p = propdata.p;
	int count = sizeof(gaia_disables) / sizeof(int);
	bool is_gaia = (propdata.pindex == GAIA_INDEX);

    if (!setts.editall)
	    for (int i = 0; i < count; i++)
		    ENABLE_WND(gaia_disables[i], !is_gaia);

	if (propdata.pindex == PLAYER1_INDEX)
		ENABLE_WND(IDC_P_ACTIVE, false);

	SetDlgItemText(dialog, IDC_P_NAME, p->name);
	LCombo_SelById(dialog, IDC_P_CIV, p->civ);
	SetDlgItemInt(dialog, IDC_P_STABLE, p->stable, TRUE);
	SetDlgItemInt(dialog, IDC_P_GOLD, p->resources[0], FALSE);
	SetDlgItemInt(dialog, IDC_P_WOOD, p->resources[1], FALSE);
	SetDlgItemInt(dialog, IDC_P_FOOD, p->resources[2], FALSE);
	SetDlgItemInt(dialog, IDC_P_STONE, p->resources[3], FALSE);
	SetDlgItemInt(dialog, IDC_P_OREX, p->resources[4], FALSE);
	SetDlgItemInt(dialog, IDC_P_OREY, p->resources[5], FALSE);
	SendDlgItemMessage(dialog, IDC_P_ACTIVE, BM_SETCHECK, p->enable, 0);
	SendDlgItemMessage(dialog, IDC_P_HUMAN, BM_SETCHECK, p->human, 0);
	SetDlgItemInt(dialog, IDC_P_POP, (int)p->pop, FALSE);
	SetDlgItemFloat(dialog, IDC_P_X, p->camera[0]);
	SetDlgItemFloat(dialog, IDC_P_Y, p->camera[1]);
	SendDlgItemMessage(dialog, IDC_P_COLOR, CB_SETCURSEL, p->color, 0);	//assuming in order
	SendDlgItemMessage(dialog, IDC_P_AV, BM_SETCHECK, p->avictory, 0);
	if (propdata.sel0 == -1) {
	    true;
		//MessageBox(
		//			dialog,
		//			"Problem detected. Reload page.",
		//			"Player Editor",
		//			MB_ICONWARNING);
	} else {
		SendDlgItemMessage(dialog, IDC_P_DSTATE, BM_SETCHECK, d_to_b[p->diplomacy[propdata.sel0]], 0);
	}
	SendDlgItemMessage(dialog, IDC_P_AGE, CB_SETCURSEL, p->age, 0);
	SetDlgItemFloat(dialog, IDC_P_UF, p->ucount);
	SetDlgItemInt(dialog, IDC_P_US0, p->u1, FALSE);
	SetDlgItemInt(dialog, IDC_P_US1, p->u2, FALSE);

    HWND hAI = GetDlgItem(dialog, IDC_P_AI);
    HWND hVC = GetDlgItem(dialog, IDC_P_VC);
    HWND hCTY = GetDlgItem(dialog, IDC_P_CTY);

    if (hAI) {
	    /* AI is special */
	    SendDlgItemMessage(dialog, IDC_P_AIMODE, BM_SETCHECK, p->aimode == AI_standard, 0);
	    SetDlgItemText(dialog, IDC_P_AI, p->ai);
	    //EnableWindow(GetDlgItem(dialog, IDC_P_AI), p->aimode != AI_standard);
	    SetDlgItemText(dialog, IDC_P_AISCRIPT, p->aifile.c_str());
	    SetDlgItemInt(dialog, IDC_P_AIMODE_VAL, p->aimode, FALSE);
	}

    if (hVC) {
	    SetDlgItemText(dialog, IDC_P_VC, p->vc);
	    SetDlgItemText(dialog, IDC_P_VCSCRIPT, p->vcfile.c_str());
	}

    if (hCTY) {
	    SetDlgItemText(dialog, IDC_P_CTY, p->cty);
	    SetDlgItemText(dialog, IDC_P_CTYSCRIPT, p->ctyfile.c_str());
	}
}

void SavePlayer(HWND dialog)
{
	Player *p = propdata.p;
	GetDlgItemText(dialog, IDC_P_NAME, p->name, 30);
	p->civ = LCombo_GetSelId(dialog, IDC_P_CIV);
	p->stable = GetDlgItemInt(dialog, IDC_P_STABLE, NULL, TRUE);
	p->resources[0] = GetDlgItemInt(dialog, IDC_P_GOLD, NULL, FALSE);
	p->resources[1] = GetDlgItemInt(dialog, IDC_P_WOOD, NULL, FALSE);
	p->resources[2] = GetDlgItemInt(dialog, IDC_P_FOOD, NULL, FALSE);
	p->resources[3] = GetDlgItemInt(dialog, IDC_P_STONE, NULL, FALSE);
	p->resources[4] = GetDlgItemInt(dialog, IDC_P_OREX, NULL, FALSE);
	p->resources[5] = GetDlgItemInt(dialog, IDC_P_OREY, NULL, FALSE);
	p->enable = Button_IsChecked(GetDlgItem(dialog, IDC_P_ACTIVE));
	p->human = Button_IsChecked(GetDlgItem(dialog, IDC_P_HUMAN));
	p->pop = static_cast<float>(GetDlgItemInt(dialog, IDC_P_POP, NULL, FALSE));
	p->camera[0] = GetDlgItemFloat(dialog, IDC_P_X);
	p->camera[1] = GetDlgItemFloat(dialog, IDC_P_Y);
	p->color = LCombo_GetSelId(dialog, IDC_P_COLOR);
	p->avictory = (SendDlgItemMessage(dialog, IDC_P_AV, BM_GETCHECK, 0, 0) != 0);
	p->diplomacy[propdata.sel0] = b_to_d[SendDlgItemMessage(dialog, IDC_P_DSTATE, BM_GETCHECK, 0, 0)];
	p->age = SendDlgItemMessage(dialog, IDC_P_AGE, CB_GETCURSEL, 0, 0);
	p->u1 = toshort(GetDlgItemInt(dialog, IDC_P_US0, NULL, FALSE));
	p->u2 = toshort(GetDlgItemInt(dialog, IDC_P_US1, NULL, FALSE));

    HWND hAI = GetDlgItem(dialog, IDC_P_AI);
    HWND hVC = GetDlgItem(dialog, IDC_P_VC);
    HWND hCTY = GetDlgItem(dialog, IDC_P_CTY);

    if (hAI) {
        // only change this when clicking on checkbox
	    //p->aimode = (SendDlgItemMessage(dialog, IDC_P_AIMODE, BM_GETCHECK, 0, 0) != 0);
	    if (p->aimode != AI_standard)
	    {
		    GetDlgItemText(dialog, IDC_P_AI, p->ai, _MAX_FNAME);
		    if (*p->ai)
			    p->aimode = AI_custom;
	    }
	    //else
	    //	strcpy(p->ai, scen.StandardAI);
	    GetWindowText(GetDlgItem(dialog, IDC_P_AISCRIPT), p->aifile);
    }

    if (hVC) {
        GetDlgItemText(dialog, IDC_P_VC, p->vc, _MAX_FNAME);
	    GetWindowText(GetDlgItem(dialog, IDC_P_VCSCRIPT), p->vcfile);
    }

    if (hCTY) {
        GetDlgItemText(dialog, IDC_P_CTY, p->cty, _MAX_FNAME);
	    GetWindowText(GetDlgItem(dialog, IDC_P_CTYSCRIPT), p->ctyfile);
	}
}

const char errorImpExpFail[] =
"Error. Please make sure the file you selected is valid.";
#define IDC_P_CTY           1188


// FIXME: separate this alternate cohesion crap
void Players_ManageAI(HWND dialog, bool import)
{
	char path[_MAX_PATH], dir[_MAX_PATH];
	OPENFILENAME ofn;

	//shared init
	ofn.lStructSize = sizeof(OPENFILENAME);
	ofn.hwndOwner = dialog;
	ofn.lpstrFilter = "AI Scripts (*.per)\0*.per\0";
	ofn.nFilterIndex = 1;
	ofn.lpstrCustomFilter = NULL;	//user should not change filter
	ofn.lpstrFile = path;
	ofn.nMaxFile = sizeof(path);
	strcpy(dir, setts.BasePath);
	strcat(dir, "ai");
	ofn.lpstrInitialDir = dir;

	if (import)
	{
		*path = '\0';
		ofn.lpstrFileTitle = propdata.p->ai;
		ofn.nMaxFileTitle = _MAX_FNAME;
		ofn.lpstrTitle = "Import AI Script";
		ofn.Flags = OFN_NOCHANGEDIR | OFN_HIDEREADONLY;
		ofn.lpstrDefExt = "per";

		if (GetOpenFileName(&ofn))
		{
			propdata.p->ai
				[ofn.nFileExtension - ofn.nFileOffset - 1]	//offset to extension '.'
				= '\0';
			//do it
			if (propdata.p->import_ai(path))
				SetWindowText(propdata.statusbar, "AI successfully imported.");
			else
				MessageBox(dialog, errorImpExpFail, "AI Import", MB_OK);
		}
	}
	else if (propdata.p->aifile.length()) //export, so check for existence
	{
		strcpy(path, propdata.p->ai);
		ofn.lpstrFileTitle = NULL;
		ofn.lpstrTitle = "Export AI Script";
		ofn.Flags = OFN_NOCHANGEDIR | OFN_NOREADONLYRETURN | OFN_OVERWRITEPROMPT;
		ofn.lpstrDefExt = "per";

		if (GetSaveFileName(&ofn))
		{
			//do it
			if (propdata.p->export_ai(path))
				SetWindowText(propdata.statusbar, "AI successfully exported.");
			else
				MessageBox(dialog, errorImpExpFail, "AI Export Warning", MB_ICONWARNING);
		}
	}
	else
		MessageBox(dialog, "Sorry, that player doesn't have an AI File.", "AI Export", MB_OK);
}

// FIXME: separate this alternate cohesion crap
void Players_ManageVC(HWND dialog, bool import)
{
	char path[_MAX_PATH], dir[_MAX_PATH];
	OPENFILENAME ofn;

	//shared init
	ofn.lStructSize = sizeof(OPENFILENAME);
	ofn.hwndOwner = dialog;
	ofn.lpstrFilter = "VC Scripts (*.vc)\0*.vc\0";
	ofn.nFilterIndex = 1;
	ofn.lpstrCustomFilter = NULL;	//user should not change filter
	ofn.lpstrFile = path;
	ofn.nMaxFile = sizeof(path);
	strcpy(dir, setts.BasePath);
	strcat(dir, "vc");
	ofn.lpstrInitialDir = dir;

	if (import)
	{
		*path = '\0';
		ofn.lpstrFileTitle = propdata.p->vc;
		ofn.nMaxFileTitle = _MAX_FNAME;
		ofn.lpstrTitle = "Import VC Script";
		ofn.Flags = OFN_NOCHANGEDIR | OFN_HIDEREADONLY;
		ofn.lpstrDefExt = "vc";

		if (GetOpenFileName(&ofn))
		{
			propdata.p->vc
				[ofn.nFileExtension - ofn.nFileOffset - 1]	//offset to extension '.'
				= '\0';
			//do it
			if (propdata.p->import_vc(path))
				SetWindowText(propdata.statusbar, "VC successfully imported.");
			else
				MessageBox(dialog, errorImpExpFail, "VC Import", MB_OK);
		}
	}
	else if (propdata.p->aifile.length()) //export, so check for existence
	{
		strcpy(path, propdata.p->vc);
		ofn.lpstrFileTitle = NULL;
		ofn.lpstrTitle = "Export VC Script";
		ofn.Flags = OFN_NOCHANGEDIR | OFN_NOREADONLYRETURN | OFN_OVERWRITEPROMPT;
		ofn.lpstrDefExt = "vc";

		if (GetSaveFileName(&ofn))
		{
			//do it
			if (propdata.p->export_vc(path))
				SetWindowText(propdata.statusbar, "VC successfully exported.");
			else
				MessageBox(dialog, errorImpExpFail, "VC Export Warning", MB_ICONWARNING);
		}
	}
	else
		MessageBox(dialog, "Sorry, that player doesn't have an VC File.", "VC Export", MB_OK);
}

// FIXME: separate this alternate cohesion crap
void Players_ManageCTY(HWND dialog, bool import)
{
	char path[_MAX_PATH], dir[_MAX_PATH];
	OPENFILENAME ofn;

	//shared init
	ofn.lStructSize = sizeof(OPENFILENAME);
	ofn.hwndOwner = dialog;
	ofn.lpstrFilter = "CTY Scripts (*.cty)\0*.cty\0";
	ofn.nFilterIndex = 1;
	ofn.lpstrCustomFilter = NULL;	//user should not change filter
	ofn.lpstrFile = path;
	ofn.nMaxFile = sizeof(path);
	strcpy(dir, setts.BasePath);
	strcat(dir, "cty");
	ofn.lpstrInitialDir = dir;

	if (import)
	{
		*path = '\0';
		ofn.lpstrFileTitle = propdata.p->cty;
		ofn.nMaxFileTitle = _MAX_FNAME;
		ofn.lpstrTitle = "Import CTY Script";
		ofn.Flags = OFN_NOCHANGEDIR | OFN_HIDEREADONLY;
		ofn.lpstrDefExt = "cty";

		if (GetOpenFileName(&ofn))
		{
			propdata.p->cty
				[ofn.nFileExtension - ofn.nFileOffset - 1]	//offset to extension '.'
				= '\0';
			//do it
			if (propdata.p->import_cty(path))
				SetWindowText(propdata.statusbar, "CTY successfully imported.");
			else
				MessageBox(dialog, errorImpExpFail, "CTY Import", MB_OK);
		}
	}
	else if (propdata.p->ctyfile.length()) //export, so check for existence
	{
		strcpy(path, propdata.p->cty);
		ofn.lpstrFileTitle = NULL;
		ofn.lpstrTitle = "Export CTY Script";
		ofn.Flags = OFN_NOCHANGEDIR | OFN_NOREADONLYRETURN | OFN_OVERWRITEPROMPT;
		ofn.lpstrDefExt = "cty";

		if (GetSaveFileName(&ofn))
		{
			//do it
			if (propdata.p->export_cty(path))
				SetWindowText(propdata.statusbar, "CTY successfully exported.");
			else
				MessageBox(dialog, errorImpExpFail, "CTY Export Warning", MB_ICONWARNING);
		}
	}
	else
		MessageBox(dialog, "Sorry, that player doesn't have an CTY File.", "CTY Export", MB_OK);
}

void Players_HandleCommand(HWND dialog, WORD code, WORD id, HWND control)
{
	Player *p = propdata.p;

	switch (code)
	{
	case BN_CLICKED:
	case CBN_SELCHANGE:
		switch (id)
		{
		case IDC_P_SWAPP1:
		    scen.swap_players(propdata.pindex, 0);
			LoadPlayer(dialog);
			SendMessage(propdata.mapview, MAP_Reset, 0, 0);
			break;
		case IDC_P_SWAPP2:
		    scen.swap_players(propdata.pindex, 1);
			LoadPlayer(dialog);
			SendMessage(propdata.mapview, MAP_Reset, 0, 0);
			break;
		case IDC_P_SWAPP3:
		    scen.swap_players(propdata.pindex, 2);
			LoadPlayer(dialog);
			SendMessage(propdata.mapview, MAP_Reset, 0, 0);
			break;
		case IDC_P_SWAPP4:
		    scen.swap_players(propdata.pindex, 3);
			LoadPlayer(dialog);
			SendMessage(propdata.mapview, MAP_Reset, 0, 0);
			break;
		case IDC_P_SWAPP5:
		    scen.swap_players(propdata.pindex, 4);
			LoadPlayer(dialog);
			SendMessage(propdata.mapview, MAP_Reset, 0, 0);
			break;
		case IDC_P_SWAPP6:
		    scen.swap_players(propdata.pindex, 5);
			LoadPlayer(dialog);
			SendMessage(propdata.mapview, MAP_Reset, 0, 0);
			break;
		case IDC_P_SWAPP7:
		    scen.swap_players(propdata.pindex, 6);
			LoadPlayer(dialog);
			SendMessage(propdata.mapview, MAP_Reset, 0, 0);
			break;
		case IDC_P_SWAPP8:
		    scen.swap_players(propdata.pindex, 7);
			LoadPlayer(dialog);
			SendMessage(propdata.mapview, MAP_Reset, 0, 0);
			break;
		case IDC_P_SWAPGA:
		    scen.swap_players(propdata.pindex, 8);
			LoadPlayer(dialog);
			SendMessage(propdata.mapview, MAP_Reset, 0, 0);
			break;
		case IDC_P_SP1:
		case IDC_P_SP2:
		case IDC_P_SP3:
		case IDC_P_SP4:
		case IDC_P_SP5:
		case IDC_P_SP6:
		case IDC_P_SP7:
		case IDC_P_SP8:
		case IDC_P_SG:
			SavePlayer(dialog);
			propdata.pindex = id - IDC_P_SP1;
			propdata.p = &scen.players[propdata.pindex];
			LoadPlayer(dialog);
			break;

		case IDC_P_EXAI:
			Players_ManageAI(dialog, false);
			break;

		case IDC_P_EXVC:
			Players_ManageVC(dialog, false);
			break;

		case IDC_P_EXCTY:
			Players_ManageCTY(dialog, false);
			break;

		case IDC_P_IMAI:
			Players_ManageAI(dialog, true);
			//we don't need a full LoadPlayer() for this
			SetDlgItemText(dialog, IDC_P_AI, propdata.p->ai);
			SetDlgItemText(dialog, IDC_P_AISCRIPT, p->aifile.c_str());
		    if (*p->ai)
			    p->aimode = AI_custom;
			SendDlgItemMessage(dialog, IDC_P_AIMODE, BM_SETCHECK, p->aimode, 0);
			SetDlgItemInt(dialog, IDC_P_AIMODE_VAL, p->aimode, FALSE);
			break;

		case IDC_P_IMVC:
			Players_ManageVC(dialog, true);
			//we don't need a full LoadPlayer() for this
			SetDlgItemText(dialog, IDC_P_VC, propdata.p->vc);
			SetDlgItemText(dialog, IDC_P_VCSCRIPT, p->vcfile.c_str());
			break;

		case IDC_P_IMCTY:
			Players_ManageCTY(dialog, true);
			//we don't need a full LoadPlayer() for this
			SetDlgItemText(dialog, IDC_P_CTY, propdata.p->cty);
			SetDlgItemText(dialog, IDC_P_CTYSCRIPT, p->ctyfile.c_str());
			break;

		case IDC_P_CLEARAI:
		    {
		        char *cstr = p->aifile.unlock(1);
	            strcpy(cstr, "");
		        p->aifile.lock();
		        SetDlgItemText(dialog, IDC_P_AISCRIPT, p->aifile.c_str());

                p->aimode = AI_none;

	            SendDlgItemMessage(dialog, IDC_P_AIMODE, BM_SETCHECK, p->aimode == AI_standard, 0);
			    SetDlgItemInt(dialog, IDC_P_AIMODE_VAL, p->aimode, FALSE);

	            strcpy(p->ai, "");
	            SetDlgItemText(dialog, IDC_P_AI, p->ai);
		    }
			break;

		case IDC_P_CLEARVC:
		    {
		        char *cstr = p->vcfile.unlock(1);
	            strcpy(cstr, "");
		        p->vcfile.lock();
		        SetDlgItemText(dialog, IDC_P_VCSCRIPT, p->vcfile.c_str());

	            strcpy(p->vc, "");
	            SetDlgItemText(dialog, IDC_P_VC, p->vc);
		    }
			break;

		case IDC_P_CLEARCTY:
		    {
		        char *cstr = p->ctyfile.unlock(1);
	            strcpy(cstr, "");
		        p->ctyfile.lock();
		        SetDlgItemText(dialog, IDC_P_CTYSCRIPT, p->ctyfile.c_str());

	            strcpy(p->cty, "");
	            SetDlgItemText(dialog, IDC_P_CTY, p->cty);
		    }
			break;

		case IDC_P_RANDOMGAME:
		    {
		        char *cstr = p->aifile.unlock(1);
	            strcpy(cstr, "");
		        p->aifile.lock();
		        SetDlgItemText(dialog, IDC_P_AISCRIPT, p->aifile.c_str());

                p->aimode = AI_standard;

	            SendDlgItemMessage(dialog, IDC_P_AIMODE, BM_SETCHECK, p->aimode == AI_standard, 0);
			    SetDlgItemInt(dialog, IDC_P_AIMODE_VAL, p->aimode, FALSE);

	            strcpy(p->ai, scen.StandardAI);
	            SetDlgItemText(dialog, IDC_P_AI, p->ai);
		    }
			break;

		case IDC_P_PROMISORY:
		    {
		        char *cstr = p->aifile.unlock(1);
	            strcpy(cstr, "");
		        p->aifile.lock();
		        SetDlgItemText(dialog, IDC_P_AISCRIPT, p->aifile.c_str());

                p->aimode = AI_standard;

	            SendDlgItemMessage(dialog, IDC_P_AIMODE, BM_SETCHECK, p->aimode == AI_standard, 0);
			    SetDlgItemInt(dialog, IDC_P_AIMODE_VAL, p->aimode, FALSE);

	            strcpy(p->ai, scen.StandardAI2);
	            SetDlgItemText(dialog, IDC_P_AI, p->ai);
		    }
			break;

		case IDC_P_SPDIP:
			propdata.p->diplomacy[propdata.sel0] = b_to_d[SendDlgItemMessage(dialog, IDC_P_DSTATE, BM_GETCHECK, 0, 0)];
			propdata.sel0 = SendMessage(control, CB_GETCURSEL, 0 ,0);
			SendDlgItemMessage(dialog, IDC_P_DSTATE, BM_SETCHECK, d_to_b[propdata.p->diplomacy[propdata.sel0]], 0);
			break;

		case IDC_P_AIMODE:
			//EnableWindow(GetDlgItem(dialog, IDC_P_AI), SendMessage(control, BM_GETCHECK, 0, 0) == BST_UNCHECKED);
			if (SendMessage(control, BM_GETCHECK, 0, 0) == BST_CHECKED) {
			    p->aimode = AI_standard;
			} else {
			    p->aimode = AI_custom;
			}
			SetDlgItemInt(dialog, IDC_P_AIMODE_VAL, p->aimode, FALSE);
			break;

		case ID_EDIT_COPY:
			SendMessage(GetFocus(), WM_COPY, 0, 0);
			break;

		case ID_EDIT_CUT:
			SendMessage(GetFocus(), WM_CUT, 0, 0);
			break;

		case ID_EDIT_PASTE:
			SendMessage(GetFocus(), WM_PASTE, 0, 0);
			break;
		}
		break;

	case EN_SETFOCUS:
		EnableMenuItem(propdata.menu, ID_EDIT_COPY, MF_ENABLED);
		EnableMenuItem(propdata.menu, ID_EDIT_CUT, MF_ENABLED);
		if (IsClipboardFormatAvailable(CF_TEXT))
			EnableMenuItem(propdata.menu, ID_EDIT_PASTE, MF_ENABLED);
		break;

	case EN_KILLFOCUS:
		EnableMenuItem(propdata.menu, ID_EDIT_COPY, MF_GRAYED);
		EnableMenuItem(propdata.menu, ID_EDIT_CUT, MF_GRAYED);
		EnableMenuItem(propdata.menu, ID_EDIT_PASTE, MF_GRAYED);
		break;
	}
}

char ttAI[] =
"Leave blank to have no AI.";

BOOL Players_Init(HWND dialog)
{
	/* Fill Combo Boxes */
	LCombo_Fill(dialog, IDC_P_CIV, esdata.civs.head());
	Combo_Fill(dialog, IDC_P_SPDIP, Player::names, NUM_PLAYERS);
	SendDlgItemMessage(dialog, IDC_P_SPDIP, CB_SETCURSEL, 0, 0);	//set to player 0, or we get diplomacy[-1]
	LCombo_Fill(dialog, IDC_P_COLOR, esdata.colors.head());
	Combo_PairFill(GetDlgItem(dialog, IDC_P_AGE), NUM_AGES, ages);

	/* Set resource names per game */
	if (scen.game == SWGB) {
	    Window_SetTextW(GetDlgItem(dialog, IDC_P_LBL_FOOD), L"Food:");
	    Window_SetTextW(GetDlgItem(dialog, IDC_P_LBL_WOOD), L"Carbon:");
	    Window_SetTextW(GetDlgItem(dialog, IDC_P_LBL_GOLD), L"Nova:");
	    Window_SetTextW(GetDlgItem(dialog, IDC_P_LBL_STONE), L"Ore:");
	    Window_SetTextW(GetDlgItem(dialog, IDC_P_LBL_OREX), L"Ore X:");
	    Window_SetTextW(GetDlgItem(dialog, IDC_P_LBL_OREY), L"Ore Y:");
	} else {
	    Window_SetTextW(GetDlgItem(dialog, IDC_P_LBL_FOOD), L"Food:");
	    Window_SetTextW(GetDlgItem(dialog, IDC_P_LBL_WOOD), L"Wood:");
	    Window_SetTextW(GetDlgItem(dialog, IDC_P_LBL_GOLD), L"Gold:");
	    Window_SetTextW(GetDlgItem(dialog, IDC_P_LBL_STONE), L"Stone:");
	    Window_SetTextW(GetDlgItem(dialog, IDC_P_LBL_OREX), L"Ore X:");
	    Window_SetTextW(GetDlgItem(dialog, IDC_P_LBL_OREY), L"Ore Y:");
	}

	SendDlgItemMessage(dialog, IDC_P_X, EM_SETLIMITTEXT, 5, 0);
	SendDlgItemMessage(dialog, IDC_P_Y, EM_SETLIMITTEXT, 5, 0);

	AddTool(dialog, IDC_P_AI, ttAI);

	return TRUE;
}

INT_PTR CALLBACK PlyAIDlgProc(HWND dialog, UINT msg, WPARAM wParam, LPARAM lParam)
{
	INT_PTR ret = FALSE;

	try
	{
		switch (msg)
		{
		case WM_INITDIALOG:
			ret = Players_Init(dialog);
			break;

		case WM_COMMAND:
			ret = 0;
			Players_HandleCommand(
				dialog, HIWORD(wParam), LOWORD(wParam), (HWND)lParam);
			break;

		case WM_NOTIFY:
			{
				ret = TRUE;

				NMHDR *header = (NMHDR*)lParam;
				switch (header->code)
				{
				case PSN_SETACTIVE:
					CheckRadioButton(
						dialog, IDC_P_SP1, IDC_P_SG, IDC_P_SP1 + propdata.pindex);
					LoadPlayer(dialog);
					return ret;

				case PSN_KILLACTIVE:
					SavePlayer(dialog);
					break;
				}
			}
			break;

		case AOKTS_Loading:
			ret = Players_Init(dialog);
			CheckRadioButton(
				dialog, IDC_P_SP1, IDC_P_SG, IDC_P_SP1 + propdata.pindex);
			LoadPlayer(dialog);
			return ret;

		case AOKTS_Saving:
			SavePlayer(dialog);
		}
	}
	catch (std::exception& ex)
	{
		// Show a user-friendly message, bug still crash to allow getting all
		// the debugging info.
		unhandledExceptionAlert(dialog, msg, ex);
		throw;
	}

	return ret;
}

INT_PTR CALLBACK PlyCTYDlgProc(HWND dialog, UINT msg, WPARAM wParam, LPARAM lParam)
{
	INT_PTR ret = FALSE;

	try
	{
		switch (msg)
		{
		case WM_INITDIALOG:
			ret = Players_Init(dialog);
			break;

		case WM_COMMAND:
			ret = 0;
			Players_HandleCommand(
				dialog, HIWORD(wParam), LOWORD(wParam), (HWND)lParam);
			break;

		case WM_NOTIFY:
			{
				ret = TRUE;

				NMHDR *header = (NMHDR*)lParam;
				switch (header->code)
				{
				case PSN_SETACTIVE:
					CheckRadioButton(
						dialog, IDC_P_SP1, IDC_P_SG, IDC_P_SP1 + propdata.pindex);
					LoadPlayer(dialog);
					return ret;

				case PSN_KILLACTIVE:
					SavePlayer(dialog);
					break;
				}
			}
			break;

		case AOKTS_Loading:
			ret = Players_Init(dialog);
			CheckRadioButton(
				dialog, IDC_P_SP1, IDC_P_SG, IDC_P_SP1 + propdata.pindex);
			LoadPlayer(dialog);
			return ret;

		case AOKTS_Saving:
			SavePlayer(dialog);
		}
	}
	catch (std::exception& ex)
	{
		// Show a user-friendly message, bug still crash to allow getting all
		// the debugging info.
		unhandledExceptionAlert(dialog, msg, ex);
		throw;
	}

	return ret;
}

INT_PTR CALLBACK PlyVCDlgProc(HWND dialog, UINT msg, WPARAM wParam, LPARAM lParam)
{
	INT_PTR ret = FALSE;

	try
	{
		switch (msg)
		{
		case WM_INITDIALOG:
			ret = Players_Init(dialog);
			break;

		case WM_COMMAND:
			ret = 0;
			Players_HandleCommand(
				dialog, HIWORD(wParam), LOWORD(wParam), (HWND)lParam);
			break;

		case WM_NOTIFY:
			{
				ret = TRUE;

				NMHDR *header = (NMHDR*)lParam;
				switch (header->code)
				{
				case PSN_SETACTIVE:
					CheckRadioButton(
						dialog, IDC_P_SP1, IDC_P_SG, IDC_P_SP1 + propdata.pindex);
					LoadPlayer(dialog);
					return ret;

				case PSN_KILLACTIVE:
					SavePlayer(dialog);
					break;
				}
			}
			break;

		case AOKTS_Loading:
			ret = Players_Init(dialog);
			CheckRadioButton(
				dialog, IDC_P_SP1, IDC_P_SG, IDC_P_SP1 + propdata.pindex);
			LoadPlayer(dialog);
			return ret;

		case AOKTS_Saving:
			SavePlayer(dialog);
		}
	}
	catch (std::exception& ex)
	{
		// Show a user-friendly message, bug still crash to allow getting all
		// the debugging info.
		unhandledExceptionAlert(dialog, msg, ex);
		throw;
	}

	return ret;
}
