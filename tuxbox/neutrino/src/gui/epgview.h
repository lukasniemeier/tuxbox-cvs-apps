/*
	Neutrino-GUI  -   DBoxII-Project

	Copyright (C) 2001 Steffen Hehn 'McClean'
	Homepage: http://dbox.cyberphoria.org/

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


#ifndef __epgview__
#define __epgview__

#include <driver/framebuffer.h>
#include <driver/rcinput.h>
#include <driver/fontrenderer.h>
#include <system/settings.h>

#include <gui/color.h>
#include "widget/menue.h"

#include <sectionsdclient/sectionsdclient.h>

#include <vector>
#include <string>

// will be divided by 10
#define BIG_FONT_FAKTOR 15

class CEpgData
{
	private:
		enum
		{
			EPG_INFO2,
			SCREENING_AFTER,
			SCREENING_BEFORE,
			EPG_INFO1
		};

		CFrameBuffer		*frameBuffer;
		CChannelEventList	evtlist;
		CChannelEventList	followlist;
		CEPGData		epgData;

		std::string 		epg_date;
		std::string 		epg_start;
		std::string 		epg_end;
		int			epg_done;
		bool 			call_fromfollowlist;

		unsigned long long	prev_id;
		time_t			prev_zeit;
		unsigned long long 	next_id;
		time_t 			next_zeit;

		int			ox, oy, sx, sy, toph, sb;
		int			emptyLineCount;
		int         		textCount;
		typedef std::pair<std::string,int> epg_pair;
		std::vector<epg_pair> epgText;
		int			topheight,topboxheight;
		int			botheight,botboxheight;
		int			boticonwidth,boticonheight;
		int			buttonheight;
		int			medlineheight,medlinecount;
		bool			bigFonts;

		void GetEPGData(const t_channel_id channel_id, unsigned long long id, time_t* startzeit );
		void GetPrevNextEPGData( unsigned long long id, time_t* startzeit );
		void addTextToArray( const std::string & text, int flag );
		void processTextToArray(std::string text, int flag = EPG_INFO2);
		void showText( int startPos, int ypos );
		bool hasFollowScreenings(const t_channel_id channel_id, const std::string & title, const time_t startzeit);
		void FollowScreenings(const time_t startzeit);
		void showTimerEventBar(bool show);

	public:

		CEpgData();
		void start( );
		int show(const t_channel_id channel_id, unsigned long long id = 0, time_t* startzeit = NULL, bool doLoop = true, bool callFromfollowlist = false );
		void hide();
};



class CEPGDataHandler : public CMenuTarget
{
	public:
		int  exec( CMenuTarget* parent,  const std::string &actionkey);

};



#endif

