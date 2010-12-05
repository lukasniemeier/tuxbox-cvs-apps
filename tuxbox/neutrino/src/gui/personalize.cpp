/*
        $Id: personalize.cpp,v 1.29 2010/12/05 22:32:12 dbt Exp $

        Customization Menu - Neutrino-GUI

        Copyright (C) 2007 Speed2206
        and some other guys
        
        Reworked by dbt (Thilo Graf)
        Copyright (C) 2010 dbt

        Comment:

        This is the customization menu, as originally showcased in
        Oxygen. It is a more advanced version of the 'user levels'
        patch currently available.
        
        The reworked version >1.24 works more dynamicly with input objects
        and their parameters and it's more code reduced. It's also independent
        from #ifdefs of items. 
        The personalize-object collects all incomming item objects.
        These will be handled here and will be shown after evaluation.


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
        Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA
        
        
        Parameters:
	addItem(CMenuWidget *menu, CMenuItem *menuItem, const int *personalize_mode, const bool defaultselected, const bool item_mode),
	
	CMenuWidget *menu 		= pointer to menuewidget object
	CMenuItem *menuItem		= pointer to a menuitem object, can be forwarder, chooser, separator...all
	const int *personalize_mode	= optional, default NULL, pointer to a specified personalize setting look at: PERSONALIZE_MODE, this regulates the personalize mode
	const bool item_mode		= optional, default true, if you don't want to see this item in personalize menue, then set it to false
	
	Icon handling:
	If you define an icon in the item object, this will be shown also in the menu (not the personilazitions menue), otherwise a shortcut will be create
	
	Shortcuts.
	A start- or reset-shortcut you can create with personalize->shortcut = 1;
	
	Separators:
	You must add separators with
	addSeparator(CMenuWidget &menu, const neutrino_locale_t locale_text, const bool item_mode)
	
		Parameters:
		CMenuWidget &menu 			= rev to menuewidget object
		const neutrino_locale_t locale_text	= optional, default NONEXISTANT_LOCALE, adds a line separator, is defined a locale then adds a text separator
		const bool item_mode		= optional, default true, if you don't want to see this sparator also in personalize menue, then set it to false, usefull for to much separtors ;-)
		
	Usage:
	It's no matter what a kind of menue item (e.g. forwarder, optionchooser ...) you would like to personalize.

	Example:
	//we need an instance of CPersonalizeGUI()
	personalize = CPersonalizeGui::getInstance();

	//create start number for shortcuts
	personalize->shortcut = 1;

	//create a menue object, this will be automaticly shown as menu item in your peronalize menu
	CMenuWidget * mn =  new CMenuWidget(LOCALE_MAINMENU_HEAD, ICON    ,width);

	//create a forwarder object:
	CMenuItem *item = new CMenuForwarder(LOCALE_MAINMENU_TVMODE, true, NULL, this, "tv", CRCInput::RC_red, NEUTRINO_ICON_BUTTON_RED);

	//now you can add this to personalization
	personalize->addItem(&mn, tvswitch, &g_settings.personalize_tvmode);

	//if you want to add a separator use this function, but this doesn't personalize it, but you must use this anstead addItem(GenericMenuSeparatorLine)  
	personalize->addSeparator(mn);
	//otherwise you can add a separator at this kind:
	personalize->addItem(&mn, GenericMenuSeparatorLine);

	//to show the items at screen, we must add the menue items
	personalize->addPersonalizedItems();
	//this member makes the same like mn->addItem(...) known from CMenuWidget()-class, but for all collected and evaluated objects
	
	//reset shortcuts:
	personalize->shortcut = 1;

*/

#include <global.h>
#include <neutrino.h>

#include <stdio.h>
#include <sstream>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

#include <driver/fontrenderer.h>
#include <driver/rcinput.h>
#include <driver/screen_max.h>
#include <daemonc/remotecontrol.h>
#include <gui/widget/helpbox.h>
#include "widget/messagebox.h"
#include "widget/hintbox.h"
#include "widget/lcdcontroler.h"
#include "widget/keychooser.h"
#include "widget/stringinput.h"
#include "widget/stringinput_ext.h"
#include "color.h"
#include "personalize.h"

#define PERSONALIZE_STD_OPTION_COUNT 3
#define PERSONALIZE_EDP_OPTION_COUNT 3
#define PERSONALIZE_EOD_OPTION_COUNT 2
#define PERSONALIZE_YON_OPTION_COUNT 2


