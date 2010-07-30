/*
	$Id: driver_boot_setup.cpp,v 1.2 2010/07/30 20:59:09 dbt Exp $

	driver_boot_setup implementation - Neutrino-GUI

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


#include "gui/driver_boot_setup.h"

#include <global.h>
#include <neutrino.h>

#include <gui/widget/icons.h>

#include <driver/screen_max.h>

#include <system/debug.h>

CDriverBootSetup::CDriverBootSetup(const neutrino_locale_t title, const char * const IconName)
{
	frameBuffer = CFrameBuffer::getInstance();

	menue_title = title != NONEXISTANT_LOCALE ? title : LOCALE_DRIVERSETTINGS_DRIVER_BOOT;
	menue_icon = IconName != NULL ? IconName : NEUTRINO_ICON_SETTINGS;

	width = w_max (500, 100);
	hheight = g_Font[SNeutrinoSettings::FONT_TYPE_MENU_TITLE]->getHeight();
	mheight = g_Font[SNeutrinoSettings::FONT_TYPE_MENU]->getHeight();
	height 	= hheight+13*mheight+ 10;
	x	= getScreenStartX (width);
	y	= getScreenStartY (height);
}

CDriverBootSetup::~CDriverBootSetup()
{

}

void CDriverBootSetup::hide()
{
	frameBuffer->paintBackgroundBoxRel(x,y, width,height);
}


int CDriverBootSetup::exec(CMenuTarget* parent, const std::string &)
{
	dprintf(DEBUG_DEBUG, "init driversettings\n");
	if(parent != NULL)
		parent->hide();

	showSetup();
	
	return menu_return::RETURN_REPAINT;	
}

#define OPTIONS_OFF1_ON0_OPTION_COUNT 2
const CMenuOptionChooser::keyval OPTIONS_OFF1_ON0_OPTIONS[OPTIONS_OFF1_ON0_OPTION_COUNT] =
{
	{ 1, LOCALE_OPTIONS_OFF },
	{ 0, LOCALE_OPTIONS_ON  }
};

#define DRIVERSETTINGS_FB_DESTINATION_OPTION_COUNT 3
const CMenuOptionChooser::keyval DRIVERSETTINGS_FB_DESTINATION_OPTIONS[DRIVERSETTINGS_FB_DESTINATION_OPTION_COUNT] =
{
	{ 0, LOCALE_OPTIONS_NULL   },
	{ 1, LOCALE_OPTIONS_SERIAL },
	{ 2, LOCALE_OPTIONS_FB     }
};

#define DRIVERSETTINGS_FDX_OPTION_COUNT 3
const CMenuOptionChooser::keyval DRIVERSETTINGS_FDX_OPTIONS[DRIVERSETTINGS_FDX_OPTION_COUNT] =
{
	{ 0, LOCALE_OPTIONS_OFF					},
	{ 1, LOCALE_OPTIONS_ON					},
	{ 2, LOCALE_DRIVERSETTINGS_FDX_FORCE	},
};

typedef struct driver_setting_files_t
{
	const neutrino_locale_t                  name;
	const char * const                       filename;
	const CMenuOptionChooser::keyval * const options;
} driver_setting_files_struct_t;

const driver_setting_files_struct_t driver_setting_files[DRIVER_SETTING_FILES_COUNT] =
{
	{LOCALE_DRIVERSETTINGS_BOOTINFO      , "/var/etc/.boot_info"     , OPTIONS_OFF0_ON1_OPTIONS },
#ifdef HAVE_DBOX_HARDWARE
#if HAVE_DVB_API_VERSION == 1
	{LOCALE_DRIVERSETTINGS_STARTBHDRIVER , "/var/etc/.bh"            , OPTIONS_OFF0_ON1_OPTIONS },
#endif
	{LOCALE_DRIVERSETTINGS_HWSECTIONS    , "/var/etc/.hw_sections"   , OPTIONS_OFF1_ON0_OPTIONS },
	{LOCALE_DRIVERSETTINGS_NOAVIAWATCHDOG, "/var/etc/.no_watchdog"   , OPTIONS_OFF1_ON0_OPTIONS },
	{LOCALE_DRIVERSETTINGS_NOENXWATCHDOG , "/var/etc/.no_enxwatchdog", OPTIONS_OFF1_ON0_OPTIONS },
	{LOCALE_DRIVERSETTINGS_PHILIPSRCPATCH, "/var/etc/.philips_rc_patch", OPTIONS_OFF0_ON1_OPTIONS },
	{LOCALE_DRIVERSETTINGS_SPTSFIX       , "/var/etc/.sptsfix"       , OPTIONS_OFF0_ON1_OPTIONS },
#ifdef ENABLE_RTC
	{LOCALE_DRIVERSETTINGS_RTC           , "/var/etc/.rtc"           , OPTIONS_OFF0_ON1_OPTIONS },
#endif
#endif
	{LOCALE_DRIVERSETTINGS_PMTUPDATE     , "/var/etc/.no_pmt_update" , OPTIONS_OFF1_ON0_OPTIONS }
};


void CDriverBootSetup::showSetup()
{

	CMenuWidget * dbs = new CMenuWidget(menue_title, menue_icon, width);
	if (menue_title != NONEXISTANT_LOCALE)
	{
		CMenuSeparator * dbs_subhead = new CMenuSeparator(CMenuSeparator::ALIGN_LEFT | CMenuSeparator::SUB_HEAD | CMenuSeparator::STRING, LOCALE_DRIVERSETTINGS_DRIVER_BOOT);
		dbs->addItem(dbs_subhead);
	}

	bool item_enabled[DRIVER_SETTING_FILES_COUNT];
	int boxtype = g_Controld->getBoxType();

#ifdef HAVE_DBOX_HARDWARE
	CSPTSNotifier *sptsNotifier = new CSPTSNotifier;
	CMenuOptionChooser * oj_spts = new CMenuOptionChooser(LOCALE_DRIVERSETTINGS_SPTSMODE, &g_settings.misc_spts, OPTIONS_OFF0_ON1_OPTIONS, OPTIONS_OFF0_ON1_OPTION_COUNT, true, sptsNotifier);
#endif

	CMenuOptionChooser * oj_switches[DRIVER_SETTING_FILES_COUNT];

	for (int i = 0; i < DRIVER_SETTING_FILES_COUNT; i++)
	{
		FILE * fd = fopen(driver_setting_files[i].filename, "r");
		if (fd)
		{
			fclose(fd);
			g_settings.misc_option[i] = 1;
		}
		else
			g_settings.misc_option[i] = 0;
		
		if (!strcmp(driver_setting_files[i].filename, "/var/etc/.philips_rc_patch") && (boxtype == 1)) // usefully for Philips RC and sometimes for Sagem RC
			item_enabled[i] = false;
		else if (!strcmp(driver_setting_files[i].filename, "/var/etc/.no_enxwatchdog") && (boxtype == 1)) // not for Nokia
			item_enabled[i] = false;
		else if (!strcmp(driver_setting_files[i].filename, "/var/etc/.sptsfix") && (boxtype != 1)) // only Nokia has Avia500
			item_enabled[i] = false;
		else
			item_enabled[i] = true;

		oj_switches[i] = new CMenuOptionChooser(driver_setting_files[i].name, &(g_settings.misc_option[i]), driver_setting_files[i].options, 2, item_enabled[i], new CTouchFileNotifier(driver_setting_files[i].filename));
	}

#ifdef HAVE_DBOX_HARDWARE
	CMenuOptionChooser * oj_boot_console = new CMenuOptionChooser(LOCALE_DRIVERSETTINGS_FB_DESTINATION, &g_settings.uboot_console, DRIVERSETTINGS_FB_DESTINATION_OPTIONS, DRIVERSETTINGS_FB_DESTINATION_OPTION_COUNT, true, new CConsoleDestChangeNotifier);
	CMenuOptionChooser * oj_dbox_duplex = new CMenuOptionChooser(LOCALE_DRIVERSETTINGS_FDX_LOAD, &g_settings.uboot_dbox_duplex, DRIVERSETTINGS_FDX_OPTIONS, DRIVERSETTINGS_FDX_OPTION_COUNT, true, new CFdxChangeNotifier);
#endif

	//paint items
	dbs->addItem(GenericMenuSeparator);
	dbs->addItem(GenericMenuBack);
	dbs->addItem(GenericMenuSeparatorLine);
	//-----------------------------------------
#ifdef HAVE_DBOX_HARDWARE
	dbs->addItem(oj_spts);
#endif
	//-----------------------------------------
	for (int i = 0; i < DRIVER_SETTING_FILES_COUNT; i++)
		dbs->addItem(oj_switches[i]);
#ifdef HAVE_DBOX_HARDWARE
	dbs->addItem(GenericMenuSeparatorLine);
	//-----------------------------------------
	dbs->addItem(oj_boot_console);
	dbs->addItem(oj_dbox_duplex);
#endif

	dbs->exec(NULL, "");
	dbs->hide();
	delete dbs;
}

