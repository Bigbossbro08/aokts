/**
	AOK Trigger Studio (See aokts.cpp for legal conditions.)
	WINDOWS VERSION.
	mapview.cpp -- Graphical representation of AOK maps

	VIEW/CONTROLLER
**/

#include "mapview.h"

#include "../util/settings.h"
#include "../model/scen.h"
#include "../res/resource.h"
#include "../util/hsv.h"
#include "editors.h"

#include "../util/winugly.h"
#include <math.h>	//all the trig functions
#include <stdio.h>
#include <climits>

/* Brushes (created with window) */
std::vector<std::vector<HBRUSH>> tBrushes;
HBRUSH *pBrushes;
HBRUSH bWhite;

AOKPT trigfocus; // only display triggers that enclose this point

/*
	The mapview window actually draws the map to a bitmap in memory
	and then blits the bitmap to the client area when needed. This
	saves processing when resizing and scrolling the window.
*/

struct Highlight
{
	short x, y;

	struct Highlight *prev, *next;
};

struct MapViewData
{
	Scenario * scen;
	HDC copydc;				//DC used for saving what's under a highlight, etc.
	HBITMAP mapbmp;			//bitmap storage for map
	int bmphsize;
	int bmpvsize;
	int offsetx, offsety;
	int scrollMaxX, scrollMaxY;
    int diamondorsquare;
	HWND statusbar;
	Highlight *highlights;	//LL of the highlighted points
} data;

BOOL SaveToFile(HBITMAP hBitmap3, LPCTSTR lpszFileName)
{
    HDC hDC;
    int iBits;
    WORD wBitCount;
    DWORD dwPaletteSize=0, dwBmBitsSize=0, dwDIBSize=0, dwWritten=0;
    BITMAP Bitmap0;
    BITMAPFILEHEADER bmfHdr;
    BITMAPINFOHEADER bi;
    LPBITMAPINFOHEADER lpbi;
    HANDLE fh, hDib, hPal,hOldPal2=NULL;
    hDC = CreateDC("DISPLAY", NULL, NULL, NULL);
    iBits = GetDeviceCaps(hDC, BITSPIXEL) * GetDeviceCaps(hDC, PLANES);
    DeleteDC(hDC);
    if (iBits <= 1)
        wBitCount = 1;
    else if (iBits <= 4)
        wBitCount = 4;
    else if (iBits <= 8)
        wBitCount = 8;
    else
        wBitCount = 24;
    GetObject(hBitmap3, sizeof(Bitmap0), (LPSTR)&Bitmap0);
    bi.biSize = sizeof(BITMAPINFOHEADER);
    bi.biWidth = Bitmap0.bmWidth;
    bi.biHeight =-Bitmap0.bmHeight;
    bi.biPlanes = 1;
    bi.biBitCount = wBitCount;
    bi.biCompression = BI_RGB;
    bi.biSizeImage = 0;
    bi.biXPelsPerMeter = 0;
    bi.biYPelsPerMeter = 0;
    bi.biClrImportant = 0;
    bi.biClrUsed = 256;
    dwBmBitsSize = ((Bitmap0.bmWidth * wBitCount +31) & ~31) /8
        * Bitmap0.bmHeight;
    hDib = GlobalAlloc(GHND,dwBmBitsSize + dwPaletteSize + sizeof(BITMAPINFOHEADER));
    lpbi = (LPBITMAPINFOHEADER)GlobalLock(hDib);
    *lpbi = bi;

    hPal = GetStockObject(DEFAULT_PALETTE);
    if (hPal)
    {
        hDC = GetDC(NULL);
        hOldPal2 = SelectPalette(hDC, (HPALETTE)hPal, FALSE);
        RealizePalette(hDC);
    }


    GetDIBits(hDC, hBitmap3, 0, (UINT) Bitmap0.bmHeight, (LPSTR)lpbi + sizeof(BITMAPINFOHEADER) + dwPaletteSize, (BITMAPINFO *)lpbi, DIB_RGB_COLORS);

    if (hOldPal2)
    {
        SelectPalette(hDC, (HPALETTE)hOldPal2, TRUE);
        RealizePalette(hDC);
        ReleaseDC(NULL, hDC);
    }

    fh = CreateFile(lpszFileName, GENERIC_WRITE,0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN, NULL);

    if (fh == INVALID_HANDLE_VALUE)
        return FALSE;

    bmfHdr.bfType = 0x4D42; // "BM"
    dwDIBSize = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) + dwPaletteSize + dwBmBitsSize;
    bmfHdr.bfSize = dwDIBSize;
    bmfHdr.bfReserved1 = 0;
    bmfHdr.bfReserved2 = 0;
    bmfHdr.bfOffBits = (DWORD)sizeof(BITMAPFILEHEADER) + (DWORD)sizeof(BITMAPINFOHEADER) + dwPaletteSize;

    WriteFile(fh, (LPSTR)&bmfHdr, sizeof(BITMAPFILEHEADER), &dwWritten, NULL);

    WriteFile(fh, (LPSTR)lpbi, dwDIBSize, &dwWritten, NULL);
    GlobalUnlock(hDib);
    GlobalFree(hDib);
    CloseHandle(fh);
    return TRUE;
}

