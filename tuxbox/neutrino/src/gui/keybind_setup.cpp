/*
	$Id: keybind_setup.cpp,v 1.4 2010/09/20 10:24:12 dbt Exp $

	keybindings setup implementation - Neutrino-GUI

	Copyright (C) 2001 Steffen Hehn 'McClean'
	and some other guys
	Homepage: http://dbox.cyberphoria.org/

	Copyright (C) 2010 T. Graf 'dbt'
	Homepage: http://www.dbox2-tuning.net/


	License: GPL

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 2 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program; if not, write to the Free Software
	Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

*/

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif


#include "gui/keybind_setup.h"
#include "gui/user_menue_setup.h"

#include <global.h>
#include <neutrino.h>

#include <gui/widget/icons.h>
#include <gui/widget/stringinput.h>
#include <gui/widget/keychooser.h>

#include <driver/screen_max.h>

#include <system/debug.h>


CKeybindSetup::CKeybindSetup(const neutrino_locale_t title, const char * const IconName)
{
	frameBuffer = CFrameBuffer::getInstance();
	
	keySetupNotifier = new CKeySetupNotifier;
	keySetupNotifier->changeNotify(NONEXISTANT_LOCALE, NULL);

	menue_title = title != NONEXISTANT_LOCALE ? title : LOCALE_MAINSETTINGS_KEYBINDING;
	menue_icon = IconName != NULL ? IconName : NEUTRINO_ICON_KEYBINDING;

	width = w_max (500, 100);
	hheight = g_Font[SNeutrinoSettings::FONT_TYPE_MENU_TITLE]->getHeight();
	mheight = g_Font[SNeutrinoSettings::FONT_TYPE_MENU]->getHeight();
	height 	= hheight+13*mheight+ 10;
	x	= getScreenStartX (width);
	y	= getScreenStartY (height);
}

CKeybindSetup::~CKeybindSetup()
{

}

int CKeybindSetup::exec(CMenuTarget* parent, const std::string &/*actionKey*/)
{
	dprintf(DEBUG_DEBUG, "init keybindings setup\n");
	int   res = menu_return::RETURN_REPAINT;

	if (parent)
	{
		parent->hide();
	}

	showSetup();
	
	return res;
}

void CKeybindSetup::hide()
{
	frameBuffer->paintBackgroundBoxRel(x,y, width,height);
}

const neutrino_locale_t keydescription_head[] =
{
	LOCALE_KEYBINDINGMENU_TVRADIOMODE_HEAD,
	LOCALE_KEYBINDINGMENU_PAGEUP_HEAD,
	LOCALE_KEYBINDINGMENU_PAGEDOWN_HEAD,
	LOCALE_KEYBINDINGMENU_CANCEL_HEAD,
	LOCALE_KEYBINDINGMENU_SORT_HEAD,
	LOCALE_EVENTFINDER_HEAD,
	LOCALE_KEYBINDINGMENU_ADDRECORD_HEAD,
	LOCALE_KEYBINDINGMENU_ADDREMIND_HEAD,
	LOCALE_KEYBINDINGMENU_RELOAD_HEAD,
	LOCALE_KEYBINDINGMENU_CHANNELUP_HEAD,
	LOCALE_KEYBINDINGMENU_CHANNELDOWN_HEAD,
	LOCALE_KEYBINDINGMENU_BOUQUETUP_HEAD,
	LOCALE_KEYBINDINGMENU_BOUQUETDOWN_HEAD,
	LOCALE_KEYBINDINGMENU_SUBCHANNELUP_HEAD,
	LOCALE_KEYBINDINGMENU_SUBCHANNELDOWN_HEAD,
	LOCALE_KEYBINDINGMENU_SUBCHANNELTOGGLE_HEAD,
	LOCALE_KEYBINDINGMENU_ZAPHISTORY_HEAD,
	LOCALE_KEYBINDINGMENU_LASTCHANNEL_HEAD
};

const neutrino_locale_t keydescription[] =
{
	LOCALE_KEYBINDINGMENU_TVRADIOMODE,
	LOCALE_KEYBINDINGMENU_PAGEUP,
	LOCALE_KEYBINDINGMENU_PAGEDOWN,
	LOCALE_KEYBINDINGMENU_CANCEL,
	LOCALE_KEYBINDINGMENU_SORT,
	LOCALE_EVENTFINDER_HEAD,
	LOCALE_KEYBINDINGMENU_ADDRECORD,
	LOCALE_KEYBINDINGMENU_ADDREMIND,
	LOCALE_KEYBINDINGMENU_RELOAD,
	LOCALE_KEYBINDINGMENU_CHANNELUP,
	LOCALE_KEYBINDINGMENU_CHANNELDOWN,
	LOCALE_KEYBINDINGMENU_BOUQUETUP,
	LOCALE_KEYBINDINGMENU_BOUQUETDOWN,
	LOCALE_KEYBINDINGMENU_SUBCHANNELUP,
	LOCALE_KEYBINDINGMENU_SUBCHANNELDOWN,
	LOCALE_KEYBINDINGMENU_SUBCHANNELTOGGLE,
	LOCALE_KEYBINDINGMENU_ZAPHISTORY,
	LOCALE_KEYBINDINGMENU_LASTCHANNEL
};

