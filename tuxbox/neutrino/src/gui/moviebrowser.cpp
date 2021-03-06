/***************************************************************************
	$Id: moviebrowser.cpp,v 1.80 2012/11/01 19:31:29 rhabarber1848 Exp $

	Neutrino-GUI  -   DBoxII-Project

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

	***********************************************************

	Module Name: moviebrowser.cpp .

	Description: Implementation of the CMovieBrowser class
	             This class provides a filebrowser window to view, select and start a movies from HD.
	             This class does replace the Filebrowser

	Date:	   Nov 2005

	Author: Guenther@tuxbox.berlios.org
		based on code of Steffen Hehn 'McClean'

****************************************************************************/

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

// experimental stuff 8)
//#define MB_SEARCH_INFO2
//#define MOVEMANAGER 1
#ifdef MOVEMANAGER
#include <gui/movemanager.h>
#endif // MOVEMANAGER

#include <algorithm>

#include <stdlib.h>
#include <driver/screen_max.h>
#include <gui/moviebrowser.h>
#include <gui/movieplayer.h>
#include <gui/filebrowser.h>
#include <gui/imageinfo.h>
#include <gui/widget/hintbox.h>
#include <gui/widget/helpbox.h>
#include <gui/widget/messagebox.h>
#include <gui/widget/stringinput_ext.h>
#include <gui/widget/mountchooser.h>
#include <gui/widget/dirchooser.h>
#include <gui/widget/stringinput.h>
#include <dirent.h>
#include <sys/stat.h>
#ifdef ENABLE_GUI_MOUNT
#include <gui/nfs.h>
#endif
#include <neutrino.h>
#include <sys/vfs.h> // for statfs
#include <gui/widget/icons.h>
#include <sys/mount.h>
#include <unistd.h>
//#include <system/ping.h>
extern "C" int pingthost ( const char *hostname, int t );

#define my_scandir scandir64
#define my_alphasort alphasort64
typedef struct stat64 stat_struct;
typedef struct dirent64 dirent_struct;
#define my_stat stat64

#define TRACE  printf
#define TRACE_1 printf

#define VLC_URI "vlc://"

#define NUMBER_OF_MOVIES_LAST 40 // This is the number of movies shown in last recored and last played list
 
#define MESSAGEBOX_BROWSER_ROW_ITEM_COUNT 20
const CMenuOptionChooser::keyval MESSAGEBOX_BROWSER_ROW_ITEM[MESSAGEBOX_BROWSER_ROW_ITEM_COUNT] =
{
	{ MB_INFO_FILENAME, LOCALE_MOVIEBROWSER_INFO_FILENAME        },
	{ MB_INFO_FILEPATH, LOCALE_MOVIEBROWSER_INFO_PATH        },
	{ MB_INFO_TITLE, LOCALE_MOVIEBROWSER_INFO_TITLE        },
	{ MB_INFO_SERIE, LOCALE_MOVIEBROWSER_INFO_SERIE        },
	{ MB_INFO_INFO1, LOCALE_MOVIEBROWSER_INFO_INFO1        },
	{ MB_INFO_MAJOR_GENRE, LOCALE_MOVIEBROWSER_INFO_GENRE_MAJOR		},
	{ MB_INFO_MINOR_GENRE, LOCALE_MOVIEBROWSER_INFO_GENRE_MINOR		},
	{ MB_INFO_INFO2, LOCALE_MOVIEBROWSER_INFO_INFO2        },
	{ MB_INFO_PARENTAL_LOCKAGE, LOCALE_MOVIEBROWSER_INFO_PARENTAL_LOCKAGE		},
	{ MB_INFO_CHANNEL, LOCALE_MOVIEBROWSER_INFO_CHANNEL		},
	{ MB_INFO_BOOKMARK, LOCALE_MOVIEBROWSER_MENU_MAIN_BOOKMARKS		},
	{ MB_INFO_QUALITY, LOCALE_MOVIEBROWSER_INFO_QUALITY		},
	{ MB_INFO_PREVPLAYDATE, LOCALE_MOVIEBROWSER_INFO_PREVPLAYDATE		},
	{ MB_INFO_RECORDDATE, LOCALE_MOVIEBROWSER_INFO_RECORDDATE		},
	{ MB_INFO_PRODDATE, LOCALE_MOVIEBROWSER_INFO_PRODYEAR		},
	{ MB_INFO_COUNTRY, LOCALE_MOVIEBROWSER_INFO_PRODCOUNTRY		},
	{ MB_INFO_GEOMETRIE, LOCALE_MOVIEBROWSER_INFO_VIDEOFORMAT		},
	{ MB_INFO_AUDIO, LOCALE_MOVIEBROWSER_INFO_AUDIO		},
	{ MB_INFO_LENGTH, LOCALE_MOVIEBROWSER_INFO_LENGTH		},
	{ MB_INFO_SIZE, LOCALE_MOVIEBROWSER_INFO_SIZE		}
 };

#define MESSAGEBOX_YES_NO_OPTIONS_COUNT 2
const CMenuOptionChooser::keyval MESSAGEBOX_YES_NO_OPTIONS[MESSAGEBOX_YES_NO_OPTIONS_COUNT] =
{
	{ 0, LOCALE_MESSAGEBOX_NO},
	{ 1, LOCALE_MESSAGEBOX_YES}
};

#define MESSAGEBOX_PARENTAL_LOCK_OPTIONS_COUNT 3
const CMenuOptionChooser::keyval MESSAGEBOX_PARENTAL_LOCK_OPTIONS[MESSAGEBOX_PARENTAL_LOCK_OPTIONS_COUNT] =
{
	{ MB_PARENTAL_LOCK_ACTIVE , LOCALE_MOVIEBROWSER_MENU_PARENTAL_LOCK_ACTIVATED_YES     },
	{ MB_PARENTAL_LOCK_OFF    , LOCALE_MOVIEBROWSER_MENU_PARENTAL_LOCK_ACTIVATED_NO      },
	{ MB_PARENTAL_LOCK_OFF_TMP, LOCALE_MOVIEBROWSER_MENU_PARENTAL_LOCK_ACTIVATED_NO_TEMP }
};

#define MESSAGEBOX_PARENTAL_LOCKAGE_OPTION_COUNT 7
const CMenuOptionChooser::keyval MESSAGEBOX_PARENTAL_LOCKAGE_OPTIONS[MESSAGEBOX_PARENTAL_LOCKAGE_OPTION_COUNT] =
{
	{ MI_PARENTAL_OVER0 , LOCALE_MOVIEBROWSER_INFO_PARENTAL_LOCKAGE_0YEAR  },
	{ MI_PARENTAL_OVER6 , LOCALE_MOVIEBROWSER_INFO_PARENTAL_LOCKAGE_6YEAR  },
	{ MI_PARENTAL_OVER8 , LOCALE_MOVIEBROWSER_INFO_PARENTAL_LOCKAGE_8YEAR  },
	{ MI_PARENTAL_OVER12, LOCALE_MOVIEBROWSER_INFO_PARENTAL_LOCKAGE_12YEAR },
	{ MI_PARENTAL_OVER16, LOCALE_MOVIEBROWSER_INFO_PARENTAL_LOCKAGE_16YEAR },
	{ MI_PARENTAL_OVER18, LOCALE_MOVIEBROWSER_INFO_PARENTAL_LOCKAGE_18YEAR },
	{ MI_PARENTAL_ALWAYS, LOCALE_MOVIEBROWSER_INFO_PARENTAL_LOCKAGE_ALWAYS }
};

#define MAX_WINDOW_WIDTH  (w_max(720, 40))
#define MAX_WINDOW_HEIGHT (h_max(576, 40))

#define MIN_WINDOW_WIDTH  (w_max(720, 0) >> 1)
#define MIN_WINDOW_HEIGHT 200

#define TITLE_BACKGROUND_COLOR ((CFBWindow::color_t)COL_MENUHEAD_PLUS_0)
#define TITLE_FONT_COLOR ((CFBWindow::color_t)COL_MENUHEAD)

#define INTER_FRAME_SPACE 6  // space between e.g. upper and lower window
#define TEXT_BORDER_WIDTH 8

const neutrino_locale_t m_localizedItemName[MB_INFO_MAX_NUMBER+1] =
{
	LOCALE_MOVIEBROWSER_SHORT_FILENAME,
	LOCALE_MOVIEBROWSER_SHORT_PATH,
	LOCALE_MOVIEBROWSER_SHORT_TITLE ,
	LOCALE_MOVIEBROWSER_SHORT_SERIE,
	LOCALE_MOVIEBROWSER_SHORT_INFO1,
	LOCALE_MOVIEBROWSER_SHORT_GENRE_MAJOR,
	LOCALE_MOVIEBROWSER_SHORT_GENRE_MINOR,
	LOCALE_MOVIEBROWSER_SHORT_INFO2,
	LOCALE_MOVIEBROWSER_SHORT_PARENTAL_LOCKAGE	,
	LOCALE_MOVIEBROWSER_SHORT_CHANNEL ,
	LOCALE_MOVIEBROWSER_SHORT_BOOK,
	LOCALE_MOVIEBROWSER_SHORT_QUALITY,
	LOCALE_MOVIEBROWSER_SHORT_PREVPLAYDATE,
	LOCALE_MOVIEBROWSER_SHORT_RECORDDATE,
	LOCALE_MOVIEBROWSER_SHORT_PRODYEAR,
	LOCALE_MOVIEBROWSER_SHORT_COUNTRY,
	LOCALE_MOVIEBROWSER_SHORT_FORMAT ,
	LOCALE_MOVIEBROWSER_SHORT_AUDIO ,
	LOCALE_MOVIEBROWSER_SHORT_LENGTH,
	LOCALE_MOVIEBROWSER_SHORT_SIZE, 
	NONEXISTANT_LOCALE
};

// default row size in pixel for any element
#define	MB_ROW_WIDTH_FILENAME 		150
#define	MB_ROW_WIDTH_FILEPATH		150
#define	MB_ROW_WIDTH_TITLE			300
#define	MB_ROW_WIDTH_SERIE			100
#define	MB_ROW_WIDTH_INFO1			100
#define	MB_ROW_WIDTH_MAJOR_GENRE 	100
#define	MB_ROW_WIDTH_MINOR_GENRE 	30
#define	MB_ROW_WIDTH_INFO2 			30
#define	MB_ROW_WIDTH_PARENTAL_LOCKAGE 25 
#define	MB_ROW_WIDTH_CHANNEL		80
#define	MB_ROW_WIDTH_BOOKMARK		50
#define	MB_ROW_WIDTH_QUALITY 		25
#define	MB_ROW_WIDTH_PREVPLAYDATE	80
#define	MB_ROW_WIDTH_RECORDDATE 	80
#define	MB_ROW_WIDTH_PRODDATE 		50
#define	MB_ROW_WIDTH_COUNTRY 		50
#define	MB_ROW_WIDTH_GEOMETRIE		50
#define	MB_ROW_WIDTH_AUDIO			50 	
#define	MB_ROW_WIDTH_LENGTH			40
#define	MB_ROW_WIDTH_SIZE 			45

const int m_defaultRowWidth[MB_INFO_MAX_NUMBER+1] = 
{
	MB_ROW_WIDTH_FILENAME ,
	MB_ROW_WIDTH_FILEPATH,
	MB_ROW_WIDTH_TITLE,
	MB_ROW_WIDTH_SERIE,
	MB_ROW_WIDTH_INFO1,
	MB_ROW_WIDTH_MAJOR_GENRE ,
	MB_ROW_WIDTH_MINOR_GENRE ,
	MB_ROW_WIDTH_INFO2 ,
	MB_ROW_WIDTH_PARENTAL_LOCKAGE ,
	MB_ROW_WIDTH_CHANNEL,
	MB_ROW_WIDTH_BOOKMARK,
	MB_ROW_WIDTH_QUALITY ,
	MB_ROW_WIDTH_PREVPLAYDATE,
	MB_ROW_WIDTH_RECORDDATE ,
	MB_ROW_WIDTH_PRODDATE ,
	MB_ROW_WIDTH_COUNTRY ,
	MB_ROW_WIDTH_GEOMETRIE,
	MB_ROW_WIDTH_AUDIO 	,
	MB_ROW_WIDTH_LENGTH,
	MB_ROW_WIDTH_SIZE, 
	0 //MB_ROW_WIDTH_MAX_NUMBER 
};

//------------------------------------------------------------------------
// sorting
//------------------------------------------------------------------------
bool sortDirection = 0;

bool compare_to_lower(const char a, const char b)
{
	return tolower(a) < tolower(b);
}

// sort operators
bool sortByTitle (const MI_MOVIE_INFO* a, const MI_MOVIE_INFO* b)
{
	if (std::lexicographical_compare(a->epgTitle.begin(), a->epgTitle.end(), b->epgTitle.begin(), b->epgTitle.end(), compare_to_lower))
		return true;
	if (std::lexicographical_compare(b->epgTitle.begin(), b->epgTitle.end(), a->epgTitle.begin(), a->epgTitle.end(), compare_to_lower))
		return false;
	return a->file.Time < b->file.Time;
}
bool sortBySerie (const MI_MOVIE_INFO* a, const MI_MOVIE_INFO* b)
{
	if (std::lexicographical_compare(a->serieName.begin(), a->serieName.end(), b->serieName.begin(), b->serieName.end(), compare_to_lower))
		return true;
	if (std::lexicographical_compare(b->serieName.begin(), b->serieName.end(), a->serieName.begin(), a->serieName.end(), compare_to_lower))
		return false;
	return sortByTitle(a,b);
}
bool sortByInfo1 (const MI_MOVIE_INFO* a, const MI_MOVIE_INFO* b)
{
	if (std::lexicographical_compare(a->epgInfo1.begin(), a->epgInfo1.end(), b->epgInfo1.begin(), b->epgInfo1.end(), compare_to_lower))
		return true;
	if (std::lexicographical_compare(b->epgInfo1.begin(), b->epgInfo1.end(), a->epgInfo1.begin(), a->epgInfo1.end(), compare_to_lower))
		return false;
	return sortByTitle(a,b);
}
bool sortByChannel (const MI_MOVIE_INFO* a, const MI_MOVIE_INFO* b)
{
	if (std::lexicographical_compare(a->epgChannel.begin(), a->epgChannel.end(), b->epgChannel.begin(), b->epgChannel.end(), compare_to_lower))
		return true;
	if (std::lexicographical_compare(b->epgChannel.begin(), b->epgChannel.end(), a->epgChannel.begin(), a->epgChannel.end(), compare_to_lower))
		return false;
	return sortByTitle(a,b);
}
bool sortByFileName (const MI_MOVIE_INFO* a, const MI_MOVIE_INFO* b)
{
	if (std::lexicographical_compare(a->file.getFileName().begin(), a->file.getFileName().end(), b->file.getFileName().begin(), b->file.getFileName().end(), compare_to_lower))
		return true;
	if (std::lexicographical_compare(b->file.getFileName().begin(), b->file.getFileName().end(), a->file.getFileName().begin(), a->file.getFileName().end(), compare_to_lower))
		return false;
	return a->file.Time < b->file.Time;
}
bool sortByRecordDate (const MI_MOVIE_INFO* a, const MI_MOVIE_INFO* b)
{
	if(sortDirection)
		return a->file.Time > b->file.Time ;
	else
		return a->file.Time < b->file.Time ;
}
bool sortBySize (const MI_MOVIE_INFO* a, const MI_MOVIE_INFO* b)
{
	if(sortDirection)
		return a->file.Size > b->file.Size;
	else
		return a->file.Size < b->file.Size;
}
bool sortByAge (const MI_MOVIE_INFO* a, const MI_MOVIE_INFO* b)
{
	if(sortDirection)
		return a->parentalLockAge > b->parentalLockAge;
	else
		return a->parentalLockAge < b->parentalLockAge;
}
bool sortByQuality (const MI_MOVIE_INFO* a, const MI_MOVIE_INFO* b)
{
	if(sortDirection)
		return a->quality > b->quality;
	else
		return a->quality < b->quality;
}
bool sortByDir (const MI_MOVIE_INFO* a, const MI_MOVIE_INFO* b)
{
	if(sortDirection)
		return a->dirItNr > b->dirItNr;
	else
		return a->dirItNr < b->dirItNr;
}

bool sortByLastPlay (const MI_MOVIE_INFO* a, const MI_MOVIE_INFO* b)
{
	if(sortDirection)
		return a->dateOfLastPlay > b->dateOfLastPlay;
	else
		return a->dateOfLastPlay < b->dateOfLastPlay;
}

bool (* const sortBy[MB_INFO_MAX_NUMBER+1])(const MI_MOVIE_INFO* a, const MI_MOVIE_INFO* b) =
{
	&sortByFileName ,	//MB_INFO_FILENAME		= 0,
	&sortByDir, 		//MB_INFO_FILEPATH		= 1,
	&sortByTitle, 		//MB_INFO_TITLE			= 2,
	&sortBySerie,		//MB_INFO_SERIE 		= 3,
	&sortByInfo1, 		//MB_INFO_INFO1			= 4,
	NULL, 				//MB_INFO_MAJOR_GENRE 	= 5,
	NULL, 				//MB_INFO_MINOR_GENRE 	= 6,
	NULL, 				//MB_INFO_INFO2 			= 7,
	&sortByAge, 		//MB_INFO_PARENTAL_LOCKAGE			= 8,
	&sortByChannel, 	//MB_INFO_CHANNEL		= 9,
	NULL, 				//MB_INFO_BOOKMARK		= 10,
	&sortByQuality, 	//MB_INFO_QUALITY		= 11,
	&sortByLastPlay, 	//MB_INFO_PREVPLAYDATE 	= 12,
	&sortByRecordDate, 	//MB_INFO_RECORDDATE	= 13,
	NULL, 				//MB_INFO_PRODDATE 		= 14,
	NULL, 				//MB_INFO_COUNTRY 		= 15,
	NULL, 				//MB_INFO_GEOMETRIE 	= 16,
	NULL, 				//MB_INFO_AUDIO 		= 17,
	NULL, 				//MB_INFO_LENGTH 		= 18,
	&sortBySize, 		//MB_INFO_SIZE 			= 19, 
	NULL				//MB_INFO_MAX_NUMBER		= 20
};
/************************************************************************
 Public API
************************************************************************/


/************************************************************************

************************************************************************/
CMovieBrowser::CMovieBrowser(const char* path): configfile ('\t')
{
	m_selectedDir = path; 
	//addDir(m_selectedDir);
	CMovieBrowser();
}
/************************************************************************

************************************************************************/
CMovieBrowser::CMovieBrowser(): configfile ('\t')
{
	TRACE("$Id: moviebrowser.cpp,v 1.80 2012/11/01 19:31:29 rhabarber1848 Exp $\r\n");
	init();
}

/************************************************************************

************************************************************************/
CMovieBrowser::~CMovieBrowser()
{
	//TRACE("[mb] del\r\n");
	//saveSettings(&m_settings);		
	hide();
}

CMovieBrowser* CMovieBrowser::getInstance()
{
	static CMovieBrowser* moviebrowser = NULL;

	if(!moviebrowser)
	{
		moviebrowser = new CMovieBrowser();
		printf("Moviebrowser Instance created\n");
	}
	return moviebrowser;
}

/************************************************************************

************************************************************************/
void CMovieBrowser::fileInfoStale(void)
{
	m_file_info_stale = true;
	m_seriename_stale = true;
	
	 // Also release memory buffers, since we have to reload this stuff next time anyhow 
	m_dirNames.clear();
	
	m_vMovieInfo.clear();
	m_vHandleBrowserList.clear();
	m_vHandleRecordList.clear();
	m_vHandlePlayList.clear();
	m_vHandleSerienames.clear();
	
	for(int i = 0; i < LF_MAX_ROWS; i++)
	{
		m_browserListLines.lineArray[i].clear();
		m_recordListLines.lineArray[i].clear();
		m_playListLines.lineArray[i].clear();
		m_FilterLines.lineArray[i].clear();
	}

}


/************************************************************************

************************************************************************/
void CMovieBrowser::init(void)
{
	//TRACE("[mb]->init\r\n");
	initGlobalSettings();
	loadSettings(&m_settings);
		
	//restart_mb_timeout = 0;
	m_file_info_stale = true;
	m_seriename_stale = true;

	m_pcWindow = NULL;
	m_pcBrowser = NULL;
	m_pcLastPlay = NULL;
	m_pcLastRecord = NULL;
	m_pcInfo = NULL;
	m_pcFilter = NULL;

	m_windowFocus = MB_FOCUS_BROWSER;

	initFonts();
	
	m_textTitle = g_Locale->getText(LOCALE_MOVIEBROWSER_HEAD);
	
	m_currentStartPos = 0;
	m_playstate = CMoviePlayerGui::STOPPED;
	
	m_movieSelectionHandler = NULL;
	m_currentBrowserSelection = 0;
	m_currentRecordSelection = 0;
	m_currentPlaySelection = 0;
 	m_prevBrowserSelection = 0;
	m_prevRecordSelection = 0;
	m_prevPlaySelection = 0;
	
	m_storageType = MB_STORAGE_TYPE_NFS;
	
	m_parentalLock = m_settings.parentalLock;
	
	// check g_setting values 
	if(m_settings.gui >= MB_GUI_MAX_NUMBER)
		m_settings.gui = MB_GUI_MOVIE_INFO;
	
	if(m_settings.sorting.direction >= MB_DIRECTION_MAX_NUMBER)
		m_settings.sorting.direction = MB_DIRECTION_DOWN;
	if(m_settings.sorting.item 	>=  MB_INFO_MAX_NUMBER)
		m_settings.sorting.item =  MB_INFO_TITLE;

	if(m_settings.filter.item >= MB_INFO_MAX_NUMBER)
		m_settings.filter.item = MB_INFO_MAX_NUMBER;
	
	if(m_settings.parentalLockAge >= MI_PARENTAL_MAX_NUMBER)
		m_settings.parentalLockAge = MI_PARENTAL_OVER18;
	if(m_settings.parentalLock >= MB_PARENTAL_LOCK_MAX_NUMBER)
		m_settings.parentalLock = MB_PARENTAL_LOCK_OFF;
	
	if(m_settings.browserFrameHeight < MIN_BROWSER_FRAME_HEIGHT )
		m_settings.browserFrameHeight = MIN_BROWSER_FRAME_HEIGHT;
	if(m_settings.browserFrameHeight > MAX_BROWSER_FRAME_HEIGHT)
		m_settings.browserFrameHeight = MAX_BROWSER_FRAME_HEIGHT;
	/***** Browser List **************/
	if(m_settings.browserRowNr == 0)
	{
		TRACE(" row error\r\n");
		// init browser row elements if not configured correctly by neutrino.config
		m_settings.browserRowNr = 6;
		m_settings.browserRowItem[0] = MB_INFO_TITLE;
		m_settings.browserRowItem[1] = MB_INFO_INFO1;
		m_settings.browserRowItem[2] = MB_INFO_RECORDDATE;
		m_settings.browserRowItem[3] = MB_INFO_SIZE;
		m_settings.browserRowItem[4] = MB_INFO_PARENTAL_LOCKAGE;
		m_settings.browserRowItem[5] = MB_INFO_QUALITY;
		m_settings.browserRowWidth[0] = m_defaultRowWidth[m_settings.browserRowItem[0]];		//300;
		m_settings.browserRowWidth[1] = m_defaultRowWidth[m_settings.browserRowItem[1]]; 		//100;
		m_settings.browserRowWidth[2] = m_defaultRowWidth[m_settings.browserRowItem[2]]; 		//80;
		m_settings.browserRowWidth[3] = m_defaultRowWidth[m_settings.browserRowItem[3]]; 		//50;
		m_settings.browserRowWidth[4] = m_defaultRowWidth[m_settings.browserRowItem[4]]; 		//30;
		m_settings.browserRowWidth[5] = m_defaultRowWidth[m_settings.browserRowItem[5]]; 		//30;
	}

	initFrames();
	initRows();
	//initDevelopment();

	refreshLastPlayList();	
	refreshLastRecordList();	
	refreshBrowserList();	
	refreshFilterList();	
#if 0
	TRACE_1("Frames\r\n\tScren:\t%3d,%3d,%3d,%3d\r\n\tMain:\t%3d,%3d,%3d,%3d\r\n\tTitle:\t%3d,%3d,%3d,%3d \r\n\tBrowsr:\t%3d,%3d,%3d,%3d \r\n\tPlay:\t%3d,%3d,%3d,%3d \r\n\tRecord:\t%3d,%3d,%3d,%3d\r\n\r\n",
	g_settings.screen_StartX,
	g_settings.screen_StartY,
	g_settings.screen_EndX,
	g_settings.screen_EndY,
	m_cBoxFrame.iX,
	m_cBoxFrame.iY,
	m_cBoxFrame.iWidth,
	m_cBoxFrame.iHeight,
	m_cBoxFrameTitleRel.iX,
	m_cBoxFrameTitleRel.iY,
	m_cBoxFrameTitleRel.iWidth,
	m_cBoxFrameTitleRel.iHeight,
	m_cBoxFrameBrowserList.iX,
	m_cBoxFrameBrowserList.iY,
	m_cBoxFrameBrowserList.iWidth,
	m_cBoxFrameBrowserList.iHeight,
	m_cBoxFrameLastPlayList.iX,
	m_cBoxFrameLastPlayList.iY,
	m_cBoxFrameLastPlayList.iWidth,
	m_cBoxFrameLastPlayList.iHeight,
	m_cBoxFrameLastRecordList.iX,
	m_cBoxFrameLastRecordList.iY,
	m_cBoxFrameLastRecordList.iWidth,
	m_cBoxFrameLastRecordList.iHeight
	);
#endif
}

