/*
	$Id: osd_setup.cpp,v 1.6 2010/09/30 20:13:59 dbt Exp $

	osd_setup implementation - Neutrino-GUI

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


#include "osd_setup.h"
#include "alphasetup.h"
#include "themes.h"
#include "screensetup.h"
#include "osdlang_setup.h"
#include "filebrowser.h"

#include <global.h>
#include <neutrino.h>

#include <gui/widget/icons.h>
#include <gui/widget/colorchooser.h>
#include <gui/widget/stringinput.h>

#include <driver/screen_max.h>

#include <system/debug.h>

static CTimingSettingsNotifier timingsettingsnotifier;

const SNeutrinoSettings::FONT_TYPES channellist_font_sizes[4] =
{
	SNeutrinoSettings::FONT_TYPE_CHANNELLIST,
	SNeutrinoSettings::FONT_TYPE_CHANNELLIST_DESCR,
	SNeutrinoSettings::FONT_TYPE_CHANNELLIST_NUMBER,
	SNeutrinoSettings::FONT_TYPE_CHANNEL_NUM_ZAP
};

const SNeutrinoSettings::FONT_TYPES eventlist_font_sizes[4] =
{
	SNeutrinoSettings::FONT_TYPE_EVENTLIST_TITLE,
	SNeutrinoSettings::FONT_TYPE_EVENTLIST_ITEMLARGE,
	SNeutrinoSettings::FONT_TYPE_EVENTLIST_ITEMSMALL,
	SNeutrinoSettings::FONT_TYPE_EVENTLIST_DATETIME,
};

const SNeutrinoSettings::FONT_TYPES infobar_font_sizes[4] =
{
	SNeutrinoSettings::FONT_TYPE_INFOBAR_NUMBER,
	SNeutrinoSettings::FONT_TYPE_INFOBAR_CHANNAME,
	SNeutrinoSettings::FONT_TYPE_INFOBAR_INFO,
	SNeutrinoSettings::FONT_TYPE_INFOBAR_SMALL
};

const SNeutrinoSettings::FONT_TYPES epg_font_sizes[4] =
{
	SNeutrinoSettings::FONT_TYPE_EPG_TITLE,
	SNeutrinoSettings::FONT_TYPE_EPG_INFO1,
	SNeutrinoSettings::FONT_TYPE_EPG_INFO2,
	SNeutrinoSettings::FONT_TYPE_EPG_DATE
};

const SNeutrinoSettings::FONT_TYPES gamelist_font_sizes[2] =
{
	SNeutrinoSettings::FONT_TYPE_GAMELIST_ITEMLARGE,
	SNeutrinoSettings::FONT_TYPE_GAMELIST_ITEMSMALL
};

const SNeutrinoSettings::FONT_TYPES other_font_sizes[4] =
{
	SNeutrinoSettings::FONT_TYPE_MENU,
	SNeutrinoSettings::FONT_TYPE_MENU_TITLE,
	SNeutrinoSettings::FONT_TYPE_MENU_INFO,
	SNeutrinoSettings::FONT_TYPE_FILEBROWSER_ITEM
};

font_sizes_groups font_sizes_groups[6] =
{
	{LOCALE_FONTMENU_CHANNELLIST, 4, channellist_font_sizes, "fontsize.dcha"},
	{LOCALE_FONTMENU_EVENTLIST  , 4, eventlist_font_sizes  , "fontsize.deve"},
	{LOCALE_FONTMENU_EPG        , 4, epg_font_sizes        , "fontsize.depg"},
	{LOCALE_FONTMENU_INFOBAR    , 4, infobar_font_sizes    , "fontsize.dinf"},
	{LOCALE_FONTMENU_GAMELIST   , 2, gamelist_font_sizes   , "fontsize.dgam"},
	{NONEXISTANT_LOCALE         , 4, other_font_sizes      , "fontsize.doth"}
};

COsdSetup::COsdSetup(const neutrino_locale_t title, const char * const IconName)
{
	frameBuffer = CFrameBuffer::getInstance();

#ifdef HAVE_DBOX_HARDWARE
	if (g_info.box_Type == CControld::TUXBOX_MAKER_NOKIA)
		frameBuffer->setBlendLevel(g_settings.gtx_alpha1, g_settings.gtx_alpha2);
#endif

	colorSetupNotifier = new CColorSetupNotifier();
	colorSetupNotifier->changeNotify(NONEXISTANT_LOCALE, NULL);

	fontsizenotifier = new CFontSizeNotifier;

	menue_title = title != NONEXISTANT_LOCALE ? title : LOCALE_OSDSETTINGS_COLORMENU_HEAD;
	menue_icon = IconName != NULL ? IconName : NEUTRINO_ICON_COLORS;

	width = w_max (500, 100);
	hheight = g_Font[SNeutrinoSettings::FONT_TYPE_MENU_TITLE]->getHeight();
	mheight = g_Font[SNeutrinoSettings::FONT_TYPE_MENU]->getHeight();
	height 	= hheight+13*mheight+ 10;
	x	= getScreenStartX (width);
	y	= getScreenStartY (height);
}

COsdSetup::~COsdSetup()
{
	delete colorSetupNotifier;
	delete fontsizenotifier;
}

void COsdSetup::hide()
{
	frameBuffer->paintBackgroundBoxRel(x,y, width,height);
}


int COsdSetup::exec(CMenuTarget* parent, const std::string &actionKey)
{
	dprintf(DEBUG_DEBUG, "init osd setup\n");

	if(parent != NULL)
		parent->hide();

	if (actionKey=="show_menue_color_setup")
	{
		showOsdMenueColorSetup();
		return menu_return::RETURN_REPAINT;
	}
	else if (actionKey=="show_infobar_color_setup")
	{
		showOsdInfobarColorSetup();
		return menu_return::RETURN_REPAINT;
	}
	else if (actionKey=="show_timeout_setup")
	{
		showOsdTimeoutSetup();
		return menu_return::RETURN_REPAINT;
	}
	else if (actionKey=="show_infobar_setup")
	{
		showOsdInfobarSetup();
		return menu_return::RETURN_REPAINT;
	}
	else if (actionKey=="show_channellist_setup")
	{
		showOsdChannelListSetup();
		return menu_return::RETURN_REPAINT;
	}
	else if (actionKey=="show_fontsize_setup")
	{
		showOsdFontSizeSetup();
		return menu_return::RETURN_REPAINT;
	}
	else if(strncmp(actionKey.c_str(), "fontsize.d", 10) == 0)
	{
		for (int i = 0; i < 6; i++)
		{
			if (actionKey == font_sizes_groups[i].actionkey)
				for (unsigned int j = 0; j < font_sizes_groups[i].count; j++)
				{
					SNeutrinoSettings::FONT_TYPES k = font_sizes_groups[i].content[j];
					CNeutrinoApp::getInstance()->getConfigFile()->setInt32(locale_real_names[neutrino_font[k].name], neutrino_font[k].defaultsize);
				}
		}

		fontsizenotifier->changeNotify(NONEXISTANT_LOCALE, NULL);
		return menu_return::RETURN_REPAINT;
	}
	else if(actionKey == "channel_logodir")
	{
		CFileBrowser b;
		b.Dir_Mode=true;
		if (b.exec(g_settings.infobar_channel_logodir))
			strncpy(g_settings.infobar_channel_logodir, b.getSelectedFile()->Name.c_str(), sizeof(g_settings.infobar_channel_logodir)-1);
		return menu_return::RETURN_REPAINT;
	}
	else if(actionKey=="osd.def")
	{
		for (int i = 0; i < TIMING_SETTING_COUNT; i++)
			g_settings.timing[i] = timing_setting[i].default_timing;

		CNeutrinoApp::getInstance()->SetupTiming();
		return menu_return::RETURN_REPAINT;
	}

	showOsdSetup();
	
	return menu_return::RETURN_REPAINT;	
}

/* for color settings menu */
#define COLORMENU_CORNERSETTINGS_TYPE_OPTION_COUNT 2
const CMenuOptionChooser::keyval COLORMENU_CORNERSETTINGS_TYPE_OPTIONS[COLORMENU_CORNERSETTINGS_TYPE_OPTION_COUNT] =
{
	{ 0, LOCALE_OSDSETTINGS_ROUNDED_CORNERS_OFF },
	{ 1, LOCALE_OSDSETTINGS_ROUNDED_CORNERS_ON  }
};

