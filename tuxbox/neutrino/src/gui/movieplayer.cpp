/*
	Neutrino-GUI  -   DBoxII-Project

	Movieplayer (c) 2003 by giggo
	Based on code by Dirch, obi and the Metzler Bros. Thanks.

	Homepage: http://www.giggo.de/dbox

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

/* KNOWN ISSUES:
 * - PS Playback does not work on Nokia Cable as the performance is too bad; I left it in because it might work on other boxes; It will be replaced by a server-side ps2ts conversion anyway
 *
*/


/* TODOs / Release Plan:

 - always: fix bugs and be nice to neutrino (state handling, background handling, interruptable etc.)
 
(currently planned order)
 - make sure that zapit works after movieplayer has been quit
 - read TS from socket (UDP?)
 - PES2TS on server side
 - DivX,XVid,AVI,MPG,etc.->TS on server side
 - Pause/Resume (removed due to performance penalty)
 - Add Error handling
 - Bookmarks
 - LCD support

*/
#define MORE_THAN_TS 1
#include <config.h>
#if HAVE_DVB_API_VERSION >= 3
#undef _FILE_OFFSET_BITS
#include <global.h>
#include <neutrino.h>
#include <system/debug.h>

#include <driver/fontrenderer.h>
#include <driver/rcinput.h>
#include <daemonc/remotecontrol.h>
#include <system/settings.h>
#include <algorithm>
#include <sys/time.h>
#include <fstream>

#include "eventlist.h"
#include "movieplayer.h"
#include <transform.h>
#include "color.h"
#include "infoviewer.h"
#include "nfs.h"

#include "widget/menue.h"
#include "widget/messagebox.h"
#include "widget/hintbox.h"
#include "widget/stringinput.h"

#include <fcntl.h>
#include <linux/dvb/audio.h>
#include <linux/dvb/dmx.h>
#include <linux/dvb/video.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#define ADAP	"/dev/dvb/adapter0"
#define ADEC	ADAP "/audio0"
#define VDEC	ADAP "/video0"
#define DMX	ADAP "/demux0"
#define DVR	ADAP "/dvr0"

#define AVIA_AV_STREAM_TYPE_0           0x00
#define AVIA_AV_STREAM_TYPE_SPTS        0x01 
#define AVIA_AV_STREAM_TYPE_PES         0x02
#define AVIA_AV_STREAM_TYPE_ES          0x03 

#define ConnectLineBox_Width	15

static int playstate;
static const char *filename;
static std::string Path;
static CFileBrowser * filebrowser;

//------------------------------------------------------------------------

CMoviePlayerGui::CMoviePlayerGui()
{
	frameBuffer = CFrameBuffer::getInstance();

	visible = false;
	selected = 0;

	filebrowser = new CFileBrowser();
	filebrowser->Multi_Select = false;
	filebrowser->Dirs_Selectable = false;
	videofilefilter.addFilter("ts");
	videofilefilter.addFilter("ps");
        videofilefilter.addFilter("mpg");
	filebrowser->Filter = &videofilefilter;
   	if(strlen(g_settings.network_nfs_local_dir[0])!=0)
      		Path = g_settings.network_nfs_local_dir[0];
	else
      		Path = "/";
}

//------------------------------------------------------------------------

CMoviePlayerGui::~CMoviePlayerGui()
{
	delete filebrowser;
	g_Zapit->setStandby(false);
	g_Sectionsd->setPauseScanning(false);

}

//------------------------------------------------------------------------
int CMoviePlayerGui::exec(CMenuTarget* parent, std::string actionKey)
{
	m_state=STOP;
	current=-1;
	selected = 0;

	//define screen width
	width = 710;
	if((g_settings.screen_EndX- g_settings.screen_StartX) < width+ConnectLineBox_Width)
		width=(g_settings.screen_EndX- g_settings.screen_StartX)-ConnectLineBox_Width;

	//define screen height
	height = 570;
	if((g_settings.screen_EndY- g_settings.screen_StartY) < height)
		height=(g_settings.screen_EndY- g_settings.screen_StartY);
	buttonHeight = min(25,g_Fonts->infobar_small->getHeight());
	theight= g_Fonts->menu_title->getHeight();
	fheight= g_Fonts->menu->getHeight();
	sheight= g_Fonts->infobar_small->getHeight();
	title_height=fheight*2+20+sheight+4;
	info_height=fheight*2;
	listmaxshow = (height-info_height-title_height-theight-2*buttonHeight)/(fheight);
	height = theight+info_height+title_height+2*buttonHeight+listmaxshow*fheight;	// recalc height

	x=(((g_settings.screen_EndX- g_settings.screen_StartX)-(width+ConnectLineBox_Width)) / 2) + g_settings.screen_StartX + ConnectLineBox_Width;
	y=(((g_settings.screen_EndY- g_settings.screen_StartY)-height)/ 2) + g_settings.screen_StartY;

	if(parent)
	{
		parent->hide();
	}

	// set zapit in standby mode
	g_Zapit->setStandby(true);

	// tell neutrino we're in ts_mode
	CNeutrinoApp::getInstance()->handleMsg( NeutrinoMessages::CHANGEMODE , NeutrinoMessages::mode_ts );
	// remember last mode
	m_LastMode=(CNeutrinoApp::getInstance()->getLastMode() /*| NeutrinoMessages::norezap*/);

	// Stop sectionsd
	g_Sectionsd->setPauseScanning(true);


	show();

	//stop();
	hide();

	g_Zapit->setStandby(false);

	// Start Sectionsd
	g_Sectionsd->setPauseScanning(false);

	// Restore last mode
	CNeutrinoApp::getInstance()->handleMsg( NeutrinoMessages::CHANGEMODE , m_LastMode );

	// always exit all
	return menu_return::RETURN_EXIT_ALL;
}

