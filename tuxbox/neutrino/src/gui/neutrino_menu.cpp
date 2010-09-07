/*
	$Id: neutrino_menu.cpp,v 1.117 2010/09/07 09:59:06 dbt Exp $
	
	Neutrino-GUI  -   DBoxII-Project

	Copyright (C) 2001 Steffen Hehn 'McClean'
	Homepage: http://dbox.cyberphoria.org/

	Copyright (C) 2008, 2009 Stefan Seyfried

	Kommentar:

	Diese GUI wurde von Grund auf neu programmiert und sollte nun vom
	Aufbau und auch den Ausbaumoeglichkeiten gut aussehen. Neutrino basiert
	auf der Client-Server Idee, diese GUI ist also von der direkten DBox-
	Steuerung getrennt. Diese wird dann von Daemons uebernommen.


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

#include <global.h>
#include <neutrino.h>

#include <system/debug.h>

#include <driver/encoding.h>
#include <driver/vcrcontrol.h>

#include "gui/bedit/bouqueteditor_bouquets.h"
#include "gui/widget/colorchooser.h"
#include "gui/widget/dirchooser.h"
#include "gui/widget/icons.h"
#include "gui/widget/keychooser.h"
#include "gui/widget/rgbcsynccontroler.h"
#include "gui/widget/stringinput.h"
#include "gui/widget/stringinput_ext.h"

#include "movieplayer_setup.h"
#include "audio_setup.h"
#include "video_setup.h"
#include "audio_select.h"
#ifdef ENABLE_AUDIOPLAYER
#include "audioplayer.h"
#endif
#ifdef ENABLE_ESD
#include "esound.h"
#endif
#ifdef ENABLE_EPGPLUS
#include "epgplus.h"
#endif
#include "favorites.h"
#include "imageinfo.h"
#include "keyhelper.h"
#if defined(ENABLE_AUDIOPLAYER) || defined(ENABLE_INTERNETRADIO) || defined(ENABLE_ESD) || defined(ENABLE_PICTUREVIEWER) || defined (ENABLE_MOVIEPLAYER)
#include "mediaplayer_setup.h"
#endif
#ifdef ENABLE_MOVIEPLAYER
#include "movieplayer.h"
#include "movieplayer_menu.h"
#include "movieplayer_setup.h"
#endif

#include "network_setup.h"
#ifdef ENABLE_GUI_MOUNT
#include "nfs.h"
#endif
#include "parentallock_setup.h"
#include "personalize.h"
#ifdef ENABLE_PICTUREVIEWER
#include "pictureviewer.h"
#endif
#include "pluginlist.h"
#include "record_setup.h"
#include "miscsettings_menu.h"
#include "scan_setup.h"
#include "screensetup.h"
#include "software_update.h"
#include "streaminfo2.h"
#include "sleeptimer.h"
#include "update.h"
#include "keybind_setup.h"
#include "lcd_setup.h"
#include "driver_boot_setup.h"
#include "osd_setup.h"


#if ENABLE_UPNP
#include "upnpbrowser.h"
#endif

#ifdef _EXPERIMENTAL_SETTINGS_
#include "experimental_menu.h"
#endif

// TODO: k26 support for ENABLE_DRIVE_GUI, it's disabled with -enable-kernel26 yet
#if ENABLE_DRIVE_GUI
#include "drive_setup.h"
#endif /*ENABLE_DRIVE_GUI*/


// static CTimingSettingsNotifier timingsettingsnotifier;

/**************************************************************************************
*                                                                                     *
*          CNeutrinoApp -  init main menu                                             *
*                                                                                     *
**************************************************************************************/

