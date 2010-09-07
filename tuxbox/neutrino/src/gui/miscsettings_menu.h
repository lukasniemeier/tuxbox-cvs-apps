/*
	$Id: miscsettings_menu.h,v 1.1 2010/09/07 09:20:50 dbt Exp $

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

#ifndef __miscsettings_menu__
#define __miscsettings_menu__

#include <gui/widget/menue.h>

#include <driver/framebuffer.h>

#include <system/settings.h>

#include <string>

 class CMiscMenue : public CMenuTarget
{	
	private:

		CFrameBuffer *frameBuffer;
		
		int x, y, width, height, menue_width, hheight, mheight;

		neutrino_locale_t menue_title;
		std::string menue_icon;

		void hide();
		void showMenue();


	public:
		CMiscMenue(const neutrino_locale_t title = NONEXISTANT_LOCALE, const char * const IconName = NULL);
		~CMiscMenue();
		int exec(CMenuTarget* parent, const std::string & actionKey);
		
};


#endif