/************************************************************************

************************************************************************/
void CMovieBrowser::initGlobalSettings(void)
{
	//TRACE("[mb]->initGlobalSettings\r\n");
	
	m_settings.gui = MB_GUI_MOVIE_INFO;
	
	m_settings.browser_serie_mode = 0;
	m_settings.serie_auto_create = 0;

	m_settings.sorting.direction = MB_DIRECTION_DOWN;
	m_settings.sorting.item 	=  MB_INFO_TITLE;

	m_settings.filter.item = MB_INFO_MAX_NUMBER;
	m_settings.filter.optionString = "---";
	m_settings.filter.optionVar = 0;
	
	m_settings.parentalLockAge = MI_PARENTAL_OVER18;
	m_settings.parentalLock = MB_PARENTAL_LOCK_OFF;
	
	for(int i = 0; i < MB_MAX_DIRS; i++)
	{
		m_settings.storageDir[i] = "";
		m_settings.storageDirUsed[i] = 0;
	}

	/***** Browser List **************/
	m_settings.browserFrameHeight = 250;
	
	m_settings.browserRowNr = 6;
	m_settings.browserRowItem[0] = MB_INFO_TITLE;
	m_settings.browserRowItem[1] = MB_INFO_INFO1;
	m_settings.browserRowItem[2] = MB_INFO_RECORDDATE;
	m_settings.browserRowItem[3] = MB_INFO_SIZE;
	m_settings.browserRowItem[4] = MB_INFO_PARENTAL_LOCKAGE;
	m_settings.browserRowItem[5] = MB_INFO_QUALITY;
	m_settings.browserRowWidth[0] = m_defaultRowWidth[m_settings.browserRowItem[0]];		//300;
	m_settings.browserRowWidth[1] = m_defaultRowWidth[m_settings.browserRowItem[1]]; 		//100;
	m_settings.browserRowWidth[2] = m_defaultRowWidth[m_settings.browserRowItem[2]]; 		//80;
	m_settings.browserRowWidth[3] = m_defaultRowWidth[m_settings.browserRowItem[3]]; 		//50;
	m_settings.browserRowWidth[4] = m_defaultRowWidth[m_settings.browserRowItem[4]]; 		//30;
	m_settings.browserRowWidth[5] = m_defaultRowWidth[m_settings.browserRowItem[5]]; 		//30;

	m_settings.storageDirMovieUsed = true;
	for(int i = 0; i < MAX_RECORDING_DIR ; i++)
	{
		m_settings.storageDirRecUsed[i] = true;
	}
	m_settings.reload = false;
	m_settings.remount = false;
}

/************************************************************************
 
************************************************************************/
void CMovieBrowser::initFrames(void)
{
	//TRACE("[mb]->initFrames\r\n");
	m_cBoxFrame.iWidth = 			w_max(720, 20);
	m_cBoxFrame.iHeight = 			h_max(720, 20);
	m_cBoxFrame.iX = 			getScreenStartX(m_cBoxFrame.iWidth);
	m_cBoxFrame.iY = 			getScreenStartY(m_cBoxFrame.iHeight);

	CFrameBuffer::getInstance()->getIconSize(NEUTRINO_ICON_EPGINFO, &m_iconWidthTitle, &m_iconHeightTitle);

	m_cBoxFrameTitleRel.iX = 		0;
	m_cBoxFrameTitleRel.iY = 		0;
	m_cBoxFrameTitleRel.iWidth = 		m_cBoxFrame.iWidth;
	m_cBoxFrameTitleRel.iHeight = 		std::max(m_iconHeightTitle, m_pcFontTitle->getHeight());

	m_cBoxFrameBrowserList.iX = 		m_cBoxFrame.iX;
	m_cBoxFrameBrowserList.iY = 		m_cBoxFrame.iY + m_cBoxFrameTitleRel.iHeight;
	m_cBoxFrameBrowserList.iWidth = 	m_cBoxFrame.iWidth;
	m_cBoxFrameBrowserList.iHeight = 	m_settings.browserFrameHeight; //m_cBoxFrame.iHeight - (m_cBoxFrame.iHeight>>1) - (INTER_FRAME_SPACE>>1);

	m_cBoxFrameFootRel.iX = 		0;
	m_cBoxFrameFootRel.iY = 		m_cBoxFrame.iHeight - m_pcFontFoot->getHeight();
	m_cBoxFrameFootRel.iWidth = 		m_cBoxFrameBrowserList.iWidth;
	m_cBoxFrameFootRel.iHeight = 		m_pcFontFoot->getHeight();

	m_cBoxFrameLastPlayList.iX = 		m_cBoxFrameBrowserList.iX;
	m_cBoxFrameLastPlayList.iY = 		m_cBoxFrameBrowserList.iY ;
	m_cBoxFrameLastPlayList.iWidth = 	(m_cBoxFrameBrowserList.iWidth>>1) - (INTER_FRAME_SPACE>>1);
	m_cBoxFrameLastPlayList.iHeight = 	m_cBoxFrameBrowserList.iHeight;
	
	m_cBoxFrameLastRecordList.iX = 		m_cBoxFrameLastPlayList.iX + m_cBoxFrameLastPlayList.iWidth + INTER_FRAME_SPACE;
	m_cBoxFrameLastRecordList.iY = 		m_cBoxFrameLastPlayList.iY;
	m_cBoxFrameLastRecordList.iWidth = 	m_cBoxFrame.iWidth - m_cBoxFrameLastPlayList.iWidth - INTER_FRAME_SPACE;
	m_cBoxFrameLastRecordList.iHeight = 	m_cBoxFrameLastPlayList.iHeight;
	
	m_cBoxFrameInfo.iX = 			m_cBoxFrameBrowserList.iX;
	m_cBoxFrameInfo.iY = 			m_cBoxFrameBrowserList.iY + m_cBoxFrameBrowserList.iHeight + INTER_FRAME_SPACE;
	m_cBoxFrameInfo.iWidth = 		m_cBoxFrameBrowserList.iWidth;
	m_cBoxFrameInfo.iHeight = 		m_cBoxFrame.iHeight - m_cBoxFrameBrowserList.iHeight - INTER_FRAME_SPACE - m_cBoxFrameFootRel.iHeight - m_cBoxFrameTitleRel.iHeight;
	
	m_cBoxFrameFilter.iX = 			m_cBoxFrameInfo.iX;
	m_cBoxFrameFilter.iY = 			m_cBoxFrameInfo.iY;
	m_cBoxFrameFilter.iWidth = 		m_cBoxFrameInfo.iWidth;
	m_cBoxFrameFilter.iHeight = 		m_cBoxFrameInfo.iHeight;
}
	



/************************************************************************
 
************************************************************************/
void CMovieBrowser::initRows(void)
{
	//TRACE("[mb]->initRows\r\n");

	/***** Last Play List **************/
	m_settings.lastPlayRowNr = 2;
	m_settings.lastPlayRow[0] = MB_INFO_TITLE;
	m_settings.lastPlayRow[1] = MB_INFO_PREVPLAYDATE;
	m_settings.lastPlayRow[2] = MB_INFO_MAX_NUMBER;
	m_settings.lastPlayRow[3] = MB_INFO_MAX_NUMBER;
	m_settings.lastPlayRow[4] = MB_INFO_MAX_NUMBER;
	m_settings.lastPlayRow[5] = MB_INFO_MAX_NUMBER;
	m_settings.lastPlayRowWidth[0] = 190;
	m_settings.lastPlayRowWidth[1]  = m_defaultRowWidth[m_settings.lastPlayRow[1]];
	m_settings.lastPlayRowWidth[2] = m_defaultRowWidth[m_settings.lastPlayRow[2]];
	m_settings.lastPlayRowWidth[3] = m_defaultRowWidth[m_settings.lastPlayRow[3]];
	m_settings.lastPlayRowWidth[4] = m_defaultRowWidth[m_settings.lastPlayRow[4]];
	m_settings.lastPlayRowWidth[5] = m_defaultRowWidth[m_settings.lastPlayRow[5]];
	
	/***** Last Record List **************/
	m_settings.lastRecordRowNr = 2;
	m_settings.lastRecordRow[0] = MB_INFO_TITLE;
	m_settings.lastRecordRow[1] = MB_INFO_RECORDDATE;
	m_settings.lastRecordRow[2] = MB_INFO_MAX_NUMBER;
	m_settings.lastRecordRow[3] = MB_INFO_MAX_NUMBER;
	m_settings.lastRecordRow[4] = MB_INFO_MAX_NUMBER;
	m_settings.lastRecordRow[5] = MB_INFO_MAX_NUMBER;
	m_settings.lastRecordRowWidth[0] = 190;
	m_settings.lastRecordRowWidth[1] = m_defaultRowWidth[m_settings.lastRecordRow[1]];
	m_settings.lastRecordRowWidth[2] = m_defaultRowWidth[m_settings.lastRecordRow[2]];
	m_settings.lastRecordRowWidth[3] = m_defaultRowWidth[m_settings.lastRecordRow[3]];
	m_settings.lastRecordRowWidth[4] = m_defaultRowWidth[m_settings.lastRecordRow[4]];
	m_settings.lastRecordRowWidth[5] = m_defaultRowWidth[m_settings.lastRecordRow[5]];
}
/************************************************************************
 
************************************************************************/
void CMovieBrowser::initDevelopment(void)
{
	TRACE("[mb]->initDevelopment\r\n");
	std::string name;
	name = "/mnt/movies/";
	//addDir(name);
	name = "/mnt/record/";
	//addDir(name);
	name = "/mnt/nfs/";
	//addDir(name);
	
}

void CMovieBrowser::defaultSettings(MB_SETTINGS* settings)
{
	CFile file;
	file.Name = MOVIEBROWSER_SETTINGS_FILE;
	delFile(file);
	loadSettings(settings);
}

/************************************************************************
 
************************************************************************/
bool CMovieBrowser::loadSettings(MB_SETTINGS* settings)
{
	bool result = true;
	//TRACE("CMovieBrowser::loadSettings\r\n"); 
	if(configfile.loadConfig(MOVIEBROWSER_SETTINGS_FILE))
	{
		settings->lastPlayMaxItems = configfile.getInt32("mb_lastPlayMaxItems", NUMBER_OF_MOVIES_LAST);
		settings->lastRecordMaxItems = configfile.getInt32("mb_lastRecordMaxItems", NUMBER_OF_MOVIES_LAST);
		settings->browser_serie_mode = configfile.getInt32("mb_browser_serie_mode", 0);
		settings->serie_auto_create = configfile.getInt32("mb_serie_auto_create", 0);

		settings->gui = (MB_GUI)configfile.getInt32("mb_gui", MB_GUI_MOVIE_INFO);
		
		settings->sorting.item = (MB_INFO_ITEM)configfile.getInt32("mb_sorting_item", MB_INFO_TITLE);
		settings->sorting.direction = (MB_DIRECTION)configfile.getInt32("mb_sorting_direction", MB_DIRECTION_UP);
		
		settings->filter.item = (MB_INFO_ITEM)configfile.getInt32("mb_filter_item", MB_INFO_INFO1);
		settings->filter.optionString = configfile.getString("mb_filter_optionString", "---");
		settings->filter.optionVar = configfile.getInt32("mb_filter_optionVar", 0);
		
		settings->parentalLockAge = (MI_PARENTAL_LOCKAGE)configfile.getInt32("mb_parentalLockAge", MI_PARENTAL_OVER18);
		settings->parentalLock = (MB_PARENTAL_LOCK)configfile.getInt32("mb_parentalLock", MB_PARENTAL_LOCK_ACTIVE);
	
		char cfg_key[81];
		for(int i = 0; i < MAX_RECORDING_DIR ; i++)
		{
			sprintf(cfg_key, "mb_storageDir_rec_%d", i);
			settings->storageDirRecUsed[i] = (bool)configfile.getInt32(cfg_key, true );
		}
		settings->storageDirMovieUsed = (bool)configfile.getInt32("mb_storageDir_movie", true );
		
		settings->reload = (bool)configfile.getInt32("mb_reload", false );
		settings->remount = (bool)configfile.getInt32("mb_remount", false );

		for(int i = 0; i < MB_MAX_DIRS; i++)
		{
			sprintf(cfg_key, "mb_dir_%d", i);
			settings->storageDir[i] = configfile.getString( cfg_key, "" );
			sprintf(cfg_key, "mb_dir_used%d", i);
			settings->storageDirUsed[i] = configfile.getInt32( cfg_key,false );
		}
		/* these variables are used for the listframes */	
		settings->browserFrameHeight  = configfile.getInt32("mb_browserFrameHeight", 250);
		settings->browserRowNr  = configfile.getInt32("mb_browserRowNr", 0);
		for(int i = 0; i < MB_MAX_ROWS && i < settings->browserRowNr; i++)
		{
			sprintf(cfg_key, "mb_browserRowItem_%d", i);
			settings->browserRowItem[i] = (MB_INFO_ITEM)configfile.getInt32(cfg_key, MB_INFO_MAX_NUMBER);
			sprintf(cfg_key, "mb_browserRowWidth_%d", i);
			settings->browserRowWidth[i] = configfile.getInt32(cfg_key, 50);
		}
	}
	else
	{
		TRACE("CMovieBrowser::loadSettings failed\r\n"); 
		configfile.clear();
		result = false;
	}
	return (result);
}

/************************************************************************
 
************************************************************************/
bool CMovieBrowser::saveSettings(MB_SETTINGS* settings)
{
	bool result = true;
	TRACE("[mb] saveSettings\r\n"); 

	configfile.setInt32("mb_lastPlayMaxItems", settings->lastPlayMaxItems);
	configfile.setInt32("mb_lastRecordMaxItems", settings->lastRecordMaxItems);
	configfile.setInt32("mb_browser_serie_mode", settings->browser_serie_mode);
	configfile.setInt32("mb_serie_auto_create", settings->serie_auto_create);

	configfile.setInt32("mb_gui", settings->gui);
	
	configfile.setInt32("mb_sorting_item", settings->sorting.item);
	configfile.setInt32("mb_sorting_direction", settings->sorting.direction);
	
	configfile.setInt32("mb_filter_item", settings->filter.item);
	configfile.setString("mb_filter_optionString", settings->filter.optionString);
	configfile.setInt32("mb_filter_optionVar", settings->filter.optionVar);
	
	char cfg_key[81];
	for(int i = 0; i < MAX_RECORDING_DIR ; i++)
	{
		sprintf(cfg_key, "mb_storageDir_rec_%d", i);
		configfile.setInt32(cfg_key, settings->storageDirRecUsed[i] );
	}
	configfile.setInt32("mb_storageDir_movie", settings->storageDirMovieUsed );
	
	configfile.setInt32("mb_parentalLockAge", settings->parentalLockAge);
	configfile.setInt32("mb_parentalLock", settings->parentalLock);

	configfile.setInt32("mb_reload", settings->reload);
	configfile.setInt32("mb_remount", settings->remount);

	for(int i = 0; i < MB_MAX_DIRS; i++)
	{
		sprintf(cfg_key, "mb_dir_%d", i);
		configfile.setString( cfg_key, settings->storageDir[i] );
		sprintf(cfg_key, "mb_dir_used%d", i);
		configfile.setInt32( cfg_key, settings->storageDirUsed[i] ); // do not save this so far
	}
	/* these variables are used for the listframes */	
	configfile.setInt32("mb_browserFrameHeight", settings->browserFrameHeight);
	configfile.setInt32("mb_browserRowNr",settings->browserRowNr);
	for(int i = 0; i < MB_MAX_ROWS && i < settings->browserRowNr; i++)
	{
		sprintf(cfg_key, "mb_browserRowItem_%d", i);
		configfile.setInt32(cfg_key, settings->browserRowItem[i]);
		sprintf(cfg_key, "mb_browserRowWidth_%d", i);
		configfile.setInt32(cfg_key, settings->browserRowWidth[i]);
	}

	if (configfile.getModifiedFlag())
		configfile.saveConfig(MOVIEBROWSER_SETTINGS_FILE);
	return (result);
}
/************************************************************************

************************************************************************/
int CMovieBrowser::exec(CMenuTarget* parent, const std::string & actionKey)
{
	int returnval = menu_return::RETURN_REPAINT;

	if(actionKey == "loaddefault")
	{
		defaultSettings(&m_settings);
	}
	else if(actionKey == "save_options")
	{
	}
	else if(actionKey == "show_movie_info_menu")
	{
		if(m_movieSelectionHandler != NULL)
			returnval = showMovieInfoMenu(m_movieSelectionHandler);
	}
	else if(actionKey == "save_movie_info")
	{
		if(m_movieSelectionHandler != NULL)
			m_movieInfo.saveMovieInfo( *m_movieSelectionHandler);
	}
	else if(actionKey == "save_movie_info_all")
	{
		std::vector<MI_MOVIE_INFO*> * current_list=NULL;

		if(m_movieSelectionHandler != NULL)
		{
			if(m_windowFocus == MB_FOCUS_BROWSER)          current_list = &m_vHandleBrowserList;
			else if(m_windowFocus == MB_FOCUS_LAST_PLAY)   current_list = &m_vHandlePlayList;
			else if(m_windowFocus == MB_FOCUS_LAST_RECORD) current_list = &m_vHandleRecordList ;

			if(current_list != NULL)
			{
				CHintBox loadBox(LOCALE_MOVIEBROWSER_HEAD,g_Locale->getText(LOCALE_MOVIEBROWSER_INFO_HEAD_UPDATE));
				loadBox.paint();
				for(unsigned int i = 0; i< current_list->size();i++)
				{
					if( !((*current_list)[i]->parentalLockAge != 0 && movieInfoUpdateAllIfDestEmptyOnly == true) &&
						movieInfoUpdateAll[MB_INFO_TITLE] )
							(*current_list)[i]->parentalLockAge = m_movieSelectionHandler->parentalLockAge;

					if( !(!(*current_list)[i]->serieName.empty() && movieInfoUpdateAllIfDestEmptyOnly == true) &&
						movieInfoUpdateAll[MB_INFO_SERIE] )
						(*current_list)[i]->serieName = m_movieSelectionHandler->serieName;

					if( !(!(*current_list)[i]->productionCountry.empty() && movieInfoUpdateAllIfDestEmptyOnly == true) &&
						movieInfoUpdateAll[MB_INFO_COUNTRY] )
						(*current_list)[i]->productionCountry = m_movieSelectionHandler->productionCountry;

					if( !((*current_list)[i]->genreMajor!=0 && movieInfoUpdateAllIfDestEmptyOnly == true) &&
						movieInfoUpdateAll[MB_INFO_MAJOR_GENRE] )
						(*current_list)[i]->genreMajor = m_movieSelectionHandler->genreMajor;

					if( !((*current_list)[i]->quality!=0 && movieInfoUpdateAllIfDestEmptyOnly == true) &&
				        	movieInfoUpdateAll[MB_INFO_QUALITY] )
    						(*current_list)[i]->quality = m_movieSelectionHandler->quality;

    				m_movieInfo.saveMovieInfo( *((*current_list)[i]) );
				}
				loadBox.hide();
			}
		}
	}
	else if(actionKey == "reload_movie_info")
	{
		loadMovies(false);
		updateMovieSelection();
	}
	else if(actionKey == "run")
	{
		if(parent) parent->hide ();
		exec(NULL, CMoviePlayerGui::STOPPED);
	}
	else if(actionKey == "book_clear_all")
	{
		if(m_movieSelectionHandler != NULL)
		{
			m_movieSelectionHandler->bookmarks.start =0;
			m_movieSelectionHandler->bookmarks.end =0;
			m_movieSelectionHandler->bookmarks.lastPlayStop =0;
			for(int i=0; i<MI_MOVIE_BOOK_USER_MAX;i++)
			{
				m_movieSelectionHandler->bookmarks.user[i].name.empty();
				m_movieSelectionHandler->bookmarks.user[i].length =0;
				m_movieSelectionHandler->bookmarks.user[i].pos =0;
			}
		}
	}
     return returnval;
}