void CNeutrinoApp::InitMainMenu(CMenuWidget &mainMenu, CMenuWidget &mainSettings, CMenuWidget &service)
{
	dprintf(DEBUG_DEBUG, "init mainmenue\n");

	// Dynamic renumbering
	int shortcut = 1;
	int shortcut2 = 1;

	CPersonalizeGui	*personalize;
	personalize = new CPersonalizeGui;
	
	// Main Menu
	mainMenu.addItem(GenericMenuSeparator);

	//tv-mode
 	personalize->addItem(mainMenu, LOCALE_MAINMENU_TVMODE, true, NULL, this, "tv", CRCInput::RC_red, NEUTRINO_ICON_BUTTON_RED, (g_settings.startmode == STARTMODE_TV || firstchannel.mode != 'r' ), g_settings.personalize_tvmode);

	//radio mode
	personalize->addItem(mainMenu, LOCALE_MAINMENU_RADIOMODE, true, NULL, this, "radio", CRCInput::RC_green, NEUTRINO_ICON_BUTTON_GREEN, (g_settings.startmode == STARTMODE_RADIO || (!(g_settings.startmode == STARTMODE_TV) && firstchannel.mode == 'r')),g_settings.personalize_radiomode);

	//scart
	personalize->addItem(mainMenu, LOCALE_MAINMENU_SCARTMODE, true, NULL, this, "scart", CRCInput::RC_yellow, NEUTRINO_ICON_BUTTON_YELLOW, false, g_settings.personalize_scartmode);

	//games
	personalize->addItem(mainMenu, LOCALE_MAINMENU_GAMES, true, NULL, new CPluginList(LOCALE_MAINMENU_GAMES,CPlugins::P_TYPE_GAME), "", CRCInput::RC_blue, NEUTRINO_ICON_BUTTON_BLUE, false, g_settings.personalize_games);

	//separator	
	if (	g_settings.personalize_tvmode		== CPersonalizeGui::PERSONALIZE_MODE_NOTVISIBLE && 
		g_settings.personalize_radiomode	== CPersonalizeGui::PERSONALIZE_MODE_NOTVISIBLE && 
		g_settings.personalize_scartmode	== CPersonalizeGui::PERSONALIZE_MODE_NOTVISIBLE && 
		g_settings.personalize_games		== CPersonalizeGui::PERSONALIZE_MODE_NOTVISIBLE)
		;// Stop seperator from appearing when menu entries have been hidden	
	else
		mainMenu.addItem(GenericMenuSeparatorLine); 
	
#ifdef ENABLE_AUDIOPLAYER
	// audioplayer
	shortcut += personalize->addItem(mainMenu, LOCALE_MAINMENU_AUDIOPLAYER, true, NULL, new CAudioPlayerGui(), NULL, CRCInput::convertDigitToKey(shortcut), NULL, false,   g_settings.personalize_audioplayer);
	
#ifdef ENABLE_INTERNETRADIO
	// internet player
	shortcut += personalize->addItem(mainMenu, LOCALE_INETRADIO_NAME, true, NULL, new CAudioPlayerGui(true), NULL, CRCInput::convertDigitToKey(shortcut), NULL, false, g_settings.personalize_inetradio);
#endif
#endif	

#ifdef ENABLE_ESD
	// esound
	if (access("/bin/esd", X_OK) == 0 || access("/var/bin/esd", X_OK) == 0)
	{
		puts("[neutrino] found esound, adding personalized esound entry to mainmenue");
		shortcut += personalize->addItem(mainMenu, LOCALE_ESOUND_NAME, true, NULL, new CEsoundGui(), NULL, CRCInput::convertDigitToKey(shortcut), NULL, false, g_settings.personalize_esound);
	}
#endif

#ifdef ENABLE_MOVIEPLAYER
	// movieplayer
	shortcut += personalize->addItem(mainMenu, LOCALE_MAINMENU_MOVIEPLAYER, true, NULL, new CMoviePlayerMenue(), NULL, CRCInput::convertDigitToKey(shortcut), NULL, false, g_settings.personalize_movieplayer);
#endif

#ifdef ENABLE_PICTUREVIEWER
	// pictureviewer
	shortcut += personalize->addItem(mainMenu, LOCALE_MAINMENU_PICTUREVIEWER, true, NULL, new CPictureViewerGui(), NULL, CRCInput::convertDigitToKey(shortcut), NULL, false, g_settings.personalize_pictureviewer);
#endif

#if ENABLE_UPNP
	// upnpbrowser
	shortcut += personalize->addItem(mainMenu, LOCALE_MAINMENU_UPNPBROWSER, true, NULL, new CUpnpBrowserGui(), NULL, CRCInput::convertDigitToKey(shortcut), NULL, false, g_settings.personalize_upnpbrowser);
#endif

	// scripts
	if (g_PluginList->hasPlugin(CPlugins::P_TYPE_SCRIPT))
		mainMenu.addItem(new CMenuForwarder(LOCALE_MAINMENU_SCRIPTS, true, NULL, new CPluginList(LOCALE_MAINMENU_SCRIPTS,CPlugins::P_TYPE_SCRIPT), "", CRCInput::convertDigitToKey(shortcut++)));

#if defined(ENABLE_AUDIOPLAYER) || defined(ENABLE_INTERNETRADIO) || defined(ENABLE_ESD) || defined(ENABLE_MOVIEPLAYER) || defined(ENABLE_PICTUREVIEWER) || defined(ENABLE_UPNP)
	// separator
	if (	g_settings.personalize_audioplayer	== CPersonalizeGui::PERSONALIZE_MODE_NOTVISIBLE && 
		g_settings.personalize_inetradio	== CPersonalizeGui::PERSONALIZE_MODE_NOTVISIBLE && 
		g_settings.personalize_esound		== CPersonalizeGui::PERSONALIZE_MODE_NOTVISIBLE && 
		g_settings.personalize_movieplayer	== CPersonalizeGui::PERSONALIZE_MODE_NOTVISIBLE && 
		g_settings.personalize_pictureviewer	== CPersonalizeGui::PERSONALIZE_MODE_NOTVISIBLE && 
		g_settings.personalize_upnpbrowser	== CPersonalizeGui::PERSONALIZE_MODE_NOTVISIBLE)
		;// Stop seperator from appearing when menu entries have been hidden
	else
		mainMenu.addItem(GenericMenuSeparatorLine); 
#endif
	// settings
	shortcut += personalize->addItem(mainMenu, LOCALE_MAINMENU_SETTINGS, true, NULL, &mainSettings, NULL, CRCInput::convertDigitToKey(shortcut), NULL, false, CPersonalizeGui::PERSONALIZE_MODE_VISIBLE, g_settings.personalize_settings);

	// service
	shortcut += personalize->addItem(mainMenu, LOCALE_MAINMENU_SERVICE, true, NULL, &service, NULL, CRCInput::convertDigitToKey(shortcut), NULL, false, CPersonalizeGui::PERSONALIZE_MODE_VISIBLE, g_settings.personalize_service);

	//separator
	if (	g_settings.personalize_sleeptimer	== CPersonalizeGui::PERSONALIZE_MODE_NOTVISIBLE && 
		g_settings.personalize_reboot		== CPersonalizeGui::PERSONALIZE_MODE_NOTVISIBLE && 
		g_settings.personalize_shutdown		== CPersonalizeGui::PERSONALIZE_MODE_NOTVISIBLE)
		 ;// Stop seperator from appearing when menu entries have been hidden
	else
		mainMenu.addItem(GenericMenuSeparatorLine);

	//10. -- only 10 shortcuts (1-9, 0), the next could be the last also!(10. => 0)
	//sleeptimer
	shortcut += personalize->addItem(mainMenu, LOCALE_MAINMENU_SLEEPTIMER, true, NULL, new CSleepTimerWidget, NULL, personalize->setShortcut(shortcut), NULL, false, g_settings.personalize_sleeptimer);

	// reboot
	shortcut += personalize->addItem(mainMenu, LOCALE_MAINMENU_REBOOT, true, NULL, this, "reboot", personalize->setShortcut(shortcut), NULL, false, g_settings.personalize_reboot);

	// shutdown
	personalize->addItem(mainMenu, LOCALE_MAINMENU_SHUTDOWN, true, NULL, this, "shutdown", CRCInput::RC_standby, NEUTRINO_ICON_BUTTON_POWER, false, g_settings.personalize_shutdown);

//----------------------
	//red Settings Menu
	addMenueIntroItems(mainSettings);
	mainSettings.addItem(new CMenuForwarder(LOCALE_MAINSETTINGS_SAVESETTINGSNOW, true, NULL, this, "savesettings", CRCInput::RC_red, NEUTRINO_ICON_BUTTON_RED));
	mainSettings.addItem(GenericMenuSeparatorLine);

	// video.
	shortcut2 += personalize->addItem(mainSettings, LOCALE_MAINSETTINGS_VIDEO, true, NULL, new CVideoSetup(), NULL, CRCInput::convertDigitToKey(shortcut2), NULL, false, g_settings.personalize_video);	

	// audio
	shortcut2 += personalize->addItem(mainSettings, LOCALE_MAINSETTINGS_AUDIO, true, NULL, new CAudioSetup(), NULL, CRCInput::convertDigitToKey(shortcut2), NULL, false, g_settings.personalize_audio);
	
	// parental lock
	if (g_settings.personalize_youth == CPersonalizeGui::PERSONALIZE_MODE_VISIBLE) {
		if(g_settings.parentallock_prompt)
			mainSettings.addItem(new CLockedMenuForwarder(LOCALE_PARENTALLOCK_PARENTALLOCK, g_settings.parentallock_pincode, true, true, NULL, new CParentalSetup(), NULL, CRCInput::convertDigitToKey(shortcut2++)));
		else
			mainSettings.addItem(new CMenuForwarder(LOCALE_PARENTALLOCK_PARENTALLOCK, true, NULL, new CParentalSetup(), NULL, CRCInput::convertDigitToKey(shortcut2++)));
 	}
	else if (g_settings.personalize_youth == CPersonalizeGui::PERSONALIZE_MODE_PIN)
		mainSettings.addItem(new CLockedMenuForwarder(LOCALE_PARENTALLOCK_PARENTALLOCK, g_settings.personalize_pincode, true, true, NULL, new CParentalSetup(), NULL, CRCInput::convertDigitToKey(shortcut2++)));

	// network
	if(networksetup == NULL)
		networksetup = new CNetworkSetup();
	shortcut2 += personalize->addItem(mainSettings, LOCALE_MAINSETTINGS_NETWORK, true, NULL, networksetup, NULL, CRCInput::convertDigitToKey(shortcut2), NULL, false, g_settings.personalize_network);

	// record settings
	shortcut2 += personalize->addItem(mainSettings, LOCALE_MAINSETTINGS_RECORDING, true, NULL, new CRecordSetup(), NULL, CRCInput::convertDigitToKey(shortcut2), NULL, false,g_settings.personalize_recording);

	// osd
	shortcut2 += personalize->addItem(mainSettings, LOCALE_MAINSETTINGS_OSD, true, NULL, new COsdSetup(LOCALE_MAINMENU_SETTINGS), NULL, CRCInput::convertDigitToKey(shortcut2), NULL, false, g_settings.personalize_colors);
	
	// lcd.
	shortcut2 += personalize->addItem(mainSettings,LOCALE_MAINSETTINGS_LCD, true, NULL, new CLcdSetup(LOCALE_MAINMENU_SETTINGS), NULL, CRCInput::convertDigitToKey(shortcut2), NULL, false, g_settings.personalize_lcd);
	
	//10. -- only 10 shortcuts (1-9, 0), the next could be the last also!(10. => 0)
	//keybindings
	shortcut2 += personalize->addItem(mainSettings,LOCALE_MAINSETTINGS_KEYBINDING, true, NULL, new CKeybindSetup(LOCALE_MAINMENU_SETTINGS) , NULL, personalize->setShortcut(shortcut2), NULL, false, g_settings.personalize_keybinding);

#ifdef ENABLE_DRIVE_GUI
	// ide, hdd, mmc setup
	shortcut2 += personalize->addItem(mainSettings, LOCALE_DRIVE_SETUP_HEAD, true, NULL, CDriveSetup::getInstance(), NULL, CRCInput::convertDigitToKey(shortcut2), NULL, false, g_settings.personalize_drive_setup_stat); 
#endif /*ENABLE_DRIVE_GUI*/
	
	//blue (audioplayer, pictureviewer, esd, mediaplayer)
#if defined(ENABLE_AUDIOPLAYER) || defined(ENABLE_PICTUREVIEWER) || defined(ENABLE_ESD) || defined(ENABLE_MOVIEPLAYER)
	personalize->addItem(mainSettings, LOCALE_MEDIAPLAYERSETTINGS_GENERAL, true, NULL, new CMediaPlayerSetup, NULL, CRCInput::RC_blue, NEUTRINO_ICON_BUTTON_BLUE, false, g_settings.personalize_mediaplayer);
#endif

	//green (driver/boot settings)
	personalize->addItem(mainSettings, LOCALE_MAINSETTINGS_DRIVER, true, NULL, new CDriverBootSetup(LOCALE_MAINMENU_SETTINGS), NULL, CRCInput::RC_green, NEUTRINO_ICON_BUTTON_GREEN, false, g_settings.personalize_driver);
	
	//yellow (miscSettings)
	personalize->addItem(mainSettings, LOCALE_MAINSETTINGS_MISC, true, NULL, new CMiscMenue(LOCALE_MAINMENU_SETTINGS), NULL, CRCInput::RC_yellow, NEUTRINO_ICON_BUTTON_YELLOW, false, g_settings.personalize_misc);
	
	// personalize
	shortcut2 += personalize->addItem(mainSettings, LOCALE_PERSONALIZE_HEAD, true, NULL, new CPersonalizeGui(), NULL, personalize->setShortcut(shortcut2), NULL, false, CPersonalizeGui::PERSONALIZE_MODE_VISIBLE, g_settings.personalize_pinstatus);

	delete personalize ;
}