const CMenuOptionChooser::keyval PERSONALIZE_STD_OPTIONS[PERSONALIZE_STD_OPTION_COUNT] =
{
	{ 0, LOCALE_PERSONALIZE_NOTVISIBLE      },                                      // The option is NOT visible on the menu's
	{ 1, LOCALE_PERSONALIZE_VISIBLE         },                                      // The option is visible on the menu's
	{ 2, LOCALE_PERSONALIZE_PIN      },                                      // PIN Protect the item on the menu
};

const CMenuOptionChooser::keyval PERSONALIZE_EDP_OPTIONS[PERSONALIZE_EDP_OPTION_COUNT] =
{
	{ 0, LOCALE_PERSONALIZE_DISABLED        },                                      // The menu is NOT enabled / accessible
	{ 1, LOCALE_PERSONALIZE_ENABLED         },                                      // The menu is enabled / accessible
	{ 2, LOCALE_PERSONALIZE_PIN      },                                      // The menu is enabled and protected with PIN
};

const CMenuOptionChooser::keyval PERSONALIZE_EOD_OPTIONS[PERSONALIZE_EOD_OPTION_COUNT] =
{
	{ 0, LOCALE_PERSONALIZE_DISABLED        },                                      // The option is NOT enabled / accessible
	{ 1, LOCALE_PERSONALIZE_ENABLED         },                                      // The option is enabled / accessible
};

const CMenuOptionChooser::keyval PERSONALIZE_YON_OPTIONS[PERSONALIZE_YON_OPTION_COUNT] =
{
	{ 0, LOCALE_PERSONALIZE_NOTPROTECTED    },                                      // The menu/option is NOT protected
	{ 1, LOCALE_PERSONALIZE_PINPROTECT      },                                      // The menu/option is protected by a PIN
};



CPersonalizeGui::CPersonalizeGui()
{
	frameBuffer = CFrameBuffer::getInstance();
	
	width 	= w_max (710, 100);
	hheight = g_Font[SNeutrinoSettings::FONT_TYPE_MENU_TITLE]->getHeight();
	mheight = g_Font[SNeutrinoSettings::FONT_TYPE_MENU]->getHeight();
	height 	= hheight+13*mheight+ 10;
	x	= getScreenStartX (width);
	y	= getScreenStartY (height);

	selected = -1;
	shortcut = 0;
}

CPersonalizeGui* CPersonalizeGui::getInstance()
{
	static CPersonalizeGui* p = NULL;

	if(!p)
	{
		p = new CPersonalizeGui();
		printf("[neutrino] GUI-Personalize instance created...\n");
	}
	return p;
}

CPersonalizeGui::~CPersonalizeGui()
{
}

int CPersonalizeGui::exec(CMenuTarget* parent, const std::string & actionKey)
{
	int res = menu_return::RETURN_REPAINT;

	if (parent)
	{
		parent->hide();
	}

	for (uint i = 0; i<(CNeutrinoApp::MENU_MAX); i++)
	{
		ostringstream i_str;
		i_str << i;
		string s(i_str.str());
		action_key[i] = s;

		if(actionKey==action_key[i]) 
		{                                     				// Personalize options menu
			ShowMenuOptions(i);
			return res;
		}
	}
		
	if (actionKey=="personalize_help") {                                    // Personalize help
		ShowHelpPersonalize();
		return res;
	}
	
	if (actionKey=="restore") {  
		ShowPersonalizationMenu	();
		return menu_return::RETURN_EXIT_ALL;
	}
	

	ShowPersonalizationMenu();                                              // Show main Personalization Menu
	return res;
}


void CPersonalizeGui::hide()
{
	frameBuffer->paintBackgroundBoxRel(x,y, width,height);
}