#define VOLUMEBAR_DISP_POS_OPTIONS_COUNT 7
const CMenuOptionChooser::keyval  VOLUMEBAR_DISP_POS_OPTIONS[VOLUMEBAR_DISP_POS_OPTIONS_COUNT]=
{
	{ 0 , LOCALE_SETTINGS_POS_TOP_RIGHT },
	{ 1 , LOCALE_SETTINGS_POS_TOP_LEFT },
	{ 2 , LOCALE_SETTINGS_POS_BOTTOM_LEFT },
	{ 3 , LOCALE_SETTINGS_POS_BOTTOM_RIGHT },
	{ 4 , LOCALE_SETTINGS_POS_DEFAULT_CENTER },
	{ 5 , LOCALE_SETTINGS_POS_HIGHER_CENTER },
	{ 6 , LOCALE_SETTINGS_POS_OFF }
};

#define SHOW_MUTE_ICON_OPTIONS_COUNT 3
const CMenuOptionChooser::keyval  SHOW_MUTE_ICON_OPTIONS[SHOW_MUTE_ICON_OPTIONS_COUNT]=
{
	{ 0 , LOCALE_MISCSETTINGS_SHOW_MUTE_ICON_NO },
	{ 1 , LOCALE_MISCSETTINGS_SHOW_MUTE_ICON_YES },
	{ 2 , LOCALE_MISCSETTINGS_SHOW_MUTE_ICON_NOT_IN_AC3MODE }
};