const CMenuOptionChooser::keyval OPTIONS_OFF0_ON1_OPTIONS[OPTIONS_OFF0_ON1_OPTION_COUNT] =
{
	{ 0, LOCALE_OPTIONS_OFF },
	{ 1, LOCALE_OPTIONS_ON  }
};

/* service menu*/
void CNeutrinoApp::InitServiceSettings(CMenuWidget &service)
{
	dprintf(DEBUG_DEBUG, "init serviceSettings\n");

	CPersonalizeGui	*personalize;
	personalize = new CPersonalizeGui;

	// Dynamic renumbering
	int shortcut3 = 1;

	addMenueIntroItems(service);

	// bouquets
	personalize->addItem(service, LOCALE_BOUQUETEDITOR_NAME, true, NULL, new CBEBouquetWidget(), NULL, CRCInput::RC_red, NEUTRINO_ICON_BUTTON_RED, false, g_settings.personalize_bouqueteditor);

	// channel scan
	personalize->addItem(service, LOCALE_SERVICEMENU_SCANTS, true, NULL, new CScanSetup, NULL, CRCInput::RC_green, NEUTRINO_ICON_BUTTON_GREEN, false, g_settings.personalize_scants);

	// separator
	if (	g_settings.personalize_bouqueteditor	== CPersonalizeGui::PERSONALIZE_MODE_NOTVISIBLE && 
		g_settings.personalize_scants		== CPersonalizeGui::PERSONALIZE_MODE_NOTVISIBLE);
		// Stop seperator from appearing when menu entries have been hidden
	else
		service.addItem(GenericMenuSeparatorLine); 

	// reload channels
	shortcut3 += personalize->addItem(service, LOCALE_SERVICEMENU_RELOAD, true, NULL, this, "reloadchannels", CRCInput::convertDigitToKey(shortcut3), NULL, false, g_settings.personalize_reload);

	// reload plugins
	shortcut3 += personalize->addItem(service, LOCALE_SERVICEMENU_GETPLUGINS, true, NULL, this, "reloadplugins" , CRCInput::convertDigitToKey(shortcut3), NULL, false, g_settings.personalize_getplugins);
	
	// restart neutrino
	shortcut3 += personalize->addItem(service, LOCALE_SERVICEMENU_RESTART, true, NULL, this, "restart", CRCInput::convertDigitToKey(shortcut3), NULL, false, g_settings.personalize_restart);
	
	// epg restart
	shortcut3 += personalize->addItem(service, LOCALE_SERVICEMENU_EPGRESTART, true, NULL, this, "EPGrestart", CRCInput::convertDigitToKey(shortcut3), NULL, false, g_settings.personalize_epgrestart);
	
#ifdef HAVE_DBOX_HARDWARE
	// ucode check
	shortcut3 += personalize->addItem(service, LOCALE_SERVICEMENU_UCODECHECK, true, NULL, UCodeChecker, NULL, CRCInput::convertDigitToKey(shortcut3), NULL, false, g_settings.personalize_ucodecheck);
#endif

	// epg status
	shortcut3 += personalize->addItem(service, LOCALE_SERVICEMENU_CHAN_EPG_STAT, true, NULL, DVBInfo, NULL, CRCInput::convertDigitToKey(shortcut3), NULL, false, g_settings.personalize_chan_epg_stat);

	// separator
	if (	g_settings.personalize_reload		== CPersonalizeGui::PERSONALIZE_MODE_NOTVISIBLE && 
		g_settings.personalize_getplugins	== CPersonalizeGui::PERSONALIZE_MODE_NOTVISIBLE && 
		g_settings.personalize_restart		== CPersonalizeGui::PERSONALIZE_MODE_NOTVISIBLE && 
		g_settings.personalize_epgrestart	== CPersonalizeGui::PERSONALIZE_MODE_NOTVISIBLE && 
		g_settings.personalize_ucodecheck	== CPersonalizeGui::PERSONALIZE_MODE_NOTVISIBLE && 
		g_settings.personalize_chan_epg_stat	== CPersonalizeGui::PERSONALIZE_MODE_NOTVISIBLE
		);// Stop seperator from appearing when menu entries have been hidden
	else
		service.addItem(GenericMenuSeparatorLine); 

	//yellow imageinfo
	personalize->addItem(service, LOCALE_SERVICEMENU_IMAGEINFO, true, NULL, new CImageInfo(), NULL, CRCInput::RC_yellow, NEUTRINO_ICON_BUTTON_YELLOW, false, g_settings.personalize_imageinfo);

	//softupdate
	if(softupdate)
	{
		// blue software update
		personalize->addItem(service, LOCALE_SERVICEMENU_UPDATE, true, NULL, new CSoftwareUpdate(), NULL, CRCInput::RC_blue, NEUTRINO_ICON_BUTTON_BLUE, false, g_settings.personalize_update);
 	}

	delete personalize;
}