int CMovieBrowser::exec(const char* path, const int playstate)
{
	bool res = false;

	TRACE("[mb] start MovieBrowser\r\n");
	int timeout = -1;
	int returnDefaultOnTimeout = true;
	neutrino_msg_t      msg;
	neutrino_msg_data_t data;

	// might be removed, for development it is good to reload the settings at any startup for testing
	//loadSettings(&m_settings);
	
	// Clear all, to avoid 'jump' in screen 
	m_vHandleBrowserList.clear();
	m_vHandleRecordList.clear();
	m_vHandlePlayList.clear();

	for(int i = 0; i < LF_MAX_ROWS; i++)
	{
		m_browserListLines.lineArray[i].clear();
		m_recordListLines.lineArray[i].clear();
		m_playListLines.lineArray[i].clear();
	}

	if (path != NULL)
		m_selectedDir = path;
	else if (!g_settings.streaming_moviedir.empty())
		m_selectedDir = g_settings.streaming_moviedir;
	else
		m_selectedDir = "/";
	
	m_playstate = playstate;

	if(paint() == false)
		return res;// paint failed due to less memory , exit 

	if ( timeout == -1 )
		timeout = g_settings.timing[SNeutrinoSettings::TIMING_FILEBROWSER];

	unsigned long long timeoutEnd = CRCInput::calcTimeoutEnd( timeout );
	
	if(m_settings.remount == true)
	{
		TRACE("[mb] remount\r\n");
		//umount automount dirs
		for(int i = 0; i < NETWORK_NFS_NR_OF_ENTRIES; i++)
		{
			if(g_settings.network_nfs_automount[i])
				umount2(g_settings.network_nfs_local_dir[i],MNT_FORCE);
		}
#ifdef ENABLE_GUI_MOUNT
		CFSMounter::automount();
#endif
	}

	if(m_file_info_stale == true)
	{
		TRACE("[mb] reload\r\n");
		loadMovies();
		refreshLCD();
	}
	else
	{
		// since we cleared everything above, we have to refresh the list now.
		refreshBrowserList();	
		refreshLastPlayList();	
		refreshLastRecordList();
	}
	
	// get old movie selection and set position in windows	
 	m_currentBrowserSelection = m_prevBrowserSelection;
	m_currentRecordSelection = m_prevRecordSelection;
	m_currentPlaySelection = m_prevPlaySelection;

	m_pcBrowser->setSelectedLine(m_currentBrowserSelection);
	m_pcLastRecord->setSelectedLine(m_currentRecordSelection);
	m_pcLastPlay->setSelectedLine(m_currentPlaySelection);
	
	updateMovieSelection();
	//refreshMovieInfo();
 
	refreshTitle();
	onSetGUIWindow(m_settings.gui);
 
	bool loop = !m_vMovieInfo.empty(); // if the movie archive is empty, skip the loop
	bool result;
	g_RCInput->clearRCMsg();	//
	while (loop)
	{
		g_RCInput->getMsgAbsoluteTimeout( &msg, &data, &timeoutEnd );

		result = onButtonPress(msg);
		if(result == false)
		{
			if (msg == CRCInput::RC_timeout && returnDefaultOnTimeout)
			{
				TRACE("[mb] Timerevent\n");
				//if( restart_mb_timeout == 1)
				//    restart_mb_timeout = 0;
				//else
				loop = false;
			}
			else if(msg == CRCInput::RC_ok && m_vHandleBrowserList.empty())
			{
			}
			else if(msg == CRCInput::RC_ok && !m_vHandleBrowserList.empty())
			{
				m_currentStartPos = 0; 
				
				if(m_movieSelectionHandler != NULL)
				{
					// If there is any available bookmark, show the bookmark menu
					if( m_movieSelectionHandler->bookmarks.lastPlayStop != 0 ||
						m_movieSelectionHandler->bookmarks.start != 0)
					{
						TRACE("[mb] stop: %d start:%d \r\n",m_movieSelectionHandler->bookmarks.lastPlayStop,m_movieSelectionHandler->bookmarks.start);
						m_currentStartPos = showStartPosSelectionMenu(); // display start menu m_currentStartPos = 
					}
				}
				TRACE("[mb] start pos: %d s\r\n",m_currentStartPos);

				if (m_currentStartPos < 0)
				{
					refresh();
					continue;
				}

#ifndef ENABLE_MOVIEPLAYER2
				res = true;
				loop = false;
#else
				char startpos[32];
				sprintf(startpos, "%d", m_currentStartPos);
				std::string actionKey = "mb.file://";
				actionKey += getSelectedFile()->Name;
				actionKey += "?startpos=";
				actionKey += startpos;

				hide();
				CNeutrinoApp::getInstance()->exec(NULL, actionKey);
				paint();

				if (m_movieSelectionHandler != NULL)
					m_movieInfo.loadMovieInfo(m_movieSelectionHandler);

			 	m_prevBrowserSelection = m_currentBrowserSelection;
				m_prevRecordSelection = m_currentRecordSelection;
				m_prevPlaySelection = m_currentPlaySelection;

				refreshBrowserList();	
				refreshLastPlayList();	
				refreshLastRecordList();

			 	m_currentBrowserSelection = m_prevBrowserSelection;
				m_currentRecordSelection = m_prevRecordSelection;
				m_currentPlaySelection = m_prevPlaySelection;

				m_pcBrowser->setSelectedLine(m_currentBrowserSelection);
				m_pcLastRecord->setSelectedLine(m_currentRecordSelection);
				m_pcLastPlay->setSelectedLine(m_currentPlaySelection);

				refreshTitle();
				onSetGUIWindow(m_settings.gui);
#endif
			}
			else if (msg == CRCInput::RC_home)
			{
				loop = false;
			}
			else if (CNeutrinoApp::getInstance()->handleMsg(msg, data) & messages_return::cancel_all)
			{
				TRACE("[mb]->exec: getInstance\r\n");
				//res  = menu_return::RETURN_EXIT_ALL;
				loop = false;
			}
		}
		
		if ( msg <= CRCInput::RC_MaxRC )
			timeoutEnd = CRCInput::calcTimeoutEnd(timeout); // calcualate next timeout
	}
	hide();
	//TRACE(" return %d\r\n",res);
	
	// save current selection if movie has to be played or is not playing
	if (res == true || m_playstate == CMoviePlayerGui::STOPPED)
	{
	 	m_prevBrowserSelection = m_currentBrowserSelection;
		m_prevRecordSelection = m_currentRecordSelection;
		m_prevPlaySelection = m_currentPlaySelection;
	}

	saveSettings(&m_settings);	// might be better done in ~CMovieBrowser, but for any reason this does not work if MB is killed by neutrino shutdown
	
	// make stale if we should reload the next time, but not if movie has to be played or is playing
	if(m_vMovieInfo.empty() || (m_settings.reload == true && res == false && m_playstate == CMoviePlayerGui::STOPPED))
	{
		TRACE("[mb] force reload next time\r\n");
		fileInfoStale();
	}
		
	return (res);
}
/************************************************************************

************************************************************************/
bool CMovieBrowser::changeNotify(const neutrino_locale_t OptionName, void *)
{
	if (ARE_LOCALES_EQUAL(OptionName, LOCALE_MOVIEBROWSER_EDIT_SERIE))
	{
		m_seriename_stale = true;
	}
	return false;
}
/************************************************************************

************************************************************************/
void CMovieBrowser::hide(void)
{
	//TRACE("[mb]->Hide\r\n");

	if (m_pcFilter != NULL)
	{
		m_currentFilterSelection  = m_pcFilter->getSelectedLine();
		delete m_pcFilter;
		m_pcFilter = NULL;
	}
	if (m_pcBrowser != NULL)
	{
		m_currentBrowserSelection = m_pcBrowser->getSelectedLine();
		delete m_pcBrowser;
		m_pcBrowser = NULL;
	}
	if (m_pcLastPlay != NULL)
	{
		m_currentPlaySelection    = m_pcLastPlay->getSelectedLine();
		delete m_pcLastPlay;
		m_pcLastPlay = NULL;
	}
	if (m_pcLastRecord != NULL) 
	{
		m_currentRecordSelection  = m_pcLastRecord->getSelectedLine();
		delete m_pcLastRecord;
		m_pcLastRecord = NULL;
	}
	if (m_pcInfo != NULL) delete m_pcInfo;
	if (m_pcWindow != NULL) delete m_pcWindow;

	m_pcWindow = NULL;
	m_pcInfo = NULL;
	
}


//init needed fonts
void CMovieBrowser::initFonts(void)
{
	m_pcFontFoot  = g_Font[SNeutrinoSettings::FONT_TYPE_INFOBAR_SMALL];
	m_pcFontTitle = g_Font[SNeutrinoSettings::FONT_TYPE_MENU_TITLE];
}

/************************************************************************

************************************************************************/
int CMovieBrowser::paint(void)
{
	if (m_pcWindow != NULL)
	{
		TRACE("[mb]  return -> window already exists\r\n");
		return (false);
	}

	TRACE("[mb]->Paint\r\n");

	CLCD::getInstance()->setMode(CLCD::MODE_MENU_UTF8, g_Locale->getText(LOCALE_MOVIEBROWSER_HEAD));

	//Font* font = TEXT_FONT;
	Font* font = NULL;
	
	/* If the MB is called again, after changing the Fontsize without a restart of Neutrino,
	   you will get a segfault. Reason: init() is only done once at start of the MB, so
	   initialize the font-variables here again*/
	initFonts();

	// create new window
	m_pcWindow = new CFBWindow( m_cBoxFrame.iX,
								m_cBoxFrame.iY,
								m_cBoxFrame.iWidth,
								m_cBoxFrame.iHeight+10);

	m_pcBrowser = new CListFrame(&m_browserListLines,
								font,
								CListFrame::SCROLL | CListFrame::HEADER_LINE, 
								&m_cBoxFrameBrowserList);

	m_pcLastPlay = new CListFrame(&m_playListLines,
								font,
								CListFrame::SCROLL | CListFrame::HEADER_LINE | CListFrame::TITLE, 
								&m_cBoxFrameLastPlayList,
								g_Locale->getText(LOCALE_MOVIEBROWSER_HEAD_PLAYLIST),
								g_Font[SNeutrinoSettings::FONT_TYPE_EPG_INFO1]);

	m_pcLastRecord = new CListFrame(&m_recordListLines,
								font,
								CListFrame::SCROLL | CListFrame::HEADER_LINE | CListFrame::TITLE, 
								&m_cBoxFrameLastRecordList,
								g_Locale->getText(LOCALE_MOVIEBROWSER_HEAD_RECORDLIST),
								g_Font[SNeutrinoSettings::FONT_TYPE_EPG_INFO1]);

	m_pcFilter = new CListFrame(&m_FilterLines,
								font,
								CListFrame::SCROLL | CListFrame::TITLE, 
								&m_cBoxFrameFilter,
								g_Locale->getText(LOCALE_MOVIEBROWSER_HEAD_FILTER),
								g_Font[SNeutrinoSettings::FONT_TYPE_EPG_INFO1]);

	m_pcInfo = new CTextBox(" ",
							NULL,
							CTextBox::SCROLL,
							&m_cBoxFrameInfo);							

	if(  m_pcWindow == NULL || 
		m_pcBrowser == NULL || 
		m_pcLastPlay == NULL || 
		m_pcLastRecord == NULL || 
		m_pcInfo == NULL || 
		m_pcFilter == NULL)
	{
		TRACE("[mb] paint, ERROR: not enought memory to allocate windows");
		if (m_pcFilter != NULL)delete m_pcFilter;
		if (m_pcBrowser != NULL)delete m_pcBrowser;
		if (m_pcLastPlay != NULL) delete m_pcLastPlay;
		if (m_pcLastRecord != NULL)delete m_pcLastRecord;
		if (m_pcInfo != NULL) delete m_pcInfo;
		if (m_pcWindow != NULL) delete m_pcWindow;

		m_pcWindow = NULL;
		m_pcInfo = NULL;
		m_pcLastPlay = NULL;
		m_pcLastRecord = NULL;
		m_pcBrowser = NULL;
		m_pcFilter = NULL;

		return (false);
	}  
	//onSetGUIWindow(m_settings.gui);	
							
	//refreshTitle();
	//refreshFoot();
	refreshLCD();
	//refresh();
	return (true);
}

/************************************************************************

************************************************************************/
void CMovieBrowser::refresh(void)
{
	//TRACE("[mb]->refresh\r\n");
	
	if (m_pcBrowser != NULL && m_showBrowserFiles == true )
		 m_pcBrowser->refresh();
	if (m_pcLastPlay != NULL && m_showLastPlayFiles == true ) 
		m_pcLastPlay->refresh();
	if (m_pcLastRecord != NULL && m_showLastRecordFiles == true)
		 m_pcLastRecord->refresh();
	if (m_pcInfo != NULL && m_showMovieInfo == true) 
		m_pcInfo->refresh();
	if (m_pcFilter != NULL && m_showFilter == true) 
		m_pcFilter->refresh();
		
	refreshTitle();
	refreshFoot();
	refreshLCD();
}

/************************************************************************

************************************************************************/
std::string CMovieBrowser::getCurrentDir(void)
{
	return(m_selectedDir);
}

/************************************************************************

************************************************************************/
CFile* CMovieBrowser::getSelectedFile(void)
{
	//TRACE("[mb]->getSelectedFile: %s\r\n",m_movieSelectionHandler->file.Name.c_str());

	if(m_movieSelectionHandler != NULL)
		return(&m_movieSelectionHandler->file);
	else
		return(NULL);
}

/************************************************************************

************************************************************************/
void CMovieBrowser::refreshMovieInfo(void)
{
	//TRACE("[mb]->refreshMovieInfo \r\n");

	if (m_vMovieInfo.empty() || m_movieSelectionHandler == NULL)
	{
		// There is no selected element, clear info box
		std::string emptytext = " ";
		if (m_pcInfo)
			m_pcInfo->setText(&emptytext);
	}
	else
	{
		if (m_movieSelectionHandler->epgInfo2.empty())
			m_pcInfo->setText(&m_movieSelectionHandler->epgInfo1);
		else
			m_pcInfo->setText(&m_movieSelectionHandler->epgInfo2);
	}
}

/************************************************************************

************************************************************************/
void CMovieBrowser::refreshLCD(void)
{
	CLCD * lcd = CLCD::getInstance();
	if (m_vMovieInfo.empty() || m_movieSelectionHandler == NULL)
	{
		// There is no selected element, clear LCD
		lcd->showMenuText(0, " ", -1, true); // UTF-8
		lcd->showMenuText(1, " ", -1, true); // UTF-8
	}
	else
	{
		lcd->showMenuText(0, m_movieSelectionHandler->epgTitle.c_str(), -1, true); // UTF-8
		lcd->showMenuText(1, m_movieSelectionHandler->epgInfo1.c_str(), -1, true); // UTF-8
	} 
}

/************************************************************************

************************************************************************/
static bool sortByAlpha(const std::string & a, const std::string & b)
{
	return (strcasecmp(a.c_str(), b.c_str()) < 0);
}

void CMovieBrowser::refreshFilterList(void)
{
	TRACE("[mb]->refreshFilterList %d\r\n",m_settings.filter.item);
		
	std::string string_item;
	
	m_FilterLines.rows = 1;
	m_FilterLines.lineArray[0].clear();
	m_FilterLines.rowWidth[0] = 400;
	m_FilterLines.lineHeader[0]= "";

	if (m_vMovieInfo.empty()) 
	{
		if (m_pcFilter)
			m_pcFilter->setLines(&m_FilterLines);
		return; // exit here if nothing else is to do
	}

	if(m_settings.filter.item == MB_INFO_MAX_NUMBER)
	{
		// show Main List
		string_item = g_Locale->getText(LOCALE_MOVIEBROWSER_INFO_GENRE_MAJOR);
		m_FilterLines.lineArray[0].push_back(string_item);
		string_item = g_Locale->getText(LOCALE_MOVIEBROWSER_INFO_INFO1);
		m_FilterLines.lineArray[0].push_back(string_item);
		string_item = g_Locale->getText(LOCALE_MOVIEBROWSER_INFO_PATH);
		m_FilterLines.lineArray[0].push_back(string_item);
		string_item = g_Locale->getText(LOCALE_MOVIEBROWSER_INFO_SERIE);
		m_FilterLines.lineArray[0].push_back(string_item);
#ifdef MB_SEARCH_INFO2
		string_item = g_Locale->getText(LOCALE_MOVIEBROWSER_INFO_INFO2);
		m_FilterLines.lineArray[0].push_back(string_item);
#endif
	}
	else
	{
		string_item = g_Locale->getText(LOCALE_MENU_BACK);
		m_FilterLines.lineArray[0].push_back(string_item);

		if(m_settings.filter.item == MB_INFO_FILEPATH)
		{
			for(unsigned int i =0 ; i < m_dirNames.size() ;i++)
			{
				m_FilterLines.lineArray[0].push_back(m_dirNames[i]);
			}
		}
		else if(m_settings.filter.item == MB_INFO_INFO1)
		{
			for(unsigned int i = 0; i < m_vMovieInfo.size(); i++)
			{
				bool found = false;
				for(unsigned int t = 0; t < m_FilterLines.lineArray[0].size() && found == false; t++)
				{
					if(strcmp(m_FilterLines.lineArray[0][t].c_str(),m_vMovieInfo[i].epgInfo1.c_str()) == 0)
						found = true;
				}
				if(found == false)
					m_FilterLines.lineArray[0].push_back(m_vMovieInfo[i].epgInfo1);
			}
			sort(m_FilterLines.lineArray[0].begin() + 1, m_FilterLines.lineArray[0].end(), sortByAlpha);
		}
		else if(m_settings.filter.item == MB_INFO_INFO2)
		{
			string_item = "->Eingabe";
			m_FilterLines.lineArray[0].push_back(string_item);
		}
		else if(m_settings.filter.item == MB_INFO_MAJOR_GENRE)
		{
			for(int i = 0; i < GENRE_ALL_COUNT; i++)
			{
				string_item = g_Locale->getText(GENRE_ALL[i].value);
				m_FilterLines.lineArray[0].push_back(string_item);
			}
			sort(m_FilterLines.lineArray[0].begin() + 1, m_FilterLines.lineArray[0].end(), sortByAlpha);
		}
		else if(m_settings.filter.item == MB_INFO_SERIE)
		{
			updateSerienames();
			for(unsigned int i = 0; i < m_vHandleSerienames.size(); i++)
			{
				m_FilterLines.lineArray[0].push_back(m_vHandleSerienames[i]->serieName);
			}
		}
	}
	m_pcFilter->setLines(&m_FilterLines);
}

/************************************************************************

************************************************************************/
void CMovieBrowser::refreshLastPlayList(void) //P2
{
	//TRACE("[mb]->refreshlastPlayList \r\n");
	std::string string_item;

	// Initialise and clear list array
	m_playListLines.rows = m_settings.lastPlayRowNr;
	for(int row = 0 ;row < m_settings.lastPlayRowNr; row++)
	{
		m_playListLines.lineArray[row].clear();
		m_playListLines.rowWidth[row] = m_settings.lastPlayRowWidth[row];
		m_playListLines.lineHeader[row]= g_Locale->getText(m_localizedItemName[m_settings.lastPlayRow[row]]);
	}
	m_vHandlePlayList.clear();

	if (m_vMovieInfo.empty()) 
	{
		m_currentPlaySelection = 0;
		if (m_pcLastPlay)
			m_pcLastPlay->setLines(&m_playListLines);
		return; // exit here if nothing else is to do
	}

	MI_MOVIE_INFO* movie_handle;
	// prepare Browser list for sorting and filtering
	for(unsigned int file=0; file < m_vMovieInfo.size(); file++)
	{
		if(	/*isFiltered(m_vMovieInfo[file]) 	   == false &&*/
			isParentalLock(m_vMovieInfo[file]) == false) 
		{
			movie_handle = &(m_vMovieInfo[file]);
			m_vHandlePlayList.push_back(movie_handle);
		}
	}
	// sort the not filtered files
	onSortMovieInfoHandleList(m_vHandlePlayList,MB_INFO_PREVPLAYDATE,MB_DIRECTION_DOWN);

	for(unsigned int handle=0; handle < m_vHandlePlayList.size() && handle < (unsigned int )m_settings.lastPlayMaxItems ;handle++)
	{
		for(int row = 0; row < m_settings.lastPlayRowNr ;row++)
		{
			if ( getMovieInfoItem(*m_vHandlePlayList[handle], m_settings.lastPlayRow[row], &string_item) == false)
			{
				string_item = "n/a";
				if(m_settings.lastPlayRow[row] == MB_INFO_TITLE)
					getMovieInfoItem(*m_vHandlePlayList[handle], MB_INFO_FILENAME, &string_item);
			}
			m_playListLines.lineArray[row].push_back(string_item);
		}
	}
	m_pcLastPlay->setLines(&m_playListLines);

	m_currentPlaySelection = m_pcLastPlay->getSelectedLine();
	// update selected movie if browser is in the focus
	if (m_windowFocus == MB_FOCUS_LAST_PLAY)
	{
		updateMovieSelection();	
	}
}

/************************************************************************

************************************************************************/
void CMovieBrowser::refreshLastRecordList(void) //P2
{
	//TRACE("[mb]->refreshLastRecordList \r\n");
	std::string string_item;

	// Initialise and clear list array
	m_recordListLines.rows = m_settings.lastRecordRowNr;
	for(int row = 0 ;row < m_settings.lastRecordRowNr; row++)
	{
		m_recordListLines.lineArray[row].clear();
		m_recordListLines.rowWidth[row] = m_settings.lastRecordRowWidth[row];
		m_recordListLines.lineHeader[row]= g_Locale->getText(m_localizedItemName[m_settings.lastRecordRow[row]]);
	}
	m_vHandleRecordList.clear();

	if (m_vMovieInfo.empty()) 
	{
		m_currentRecordSelection = 0;
		if (m_pcLastRecord)
			m_pcLastRecord->setLines(&m_recordListLines);
		return; // exit here if nothing else is to do
	}

	MI_MOVIE_INFO* movie_handle;
	// prepare Browser list for sorting and filtering
	for(unsigned int file=0; file < m_vMovieInfo.size()  ;file++)
	{
		if(	/*isFiltered(m_vMovieInfo[file]) 	   == false &&*/
			isParentalLock(m_vMovieInfo[file]) == false) 
		{
			movie_handle = &(m_vMovieInfo[file]);
			m_vHandleRecordList.push_back(movie_handle);
		}
	}
	// sort the not filtered files
	onSortMovieInfoHandleList(m_vHandleRecordList,MB_INFO_RECORDDATE,MB_DIRECTION_DOWN);

	for(unsigned int handle=0; handle < m_vHandleRecordList.size() && handle < (unsigned int )m_settings.lastRecordMaxItems ;handle++)
	{
		for(int row = 0; row < m_settings.lastRecordRowNr ;row++)
		{
			if ( getMovieInfoItem(*m_vHandleRecordList[handle], m_settings.lastRecordRow[row], &string_item) == false)
			{
				string_item = "n/a";
				if(m_settings.lastRecordRow[row] == MB_INFO_TITLE)
					getMovieInfoItem(*m_vHandleRecordList[handle], MB_INFO_FILENAME, &string_item);
			}
			m_recordListLines.lineArray[row].push_back(string_item);
		}
	}

	m_pcLastRecord->setLines(&m_recordListLines);

	m_currentRecordSelection = m_pcLastRecord->getSelectedLine();
	// update selected movie if browser is in the focus
	if (m_windowFocus == MB_FOCUS_LAST_RECORD)
	{
		updateMovieSelection();	
	}
}