/* Handles negatives as well */
inline int myround(double n)
{
	return int((n >= 0.0) ? n + 0.5 : n - 0.5);
}

#define pi 3.1415926536

const double sin45 = sin(pi / 4);
const double nsin45 = sin(-pi / 4);
const double root2 = sqrt(2.0);

/* rotates points 45deg to make a diamond from a square */

void rotate(int originx, int originy, int xi, int yi, int &xo, int &yo)
{
    if (data.diamondorsquare) {
        xo = xi * setts.zoom;
        yo = yi * setts.zoom;
        yo = originy * 2 * setts.zoom - yi * setts.zoom - setts.zoom;
    } else {
	/**
	 * This algorithm deserves a bit of explanation. We're basically rotating
	 * the square map by 45deg. to make a diamon as AOK displays it. The
	 * algorithm is as follows:
	 *
	 * 1) We're rotating about the center, so translate points to be relative
	 *    to center, not bottom-left corner.
	 */

	/**
	 * 2) Perform the actual rotation:
	 *
	 *    Let x, y be initial coordinates, x2, y2 be rotated coordinates, and
	 *    let (r,theta) be the initital coordinates in polar form.
	 *
	 *    y2 = r sin (theta + 45deg)
	 *       = r (sin theta * cos 45deg + cos theta * sin 45deg)
	 *       = y cos 45deg + x sin 45deg
	 *    y2 = (y + x) sin 45deg
	 *
	 *    x2 = r cos (theta * 45deg)
	 *       = r (cos theta * cos 45deg - sin theta * sin 45deg)
	 *       = x cos 45deg - y sin 45deg
	 *    x2 = (x - y) sin 45deg
	 *
	 *    (Due to sine and cosine addition identities.)
	 *
	 * 3) Translate to coordinates with (0,0) as left corner of diamond, and
	 *    scale by root2 so that new horizontal and vertical axes
	 *    (cross-sections of diamond) are in whole pixels.
	 *
	 *    (Note that for y, /down/ is positive.)
	 *
	 *    y3 = (y2 + origin * root2) * root2
	 *    x3 = (x2 + origin * root2) * root2
	 */
        xo = myround((xi + yi) * sin45 * root2);
        yo = myround((xi - yi) * sin45 * root2) + originy * 2;
	    xo *= setts.zoom;
	    yo *= setts.zoom;
	    yo /=2;
	}
}

void unrotate(int originx, int originy, int xi, int yi, unsigned &xo, unsigned &yo)
{
    if (data.diamondorsquare) {
        xo = xi / setts.zoom;
        yo = yi / setts.zoom;
        yo = ((originy * 2 * setts.zoom - yi)) / setts.zoom;
    } else {
        yi *=2;
        xi /= setts.zoom;
	    yi /= setts.zoom;
	    yi -= originy * 2;

        xo = -myround((xi + yi) * nsin45 / root2);
        yo = -myround((xi - yi) * nsin45 / root2);
    }
}

