/*
	$Id: lcd_setup.cpp,v 1.1 2010/07/30 20:52:16 dbt Exp $

	lcd setup implementation - Neutrino-GUI

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


#include "gui/lcd_setup.h"

#include <global.h>
#include <neutrino.h>

#include <gui/widget/icons.h>
#include <gui/widget/lcdcontroler.h>
#include <gui/widget/stringinput.h>

#include <driver/screen_max.h>

#include <system/debug.h>

CLcdSetup::CLcdSetup(const neutrino_locale_t title, const char * const IconName)
{
	frameBuffer = CFrameBuffer::getInstance();

	menue_title = title != NONEXISTANT_LOCALE ? title : LOCALE_LCDMENU_HEAD;
	menue_icon = IconName != NULL ? IconName : NEUTRINO_ICON_LCD;

	width = w_max (500, 100);
	hheight = g_Font[SNeutrinoSettings::FONT_TYPE_MENU_TITLE]->getHeight();
	mheight = g_Font[SNeutrinoSettings::FONT_TYPE_MENU]->getHeight();
	height 	= hheight+13*mheight+ 10;
	x	= getScreenStartX (width);
	y	= getScreenStartY (height);
}

CLcdSetup::~CLcdSetup()
{

}

void CLcdSetup::hide()
{
	frameBuffer->paintBackgroundBoxRel(x,y, width,height);
}


int CLcdSetup::exec(CMenuTarget* parent, const std::string &)
{
	dprintf(DEBUG_DEBUG, "init lcd setup\n");
	if(parent != NULL)
		parent->hide();

	showSetup();
	
	return menu_return::RETURN_REPAINT;	
}

/* for lcd settings menu*/
#define LCDMENU_STATUSLINE_OPTION_COUNT 4
const CMenuOptionChooser::keyval LCDMENU_STATUSLINE_OPTIONS[LCDMENU_STATUSLINE_OPTION_COUNT] =
{
	{ 0, LOCALE_LCDMENU_STATUSLINE_PLAYTIME   },
	{ 1, LOCALE_LCDMENU_STATUSLINE_VOLUME     },
	{ 2, LOCALE_LCDMENU_STATUSLINE_BOTH       },
	{ 3, LOCALE_LCDMENU_STATUSLINE_BOTH_AUDIO }
};

/* for lcd EPG menu*/
#define LCDMENU_EPG_OPTION_COUNT 6
const CMenuOptionChooser::keyval LCDMENU_EPG_OPTIONS[LCDMENU_EPG_OPTION_COUNT] =
{
	{ 1, LOCALE_LCDMENU_EPG_NAME		},
	{ 2, LOCALE_LCDMENU_EPG_TITLE		},
	{ 3, LOCALE_LCDMENU_EPG_NAME_TITLE	},
	{ 7, LOCALE_LCDMENU_EPG_NAME_SEPLINE_TITLE },
	{ 11, LOCALE_LCDMENU_EPG_NAMESHORT_TITLE },
	{ 15, LOCALE_LCDMENU_EPG_NAMESHORT_SEPLINE_TITLE }
};

#define LCDMENU_EPGALIGN_OPTION_COUNT 2
const CMenuOptionChooser::keyval LCDMENU_EPGALIGN_OPTIONS[LCDMENU_EPGALIGN_OPTION_COUNT] =
{
	{ 0, LOCALE_LCDMENU_EPGALIGN_LEFT   },
	{ 1, LOCALE_LCDMENU_EPGALIGN_CENTER	}
};