void* RcThread( void* dummy )
{
	uint msg, data;

	while( playstate >= 1 )
	{
		g_RCInput->getMsg( &msg, &data, 100 );	// 10 secs..

		if( msg == CRCInput::RC_red ||
		    msg == CRCInput::RC_home )
		{
			//exit play
			playstate = 0;
		}
		else if( msg == CRCInput::RC_yellow )
		{
		  	if( playstate == 1 )
		  	{
		  		// pause play
		  		playstate = 2;
		  	}
		  	else
		  	{
		  		// resume play
		  		playstate = 1;
		  	}
		}
		else if( msg == CRCInput::RC_left ||
		         msg == CRCInput::RC_right )
		{
			// todo: next/prev file
			filename = NULL;

			if( filebrowser->exec(Path) )
			{
				Path = filebrowser->getCurrentDir();
				if( (filename = filebrowser->getSelectedFile()->Name.c_str()) != NULL )
					playstate = -1;
			}
		}
		else if(msg == NeutrinoMessages::RECORD_START ||
				  msg == NeutrinoMessages::ZAPTO ||
				  msg == NeutrinoMessages::STANDBY_ON ||
				  msg == NeutrinoMessages::SHUTDOWN ||
				  msg == NeutrinoMessages::SLEEPTIMER)
		{
			// Exit for Record/Zapto Timers
			playstate = 0;
			g_RCInput->postMsg( msg, data );
		}
		else
		{
			if( CNeutrinoApp::getInstance()->handleMsg( msg, data ) == messages_return::cancel_all )
				playstate = 0;
		}
	}

	pthread_exit(NULL);
}