#define INFOBAR_SHOW_OPTIONS_COUNT 3
const CMenuOptionChooser::keyval  INFOBAR_SHOW_OPTIONS[INFOBAR_SHOW_OPTIONS_COUNT]=
{
	{ 0 , LOCALE_OPTIONS_OFF },
	{ 1 , LOCALE_PICTUREVIEWER_RESIZE_SIMPLE },
	{ 2 , LOCALE_PICTUREVIEWER_RESIZE_COLOR_AVERAGE }
};


#define MAINMENU_RECORDING_OPTION_COUNT 2
const CMenuOptionChooser::keyval MAINMENU_RECORDING_OPTIONS[MAINMENU_RECORDING_OPTION_COUNT] =
{
	{ 0, LOCALE_MAINMENU_RECORDING_START },
	{ 1, LOCALE_MAINMENU_RECORDING_STOP  }
};


// USERMENU
bool CNeutrinoApp::showUserMenu(int button)
{
	if(button < 0 || button >= SNeutrinoSettings::BUTTON_MAX) 
		return false;
		
	CMenuItem* menu_item = NULL;
	CKeyHelper keyhelper;
	neutrino_msg_t key = CRCInput::RC_nokey;
	const char * icon = NULL; 
	
	char id[5];
	int menu_items = 0;
	int menu_prev = -1;
	int cnt = 0;

	// define classes
	CFavorites* tmpFavorites				= NULL;
	CPauseSectionsdNotifier* tmpPauseSectionsdNotifier 	= NULL;
	CAudioSelectMenuHandler* tmpAudioSelectMenuHandler 	= NULL;
	CMenuWidget* tmpNVODSelector				= NULL;
	CStreamInfo2Handler*	tmpStreamInfo2Handler 		= NULL;
	CEventListHandler* tmpEventListHandler			= NULL;
#ifdef ENABLE_EPGPLUS
	CEPGplusHandler* tmpEPGplusHandler			= NULL;
#endif
	CEPGDataHandler* tmpEPGDataHandler			= NULL;
	
	std::string txt = g_settings.usermenu_text[button];
	if (button == SNeutrinoSettings::BUTTON_RED)
	{
		if( txt.empty() )
			txt = g_Locale->getText(LOCALE_INFOVIEWER_EVENTLIST);
	}
	else if( button == SNeutrinoSettings::BUTTON_GREEN)
	{
		if( txt.empty() )
			txt = g_Locale->getText(LOCALE_INFOVIEWER_LANGUAGES);
	}
	else if( button == SNeutrinoSettings::BUTTON_YELLOW)
	{
		if( txt.empty() ) 
			txt = g_Locale->getText((g_RemoteControl->are_subchannels) ? LOCALE_INFOVIEWER_SUBSERVICE : LOCALE_INFOVIEWER_SELECTTIME);
			//txt = g_Locale->getText(LOCALE_NVODSELECTOR_DIRECTORMODE);
	}
	else if( button == SNeutrinoSettings::BUTTON_BLUE)
	{	
		if( txt.empty() )
			txt = g_Locale->getText(LOCALE_INFOVIEWER_STREAMINFO);
	}
	CMenuWidget *menu = new CMenuWidget(txt.c_str() , NEUTRINO_ICON_FEATURES, 400);
	if (menu == NULL) 
		return 0;
	menu->addItem(GenericMenuSeparator);
	
	// go through any postition number
	for(int pos = 0; pos < SNeutrinoSettings::ITEM_MAX ; pos++)
	{
		int dummy;
		// now compare pos with the position of any item. Add this item if position is the same 
		switch(g_settings.usermenu[button][pos])
		{
			case SNeutrinoSettings::ITEM_NONE: 
				// do nothing 
				break;

			case SNeutrinoSettings::ITEM_BAR: 
				if(menu_prev == -1 && menu_prev == SNeutrinoSettings::ITEM_BAR )
					break;
					
				menu->addItem(GenericMenuSeparatorLine);
				menu_prev = SNeutrinoSettings::ITEM_BAR;
				break;

			case SNeutrinoSettings::ITEM_VTXT:
				for(unsigned int count = 0; count < (unsigned int) g_PluginList->getNumberOfPlugins(); count++)
				{
					std::string tmp = g_PluginList->getName(count);
					if (g_PluginList->getType(count)== CPlugins::P_TYPE_TOOL && !g_PluginList->isHidden(count) && tmp.find("Teletext") != std::string::npos)
					{
						sprintf(id, "%d", count);
						menu_items++;
						menu_prev = SNeutrinoSettings::ITEM_VTXT;
						keyhelper.get(&key, &icon, cnt == 0 ? CRCInput::RC_blue : CRCInput::RC_nokey);
						menu_item = new CMenuForwarderNonLocalized(g_PluginList->getName(count), true, NULL, StreamFeaturesChanger, id, key, icon);
						menu->addItem(menu_item, (cnt == 0));
						cnt++;
					}
				}
				break;

			case SNeutrinoSettings::ITEM_PLUGIN:
				for(unsigned int count = 0; count < (unsigned int) g_PluginList->getNumberOfPlugins(); count++)
				{
					std::string tmp = g_PluginList->getName(count);
					if (g_PluginList->getType(count)== CPlugins::P_TYPE_TOOL && !g_PluginList->isHidden(count) && tmp.find("Teletext") == std::string::npos)
					{
						sprintf(id, "%d", count);
						menu_items++;
						menu_prev = SNeutrinoSettings::ITEM_PLUGIN;
						keyhelper.get(&key, &icon, cnt == 0 ? CRCInput::RC_blue : CRCInput::RC_nokey);
						menu_item = new CMenuForwarderNonLocalized(g_PluginList->getName(count), true, NULL, StreamFeaturesChanger, id, key, icon);
						menu->addItem(menu_item, (cnt == 0));
						cnt++;
					}
				}
				break;
				
			case SNeutrinoSettings::ITEM_FAVORITS: 
				menu_items++;
				menu_prev = SNeutrinoSettings::ITEM_FAVORITS;
				tmpFavorites = new CFavorites;
				keyhelper.get(&key,&icon);
				menu_item = new CMenuForwarder(LOCALE_FAVORITES_MENUEADD, true, NULL, tmpFavorites, "-1", key, icon);
				menu->addItem(menu_item, false);
				break;

			case SNeutrinoSettings::ITEM_RECORD: 
				if(g_settings.recording_type == RECORDING_OFF) 
					break;
				
				menu_items++;
				menu_prev = SNeutrinoSettings::ITEM_RECORD;
				keyhelper.get(&key,&icon,CRCInput::RC_red);
				menu_item = new CMenuOptionChooser(LOCALE_MAINMENU_RECORDING, &recordingstatus, MAINMENU_RECORDING_OPTIONS, MAINMENU_RECORDING_OPTION_COUNT, true, this, key, icon);
				menu->addItem(menu_item, false);
				break;

#ifdef ENABLE_MOVIEPLAYER
			case SNeutrinoSettings::ITEM_MOVIEPLAYER_TS: 
				menu_items++;
				menu_prev = SNeutrinoSettings::ITEM_MOVIEPLAYER_TS;
				keyhelper.get(&key,&icon,CRCInput::RC_green);
				menu_item = new CMenuForwarder(LOCALE_MOVIEPLAYER_TSPLAYBACK, true, NULL, new CMoviePlayerGui()/*this->moviePlayerGui*/, "tsplayback", key, icon);
				menu->addItem(menu_item, false);
				break;

			case SNeutrinoSettings::ITEM_MOVIEPLAYER_MB: 
				menu_items++;
				menu_prev = SNeutrinoSettings::ITEM_MOVIEPLAYER_MB;
				keyhelper.get(&key,&icon,CRCInput::RC_green);
#ifndef ENABLE_MOVIEPLAYER2
				menu_item = new CMenuForwarder(LOCALE_MOVIEBROWSER_HEAD, true, NULL, new CMoviePlayerGui(), "tsmoviebrowser", key, icon);
#else
				menu_item = new CMenuForwarder(LOCALE_MOVIEBROWSER_HEAD, true, NULL, CMovieBrowser::getInstance(), "run", key, icon);
#endif
				menu->addItem(menu_item, false);
				break;
#endif

			case SNeutrinoSettings::ITEM_TIMERLIST: 
				menu_items++;
				menu_prev = SNeutrinoSettings::ITEM_TIMERLIST;
				keyhelper.get(&key,&icon,CRCInput::RC_yellow);
				menu_item = new CMenuForwarder(LOCALE_TIMERLIST_NAME, true, NULL, Timerlist, "-1", key, icon);
				menu->addItem(menu_item, false);
				break;

			case SNeutrinoSettings::ITEM_REMOTE: 
				menu_items++;
				menu_prev = SNeutrinoSettings::ITEM_REMOTE;
				keyhelper.get(&key,&icon);
				menu_item = new CMenuForwarder(LOCALE_RCLOCK_MENUEADD, true, NULL, this->rcLock, "-1" , key, icon );
				menu->addItem(menu_item, false);
				break;

#ifdef ENABLE_EPGPLUS
			case SNeutrinoSettings::ITEM_EPG_SUPER: 
				menu_items++;
				menu_prev = SNeutrinoSettings::ITEM_EPG_SUPER;
				tmpEPGplusHandler = new CEPGplusHandler();
				keyhelper.get(&key,&icon,CRCInput::RC_green);
				menu_item = new CMenuForwarder(LOCALE_EPGMENU_EPGPLUS   , true, NULL, tmpEPGplusHandler  ,  "-1", key, icon);
				menu->addItem(menu_item, false);
				break;
#endif

			case SNeutrinoSettings::ITEM_EPG_LIST: 
				menu_items++;
				menu_prev = SNeutrinoSettings::ITEM_EPG_LIST;
				tmpEventListHandler = new CEventListHandler();
				keyhelper.get(&key,&icon,CRCInput::RC_red);
				menu_item = new CMenuForwarder(LOCALE_EPGMENU_EVENTLIST , true, NULL, tmpEventListHandler,  "-1", key, icon);
				menu->addItem(menu_item, false);
				break;

			case SNeutrinoSettings::ITEM_EPG_INFO: 
				menu_items++;
				menu_prev = SNeutrinoSettings::ITEM_EPG_INFO;
				tmpEPGDataHandler = new CEPGDataHandler();
				keyhelper.get(&key,&icon,CRCInput::RC_yellow);
				menu_item = new CMenuForwarder(LOCALE_EPGMENU_EVENTINFO , true, NULL, tmpEPGDataHandler ,  "-1", key, icon);
				menu->addItem(menu_item, false);
				break;

			case SNeutrinoSettings::ITEM_EPG_MISC: 
				menu_items++;
				menu_prev = SNeutrinoSettings::ITEM_EPG_MISC;
				dummy = g_Sectionsd->getIsScanningActive();
				tmpPauseSectionsdNotifier = new CPauseSectionsdNotifier;
				keyhelper.get(&key,&icon);
				menu_item = new CMenuOptionChooser(LOCALE_MAINMENU_PAUSESECTIONSD, &dummy, OPTIONS_OFF0_ON1_OPTIONS, OPTIONS_OFF0_ON1_OPTION_COUNT, true, tmpPauseSectionsdNotifier , key, icon );
				menu->addItem(menu_item, false);
				menu_items++;
				keyhelper.get(&key,&icon);
				menu_item = new CMenuForwarder(LOCALE_MAINMENU_CLEARSECTIONSD, true, NULL, this, "clearSectionsd", key,icon);
				menu->addItem(menu_item, false);
				break;
				
			case SNeutrinoSettings::ITEM_AUDIO_SELECT: 
				// -- new Audio Selector Menu (rasc 2005-08-30)
				if (g_settings.audio_left_right_selectable ||
				    g_RemoteControl->current_PIDs.APIDs.size() > 1)
				{
					menu_items++;
					menu_prev = SNeutrinoSettings::ITEM_AUDIO_SELECT;
					tmpAudioSelectMenuHandler = new CAudioSelectMenuHandler;
					keyhelper.get(&key,&icon);
					menu_item = new CMenuForwarder(LOCALE_AUDIOSELECTMENUE_HEAD, true, NULL, tmpAudioSelectMenuHandler, "-1", key,icon);
					menu->addItem(menu_item, false);
				}
				break;
				
			case SNeutrinoSettings::ITEM_SUBCHANNEL: 
				if (!(g_RemoteControl->subChannels.empty()))
				{
					// NVOD/SubService- Kanal!
					tmpNVODSelector = new CMenuWidget(g_RemoteControl->are_subchannels ? LOCALE_NVODSELECTOR_SUBSERVICE : LOCALE_NVODSELECTOR_HEAD, NEUTRINO_ICON_VIDEO, 350);
					if(getNVODMenu(tmpNVODSelector))
					{
						menu_items++;
						menu_prev = SNeutrinoSettings::ITEM_SUBCHANNEL;
						keyhelper.get(&key,&icon);
						menu_item = new CMenuForwarder(g_RemoteControl->are_subchannels ? LOCALE_NVODSELECTOR_SUBSERVICE : LOCALE_NVODSELECTOR_HEAD, true, NULL, tmpNVODSelector, "-1", key,icon);
						menu->addItem(menu_item, false);
					}
				}
				break;
				
			case SNeutrinoSettings::ITEM_TECHINFO: 
				menu_items++;
				menu_prev = SNeutrinoSettings::ITEM_TECHINFO;
				tmpStreamInfo2Handler = new CStreamInfo2Handler();
				keyhelper.get(&key,&icon,CRCInput::RC_blue);
				menu_item = new CMenuForwarder(LOCALE_EPGMENU_STREAMINFO, true, NULL, tmpStreamInfo2Handler    , "-1", key, icon );
				menu->addItem(menu_item, false);
				break;

			default: 
				printf("[neutrino] WARNING! menu wrong item!!\n");
				break;
		}
	} 
	
	// Allow some tailoring for privat image bakers ;)
	if (button == SNeutrinoSettings::BUTTON_RED)
	{
	}
	else if( button == SNeutrinoSettings::BUTTON_GREEN)
	{
	}
	else if( button == SNeutrinoSettings::BUTTON_YELLOW)
	{
	}
	else if( button == SNeutrinoSettings::BUTTON_BLUE)
	{	
#ifdef _EXPERIMENTAL_SETTINGS_
		//Experimental Settings
		if(menu_prev != -1)
			menu->addItem(GenericMenuSeparatorLine);
		menu_items ++;
		menu_key++;
		// FYI: there is a memory leak with 'new CExperimentalSettingsMenuHandler()
		menu_item = new CMenuForwarder(LOCALE_EXPERIMENTALSETTINGS, true, NULL, new CExperimentalSettingsMenuHandler(), "-1", CRCInput::convertDigitToKey(menu_key), "");
		menu->addItem(menu_item, false);
#endif
	}
	
	if(menu_items >	1 ) 				// show menu if there are more than 2 items only
	{
		menu->exec(NULL,"");
	}
	else if (menu_item != NULL)			// otherwise, we start the item directly (must be the last one)
	{
		menu_item->exec( NULL );
	}									// neither nor, we do nothing

	// restore mute symbol
	//AudioMute(current_muted, true);
	
	// clear the heap
	if(menu)			delete menu;
	if(tmpFavorites)		delete tmpFavorites;
	if(tmpPauseSectionsdNotifier)	delete tmpPauseSectionsdNotifier;
	if(tmpAudioSelectMenuHandler)	delete tmpAudioSelectMenuHandler;
	if(tmpNVODSelector)		delete tmpNVODSelector;
	if(tmpStreamInfo2Handler)	delete tmpStreamInfo2Handler;
	if(tmpEventListHandler)		delete tmpEventListHandler;
#ifdef ENABLE_EPGPLUS
	if(tmpEPGplusHandler)		delete tmpEPGplusHandler;
#endif
	if(tmpEPGDataHandler)		delete tmpEPGDataHandler;
	return true;
}