//This is the main personalization menu. From here we can go to the other sub-menu's and enable/disable
//the PIN code feature, as well as determine whether or not the EPG menu/Features menu is accessible.
void CPersonalizeGui::ShowPersonalizationMenu()
{


	handleSetting(&g_settings.personalize_pinstatus);

	CMenuWidget* pMenu = new CMenuWidget(LOCALE_PERSONALIZE_HEAD,NEUTRINO_ICON_PROTECTING, width);
	pMenu->setPreselected(selected);

	CPINChangeWidget * pinChangeWidget = new CPINChangeWidget(LOCALE_PERSONALIZE_PINCODE, g_settings.personalize_pincode, 4, LOCALE_PERSONALIZE_PINHINT);

	pMenu->addItem(GenericMenuSeparator);
	pMenu->addItem(GenericMenuBack);
	pMenu->addItem(new CMenuSeparator(CMenuSeparator::LINE | CMenuSeparator::STRING, LOCALE_PERSONALIZE_MENUCONFIGURATION));
	
	pMenu->addItem(new CMenuOptionChooser(LOCALE_PERSONALIZE_PINSTATUS, (int *)&g_settings.personalize_pinstatus, PERSONALIZE_YON_OPTIONS, PERSONALIZE_YON_OPTION_COUNT, true, NULL, CRCInput::RC_red, NEUTRINO_ICON_BUTTON_RED));
	pMenu->addItem(new CMenuForwarder(LOCALE_PERSONALIZE_PINCODE, true, g_settings.personalize_pincode, pinChangeWidget, NULL, CRCInput::RC_green, NEUTRINO_ICON_BUTTON_GREEN));
	pMenu->addItem(new CMenuSeparator(CMenuSeparator::LINE | CMenuSeparator::STRING, LOCALE_PERSONALIZE_ACCESS));
	
	CMenuForwarderNonLocalized *p_mn[CNeutrinoApp::MENU_MAX];
	std::string mn_name;
	
 	for (uint i = 0; i<(CNeutrinoApp::MENU_MAX); i++)
	{
		mn_name = CNeutrinoApp::getInstance()->menus[i]->getName();
		p_mn[i] = new CMenuForwarderNonLocalized(mn_name.c_str(), true, NULL, this, action_key[i].c_str(), CRCInput::convertDigitToKey(i+1));
 		pMenu->addItem(p_mn[i]);
	}

	pMenu->addItem(GenericMenuSeparatorLine);
	pMenu->addItem(new CMenuOptionChooser(LOCALE_INFOVIEWER_STREAMINFO, (int *)&g_settings.personalize_bluebutton, PERSONALIZE_EOD_OPTIONS, PERSONALIZE_EOD_OPTION_COUNT, true, NULL, CRCInput::RC_yellow, NEUTRINO_ICON_BUTTON_YELLOW));
	pMenu->addItem(new CMenuOptionChooser(LOCALE_INFOVIEWER_EVENTLIST, (int *)&g_settings.personalize_redbutton, PERSONALIZE_EOD_OPTIONS, PERSONALIZE_EOD_OPTION_COUNT, true, NULL, CRCInput::RC_blue, NEUTRINO_ICON_BUTTON_BLUE));

	pMenu->addItem(GenericMenuSeparatorLine);
	pMenu->addItem(new CMenuForwarder(LOCALE_PERSONALIZE_HELP, true, NULL, this, "personalize_help", CRCInput::RC_help, NEUTRINO_ICON_BUTTON_HELP));

	pMenu->exec(NULL, "");
	pMenu->hide();
	selected = pMenu->getSelected();
	delete pMenu;
	
	if (haveChangedSettings())
	{
		if (ShowMsgUTF(LOCALE_PERSONALIZE_HEAD, g_Locale->getText(LOCALE_PERSONALIZE_SAVERESTART), CMessageBox::mbrNo, CMessageBox::mbYes | CMessageBox::mbNo, NEUTRINO_ICON_PROTECTING) == CMessageBox::mbrYes)
			SaveAndRestart();
		else
			if (ShowMsgUTF(LOCALE_PERSONALIZE_HEAD, g_Locale->getText(LOCALE_MESSAGEBOX_DISCARD), CMessageBox::mbrNo, CMessageBox::mbYes | CMessageBox::mbNo, NEUTRINO_ICON_PROTECTING) == CMessageBox::mbrYes)
				restoreSettings();
	}
		
}

