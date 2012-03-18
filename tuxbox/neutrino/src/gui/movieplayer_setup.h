/*
	$Id: movieplayer_setup.h,v 1.6 2012/03/18 11:20:14 rhabarber1848 Exp $

	movieplaye setup implementation - Neutrino-GUI

	Copyright (C) 2001 Steffen Hehn 'McClean'
	and some other guys
	Homepage: http://dbox.cyberphoria.org/

	Copyright (C) 2009 T. Graf 'dbt'
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

#ifndef __movieplayer_setup__
#define __movieplayer_setup__

#include <gui/widget/menue.h>

#include <string>

class CMoviePlayerSetup : public CMenuTarget
{
	private:
		int width, selected;

		int showMoviePlayerSetup();
		int showMoviePlayerSelectPlugin();

	public:	
		CMoviePlayerSetup();
		~CMoviePlayerSetup();
		int exec(CMenuTarget* parent, const std::string & actionKey);
};

class CMoviePluginChangeExec : public CMenuTarget
{
	public:
		int exec(CMenuTarget* parent, const std::string & actionKey);
};

class CStreamingNotifier : public CChangeObserver
{
	private:
		CMenuItem* toDisable[12];
	public:
		CStreamingNotifier( CMenuItem*, CMenuItem*, CMenuItem*, CMenuItem*, CMenuItem*, CMenuItem*, CMenuItem*, CMenuItem*, CMenuItem*, CMenuItem*, CMenuItem*, CMenuItem*);
		bool changeNotify(const neutrino_locale_t, void *);
};

#endif