void CLcdSetup::showSetup()
{

	CMenuWidget * lcds = new CMenuWidget(menue_title, menue_icon, width);
	if (menue_title != NONEXISTANT_LOCALE)
	{
		CMenuSeparator * lcds_subhead = new CMenuSeparator(CMenuSeparator::ALIGN_LEFT | CMenuSeparator::SUB_HEAD | CMenuSeparator::STRING, LOCALE_LCDMENU_HEAD);
		lcds->addItem(lcds_subhead);
	}

	//sliders
	CLcdControler* lcdsliders = new CLcdControler(LOCALE_LCDMENU_HEAD, NULL);

	//option invert
	CLcdNotifier* lcdnotifier = new CLcdNotifier();
	CMenuOptionChooser* oj_inverse = new CMenuOptionChooser(LOCALE_LCDMENU_INVERSE, &g_settings.lcd_setting[SNeutrinoSettings::LCD_INVERSE], OPTIONS_OFF0_ON1_OPTIONS, OPTIONS_OFF0_ON1_OPTION_COUNT, true, lcdnotifier);

#ifndef HAVE_TRIPLEDRAGON
	CMenuOptionChooser* oj_bias = NULL;
	if (g_info.box_Type == CControld::TUXBOX_MAKER_PHILIPS)
		oj_bias = new CMenuOptionChooser(LOCALE_LCDMENU_BIAS, &g_settings.lcd_setting[SNeutrinoSettings::LCD_BIAS], OPTIONS_OFF0_ON1_OPTIONS, OPTIONS_OFF0_ON1_OPTION_COUNT, true, lcdnotifier);

	CMenuOptionChooser* oj_power = new CMenuOptionChooser(LOCALE_LCDMENU_POWER, &g_settings.lcd_setting[SNeutrinoSettings::LCD_POWER], OPTIONS_OFF0_ON1_OPTIONS, OPTIONS_OFF0_ON1_OPTION_COUNT, true, lcdnotifier);

	// Autodimm available on Sagem/Philips only
	CMenuOptionChooser* oj_dimm = NULL;
	if ((g_info.box_Type == CControld::TUXBOX_MAKER_PHILIPS) || (g_info.box_Type == CControld::TUXBOX_MAKER_SAGEM))
		oj_dimm = new CMenuOptionChooser(LOCALE_LCDMENU_AUTODIMM, &g_settings.lcd_setting[SNeutrinoSettings::LCD_AUTODIMM], OPTIONS_OFF0_ON1_OPTIONS, OPTIONS_OFF0_ON1_OPTION_COUNT, true, lcdnotifier);

	//dimm time
	CStringInput * dim_time = new CStringInput(LOCALE_LCDMENU_DIM_TIME, g_settings.lcd_setting_dim_time, 3,	NONEXISTANT_LOCALE, NONEXISTANT_LOCALE,"0123456789 ");
	CMenuForwarder * fw_dim_time = new CMenuForwarder(LOCALE_LCDMENU_DIM_TIME,true, g_settings.lcd_setting_dim_time,dim_time);

	//brightness
	CStringInput * dim_brightness = new CStringInput(LOCALE_LCDMENU_DIM_BRIGHTNESS, g_settings.lcd_setting_dim_brightness, 3,NONEXISTANT_LOCALE, NONEXISTANT_LOCALE,"0123456789 ");
	CMenuForwarder * fw_brightness = new CMenuForwarder(LOCALE_LCDMENU_DIM_BRIGHTNESS,true, g_settings.lcd_setting_dim_brightness,dim_brightness);
#endif
	//sliders
	CMenuForwarder * fw_sliders = new CMenuForwarder(LOCALE_LCDMENU_LCDCONTROLER, true, NULL, lcdsliders);

	//status display
	CMenuOptionChooser* oj_status = new CMenuOptionChooser(LOCALE_LCDMENU_STATUSLINE, &g_settings.lcd_setting[SNeutrinoSettings::LCD_SHOW_VOLUME], LCDMENU_STATUSLINE_OPTIONS, LCDMENU_STATUSLINE_OPTION_COUNT, true);
	
	//lcd_epg
	CMenuOptionChooser* oj_epg = new CMenuOptionChooser(LOCALE_LCDMENU_EPG, &g_settings.lcd_setting[SNeutrinoSettings::LCD_EPGMODE], LCDMENU_EPG_OPTIONS, LCDMENU_EPG_OPTION_COUNT, true);

	//align
	CMenuOptionChooser* oj_align = new CMenuOptionChooser(LOCALE_LCDMENU_EPGALIGN, &g_settings.lcd_setting[SNeutrinoSettings::LCD_EPGALIGN], LCDMENU_EPGALIGN_OPTIONS, LCDMENU_EPGALIGN_OPTION_COUNT, true);


	//paint items
	lcds->addItem(GenericMenuSeparator);
	lcds->addItem(GenericMenuBack);
	lcds->addItem(GenericMenuSeparatorLine);
	//----------------------------------------
	lcds->addItem(oj_inverse);
#ifndef HAVE_TRIPLEDRAGON
	if (oj_bias !=NULL)
		lcds->addItem(oj_bias);
	lcds->addItem(oj_power);
	if (oj_dimm !=NULL)
		lcds->addItem(oj_dimm);
	lcds->addItem(fw_dim_time);
	lcds->addItem(fw_brightness);
#endif
	//---------------------------------------
	lcds->addItem(GenericMenuSeparatorLine);
	lcds->addItem(fw_sliders);
	//---------------------------------------
	lcds->addItem(GenericMenuSeparatorLine);
	lcds->addItem(oj_status);
	//---------------------------------------
	lcds->addItem(GenericMenuSeparatorLine);
	lcds->addItem(oj_epg);
	lcds->addItem(oj_align);

	lcds->exec(NULL, "");
	lcds->hide();
	delete lcds;
}