//show osd setup
void COsdSetup::showOsdSetup()
{
	//osd main settings
	CMenuWidget *osd_setup 		= new CMenuWidget(menue_title, menue_icon, width);
	//osd settings color sbubmenue
	CMenuWidget *osd_setup_colors 	= new CMenuWidget(menue_title, menue_icon, width);


	// language
	CMenuForwarder *osd_lang_fw = new CMenuForwarder(LOCALE_MAINSETTINGS_LANGUAGE, true, NULL, new COsdLangSetup() , NULL, CRCInput::RC_red, NEUTRINO_ICON_BUTTON_RED);
	//osd themes setup forwarder
	CMenuForwarder *osd_themes_fw	= new CMenuForwarder(LOCALE_OSDSETTINGS_THEMESELECT, true, NULL, new CThemes(), NULL, CRCInput::RC_green, NEUTRINO_ICON_BUTTON_GREEN);
	//osd color setup forwarder
	CMenuForwarder *osd_setup_color_sub_fw	= new CMenuForwarder(LOCALE_OSDSETTINGS_COLORMENU_HEAD, true, NULL, osd_setup_colors, NULL, CRCInput::RC_yellow, NEUTRINO_ICON_BUTTON_YELLOW);
		//osd color setup subhead	
		CMenuSeparator * osd_setup_color_subhead = new CMenuSeparator(CMenuSeparator::ALIGN_LEFT | CMenuSeparator::SUB_HEAD | CMenuSeparator::STRING, LOCALE_OSDSETTINGS_COLORMENU_HEAD);
		//osd menue colors forwarder
		CMenuForwarder *osd_menucolor_fw = new CMenuForwarder(LOCALE_OSDSETTINGS_COLORMENU_MENUCOLORS, true, NULL, this, "show_menue_color_setup", CRCInput::RC_red, NEUTRINO_ICON_BUTTON_RED);
		//osd infobar setup forwarder
		CMenuForwarder *osd_sbcolor_fw = new CMenuForwarder(LOCALE_OSDSETTINGS_COLORMENU_STATUSBAR, true, NULL, this, "show_infobar_color_setup", CRCInput::RC_green, NEUTRINO_ICON_BUTTON_GREEN);
		//osd timeout setup forwarder
	CMenuForwarder *osd_timeout_fw = new CMenuForwarder(LOCALE_TIMING_HEAD, true, NULL,  this, "show_timeout_setup", CRCInput::RC_blue, NEUTRINO_ICON_BUTTON_BLUE);
	//osd screen setup
	CMenuForwarder *osd_screen_fw = new CMenuForwarder(LOCALE_VIDEOMENU_SCREENSETUP, true, NULL, new CScreenSetup(), NULL, CRCInput::RC_1, NEUTRINO_ICON_BUTTON_1);
	//osd infobar setup
	CMenuForwarder *osd_infobar_fw = new CMenuForwarder(LOCALE_OSDSETTINGS_INFOBAR, true, NULL, this, "show_infobar_setup", CRCInput::RC_2, NEUTRINO_ICON_BUTTON_2);
	//osd channellist setup
	CMenuForwarder *osd_chanlist_fw = new CMenuForwarder(LOCALE_MISCSETTINGS_CHANNELLIST, true, NULL, this, "show_channellist_setup", CRCInput::RC_3, NEUTRINO_ICON_BUTTON_3);
	//osd fontzize setup
	CMenuForwarder *osd_fontsize_fw = new CMenuForwarder(LOCALE_FONTMENU_HEAD, true, NULL, this, "show_fontsize_setup", CRCInput::RC_4, NEUTRINO_ICON_BUTTON_4);

	//osd volumbar position
 	CMenuOptionChooser* osd_volumbarpos_ch = new CMenuOptionChooser(LOCALE_OSDSETTINGS_VOLUMEBAR_DISP_POS, &g_settings.volumebar_disp_pos, VOLUMEBAR_DISP_POS_OPTIONS, VOLUMEBAR_DISP_POS_OPTIONS_COUNT, true);
	//osd mute icon options
 	CMenuOptionChooser* osd_mute_icon_ch = new CMenuOptionChooser(LOCALE_OSDSETTINGS_SHOW_MUTE_ICON, &g_settings.show_mute_icon, SHOW_MUTE_ICON_OPTIONS, SHOW_MUTE_ICON_OPTIONS_COUNT, true);
	//osd corner chooser
	CMenuOptionChooser* osd_corners_ch = new CMenuOptionChooser(LOCALE_OSDSETTINGS_ROUNDED_CORNERS, &g_settings.rounded_corners, COLORMENU_CORNERSETTINGS_TYPE_OPTIONS, COLORMENU_CORNERSETTINGS_TYPE_OPTION_COUNT, true );

	//osd main settings, subhead
	if (menue_title != NONEXISTANT_LOCALE)
	{
		CMenuSeparator * osd_setup_subhead = new CMenuSeparator(CMenuSeparator::ALIGN_LEFT | CMenuSeparator::SUB_HEAD | CMenuSeparator::STRING, LOCALE_MAINSETTINGS_OSD);
		osd_setup->addItem(osd_setup_subhead);
	}
	osd_setup->addItem(GenericMenuSeparator);
	osd_setup->addItem(GenericMenuBack);
	osd_setup->addItem(GenericMenuSeparatorLine);
	//--------------------------------------------
	osd_setup->addItem(osd_lang_fw);	//language setup
	osd_setup->addItem(osd_themes_fw);	//themes setup
	osd_setup->addItem(osd_setup_color_sub_fw);	//color setup
		osd_setup_colors->addItem(osd_setup_color_subhead); //color setup subhead
		osd_setup_colors->addItem(GenericMenuSeparator);
		osd_setup_colors->addItem(GenericMenuBack);
		osd_setup_colors->addItem(GenericMenuSeparatorLine);
		osd_setup_colors->addItem(osd_menucolor_fw);	//menue colors
		osd_setup_colors->addItem(osd_sbcolor_fw);	//infobar colors
	osd_setup->addItem(osd_timeout_fw);	//timeout
	osd_setup->addItem(GenericMenuSeparatorLine);
	//-------------------------------------------
	osd_setup->addItem(osd_screen_fw);	//screen setup
	osd_setup->addItem(osd_infobar_fw);	//infobar setup
	osd_setup->addItem(osd_chanlist_fw);	//channellist setup
	osd_setup->addItem(osd_fontsize_fw);	//fontsize setup
#ifdef HAVE_DBOX_HARDWARE
	if ((g_info.box_Type == CControld::TUXBOX_MAKER_PHILIPS) || (g_info.box_Type == CControld::TUXBOX_MAKER_SAGEM))
	{	
		// eNX
		osd_setup->addItem(new CMenuOptionChooser(LOCALE_OSDSETTINGS_COLORMENU_FADE, &g_settings.widget_fade, OPTIONS_OFF0_ON1_OPTIONS, OPTIONS_OFF0_ON1_OPTION_COUNT, true )); //fade

	}
	else 
	{
		//GTX
		CAlphaSetup* chAlphaSetup = new CAlphaSetup(LOCALE_OSDSETTINGS_COLORMENU_GTX_ALPHA);
		osd_setup->addItem(new CMenuForwarder(LOCALE_OSDSETTINGS_COLORMENU_GTX_ALPHA, true, NULL, chAlphaSetup, NULL, CRCInput::RC_5, NEUTRINO_ICON_BUTTON_5));
	}
#else 	
	//Dream and TD
	osd_setup->addItem(new CMenuOptionChooser(LOCALE_OSDSETTINGS_COLORMENU_FADE, &g_settings.widget_fade, OPTIONS_OFF0_ON1_OPTIONS, OPTIONS_OFF0_ON1_OPTION_COUNT, true )); //fade
#endif
	osd_setup->addItem(GenericMenuSeparatorLine);
	//-------------------------------------------
	osd_setup->addItem(osd_corners_ch); //corner style
	osd_setup->addItem(osd_volumbarpos_ch);	//volumebar
	osd_setup->addItem(osd_mute_icon_ch);	//mute icon

	osd_setup->exec(NULL, "");
	osd_setup->hide();
	delete osd_setup;
}