void PaintUnits(HDC dc)
{
	using std::vector;

	int rx, ry;
	int half = max(data.scen->map.x, data.scen->map.y) / 2;
	RECT area;

	for (int i = 0; i < 9; i++)
	{
	    if (setts.drawplayer[i]) {
		    Player& p = data.scen->players[i];

		    for (vector<Unit>::const_iterator iter = p.units.begin();
			    iter != p.units.end(); ++iter)
		    {
			    rotate(data.scen->map.x/2, data.scen->map.y/2, (int)iter->x, (int)iter->y, rx, ry);
			    area.left = rx;
			    area.right = rx + setts.zoom;
			    area.top = ry;
			    area.bottom = ry + setts.zoom;
			    FrameRect(dc, &area, pBrushes[p.color]);
		    }
		}
	}

	/* TODO: Paint GAIA units here
	p = data.scen->players + GAIA_INDEX;
	parse = p->units.first();
	count = p->units.count();
	SelectObject(dc, bWhite);

	while (count--)
	{
		parse++;
	}
	*/

	/* Highlighting */
	for (Highlight *hparse = data.highlights; hparse; hparse = hparse->next)
	{
		rotate(data.scen->map.x/2, data.scen->map.y/2, hparse->x, hparse->y, rx, ry);
		area.left = rx;
		area.right = rx + setts.zoom;
		area.top = ry;
		area.bottom = ry + setts.zoom;
		FillRect(dc, &area, bWhite);
	}
}

void PaintTriggers(HDC dc)
{
	using std::vector;

	int rx, ry;
	int half = max(data.scen->map.x, data.scen->map.y) / 2;
	RECT area;

    // each triggers
    // (left, bottom) - (right, top)
	for (vector<Trigger>::iterator trig = scen.triggers.begin(); trig != scen.triggers.end(); ++trig) {
	    // each effect
	    for (vector<Effect>::iterator iter = trig->effects.begin(); iter != trig->effects.end(); ++iter) {
	        if (iter->area.left >=0 && iter->area.right >= iter->area.left) {
			    if (setts.draweffects) {
			        rotate(data.scen->map.x/2, data.scen->map.y/2, (int)iter->area.left, (int)iter->area.top, rx, ry);
			        area.left = rx + setts.zoom / 4;
			        area.bottom = ry + 3 * setts.zoom / 4;
			        area.top = ry + setts.zoom / 4;
			        area.right = rx + 3 * setts.zoom / 4;
			        FrameRect(dc, &area, pBrushes[scen.players[0].color]);
			        MoveToEx(dc, rx + setts.zoom / 2, ry + setts.zoom / 2, (LPPOINT) NULL);
			        rotate(data.scen->map.x/2, data.scen->map.y/2, (int)iter->area.right, (int)iter->area.bottom, rx, ry);
			        area.left = rx + setts.zoom / 4;
			        area.bottom = ry + 3 * setts.zoom / 4;
			        area.top = ry + setts.zoom / 4;
			        area.right = rx + 3 * setts.zoom / 4;
			        FrameRect(dc, &area, pBrushes[scen.players[1].color]);
			        LineTo(dc, rx + setts.zoom / 2, ry + setts.zoom / 2);
			        StrokeAndFillPath(dc);
			        rotate(data.scen->map.x/2, data.scen->map.y/2, (int)iter->location.x, (int)iter->location.y, rx, ry);
			        area.left = rx + setts.zoom / 6;
			        area.bottom = ry + 5 * setts.zoom / 6;
			        area.top = ry + setts.zoom / 6;
			        area.right = rx + 5 * setts.zoom / 6;
			        FrameRect(dc, &area, pBrushes[scen.players[2].color]);
			    }
	        }
	    }
	    // conditions
	    for (vector<Condition>::iterator iter = trig->conds.begin(); iter != trig->conds.end(); ++iter) {
	        if (iter->area.left >=0 && iter->area.right >= iter->area.left) {
			    if (setts.drawconds) {
			        rotate(data.scen->map.x/2, data.scen->map.y/2, (int)iter->area.left, (int)iter->area.top, rx, ry);
			        area.left = rx + setts.zoom / 8;
			        area.bottom = ry + 7 * setts.zoom / 8;
			        area.top = ry + setts.zoom / 8;
			        area.right = rx + 7 * setts.zoom / 8;
			        FrameRect(dc, &area, pBrushes[scen.players[3].color]);
			        MoveToEx(dc, rx + setts.zoom / 2, ry + setts.zoom / 2, (LPPOINT) NULL);
			        rotate(data.scen->map.x/2, data.scen->map.y/2, (int)iter->area.right, (int)iter->area.bottom, rx, ry);
			        area.left = rx + setts.zoom / 8;
			        area.bottom = ry + 7 * setts.zoom / 8;
			        area.top = ry + setts.zoom / 8;
			        area.right = rx + 7 * setts.zoom / 8;
			        FrameRect(dc, &area, pBrushes[scen.players[4].color]);
			        LineTo(dc, rx + setts.zoom / 2, ry + setts.zoom / 2);
			        StrokeAndFillPath(dc);
			    }
	        }
        }
	}
}