/************************************************************************

************************************************************************/
void CMovieBrowser::refreshBrowserList(void) //P1
{
	//TRACE("[mb]->refreshBrowserList \r\n");
	std::string string_item;

	// Initialise and clear list array
	m_browserListLines.rows = m_settings.browserRowNr;
	for(int row = 0; row < m_settings.browserRowNr; row++)
	{
		m_browserListLines.lineArray[row].clear();
		m_browserListLines.rowWidth[row] = m_settings.browserRowWidth[row];
		m_browserListLines.lineHeader[row]= g_Locale->getText(m_localizedItemName[m_settings.browserRowItem[row]]);
	}
	m_vHandleBrowserList.clear();
	
	if (m_vMovieInfo.empty()) 
	{
		m_currentBrowserSelection = 0;
		m_movieSelectionHandler = NULL;
		if (m_pcBrowser)
			m_pcBrowser->setLines(&m_browserListLines);
		return; // exit here if nothing else is to do
	}
	
	MI_MOVIE_INFO* movie_handle;
	// prepare Browser list for sorting and filtering
	for(unsigned int file=0; file < m_vMovieInfo.size(); file++)
	{
		if(	isFiltered(m_vMovieInfo[file]) 	   == false &&
			isParentalLock(m_vMovieInfo[file]) == false  &&
			(m_settings.browser_serie_mode == 0 || m_vMovieInfo[file].serieName.empty() || m_settings.filter.item == MB_INFO_SERIE) )
		{
			movie_handle = &(m_vMovieInfo[file]);
			m_vHandleBrowserList.push_back(movie_handle);
		}
	}
	// sort the not filtered files
	onSortMovieInfoHandleList(m_vHandleBrowserList,m_settings.sorting.item,MB_DIRECTION_AUTO);

	for(unsigned int handle=0; handle < m_vHandleBrowserList.size() ;handle++)
	{
		for(int row = 0; row < m_settings.browserRowNr ;row++)
		{
			if ( getMovieInfoItem(*m_vHandleBrowserList[handle], m_settings.browserRowItem[row], &string_item) == false)
			{
				string_item = "n/a";
				if(m_settings.browserRowItem[row] == MB_INFO_TITLE)
					getMovieInfoItem(*m_vHandleBrowserList[handle], MB_INFO_FILENAME, &string_item);
			}
			m_browserListLines.lineArray[row].push_back(string_item);
		}
	}
	m_pcBrowser->setLines(&m_browserListLines);

	m_currentBrowserSelection = m_pcBrowser->getSelectedLine();
	// update selected movie if browser is in the focus
	if (m_windowFocus == MB_FOCUS_BROWSER)
	{
		updateMovieSelection();	
	}
}

/************************************************************************

************************************************************************/
void CMovieBrowser::refreshBookmarkList(void) // P3
{
	//TRACE("[mb]->refreshBookmarkList \r\n");
}

/************************************************************************

************************************************************************/
void CMovieBrowser::refreshTitle(void) 
{
	if( m_pcWindow == NULL) return;
	//Paint Text Background
	//TRACE("[mb]->refreshTitle : %s\r\n",m_textTitle.c_str());
	m_pcWindow->paintBoxRel(	m_cBoxFrameTitleRel.iX , 
								m_cBoxFrameTitleRel.iY, 
								m_cBoxFrameTitleRel.iWidth, 
								m_cBoxFrameTitleRel.iHeight, 
								TITLE_BACKGROUND_COLOR, 
								RADIUS_MID,
								CORNER_TOP);
	
	m_pcWindow->paintIcon(NEUTRINO_ICON_EPGINFO, 
								m_cBoxFrameTitleRel.iX + 6, 
								m_cBoxFrameTitleRel.iY + m_cBoxFrameTitleRel.iHeight / 2 - m_iconHeightTitle / 2);
									
	m_pcWindow->RenderString(	m_pcFontTitle,
								m_cBoxFrameTitleRel.iX + 6 + m_iconWidthTitle + TEXT_BORDER_WIDTH,
								m_cBoxFrameTitleRel.iY + m_cBoxFrameTitleRel.iHeight + 2, 
								m_cBoxFrameTitleRel.iWidth - 6 - m_iconWidthTitle - TEXT_BORDER_WIDTH,
								m_textTitle.c_str(), 
								TITLE_FONT_COLOR, 
								0, 
								true); // UTF-8

	if(m_playstate == CMoviePlayerGui::STOPPED)
	{
		int iconw, iconh;
		CFrameBuffer::getInstance()->getIconSize(NEUTRINO_ICON_BUTTON_DBOX, &iconw, &iconh);
		m_pcWindow->paintIcon(NEUTRINO_ICON_BUTTON_DBOX, 
									m_cBoxFrameTitleRel.iX + m_cBoxFrameTitleRel.iWidth - iconw - 12, 
									m_cBoxFrameTitleRel.iY + m_cBoxFrameTitleRel.iHeight / 2 - iconh / 2);
	}
}

/************************************************************************

************************************************************************/
void CMovieBrowser::refreshFoot(void) 
{
	CFrameBuffer * framebuffer = CFrameBuffer::getInstance();
	//TRACE("[mb]->refreshButtonLine \r\n");
	int	color   = (CFBWindow::color_t)COL_INFOBAR_SHADOW;
	int	bgcolor = (CFBWindow::color_t)COL_INFOBAR_SHADOW_PLUS_0;
	int	c_rad_mid = RADIUS_MID;
	int	footheight = m_cBoxFrameFootRel.iHeight + 4;

	//button caption for green (filter)
	std::string filter_text = g_Locale->getText(LOCALE_MOVIEBROWSER_FOOT_FILTER);
	filter_text += " ";
	filter_text += m_settings.filter.optionString;
	if (filter_text.length() > 27)
		filter_text = filter_text.substr(0, 27) + "...";

	//button caption for red (sort)
	std::string sort_text = g_Locale->getText(LOCALE_MOVIEBROWSER_FOOT_SORT);
	sort_text += " ";
	sort_text += g_Locale->getText(m_localizedItemName[m_settings.sorting.item]);

	//button caption for ok (select/start)
	std::string ok_text;
	if (m_settings.gui == MB_GUI_FILTER && m_windowFocus == MB_FOCUS_FILTER)
		ok_text = g_Locale->getText(LOCALE_MOVIEBROWSER_FOOT_SELECT);
	else
		ok_text = g_Locale->getText(LOCALE_MOVIEBROWSER_FOOT_PLAY);

	// draw the background first
	m_pcWindow->paintBoxRel(	m_cBoxFrameFootRel.iX, 
								m_cBoxFrameFootRel.iY, 
								m_cBoxFrameFootRel.iWidth, 
								footheight,  
								(CFBWindow::color_t)bgcolor,
								c_rad_mid, 
								CORNER_BOTTOM); 

	int width = m_cBoxFrameFootRel.iWidth;
	int xoffset = 4;

	//red button
	int xpos1 = m_cBoxFrameFootRel.iX + xoffset;
	int width1 = g_Font[SNeutrinoSettings::FONT_TYPE_MENU]->getRenderWidth(sort_text, true) + xoffset; // UTF-8;

	//green button
	int xpos2 = xpos1 + width1 + xoffset;
	int width2 = g_Font[SNeutrinoSettings::FONT_TYPE_MENU]->getRenderWidth(filter_text, true) + xoffset; // UTF-8;

	//ok button should be painted centered, but not if captions of red and green buttons are to big
	int xpos3 = ((width1 + width2) < width/2) ? width/2 + xoffset : xpos2 + width2 + xoffset;
	int width3 = g_Font[SNeutrinoSettings::FONT_TYPE_MENU]->getRenderWidth(ok_text, true) + xoffset; // UTF-8;
	int xpos3_max = m_cBoxFrameFootRel.iX + width - width3;
	if (xpos3 > xpos3_max)
		xpos3 = xpos3_max;

	int foot_hmid = footheight/2;

	int ypos_buttontext = m_cBoxFrameFootRel.iY + m_cBoxFrameFootRel.iHeight + 2;

	// draw red (sort)
	int iconw = 0, iconh = 0;
	if (m_settings.gui != MB_GUI_LAST_PLAY && m_settings.gui != MB_GUI_LAST_RECORD)
	{
		framebuffer->getIconSize(NEUTRINO_ICON_BUTTON_RED, &iconw, &iconh);
		m_pcWindow->paintBoxRel(xpos1, m_cBoxFrameFootRel.iY, width1, footheight, (CFBWindow::color_t)bgcolor, c_rad_mid, CORNER_BOTTOM_LEFT);
		m_pcWindow->paintIcon(NEUTRINO_ICON_BUTTON_RED, xpos1, (m_cBoxFrameFootRel.iY + foot_hmid) - iconh/2);
		m_pcWindow->RenderString(m_pcFontFoot, xpos1 + iconw + xoffset, ypos_buttontext , width1, sort_text.c_str(), (CFBWindow::color_t)color, 0, true); // UTF-8
	}

	// draw green (filter)
	if (m_settings.gui != MB_GUI_LAST_PLAY && m_settings.gui != MB_GUI_LAST_RECORD)
	{
		framebuffer->getIconSize(NEUTRINO_ICON_BUTTON_GREEN, &iconw, &iconh);
		m_pcWindow->paintBoxRel(xpos2, m_cBoxFrameFootRel.iY, width2, footheight, (CFBWindow::color_t)bgcolor);
		m_pcWindow->paintIcon(NEUTRINO_ICON_BUTTON_GREEN, xpos2, (m_cBoxFrameFootRel.iY + foot_hmid) - iconh/2);
		m_pcWindow->RenderString(m_pcFontFoot, xpos2 + iconw + xoffset, ypos_buttontext , width2, filter_text.c_str(), (CFBWindow::color_t)color, 0, true); // UTF-8
	}

	// draw ok (select/start)
	framebuffer->getIconSize(NEUTRINO_ICON_BUTTON_OKAY, &iconw, &iconh);
	m_pcWindow->paintBoxRel(xpos3, m_cBoxFrameFootRel.iY, width3, footheight, (CFBWindow::color_t)bgcolor, c_rad_mid, CORNER_BOTTOM_RIGHT);
	m_pcWindow->paintIcon(NEUTRINO_ICON_BUTTON_OKAY, xpos3, (m_cBoxFrameFootRel.iY + foot_hmid) - iconh/2);
	m_pcWindow->RenderString(m_pcFontFoot, xpos3 + iconw + xoffset, ypos_buttontext , width3, ok_text.c_str(), (CFBWindow::color_t)color, 0, true); // UTF-8
}

/************************************************************************

************************************************************************/
bool CMovieBrowser::onButtonPress(neutrino_msg_t msg)
{
//	TRACE("[mb]->onButtonPress %d\r\n",msg);
	bool result = false;
	neutrino_msg_t msg_repeatok = msg & ~CRCInput::RC_Repeat;
	
	result = onButtonPressMainFrame(msg);
	if(result == false)
	{
		// if Main Frame didnot process the button, the focused window may do
		switch(m_windowFocus)
		{
			case MB_FOCUS_BROWSER:
			 	result = onButtonPressBrowserList(msg_repeatok);
				break;
			case MB_FOCUS_LAST_PLAY:
			 	result = onButtonPressLastPlayList(msg_repeatok);
				break;
			case MB_FOCUS_LAST_RECORD:
			 	result = onButtonPressLastRecordList(msg_repeatok);
				break;
			case MB_FOCUS_MOVIE_INFO:
			 	result = onButtonPressMovieInfoList(msg_repeatok);
				break;
			case MB_FOCUS_FILTER:
			 	result = onButtonPressFilterList(msg);		
				break;
			default:
				break;
		}
	}
	return (result);
}

/************************************************************************

************************************************************************/
bool CMovieBrowser::onButtonPressMainFrame(neutrino_msg_t msg)
{
	//TRACE("[mb]->onButtonPressMainFrame: %d\r\n",msg);
	bool result = true;

	if (msg == CRCInput::RC_home)
	{
		if(m_settings.gui == MB_GUI_FILTER)
			onSetGUIWindow(MB_GUI_MOVIE_INFO);
		else
			result = false;
	}
	else if (msg == CRCInput::RC_left) 
	{
		onSetGUIWindowPrev();
		//refreshMovieInfo();
	}
	else if (msg == CRCInput::RC_right) 
	{
		onSetGUIWindowNext();
		//refreshMovieInfo();
	}
	else if (msg == CRCInput::RC_green) 
	{
		if(m_settings.gui == MB_GUI_MOVIE_INFO)
			onSetGUIWindow(MB_GUI_FILTER);
		else if(m_settings.gui == MB_GUI_FILTER)
			onSetGUIWindow(MB_GUI_MOVIE_INFO);
		// no effect if gui is last play or record			
	}
	else if (msg == CRCInput::RC_yellow) 
	{
		onSetFocusNext();
	}
	else if (msg == CRCInput::RC_blue) 
	{
		if(m_playstate == CMoviePlayerGui::STOPPED)
		{
			loadMovies();
			refresh();
		}
	}
	else if (msg == CRCInput::RC_red ) 
	{
		if(m_settings.gui != MB_GUI_LAST_PLAY && m_settings.gui != MB_GUI_LAST_RECORD)
		{
			// sorting is not avialable for last play and record
			do
			{
				if(m_settings.sorting.item + 1 >= MB_INFO_MAX_NUMBER)
					m_settings.sorting.item = (MB_INFO_ITEM)0;
				else
					m_settings.sorting.item = (MB_INFO_ITEM)(m_settings.sorting.item + 1);
			}while(sortBy[m_settings.sorting.item] == NULL);
					
			TRACE("[mb]->new sorting %d,%s\r\n",m_settings.sorting.item,g_Locale->getText(m_localizedItemName[m_settings.sorting.item]));
			refreshBrowserList();	
			refreshFoot();
		}
	}
	else if (msg == CRCInput::RC_spkr) 
	{
		if (!m_vMovieInfo.empty())
		{	
			if(m_movieSelectionHandler != NULL)
			{
			 	onDeleteFile(*m_movieSelectionHandler);
			}
		}
	}
	else if (msg == CRCInput::RC_help) 
	{
		if(m_movieSelectionHandler != NULL)
		{
			m_movieInfo.showMovieInfo(*m_movieSelectionHandler);
			//m_movieInfo.printDebugMovieInfo(*m_movieSelectionHandler);
		}
	}
	else if (msg == CRCInput::RC_setup) 
	{
		if(m_playstate == CMoviePlayerGui::STOPPED && m_movieSelectionHandler != NULL)
			showMenu(m_movieSelectionHandler);
	}
#ifdef MOVEMANAGER	
	else if (msg == CRCInput::RC_1) 
	{
		std::string source;
		static std::string dest =	"/hdd/Filme";
		source = m_movieSelectionHandler->file.Name;
		CDirChooser dir(&dest,"/mnt/","/hdd");
		dir.exec(NULL,"");   
		if(!dest.empty())
		{
				dest += "/";
				dest += m_movieSelectionHandler->file.getFileName();
				CMoveManager::getInstance()->newMove(dest,source,MOVE_TYPE_COPY);
		
				if(m_movieInfo.convertTs2XmlName(&source) == true)  
					if(m_movieInfo.convertTs2XmlName(&dest) == true)  
						CMoveManager::getInstance()->newMove(dest,source,MOVE_TYPE_COPY);
		}
		refresh();
	}
	else if (msg == CRCInput::RC_0) 
	{
		std::string source;
		std::string dest =	"/hdd/Filme";
		source = m_movieSelectionHandler->file.Name;
		CDirChooser dir(&dest,"/mnt/","/hdd");
		dir.exec(NULL,"");   
		if(!dest.empty())
		{
				dest += "/";
				dest += m_movieSelectionHandler->file.getFileName();
				CMoveManager::getInstance()->newMove(dest,source,MOVE_STATE_MOVE);
		
				if(m_movieInfo.convertTs2XmlName(&source) == true)  
					if(m_movieInfo.convertTs2XmlName(&dest) == true)  
						CMoveManager::getInstance()->newMove(dest,source,MOVE_STATE_MOVE);
		}
		refresh();
	}
#endif //MOVEMANAGER	
	else
	{
		//TRACE("[mb]->onButtonPressMainFrame none\r\n");
		result = false;
	}

	return (result);
}
/************************************************************************

************************************************************************/
bool CMovieBrowser::onButtonPressBrowserList(neutrino_msg_t msg) 
{
	//TRACE("[mb]->onButtonPressBrowserList %d\r\n",msg);
	bool result = true;
	
	if(msg==CRCInput::RC_up)
	{
		m_pcBrowser->scrollLineUp();
	}
	else if (msg == CRCInput::RC_down)
	{
		m_pcBrowser->scrollLineDown();
	}
	else if (msg == g_settings.key_channelList_pageup)
	{
		m_pcBrowser->scrollPageUp();
	}
	else if (msg == g_settings.key_channelList_pagedown)
	{
		m_pcBrowser->scrollPageDown();
	}
	else
	{
		// default
		result = false;
	}
	if(result == true)
		updateMovieSelection();

	return (result);
}

/************************************************************************

************************************************************************/
bool CMovieBrowser::onButtonPressLastPlayList(neutrino_msg_t msg) 
{
	//TRACE("[mb]->onButtonPressLastPlayList %d\r\n",msg);
	bool result = true;
	
	if(msg==CRCInput::RC_up)
	{
		m_pcLastPlay->scrollLineUp();
	}
	else if (msg == CRCInput::RC_down)
	{
		m_pcLastPlay->scrollLineDown();
	}
	else if (msg == g_settings.key_channelList_pageup)
	{
		m_pcLastPlay->scrollPageUp();
	}
	else if (msg == g_settings.key_channelList_pagedown)
	{
		m_pcLastPlay->scrollPageDown();
	}
	else
	{
		// default
		result = false;
	}
	if(result == true)
		updateMovieSelection();

	return (result);
}

/************************************************************************

************************************************************************/
bool CMovieBrowser::onButtonPressLastRecordList(neutrino_msg_t msg) 
{
	//TRACE("[mb]->onButtonPressLastRecordList %d\r\n",msg);
	bool result = true;
	
	if(msg==CRCInput::RC_up)
	{
		m_pcLastRecord->scrollLineUp();
	}
	else if (msg == CRCInput::RC_down)
	{
		m_pcLastRecord->scrollLineDown();
	}
	else if (msg == g_settings.key_channelList_pageup)
	{
		m_pcLastRecord->scrollPageUp();
	}
	else if (msg == g_settings.key_channelList_pagedown)
	{
		m_pcLastRecord->scrollPageDown();
	}
	else
	{
		// default
		result = false;
	}

	if(result == true)
		updateMovieSelection();

	return (result);
}

/************************************************************************

************************************************************************/
bool CMovieBrowser::onButtonPressBookmarkList(neutrino_msg_t /*msg*/) 
{
	//TRACE("[mb]->onButtonPressBookmarkList %d\r\n",msg);
	bool result = true;
	
	result = false;
	return (result);
}
/************************************************************************

************************************************************************/
bool CMovieBrowser::onButtonPressFilterList(neutrino_msg_t msg) 
{
	//TRACE("[mb]->onButtonPressFilterList %d,%d\r\n",msg,m_settings.filter.item);
	bool result = true;
	neutrino_msg_t msg_repeatok = msg & ~CRCInput::RC_Repeat;

	if (msg_repeatok == CRCInput::RC_up)
	{
		m_pcFilter->scrollLineUp();
	}
	else if (msg_repeatok == CRCInput::RC_down)
	{
		m_pcFilter->scrollLineDown();
	}
	else if (msg_repeatok == g_settings.key_channelList_pageup)
	{
		m_pcFilter->scrollPageUp();
	}
	else if (msg_repeatok == g_settings.key_channelList_pagedown)
	{
		m_pcFilter->scrollPageDown();
	}
	else if (msg == CRCInput::RC_ok)
	{
		int selected_line = m_pcFilter->getSelectedLine();
		if(m_settings.filter.item == MB_INFO_MAX_NUMBER)
		{
			if(selected_line == 0) m_settings.filter.item = MB_INFO_MAJOR_GENRE;
			if(selected_line == 1) m_settings.filter.item = MB_INFO_INFO1;
			if(selected_line == 4) m_settings.filter.item = MB_INFO_INFO2;
			if(selected_line == 2) m_settings.filter.item = MB_INFO_FILEPATH;
			if(selected_line == 3) m_settings.filter.item = MB_INFO_SERIE;
			refreshFilterList();
			m_pcFilter->setSelectedLine(0);
		}
		else
		{
			if(selected_line == 0)
			{
				m_settings.filter.item = MB_INFO_MAX_NUMBER;
				m_settings.filter.optionString = "---";
				m_settings.filter.optionVar = 0;
				refreshFilterList();
				m_pcFilter->setSelectedLine(0);
				refreshBrowserList();	
				refreshLastPlayList();	
				refreshLastRecordList();	
				refreshFoot();
			}
			else
			{
				updateFilterSelection();
			}
		}
	}
	else
	{
		// default
		result = false;
	}
	
	return (result);
}

/************************************************************************

************************************************************************/
bool CMovieBrowser::onButtonPressMovieInfoList(neutrino_msg_t msg) 
{
//	TRACE("[mb]->onButtonPressEPGInfoList %d\r\n",msg);
	bool result = true;
	
	if(msg == CRCInput::RC_up)
	{
		m_pcInfo->scrollPageUp();
	}
	else if (msg == CRCInput::RC_down)
	{
		m_pcInfo->scrollPageDown();
	}
	else
	{
		// default
		result = false;
	}	

	return (result);
}
/************************************************************************

************************************************************************/
void CMovieBrowser::onDeleteFile(MI_MOVIE_INFO& movieSelectionHandler)
{
	//TRACE( "[onDeleteFile] ");
	int test = movieSelectionHandler.file.Name.rfind(".ts");
	if (test == -1 || test != (int)movieSelectionHandler.file.Name.length() - 3) 
	{ 
		// not a TS file, return!!!!! 
		TRACE( "show_ts_info: not a TS file ");
	}
	else
	{
		std::string msg = g_Locale->getText(LOCALE_FILEBROWSER_DODELETE1);
		msg += "\r\n ";
		if (movieSelectionHandler.file.Name.length() > 40)
		{
			msg += movieSelectionHandler.file.Name.substr(0,40);
			msg += "...";
		}
		else
			msg += movieSelectionHandler.file.Name;
			
		msg += "\r\n ";
		msg += g_Locale->getText(LOCALE_FILEBROWSER_DODELETE2);
		if (ShowMsgUTF(LOCALE_FILEBROWSER_DELETE, msg, CMessageBox::mbrNo, CMessageBox::mbYes|CMessageBox::mbNo)==CMessageBox::mbrYes)
		{
			CHintBox hintBox(LOCALE_MESSAGEBOX_INFO, g_Locale->getText(LOCALE_MOVIEBROWSER_DELETE_INFO));
			hintBox.paint();

			CFile file_xml  = movieSelectionHandler.file; 
			if(m_movieInfo.convertTs2XmlName(&file_xml.Name) == true)  
			{
				delFile(file_xml);
			}

			delFile(movieSelectionHandler.file);

			hintBox.hide();
			g_RCInput->clearRCMsg();

			if (!m_vHandleSerienames.empty())
				m_seriename_stale = true;
			m_vMovieInfo.erase( (std::vector<MI_MOVIE_INFO>::iterator)&movieSelectionHandler);
			updateSerienames();
			refreshBrowserList();
			refreshLastPlayList();	
			refreshLastRecordList();	
			refreshMovieInfo();
	
			refresh();
		}
	} 
}