//show menue colorsetup
void COsdSetup::showOsdMenueColorSetup()
{
	CMenuWidget * ocs = new CMenuWidget(menue_title, menue_icon, width);
	CMenuSeparator * ocs_setup_subhead = new CMenuSeparator(CMenuSeparator::ALIGN_LEFT | CMenuSeparator::SUB_HEAD | CMenuSeparator::STRING, LOCALE_OSDSETTINGS_COLORMENU_MENUCOLORS);

	ocs->addItem(ocs_setup_subhead);
	ocs->addItem(GenericMenuSeparator);
	ocs->addItem(GenericMenuBack);

	CColorChooser* chHeadcolor = new CColorChooser(LOCALE_OSDSETTINGS_COLORMENU_BACKGROUND_HEAD, &g_settings.menu_Head_red, &g_settings.menu_Head_green, &g_settings.menu_Head_blue,  &g_settings.menu_Head_alpha, colorSetupNotifier);
	CColorChooser* chHeadTextcolor = new CColorChooser(LOCALE_OSDSETTINGS_COLORMENU_TEXTCOLOR_HEAD, &g_settings.menu_Head_Text_red, &g_settings.menu_Head_Text_green, &g_settings.menu_Head_Text_blue, NULL, colorSetupNotifier);
	CColorChooser* chContentcolor = new CColorChooser(LOCALE_OSDSETTINGS_COLORMENU_BACKGROUND_HEAD, &g_settings.menu_Content_red, &g_settings.menu_Content_green, &g_settings.menu_Content_blue,&g_settings.menu_Content_alpha, colorSetupNotifier);
	CColorChooser* chContentTextcolor = new CColorChooser(LOCALE_OSDSETTINGS_COLORMENU_TEXTCOLOR_HEAD, &g_settings.menu_Content_Text_red, &g_settings.menu_Content_Text_green, &g_settings.menu_Content_Text_blue, NULL, colorSetupNotifier);
	CColorChooser* chContentSelectedcolor = new CColorChooser(LOCALE_OSDSETTINGS_COLORMENU_BACKGROUND_HEAD, &g_settings.menu_Content_Selected_red, &g_settings.menu_Content_Selected_green, &g_settings.menu_Content_Selected_blue,&g_settings.menu_Content_Selected_alpha, colorSetupNotifier);
	CColorChooser* chContentSelectedTextcolor = new CColorChooser(LOCALE_OSDSETTINGS_COLORMENU_TEXTCOLOR_HEAD, &g_settings.menu_Content_Selected_Text_red, &g_settings.menu_Content_Selected_Text_green, &g_settings.menu_Content_Selected_Text_blue,NULL, colorSetupNotifier);
	CColorChooser* chContentInactivecolor = new CColorChooser(LOCALE_OSDSETTINGS_COLORMENU_BACKGROUND_HEAD, &g_settings.menu_Content_inactive_red, &g_settings.menu_Content_inactive_green, &g_settings.menu_Content_inactive_blue, &g_settings.menu_Content_inactive_alpha, colorSetupNotifier);
	CColorChooser* chContentInactiveTextcolor = new CColorChooser(LOCALE_OSDSETTINGS_COLORMENU_TEXTCOLOR_HEAD, &g_settings.menu_Content_inactive_Text_red, &g_settings.menu_Content_inactive_Text_green, &g_settings.menu_Content_inactive_Text_blue,NULL, colorSetupNotifier);

	ocs->addItem( new CMenuSeparator(CMenuSeparator::LINE | CMenuSeparator::STRING, LOCALE_COLORMENUSETUP_MENUHEAD));
	ocs->addItem( new CMenuForwarder(LOCALE_OSDSETTINGS_COLORMENU_BACKGROUND, true, NULL, chHeadcolor ));
	ocs->addItem( new CMenuForwarder(LOCALE_OSDSETTINGS_COLORMENU_TEXTCOLOR, true, NULL, chHeadTextcolor ));
	ocs->addItem( new CMenuSeparator(CMenuSeparator::LINE | CMenuSeparator::STRING, LOCALE_COLORMENUSETUP_MENUCONTENT));
	ocs->addItem( new CMenuForwarder(LOCALE_OSDSETTINGS_COLORMENU_BACKGROUND, true, NULL, chContentcolor ));
	ocs->addItem( new CMenuForwarder(LOCALE_OSDSETTINGS_COLORMENU_TEXTCOLOR, true, NULL, chContentTextcolor ));
	ocs->addItem( new CMenuSeparator(CMenuSeparator::LINE | CMenuSeparator::STRING, LOCALE_COLORMENUSETUP_MENUCONTENT_INACTIVE));
	ocs->addItem( new CMenuForwarder(LOCALE_OSDSETTINGS_COLORMENU_BACKGROUND, true, NULL, chContentInactivecolor ));
	ocs->addItem( new CMenuForwarder(LOCALE_OSDSETTINGS_COLORMENU_TEXTCOLOR, true, NULL, chContentInactiveTextcolor));
	ocs->addItem( new CMenuSeparator(CMenuSeparator::LINE | CMenuSeparator::STRING, LOCALE_COLORMENUSETUP_MENUCONTENT_SELECTED));
	ocs->addItem( new CMenuForwarder(LOCALE_OSDSETTINGS_COLORMENU_BACKGROUND, true, NULL, chContentSelectedcolor ));
	ocs->addItem( new CMenuForwarder(LOCALE_OSDSETTINGS_COLORMENU_TEXTCOLOR, true, NULL, chContentSelectedTextcolor ));

	ocs->exec(NULL, "");
	ocs->hide();
	delete ocs;
}


