/*
	$Id: driver_boot_setup.h,v 1.5 2012/03/18 11:20:14 rhabarber1848 Exp $

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

#ifndef __driver_boot_setup__
#define __driver_boot_setup__

#include <gui/widget/menue.h>

#include <string>

class CDriverBootSetup : public CMenuTarget
{
	private:
		int width, selected;

		neutrino_locale_t menue_title;
		std::string menue_icon;

		int showSetup();

	public:
		CDriverBootSetup(const neutrino_locale_t title = NONEXISTANT_LOCALE, const char * const IconName = NULL);
		~CDriverBootSetup();
		int exec(CMenuTarget* parent, const std::string & actionKey);
};

class CConsoleDestChangeNotifier : public CChangeObserver
{
	public:
		bool changeNotify(const neutrino_locale_t, void * Data);
};

class CFdxChangeNotifier : public CChangeObserver
{
	public:
		bool changeNotify(const neutrino_locale_t, void * Data);
};

#ifdef HAVE_DBOX_HARDWARE
class CSPTSNotifier : public CChangeObserver
{
	public:
		bool changeNotify(const neutrino_locale_t, void *);
};
#endif

class CTouchFileNotifier : public CChangeObserver
{
	const char * filename;
 public:
	inline CTouchFileNotifier(const char * file_to_modify)
		{
			filename = file_to_modify;
		};
	bool changeNotify(const neutrino_locale_t, void * data);
};

#endif