void PaintMap(HDC dcdest)
{
	int half,full;
	unsigned y, x;
	int ry, rx;	//rotated
	Map::Terrain *parse;
	RECT area;

	/* Create a bitmap */
	full = max(data.scen->map.x, data.scen->map.y);
	data.bmphsize = (2 * full + 1) * setts.zoom;
	data.bmpvsize = (full + 1) * setts.zoom;
	data.mapbmp = CreateCompatibleBitmap(dcdest, data.bmphsize, data.bmpvsize);
	half = full / 2;

	/* Do the painting. */
	SelectObject(data.copydc, data.mapbmp);
	for (x = 0; x < data.scen->map.x; x++)
	{
		parse = data.scen->map.terrain[x];
		for (y = 0; y < data.scen->map.y; y++, parse++)
		{
			rotate(data.scen->map.x/2, data.scen->map.y/2, x, y, rx, ry);
			area.left = rx;
			area.right = rx + setts.zoom;
			area.top = ry;
			area.bottom = ry + setts.zoom;
			FillRect(data.copydc, &area, tBrushes.at(parse->cnst).at(parse->elev));
		}
	}

	PaintUnits(data.copydc);
	PaintTriggers(data.copydc);
}

POINT CalculateMinSize(HWND mapview)
{
	// HACK: who knows how big the border really is
	const int bordersize = 18;
	RECT statusrect;
	GetWindowRect(data.statusbar, &statusrect);

	// If scenario is silly small, we still want a resonable size.
	int scenx = max(data.scen->map.x, 50);
	int sceny = max(data.scen->map.y, 50);

	POINT minsize = {
		scenx * 2 + bordersize,
		sceny * 2 + (statusrect.bottom - statusrect.top) + 10 + bordersize
	};

	return minsize;
}

/* resizes the scrollbars when window is reset or resized */
void UpdateScrollbars(HWND window, int width, int height)
{
	SCROLLINFO si;
	RECT statusSize;

	/* common init */
	si.cbSize = sizeof(si);
	si.fMask = SIF_RANGE | SIF_POS | SIF_PAGE;
	si.nMin = 0;
	si.nPos = 0;

	/* HACK: statusbar size */
	GetWindowRect(data.statusbar, &statusSize);

	/* horizontal scrollbar */
	si.nMax = data.scrollMaxX = data.bmphsize;
	si.nPage = si.nMax / 10;
	SetScrollInfo(window, SB_HORZ, &si, TRUE);

	/* vertical scrollbar */
	si.nMax = data.scrollMaxY = data.bmpvsize;	//big hack here
	si.nPage = si.nMax / 10;
	SetScrollInfo(window, SB_VERT, &si, TRUE);
}