/* infobar colors */
void COsdSetup::showOsdInfobarColorSetup()
{
	CMenuWidget * ois = new CMenuWidget(menue_title, menue_icon, width);
	CMenuSeparator * ois_setup_subhead = new CMenuSeparator(CMenuSeparator::ALIGN_LEFT | CMenuSeparator::SUB_HEAD | CMenuSeparator::STRING, LOCALE_COLORSTATUSBAR_TEXT);

	CColorChooser *chInfobarcolor 		= new CColorChooser(LOCALE_OSDSETTINGS_COLORMENU_BACKGROUND_HEAD, &g_settings.infobar_red, &g_settings.infobar_green, &g_settings.infobar_blue,  &g_settings.infobar_alpha, colorSetupNotifier);
	CColorChooser *chInfobarTextcolor_head 	= new CColorChooser(LOCALE_OSDSETTINGS_COLORMENU_TEXTCOLOR_HEAD, &g_settings.infobar_Text_red, &g_settings.infobar_Text_green, &g_settings.infobar_Text_blue, NULL, colorSetupNotifier);
	CMenuForwarder *fwInfobarBackground 	= new CMenuForwarder(LOCALE_OSDSETTINGS_COLORMENU_BACKGROUND, true, NULL, chInfobarcolor);
	CMenuForwarder *fwInfobarTextcolor 	= new CMenuForwarder(LOCALE_OSDSETTINGS_COLORMENU_TEXTCOLOR, true, NULL, chInfobarTextcolor_head);

	ois->addItem(ois_setup_subhead);
	ois->addItem(GenericMenuSeparator);
	ois->addItem(GenericMenuBack);
	ois->addItem(GenericMenuSeparatorLine);
	//-----------------------------------------------
	ois->addItem(fwInfobarBackground);
	ois->addItem(fwInfobarTextcolor);

	ois->exec(NULL, "");
	ois->hide();
	delete ois;
}


/* OSD timeouts */
void COsdSetup::showOsdTimeoutSetup()
{
	/* note: SetupTiming() is already called in CNeutrinoApp::run */

	CMenuWidget * ots = new CMenuWidget(menue_title, menue_icon, width);
	CMenuSeparator * ots_setup_subhead = new CMenuSeparator(CMenuSeparator::ALIGN_LEFT | CMenuSeparator::SUB_HEAD | CMenuSeparator::STRING, LOCALE_TIMING_HEAD);

	ots->addItem(ots_setup_subhead);
	ots->addItem(GenericMenuSeparator);
	ots->addItem(GenericMenuBack);
	ots->addItem(GenericMenuSeparatorLine);

	for (int i = 0; i < TIMING_SETTING_COUNT; i++)
	{
		CStringInput * colorSettings_timing_item = new CStringInput(timing_setting[i].name, g_settings.timing_string[i], 3, LOCALE_TIMING_HINT_1, LOCALE_TIMING_HINT_2, "0123456789 ", &timingsettingsnotifier);
		ots->addItem(new CMenuForwarder(timing_setting[i].name, true, g_settings.timing_string[i], colorSettings_timing_item));
	}

	ots->addItem(GenericMenuSeparatorLine);
	ots->addItem(new CMenuForwarder(LOCALE_OPTIONS_DEFAULT, true, NULL, this, "osd.def"));

	ots->exec(NULL, "");
	ots->hide();
	delete ots;
}