/************************************************************************

************************************************************************/
void CMovieBrowser::onSetGUIWindow(MB_GUI gui)
{
	m_settings.gui = gui;
	
	if(gui == MB_GUI_MOVIE_INFO)
	{
		TRACE("[mb] browser info\r\n");
		// Paint these frames ...

		m_showMovieInfo = true;
		m_showBrowserFiles = true;

		// ... and hide these frames
		m_showLastRecordFiles = false;
		m_showLastPlayFiles = false;
		m_showFilter = false;

		m_pcLastPlay->hide();
		m_pcLastRecord->hide();
		m_pcFilter->hide();
		m_pcBrowser->paint();
		onSetFocus(MB_FOCUS_BROWSER);
		refreshMovieInfo();
		m_pcInfo->paint();
	}
	else if(gui == MB_GUI_LAST_PLAY)
	{
		TRACE("[mb] last play \r\n");
		// Paint these frames ...
		m_showLastRecordFiles = true;
		m_showLastPlayFiles = true;
		m_showMovieInfo = true;

		// ... and hide these frames
		m_showBrowserFiles = false;
		m_showFilter = false;

		m_pcBrowser->hide();
		m_pcFilter->hide();
		m_pcLastRecord->paint();
		m_pcLastPlay->paint();

		onSetFocus(MB_FOCUS_LAST_PLAY);
		refreshMovieInfo();
		m_pcInfo->paint();
	}
	else if(gui == MB_GUI_LAST_RECORD)
	{
		TRACE("[mb] last record \r\n");
		// Paint these frames ...
		m_showLastRecordFiles = true;
		m_showLastPlayFiles = true;
		m_showMovieInfo = true;

		// ... and hide these frames
		m_showBrowserFiles = false;
		m_showFilter = false;

		m_pcBrowser->hide();
		m_pcFilter->hide();
		m_pcLastRecord->paint();
		m_pcLastPlay->paint();

		onSetFocus(MB_FOCUS_LAST_RECORD);
		refreshMovieInfo();
		m_pcInfo->paint();
	}
 	else if(gui == MB_GUI_FILTER)
	{
		TRACE("[mb] filter \r\n");
		// Paint these frames ...
		m_showFilter = true;
		// ... and hide these frames
		m_showMovieInfo = false;
		m_pcInfo->hide();
		m_pcFilter->paint();
		
		onSetFocus(MB_FOCUS_FILTER);
	}
}

/************************************************************************

************************************************************************/
void CMovieBrowser::onSetGUIWindowNext(void) 
{
	if(m_settings.gui == MB_GUI_MOVIE_INFO )
	{
		onSetGUIWindow(MB_GUI_LAST_PLAY);
	}
	else if(m_settings.gui == MB_GUI_LAST_PLAY)
	{
		onSetGUIWindow(MB_GUI_LAST_RECORD);
	}
	else
	{
		onSetGUIWindow(MB_GUI_MOVIE_INFO);
	}
}
/************************************************************************

************************************************************************/
void CMovieBrowser::onSetGUIWindowPrev(void) 
{
	if(m_settings.gui == MB_GUI_MOVIE_INFO )
	{
		onSetGUIWindow(MB_GUI_LAST_RECORD);
	}
	else if(m_settings.gui == MB_GUI_LAST_RECORD)
	{
		onSetGUIWindow(MB_GUI_LAST_PLAY);
	}
	else
	{
		onSetGUIWindow(MB_GUI_MOVIE_INFO);
	}
}

/************************************************************************

************************************************************************/
void CMovieBrowser::onSetFocus(MB_FOCUS new_focus)
{
	//TRACE("[mb]->onSetFocus %d \r\n",new_focus);
	m_windowFocus = new_focus;
	if(m_windowFocus == MB_FOCUS_BROWSER)
	{
			m_pcBrowser->showSelection(true);
			m_pcLastRecord->showSelection(false);
			m_pcLastPlay->showSelection(false);
			m_pcFilter->showSelection(false);
			//m_pcInfo->showSelection(false);
	}
	else if(m_windowFocus == MB_FOCUS_LAST_PLAY)
	{
			m_pcBrowser->showSelection(false);
			m_pcLastRecord->showSelection(false);
			m_pcLastPlay->showSelection(true);
			m_pcFilter->showSelection(false);
			//m_pcInfo->showSelection(false);
	}
	else if(m_windowFocus == MB_FOCUS_LAST_RECORD)
	{
			m_pcBrowser->showSelection(false);
			m_pcLastRecord->showSelection(true);
			m_pcLastPlay->showSelection(false);
			m_pcFilter->showSelection(false);
			//m_pcInfo->showSelection(false);
	}
	else if(m_windowFocus == MB_FOCUS_MOVIE_INFO)
	{
			m_pcBrowser->showSelection(false);
			m_pcLastRecord->showSelection(false);
			m_pcLastPlay->showSelection(false);
			m_pcFilter->showSelection(false);
			//m_pcInfo->showSelection(true);
	}
	else if(m_windowFocus == MB_FOCUS_FILTER)
	{
			m_pcBrowser->showSelection(false);
			m_pcLastRecord->showSelection(false);
			m_pcLastPlay->showSelection(false);
			m_pcFilter->showSelection(true);
			//m_pcInfo->showSelection(false);
	}
	updateMovieSelection();
	refreshFoot();
}
/************************************************************************

************************************************************************/
void CMovieBrowser::onSetFocusNext(void) 
{
	//TRACE("[mb]->onSetFocusNext \r\n");
	
	if(m_settings.gui == MB_GUI_FILTER)
	{
		if(m_windowFocus == MB_FOCUS_BROWSER)
		{
			TRACE("[mb] MB_FOCUS_FILTER\r\n");
			onSetFocus(MB_FOCUS_FILTER);
		}
		else
		{
			TRACE("[mb] MB_FOCUS_BROWSER\r\n");
			onSetFocus(MB_FOCUS_BROWSER);
		}
	}
	else if(m_settings.gui == MB_GUI_MOVIE_INFO)
	{
		if(m_windowFocus == MB_FOCUS_BROWSER)
		{
			TRACE("[mb] MB_FOCUS_MOVIE_INFO\r\n");
			onSetFocus(MB_FOCUS_MOVIE_INFO);
			m_windowFocus = MB_FOCUS_MOVIE_INFO;
		}
		else
		{
			TRACE("[mb] MB_FOCUS_BROWSER\r\n");
			onSetFocus(MB_FOCUS_BROWSER);
		}
	}
	else if(m_settings.gui == MB_GUI_LAST_PLAY)
	{
		if(m_windowFocus == MB_FOCUS_MOVIE_INFO)
		{
			onSetFocus(MB_FOCUS_LAST_PLAY);
		}
		else if(m_windowFocus == MB_FOCUS_LAST_PLAY)
		{
			onSetFocus(MB_FOCUS_MOVIE_INFO);
		}
	}
	else if(m_settings.gui == MB_GUI_LAST_RECORD)
	{
		if(m_windowFocus == MB_FOCUS_MOVIE_INFO)
		{
			onSetFocus(MB_FOCUS_LAST_RECORD);
		}
		else if(m_windowFocus == MB_FOCUS_LAST_RECORD)
		{
			onSetFocus(MB_FOCUS_MOVIE_INFO);
		}
	}
}

/************************************************************************

************************************************************************/
bool CMovieBrowser::onSortMovieInfoHandleList(std::vector<MI_MOVIE_INFO*>& handle_list, MB_INFO_ITEM sort_item, MB_DIRECTION direction)
{
	//TRACE("sort: %d\r\n",direction);
	if (handle_list.empty()) 
		return (false); // nothing to sort, return immedately
	if(sortBy[sort_item] == NULL) 
		return (false);
	
	if(direction == MB_DIRECTION_AUTO)
	{
		if( sort_item == MB_INFO_QUALITY || 
			sort_item == MB_INFO_PARENTAL_LOCKAGE || 
			sort_item == MB_INFO_PREVPLAYDATE || 
			sort_item == MB_INFO_RECORDDATE || 
			sort_item == MB_INFO_PRODDATE ||
			sort_item == MB_INFO_SIZE)
		{
			sortDirection = 1;
		}
		else
		{
			sortDirection = 0;
		}
	}
	else if(direction == MB_DIRECTION_UP)
	{
		sortDirection = 0;
	}
	else
	{
		sortDirection = 1;
	}
	
	//TRACE("sort: %d\r\n",sortDirection);
	sort(handle_list.begin(), handle_list.end(), sortBy[sort_item]);
	
	return (true);
}

void CMovieBrowser::updateDir(void)
{
	m_dir.clear();
	// check if there is a movie dir and if we should use it
	if(g_settings.streaming_moviedir[0] != 0 )
	{
		std::string name = g_settings.streaming_moviedir;
		addDir(name,&m_settings.storageDirMovieUsed);
	}
	// check if there is a record dir and if we should use it
	for(int i = 0; i < MAX_RECORDING_DIR; i++)
	{
		if(!g_settings.recording_dir[i].empty())
			addDir(g_settings.recording_dir[i],&m_settings.storageDirRecUsed[i]);
	}
	
	for(int i = 0; i < MB_MAX_DIRS; i++)
	{
		if(!m_settings.storageDir[i].empty())
			addDir(m_settings.storageDir[i],&m_settings.storageDirUsed[i]);
	}
}
/************************************************************************

************************************************************************/
void CMovieBrowser::loadAllTsFileNamesFromStorage(void)
{
	//TRACE("[mb]->loadAllTsFileNamesFromStorage \r\n");
	bool result;
	int i,size; 
	
	m_movieSelectionHandler = NULL;
	m_dirNames.clear();
	m_vMovieInfo.clear();
	
	updateDir();

	size = m_dir.size();
	for(i=0; i < size;i++)
	{
		if(*m_dir[i].used == true )
			result = loadTsFileNamesFromDir(m_dir[i].name);
	}
	
	TRACE("[mb] Dir%d, Files:%d \r\n",m_dirNames.size(),m_vMovieInfo.size());
	if (m_vMovieInfo.empty())
	{
		std::string msg = g_Locale->getText(LOCALE_MOVIEBROWSER_ERROR_NO_MOVIES);
		DisplayErrorMessage(msg.c_str());
	}
}

/************************************************************************
Note: this function is used recursive, do not add any return within the body due to the recursive counter
************************************************************************/
bool CMovieBrowser::loadTsFileNamesFromDir(const std::string & dirname)
{
	//TRACE("[mb]->loadTsFileNamesFromDir %s\r\n",dirname.c_str());

	static int recursive_counter = 0; // recursive counter to be used to avoid hanging
	bool result = false;
	int file_found_in_dir = false;

	if (recursive_counter > 10)
	{
		TRACE("[mb]loadTsFileNamesFromDir: return->recoursive error\r\n"); 
		return (false); // do not go deeper than 10 directories
	}

	/* check if directory was already searched once */
	int size = m_dirNames.size();
	for(int i = 0; i < size; i++)
	{
		if(strcmp(m_dirNames[i].c_str(),dirname.c_str()) == 0)	
		{
			// string is identical to previous one
			TRACE("[mb]Dir already in list: %s\r\n",dirname.c_str()); 
			return (false); 
		}
	}
	/* !!!!!! no return statement within the body after here !!!!*/
	recursive_counter++;

	CFileList flist;
	if(readDir(dirname, &flist) == true)
	{
		MI_MOVIE_INFO movieInfo;
		m_movieInfo.clearMovieInfo(&movieInfo); // refresh structure
		for(unsigned int i = 0; i < flist.size(); i++)
		{
			if (flist[i].isDir())
			{
				flist[i].Name += '/';
				//TRACE("[mb] Dir: '%s'\r\n",movieInfo.file.Name.c_str());
				loadTsFileNamesFromDir(flist[i].Name);
			}
			else
			{
				int test = flist[i].getFileName().rfind(".ts");
				if (test == -1 || test != (int)flist[i].getFileName().length() - 3)
				{
					//TRACE("[mb] other file: '%s'\r\n",movieInfo.file.Name.c_str());
				}
				else
				{
					movieInfo.file.Name = flist[i].Name;
					movieInfo.file.Mode = flist[i].Mode;
					movieInfo.file.Size = flist[i].Size;
					movieInfo.file.Time = flist[i].Time;
					//TRACE(" N:%s,s:%d,t:%d\r\n",movieInfo.file.getFileName().c_str(),movieInfo.file.Size,movieInfo.file.Time);
					//TRACE(" N:%s,s:%d\r\n",movieInfo.file.getFileName().c_str(),movieInfo.file.Size>>20);
					//TRACE(" s:%d\r\n",movieInfo.file.getFileName().c_str(),movieInfo.file.Size);
					//TRACE(" s:%llu\r\n",movieInfo.file.getFileName().c_str(),movieInfo.file.Size);
					if(file_found_in_dir == false)
					{
						// first file in directory found, add directory to list 
						m_dirNames.push_back(dirname);
						file_found_in_dir = true;
						//TRACE("[mb] new dir: :%s\r\n",dirname);
					}
					movieInfo.dirItNr = m_dirNames.size()-1;
					m_vMovieInfo.push_back(movieInfo);
				}
			}
		}
		result = true;
	}	
 	
	recursive_counter--;
	return (result);
}

/************************************************************************

************************************************************************/
bool CMovieBrowser::readDir(const std::string & dirname, CFileList* flist)
{
	bool result = false;
	if (strncmp(dirname.c_str(), VLC_URI, strlen(VLC_URI)) == 0)
	{
		result = readDir_vlc(dirname, flist);
	}
	else
	{
		result = readDir_std(dirname, flist);
	}
	return(result);
}
/************************************************************************

************************************************************************/
bool CMovieBrowser::readDir_vlc(const std::string & /*dirname*/, CFileList* /*flist*/)
{
	return false;
}
/************************************************************************

************************************************************************/
bool CMovieBrowser::readDir_std(const std::string & dirname, CFileList* flist)
{
	bool result = true;
	//TRACE("readDir_std %s\n",dirname.c_str());
	stat_struct statbuf;
	dirent_struct **namelist;
	int n;

	n = my_scandir(dirname.c_str(), &namelist, 0, my_alphasort);
	if (n < 0)
	{
		perror(("[mb] scandir: "+dirname).c_str());
		return false;
	}
	CFile file;
	for(int i = 0; i < n;i++)
	{
		if(namelist[i]->d_name[0] != '.')
		{
			file.Name = dirname;
			file.Name += namelist[i]->d_name;

//			printf("file.Name: '%s', getFileName: '%s' getPath: '%s'\n",file.Name.c_str(),file.getFileName().c_str(),file.getPath().c_str());
			if(my_stat((file.Name).c_str(),&statbuf) != 0)
				perror("stat error");
			else
			{
				file.Mode = statbuf.st_mode;
				file.Time = statbuf.st_mtime;
				file.Size = statbuf.st_size;
				flist->push_back(file);
			}
		}
		free(namelist[i]);
	}

	free(namelist);

	return(result);
}

/************************************************************************

************************************************************************/
bool CMovieBrowser::delFile(CFile& file)
{
	bool result = false;
	//only std supported yet
	if(1)
	{
		result = delFile_std(file);
	}
#ifdef ENABLE_MOVIEPLAYER_VLC
	else
	{
		result = delFile_vlc(file);
	}
#endif
	return(result);
}
/************************************************************************

************************************************************************/
#ifdef ENABLE_MOVIEPLAYER_VLC
bool CMovieBrowser::delFile_vlc(CFile& /*file*/)
{
	bool result = false;
	return(result);
}
#endif
/************************************************************************

************************************************************************/
bool CMovieBrowser::delFile_std(CFile& file)
{
	bool result = true;
	unlink(file.Name.c_str()); // fix: use full path
	TRACE("  delete file: %s\r\n",file.Name.c_str());
	return(result);
}
/************************************************************************

************************************************************************/
void CMovieBrowser::updateMovieSelection(void)
{
	//TRACE("[mb]->updateMovieSelection %d\r\n",m_windowFocus);
	if (m_vMovieInfo.empty()) return;
	bool new_selection = false;
	 
	unsigned int old_movie_selection;
	if(m_windowFocus == MB_FOCUS_BROWSER)
	{
		if (m_vHandleBrowserList.empty())
		{
			// There are no elements in the Filebrowser, clear all handles
			m_currentBrowserSelection = 0;
			m_movieSelectionHandler = NULL;
			new_selection = true;
		}
		else
		{
			old_movie_selection = m_currentBrowserSelection;
			m_currentBrowserSelection = m_pcBrowser->getSelectedLine();
			//TRACE("	sel1:%d\r\n",m_currentBrowserSelection);
			if(m_currentBrowserSelection != old_movie_selection)
				new_selection = true;
			
			if(m_currentBrowserSelection < m_vHandleBrowserList.size())
				m_movieSelectionHandler = m_vHandleBrowserList[m_currentBrowserSelection];
		}
	}
	else if(m_windowFocus == MB_FOCUS_LAST_PLAY)
	{
		if (m_vHandlePlayList.empty())
		{
			// There are no elements in the Filebrowser, clear all handles
			m_currentPlaySelection = 0;
			m_movieSelectionHandler = NULL;
			new_selection = true;
		}
		else
		{
			old_movie_selection = m_currentPlaySelection;
			m_currentPlaySelection = m_pcLastPlay->getSelectedLine();
			//TRACE("    sel2:%d\r\n",m_currentPlaySelection);
			if(m_currentPlaySelection != old_movie_selection)
				new_selection = true;
	
			 if(m_currentPlaySelection < m_vHandlePlayList.size())
				m_movieSelectionHandler = m_vHandlePlayList[m_currentPlaySelection];
		}
	}
	else if(m_windowFocus == MB_FOCUS_LAST_RECORD)
	{
		if (m_vHandleRecordList.empty())
		{
			// There are no elements in the Filebrowser, clear all handles
			m_currentRecordSelection = 0;
			m_movieSelectionHandler = NULL;
			new_selection = true;
		}
		else
		{
			old_movie_selection = m_currentRecordSelection;
			m_currentRecordSelection = m_pcLastRecord->getSelectedLine();
			//TRACE("    sel3:%d\r\n",m_currentRecordSelection);
			if(m_currentRecordSelection != old_movie_selection)
				new_selection = true;
	
			if(m_currentRecordSelection < m_vHandleRecordList.size())
				m_movieSelectionHandler = m_vHandleRecordList[m_currentRecordSelection];
		}
	}	
	
	if(new_selection == true)
	{
		//TRACE("new\r\n");
		refreshMovieInfo();
		refreshLCD();
	}
	//TRACE("\r\n");
}

/************************************************************************

************************************************************************/
void CMovieBrowser::updateFilterSelection(void)
{
	//TRACE("[mb]->updateFilterSelection \r\n");
	if (m_FilterLines.lineArray[0].empty()) return;

	bool result = true;
	int selected_line = m_pcFilter->getSelectedLine();
	if(selected_line > 0)
		selected_line--;

	if(m_settings.filter.item == MB_INFO_FILEPATH)
	{
		m_settings.filter.optionString = m_FilterLines.lineArray[0][selected_line+1];
		m_settings.filter.optionVar = selected_line;
	}
	else if(m_settings.filter.item == MB_INFO_INFO1)
	{
		m_settings.filter.optionString = m_FilterLines.lineArray[0][selected_line+1];
	}
	else if(m_settings.filter.item == MB_INFO_INFO2)
	{
		std::string text;
		CStringInputSMS stringInputSMS(LOCALE_MOVIEBROWSER_INFO_INFO2, &text, 20, true, NONEXISTANT_LOCALE, NONEXISTANT_LOCALE, "abcdefghijklmnopqrstuvwxyz\xE4\xF6\xFC\xDF""0123456789-.: ");
		stringInputSMS.exec(NULL,"");
		m_settings.filter.optionString = text;
		refreshFilterList(); 
		refreshTitle();   
	}
	else if(m_settings.filter.item == MB_INFO_MAJOR_GENRE)
	{
		m_settings.filter.optionString = m_FilterLines.lineArray[0][selected_line+1];
		for(int i = 0; i < GENRE_ALL_COUNT; i++)
		{
			if(strcmp(m_settings.filter.optionString.c_str(), g_Locale->getText(GENRE_ALL[i].value)) == 0)
			{
				m_settings.filter.optionVar = GENRE_ALL[i].key;
				break;
			}
		}
	}
	else if(m_settings.filter.item == MB_INFO_SERIE)
	{
		m_settings.filter.optionString = m_FilterLines.lineArray[0][selected_line+1];
	}
	else
	{
		result = false;
	}
	if(result == true)
	{
		refreshBrowserList();	
		refreshLastPlayList();	
		refreshLastRecordList();	
		refreshFoot();
	}
}

/************************************************************************

************************************************************************/

