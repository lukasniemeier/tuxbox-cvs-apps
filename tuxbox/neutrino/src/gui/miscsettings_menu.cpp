/*
	$Id: miscsettings_menu.cpp,v 1.2 2010/09/12 21:00:06 dbt Exp $

	miscsettings_menu implementation - Neutrino-GUI

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

#include "miscsettings_menu.h"
#include "zapit_setup.h"
#include "filebrowser.h"
#include "keybind_setup.h"

#include <global.h>
#include <neutrino.h>

#include <gui/widget/icons.h>
#include "gui/widget/stringinput.h"

#include <driver/screen_max.h>

#include <system/debug.h>

CMiscMenue::CMiscMenue(const neutrino_locale_t title, const char * const IconName)
{
	frameBuffer = CFrameBuffer::getInstance();

	menue_title = title != NONEXISTANT_LOCALE ? title : LOCALE_MISCSETTINGS_HEAD;
	menue_icon = IconName != NULL ? IconName : NEUTRINO_ICON_SETTINGS;

	width = w_max (500, 100);
	hheight = g_Font[SNeutrinoSettings::FONT_TYPE_MENU_TITLE]->getHeight();
	mheight = g_Font[SNeutrinoSettings::FONT_TYPE_MENU]->getHeight();
	height 	= hheight+13*mheight+ 10;
	x	= getScreenStartX (width);
	y	= getScreenStartY (height);
}

CMiscMenue::~CMiscMenue()
{

}

void CMiscMenue::hide()
{
	frameBuffer->paintBackgroundBoxRel(x,y, width,height);
}


int CMiscMenue::exec(CMenuTarget* parent, const std::string &actionKey)
{
	dprintf(DEBUG_DEBUG, "init extended settings menue\n");

	if(parent != NULL)
		parent->hide();

	if(actionKey == "epgdir")
	{
		CFileBrowser b;
		b.Dir_Mode=true;
		if (b.exec(g_settings.epg_dir.c_str()))
		{
			if((b.getSelectedFile()->Name) == "/")
			{
				// if selected dir is root -> clear epg_dir
				g_settings.epg_dir = "";
			} else {
				g_settings.epg_dir = b.getSelectedFile()->Name + "/";
			}
			CNeutrinoApp::getInstance()->SendSectionsdConfig(); // update notifier
		}
		return menu_return::RETURN_REPAINT;
	}


	showMenue();
	
	return menu_return::RETURN_REPAINT;	
}

#define MESSAGEBOX_NO_YES_OPTION_COUNT 2
const CMenuOptionChooser::keyval MESSAGEBOX_NO_YES_OPTIONS[MESSAGEBOX_NO_YES_OPTION_COUNT] =
{
	{ 0, LOCALE_MESSAGEBOX_NO  },
	{ 1, LOCALE_MESSAGEBOX_YES }
};

#define OPTIONS_OFF1_ON0_OPTION_COUNT 2
const CMenuOptionChooser::keyval OPTIONS_OFF1_ON0_OPTIONS[OPTIONS_OFF1_ON0_OPTION_COUNT] =
{
	{ 1, LOCALE_OPTIONS_OFF },
	{ 0, LOCALE_OPTIONS_ON  }
};

#define MISCSETTINGS_FILESYSTEM_IS_UTF8_OPTION_COUNT 2
const CMenuOptionChooser::keyval MISCSETTINGS_FILESYSTEM_IS_UTF8_OPTIONS[MISCSETTINGS_FILESYSTEM_IS_UTF8_OPTION_COUNT] =
{
	{ 0, LOCALE_FILESYSTEM_IS_UTF8_OPTION_ISO8859_1 },
	{ 1, LOCALE_FILESYSTEM_IS_UTF8_OPTION_UTF8      }
};

#define REMOTE_CONTROL_STANDBY_OFF_WITH_OPTIONS_COUNT 4
const CMenuOptionChooser::keyval  REMOTE_CONTROL_STANDBY_OFF_WITH_OPTIONS[REMOTE_CONTROL_STANDBY_OFF_WITH_OPTIONS_COUNT]=
{
   { 0 , LOCALE_MISCSETTINGS_RC_STANDBY_OFF_WITH_POWER },
   { 1 , LOCALE_MISCSETTINGS_RC_STANDBY_OFF_WITH_POWER_OK },
   { 2 , LOCALE_MISCSETTINGS_RC_STANDBY_OFF_WITH_POWER_HOME },
   { 3 , LOCALE_MISCSETTINGS_RC_STANDBY_OFF_WITH_POWER_HOME_OK }
};

#if !defined(ENABLE_AUDIOPLAYER) && !defined(ENABLE_ESD)
#define MISCSETTINGS_STARTMODE_WITH_OPTIONS_COUNT 5
#else
#if !defined(ENABLE_AUDIOPLAYER) && defined(ENABLE_ESD)
#define MISCSETTINGS_STARTMODE_WITH_OPTIONS_COUNT 6
#else
#if defined(ENABLE_AUDIOPLAYER) && !defined(ENABLE_ESD)
#define MISCSETTINGS_STARTMODE_WITH_OPTIONS_COUNT 7
#else
#define MISCSETTINGS_STARTMODE_WITH_OPTIONS_COUNT 8
#endif
#endif
#endif
const CMenuOptionChooser::keyval  MISCSETTINGS_STARTMODE_WITH_OPTIONS[MISCSETTINGS_STARTMODE_WITH_OPTIONS_COUNT]=
{
   { 0 , LOCALE_MISCSETTINGS_STARTMODE_RESTORE },
   { 1 , LOCALE_MAINMENU_TVMODE },
   { 2 , LOCALE_MAINMENU_RADIOMODE },
   { 3 , LOCALE_MAINMENU_SCARTMODE },
#ifdef ENABLE_AUDIOPLAYER
   { 4 , LOCALE_MAINMENU_AUDIOPLAYER },
   { 5 , LOCALE_INETRADIO_NAME },
#endif
#ifdef ENABLE_ESD
   { 6 , LOCALE_ESOUND_NAME },
#endif
   { 7 , LOCALE_TIMERLIST_TYPE_STANDBY },
};

//show misc settings menue
void CMiscMenue::showMenue()
{
	//misc settings
	CMenuWidget *misc_menue 		= new CMenuWidget(menue_title, menue_icon, width);
	//general
	CMenuWidget *misc_menue_energy 	= new CMenuWidget(LOCALE_MISCSETTINGS_HEAD, menue_icon, width);
	//epg
	CMenuWidget *misc_menue_epg 		= new CMenuWidget(LOCALE_MISCSETTINGS_HEAD, menue_icon, width);
	//filebrowser
	CMenuWidget *misc_menue_filebrowser 	= new CMenuWidget(LOCALE_MISCSETTINGS_HEAD, menue_icon, width);

	//osd main settings, subhead
	if (menue_title != NONEXISTANT_LOCALE)
	{
		CMenuSeparator * misc_menue_subhead = new CMenuSeparator(CMenuSeparator::ALIGN_LEFT | CMenuSeparator::SUB_HEAD | CMenuSeparator::STRING, LOCALE_MISCSETTINGS_HEAD);
		misc_menue->addItem(misc_menue_subhead);
	}

	misc_menue->addItem(GenericMenuSeparator);
	misc_menue->addItem(GenericMenuBack);
	misc_menue->addItem(GenericMenuSeparatorLine);


	// general
	misc_menue->addItem(new CMenuForwarder(LOCALE_MISCSETTINGS_RC_STANDBY_MODES,	true, NULL, misc_menue_energy, NULL,  CRCInput::RC_red, NEUTRINO_ICON_BUTTON_RED));	
	// epg settings
	misc_menue->addItem(new CMenuForwarder(LOCALE_MISCSETTINGS_EPG_HEAD, 		true, NULL, misc_menue_epg, NULL, CRCInput::RC_green, NEUTRINO_ICON_BUTTON_GREEN));
	// zapit settings
	misc_menue->addItem(new CMenuForwarder(LOCALE_ZAPITCONFIG_HEAD, 		true, NULL, new CZapitSetup(LOCALE_MAINSETTINGS_MISC), NULL, CRCInput::RC_yellow, NEUTRINO_ICON_BUTTON_YELLOW));
	// filebrowser
	misc_menue->addItem(new CMenuForwarder(LOCALE_FILEBROWSER_HEAD, 		true, NULL, misc_menue_filebrowser, NULL, CRCInput::RC_blue, NEUTRINO_ICON_BUTTON_BLUE));

	misc_menue->addItem(GenericMenuSeparatorLine);
#ifndef TUXTXT_CFG_STANDALONE
	//tutxt cache
	CTuxtxtCacheNotifier *tuxtxtcacheNotifier = new CTuxtxtCacheNotifier;
	misc_menue->addItem(new CMenuOptionChooser(LOCALE_MISCSETTINGS_TUXTXT_CACHE, &g_settings.tuxtxt_cache, OPTIONS_OFF0_ON1_OPTIONS, OPTIONS_OFF0_ON1_OPTION_COUNT, true, tuxtxtcacheNotifier));
#endif
	// startmode
	misc_menue->addItem(new CMenuOptionChooser(LOCALE_MISCSETTINGS_STARTMODE, &g_settings.startmode, MISCSETTINGS_STARTMODE_WITH_OPTIONS, MISCSETTINGS_STARTMODE_WITH_OPTIONS_COUNT, true));

	
	/* misc settings sub menues */
	// general
	misc_menue_energy->addItem( new CMenuSeparator(CMenuSeparator::ALIGN_LEFT | CMenuSeparator::SUB_HEAD | CMenuSeparator::STRING, LOCALE_MISCSETTINGS_RC_STANDBY_MODES));

	misc_menue_energy->addItem(GenericMenuSeparator);
	misc_menue_energy->addItem(GenericMenuBack);
	misc_menue_energy->addItem(GenericMenuSeparatorLine);



	//rc delay
	CMenuOptionChooser *m1 = new CMenuOptionChooser(LOCALE_MISCSETTINGS_SHUTDOWN_REAL_RCDELAY, &g_settings.shutdown_real_rcdelay, OPTIONS_OFF0_ON1_OPTIONS, OPTIONS_OFF0_ON1_OPTION_COUNT, !g_settings.shutdown_real);
	//standby with...
	CMenuOptionChooser *m5 = new CMenuOptionChooser(LOCALE_MISCSETTINGS_RC_STANDBY_OFF_WITH, &g_settings.standby_off_with, REMOTE_CONTROL_STANDBY_OFF_WITH_OPTIONS, REMOTE_CONTROL_STANDBY_OFF_WITH_OPTIONS_COUNT, !g_settings.shutdown_real);
	CShutdownCountNotifier *shutDownCountNotifier = new CShutdownCountNotifier;
	//shutdown count
	CStringInput * miscSettings_shutdown_count = new CStringInput(LOCALE_MISCSETTINGS_SHUTDOWN_COUNT, g_settings.shutdown_count, 3, LOCALE_MISCSETTINGS_SHUTDOWN_COUNT_HINT1, LOCALE_MISCSETTINGS_SHUTDOWN_COUNT_HINT2, "0123456789 ", shutDownCountNotifier);
	CMenuForwarder *m4 = new CMenuForwarder(LOCALE_MISCSETTINGS_SHUTDOWN_COUNT, !g_settings.shutdown_real, g_settings.shutdown_count, miscSettings_shutdown_count);
	//standby save power
	CMenuOptionChooser *m3 = new CMenuOptionChooser(LOCALE_MISCSETTINGS_STANDBY_SAVE_POWER, &g_settings.standby_save_power, OPTIONS_OFF0_ON1_OPTIONS, OPTIONS_OFF0_ON1_OPTION_COUNT, !g_settings.shutdown_real);
	CMiscNotifier* miscNotifier = new CMiscNotifier(m1, m3, m4, m5);
	//sutdown real
	CMenuOptionChooser *m2 = new CMenuOptionChooser(LOCALE_MISCSETTINGS_SHUTDOWN_REAL, &g_settings.shutdown_real, OPTIONS_OFF1_ON0_OPTIONS, OPTIONS_OFF1_ON0_OPTION_COUNT, true, miscNotifier);

	misc_menue_energy->addItem(m2);