#define INFOBAR_EPG_SHOW_OPTIONS_COUNT 3
const CMenuOptionChooser::keyval  INFOBAR_EPG_SHOW_OPTIONS[INFOBAR_EPG_SHOW_OPTIONS_COUNT]=
{
   	{ 0 , LOCALE_OPTIONS_OFF },
   	{ 1 , LOCALE_INFOVIEWER_EPGINFO_SIMPLE_MESSAGE },
   	{ 2 , LOCALE_INFOVIEWER_EPGINFO_EXPENSIVE_MESSAGE }
};

#define INFOBAR_CHANNELLOGO_SHOW_OPTIONS_COUNT 4
const CMenuOptionChooser::keyval  INFOBAR_CHANNELLOGO_SHOW_OPTIONS[INFOBAR_CHANNELLOGO_SHOW_OPTIONS_COUNT]=
{
   	{ 0 , LOCALE_INFOVIEWER_CHANNELLOGO_OFF },
   	{ 1 , LOCALE_INFOVIEWER_CHANNELLOGO_SHOW_IN_NUMBERBOX },
  	{ 2 , LOCALE_INFOVIEWER_CHANNELLOGO_SHOW_AS_CHANNELNAME },
   	{ 3 , LOCALE_INFOVIEWER_CHANNELLOGO_SHOW_BESIDE_CHANNELNAME }
};

#define INFOBAR_CHANNELLOGO_BACKGROUND_SHOW_OPTIONS_COUNT 3
const CMenuOptionChooser::keyval  INFOBAR_CHANNELLOGO_BACKGROUND_SHOW_OPTIONS[INFOBAR_CHANNELLOGO_BACKGROUND_SHOW_OPTIONS_COUNT]=
{
   	{ 0 , LOCALE_INFOVIEWER_CHANNELLOGO_BACKGROUND_OFF },
   	{ 1 , LOCALE_INFOVIEWER_CHANNELLOGO_BACKGROUND_FRAMED },
   	{ 2 , LOCALE_INFOVIEWER_CHANNELLOGO_BACKGROUND_SHADED }
};

#define INFOBAR_SUBCHAN_DISP_POS_OPTIONS_COUNT 5
const CMenuOptionChooser::keyval  INFOBAR_SUBCHAN_DISP_POS_OPTIONS[INFOBAR_SUBCHAN_DISP_POS_OPTIONS_COUNT]=
{
	{ 0 , LOCALE_SETTINGS_POS_TOP_RIGHT },
	{ 1 , LOCALE_SETTINGS_POS_TOP_LEFT },
	{ 2 , LOCALE_SETTINGS_POS_BOTTOM_LEFT },
	{ 3 , LOCALE_SETTINGS_POS_BOTTOM_RIGHT },
	{ 4 , LOCALE_SETTINGS_POS_INFOBAR }
};