bool CMovieBrowser::addDir(std::string& dirname, int* used)
{
	if(dirname.empty()) return false;
	if(dirname == "/") return false;
	
	MB_DIR newdir;
	newdir.name = dirname;

	if(newdir.name.rfind('/') != newdir.name.length()-1 ||
		newdir.name.empty() ||
		newdir.name == VLC_URI)
	{
		newdir.name += '/';
	}

	int size = m_dir.size();
	for(int i = 0; i < size; i++)
	{
		if(strcmp(m_dir[i].name.c_str(),newdir.name.c_str()) == 0)
		{
			// string is identical to previous one
			TRACE("[mb] Dir already in list: %s\r\n",newdir.name.c_str());
			return (false); 
		}
	}
	TRACE("[mb] new Dir: %s\r\n",newdir.name.c_str());
	newdir.used = used;
	m_dir.push_back(newdir);
	if(*used == true)
	{
		m_file_info_stale = true; // we got a new Dir, search again for all movies next time
		m_seriename_stale = true;
	}
	return (true);
}
/************************************************************************

************************************************************************/
void CMovieBrowser::loadMovies(bool doRefresh)
{
		time_t time_start = time(NULL);
		clock_t clock_start = clock()/10000; // CLOCKS_PER_SECOND
		//clock_t clock_prev = clock_start;
		clock_t clock_act = clock_start;

		TRACE("[mb] loadMovies: \n");
		
		CHintBox loadBox(LOCALE_MOVIEBROWSER_HEAD,g_Locale->getText(LOCALE_MOVIEBROWSER_SCAN_FOR_MOVIES));
		loadBox.paint();

		//clock_act = clock()/10000;TRACE("[mb] *1: time %9ld  clock %6ld  dclock %6ld*\n",(long)time(NULL),clock_act,clock_act - clock_prev);clock_prev = clock_act;
		loadAllTsFileNamesFromStorage(); // P1
		//clock_act = clock()/10000;TRACE("[mb] *2: time %9ld  clock %6ld  dclock %6ld*\n",(long)time(NULL),clock_act,clock_act - clock_prev);clock_prev = clock_act;
		loadAllMovieInfo(); // P1
		//clock_act = clock()/10000;TRACE("[mb] *3: time %9ld  clock %6ld  dclock %6ld*\n",(long)time(NULL),clock_act,clock_act - clock_prev);clock_prev = clock_act;
		m_file_info_stale = false;
		m_seriename_stale = true; // we reloded the movie info, so make sure the other list are  updated later on as well
		updateSerienames();
		//clock_act = clock()/10000;TRACE("[mb] *4: time %9ld  clock %6ld  dclock %6ld*\n",(long)time(NULL),clock_act,clock_act - clock_prev);clock_prev = clock_act;
		if(m_settings.serie_auto_create == 1)
		{
			autoFindSerie();
		}

		loadBox.hide();

		if (doRefresh)
		{
			//clock_act = clock()/10000;TRACE("[mb] *5: time %9ld  clock %6ld  dclock %6ld*\n",(long)time(NULL),clock_act,clock_act - clock_prev);clock_prev = clock_act;
			refreshBrowserList();	
			//clock_act = clock()/10000;TRACE("[mb] *6: time %9ld  clock %6ld  dclock %6ld*\n",(long)time(NULL),clock_act,clock_act - clock_prev);clock_prev = clock_act;
			refreshLastPlayList();	
			refreshLastRecordList();
			refreshFilterList();
			refreshMovieInfo();	// is done by refreshBrowserList if needed
			//clock_act = clock()/10000;TRACE("[mb] *7: time %9ld  clock %6ld  dclock %6ld*\n",(long)time(NULL),clock_act,clock_act - clock_prev);clock_prev = clock_act;
		}

		clock_act = clock()/10000;
		TRACE("[mb] ***Total:time %ld clock %ld***\n",(time(NULL)-time_start), clock_act-clock_start);
}
/************************************************************************

************************************************************************/
void CMovieBrowser::loadAllMovieInfo(void)
{
	//TRACE("[mb]->loadAllMovieInfo \r\n");

	for(unsigned int i=0; i < m_vMovieInfo.size();i++)
	{
		m_movieInfo.loadMovieInfo( &(m_vMovieInfo[i]));
	}
}


/************************************************************************

************************************************************************/
void CMovieBrowser::showHelp(void)
{
	CMovieHelp help;
	help.exec(NULL,NULL);
}

#define MAX_STRING 30
/************************************************************************

************************************************************************/
int CMovieBrowser::showMovieInfoMenu(MI_MOVIE_INFO* movie_info)
{
	/********************************************************************/
	/**  MovieInfo menu ******************************************************/

	/********************************************************************/
	/**  bookmark ******************************************************/
	CStringInputSMS*    pBookNameInput[MAX_NUMBER_OF_BOOKMARK_ITEMS];
	CIntInput*          pBookPosIntInput[MAX_NUMBER_OF_BOOKMARK_ITEMS];
	CIntInput*	    pBookTypeIntInput[MAX_NUMBER_OF_BOOKMARK_ITEMS];
	CMenuWidget*        pBookItemMenu[MAX_NUMBER_OF_BOOKMARK_ITEMS];
	CMenuForwarder* pBookItemMenuForwarder[MAX_NUMBER_OF_BOOKMARK_ITEMS];
	CBookItemMenuForwarderNotifier* pBookItemMenuForwarderNotifier[MAX_NUMBER_OF_BOOKMARK_ITEMS];

	CIntInput bookStartIntInput (LOCALE_MOVIEBROWSER_EDIT_BOOK, (int&)movie_info->bookmarks.start,        5, LOCALE_MOVIEBROWSER_EDIT_BOOK_POS_INFO1, LOCALE_MOVIEBROWSER_EDIT_BOOK_POS_INFO2);
	CIntInput bookLastIntInput (LOCALE_MOVIEBROWSER_EDIT_BOOK,  (int&)movie_info->bookmarks.lastPlayStop, 5, LOCALE_MOVIEBROWSER_EDIT_BOOK_POS_INFO1, LOCALE_MOVIEBROWSER_EDIT_BOOK_POS_INFO2);
	CIntInput bookEndIntInput (LOCALE_MOVIEBROWSER_EDIT_BOOK,   (int&)movie_info->bookmarks.end,          5, LOCALE_MOVIEBROWSER_EDIT_BOOK_POS_INFO1, LOCALE_MOVIEBROWSER_EDIT_BOOK_POS_INFO2);

	CMenuWidget bookmarkMenu(LOCALE_MOVIEBROWSER_INFO_HEAD, NEUTRINO_ICON_STREAMING);
	bookmarkMenu.addIntroItems(LOCALE_MOVIEBROWSER_BOOK_HEAD);
	bookmarkMenu.addItem( new CMenuForwarder(LOCALE_MOVIEBROWSER_BOOK_CLEAR_ALL, true, NULL, this, "book_clear_all", CRCInput::RC_red));
	bookmarkMenu.addItem(GenericMenuSeparatorLine);
	bookmarkMenu.addItem( new CMenuForwarder(LOCALE_MOVIEBROWSER_BOOK_MOVIESTART,    true, bookStartIntInput.getValue(), &bookStartIntInput));
	bookmarkMenu.addItem( new CMenuForwarder(LOCALE_MOVIEBROWSER_BOOK_MOVIEEND,      true, bookEndIntInput.getValue(),   &bookEndIntInput));
	bookmarkMenu.addItem( new CMenuForwarder(LOCALE_MOVIEBROWSER_BOOK_LASTMOVIESTOP, true, bookLastIntInput.getValue(),  &bookLastIntInput));
	bookmarkMenu.addItem(GenericMenuSeparatorLine);

	for(int i =0 ; i < MI_MOVIE_BOOK_USER_MAX && i < MAX_NUMBER_OF_BOOKMARK_ITEMS; i++ )
	{
		pBookItemMenuForwarderNotifier[i] = new CBookItemMenuForwarderNotifier();

		pBookNameInput[i] =    new CStringInputSMS (LOCALE_MOVIEBROWSER_EDIT_BOOK, &movie_info->bookmarks.user[i].name, 20, true, LOCALE_MOVIEBROWSER_EDIT_BOOK_NAME_INFO1, LOCALE_MOVIEBROWSER_EDIT_BOOK_NAME_INFO2, "abcdefghijklmnopqrstuvwxyz\xE4\xF6\xFC\xDF""0123456789-_/()<>=.,:!?\\'\"& ", pBookItemMenuForwarderNotifier[i]);
		pBookPosIntInput[i] =  new CIntInput (LOCALE_MOVIEBROWSER_EDIT_BOOK, (int&) movie_info->bookmarks.user[i].pos, 20, LOCALE_MOVIEBROWSER_EDIT_BOOK_POS_INFO1, LOCALE_MOVIEBROWSER_EDIT_BOOK_POS_INFO2);
		pBookTypeIntInput[i] = new CIntInput (LOCALE_MOVIEBROWSER_EDIT_BOOK, (int&) movie_info->bookmarks.user[i].length, 20, LOCALE_MOVIEBROWSER_EDIT_BOOK_TYPE_INFO1, LOCALE_MOVIEBROWSER_EDIT_BOOK_TYPE_INFO2);

		pBookItemMenu[i] = new CMenuWidget(LOCALE_MOVIEBROWSER_INFO_HEAD, NEUTRINO_ICON_STREAMING);
		pBookItemMenu[i]->addIntroItems(LOCALE_MOVIEBROWSER_BOOK_HEAD);
		pBookItemMenu[i]->addItem( new CMenuForwarder(LOCALE_MOVIEBROWSER_BOOK_NAME,     true,  movie_info->bookmarks.user[i].name,pBookNameInput[i]));
		pBookItemMenu[i]->addItem( new CMenuForwarder(LOCALE_MOVIEBROWSER_BOOK_POSITION, true,  pBookPosIntInput[i]->getValue(), pBookPosIntInput[i]));
		pBookItemMenu[i]->addItem( new CMenuForwarder(LOCALE_MOVIEBROWSER_BOOK_TYPE,     true,  pBookTypeIntInput[i]->getValue(),pBookTypeIntInput[i]));

		pBookItemMenuForwarder[i] = new CMenuForwarder(movie_info->bookmarks.user[i].name.c_str(), true, pBookPosIntInput[i]->getValue(), pBookItemMenu[i]);
		pBookItemMenuForwarderNotifier[i]->setItem(pBookItemMenuForwarder[i]);

		bookmarkMenu.addItem(pBookItemMenuForwarder[i]);
	}

/********************************************************************/
/**  serie******************************************************/
	CStringInputSMS serieUserInput(LOCALE_MOVIEBROWSER_EDIT_SERIE, &movie_info->serieName, 20, true, NONEXISTANT_LOCALE, NONEXISTANT_LOCALE, "abcdefghijklmnopqrstuvwxyz\xE4\xF6\xFC\xDF""0123456789-_/()<>=.,:!?\\'\"& ", this);

	CMenuWidget serieMenu(LOCALE_MOVIEBROWSER_INFO_HEAD, NEUTRINO_ICON_STREAMING);
	serieMenu.addIntroItems(LOCALE_MOVIEBROWSER_SERIE_HEAD);
	serieMenu.addItem( new CMenuForwarder(LOCALE_MOVIEBROWSER_SERIE_NAME,   true, movie_info->serieName,&serieUserInput));
	if (!m_vHandleSerienames.empty())
	{
		m_serienames.clear();
		serieMenu.addItem(new CMenuSeparator(CMenuSeparator::LINE | CMenuSeparator::STRING, LOCALE_MOVIEBROWSER_SERIE_EXISTINGNAME));
	}
	for(unsigned int i=0; i < m_vHandleSerienames.size(); i++)
	{
		m_serienames.push_back(m_vHandleSerienames[i]->serieName);
		serieMenu.addItem( new CMenuSelector(m_serienames.back().c_str(), true,  movie_info->serieName));
	}

    /********************************************************************/
    /**  update movie info  ******************************************************/
	for(int i=0; i<MB_INFO_MAX_NUMBER; i++)
		movieInfoUpdateAll[i]=0;

	movieInfoUpdateAllIfDestEmptyOnly=true;

	CMenuWidget movieInfoMenuUpdate(LOCALE_MOVIEBROWSER_INFO_HEAD, NEUTRINO_ICON_STREAMING, m_cBoxFrame.iWidth);
	movieInfoMenuUpdate.addIntroItems(LOCALE_MOVIEBROWSER_INFO_HEAD_UPDATE);
	movieInfoMenuUpdate.addItem( new CMenuForwarder(LOCALE_MOVIEBROWSER_MENU_SAVE_ALL, true, NULL, this, "save_movie_info_all", CRCInput::RC_red), true);
	movieInfoMenuUpdate.addItem(GenericMenuSeparatorLine);
	movieInfoMenuUpdate.addItem( new CMenuOptionChooser(LOCALE_MOVIEBROWSER_UPDATE_IF_DEST_EMPTY_ONLY, (&movieInfoUpdateAllIfDestEmptyOnly), MESSAGEBOX_YES_NO_OPTIONS, MESSAGEBOX_YES_NO_OPTIONS_COUNT, true, NULL, CRCInput::RC_green));
	movieInfoMenuUpdate.addItem(GenericMenuSeparatorLine);
	movieInfoMenuUpdate.addItem( new CMenuOptionChooser(LOCALE_MOVIEBROWSER_INFO_TITLE, &movieInfoUpdateAll[MB_INFO_TITLE], MESSAGEBOX_YES_NO_OPTIONS, MESSAGEBOX_YES_NO_OPTIONS_COUNT, true, NULL, CRCInput::RC_1));
	movieInfoMenuUpdate.addItem( new CMenuOptionChooser(LOCALE_MOVIEBROWSER_INFO_INFO1, &movieInfoUpdateAll[MB_INFO_INFO1], MESSAGEBOX_YES_NO_OPTIONS, MESSAGEBOX_YES_NO_OPTIONS_COUNT, true, NULL, CRCInput::RC_2));
	movieInfoMenuUpdate.addItem( new CMenuOptionChooser(LOCALE_MOVIEBROWSER_INFO_SERIE, &movieInfoUpdateAll[MB_INFO_SERIE], MESSAGEBOX_YES_NO_OPTIONS, MESSAGEBOX_YES_NO_OPTIONS_COUNT, true, NULL, CRCInput::RC_3));
	movieInfoMenuUpdate.addItem( new CMenuOptionChooser(LOCALE_MOVIEBROWSER_INFO_QUALITY, &movieInfoUpdateAll[MB_INFO_QUALITY], MESSAGEBOX_YES_NO_OPTIONS, MESSAGEBOX_YES_NO_OPTIONS_COUNT, true, NULL, CRCInput::RC_4));
	movieInfoMenuUpdate.addItem( new CMenuOptionChooser(LOCALE_MOVIEBROWSER_INFO_PARENTAL_LOCKAGE, &movieInfoUpdateAll[MB_INFO_PARENTAL_LOCKAGE], MESSAGEBOX_YES_NO_OPTIONS, MESSAGEBOX_YES_NO_OPTIONS_COUNT, true, NULL, CRCInput::RC_5));
	movieInfoMenuUpdate.addItem( new CMenuOptionChooser(LOCALE_MOVIEBROWSER_INFO_GENRE_MAJOR, &movieInfoUpdateAll[MB_INFO_MAJOR_GENRE], MESSAGEBOX_YES_NO_OPTIONS, MESSAGEBOX_YES_NO_OPTIONS_COUNT, true, NULL, CRCInput::RC_6));
	movieInfoMenuUpdate.addItem( new CMenuOptionChooser(LOCALE_MOVIEBROWSER_INFO_PRODYEAR, &movieInfoUpdateAll[MB_INFO_PRODDATE], MESSAGEBOX_YES_NO_OPTIONS, MESSAGEBOX_YES_NO_OPTIONS_COUNT, true, NULL, CRCInput::RC_7));
	movieInfoMenuUpdate.addItem( new CMenuOptionChooser(LOCALE_MOVIEBROWSER_INFO_PRODCOUNTRY, &movieInfoUpdateAll[MB_INFO_COUNTRY], MESSAGEBOX_YES_NO_OPTIONS, MESSAGEBOX_YES_NO_OPTIONS_COUNT, true, NULL, CRCInput::RC_8));
	movieInfoMenuUpdate.addItem( new CMenuOptionChooser(LOCALE_MOVIEBROWSER_INFO_LENGTH, &movieInfoUpdateAll[MB_INFO_LENGTH], MESSAGEBOX_YES_NO_OPTIONS, MESSAGEBOX_YES_NO_OPTIONS_COUNT, true, NULL, CRCInput::RC_9));

/********************************************************************/
/**  movieInfo  ******************************************************/
#define BUFFER_SIZE 100
	char dirItNr[BUFFER_SIZE];
	char size[BUFFER_SIZE];

	if(movie_info != NULL)
	{
		strncpy(dirItNr, m_dirNames[movie_info->dirItNr].c_str(),BUFFER_SIZE);
		snprintf(size,BUFFER_SIZE,"%5llu",movie_info->file.Size>>20);
	}

	CStringInputSMS titelUserInput(LOCALE_MOVIEBROWSER_INFO_TITLE, &movie_info->epgTitle, MAX_STRING, true, NONEXISTANT_LOCALE, NONEXISTANT_LOCALE, "abcdefghijklmnopqrstuvwxyz\xE4\xF6\xFC\xDF""0123456789-_/()<>=.,:!?\\'\"& ");
	CStringInputSMS channelUserInput(LOCALE_MOVIEBROWSER_INFO_CHANNEL, &movie_info->epgChannel, MAX_STRING, true, NONEXISTANT_LOCALE, NONEXISTANT_LOCALE, "abcdefghijklmnopqrstuvwxyz\xE4\xF6\xFC\xDF""0123456789-_/()<>=.,:!?\\'\"& ");
	CStringInputSMS epgUserInput(LOCALE_MOVIEBROWSER_INFO_INFO1, &movie_info->epgInfo1, MAX_STRING, true, NONEXISTANT_LOCALE, NONEXISTANT_LOCALE, "abcdefghijklmnopqrstuvwxyz\xE4\xF6\xFC\xDF""0123456789-_/()<>=.,:!?\\'\"& ");
	CDateInput dateUserDateInput(LOCALE_MOVIEBROWSER_INFO_LENGTH, &movie_info->dateOfLastPlay);
	CDateInput recUserDateInput(LOCALE_MOVIEBROWSER_INFO_LENGTH, &movie_info->file.Time);
	CStringInputSMS countryUserInput(LOCALE_MOVIEBROWSER_INFO_PRODCOUNTRY, &movie_info->productionCountry, 11, true, NONEXISTANT_LOCALE, NONEXISTANT_LOCALE, "abcdefghijklmnopqrstuvwxyz\xE4\xF6\xFC\xDF""0123456789-_/()<>=.,:!?\\'\"& ");

	CMenuWidget movieInfoMenu(LOCALE_MOVIEBROWSER_MENU_MAIN_HEAD, NEUTRINO_ICON_STREAMING, m_cBoxFrame.iWidth);
	movieInfoMenu.addIntroItems(LOCALE_MOVIEBROWSER_INFO_HEAD);
	movieInfoMenu.addItem( new CMenuForwarder(LOCALE_MOVIEBROWSER_MENU_SAVE, true, NULL, this, "save_movie_info", CRCInput::RC_red), true);
	movieInfoMenu.addItem( new CMenuForwarder(LOCALE_MOVIEBROWSER_INFO_HEAD_UPDATE, true, NULL, &movieInfoMenuUpdate, NULL, CRCInput::RC_green));
	movieInfoMenu.addItem(GenericMenuSeparatorLine);
	movieInfoMenu.addItem( new CMenuForwarder(LOCALE_MOVIEBROWSER_INFO_TITLE, true, movie_info->epgTitle, &titelUserInput,NULL, CRCInput::RC_1));
	movieInfoMenu.addItem( new CMenuForwarder(LOCALE_MOVIEBROWSER_INFO_SERIE, true, movie_info->serieName, &serieMenu,NULL, CRCInput::RC_2));
	movieInfoMenu.addItem( new CMenuForwarder(LOCALE_MOVIEBROWSER_INFO_INFO1, true, movie_info->epgInfo1, &epgUserInput,NULL, CRCInput::RC_3));
	movieInfoMenu.addItem( new CMenuOptionChooser(LOCALE_MOVIEBROWSER_INFO_GENRE_MAJOR, &movie_info->genreMajor, GENRE_ALL, GENRE_ALL_COUNT, true, NULL, CRCInput::RC_4, "", true, true));
	movieInfoMenu.addItem(GenericMenuSeparatorLine);
	movieInfoMenu.addItem( new CMenuOptionNumberChooser(LOCALE_MOVIEBROWSER_INFO_QUALITY, &movie_info->quality, true, 0, 3, 0, 0, NONEXISTANT_LOCALE, NULL, NULL, CRCInput::RC_5));
	movieInfoMenu.addItem( new CMenuOptionChooser(LOCALE_MOVIEBROWSER_INFO_PARENTAL_LOCKAGE, &movie_info->parentalLockAge, MESSAGEBOX_PARENTAL_LOCKAGE_OPTIONS, MESSAGEBOX_PARENTAL_LOCKAGE_OPTION_COUNT, true, NULL, CRCInput::RC_6));
	movieInfoMenu.addItem( new CMenuOptionNumberChooser(LOCALE_MOVIEBROWSER_INFO_PRODYEAR, &movie_info->productionDate, true, 0, 9999, 0, 0, NONEXISTANT_LOCALE, NULL, NULL, CRCInput::RC_7, "", true));
	movieInfoMenu.addItem( new CMenuForwarder(LOCALE_MOVIEBROWSER_INFO_PRODCOUNTRY, true, movie_info->productionCountry, &countryUserInput, NULL, CRCInput::RC_8));
	movieInfoMenu.addItem( new CMenuOptionNumberChooser(LOCALE_MOVIEBROWSER_INFO_LENGTH, &movie_info->length, true, 0, 999, 0, 0, NONEXISTANT_LOCALE, NULL, NULL, CRCInput::RC_9, "", true));
	movieInfoMenu.addItem( new CMenuForwarder(LOCALE_MOVIEBROWSER_INFO_CHANNEL, true, movie_info->epgChannel, &channelUserInput, NULL, CRCInput::RC_0));//LOCALE_TIMERLIST_CHANNEL
	movieInfoMenu.addItem(GenericMenuSeparatorLine);
	movieInfoMenu.addItem( new CMenuForwarder(LOCALE_MOVIEBROWSER_INFO_PATH, false, dirItNr)); //LOCALE_TIMERLIST_RECORDING_DIR
	movieInfoMenu.addItem( new CMenuForwarder(LOCALE_MOVIEBROWSER_INFO_PREVPLAYDATE, false, dateUserDateInput.getValue()));//LOCALE_FLASHUPDATE_CURRENTVERSIONDATE
	movieInfoMenu.addItem( new CMenuForwarder(LOCALE_MOVIEBROWSER_INFO_RECORDDATE, false, recUserDateInput.getValue()));//LOCALE_FLASHUPDATE_CURRENTVERSIONDATE
	movieInfoMenu.addItem( new CMenuForwarder(LOCALE_MOVIEBROWSER_INFO_SIZE, false, size));
	movieInfoMenu.addItem(GenericMenuSeparatorLine);
	movieInfoMenu.addItem( new CMenuForwarder(LOCALE_MOVIEBROWSER_BOOK_HEAD, true, NULL, &bookmarkMenu, NULL, CRCInput::RC_yellow));

	int returnval = movieInfoMenu.exec(NULL,"");

	updateSerienames();
	for(int i =0 ; i < MI_MOVIE_BOOK_USER_MAX && i < MAX_NUMBER_OF_BOOKMARK_ITEMS; i++ )
	{
		delete pBookNameInput[i] ;
		delete pBookPosIntInput[i] ;
		delete pBookTypeIntInput[i];
		delete pBookItemMenu[i];
		delete pBookItemMenuForwarderNotifier[i];
	}

	return returnval;
}