//Here we give the user the option to enable, disable, or PIN protect items on the Main Menu.
//We also provide a means of PIN protecting the menu itself.
void CPersonalizeGui::ShowMenuOptions(const int& menu)
{
	std::string mn_name = CNeutrinoApp::getInstance()->menus[menu]->getName();

	CMenuWidget* pm = new CMenuWidget(LOCALE_PERSONALIZE_HEAD, NEUTRINO_ICON_PROTECTING, width);
	
	//subhead
	CMenuSeparator * pm_subhead = new CMenuSeparator(CMenuSeparator::ALIGN_LEFT | CMenuSeparator::SUB_HEAD | CMenuSeparator::STRING);
	std::string 	s_sh = g_Locale->getText(LOCALE_PERSONALIZE_ACCESS);
			s_sh += ": " + mn_name;
	pm_subhead->setString(s_sh);
	
	pm->addItem(pm_subhead);
	pm->addItem(GenericMenuSeparator);
	pm->addItem(GenericMenuBack);
	pm->addItem(GenericMenuSeparatorLine);

	//add all needed items
	for (uint i = 0; i<v_item.size(); i++)
	{
		if (mn_name == v_item[i].menu->getName())
		{
			int show_mode = v_item[i].item_mode;
			
			if (show_mode != PERSONALIZE_SHOW_NO)
			{
				if (show_mode == PERSONALIZE_SHOW_AS_ITEM_OPTION) 
				{	
					if (v_item[i].personalize_mode != NULL) //option chooser
						pm->addItem(new CMenuOptionChooser(v_item[i].locale_name, v_item[i].personalize_mode, PERSONALIZE_STD_OPTIONS, PERSONALIZE_STD_OPTION_COUNT, v_item[i].menuItem->active));
					else 
						pm->addItem(v_item[i].menuItem); //separator
				}
				
				//pin protected items only
				if (show_mode == PERSONALIZE_SHOW_AS_ACCESS_OPTION)
				{
					std::string 	itm_name = g_Locale->getText(v_item[i].locale_name);
							itm_name += " ";
							itm_name += g_Locale->getText(LOCALE_PERSONALIZE_PINSTATUS);
							
					if (v_item[i].personalize_mode != NULL) 	
						pm->addItem(new CMenuOptionChooser(itm_name.c_str(), v_item[i].personalize_mode, PERSONALIZE_YON_OPTIONS, PERSONALIZE_YON_OPTION_COUNT, v_item[i].menuItem->active));
				}
				
				//only show in personalize menu, usefull to hide separators in menu, but visible only in personalizing menu
				if (show_mode == PERSONALIZE_SHOW_ONLY_IN_PERSONALIZE_MENU)
					pm->addItem(v_item[i].menuItem); 
			}	
		}
	}
	
	pm->exec (NULL, "");
	pm->hide ();
	delete pm;

}

//shows a short help message
void CPersonalizeGui::ShowHelpPersonalize()
{
	Helpbox helpbox;
	
	for (int i = (int)LOCALE_PERSONALIZE_HELP_LINE1; i<= (int)LOCALE_PERSONALIZE_HELP_LINE8; i++)
		helpbox.addLine(g_Locale->getText((neutrino_locale_t)i));

	hide();
	helpbox.show(LOCALE_PERSONALIZE_HELP);
}

void CPersonalizeGui::SaveAndRestart()
{
	// Save the settings and restart Neutrino, if user wants to!
	CHintBox * hintBox = new CHintBox(LOCALE_MESSAGEBOX_INFO, g_Locale->getText(LOCALE_MAINSETTINGS_SAVESETTINGSNOW_HINT)); // UTF-8
	hintBox->paint();
	CNeutrinoApp::getInstance()->saveSetup();
	hintBox->hide();
	delete hintBox;

	CNeutrinoApp::getInstance()->exec(NULL, "restart");
}

