/*
	$Id: sambaserver_setup.cpp,v 1.2 2010/04/01 19:29:12 dbt Exp $

	sambaserver setup menue - Neutrino-GUI

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

#include "gui/sambaserver_setup.h"

#include <global.h>
#include <neutrino.h>

#include <gui/widget/stringinput.h>
#include <gui/widget/dirchooser.h>

#include <driver/screen_max.h>
#include <system/debug.h>
#include <system/setting_helpers.h>

#include <iostream>
#include <fstream>
#include <sstream>
#include <errno.h>
#include <libnet.h>
#include <signal.h>
#include <sys/vfs.h> // statfs

using namespace std;

CSambaSetup::CSambaSetup(const neutrino_locale_t title, const char * const IconName )
{
	frameBuffer = CFrameBuffer::getInstance();

	menue_title = title != NONEXISTANT_LOCALE ? title : LOCALE_SAMBASERVER_SETUP;
	menue_icon = IconName != NEUTRINO_ICON_SETTINGS ? IconName : NEUTRINO_ICON_SETTINGS;

	width 	= w_max (550, 100);
	hheight = g_Font[SNeutrinoSettings::FONT_TYPE_MENU_TITLE]->getHeight();
	mheight = g_Font[SNeutrinoSettings::FONT_TYPE_MENU]->getHeight();
	height 	= hheight+13*mheight+ 10;
	x	= getScreenStartX (width);
	y	= getScreenStartY (height);

	interface = getInterface();
}

CSambaSetup::~CSambaSetup()
{
}

int CSambaSetup::exec(CMenuTarget* parent, const std::string &/*actionKey*/)
{
	dprintf(DEBUG_DEBUG, "init samba menu\n");
	int   res = menu_return::RETURN_REPAINT;

	if (parent)
	{
		parent->hide();
	}


	Init();
	g_settings.smb_setup_samba_workgroup = upperString(g_settings.smb_setup_samba_workgroup);
	return res;
}

void CSambaSetup::hide()
{
	frameBuffer->paintBackgroundBoxRel(x,y, width,height);
}

// init menue
void CSambaSetup::Init()
{
	showSambaSetup();
}

#define SMB_ON_OFF_OPTION_COUNT 2
const CMenuOptionChooser::keyval SMB_ON_OFF_OPTIONS[SMB_ON_OFF_OPTION_COUNT] =
{
	{ CSambaSetup::SMB_STOPPED, LOCALE_SAMBASERVER_SETUP_STAT_STOPPED  },
	{ CSambaSetup::SMB_RUNNING, LOCALE_SAMBASERVER_SETUP_STAT_RUNNING }
};

#define SMB_YES_NO_OPTION_COUNT 2
const CMenuOptionChooser::keyval SMB_YES_NO_OPTIONS[SMB_YES_NO_OPTION_COUNT] =
{
	{ CSambaSetup::OFF, LOCALE_MESSAGEBOX_NO  },
	{ CSambaSetup::ON, LOCALE_MESSAGEBOX_YES }
};

/* shows entries for samba settings */
void CSambaSetup::showSambaSetup()
{
	//init
	CMenuWidget * sm = new CMenuWidget(menue_title, menue_icon, width);
	if (menue_title != NONEXISTANT_LOCALE)
	{
		CMenuSeparator * sm_subhead = new CMenuSeparator(CMenuSeparator::ALIGN_LEFT | CMenuSeparator::SUB_HEAD | CMenuSeparator::STRING, LOCALE_SAMBASERVER_SETUP);
		sm->addItem(sm_subhead);
	}

	//samba binaries install path
	CDirChooser * sh_ch_inst_path = new CDirChooser(&g_settings.smb_setup_samba_installdir);
	CMenuForwarder * sh_fw_inst_path = new CMenuForwarder(LOCALE_SAMBASERVER_SETUP_INSTALL_DIR, true, g_settings.smb_setup_samba_installdir, sh_ch_inst_path, NULL, CRCInput::RC_red, NEUTRINO_ICON_BUTTON_RED);
 
 	//samba config file path
	CStringInputSMS * sh_input_conf_path = new CStringInputSMS(LOCALE_SAMBASERVER_SETUP_CONFIGFILE_PATH, & g_settings.smb_setup_samba_conf_path, 25, LOCALE_SAMBASERVER_SETUP_CONFIGFILE_PATH_HINT1, LOCALE_SAMBASERVER_SETUP_CONFIGFILE_PATH_HINT2, "abcdefghijklmnopqrstuvwxyz0123456789!""§$%&/()=?-. ");
	CMenuForwarder * sh_fw_conf_path = new CMenuForwarder(LOCALE_SAMBASERVER_SETUP_CONFIGFILE_PATH, true, g_settings.smb_setup_samba_conf_path, sh_input_conf_path, NULL, CRCInput::RC_green, NEUTRINO_ICON_BUTTON_GREEN);

	//start/stop sambaserver, set real server status
	if (getPidof(NMBD).empty() || getPidof(SMBD).empty())
	{
		g_settings.smb_setup_samba_on_off = SMB_STOPPED;
		remove(SAMBA_MARKER);
	}
	else
		g_settings.smb_setup_samba_on_off = SMB_RUNNING;

	CSambaOnOffNotifier * smb_notifier = new CSambaOnOffNotifier(SAMBA_MARKER);
	CMenuOptionChooser * sm_start = new CMenuOptionChooser(LOCALE_SAMBASERVER_SETUP_STAT, &g_settings.smb_setup_samba_on_off, SMB_ON_OFF_OPTIONS, SMB_ON_OFF_OPTION_COUNT, true, smb_notifier, CRCInput::RC_standby, NEUTRINO_ICON_BUTTON_POWER);

 	//workroup, domainname
	CStringInputSMS * sm_input_domain = new CStringInputSMS(LOCALE_SAMBASERVER_SETUP_WORKGROUP, &g_settings.smb_setup_samba_workgroup,15, LOCALE_SAMBASERVER_SETUP_WORKGROUP_HINT1, LOCALE_SAMBASERVER_SETUP_WORKGROUP_HINT2, "abcdefghijklmnopqrstuvwxyz0123456789_ ");
	CMenuForwarder * sm_fw_domain = new CMenuForwarder(LOCALE_SAMBASERVER_SETUP_WORKGROUP, true, g_settings.smb_setup_samba_workgroup, sm_input_domain, NULL, CRCInput::RC_blue, NEUTRINO_ICON_BUTTON_BLUE);

	//address,interface
	CMenuForwarder * sm_fw_interface = new CMenuForwarder(LOCALE_SAMBASERVER_SETUP_INTERFACE, false, interface);

	//add items
	sm->addItem(GenericMenuSeparator);
	sm->addItem(GenericMenuBack);
	sm->addItem(GenericMenuSeparatorLine);
	//-----------------------------------
 	sm->addItem(sm_start);			//server stat
	sm->addItem(GenericMenuSeparatorLine);
	//-----------------------------------
 	sm->addItem(sh_fw_inst_path);		//install dir
 	sm->addItem(sh_fw_conf_path);		//config file path
 	sm->addItem(GenericMenuSeparatorLine);
 	//-----------------------------------
 	sm->addItem(sm_fw_domain);		//workgroup/domain input
	sm->addItem(sm_fw_interface);		//interface

	sm->exec(NULL, "");
	sm->hide();
	delete sm;
}