/************************************************************************

************************************************************************/
bool CMovieBrowser::showMenu(MI_MOVIE_INFO* /*movie_info*/)
{
	/* first clear screen */
	m_pcWindow->paintBoxRel(0, 0, m_cBoxFrame.iWidth, m_cBoxFrame.iHeight + 10, (CFBWindow::color_t)COL_BACKGROUND);
	int i;
/********************************************************************/
/**  directory menu ******************************************************/
	CDirMenu dirMenu(&m_dir);

/********************************************************************/
/**  options menu **************************************************/

/********************************************************************/
/**  parental lock **************************************************/
	CMenuWidget parentalMenu(LOCALE_EPGPLUS_OPTIONS, NEUTRINO_ICON_STREAMING, 450);
	parentalMenu.addIntroItems(LOCALE_MOVIEBROWSER_MENU_PARENTAL_LOCK_HEAD);
	parentalMenu.addItem( new CMenuOptionChooser(LOCALE_MOVIEBROWSER_MENU_PARENTAL_LOCK_ACTIVATED, (int*)(&m_parentalLock), MESSAGEBOX_PARENTAL_LOCK_OPTIONS, MESSAGEBOX_PARENTAL_LOCK_OPTIONS_COUNT, true ));
	parentalMenu.addItem( new CMenuOptionChooser(LOCALE_MOVIEBROWSER_MENU_PARENTAL_LOCK_RATE_HEAD, (int*)(&m_settings.parentalLockAge), MESSAGEBOX_PARENTAL_LOCKAGE_OPTIONS, MESSAGEBOX_PARENTAL_LOCKAGE_OPTION_COUNT, true ));


/********************************************************************/
/**  optionsVerzeichnisse  **************************************************/
	CMenuWidget optionsMenuDir(LOCALE_EPGPLUS_OPTIONS, NEUTRINO_ICON_STREAMING, 450);

	optionsMenuDir.addIntroItems(LOCALE_MOVIEBROWSER_MENU_DIRECTORIES_HEAD, LOCALE_MOVIEBROWSER_USE_MOVIE_DIR);
	optionsMenuDir.addItem( new CMenuOptionChooser(g_settings.streaming_moviedir.c_str(),     (int*)(&m_settings.storageDirMovieUsed), MESSAGEBOX_YES_NO_OPTIONS, MESSAGEBOX_YES_NO_OPTIONS_COUNT, true ));
	
	optionsMenuDir.addItem(new CMenuSeparator(CMenuSeparator::LINE | CMenuSeparator::STRING, LOCALE_TIMERLIST_RECORDING_DIR));
	CMenuOptionChooser* chooserRec[MAX_RECORDING_DIR];
	for(i = 0; i < MAX_RECORDING_DIR; i++)
	{
		if(!g_settings.recording_dir[i].empty() &&
			g_settings.recording_dir[i] != g_settings.streaming_moviedir)
		{
		chooserRec[i] =   new CMenuOptionChooser(g_settings.recording_dir[i].c_str() , &m_settings.storageDirRecUsed[i]  , MESSAGEBOX_YES_NO_OPTIONS, MESSAGEBOX_YES_NO_OPTIONS_COUNT, true);
		optionsMenuDir.addItem(chooserRec[i] );
        	}
	}

	CMenuWidget optionsMenuDirUser(LOCALE_MOVIEBROWSER_MENU_DIRECTORIES_HEAD, NEUTRINO_ICON_STREAMING, 450);
	optionsMenuDirUser.addIntroItems(LOCALE_MOVIEBROWSER_DIR_HEAD);

	CDirChooser*      dirInput[MB_MAX_DIRS];
	CMenuOptionChooser* chooser[MB_MAX_DIRS];
	COnOffNotifier*     notifier[MB_MAX_DIRS];
	CMenuForwarder*     forwarder[MB_MAX_DIRS];
	for(i=0; i<MB_MAX_DIRS ;i++)
	{
		dirInput[i] =  new CDirChooser(&m_settings.storageDir[i]);
		forwarder[i] = new CMenuForwarder(LOCALE_MOVIEBROWSER_DIR,        m_settings.storageDirUsed[i], m_settings.storageDir[i],      dirInput[i]);
		notifier[i] =  new COnOffNotifier();
		notifier[i]->addItem(forwarder[i]);
		chooser[i] =   new CMenuOptionChooser(LOCALE_MOVIEBROWSER_USE_DIR , &m_settings.storageDirUsed[i]  , MESSAGEBOX_YES_NO_OPTIONS, MESSAGEBOX_YES_NO_OPTIONS_COUNT, true,notifier[i]);
		if(i > 0)
			optionsMenuDirUser.addItem(GenericMenuSeparator);
		optionsMenuDirUser.addItem(chooser[i] );
		optionsMenuDirUser.addItem(forwarder[i] );
	}

	optionsMenuDir.addItem( new CMenuForwarder(LOCALE_MOVIEBROWSER_DIR_HEAD, true, NULL, &optionsMenuDirUser));

/********************************************************************/
/**  optionsMenuBrowser  **************************************************/
	CMenuWidget optionsMenuBrowser(LOCALE_EPGPLUS_OPTIONS, NEUTRINO_ICON_STREAMING, 480);
	optionsMenuBrowser.addIntroItems(LOCALE_MOVIEBROWSER_OPTION_BROWSER);
	optionsMenuBrowser.addItem( new CMenuOptionNumberChooser(LOCALE_MOVIEBROWSER_LAST_PLAY_MAX_ITEMS, &m_settings.lastPlayMaxItems, true, 0, 999, 0, 0, NONEXISTANT_LOCALE, NULL, NULL, CRCInput::RC_nokey, "", true));
	optionsMenuBrowser.addItem( new CMenuOptionNumberChooser(LOCALE_MOVIEBROWSER_LAST_RECORD_MAX_ITEMS, &m_settings.lastRecordMaxItems, true, 0, 999, 0, 0, NONEXISTANT_LOCALE, NULL, NULL, CRCInput::RC_nokey, "", true));
	optionsMenuBrowser.addItem( new CMenuOptionNumberChooser(LOCALE_MOVIEBROWSER_BROWSER_FRAME_HIGH, &m_settings.browserFrameHeight, true, MIN_BROWSER_FRAME_HEIGHT, MAX_BROWSER_FRAME_HEIGHT, 0, 0, NONEXISTANT_LOCALE, NULL, NULL, CRCInput::RC_nokey, "", true));
	optionsMenuBrowser.addItem(new CMenuSeparator(CMenuSeparator::LINE | CMenuSeparator::STRING, LOCALE_MOVIEBROWSER_BROWSER_ROW_HEAD));
	optionsMenuBrowser.addItem( new CMenuOptionNumberChooser(LOCALE_MOVIEBROWSER_BROWSER_ROW_NR, &m_settings.browserRowNr, true, 1, MB_MAX_ROWS, 0, 0, NONEXISTANT_LOCALE, NULL, NULL, CRCInput::RC_nokey, "", true));
	for(i=0; i<MB_MAX_ROWS; i++)
	{
		optionsMenuBrowser.addItem(GenericMenuSeparator);
		optionsMenuBrowser.addItem( new CMenuOptionChooser(LOCALE_MOVIEBROWSER_BROWSER_ROW_ITEM, (int*)(&m_settings.browserRowItem[i]), MESSAGEBOX_BROWSER_ROW_ITEM, MESSAGEBOX_BROWSER_ROW_ITEM_COUNT, true, NULL, CRCInput::convertDigitToKey(i+1), "", true, true));
		optionsMenuBrowser.addItem( new CMenuOptionNumberChooser(LOCALE_MOVIEBROWSER_BROWSER_ROW_WIDTH, &m_settings.browserRowWidth[i], true, 10, 500, 0, 0, NONEXISTANT_LOCALE, NULL, NULL, CRCInput::RC_nokey, "", true));
	}

/********************************************************************/
/**  options  **************************************************/

	CMenuWidget optionsMenu(LOCALE_MOVIEBROWSER_MENU_MAIN_HEAD, NEUTRINO_ICON_STREAMING, 450);
	optionsMenu.addIntroItems(LOCALE_EPGPLUS_OPTIONS);
	optionsMenu.addItem( new CMenuForwarder(LOCALE_MOVIEBROWSER_LOAD_DEFAULT, true, NULL, this, "loaddefault", CRCInput::RC_red));
	optionsMenu.addItem( new CMenuForwarder(LOCALE_MOVIEBROWSER_OPTION_BROWSER, true, NULL, &optionsMenuBrowser, NULL, CRCInput::RC_green));
	optionsMenu.addItem( new CMenuForwarder(LOCALE_MOVIEBROWSER_MENU_DIRECTORIES_HEAD, true, NULL, &optionsMenuDir, NULL, CRCInput::RC_yellow));
	optionsMenu.addItem( new CLockedMenuForwarder(LOCALE_MOVIEBROWSER_MENU_PARENTAL_LOCK_HEAD, g_settings.parentallock_pincode, m_parentalLock != MB_PARENTAL_LOCK_OFF, true, NULL, &parentalMenu, NULL, CRCInput::RC_blue));
	optionsMenu.addItem(GenericMenuSeparatorLine);
	optionsMenu.addItem( new CMenuOptionChooser(LOCALE_MOVIEBROWSER_RELOAD_AT_START,   (int*)(&m_settings.reload), MESSAGEBOX_YES_NO_OPTIONS, MESSAGEBOX_YES_NO_OPTIONS_COUNT, true ));
	optionsMenu.addItem( new CMenuOptionChooser(LOCALE_MOVIEBROWSER_REMOUNT_AT_START,  (int*)(&m_settings.remount), MESSAGEBOX_YES_NO_OPTIONS, MESSAGEBOX_YES_NO_OPTIONS_COUNT, true ));
	optionsMenu.addItem(GenericMenuSeparatorLine);
	optionsMenu.addItem( new CMenuOptionChooser(LOCALE_MOVIEBROWSER_HIDE_SERIES,       (int*)(&m_settings.browser_serie_mode), MESSAGEBOX_YES_NO_OPTIONS, MESSAGEBOX_YES_NO_OPTIONS_COUNT, true ));
    optionsMenu.addItem( new CMenuOptionChooser(LOCALE_MOVIEBROWSER_SERIE_AUTO_CREATE, (int*)(&m_settings.serie_auto_create), MESSAGEBOX_YES_NO_OPTIONS, MESSAGEBOX_YES_NO_OPTIONS_COUNT, true ));
 
/********************************************************************/
/**  main menu ******************************************************/
	CMovieHelp* movieHelp = new CMovieHelp();
#ifdef ENABLE_GUI_MOUNT
	//CNFSSmallMenu* nfs = new CNFSSmallMenu();
#endif

	CMenuWidget mainMenu(LOCALE_MOVIEBROWSER_MENU_MAIN_HEAD, NEUTRINO_ICON_STREAMING);
	mainMenu.addIntroItems();
	mainMenu.addItem( new CMenuForwarder(LOCALE_MOVIEBROWSER_INFO_HEAD, (m_movieSelectionHandler != NULL), NULL, this, "show_movie_info_menu", CRCInput::RC_red), true);
	mainMenu.addItem(GenericMenuSeparatorLine);
	mainMenu.addItem( new CMenuForwarder(LOCALE_EPGPLUS_OPTIONS, true, NULL, &optionsMenu, NULL, CRCInput::RC_green));
	mainMenu.addItem( new CMenuForwarder(LOCALE_MOVIEBROWSER_MENU_DIRECTORIES_HEAD, true, NULL, &dirMenu, NULL, CRCInput::RC_yellow));
	mainMenu.addItem( new CMenuForwarder(LOCALE_MOVIEBROWSER_SCAN_FOR_MOVIES,true, NULL, this, "reload_movie_info", CRCInput::RC_blue));
#ifdef ENABLE_GUI_MOUNT
	//mainMenu.addItem( new CMenuForwarder(LOCALE_MOVIEBROWSER_MENU_NFS_HEAD, true, NULL, nfs, NULL, CRCInput::RC_setup));
#endif
#ifdef MOVEMANAGER
	mainMenu.addItem(GenericMenuSeparatorLine);
	mainMenu.addItem( new CMenuForwarder("Kopierwerk", true, NULL, CMoveManager::getInstance()));
#endif // MOVEMANAGER
	mainMenu.addItem(GenericMenuSeparatorLine);
	mainMenu.addItem( new CMenuForwarder(LOCALE_MOVIEBROWSER_MENU_HELP_HEAD, true, NULL, movieHelp, NULL, CRCInput::RC_help));
	mainMenu.exec(NULL, " ");

	// post menu handling
	if (m_parentalLock != MB_PARENTAL_LOCK_OFF_TMP)
		m_settings.parentalLock = m_parentalLock;

	if(dirMenu.isChanged())
		loadMovies();

	updateSerienames();
	refreshBrowserList();
	refreshLastPlayList();
	refreshLastRecordList();
	refreshFilterList();
	refreshMovieInfo();
	refresh();
	for(i=0; i<MB_MAX_DIRS ;i++)
	{
		delete dirInput[i];
		delete notifier[i];
	}

	delete movieHelp;
#ifdef ENABLE_GUI_MOUNT
	//delete nfs;
#endif
	
	//restart_mb_timeout = 1;

	return(true);
}

/************************************************************************

************************************************************************/
int CMovieBrowser::showStartPosSelectionMenu(void) // P2
{
	//TRACE("[mb]->showStartPosSelectionMenu\r\n");
	int pos = -1;
	int result = 0;
	int menu_nr= 0;
	int position[MAX_NUMBER_OF_BOOKMARK_ITEMS];
	CMenuForwarder* startPosItem;
	bool defaultselected = true;
	bool sep_added = false;
	
	if(m_movieSelectionHandler == NULL) return(result);

	char start_pos[13];
	snprintf(start_pos, 13, "%3d %s", m_movieSelectionHandler->bookmarks.start / 60, g_Locale->getText(LOCALE_WORD_MINUTES_SHORT));
	char play_pos[13];
	snprintf(play_pos, 13, "%3d %s", m_movieSelectionHandler->bookmarks.lastPlayStop / 60, g_Locale->getText(LOCALE_WORD_MINUTES_SHORT));

	char book[MI_MOVIE_BOOK_USER_MAX][20];
	char menu_nr_str[5];

	CMenuWidget startPosSelectionMenu(LOCALE_MOVIEBROWSER_START_HEAD , NEUTRINO_ICON_STREAMING);
	CMenuSelectorTarget startPosChanger(&result);
	
	startPosSelectionMenu.addIntroItems(NONEXISTANT_LOCALE, NONEXISTANT_LOCALE, CMenuWidget::BTN_TYPE_CANCEL);
	
	if( m_movieSelectionHandler->bookmarks.start != 0)
	{
		sprintf(menu_nr_str, "%d", menu_nr);
		startPosItem = new CMenuForwarder(LOCALE_MOVIEBROWSER_BOOK_MOVIESTART, true, start_pos, &startPosChanger, menu_nr_str);
		startPosItem->setItemButton(NEUTRINO_ICON_BUTTON_OKAY, true);
		startPosSelectionMenu.addItem(startPosItem, defaultselected);
		defaultselected = false;
		position[menu_nr++] = m_movieSelectionHandler->bookmarks.start;
	}
	if( m_movieSelectionHandler->bookmarks.lastPlayStop != 0) 
	{
		sprintf(menu_nr_str, "%d", menu_nr);
		startPosItem = new CMenuForwarder(LOCALE_MOVIEBROWSER_BOOK_LASTMOVIESTOP, true, play_pos, &startPosChanger, menu_nr_str);
		startPosItem->setItemButton(NEUTRINO_ICON_BUTTON_OKAY, true);
		startPosSelectionMenu.addItem(startPosItem, defaultselected);
		defaultselected = false;
		position[menu_nr++] = m_movieSelectionHandler->bookmarks.lastPlayStop;
	}
	sprintf(menu_nr_str, "%d", menu_nr);
	startPosItem = new CMenuForwarder(LOCALE_MOVIEBROWSER_START_RECORD_START, true, NULL, &startPosChanger, menu_nr_str);
	startPosItem->setItemButton(NEUTRINO_ICON_BUTTON_OKAY, true);
	startPosSelectionMenu.addItem(startPosItem, defaultselected);
	position[menu_nr++] = 0;

	for(int i =0 ; i < MI_MOVIE_BOOK_USER_MAX && menu_nr < MAX_NUMBER_OF_BOOKMARK_ITEMS; i++ )
	{
		if( m_movieSelectionHandler->bookmarks.user[i].pos != 0 )
		{
			if(!sep_added)
			{
				startPosSelectionMenu.addItem(GenericMenuSeparatorLine);
				sep_added = true;
			}

			if(m_movieSelectionHandler->bookmarks.user[i].length >= 0)
				position[menu_nr] = m_movieSelectionHandler->bookmarks.user[i].pos;
			else
				position[menu_nr] = m_movieSelectionHandler->bookmarks.user[i].pos + m_movieSelectionHandler->bookmarks.user[i].length;

			snprintf(book[i], 20, "%5d %s", position[menu_nr] / 60, g_Locale->getText(LOCALE_WORD_MINUTES_SHORT));
			sprintf(menu_nr_str, "%d", menu_nr);
			startPosItem = new CMenuForwarder(m_movieSelectionHandler->bookmarks.user[i].name.c_str(), true, book[i], &startPosChanger, menu_nr_str);
			startPosItem->setItemButton(NEUTRINO_ICON_BUTTON_OKAY, true);
			startPosSelectionMenu.addItem(startPosItem);
			menu_nr++;
		}
	}

	result = -1;
	startPosSelectionMenu.exec(NULL, "12345");
	/* check what menu item was ok'd  and set the appropriate play offset*/
	if (result > -1 && result < MAX_NUMBER_OF_BOOKMARK_ITEMS)
		pos = position[result];
	
	TRACE("[mb] res = %d,%d \r\n",result,pos);
	
	return(pos) ;
}


/************************************************************************

************************************************************************/
bool CMovieBrowser::isParentalLock(MI_MOVIE_INFO& movie_info)
{
	bool result = false;
	if(m_parentalLock == MB_PARENTAL_LOCK_ACTIVE && m_settings.parentalLockAge <= movie_info.parentalLockAge )
	{
		result = true;
	}
	return (result);
}
/************************************************************************

************************************************************************/
bool CMovieBrowser::isFiltered(MI_MOVIE_INFO& movie_info)
{
	bool result = true;
	
	switch(m_settings.filter.item)
	{
		case MB_INFO_FILEPATH:
			if(m_settings.filter.optionVar == movie_info.dirItNr)
				result = false;
			break;
		case MB_INFO_INFO1:
			if(strcmp(m_settings.filter.optionString.c_str(),movie_info.epgInfo1.c_str()) == 0) 
				result = false;
			break;
		case MB_INFO_INFO2:
			if(movie_info.epgInfo2.find(m_settings.filter.optionString) != std::string::npos )
				 result = false;
			break;
		case MB_INFO_MAJOR_GENRE:
			if(m_settings.filter.optionVar == movie_info.genreMajor)
				result = false;
			break;
		case MB_INFO_SERIE:
			if(strcmp(m_settings.filter.optionString.c_str(),movie_info.serieName.c_str()) == 0) 
				result = false;
			break;
		default:
				result = false;
			break;
	}
	return (result);
}

/************************************************************************

************************************************************************/
bool CMovieBrowser::getMovieInfoItem(MI_MOVIE_INFO& movie_info, MB_INFO_ITEM item, std::string* item_string)
{
	#define MAX_STR_TMP 100
	char str_tmp[MAX_STR_TMP];
	bool result = true;
	*item_string="";
	tm* tm_tmp;
	
	char text[20];
	int i=0;
	int counter=0;

	switch(item)
	{
		case MB_INFO_FILENAME: 				// 		= 0,
			*item_string = movie_info.file.getFileName();
			break;
		case MB_INFO_FILEPATH: 				// 		= 1,
			if (!m_dirNames.empty())
				*item_string = m_dirNames[movie_info.dirItNr];
			break;
		case MB_INFO_TITLE: 				// 		= 2,
			*item_string = movie_info.epgTitle;
			if(strcmp("not available",movie_info.epgTitle.c_str()) == 0)
				result = false;
			if(movie_info.epgTitle.empty())
				result = false;
			break;
		case MB_INFO_SERIE: 				// 		= 3,
			*item_string = movie_info.serieName;
			break;
		case MB_INFO_INFO1: 			//		= 4,
			*item_string = movie_info.epgInfo1;
			break;
		case MB_INFO_MAJOR_GENRE: 			// 		= 5,
			snprintf(str_tmp,MAX_STR_TMP,"%2d",movie_info.genreMajor);
			*item_string = str_tmp;
			break;
		case MB_INFO_MINOR_GENRE: 			// 		= 6,
			snprintf(str_tmp,MAX_STR_TMP,"%2d",movie_info.genreMinor);
			*item_string = str_tmp;
			break;
		case MB_INFO_INFO2: 					// 		= 7,
			*item_string = movie_info.epgInfo2;
			break;
		case MB_INFO_PARENTAL_LOCKAGE: 					// 		= 8,
			snprintf(str_tmp,MAX_STR_TMP,"%2d",movie_info.parentalLockAge);
			*item_string = str_tmp;
			break;
		case MB_INFO_CHANNEL: 				// 		= 9,
			*item_string = movie_info.epgChannel;
			break;
		case MB_INFO_BOOKMARK: 				//		= 10,
			// we just return the number of bookmarks
			for(i = 0; i < MI_MOVIE_BOOK_USER_MAX; i++)
			{
				if(movie_info.bookmarks.user[i].pos != 0) 
					counter++;
			}
			snprintf(text, 8,"%d",counter);
			text[9] = 0; // just to make sure string is terminated
			*item_string = text;
			break;
		case MB_INFO_QUALITY: 				// 		= 11,
			snprintf(str_tmp,MAX_STR_TMP,"%d",movie_info.quality);
			*item_string = str_tmp;
			break;
		case MB_INFO_PREVPLAYDATE: 			// 		= 12,
			tm_tmp = localtime(&movie_info.dateOfLastPlay);
			snprintf(str_tmp,MAX_STR_TMP,"%02d.%02d.%02d",tm_tmp->tm_mday,(tm_tmp->tm_mon)+ 1, tm_tmp->tm_year >= 100 ? tm_tmp->tm_year-100 : tm_tmp->tm_year);
			*item_string = str_tmp;
			break;
		case MB_INFO_RECORDDATE: 			// 		= 13,
			tm_tmp = localtime(&movie_info.file.Time);
			snprintf(str_tmp,MAX_STR_TMP,"%02d.%02d.%02d",tm_tmp->tm_mday,(tm_tmp->tm_mon) + 1,tm_tmp->tm_year >= 100 ? tm_tmp->tm_year-100 : tm_tmp->tm_year);
			*item_string = str_tmp;
			break;
		case MB_INFO_PRODDATE: 				// 		= 14,
			snprintf(str_tmp,MAX_STR_TMP,"%d",movie_info.productionDate);
			*item_string = str_tmp;
			break;
		case MB_INFO_COUNTRY: 				// 		= 15,
			*item_string = movie_info.productionCountry;
			break;
		case MB_INFO_GEOMETRIE: 			// 		= 16,
			result = false;
			break;
		case MB_INFO_AUDIO: 				// 		= 17,
#if 1  // MB_INFO_AUDIO test
			// we just return the number of audiopids
			char text2[10];
			snprintf(text2, 8,"%d",movie_info.audioPids.size());
			text2[9] = 0; // just to make sure string is terminated
			*item_string = text2;
#else // MB_INFO_AUDIO test
			for(i=0; i < movie_info.audioPids.size() && i < 10; i++)
			{
				if(movie_info.audioPids[i].epgAudioPidName[0].size() < 2)
				{
					text[counter++] = '?'; // two chars ??? -> strange name
					continue;
				}
				
				// check for Dolby Digital / AC3 Audio audiopids (less than 5.1 is not remarkable)
				if(	(movie_info.audioPids[i].epgAudioPidName.find("AC3") != -1 ) || 
					(movie_info.audioPids[i].epgAudioPidName.find("5.1") != -1 ))
				{
					ac3_found = true;
				}
				// Check for german audio pids
				if( movie_info.audioPids[i].epgAudioPidName[0] == 'D' || // Deutsch
					movie_info.audioPids[i].epgAudioPidName[0] == 'd' ||
					movie_info.audioPids[i].epgAudioPidName[0] == 'G' || // German
					movie_info.audioPids[i].epgAudioPidName[0] == 'g' ||
					movie_info.audioPids[i].epgAudioPidName[0] == 'M' || // for Mono, mono and Stereo, stereo we assume German ;)
					movie_info.audioPids[i].epgAudioPidName[0] == 'n' || 
					(movie_info.audioPids[i].epgAudioPidName[0] == 'S' && movie_info.audioPids[i].epgAudioPidName[1] == 't' ) || 
					(movie_info.audioPids[i].epgAudioPidName[0] == 's' && movie_info.audioPids[i].epgAudioPidName[1] == 't' ))
				{
					text[counter++] = 'D';
					continue;
				}
				// Check for english audio pids
				if( movie_info.audioPids[i].epgAudioPidName[0] == 'E' ||
					movie_info.audioPids[i].epgAudioPidName[0] == 'e')
				{
					text[counter++] = 'E';
					continue;
				}
				// Check for french audio pids
				if( movie_info.audioPids[i].epgAudioPidName[0] == 'F' ||
					movie_info.audioPids[i].epgAudioPidName[0] == 'f')
				{
					text[counter++] = 'F';
					continue;
				}
				// Check for italian audio pids
				if( movie_info.audioPids[i].epgAudioPidName[0] == 'I' ||
					movie_info.audioPids[i].epgAudioPidName[0] == 'i')
				{
					text[counter++] = 'I';
					continue;
				}
				// Check for spanish audio pids
				if( movie_info.audioPids[i].epgAudioPidName[0] == 'E' ||
					movie_info.audioPids[i].epgAudioPidName[0] == 'e' ||
					movie_info.audioPids[i].epgAudioPidName[0] == 'S' ||
					movie_info.audioPids[i].epgAudioPidName[0] == 's')
				{
					text[counter++] = 'S';
					continue;
				}
				text[counter++] = '?'; // We have not found any language for this pid
			}
			if(ac3_found == true)
			{
				text[counter++] = '5';
				text[counter++] = '.';
				text[counter++] = '1';
			}
			text[counter] = 0; // terminate string 
#endif	// MB_INFO_AUDIO test
			break;
		case MB_INFO_LENGTH: 				// 		= 18,
			snprintf(str_tmp,MAX_STR_TMP,"%4d", movie_info.rec_length > 0 ? (movie_info.rec_length + 30) / 60 : movie_info.length);
			*item_string = str_tmp;
			break;
		case MB_INFO_SIZE: 					// 		= 19, 
			snprintf(str_tmp,MAX_STR_TMP,"%4llu",movie_info.file.Size>>20);
			*item_string = str_tmp;
			break;
		case MB_INFO_MAX_NUMBER: 			//		= 20 
		default:
			*item_string="";
			result = false;
			break;
	}
	//TRACE("   getMovieInfoItem: %d,>%s<",item,*item_string.c_str());
	return(result);
}