/* Refresh: redrawing the map, not just blitting */
void Refresh(HWND window, BOOL erase)
{
	HDC dc;
	RECT wndSize;

	/* actually redraw the map */
	dc = GetDC(window);
	if (data.mapbmp)
		DeleteObject(data.mapbmp);
	PaintMap(dc);
	ReleaseDC(window, dc);

	/* if we're doing a hard reset (erasing), update scroll bars too */
	if (erase)
	{
		GetClientRect(window, &wndSize);
		UpdateScrollbars(window, wndSize.right - wndSize.left, wndSize.bottom - wndSize.top);
	}

	/* now repaint the window */
	InvalidateRect(window, NULL, erase);
}

void HandlePaint(HWND window)
{
	HDC dc;
	PAINTSTRUCT ps;
	SCROLLINFO si;

	if (GetUpdateRect(window, NULL, FALSE))
	{
		/* Init */
		dc = BeginPaint(window, &ps);
		si.cbSize = sizeof(si);
		si.fMask = SIF_POS;

		/* scrolling offset */
		GetScrollInfo(window, SB_HORZ, &si);
		data.offsetx = (data.scrollMaxX > 0) ? -si.nPos : 0;
		GetScrollInfo(window, SB_VERT, &si);
		data.offsety = (data.scrollMaxY > 0) ? -si.nPos : 0;

		SelectObject(data.copydc, data.mapbmp);
		BitBlt(dc, data.offsetx, data.offsety, data.bmphsize, data.bmpvsize,
			data.copydc, 0, 0, SRCCOPY);

		/* Cleanup */
		EndPaint(window, &ps);
	}
}

void UnhighlightPoint(HWND window, Highlight *h)
{
	/* Remove the link */
	if (h == data.highlights)
		data.highlights = h->next;

	else if (h->prev)
		h->prev->next = h->next;

	if (h->next)
		h->next->prev = h->prev;

	delete h;

	Refresh(window, FALSE);
}

void UnhighlightPoint(HWND window, int x, int y)
{
	Highlight *parse, *next;

	if (x >= 0 && y >= 0)
	{
		/* Find the right highlight struct */
		for (parse = data.highlights; parse; parse = parse->next)
		{
			if (parse->x == x && parse->y == y)
				break;
		}

		/* Did we find one? */
		if (!parse)
			return;

		UnhighlightPoint(window, parse);
	}
	else
	{
		for (parse = data.highlights; parse; parse = next)
		{
			next = parse->next;	//need to do this becuase Unhighlight deletes parse
			UnhighlightPoint(window, parse);
		}
	}
}

void HighlightPoint(HWND window, int x, int y)
{
	Highlight *nh;

	if (x < 0 || x > SHRT_MAX ||
		y < 0 || y > SHRT_MAX)
		return;

	nh = new Highlight;
	if (!nh)
		return;

	/* Prepend the new link since we don't keep track of the last one. */
	nh->prev = NULL;
	nh->next = data.highlights;
	if (data.highlights)
		data.highlights->prev = nh;
	data.highlights = nh;

	/* Set up new highlight. Short bounds checked above */
	nh->x = static_cast<short>(x);
	nh->y = static_cast<short>(y);

	Refresh(window, FALSE);
}

#define NUM_SB_SIZES 2
int sb_sizes[NUM_SB_SIZES] =
{ 360, -1 };
const char *sb_msg = "R refreshes, Space toggles diamond, -= zooms, S saves bitmap, O reset";

HWND makestatus(HWND parent)
{
	HWND ret;
	INITCOMMONCONTROLSEX iccx;

	iccx.dwSize = sizeof(iccx);
	iccx.dwICC = ICC_WIN95_CLASSES;

	InitCommonControlsEx(&iccx);

	ret = CreateWindow(STATUSCLASSNAME, sb_msg,
		WS_CHILD | WS_VISIBLE,
		CW_USEDEFAULT, 0, CW_USEDEFAULT, 0,
		parent,
		(HMENU)100,	//status bar uses menu parameter as ID
		NULL, NULL);

	if (ret)
		ScrollBar_SetParts(ret, NUM_SB_SIZES, sb_sizes);

	return ret;
}