// should be remove, now just for reference
void CNeutrinoApp::ShowStreamFeatures()
{
	char id[5];
	int cnt = 0;
	int enabled_count = 0;
	CMenuWidget *StreamFeatureSelector = new CMenuWidget(LOCALE_STREAMFEATURES_HEAD, NEUTRINO_ICON_FEATURES, 350);
	if (StreamFeatureSelector == NULL) return;

	StreamFeatureSelector->addItem(GenericMenuSeparator);

	for(unsigned int count=0;count < (unsigned int) g_PluginList->getNumberOfPlugins();count++)
	{
		if (g_PluginList->getType(count)== CPlugins::P_TYPE_TOOL && !g_PluginList->isHidden(count))
		{
			// zB vtxt-plugins

			sprintf(id, "%d", count);

			enabled_count++;

			StreamFeatureSelector->addItem(new CMenuForwarderNonLocalized(g_PluginList->getName(count), true, NULL, StreamFeaturesChanger, id, (cnt== 0) ? CRCInput::RC_blue : CRCInput::convertDigitToKey(enabled_count-1), (cnt == 0) ? NEUTRINO_ICON_BUTTON_BLUE : ""), (cnt == 0));
			cnt++;
		}
	}

	if(cnt>0)
	{
		StreamFeatureSelector->addItem(GenericMenuSeparatorLine);
	}

	sprintf(id, "%d", -1);

	// -- Add Channel to favorites
//	StreamFeatureSelector.addItem(new CMenuForwarder(LOCALE_FAVORITES_MENUEADD, true, NULL, new CFavorites, id, CRCInput::RC_green, NEUTRINO_ICON_BUTTON_GREEN), false);
	CFavorites *tmpFavorites = new CFavorites;
	StreamFeatureSelector->addItem(new CMenuForwarder(LOCALE_FAVORITES_MENUEADD, true, NULL, tmpFavorites, id, CRCInput::convertDigitToKey(enabled_count), ""), false);

	StreamFeatureSelector->addItem(GenericMenuSeparatorLine);

	// start/stop recording
	if (g_settings.recording_type != RECORDING_OFF)
	{
		StreamFeatureSelector->addItem(new CMenuOptionChooser(LOCALE_MAINMENU_RECORDING, &recordingstatus, MAINMENU_RECORDING_OPTIONS, MAINMENU_RECORDING_OPTION_COUNT, true, this, CRCInput::RC_red, NEUTRINO_ICON_BUTTON_RED));
	}

#ifdef ENABLE_MOVIEPLAYER
	// -- Add TS Playback to blue button
	StreamFeatureSelector->addItem(new CMenuForwarder(LOCALE_MOVIEPLAYER_TSPLAYBACK, true, NULL, new CMoviePlayerGui(), "tsplayback", CRCInput::RC_green, NEUTRINO_ICON_BUTTON_GREEN), false);
#endif

	// -- Timer-Liste
	CTimerList *tmpTimerlist = new CTimerList;
	StreamFeatureSelector->addItem(new CMenuForwarder(LOCALE_TIMERLIST_NAME, true, NULL, tmpTimerlist, id, CRCInput::RC_yellow, NEUTRINO_ICON_BUTTON_YELLOW), false);

	StreamFeatureSelector->addItem(GenericMenuSeparatorLine);

	// --  Lock remote control
	StreamFeatureSelector->addItem(new CMenuForwarder(LOCALE_RCLOCK_MENUEADD, true, NULL, this->rcLock, id, CRCInput::RC_nokey, ""), false);

	// -- Sectionsd pause
	int dummy = g_Sectionsd->getIsScanningActive();
	CMenuOptionChooser* oj = new CMenuOptionChooser(LOCALE_MAINMENU_PAUSESECTIONSD, &dummy, OPTIONS_OFF0_ON1_OPTIONS, OPTIONS_OFF0_ON1_OPTION_COUNT, true, new CPauseSectionsdNotifier );
	StreamFeatureSelector->addItem(oj);

#ifdef _EXPERIMENTAL_SETTINGS_
	//Experimental Settings
	StreamFeatureSelector->addItem(new CMenuForwarder(LOCALE_EXPERIMENTALSETTINGS, true, NULL, new CExperimentalSettingsMenuHandler(), id, CRCInput::RC_nokey, ""), false);
#endif

	StreamFeatureSelector->exec(NULL,"");

	// restore mute symbol
	delete StreamFeatureSelector;
	delete tmpFavorites;
	delete tmpTimerlist;
}