#define KEYBINDINGMENU_BOUQUETHANDLING_OPTION_COUNT 3
const CMenuOptionChooser::keyval KEYBINDINGMENU_BOUQUETHANDLING_OPTIONS[KEYBINDINGMENU_BOUQUETHANDLING_OPTION_COUNT] =
{
	{ 0, LOCALE_KEYBINDINGMENU_BOUQUETCHANNELS_ON_OK },
	{ 1, LOCALE_KEYBINDINGMENU_BOUQUETLIST_ON_OK     },
	{ 2, LOCALE_KEYBINDINGMENU_ALLCHANNELS_ON_OK     }
};

void CKeybindSetup::showSetup()
{
	CMenuWidget * ks = new CMenuWidget(menue_title, menue_icon, width);
	if (menue_title != NONEXISTANT_LOCALE)
	{
		CMenuSeparator * ks_subhead = new CMenuSeparator(CMenuSeparator::ALIGN_LEFT | CMenuSeparator::SUB_HEAD | CMenuSeparator::STRING, LOCALE_MAINSETTINGS_KEYBINDING);
		ks->addItem(ks_subhead);
	}

	//usermenue
	CMenuSeparator * ks_um_sep 	= new CMenuSeparator(CMenuSeparator::LINE | CMenuSeparator::STRING, LOCALE_USERMENU_HEAD);
	CMenuForwarder * ks_um_red 	= new CMenuForwarder(LOCALE_USERMENU_BUTTON_RED, 	true, NULL, 	new CUserMenuSetup(LOCALE_USERMENU_BUTTON_RED,0), 	NULL, CRCInput::RC_red,NEUTRINO_ICON_BUTTON_RED);
	CMenuForwarder * ks_um_green 	= new CMenuForwarder(LOCALE_USERMENU_BUTTON_GREEN, 	true, NULL, 	new CUserMenuSetup(LOCALE_USERMENU_BUTTON_GREEN,1), 	NULL, CRCInput::RC_green,NEUTRINO_ICON_BUTTON_GREEN);
	CMenuForwarder * ks_um_yellow 	= new CMenuForwarder(LOCALE_USERMENU_BUTTON_YELLOW, 	true, NULL, 	new CUserMenuSetup(LOCALE_USERMENU_BUTTON_YELLOW,2), 	NULL, CRCInput::RC_yellow,NEUTRINO_ICON_BUTTON_YELLOW);
	CMenuForwarder * ks_um_blue 	= new CMenuForwarder(LOCALE_USERMENU_BUTTON_BLUE, 	true, NULL, 	new CUserMenuSetup(LOCALE_USERMENU_BUTTON_BLUE,3), 	NULL, CRCInput::RC_blue,NEUTRINO_ICON_BUTTON_BLUE);

	neutrino_msg_t * keyvalue_p[] =
		{
			&g_settings.key_tvradio_mode,
			&g_settings.key_channelList_pageup,
			&g_settings.key_channelList_pagedown,
			&g_settings.key_channelList_cancel,
			&g_settings.key_channelList_sort,
			&g_settings.key_channelList_search,
			&g_settings.key_channelList_addrecord,
			&g_settings.key_channelList_addremind,
			&g_settings.key_channelList_reload,
			&g_settings.key_quickzap_up,
			&g_settings.key_quickzap_down,
			&g_settings.key_bouquet_up,
			&g_settings.key_bouquet_down,
			&g_settings.key_subchannel_up,
			&g_settings.key_subchannel_down,
			&g_settings.key_subchannel_toggle,
			&g_settings.key_zaphistory,
			&g_settings.key_lastchannel
		};

	CKeyChooser * keychooser[MAX_NUM_KEYNAMES];
	for (int i = 0; i < MAX_NUM_KEYNAMES; i++)
		keychooser[i] = new CKeyChooser(keyvalue_p[i], keydescription_head[i], NEUTRINO_ICON_SETTINGS);


	//remote control
	CMenuWidget * ks_rc 		= new CMenuWidget(menue_title, menue_icon, width);
	CMenuForwarder *ks_rc_fw 	= new CMenuForwarder(LOCALE_KEYBINDINGMENU, true, NULL, ks_rc, NULL, CRCInput::RC_setup, NEUTRINO_ICON_BUTTON_DBOX);
	
	CMenuSeparator * ks_rc_subhead 	= new CMenuSeparator(CMenuSeparator::ALIGN_LEFT | CMenuSeparator::SUB_HEAD | CMenuSeparator::STRING, LOCALE_KEYBINDINGMENU);

	CMenuSeparator *ks_rc_sep 				= new CMenuSeparator(CMenuSeparator::LINE | CMenuSeparator::STRING, LOCALE_KEYBINDINGMENU_RC);
	
	CStringInput * keySettings_repeat_genericblocker 	= new CStringInput(LOCALE_KEYBINDINGMENU_REPEATBLOCKGENERIC, g_settings.repeat_genericblocker, 3, LOCALE_REPEATBLOCKER_HINT_1, LOCALE_REPEATBLOCKER_HINT_2, "0123456789 ", keySetupNotifier);
	CStringInput * keySettings_repeatBlocker 		= new CStringInput(LOCALE_KEYBINDINGMENU_REPEATBLOCK, g_settings.repeat_blocker, 3, LOCALE_REPEATBLOCKER_HINT_1, LOCALE_REPEATBLOCKER_HINT_2, "0123456789 ", keySetupNotifier);
	keySetupNotifier->changeNotify(NONEXISTANT_LOCALE, NULL);
	CMenuForwarder *ks_rc_repeat_fw 			= new CMenuForwarder(LOCALE_KEYBINDINGMENU_REPEATBLOCK, true, g_settings.repeat_blocker, keySettings_repeatBlocker);
	CMenuForwarder *ks_rc_repeat_generic_fw 		= new CMenuForwarder(LOCALE_KEYBINDINGMENU_REPEATBLOCKGENERIC, true, g_settings.repeat_genericblocker, keySettings_repeat_genericblocker);

	//mode change
	CMenuSeparator * ks_mc_sep = new CMenuSeparator(CMenuSeparator::LINE | CMenuSeparator::STRING, LOCALE_KEYBINDINGMENU_MODECHANGE);
	CMenuForwarder * ks_mc_fw = new CMenuForwarder(keydescription[VIRTUALKEY_TV_RADIO_MODE], true, NULL, keychooser[VIRTUALKEY_TV_RADIO_MODE]);

	//channellist
	CMenuSeparator * ks_cl_sep = new CMenuSeparator(CMenuSeparator::LINE | CMenuSeparator::STRING, LOCALE_KEYBINDINGMENU_CHANNELLIST);
	CMenuOptionChooser *ks_cl_oj = new CMenuOptionChooser(LOCALE_KEYBINDINGMENU_BOUQUETHANDLING, &g_settings.bouquetlist_mode, KEYBINDINGMENU_BOUQUETHANDLING_OPTIONS, KEYBINDINGMENU_BOUQUETHANDLING_OPTION_COUNT, true );

	//quickzap
	CMenuSeparator * ks_qz_sep = new CMenuSeparator(CMenuSeparator::LINE | CMenuSeparator::STRING, LOCALE_KEYBINDINGMENU_QUICKZAP);
	CMenuForwarder * ks_qz_fw1 = new CMenuForwarder(keydescription[VIRTUALKEY_SUBCHANNEL_TOGGLE], 	true, NULL, keychooser[VIRTUALKEY_SUBCHANNEL_TOGGLE]);
	CMenuForwarder * ks_qz_fw2 = new CMenuForwarder(keydescription[VIRTUALKEY_ZAP_HISTORY], 	true, NULL, keychooser[VIRTUALKEY_ZAP_HISTORY]);
	CMenuForwarder * ks_qz_fw3 = new CMenuForwarder(keydescription[VIRTUALKEY_LASTCHANNEL], 	true, NULL, keychooser[VIRTUALKEY_LASTCHANNEL]);
	

	//paint items
	ks->addItem(GenericMenuSeparator);
	ks->addItem(GenericMenuBack);
	//----------------------------------
	//remote control
	ks->addItem(ks_rc_sep);
	ks->addItem(ks_rc_repeat_fw);
	ks->addItem(ks_rc_repeat_generic_fw);
	//----------------------------------
	//keysetup
	ks->addItem(GenericMenuSeparatorLine);
	ks->addItem(ks_rc_fw);
	//----------------------------------
		ks_rc->addItem(ks_rc_subhead);
		ks_rc->addItem(GenericMenuSeparator);
		ks_rc->addItem(GenericMenuBack);
		//show mode change item
		ks_rc->addItem(ks_mc_sep);
		ks_rc->addItem(ks_mc_fw);
		//----------------------------------
		//show channellist items
		ks_rc->addItem(ks_cl_sep);
		ks_rc->addItem(ks_cl_oj);
		for (int i = VIRTUALKEY_PAGE_UP; i <= VIRTUALKEY_RELOAD; i++)
			ks_rc->addItem(new CMenuForwarder(keydescription[i], true, NULL, keychooser[i]));
		//----------------------------------
		//show quickzap items
		ks_rc->addItem(ks_qz_sep);
		for (int i = VIRTUALKEY_CHANNEL_UP; i <= VIRTUALKEY_SUBCHANNEL_DOWN; i++)
			ks_rc->addItem(new CMenuForwarder(keydescription[i], true, NULL, keychooser[i]));
		ks_rc->addItem(ks_qz_fw1);
		ks_rc->addItem(ks_qz_fw2);
		ks_rc->addItem(ks_qz_fw3);
	//----------------------------------
	//show user menue items
 	ks->addItem(ks_um_sep);
	ks->addItem(ks_um_red);
	ks->addItem(ks_um_green);
	ks->addItem(ks_um_yellow);
	ks->addItem(ks_um_blue);

	ks->exec(NULL, "");
	ks->hide();
	delete ks;
}
