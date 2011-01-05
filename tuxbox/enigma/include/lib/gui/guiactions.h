#ifndef __src__lib__gui__guiactions_h__
#define __src__lib__gui__guiactions_h__

#include <lib/system/init.h>
#include <lib/base/i18n.h>
#include <lib/gui/actions.h>

struct cursorActions
{
	eActionMap map;
	eAction up, down, left, right, insertchar, deletechar, capslock, ok, cancel, help;
	cursorActions():
		map("cursor", "Cursor"),
		up(map, "up", 0, eAction::prioWidget),
		down(map, "down", 0, eAction::prioWidget),
		left(map, "left", 0, eAction::prioWidget),
		right(map, "right", 0, eAction::prioWidget),
		insertchar(map, "insertchar", 0, eAction::prioWidget),
		deletechar(map, "deletechar", 0, eAction::prioWidget),
		capslock(map, "capslock", 0, eAction::prioWidget),
		ok(map, "ok", 0, eAction::prioWidget),
		cancel(map, "cancel", 0, eAction::prioDialog),
		help(map, "help", 0, eAction::prioGlobal)
	{
	}
};

extern eAutoInitP0<cursorActions> i_cursorActions;

struct focusActions
{
	eActionMap map;
	eAction up, down, left, right;
	focusActions(): 
		map("focus", "Focus"),
		up(map, "up", 0, eAction::prioGlobal),
		down(map, "down", 0, eAction::prioGlobal),
		left(map, "left", 0, eAction::prioGlobal),
		right(map, "right", 0, eAction::prioGlobal)
	{
	}
};

extern eAutoInitP0<focusActions> i_focusActions;

struct listActions
{
	eActionMap map;
	eAction pageup, pagedown;
	listActions():
		map("list", "Listen"),
		pageup(map, "pageup", 0, eAction::prioWidget+1),
		pagedown(map, "pagedown", 0, eAction::prioWidget+1)
	{
	}
};

extern eAutoInitP0<listActions> i_listActions;

struct shortcutActions
{
	eActionMap map;
	eAction number0, number1, number2, number3, number4,
			number5, number6, number7, number8, number9, 
			red, green, yellow, blue, menu, escape;
	shortcutActions():
		map("shortcut", "Shortcuts"),
		number0(map, "0", 0, eAction::prioGlobal),
		number1(map, "1", 0, eAction::prioGlobal),
		number2(map, "2", 0, eAction::prioGlobal),
		number3(map, "3", 0, eAction::prioGlobal),
		number4(map, "4", 0, eAction::prioGlobal),
		number5(map, "5", 0, eAction::prioGlobal),
		number6(map, "6", 0, eAction::prioGlobal),
		number7(map, "7", 0, eAction::prioGlobal),
		number8(map, "8", 0, eAction::prioGlobal),
		number9(map, "9", 0, eAction::prioGlobal),
		red(map, "red", 0, eAction::prioGlobal),
		green(map, "green", 0, eAction::prioGlobal),
		yellow(map, "yellow", 0, eAction::prioGlobal),
		blue(map, "blue", 0, eAction::prioGlobal),
		menu(map, "menu", 0, eAction::prioGlobal),
		escape(map, "escape", 0, eAction::prioGlobal)
	{
	}
};

extern eAutoInitP0<shortcutActions> i_shortcutActions;


#endif