int CMoviePlayerGui::PlayStream( bool isTS )
{
	unsigned char buf[384*188];
	unsigned short pida, pidv;
	int file, done, fd, dmxa, dmxv, dvr, adec, vdec;
	struct dmx_pes_filter_params p;
	ssize_t wr;
	size_t r;

	//INFO("Green (Play TS File");
	playstate = -1;
	filename = NULL;

	if( filebrowser->exec(Path) )
	{
		Path = filebrowser->getCurrentDir();
		filename = filebrowser->getSelectedFile()->Name.c_str();
	}

	while( playstate < 0 )
	{
		pida = pidv = fd = dmxa = dmxv = dvr = adec = vdec = 0;

		if( filename == NULL )
			return -1;
		//printf("[movieplayer.cpp] TS Filename %s\n", filename);

		if( (fd = open(filename, O_RDONLY|O_LARGEFILE)) < 0 )
			return -1;

		// todo: check if file is valid ts or pes
		if( isTS )
		{
			find_avpids( fd, &pidv, &pida );
			fprintf( stdout, "[movieplayer.cpp] found pida: 0x%04X ; pidv: 0x%04X\n", pida, pidv );
		}
		else
		{	// Play PES
			pida=0x900;
			pidv=0x8ff;
		}
		lseek( fd, 0, SEEK_SET );

		if( (dmxa = open(DMX, O_RDWR)) < 0 ||
		    (dmxv = open(DMX, O_RDWR)) < 0 ||
		    (dvr  = open(DVR, O_WRONLY)) < 0 ||
		    (adec = open(ADEC, O_RDWR)) < 0 ||
		    (vdec = open(VDEC, O_RDWR)) < 0 ||
		    (file   = open(filename, O_RDONLY)) < 0 ) {
			close(fd);
			close(dmxa);
			close(dmxv);
			close(dvr);
			close(adec);
			close(vdec);
		    	return -1;
		}
	
		p.input = DMX_IN_DVR;
		p.output = DMX_OUT_DECODER;
		p.flags = DMX_IMMEDIATE_START;

		p.pid = pida;
		p.pes_type = DMX_PES_AUDIO;
		if( ioctl(dmxa, DMX_SET_PES_FILTER, &p) < 0 ||
		    ioctl(adec, AUDIO_STOP) < 0 ||
		    ioctl(vdec, VIDEO_STOP) < 0 ) {
			close(fd);
			close(dmxa);
			close(dmxv);
			close(dvr);
			close(adec);
			close(vdec);
		    	return -1;
		}

		p.pid = pidv;
		p.pes_type = DMX_PES_VIDEO;
		if( ioctl(dmxv, DMX_SET_PES_FILTER, &p) < 0 ||
		    ioctl(adec, AUDIO_PLAY) < 0 ||
		    ioctl(vdec, VIDEO_PLAY) < 0 ) {
			close(fd);
			close(dmxa);
			close(dmxv);
			close(dvr);
			close(adec);
			close(vdec);
		    	return -1;
		}

		CLCD::getInstance()->setMode(CLCD::MODE_TVRADIO);
		CLCD::getInstance()->showServicename(filebrowser->getSelectedFile()->getFileName().c_str());

		if( isTS )
		{
			playstate = 1;
			pthread_create(&rct, 0, RcThread, 0);

			while( (r = read(file, buf, sizeof(buf))) > 0 && playstate >= 1 )
			{
				done = 0;
	
				if( playstate == 2 )
				{	// pause play
					CLCD::getInstance()->showServicename("("+filebrowser->getSelectedFile()->getFileName()+")");
		
					while( playstate == 2 )
						usleep(200000);
	
					/* Filters need to be re-set, or playback will not work correctly */
					p.pid = pida;
					p.pes_type = DMX_PES_AUDIO;
					ioctl(dmxa, DMX_SET_PES_FILTER, &p);
					p.pid = pidv;
					p.pes_type = DMX_PES_VIDEO;
					ioctl(dmxv, DMX_SET_PES_FILTER, &p);
					CLCD::getInstance()->showServicename(filebrowser->getSelectedFile()->getFileName());
				}
	
				while( r > 0 )
				{
					if( (wr = write(dvr, &buf[done], r)) <= 0 )
						continue;
					r -= wr;
					done += wr;
				}
			}
		}
		else
		{
			playstate = 0;	// no rc support
			pes_to_ts2( file, dvr, pida, pidv );	// VERY bad performance!!!
		}

		ioctl(vdec, VIDEO_STOP);
		ioctl(adec, AUDIO_STOP);
		ioctl(dmxv, DMX_STOP);
		ioctl(dmxa, DMX_STOP);

		close(file);
		close(fd);
		close(dmxa);
		close(dmxv);
		close(dvr);
		close(adec);
		close(vdec);
	}

	if( playstate >= 1 )
	{	// exit rcthread
		g_RCInput->postMsg( CRCInput::RC_home, 0 );	// force faster exit of rcthread
		pthread_join( rct, NULL );
	}

	return 1;
}

int CMoviePlayerGui::show()
{
	int res = -1;
	uint msg, data;
	bool loop = true, update = true;

	key_level = 0;

	while(loop)
	{
		if(CNeutrinoApp::getInstance()->getMode()!=NeutrinoMessages::mode_ts)
		{
			// stop if mode was changed in another thread
			loop = false;
		}

		if(update)
		{
			hide();
			update = false;
			paint();
		}

		// Check Remote Control

		g_RCInput->getMsg( &msg, &data, 10 ); // 1 sec timeout to update play/stop state display

		if( msg == CRCInput::RC_home )
		{ //Exit after cancel key
			loop = false;
		}
		else if( msg == CRCInput::RC_timeout )
		{
			// do nothing
		}
//------------ GREEN --------------------
		else if( msg==CRCInput::RC_green )
		{
			hide();
			PlayStream( true );
			paint();
		}
//------------ YELLOW --------------------
#ifdef MORE_THAN_TS
		else if(msg==CRCInput::RC_yellow)
		{
			hide();
			PlayStream( false );
			paint();
		}
#endif	
		else if(msg == NeutrinoMessages::CHANGEMODE)
		{
			if((data & NeutrinoMessages::mode_mask) !=NeutrinoMessages::mode_ts)
			{
				loop = false;
				m_LastMode=data;
			}
		}
		else if(msg == NeutrinoMessages::RECORD_START ||
				  msg == NeutrinoMessages::ZAPTO ||
				  msg == NeutrinoMessages::STANDBY_ON ||
				  msg == NeutrinoMessages::SHUTDOWN ||
				  msg == NeutrinoMessages::SLEEPTIMER)
		{
			// Exit for Record/Zapto Timers
			// Add bookmark
			loop = false;
			g_RCInput->postMsg(msg, data);
		}
		else
		{
			if( CNeutrinoApp::getInstance()->handleMsg( msg, data ) == messages_return::cancel_all )
			{
				loop = false;
			}
			// update mute icon
			paintHead();
		}
	}
	hide();

	if(m_state != STOP) {
		//stop();
        }

	return(res);
}

