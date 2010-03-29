/*
	$Id: sambaserver_setup.h,v 1.1 2010/03/29 19:13:17 dbt Exp $

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

#ifndef __sambaserver_setup__
#define __sambaserver_setup__

#include <configfile.h>

#include <gui/widget/menue.h>
#include <gui/widget/icons.h>

#include <driver/framebuffer.h>

#include <string>

#define NMBD		"nmbd"
#define SMBD		"smbd"
#define SAMBA_MARKER	"/var/etc/.sambaserver"

class CSambaSetup : public CMenuTarget
{
	private:

		enum SMB_GLOBAL_SETTINGS_NUM
		{
			G_INTERFACES,
			G_WORKGROUP,
		};

		enum SMB_SETTINGS_NUM	
		{
			SET_GLOBAL,
			SET_SHARE
		};

		enum SMB_SHARE_SETTINGS_NUM	
		{
			SHARE_COMMENT,
			SHARE_PATH,
			SHARE_RO,
			SHARE_PUBLIC	
		};

		CFrameBuffer *frameBuffer;

		int x, y, width, height, hheight, mheight;

		neutrino_locale_t menue_title;
		std::string menue_icon;
		std::string interface;

		//helper
		std::string upperString(const std::string& to_upper_str);

		void hide();
		void showSambaSetup();
		void Init();

	public:	
		CSambaSetup(const neutrino_locale_t title = NONEXISTANT_LOCALE, const char * const IconName = NEUTRINO_ICON_SETTINGS);
		~CSambaSetup();

		enum SMB_SERVER_STATUS_NUM
		{
			SMB_STOPPED,
			SMB_RUNNING,
		};
		enum SMB_ON_OFF_NUM	
		{
			OFF,
			ON,
		};

		int exec(CMenuTarget* parent, const std::string & actionKey);
};

class CSambaOnOffNotifier : public CChangeObserver
{
	const char * filename;

	public:
		inline CSambaOnOffNotifier(const char * file_to_modify)
		{
			filename = file_to_modify;
		};
		bool changeNotify(const neutrino_locale_t, void * data);
};


#endif