/************************************************************************

************************************************************************/
void CMovieBrowser::updateSerienames(void)
{
	if(m_seriename_stale == false) 
		return;
		
	m_vHandleSerienames.clear();
	for(unsigned int i = 0; i < m_vMovieInfo.size(); i++)
	{
		if(!m_vMovieInfo[i].serieName.empty())
		{
			// current series name is not empty, lets see if we already have it in the list, and if not save it to the list.
			bool found = false;
			for(unsigned int t = 0; t < m_vHandleSerienames.size() && found == false; t++)
			{
				if(strcmp(m_vHandleSerienames[t]->serieName.c_str(),m_vMovieInfo[i].serieName.c_str()) == 0)
					found = true;
			}
			if(found == false)
				m_vHandleSerienames.push_back(&m_vMovieInfo[i]);
		}
	}
	TRACE("[mb]->updateSerienames: %d\r\n",m_vHandleSerienames.size());
	sort(m_vHandleSerienames.begin(), m_vHandleSerienames.end(), sortBySerie);
	m_seriename_stale = false;
}	

void CMovieBrowser::autoFindSerie(void)
{
	TRACE("autoFindSerie\n");
	updateSerienames(); // we have to make sure that the seriename array is up to date, otherwise this does not work
			// if the array is not stale, the function is left immediately
	for(unsigned int i = 0; i < m_vMovieInfo.size(); i++)
	{
		// For all movie infos, which do not have a seriename, we try to find one.
		// We search for a movieinfo with seriename, and than we do check if the title is the same
		// in case of same title, we assume both belongs to the same serie
		//TRACE("%s ",m_vMovieInfo[i].serieName);
		if( m_vMovieInfo[i].serieName.empty())
		{
			for(unsigned int t=0; t < m_vHandleSerienames.size();t++)
			{
				//TRACE("%s ",m_vHandleSerienames[i].serieName);
				if(m_vMovieInfo[i].epgTitle == m_vHandleSerienames[t]->epgTitle )
				{
					//TRACE("x");
					m_vMovieInfo[i].serieName = m_vHandleSerienames[t]->serieName;
					break; // we  found a maching serie, nothing to do else, leave for(t=0)
				}
			}
		 //TRACE("\n");
		}
	}
}

/************************************************************************

************************************************************************/

CBookItemMenuForwarderNotifier::CBookItemMenuForwarderNotifier(CMenuForwarder* MenuForwarder)
{
	menuForwarder = MenuForwarder;
}

void CBookItemMenuForwarderNotifier::setItem(CMenuForwarder* MenuForwarder)
{
	menuForwarder = MenuForwarder;
}

bool CBookItemMenuForwarderNotifier::changeNotify(const neutrino_locale_t, void* Data)
{
	if (menuForwarder)
	{
		char* text = (char*)Data;
		menuForwarder->setText(text);
	}
	return true;
}

/************************************************************************

************************************************************************/

const char * CSelectedMenu::getTargetValue()
{
	if (value)
	{
		if (*value > 0)
		{
			char tmp[16];
			sprintf(tmp, "%d", *value / 60);
			value_string = tmp;
			value_string += " ";
			value_string += g_Locale->getText(LOCALE_WORD_MINUTES_SHORT);
		}
		else
			value_string = "---";
		return value_string.c_str();
	}
	return NULL;
}

/************************************************************************

************************************************************************/

CMenuSelector::CMenuSelector(const char * OptionName, const bool Active , char * OptionValue, int* ReturnInt ,int ReturnIntValue ) : CMenuItem()
{
	optionValueString = NULL;
	optionName = 		OptionName;
	optionValue = 		OptionValue;
	active = 			Active;
	returnIntValue =	ReturnIntValue;
	returnInt = 		ReturnInt;
}

CMenuSelector::CMenuSelector(const char * OptionName, const bool Active , std::string& OptionValue, int* ReturnInt ,int ReturnIntValue ) : CMenuItem()
{
	optionValueString = &OptionValue;
	optionName =        OptionName;
	strncpy(buffer,OptionValue.c_str(),BUFFER_MAX);
	buffer[BUFFER_MAX-1] = 0;// terminate string
	optionValue =       buffer;
	active =        Active;
	returnIntValue =    ReturnIntValue;
	returnInt =         ReturnInt;
}

int CMenuSelector::getHeight(void) const
{
	return g_Font[SNeutrinoSettings::FONT_TYPE_MENU]->getHeight();
}

int CMenuSelector::exec(CMenuTarget* /*parent*/)
{ 
	if(returnInt != NULL)
		*returnInt= returnIntValue;
		
	if(optionValue != NULL && optionName != NULL) 
	{
		if(optionValueString == NULL)
			strcpy(optionValue,optionName); 
		else
			*optionValueString = optionName;
	}
	return menu_return::RETURN_EXIT;
}

int CMenuSelector::paint(bool selected)
{
	int height = getHeight();

	//paint item
	prepareItem(selected, height);

	//paint item icon
	paintItemButton(selected, height, NEUTRINO_ICON_BUTTON_OKAY);

	//paint text
	paintItemCaption(selected, height, optionName);

	return y+height;
}



/************************************************************************

************************************************************************/
int CMovieHelp::exec(CMenuTarget* /*parent*/, const std::string & /*actionKey*/)
{
	static CMovieBrowser mb;
	std::string version = "Moviebrowser: " + mb.getMovieBrowserVersion() + " by Günther";
	Helpbox helpbox;
	helpbox.addLine(version);
	helpbox.addLine("");

	helpbox.addLine(NEUTRINO_ICON_BUTTON_RED, g_Locale->getText(LOCALE_PICTUREVIEWER_HELP20));
	helpbox.addLine(NEUTRINO_ICON_BUTTON_GREEN, g_Locale->getText(LOCALE_MOVIEBROWSER_WINDOW_FILTER));
	helpbox.addLine(NEUTRINO_ICON_BUTTON_YELLOW, g_Locale->getText(LOCALE_MOVIEBROWSER_WINDOW_CHANGE_ACTIVE));
	helpbox.addLine(NEUTRINO_ICON_BUTTON_BLUE, g_Locale->getText(LOCALE_MOVIEBROWSER_INFO_FILM_RELOAD));
	helpbox.addLine(NEUTRINO_ICON_BUTTON_DBOX, g_Locale->getText(LOCALE_MAINMENU_HEAD));
	helpbox.addLine(NEUTRINO_ICON_BUTTON_LEFT, g_Locale->getText(LOCALE_MOVIEBROWSER_CHANGE_VIEW));
	helpbox.addLine(NEUTRINO_ICON_BUTTON_RIGHT, g_Locale->getText(LOCALE_MOVIEBROWSER_CHANGE_VIEW));
	helpbox.addLine(NEUTRINO_ICON_BUTTON_MUTE_SMALL, g_Locale->getText(LOCALE_MOVIEBROWSER_INFO_FILM_DELETE));
	helpbox.addLine("");
	helpbox.addLine(g_Locale->getText(LOCALE_MOVIEBROWSER_INFO_FILM_DURING));
	helpbox.addLine(NEUTRINO_ICON_BUTTON_BLUE, g_Locale->getText(LOCALE_MOVIEBROWSER_MENU_MARK));
	helpbox.addLine(NEUTRINO_ICON_BUTTON_0, g_Locale->getText(LOCALE_MOVIEBROWSER_MENU_MARK_NOACTION));
	
	helpbox.show(LOCALE_MOVIEBROWSER_MENU_HELP_HEAD);
	return menu_return::RETURN_NONE;
}


/////////////////////////////////////////////////
// MenuTargets
////////////////////////////////////////////////

/************************************************************************/
CDirMenu::CDirMenu(std::vector<MB_DIR>* dir_list)
/************************************************************************/
{
	unsigned int i;
	changed = false;
	dirList = dir_list;

	if( dirList->empty())
		return;

	for(i = 0; i < MAX_DIR; i++)
		dirNfsMountNr[i]=-1;

	for(i = 0; i < dirList->size() && i < MAX_DIR; i++)
	{
		for(int nfs = 0; nfs < NETWORK_NFS_NR_OF_ENTRIES; nfs++)
		{
			if(g_settings.network_nfs_local_dir[nfs][0] != 0)
			{
			std::string tmp = g_settings.network_nfs_local_dir[nfs];
			int result = (*dirList)[i].name.compare( 0,tmp.size(),tmp) ;
			//printf("[CDirMenu] (nfs%d) %s == (mb%d) %s (%d)\n",nfs,g_settings.network_nfs_local_dir[nfs],i,(*dirList)[i].name.c_str(),result);

				if(result == 0)
				{
					dirNfsMountNr[i] = nfs;
					break;
				}
			}
		}
	}
}

/************************************************************************/
int CDirMenu::exec(CMenuTarget* parent, const std::string & actionKey)
/************************************************************************/
{
    int returnval = menu_return::RETURN_REPAINT;

	if(actionKey.empty())
	{
		if(parent)
			parent->hide();

		changed = false;
		returnval = show();
	}
	else if(actionKey.size() == 1)
	{
		printf("[CDirMenu].exec %s\n",actionKey.c_str());
		int number = atoi(actionKey.c_str());
		if(number < MAX_DIR)
		{
			if(dirState[number] == DIR_STATE_SERVER_DOWN)
			{
				std::string command = "ether-wake ";
				command += g_settings.network_nfs_mac[dirNfsMountNr[number]];
				printf("try to start server: %s\n",command.c_str());
				if(system(command.c_str()) != 0)
					perror("ether-wake failed");

				dirOptionText[number]="STARTE SERVER";
			}
#ifdef ENABLE_GUI_MOUNT
			else if(dirState[number] == DIR_STATE_NOT_MOUNTED)
			{
				printf("[CDirMenu] try to mount %d,%d\n",number,dirNfsMountNr[number]);
				CFSMounter::MountRes res;
				res = CFSMounter::mount(  g_settings.network_nfs_ip[dirNfsMountNr[number]].c_str(),
				g_settings.network_nfs_dir[dirNfsMountNr[number]] ,
				g_settings.network_nfs_local_dir[dirNfsMountNr[number]] ,
				(CFSMounter::FSType)g_settings.network_nfs_type[dirNfsMountNr[number]] ,
				g_settings.network_nfs_username[dirNfsMountNr[number]] ,
				g_settings.network_nfs_password[dirNfsMountNr[number]] ,
				g_settings.network_nfs_mount_options1[dirNfsMountNr[number]] ,
				g_settings.network_nfs_mount_options2[dirNfsMountNr[number]] );
				if(res ==  CFSMounter::MRES_OK) // if mount is successful we set the state to active in any case
				{
					*(*dirList)[number].used = true;
				}
				// try to mount
				updateDirState();
				changed = true;
			}
#endif
			else if(dirState[number] == DIR_STATE_MOUNTED)
			{
				if(*(*dirList)[number].used == true)
				{
					*(*dirList)[number].used = false;
				}
				else
				{
					*(*dirList)[number].used = true;
				}
				//CFSMounter::umount(g_settings.network_nfs_local_dir[dirNfsMountNr[number]]);
				updateDirState();
				changed = true;
			}
		}
	}
	return returnval;
}

/************************************************************************/
void CDirMenu::updateDirState(void)
/************************************************************************/
{
	unsigned int drivefree = 0;
	struct statfs s;

	for(unsigned int i = 0; i < dirList->size() && i < MAX_DIR; i++)
	{
		 dirOptionText[i] = "UNKNOWN";
		 dirState[i] = DIR_STATE_UNKNOWN;
		// 1st ping server
		if(dirNfsMountNr[i] != -1)
		{
			int retvalue = pingthost(g_settings.network_nfs_ip[dirNfsMountNr[i]].c_str(), 500); // get ping for 60ms - increased
			if (retvalue == 0)//LOCALE_PING_UNREACHABLE
			{
				dirOptionText[i] = g_Locale->getText(LOCALE_RECDIRCHOOSER_SERVER_DOWN);
				dirState[i] = DIR_STATE_SERVER_DOWN;
			}
#ifdef ENABLE_GUI_MOUNT
			else if (retvalue == 1)//LOCALE_PING_OK
			{
				if(CFSMounter::isMounted (g_settings.network_nfs_local_dir[dirNfsMountNr[i]]) == 0)
				{
					dirOptionText[i] = g_Locale->getText(LOCALE_RECDIRCHOOSER_NOT_MOUNTED);
					dirState[i] = DIR_STATE_NOT_MOUNTED;
			   }
				else
				{
					  dirState[i] = DIR_STATE_MOUNTED;
				}
			}
#endif
		}
		else
		{
			// not a nfs dir, probably IDE? we accept this so far
			dirState[i] = DIR_STATE_MOUNTED;
		}
		if(dirState[i] == DIR_STATE_MOUNTED)
		{
			if(*(*dirList)[i].used == true)
			{
				if (statfs((*dirList)[i].name.c_str(), &s) >= 0 )
				{
					drivefree = (s.f_bfree * s.f_bsize)>>30;
					char tmp[20];
					snprintf(tmp, 19,g_Locale->getText(LOCALE_RECDIRCHOOSER_FREE),drivefree);
					tmp[19]=0;
					dirOptionText[i]=tmp;
				}
				else
				{
					dirOptionText[i]="? GB";
				}
			}
			else
			{
				dirOptionText[i] = "INAKTIV";
			}
		}
	}
}

/************************************************************************/
std::string CMovieBrowser::getMovieBrowserVersion(void)
/************************************************************************/
{	
	static CImageInfo imageinfo;
	return imageinfo.getModulVersion("","$Revision: 1.80 $");
}

/************************************************************************/
int CDirMenu::show(void)
/************************************************************************/
{
	int returnval = menu_return::RETURN_REPAINT;

	if(dirList->empty())
		return returnval;

	char tmp[20];

	CMenuWidget dirMenu(LOCALE_MOVIEBROWSER_MENU_MAIN_HEAD, NEUTRINO_ICON_STREAMING, 440);
	dirMenu.addIntroItems(LOCALE_MOVIEBROWSER_MENU_DIRECTORIES_HEAD);

	updateDirState();
	for(unsigned int i = 0; i < dirList->size() && i < MAX_DIR; i++)
	{
		sprintf(tmp,"%d",i);
		tmp[1]=0;
		CMenuForwarder* fw = new CMenuForwarder((*dirList)[i].name.c_str(), (dirState[i] != DIR_STATE_UNKNOWN), dirOptionText[i], this, tmp);
		fw->setItemButton(NEUTRINO_ICON_BUTTON_OKAY, true);
		dirMenu.addItem(fw, i == 0);
	}

	returnval = dirMenu.exec(NULL," ");
	return returnval;
}

/************************************************************************/
void CMovieBrowser::showHelpVLC()
/************************************************************************/
{
	static CMoviePlayerGui mp;
	std::string version = "Version: " + mp.getMoviePlayerVersion();
	Helpbox helpbox;
#ifdef ENABLE_MOVIEPLAYER2
	helpbox.addLine(NEUTRINO_ICON_BUTTON_RED, g_Locale->getText(LOCALE_MOVIEPLAYER_TSHELP17));
	helpbox.addLine(NEUTRINO_ICON_BUTTON_GREEN, g_Locale->getText(LOCALE_INFOVIEWER_LANGUAGES));
#else
	helpbox.addLine(NEUTRINO_ICON_BUTTON_RED, g_Locale->getText(LOCALE_MOVIEPLAYER_VLCHELP1));
	helpbox.addLine(NEUTRINO_ICON_BUTTON_GREEN, g_Locale->getText(LOCALE_MOVIEPLAYER_VLCHELP2));
#endif
	helpbox.addLine(NEUTRINO_ICON_BUTTON_YELLOW, g_Locale->getText(LOCALE_MOVIEPLAYER_VLCHELP3));
	helpbox.addLine(NEUTRINO_ICON_BUTTON_BLUE, g_Locale->getText(LOCALE_MOVIEPLAYER_VLCHELP4));
	helpbox.addLine(NEUTRINO_ICON_BUTTON_DBOX, g_Locale->getText(LOCALE_MOVIEPLAYER_VLCHELP5));
	helpbox.addLine(NEUTRINO_ICON_BUTTON_1, g_Locale->getText(LOCALE_MOVIEPLAYER_VLCHELP6));
	helpbox.addLine(NEUTRINO_ICON_BUTTON_3, g_Locale->getText(LOCALE_MOVIEPLAYER_VLCHELP7));
	helpbox.addLine(NEUTRINO_ICON_BUTTON_4, g_Locale->getText(LOCALE_MOVIEPLAYER_VLCHELP8));
	helpbox.addLine(NEUTRINO_ICON_BUTTON_6, g_Locale->getText(LOCALE_MOVIEPLAYER_VLCHELP9));
	helpbox.addLine(NEUTRINO_ICON_BUTTON_7, g_Locale->getText(LOCALE_MOVIEPLAYER_VLCHELP10));
	helpbox.addLine(NEUTRINO_ICON_BUTTON_9, g_Locale->getText(LOCALE_MOVIEPLAYER_VLCHELP11));
#ifdef ENABLE_MOVIEPLAYER2
	helpbox.addLine(NEUTRINO_ICON_BUTTON_5, g_Locale->getText(LOCALE_MOVIEPLAYER_VLCHELP13));
	helpbox.addLine(NEUTRINO_ICON_BUTTON_0, g_Locale->getText(LOCALE_MOVIEPLAYER_VLCHELP2));
	helpbox.addLine(NEUTRINO_ICON_BUTTON_RIGHT, g_Locale->getText(LOCALE_MOVIEPLAYER_TSHELP18));
	helpbox.addLine(NEUTRINO_ICON_BUTTON_LEFT, g_Locale->getText(LOCALE_MOVIEPLAYER_TSHELP19));
	helpbox.addLine(NEUTRINO_ICON_BUTTON_TOP, g_Locale->getText(LOCALE_MOVIEPLAYER_VLCHELP16));
	helpbox.addLine(NEUTRINO_ICON_BUTTON_DOWN, g_Locale->getText(LOCALE_MOVIEPLAYER_VLCHELP15));
#else
	helpbox.addLine(NEUTRINO_ICON_BUTTON_DOWN, g_Locale->getText(LOCALE_MOVIEPLAYER_VLCHELP13));
	helpbox.addLine(NEUTRINO_ICON_BUTTON_RIGHT, g_Locale->getText(LOCALE_MOVIEPLAYER_VLCHELP15));
	helpbox.addLine(NEUTRINO_ICON_BUTTON_LEFT, g_Locale->getText(LOCALE_MOVIEPLAYER_VLCHELP16));
	helpbox.addLine(NEUTRINO_ICON_BUTTON_HELP, g_Locale->getText(LOCALE_MOVIEPLAYER_VLCHELP14));
#endif
	helpbox.addLine(NEUTRINO_ICON_BUTTON_HOME, g_Locale->getText(LOCALE_MOVIEPLAYER_VLCHELP1));
	helpbox.addLine(g_Locale->getText(LOCALE_MOVIEPLAYER_VLCHELP12));
	helpbox.addLine(version);
#ifndef ENABLE_MOVIEPLAYER2
	helpbox.addLine("Movieplayer (c) 2003, 2004 by gagga");
#endif
	hide();
	helpbox.show(LOCALE_MESSAGEBOX_INFO);
}