//infobar settings
void COsdSetup::showOsdInfobarSetup()
{
	CMenuWidget * oibs = new CMenuWidget(menue_title, menue_icon, width);
	CMenuSeparator * oibs_setup_subhead 	= new CMenuSeparator(CMenuSeparator::ALIGN_LEFT | CMenuSeparator::SUB_HEAD | CMenuSeparator::STRING, LOCALE_OSDSETTINGS_INFOBAR);

	//prepare items
	CMenuOptionChooser *oibs_sat_display_ch = new CMenuOptionChooser(LOCALE_OSDSETTINGS_INFOBAR_SAT_DISPLAY, &g_settings.infobar_sat_display, OPTIONS_OFF0_ON1_OPTIONS, OPTIONS_OFF0_ON1_OPTION_COUNT, true);
	CMenuOptionChooser *oibs_subchanpos_ch 	= new CMenuOptionChooser(LOCALE_OSDSETTINGS_INFOVIEWER_SUBCHAN_DISP_POS, &g_settings.infobar_subchan_disp_pos, INFOBAR_SUBCHAN_DISP_POS_OPTIONS, INFOBAR_SUBCHAN_DISP_POS_OPTIONS_COUNT, true);
	CMenuOptionChooser *oibs_vzap_ch 	= new CMenuOptionChooser(LOCALE_OSDSETTINGS_INFOBAR_VIRTUAL_ZAP_MODE, &g_settings.virtual_zap_mode, OPTIONS_OFF0_ON1_OPTIONS, OPTIONS_OFF0_ON1_OPTION_COUNT, true);
	CMenuOptionChooser *oibs_epgshow_ch 	= new CMenuOptionChooser(LOCALE_OSDSETTINGS_INFOBAR_SHOW, &g_settings.infobar_show, INFOBAR_EPG_SHOW_OPTIONS, INFOBAR_EPG_SHOW_OPTIONS_COUNT, true);

#ifdef ENABLE_RADIOTEXT
	CRadiotextNotifier *radiotextNotifier 	= new CRadiotextNotifier;
	CMenuOptionChooser *oibs_radiotext_ch 	= new CMenuOptionChooser(LOCALE_OSDSETTINGS_INFOVIEWER_RADIOTEXT, &g_settings.radiotext_enable, OPTIONS_OFF0_ON1_OPTIONS, OPTIONS_OFF0_ON1_OPTION_COUNT, true, radiotextNotifier);
#endif
	CMenuSeparator     *oibs_chanlogo_sep 	= new CMenuSeparator(CMenuSeparator::LINE | CMenuSeparator::STRING, LOCALE_OSDSETTINGS_INFOBAR_CHANNELLOGO);
	
	
	//channel logo
	COsdSetupChannelLogoNotifier *channelLogoNotifier = NULL;
	bool activ_logo_opts = g_settings.infobar_show_channellogo != INFOBAR_NO_LOGO ? true : false; 
	CMenuForwarder 	   *oibs_chanlogo_fw 	= new CMenuForwarder(LOCALE_OSDSETTINGS_INFOBAR_CHANNELLOGO_LOGODIR, activ_logo_opts, g_settings.infobar_channel_logodir, this, "channel_logodir");
	CMenuOptionChooser *oibs_chanlogo_bg_ch = new CMenuOptionChooser(LOCALE_OSDSETTINGS_INFOBAR_CHANNELLOGO_BACKGROUND, &g_settings.infobar_channellogo_background, INFOBAR_CHANNELLOGO_BACKGROUND_SHOW_OPTIONS, INFOBAR_CHANNELLOGO_BACKGROUND_SHOW_OPTIONS_COUNT, activ_logo_opts);

	channelLogoNotifier = new COsdSetupChannelLogoNotifier(oibs_chanlogo_fw, oibs_chanlogo_bg_ch );
	CMenuOptionChooser *oibs_chanlogo_ch 	= new CMenuOptionChooser(LOCALE_OSDSETTINGS_INFOBAR_CHANNELLOGO_SHOW, &g_settings.infobar_show_channellogo, INFOBAR_CHANNELLOGO_SHOW_OPTIONS, INFOBAR_CHANNELLOGO_SHOW_OPTIONS_COUNT, true, channelLogoNotifier);
	


	//show items
	oibs->addItem(oibs_setup_subhead);
	oibs->addItem(GenericMenuSeparator);
	oibs->addItem(GenericMenuBack);
	oibs->addItem(GenericMenuSeparatorLine);
	//-------------------------------------------------
	oibs->addItem(oibs_sat_display_ch);
	oibs->addItem(oibs_subchanpos_ch);
	oibs->addItem(oibs_vzap_ch);
	oibs->addItem(oibs_epgshow_ch);
#ifdef ENABLE_RADIOTEXT
	oibs->addItem(oibs_radiotext_ch);
#endif
	oibs->addItem(oibs_chanlogo_sep);
	//-------------------------------------------------	
	oibs->addItem(oibs_chanlogo_ch);
	oibs->addItem(oibs_chanlogo_fw);
	oibs->addItem(oibs_chanlogo_bg_ch);

	oibs->exec(NULL, "");
	oibs->hide();
	delete oibs;
}
#include <iostream>
// class COsdSetupChannelLogoNotifier
//enable disable entry for channel logo path 
COsdSetupChannelLogoNotifier::COsdSetupChannelLogoNotifier( CMenuForwarder* fw1, CMenuOptionChooser* oj1)
{
	toDisable1 = fw1;
	toDisable2 = oj1;
}
bool COsdSetupChannelLogoNotifier::changeNotify(const neutrino_locale_t, void * Data)
{
	if (*((int *)Data) == 0)
	{
		toDisable1->setActive(false);
		toDisable2->setActive(false);
	}
	else
	{
		toDisable1->setActive(true);
		toDisable2->setActive(true);
	}

	return true;
}

#define CHANNELLIST_EPGTEXT_ALIGN_RIGHT_OPTIONS_COUNT 2
const CMenuOptionChooser::keyval  CHANNELLIST_EPGTEXT_ALIGN_RIGHT_OPTIONS[CHANNELLIST_EPGTEXT_ALIGN_RIGHT_OPTIONS_COUNT]=
{
	{ 0 , LOCALE_CHANNELLIST_EPGTEXT_ALIGN_LEFT },
	{ 1 , LOCALE_CHANNELLIST_EPGTEXT_ALIGN_RIGHT }
};