void OnWM_Create(HWND window, CREATESTRUCT * cs)
{
	ColorLink *parse;
	int i, j;
	HDC dc;
	hsv_t * hsv;
    COLORREF tmp;

	/* init window data */
	data.scen = static_cast<Scenario *>(cs->lpCreateParams);
	data.mapbmp = NULL;
	data.bmphsize = 0;
	data.bmpvsize = 0;
	data.diamondorsquare = -1;
	data.statusbar = NULL;
	data.highlights = NULL;

	tBrushes.reserve(esdata.getCount(ESD_terrains));

	for (i = 0, parse = esdata.terrains.head(); parse; parse = (ColorLink*)parse->next(), i++)
	{
	    tBrushes.push_back(std::vector<HBRUSH>());
	    tBrushes[i].reserve(256);
	    // this messes up the memory when 256
	    // need to reduce to 200 for aokts and 190 for swgbts because
	    // there are about 10 more terrains
	    for (j = 0; j < 190; j++)
	    {
	        hsv = new hsv_t();
            rgb2hsv(parse->ref, hsv);
            hsv->value /= 2;
            hsv->value += j * 256 / 16;
            tmp = hsv2rgb(hsv);
		    //tBrushes[i].push_back(CreateSolidBrush(tmp));
		    tBrushes[i].push_back(CreateSolidBrush(tmp));
	    }
	}

	pBrushes = new HBRUSH[esdata.getCount(ESD_colors)];

	for (i = 0, parse = esdata.colors.head();
		parse;
		parse = (ColorLink*)parse->next(), i++)
	{
		pBrushes[i] = CreateSolidBrush(parse->ref);
	}

	bWhite = CreateSolidBrush(0xFFFFFF);
	data.statusbar = makestatus(window);

	dc = GetWindowDC(window);
	data.copydc = CreateCompatibleDC(dc);
	ReleaseDC(window, dc);

	POINT size = CalculateMinSize(window);
	SetWindowPos(window, NULL, 0, 0, size.x, size.y,
		SWP_NOMOVE | SWP_NOZORDER);
}

void OnWM_MOUSEMOVE(HWND, int x, int y)
{
	const size_t BUFSIZE = 20;

	char text[BUFSIZE] = "Outside boundaries";
	unsigned rx, ry;	//un-rotated

	x -= data.offsetx;
	y -= data.offsety;
	unrotate(data.scen->map.x/2, data.scen->map.y/2, x, y, rx, ry);

	if (rx >= 0 && rx < data.scen->map.x && ry >= 0 && ry < data.scen->map.y)
		sprintf(text, "%d, %d", rx, ry);

	ScrollBar_SetText(data.statusbar, 1, text);
}

void OnWM_LBUTTONUP(HWND window, int x, int y)
{
	HWND owner = GetWindow(window, GW_OWNER);
	unsigned rx, ry;	//un-rotated

	unrotate(data.scen->map.x/2, data.scen->map.y/2, x - data.offsetx, y - data.offsety, rx, ry);

	if (rx >= 0 && rx < data.scen->map.x && ry >= 0 && ry < data.scen->map.y)
		SendMessage(owner, MAP_Click, 0, MAKELPARAM(rx, ry));
}

void OnWM_HSCROLL(HWND window, WORD type, WORD pos)
{
	SCROLLINFO si;
	int newpos;

	/* get the current scroll info */
	si.cbSize = sizeof(si);
	si.fMask = SIF_POS | SIF_RANGE;
	GetScrollInfo(window, SB_HORZ, &si);

	/* set the new position and update window */
	switch (type)
	{
	case SB_LEFT:
		newpos = si.nMin;
		break;

	case SB_RIGHT:
		newpos = si.nMax;
		break;

	case SB_LINELEFT:
		newpos = si.nPos - 1;
		break;

	case SB_LINERIGHT:
		newpos = si.nPos + 1;
		break;

	case SB_THUMBPOSITION:
	case SB_THUMBTRACK:
		newpos = pos;
		break;

	default:
		newpos = si.nPos;
	}

	InvalidateRect(window, NULL, FALSE);

	si.nPos = newpos;
	si.fMask = SIF_POS;
	SetScrollInfo(window, SB_HORZ, &si, TRUE);

}

