/*
	$Id: setting_helpers.cpp,v 1.197 2012/03/18 11:20:15 rhabarber1848 Exp $

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

#include <system/setting_helpers.h>
#include <system/configure_network.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#include "libnet.h"

#include <sstream>
#include <iostream>
#include <fstream>
#include <iomanip>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <linux/if.h>
#include <dirent.h>
#include <errno.h>

#include <libucodes.h>

#include <config.h>

#include <global.h>
#include <neutrino.h>
#include <gui/widget/messagebox.h>

extern "C" int pinghost( const char *hostname );

#ifndef TUXTXT_CFG_STANDALONE
extern "C" int  tuxtxt_stop();
extern "C" void tuxtxt_close();
extern "C" int  tuxtxt_init();
extern "C" int  tuxtxt_start(int tpid);
#endif

#define PROCDIR "/proc"

COnOffNotifier::COnOffNotifier( CMenuItem* a1,CMenuItem* a2,CMenuItem* a3,CMenuItem* a4,CMenuItem* a5)
{
	number = 0;
	if(a1 != NULL){ toDisable[0] =a1;number++;};
	if(a2 != NULL){ toDisable[1] =a2;number++;};
	if(a3 != NULL){ toDisable[2] =a3;number++;};
	if(a4 != NULL){ toDisable[3] =a4;number++;};
	if(a5 != NULL){ toDisable[4] =a5;number++;};
}

bool COnOffNotifier::changeNotify(const neutrino_locale_t, void *Data)
{
	if(*(int*)(Data) == 0)
	{
		for (int i=0; i<number ; i++)
			toDisable[i]->setActive(false);
	}
	else
	{
		for (int i=0; i<number ; i++)
			toDisable[i]->setActive(true);
	}
	return true;
}

bool CPauseSectionsdNotifier::changeNotify(const neutrino_locale_t, void * Data)
{
	g_Sectionsd->setPauseScanning((*((int *)Data)) == 0);

	return true;
}

#ifndef TUXTXT_CFG_STANDALONE
bool CTuxtxtCacheNotifier::changeNotify(const neutrino_locale_t, void *)
{
	int vtpid=g_RemoteControl->current_PIDs.PIDs.vtxtpid;

	if (g_settings.tuxtxt_cache)
	{
		tuxtxt_init();
		if (vtpid)
			tuxtxt_start(vtpid);
	}
	else
	{
		tuxtxt_stop();
		tuxtxt_close();
	}

	return true;
}
#endif

bool CSectionsdConfigNotifier::changeNotify(const neutrino_locale_t, void *)
{
	CNeutrinoApp::getInstance()->SendSectionsdConfig();
	return true;
}

int CNVODChangeExec::exec(CMenuTarget* parent, const std::string & actionKey)
{
	//    printf("CNVODChangeExec exec: %s\n", actionKey.c_str());
	unsigned sel= atoi(actionKey.c_str());
	g_RemoteControl->setSubChannel(sel);

	parent->hide();
	g_InfoViewer->showSubchan();
	return menu_return::RETURN_EXIT;
}

int CStreamFeaturesChangeExec::exec(CMenuTarget* parent, const std::string & actionKey)
{
	//printf("CStreamFeaturesChangeExec exec: %s\n", actionKey.c_str());
	int sel= atoi(actionKey.c_str());

	if(parent != NULL)
		parent->hide();
	// -- obsolete (rasc 2004-06-10)
	// if (sel==-1)
	// {
	// 	CStreamInfo StreamInfo;
	//	StreamInfo.exec(NULL, "");
	// } else
	if (sel>=0)
	{
		CLCD::getInstance()->setMode(CLCD::MODE_TVRADIO);
		g_PluginList->startPlugin(sel);
	}

	return menu_return::RETURN_EXIT;
}

#ifdef HAVE_DBOX_HARDWARE
int CUCodeCheckExec::exec(CMenuTarget*, const std::string &)
{
	std::stringstream text;
	char res[60];

	switch (g_info.avia_chip)
	{
		case CControld::TUXBOX_AVIACHIP_AVIA500:
			text << g_Locale->getText(LOCALE_UCODECHECK_AVIA500) << ": ";
			checkFile(UCODEDIR "/avia500.ux", (char*) &res);
			text << res << "\n";
			break;
		case CControld::TUXBOX_AVIACHIP_AVIA600:
			text << g_Locale->getText(LOCALE_UCODECHECK_AVIA600) << ": ";
			checkFile(UCODEDIR "/avia600.ux", (char*) &res);
			text << res << "\n";
			break;
	}
	text << g_Locale->getText(LOCALE_UCODECHECK_UCODE) << ": ";
	checkFile(UCODEDIR "/ucode.bin", (char*) &res);
	if (strcmp("not found", res) == 0)
		text << "ucode_0014 (built-in)";
	else
		text << res;
	text << "\n" << g_Locale->getText(LOCALE_UCODECHECK_CAM_ALPHA) << ": ";
	checkFile(UCODEDIR "/cam-alpha.bin", (char*) &res);
	text << res;

	ShowMsgUTF(LOCALE_UCODECHECK_HEAD, text.str(), CMessageBox::mbrBack, CMessageBox::mbBack); // UTF-8
	return 1;
}
#endif

int CDVBInfoExec::exec(CMenuTarget*, const std::string &)
{
	std::stringstream text;

//	text<<std::hex<<std::setfill('0')<<std::setw(2)<<(int)addr[i]<<':';
	text << g_Locale->getText(LOCALE_TIMERLIST_MODETV) << ": " << CNeutrinoApp::getInstance()->channelListTV->getSize() << "\n";
	text << g_Locale->getText(LOCALE_TIMERLIST_MODERADIO) << ": " << CNeutrinoApp::getInstance()->channelListRADIO->getSize() << "\n \n";
	text << g_Locale->getText(LOCALE_SERVICEMENU_CHAN_EPG_STAT_EPG_STAT) << ":\n" << g_Sectionsd->getStatusinformation() << "\n";

	ShowMsgUTF(LOCALE_SERVICEMENU_CHAN_EPG_STAT, text.str(), CMessageBox::mbrBack, CMessageBox::mbBack); // UTF-8
	return 1;
}

long CNetAdapter::mac_addr_sys ( u_char *addr) //only for function getMacAddr()
{
	struct ifreq ifr;
	struct ifreq *IFR;
	struct ifconf ifc;
	char buf[1024];
	int s, i;
	int ok = 0;
	s = socket(AF_INET, SOCK_DGRAM, 0);
	if (s==-1) 
	{
		return -1;
	}

	ifc.ifc_len = sizeof(buf);
	ifc.ifc_buf = buf;
	ioctl(s, SIOCGIFCONF, &ifc);
	IFR = ifc.ifc_req;
	for (i = ifc.ifc_len / sizeof(struct ifreq); --i >= 0; IFR++)
	{
		strcpy(ifr.ifr_name, IFR->ifr_name);
		if (ioctl(s, SIOCGIFFLAGS, &ifr) == 0) 
		{
			if (! (ifr.ifr_flags & IFF_LOOPBACK)) 
			{
				if (ioctl(s, SIOCGIFHWADDR, &ifr) == 0) 
				{
					ok = 1;
					break;
				}
			}
		}
	}
	close(s);
	if (ok)
	{
		memmove(addr, ifr.ifr_hwaddr.sa_data, 6);
	}
	else
	{
		return -1;
	}
	return 0;
}

std::string CNetAdapter::getMacAddr(void)
{
	long stat;
	u_char addr[6];
	stat = mac_addr_sys( addr);
	if (0 == stat)
	{
		std::stringstream mac_tmp;
		for(int i=0;i<6;++i)
		mac_tmp<<std::hex<<std::setfill('0')<<std::setw(2)<<(int)addr[i]<<':';
		return mac_tmp.str().substr(0,17);
	}
	else
	{
		return "not found";
	}
}

const char * mypinghost(const char * const host)
{
	int retvalue = pinghost(host);
	switch (retvalue)
	{
		case 1: return (g_Locale->getText(LOCALE_PING_OK));
		case 0: return (g_Locale->getText(LOCALE_PING_UNREACHABLE));
		case -1: return (g_Locale->getText(LOCALE_PING_PROTOCOL));
		case -2: return (g_Locale->getText(LOCALE_PING_SOCKET));
	}
	return "";
}

void testNetworkSettings(const char* ip, const char* netmask, const char* broadcast, const char* gateway, const char* nameserver, bool ip_static)
{
	char our_ip[16];
	char our_mask[16];
	char our_broadcast[16];
	char our_gateway[16];
	char our_nameserver[16];
	std::string text, ethID, testsite;
	//set default testdomain and wiki-IP
	std::string defaultsite = "www.google.de", wiki_IP = "91.224.67.93";
	
	//set physical adress
	static CNetAdapter netadapter; ethID=netadapter.getMacAddr();
	
	//get www-domain testsite from /.version 	
	CConfigFile config('\t');
	config.loadConfig("/.version");
	testsite = config.getString("homepage",defaultsite);	
	testsite.replace( 0, testsite.find("www",0), "" );
	
	//use default testdomain if testsite missing
	if (testsite.length()==0) testsite = defaultsite; 

	if (ip_static) {
		strcpy(our_ip, ip);
		strcpy(our_mask, netmask);
		strcpy(our_broadcast, broadcast);
		strcpy(our_gateway, gateway);
		strcpy(our_nameserver, nameserver);
	}
	else {
		netGetIP("eth0", our_ip, our_mask, our_broadcast);
		netGetDefaultRoute(our_gateway);
		netGetNameserver(our_nameserver);
	}
	
	printf("testNw IP: %s\n", our_ip);
	printf("testNw MAC-address: %s\n", ethID.c_str());
	printf("testNw Netmask: %s\n", our_mask);
	printf("testNw Broadcast: %s\n", our_broadcast);
	printf("testNw Gateway: %s\n", our_gateway);
	printf("testNw Nameserver: %s\n", our_nameserver);
	printf("testNw Testsite %s\n", testsite.c_str());
 
	text = (std::string)"dbox:\n"
	     + "    " + our_ip + ": " + mypinghost(our_ip) + '\n'
		 + "    " + "eth-ID: " + ethID + '\n'
	     + g_Locale->getText(LOCALE_NETWORKMENU_GATEWAY) + ":\n"
	     + "    " + our_gateway + ": " + ' ' + mypinghost(our_gateway) + '\n'
	     + g_Locale->getText(LOCALE_NETWORKMENU_NAMESERVER) + ":\n"
	     + "    " + our_nameserver + ": " + ' ' + mypinghost(our_nameserver) + '\n'
	     + "wiki.tuxbox.org:\n"
	     + "    via IP (" + wiki_IP + "): " + mypinghost(wiki_IP.c_str()) + '\n';
	if (1 == pinghost(our_nameserver)) text += (std::string)
	       "    via DNS: " + mypinghost("wiki.tuxbox.org") + '\n'
	     + testsite + ":\n"
	     + "    via DNS: " + mypinghost(testsite.c_str()) + '\n';

	ShowMsgUTF(LOCALE_NETWORKMENU_TEST, text, CMessageBox::mbrBack, CMessageBox::mbBack); // UTF-8
}

void showCurrentNetworkSettings()
{
	char ip[16];
	char mask[16];
	char broadcast[16];
	char router[16];
	char nameserver[16];
	std::string text;
	
	netGetIP("eth0", ip, mask, broadcast);
	if (ip[0] == 0) {
		text = g_Locale->getText(LOCALE_NETWORKMENU_INACTIVE);
	}
	else {
		netGetNameserver(nameserver);
		netGetDefaultRoute(router);
		CNetworkConfig  networkConfig;
		std::string dhcp = networkConfig.inet_static ? g_Locale->getText(LOCALE_OPTIONS_OFF) : g_Locale->getText(LOCALE_OPTIONS_ON);

		text = (std::string)g_Locale->getText(LOCALE_NETWORKMENU_DHCP) + ": " + dhcp + '\n'
				  + g_Locale->getText(LOCALE_NETWORKMENU_IPADDRESS ) + ": " + ip + '\n'
				  + g_Locale->getText(LOCALE_NETWORKMENU_NETMASK   ) + ": " + mask + '\n'
				  + g_Locale->getText(LOCALE_NETWORKMENU_BROADCAST ) + ": " + broadcast + '\n'
				  + g_Locale->getText(LOCALE_NETWORKMENU_NAMESERVER) + ": " + nameserver + '\n'
				  + g_Locale->getText(LOCALE_NETWORKMENU_GATEWAY   ) + ": " + router;
	}
	ShowMsgUTF(LOCALE_NETWORKMENU_SHOW, text, CMessageBox::mbrBack, CMessageBox::mbBack); // UTF-8
}

unsigned long long getcurrenttime()
{
	struct timeval tv;
	gettimeofday( &tv, NULL );
	return (unsigned long long) tv.tv_usec + (unsigned long long)((unsigned long long) tv.tv_sec * (unsigned long long) 1000000);
}

//helper: returns a selectable tab entry from file 
std::string getFileEntryString(const char* filename, const std::string& filter_entry, const int& column_num)
{
	std::string ret = "";
	char line[256];
	std::ifstream in (filename, std::ios::in);

	if (!in) 
	{
		std::cerr<<__FUNCTION__ <<": error while open "<<filename<<" "<< strerror(errno)<<std::endl;
		return ret;
	}

	while (in.getline (line, 256))
	{
		std::string tab_line = (std::string)line, str_res;
		std::string::size_type loc = tab_line.find( filter_entry, 0 );

		if ( loc != std::string::npos ) 
		{
			std::stringstream stream(tab_line);

			for(int i = 0; i <= 10; i++)
			{
				stream >> str_res;
				if (i==column_num) 
				{
					ret = str_res;
					in.close();
					return ret;
				}

			}
		}
	}
	in.close();
	return ret;
}

//helper, returns pid of process name as string, if no pid found returns an empty string
std::string getPidof(const std::string& process_name)
{
	std::string ret = "";
	std::string p_filter = process_name;
	std::string p_name;
	DIR *dir;
	struct dirent *entry;
	
	dir = opendir(PROCDIR);
	if (dir)
	{
		do
		{
			entry = readdir(dir);
			if (entry)
			{
				std::string dir_x = entry->d_name;

				char filename[255];
				sprintf(filename,"%s/%s/status", PROCDIR, dir_x.c_str());

				if(access(filename, R_OK) ==0)
				{
					p_name = getFileEntryString(filename, "Name:", 2);
					if (p_name == p_filter)
					{
						closedir(dir);
						return dir_x;
					}
				}
			}
		}
        	while (entry);
	}
	closedir(dir);

	return ret;
}

//returns interface
std::string getInterface()
{
	char ret[19];
	char ip[3][16];
	char our_ip[3][16];

	CNetworkConfig  *network = CNetworkConfig::getInstance();
	
	if (network->inet_static)
	{
		sprintf(ip[0], "%s", network->address.c_str());
		strcpy(our_ip[0], ip[0]);
	}
	else 	//Note: netGetIP returns also mask and broadcast, but not needed here 
		netGetIP("eth0", our_ip[0]/*IP*/, our_ip[1]/*MASK*/, our_ip[2]/*BROADCAST*/);

	sprintf(ret, "%s/24", our_ip[0]);
	
	return ret;
}