//------------------------------------------------------------------------

void CMoviePlayerGui::hide()
{
	if(visible)
	{
		frameBuffer->paintBackgroundBoxRel(x-ConnectLineBox_Width-1, y+title_height-1, width+ConnectLineBox_Width+2, height+2-title_height);
		frameBuffer->paintBackgroundBoxRel(x, y, width, title_height);
		frameBuffer->ClearFrameBuffer();
		visible = false;
	}
}

//------------------------------------------------------------------------

void CMoviePlayerGui::paintHead()
{
//	printf("paintHead{\n");
	std::string strCaption = g_Locale->getText("movieplayer.head");
	frameBuffer->paintBoxRel(x,y+title_height, width,theight, COL_MENUHEAD);
	frameBuffer->paintIcon("movie.raw",x+7,y+title_height+10);
	g_Fonts->menu_title->RenderString(x+35,y+theight+title_height+0, width- 45, strCaption.c_str(), COL_MENUHEAD, 0, true); // UTF-8
	int ypos=y+title_height;
	if(theight > 26)
		ypos = (theight-26) / 2 + y + title_height;
	frameBuffer->paintIcon("dbox.raw", x+ width- 30, ypos );
	if( CNeutrinoApp::getInstance()->isMuted() )
	{
		int xpos=x+width-75;
		ypos=y+title_height;
		if(theight > 32)
			ypos = (theight-32) / 2 + y + title_height;
		frameBuffer->paintIcon("mute.raw", xpos, ypos);
	}
	visible = true;

}

//------------------------------------------------------------------------

void CMoviePlayerGui::paintImg()
{
	// TODO: find better image
	frameBuffer->paintBoxRel(x,y+title_height+theight, width,height-info_height-2*buttonHeight-title_height-theight, COL_BACKGROUND);
	frameBuffer->paintIcon("movieplayer.raw",x+25,y+15+title_height+theight);
}

//------------------------------------------------------------------------
void CMoviePlayerGui::paintFoot()
{
	if(m_state==STOP) // insurance
		key_level=0;
	int ButtonWidth = (width-20) / 4;
	frameBuffer->paintBoxRel(x,y+(height-info_height-2*buttonHeight), width,2*buttonHeight, COL_MENUHEAD);
	frameBuffer->paintHLine(x, x+width-x,  y+(height-info_height-2*buttonHeight), COL_INFOBAR_SHADOW);

#ifdef MORE_THAN_TS
	frameBuffer->paintIcon("rot.raw", x+ 0* ButtonWidth + 10, y+(height-info_height-2*buttonHeight)+4);
		g_Fonts->infobar_small->RenderString(x + 0* ButtonWidth + 30, y+(height-info_height-2*buttonHeight)+24 - 1,
						     ButtonWidth- 20, g_Locale->getText("movieplayer.bookmark").c_str(), COL_INFOBAR, 0, true); // UTF-8
#endif

		frameBuffer->paintIcon("gruen.raw", x+ 1* ButtonWidth + 10, y+(height-info_height-2*buttonHeight)+4);
		g_Fonts->infobar_small->RenderString(x+ 1* ButtonWidth +30, y+(height-info_height-2*buttonHeight)+24 - 1,
						     ButtonWidth- 20, g_Locale->getText("movieplayer.choosets").c_str(), COL_INFOBAR, 0, true); // UTF-8

#ifdef MORE_THAN_TS
		frameBuffer->paintIcon("gelb.raw", x+ 2* ButtonWidth + 10, y+(height-info_height-2*buttonHeight)+4);
		g_Fonts->infobar_small->RenderString(x+ 2* ButtonWidth + 30, y+(height-info_height-2*buttonHeight)+24 - 1,
						     ButtonWidth- 20, g_Locale->getText("movieplayer.chooseps").c_str(), COL_INFOBAR, 0, true); // UTF-8
#endif
}

void CMoviePlayerGui::paint()
{
	CLCD::getInstance()->setMode(CLCD::MODE_TVRADIO);
	CLCD::getInstance()->showServicename("Movieplayer");
	frameBuffer->loadPal("radiomode.pal", 18, COL_MAXFREE);
	frameBuffer->loadBackground("radiomode.raw");
	frameBuffer->useBackground(true);
	frameBuffer->paintBackground();

	paintHead();
	paintImg();
	paintFoot();

	visible = true;
}

#endif