#ifndef HAVE_TRIPLEDRAGON
	/* do not allow TD users to shoot themselves in the foot ;) */
	misc_menue_energy->addItem(m3);
#endif
	misc_menue_energy->addItem(m4);
	misc_menue_energy->addItem(m1);
	misc_menue_energy->addItem(m5);

	//epg settings
	misc_menue_epg->addItem( new CMenuSeparator(CMenuSeparator::ALIGN_LEFT | CMenuSeparator::SUB_HEAD | CMenuSeparator::STRING, LOCALE_MISCSETTINGS_EPG_HEAD));

	misc_menue_epg->addItem(GenericMenuSeparator);
	misc_menue_epg->addItem(GenericMenuBack);
	misc_menue_epg->addItem(GenericMenuSeparatorLine);

	//epg cache ??is this really usefull??
	CSectionsdConfigNotifier* sectionsdConfigNotifier = new CSectionsdConfigNotifier;
	CStringInput * miscSettings_epg_cache = new CStringInput(LOCALE_MISCSETTINGS_EPG_CACHE, &g_settings.epg_cache, 2,LOCALE_MISCSETTINGS_EPG_CACHE_HINT1, LOCALE_MISCSETTINGS_EPG_CACHE_HINT2 , "0123456789 ", sectionsdConfigNotifier);
	misc_menue_epg->addItem(new CMenuForwarder(LOCALE_MISCSETTINGS_EPG_CACHE, true, g_settings.epg_cache, miscSettings_epg_cache));

	//extended epg cache
	CStringInput * miscSettings_epg_extendedcache = new CStringInput(LOCALE_MISCSETTINGS_EPG_EXTENDEDCACHE, &g_settings.epg_extendedcache, 2,LOCALE_MISCSETTINGS_EPG_EXTENDEDCACHE_HINT1, LOCALE_MISCSETTINGS_EPG_EXTENDEDCACHE_HINT2 , "0123456789 ", sectionsdConfigNotifier);
	misc_menue_epg->addItem(new CMenuForwarder(LOCALE_MISCSETTINGS_EPG_EXTENDEDCACHE, true, g_settings.epg_extendedcache, miscSettings_epg_extendedcache));
	//old events
	CStringInput * miscSettings_epg_old_events = new CStringInput(LOCALE_MISCSETTINGS_EPG_OLD_EVENTS, &g_settings.epg_old_events, 2,LOCALE_MISCSETTINGS_EPG_OLD_EVENTS_HINT1, LOCALE_MISCSETTINGS_EPG_OLD_EVENTS_HINT2 , "0123456789 ", sectionsdConfigNotifier);
	misc_menue_epg->addItem(new CMenuForwarder(LOCALE_MISCSETTINGS_EPG_OLD_EVENTS, true, g_settings.epg_old_events, miscSettings_epg_old_events));
	//max epg events
	CStringInput * miscSettings_epg_max_events = new CStringInput(LOCALE_MISCSETTINGS_EPG_MAX_EVENTS, &g_settings.epg_max_events, 5,LOCALE_MISCSETTINGS_EPG_MAX_EVENTS_HINT1, LOCALE_MISCSETTINGS_EPG_MAX_EVENTS_HINT2 , "0123456789 ", sectionsdConfigNotifier);
	misc_menue_epg->addItem(new CMenuForwarder(LOCALE_MISCSETTINGS_EPG_MAX_EVENTS, true, g_settings.epg_max_events, miscSettings_epg_max_events));
	misc_menue_epg->addItem(new CMenuForwarder(LOCALE_MISCSETTINGS_EPG_DIR, true, g_settings.epg_dir, this, "epgdir"));
	
	//filebrowser
	misc_menue_filebrowser->addItem( new CMenuSeparator(CMenuSeparator::ALIGN_LEFT | CMenuSeparator::SUB_HEAD | CMenuSeparator::STRING, LOCALE_FILEBROWSER_HEAD));
	
	misc_menue_filebrowser->addItem(GenericMenuSeparator);
	misc_menue_filebrowser->addItem(GenericMenuBack);
	misc_menue_filebrowser->addItem(GenericMenuSeparatorLine);

	misc_menue_filebrowser->addItem(new CMenuOptionChooser(LOCALE_FILESYSTEM_IS_UTF8            , &g_settings.filesystem_is_utf8            , MISCSETTINGS_FILESYSTEM_IS_UTF8_OPTIONS, MISCSETTINGS_FILESYSTEM_IS_UTF8_OPTION_COUNT, true ));
	misc_menue_filebrowser->addItem(new CMenuOptionChooser(LOCALE_FILEBROWSER_SHOWRIGHTS        , &g_settings.filebrowser_showrights        , MESSAGEBOX_NO_YES_OPTIONS              , MESSAGEBOX_NO_YES_OPTION_COUNT              , true ));
	misc_menue_filebrowser->addItem(new CMenuOptionChooser(LOCALE_FILEBROWSER_DENYDIRECTORYLEAVE, &g_settings.filebrowser_denydirectoryleave, MESSAGEBOX_NO_YES_OPTIONS              , MESSAGEBOX_NO_YES_OPTION_COUNT              , true ));


	misc_menue->exec(NULL, "");
	misc_menue->hide();
	delete misc_menue;
}