void OnWM_VSCROLL(HWND window, WORD type, WORD pos)
{
	SCROLLINFO si;
	int newpos;

	/* get the current scroll info */
	si.cbSize = sizeof(si);
	si.fMask = SIF_POS | SIF_RANGE;
	GetScrollInfo(window, SB_VERT, &si);

	/* set the new position and update window */
	switch (type)
	{
	case SB_TOP:
		newpos = si.nMin;
		break;

	case SB_BOTTOM:
		newpos = si.nMax;
		break;

	case SB_LINEUP:
		newpos = si.nPos - 1;
		break;

	case SB_LINEDOWN:
		newpos = si.nPos + 1;
		break;

	case SB_THUMBPOSITION:
	case SB_THUMBTRACK:
		newpos = pos;
		break;

	default:
		newpos = si.nPos;
	}

	InvalidateRect(window, NULL, FALSE);

	si.nPos = newpos;
	si.fMask = SIF_POS;
	SetScrollInfo(window, SB_VERT, &si, TRUE);
}

void OnMAP_Reset(HWND mapview, bool resize)
{
	if (resize)
	{
		RECT cursize;
		GetWindowRect(mapview, &cursize);
		POINT minsize = CalculateMinSize(mapview);

		if (minsize.y > (cursize.bottom - cursize.top) ||
			minsize.x > (cursize.right - cursize.left))
		{
			// I wrote less-terrible functions than SetWindowPos() in high
			// school.
			SetWindowPos(mapview, NULL, 0, 0, minsize.x, minsize.y,
				SWP_NOMOVE | SWP_NOZORDER);
		}
	}

	// Erase everything and re-draw
	// This is used for things such as duplicating units and terrain.
	// therefore don't reset scrollbars
	Refresh(mapview, FALSE);
}