// USERMENU
// leave this functions, somebody might want to use it in the future again
void CNeutrinoApp::SelectNVOD()
{
	if (!(g_RemoteControl->subChannels.empty()))
	{
		// NVOD/SubService- Kanal!
		CMenuWidget NVODSelector(g_RemoteControl->are_subchannels ? LOCALE_NVODSELECTOR_SUBSERVICE : LOCALE_NVODSELECTOR_HEAD, NEUTRINO_ICON_VIDEO, 350);
		if(getNVODMenu(&NVODSelector))
			NVODSelector.exec(NULL, "");
	}
}


bool CNeutrinoApp::getNVODMenu(CMenuWidget* menu)
{
	if(menu == NULL) 
		return false;
	if (g_RemoteControl->subChannels.empty())
		return false;
		
	menu->addItem(GenericMenuSeparator);
	menu->addItem(GenericMenuCancel);
	menu->addItem(GenericMenuSeparatorLine);

	int count = 0;
	char nvod_id[5];

	for( CSubServiceListSorted::iterator e=g_RemoteControl->subChannels.begin(); e!=g_RemoteControl->subChannels.end(); ++e)
	{
		sprintf(nvod_id, "%d", count);

		if( !g_RemoteControl->are_subchannels )
		{
			char nvod_time_a[50], nvod_time_e[50], nvod_time_x[50];
			char nvod_s[100];
			struct  tm *tmZeit;

			tmZeit= localtime(&e->startzeit);
			sprintf(nvod_time_a, "%02d:%02d", tmZeit->tm_hour, tmZeit->tm_min);

			time_t endtime = e->startzeit+ e->dauer;
			tmZeit= localtime(&endtime);
			sprintf(nvod_time_e, "%02d:%02d", tmZeit->tm_hour, tmZeit->tm_min);

			time_t jetzt=time(NULL);
			if(e->startzeit > jetzt)
			{
				int mins=(e->startzeit- jetzt)/ 60;
				sprintf(nvod_time_x, g_Locale->getText(LOCALE_NVOD_STARTING), mins);
			}
			else
				if( (e->startzeit<= jetzt) && (jetzt < endtime) )
			{
				int proz=(jetzt- e->startzeit)*100/ e->dauer;
				sprintf(nvod_time_x, g_Locale->getText(LOCALE_NVOD_PERCENTAGE), proz);
			}
			else
				nvod_time_x[0]= 0;

			sprintf(nvod_s, "%s - %s %s", nvod_time_a, nvod_time_e, nvod_time_x);
			menu->addItem(new CMenuForwarderNonLocalized(nvod_s, true, NULL, NVODChanger, nvod_id), (count == g_RemoteControl->selected_subchannel));
		}
		else
		{
			if (count == 0)
				menu->addItem(new CMenuForwarderNonLocalized( (Latin1_to_UTF8(e->subservice_name)).c_str(), true, NULL, NVODChanger, nvod_id, CRCInput::RC_blue, NEUTRINO_ICON_BUTTON_BLUE));
			else
				menu->addItem(new CMenuForwarderNonLocalized( (Latin1_to_UTF8(e->subservice_name)).c_str(), true, NULL, NVODChanger, nvod_id, CRCInput::convertDigitToKey(count)), (count == g_RemoteControl->selected_subchannel));
		}

		count++;
	}

	if( g_RemoteControl->are_subchannels )
	{
		menu->addItem(GenericMenuSeparatorLine);
		CMenuOptionChooser* oj = new CMenuOptionChooser(LOCALE_NVODSELECTOR_DIRECTORMODE, &g_RemoteControl->director_mode, OPTIONS_OFF0_ON1_OPTIONS, OPTIONS_OFF0_ON1_OPTION_COUNT, true, NULL, CRCInput::RC_yellow, NEUTRINO_ICON_BUTTON_YELLOW);
		menu->addItem(oj);
	}

	return true;
}


void CNeutrinoApp::addMenueIntroItems(CMenuWidget &item)
/*adds the typical menu intro with separator, back button and separatorline to menu*/
{
	item.addItem(GenericMenuSeparator);
	item.addItem(GenericMenuBack);
	item.addItem(GenericMenuSeparatorLine);
}