//helper: uppercase string
string CSambaSetup::upperString(const string& to_upper_str)
{
	char s[23];
	sprintf(s, "%s", to_upper_str.c_str());
	unsigned n=0;

	while (*(s+n))
	{
		if (*(s+n)>96 && *(s+n)<123)
			*(s+n)-=32;
		n++;
	}

	return s;
}

//notfier for samba switching
#define SAMBA_COMMANDS_COUNT 2
typedef struct smb_cmd_t
{
	const string bin;
	const string option;
} smb_cmd_struct_t;

const smb_cmd_struct_t smb_cmd[SAMBA_COMMANDS_COUNT] =
{
	{NMBD, " -D"},
	{SMBD, (string)" -D -a -s "},
};

bool CSambaOnOffNotifier::changeNotify(const neutrino_locale_t, void * data)
{
	bool ret = true;
	string err_msg;

	if ((*(int *)data) != 0)
	{
		for (int i = 0; i<SAMBA_COMMANDS_COUNT; i++)
		{
			string pid = getPidof(smb_cmd[i].bin);

			if (pid.empty())
			{
				string cmd = g_settings.smb_setup_samba_installdir + "/" + smb_cmd[i].bin + smb_cmd[i].option + " " +  g_settings.smb_setup_samba_conf_path;

				int result = CNeutrinoApp::getInstance()->execute_sys_command(cmd.c_str());
				if (result !=0)
				{
					err_msg = "Error while start samba server!\n";
					cerr<<"[samba setup] "<<__FUNCTION__ <<": error while executing " <<smb_cmd[i].bin<<endl;
					if (result == 127)
						err_msg += smb_cmd[i].bin + " not found!\n";
					else if (result == 1)
						err_msg += "Please check path of samba config file!\n";
					else
						err_msg += cmd + "\n" + strerror(result);
					ret = false;
				}
				else
					cout << "[samba setup] started "<< smb_cmd[i].bin << " with options:" << smb_cmd[i].option<<endl;
			}

		}		
		
		if (ret)
		{
			FILE * fd = fopen(filename, "w");
			if (fd)
				fclose(fd);
			else
				return false;
		}
	}
	else
	{
		string pid;
		for (int i = 0; i<SAMBA_COMMANDS_COUNT; i++)
		{
			while (!(pid = getPidof(smb_cmd[i].bin)).empty())
			{
				stringstream Str;
				Str << pid;
				int i_pid;
				Str >> i_pid;
				
				if (!pid.empty())
				{	
					if (kill(i_pid, SIGKILL) !=0)
					{
						cerr << "[samba setup] "<<__FUNCTION__ <<":  error while terminating: "<<smb_cmd[i].bin <<" "<< strerror(errno)<<endl;
						ret = false;
					}
					else
						cout << "[samba setup] killed "<< smb_cmd[i].bin << " pid: "<<pid<<endl;
				}
			}
		}
		remove(filename);
	}

	if (!ret)
	{
		DisplayInfoMessage(err_msg.c_str());
		g_settings.smb_setup_samba_on_off = ret;
	}

	return ret;
}