//channellist
void COsdSetup::showOsdChannelListSetup()
{
	CMenuWidget * ocls = new CMenuWidget(menue_title, menue_icon, width);
	CMenuSeparator * ocls_setup_subhead 	= new CMenuSeparator(CMenuSeparator::ALIGN_LEFT | CMenuSeparator::SUB_HEAD | CMenuSeparator::STRING, LOCALE_MISCSETTINGS_CHANNELLIST);

	CMenuOptionChooser *ocls_align_ch 	= new CMenuOptionChooser(LOCALE_MISCSETTINGS_CHANNELLIST_EPGTEXT_ALIGN, &g_settings.channellist_epgtext_align_right, CHANNELLIST_EPGTEXT_ALIGN_RIGHT_OPTIONS, CHANNELLIST_EPGTEXT_ALIGN_RIGHT_OPTIONS_COUNT, true);
	CMenuOptionChooser *ocls_ext_ch 	= new CMenuOptionChooser(LOCALE_CHANNELLIST_EXTENDED, &g_settings.channellist_extended, OPTIONS_OFF0_ON1_OPTIONS, OPTIONS_OFF0_ON1_OPTION_COUNT, true);

	//show items
	ocls->addItem(ocls_setup_subhead);
	ocls->addItem(GenericMenuSeparator);
	ocls->addItem(GenericMenuBack);
	ocls->addItem(GenericMenuSeparatorLine);
	//-------------------------------------------------
	ocls->addItem(ocls_align_ch);
	ocls->addItem(ocls_ext_ch);

	ocls->exec(NULL, "");
	ocls->hide();
	delete ocls;
}


/* for font size setup */
class CMenuNumberInput : public CMenuForwarder, CMenuTarget, CChangeObserver
{
private:
	CChangeObserver * observer;
	CConfigFile     * configfile;
	int32_t           defaultvalue;
	char              value[11];

protected:

	virtual const char * getOption(void)
		{
			sprintf(value, "%u", configfile->getInt32(locale_real_names[text], defaultvalue));
			return value;
		}

	virtual bool changeNotify(const neutrino_locale_t OptionName, void * Data)
		{
			configfile->setInt32(locale_real_names[text], atoi(value));
			return observer->changeNotify(OptionName, Data);
		}


public:
	CMenuNumberInput(const neutrino_locale_t Text, const int32_t DefaultValue, CChangeObserver * const Observer, CConfigFile * const Configfile) : CMenuForwarder(Text, true, NULL, this)
		{
			observer     = Observer;
			configfile   = Configfile;
			defaultvalue = DefaultValue;
		}

	int exec(CMenuTarget * parent, const std::string & action_Key)
		{
			CStringInput input(text, (char *)getOption(), 3, LOCALE_IPSETUP_HINT_1, LOCALE_IPSETUP_HINT_2, "0123456789 ", this);
			return input.exec(parent, action_Key);
		}
};

void COsdSetup::AddFontSettingItem(CMenuWidget *fontSettings, const SNeutrinoSettings::FONT_TYPES number_of_fontsize_entry)
{
	fontSettings->addItem(new CMenuNumberInput(neutrino_font[number_of_fontsize_entry].name, neutrino_font[number_of_fontsize_entry].defaultsize, fontsizenotifier, CNeutrinoApp::getInstance()->getConfigFile()));
}

/* font settings  */
#warning FIXME: change of font is broken since neutrino revision 1.1029, commit: 436fd3af9c66be82728e7b93f0518efa825521f0
void COsdSetup::showOsdFontSizeSetup()
{
	CMenuWidget * fontSettings = new CMenuWidget(menue_title, menue_icon, width);
	CMenuSeparator * fontSettings_subhead 	= new CMenuSeparator(CMenuSeparator::ALIGN_LEFT | CMenuSeparator::SUB_HEAD | CMenuSeparator::STRING, LOCALE_FONTMENU_HEAD);

	fontSettings->addItem(fontSettings_subhead);
	fontSettings->addItem(GenericMenuSeparator);
	fontSettings->addItem(GenericMenuBack);
	fontSettings->addItem(GenericMenuSeparatorLine);

	AddFontSettingItem(fontSettings, SNeutrinoSettings::FONT_TYPE_MENU_TITLE);
	AddFontSettingItem(fontSettings, SNeutrinoSettings::FONT_TYPE_MENU);
	AddFontSettingItem(fontSettings, SNeutrinoSettings::FONT_TYPE_MENU_INFO);

	fontSettings->addItem(GenericMenuSeparatorLine);

	for (int i = 0; i < 5; i++)
	{
		CMenuWidget * fontSettingsSubMenu = new CMenuWidget(font_sizes_groups[i].groupname, NEUTRINO_ICON_COLORS, width);
		fontSettingsSubMenu->addItem(GenericMenuSeparator);
		fontSettingsSubMenu->addItem(GenericMenuBack);
		fontSettingsSubMenu->addItem(GenericMenuSeparatorLine);
		for (unsigned int j = 0; j < font_sizes_groups[i].count; j++)
		{
			AddFontSettingItem(fontSettingsSubMenu, font_sizes_groups[i].content[j]);
		}
		fontSettingsSubMenu->addItem(GenericMenuSeparatorLine);
		fontSettingsSubMenu->addItem(new CMenuForwarder(LOCALE_OPTIONS_DEFAULT, true, NULL, this, font_sizes_groups[i].actionkey));

		fontSettings->addItem(new CMenuForwarder(font_sizes_groups[i].groupname, true, NULL, fontSettingsSubMenu));
	}

	AddFontSettingItem(fontSettings, SNeutrinoSettings::FONT_TYPE_FILEBROWSER_ITEM);
	fontSettings->addItem(GenericMenuSeparatorLine);
	fontSettings->addItem(new CMenuForwarder(LOCALE_OPTIONS_DEFAULT, true, NULL, this, font_sizes_groups[5].actionkey));


	fontSettings->exec(NULL, "");
	fontSettings->hide();
	delete fontSettings;
}