//adds a personalized menu item object to menu with personalizing parameters
void CPersonalizeGui::addItem(CMenuWidget *menu, CMenuItem *menu_Item, const int *personalize_mode, const bool defaultselected, const int item_mode)
{
	CMenuForwarder *fw = static_cast <CMenuForwarder*> (menu_Item);
	
	CMenuItem *menu_item = menu_Item;
	
	if (personalize_mode != NULL)// if we in pinmode, transforming item to LockedMenuForwarder
	{
		//if item in pin mode, then use LockedMenuForwarder for no/visible/pin option in personalize menu OR if item in protect mode, then use also LockedMenuForwarder for access option (PinOn/PinOff) in personalize menu
		if ((*personalize_mode == PERSONALIZE_MODE_PIN && item_mode != PERSONALIZE_SHOW_AS_ACCESS_OPTION) || (*personalize_mode == PROTECT_MODE_PIN_PROTECTED && item_mode == PERSONALIZE_SHOW_AS_ACCESS_OPTION))
			menu_item = new CLockedMenuForwarder(fw->getTextLocale(), g_settings.personalize_pincode, true, fw->active, NULL, fw->getTarget(), fw->getActionKey().c_str(), fw->directKey, fw->iconName.c_str());
	}
	
	menu_item_t item = {menu, menu_item, defaultselected, fw->getTextLocale(), (int*)personalize_mode, item_mode};
	
	std::string icon = item.menuItem->iconName;
	neutrino_msg_t d_key = item.menuItem->directKey;
	bool add_shortcut = false;
	
	if (icon.empty() &&  d_key == CRCInput::RC_nokey && item.menuItem->active)
		add_shortcut = true;

	if (add_shortcut)
		item.menuItem->directKey = getShortcut(shortcut);

	if (item_mode == PERSONALIZE_SHOW_AS_ACCESS_OPTION)
	{
		if (add_shortcut)
			shortcut++;
		
		v_item.push_back(item);
		
		handleSetting((int*)personalize_mode);
	}
	else if (personalize_mode == NULL)
	{
		if (add_shortcut)
			shortcut++;

		v_item.push_back(item);
	}
	else if (personalize_mode != NULL)
	{
		if (*personalize_mode != PERSONALIZE_MODE_NOTVISIBLE)
		{
			if (add_shortcut)
				shortcut++;
		}
		
		v_item.push_back(item);
		
		handleSetting((int*)personalize_mode);
	}
	else
		return;
}

//adds a menu separator to menue, based upon GenericMenuSeparatorLine or CMenuSeparator objects with locale
//expands with parameter within you can show or hide this item in personalize options
void CPersonalizeGui::addSeparator(CMenuWidget &menu, const neutrino_locale_t locale_text, const int item_mode)
{
	menu_item_t to_add_sep[2] = {	{&menu, GenericMenuSeparatorLine, false, locale_text, NULL, item_mode}, 
					{&menu, new CMenuSeparator(CMenuSeparator::LINE | CMenuSeparator::STRING, locale_text), false, locale_text, NULL, item_mode}};
	
	if (locale_text == NONEXISTANT_LOCALE)
		v_item.push_back(to_add_sep[0]);
	else
		v_item.push_back(to_add_sep[1]);
}

//paint all available personalized menu items and separators to menu
//this replaces all collected actual and handled "widget->addItem()" tasks at once
void CPersonalizeGui::addPersonalizedItems()
{
 	for (uint i = 0; i < v_item.size(); i++)
	{	
		if (v_item[i].item_mode != PERSONALIZE_SHOW_ONLY_IN_PERSONALIZE_MENU)
		{
			if (v_item[i].personalize_mode != NULL)
			{
				if (v_item[i].menuItem->active && (*v_item[i].personalize_mode != PERSONALIZE_MODE_NOTVISIBLE || v_item[i].item_mode == PERSONALIZE_SHOW_AS_ACCESS_OPTION))
					v_item[i].menu->addItem(v_item[i].menuItem, v_item[i].default_selected); //forwarders...
			}
			else 
				v_item[i].menu->addItem(v_item[i].menuItem, v_item[i].default_selected); //separators
		}
	}
}

// returns RC_key depends of shortcut between key number 1 to 0, 10 returns 0, >10 returns no key
// parameter alternate_rc_key allows using an alternate key, default key is RC_nokey
neutrino_msg_t CPersonalizeGui::getShortcut(const int & shortcut_num, neutrino_msg_t alternate_rc_key)
{
	if (shortcut_num < 10) 
		return CRCInput::convertDigitToKey(shortcut_num);
	else if (shortcut_num == 10) 
		return CRCInput::RC_0;
	else	
		return alternate_rc_key;
}

//handle/collects old int settings
void  CPersonalizeGui::handleSetting(int *setting)
{	
	settings_int_t val	= {*setting, setting};
	v_int_settings.push_back(val);
}

//check for setup changes
bool  CPersonalizeGui::haveChangedSettings()
{
	//compare old settings with current settings
	for (uint i = 0; i < v_int_settings.size(); i++)
		if (v_int_settings[i].old_val != *v_int_settings[i].p_val)
			return true;

	return false;
}

//restore old settings
void CPersonalizeGui::restoreSettings()
{
	//restore settings with current settings
	for (uint i = 0; i < v_int_settings.size(); i++)
		*v_int_settings[i].p_val = v_int_settings[i].old_val;
	
	exec(NULL, "restore");
}