LRESULT CALLBACK MapWndProc(HWND window, UINT msg, WPARAM wParam, LPARAM lParam)
{
	LRESULT ret = 0;
	int i;

	switch (msg)
	{
	case WM_CREATE:
		ret = 0;
		OnWM_Create(window, (CREATESTRUCT*)lParam);
		break;

	case WM_PAINT:
		ret = 0;
		HandlePaint(window);
		break;

	case WM_CLOSE:
		SendMessage(GetWindow(window, GW_OWNER), MAP_Close, 0, 0);
		DestroyWindow(window);
		break;

	case WM_DESTROY:
		ret = 0;
		for (i = 0; i < esdata.getCount(ESD_colors); i++)
			DeleteObject(pBrushes[i]);
		delete [] pBrushes;
		DeleteDC(data.copydc);
		break;

	case WM_MOUSEMOVE:
		OnWM_MOUSEMOVE(window, LOWORD(lParam), HIWORD(lParam));
		break;

	case WM_LBUTTONUP:
		OnWM_LBUTTONUP(window, LOWORD(lParam), HIWORD(lParam));
		break;

	// Virtual-key codes
	case WM_KEYDOWN:
	{
		switch (wParam)
		{
		case VK_SPACE:
            data.diamondorsquare = !data.diamondorsquare;
		    Refresh(window, FALSE);
		    break;
		case 0x31: // 1 key
		    setts.drawplayer[0] = !setts.drawplayer[0];
		    Refresh(window, FALSE);
		    break;
		case 0x32: // 2 key
		    setts.drawplayer[1] = !setts.drawplayer[1];
		    Refresh(window, FALSE);
		    break;
		case 0x33: // 3 key
		    setts.drawplayer[2] = !setts.drawplayer[2];
		    Refresh(window, FALSE);
		    break;
		case 0x34: // 4 key
		    setts.drawplayer[3] = !setts.drawplayer[3];
		    Refresh(window, FALSE);
		    break;
		case 0x35: // 5 key
		    setts.drawplayer[4] = !setts.drawplayer[4];
		    Refresh(window, FALSE);
		    break;
		case 0x36: // 6 key
		    setts.drawplayer[5] = !setts.drawplayer[5];
		    Refresh(window, FALSE);
		    break;
		case 0x37: // 7 key
		    setts.drawplayer[6] = !setts.drawplayer[6];
		    Refresh(window, FALSE);
		    break;
		case 0x38: // 8 key
		    setts.drawplayer[7] = !setts.drawplayer[7];
		    Refresh(window, FALSE);
		    break;
		case 0x47: // G key
		    setts.drawplayer[8] = !setts.drawplayer[8];
		    Refresh(window, FALSE);
		    break;
		case 0x53: // S key
	        SaveToFile(data.mapbmp, "preview.bmp");
	        break;
		case 0x4F: // O key
		    Refresh(window, TRUE);
		    break;
		case 0x52: // R key
		    Refresh(window, FALSE);
		    break;
		case 0x45: // E key
		    setts.draweffects = !setts.draweffects;
		    Refresh(window, FALSE);
		    break;
		case 0x43: // C key
		    setts.drawconds = !setts.drawconds;
		    Refresh(window, FALSE);
		    break;
		case VK_OEM_PLUS:
		    if (setts.zoom < 15) {
		        setts.zoom+=1;
		        Refresh(window, FALSE);
		    }
		    break;
		case VK_OEM_MINUS:
		    if (setts.zoom > 1) {
		        setts.zoom-=1;
		        Refresh(window, FALSE);
		    }
		    break;
		}
		break;
	}

	case WM_SIZE:
		UpdateScrollbars(window, LOWORD(lParam), HIWORD(lParam));
		SendMessage(data.statusbar, WM_SIZE, 0, 0);
		break;

	case WM_HSCROLL:
		OnWM_HSCROLL(window, LOWORD(wParam), HIWORD(wParam));
		break;

	case WM_VSCROLL:
		OnWM_VSCROLL(window, LOWORD(wParam), HIWORD(wParam));
		break;

	case WM_NOTIFY:
		{
			const NMHDR *header = (NMHDR*)lParam;
			if (header->hwndFrom == data.statusbar && header->code == NM_CLICK) {
				Refresh(window, TRUE);
			}
		}
		break;

	case MAP_HighlightPoint:
		HighlightPoint(window, wParam, lParam);
		break;

	case MAP_UnhighlightPoint:
		UnhighlightPoint(window, wParam, lParam);
		break;

	case MAP_Reset:
		OnMAP_Reset(window, wParam != 0);
		break;

	default:
		ret = DefWindowProc(window, msg, wParam, lParam);
	}

	return ret;
}

void MakeMapClass(HINSTANCE instance)
{
	WNDCLASS wc;

	wc.style       = CS_HREDRAW | CS_VREDRAW;
//	wc.style       = 0;
	wc.lpfnWndProc = MapWndProc;
	wc.cbClsExtra  = 0;
	wc.cbWndExtra  = 0;
	wc.hInstance   = instance;
	wc.hIcon       = NULL;
	wc.hCursor =
		(HCURSOR)LoadImage(NULL, MAKEINTRESOURCE(OCR_NORMAL), IMAGE_CURSOR, 0, 0, LR_SHARED);
	wc.hbrBackground = (HBRUSH)(COLOR_BACKGROUND);
	wc.lpszMenuName = NULL;
	wc.lpszClassName = MapClass;

	RegisterClass(&wc);
}

HWND CreateMapView(HWND owner, int x, int y, Scenario * scenario)
{
	static bool registered = false;

	// WTF is this for? Yeah, I've read the docs, but the question remains.
	HINSTANCE instance = GetModuleHandle(NULL);

	if (!registered)
	{
		MakeMapClass(instance);
		registered = true;
	}

	HWND mapview = CreateWindowEx(WS_EX_TOOLWINDOW, MapClass, "Map Viewer",
		WS_OVERLAPPEDWINDOW | WS_HSCROLL | WS_VSCROLL,
		x, y,
		0, 0,   // width, height are calculated in OnWM_CREATE
		owner, NULL, instance, scenario);

	return mapview;
}
