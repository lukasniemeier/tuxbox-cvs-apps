#ifdef ENABLE_DYN_CONF
#ifndef DISABLE_FILE

#include <map>
#include <time.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>

#include <sys/socket.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <arpa/inet.h>
#include <linux/if_ether.h>
#include <iomanip>
#include <iostream>
#include <fstream>

#include <enigma.h>
#include <enigma_main.h>
#include <enigma_standby.h>
#include <timer.h>
#include <lib/driver/eavswitch.h>
#include <lib/dvb/dvb.h>
#include <lib/dvb/edvb.h>
#include <lib/dvb/epgcache.h>
#include <lib/dvb/servicestructure.h>
#include <lib/dvb/decoder.h>
#include <lib/dvb/dvbservice.h>
#include <lib/dvb/service.h>
#include <lib/dvb/record.h>
#include <lib/dvb/serviceplaylist.h>

#include <lib/system/info.h>
#include <lib/system/http_dyn.h>
#include <lib/system/econfig.h>
#include <enigma_dyn.h>
#include <enigma_dyn_utils.h>
#include <enigma_dyn_conf.h>
#include <configfile.h>

using namespace std;

bool dreamFlashIsInstalled(void)
{
	return ((access("/var/mnt/usb/tools/lcdmenu.conf", R_OK) == 0)
		&& (access("/var/mnt/usb/tools/lcdmenu", X_OK) == 0)
		&& (access("/var/mnt/usb/tools/menu", X_OK) == 0)
		);	
}

eString getInstalledImages(void)
{
	eString result;
	eString image;
	unsigned int pos = 0;
	int i = 0;
	eString dreamFlashImages = getAttribute("/var/mnt/usb/lcdmenu.conf", "menu_items");
	eString activeImage = getAttribute("/var/mnt/usb/lcdmenu.conf", "default_entry");
	if (dreamFlashImages.length() > 0)
		dreamFlashImages = dreamFlashImages.substr(0, dreamFlashImages.length() - 1); //remove last comma
	while (dreamFlashImages.length() > 0)
	{
		if ((pos = dreamFlashImages.find(",")) != eString::npos)
		{
			image = dreamFlashImages.substr(0, pos);
			dreamFlashImages = dreamFlashImages.substr(pos + 1);
		}
		else
		{
			image = dreamFlashImages;
			dreamFlashImages = "";
		}
		result += "<tr>";
		result += "<td>";
		if (i == atoi(activeImage.c_str()))
			result += "<img src=\"on.gif\" alt=\"online\" border=0>";
		else
			result += "<img src=\"off.gif\" alt=\"offline\" border=0>";
		result += "</td>";
		result += "<td>";
		result += button(100, "Select", GREEN, "javascript:selectImage('" + eString().sprintf("%d", i) + "')");
		result += "</td>";
		result += "</tr>";
		i++;
	}
	
	return result;
}

void activateSwapFile(eString swapFile)
{
	eString cmd;
	cmd = "mkswap " + swapFile;
	system(cmd.c_str());
	cmd = "swapon " + swapFile;
	system(cmd.c_str());
}

void deactivateSwapFile(eString swapFile)
{
	eString cmd;
	cmd = "swapoff " + swapFile;
	system(cmd.c_str());
}

void setSwapFile(int nextswapfile, eString nextswapfilename)
{
	eConfig::getInstance()->setKey("/extras/swapfile", nextswapfile);
	if (nextswapfile == 1)
	{
		eConfig::getInstance()->setKey("/extras/swapfilename", nextswapfilename.c_str());
		activateSwapFile(nextswapfilename);
	}
	else
		deactivateSwapFile(nextswapfilename);
}

eString setConfigSwapFile(eString request, eString dirpath, eString opts, eHTTPConnection *content)
{
	std::map<eString, eString> opt = getRequestOptions(opts, '&');
	eString swap = opt["swap"];
	eString swapFile = opt["swapfile"];

	setSwapFile((swap == "on") ? 1 : 0, swapFile);

	return closeWindow(content, "", 500);
}

eString setConfigMultiBoot(eString request, eString dirpath, eString opts, eHTTPConnection *content)
{
	std::map<eString, eString> opt = getRequestOptions(opts, '&');
	eString imageNumber = opt["image"];
	
	CConfigFile *config = new CConfigFile(',');
	if (config->loadConfig("/var/mnt/usb/tools/lcdmenu.conf"))
	{
		config->setString("default_entry", imageNumber);
		config->setModifiedFlag(true);
		config->saveConfig("/var/mnt/usb/tools/lcdmenu.conf");

		delete(config);
	}

	return closeWindow(content, "", 500);
}

void ezapConfInitializeDyn(eHTTPDynPathResolver *dyn_resolver, bool lockWeb)
{
	dyn_resolver->addDyn("GET", "/cgi-bin/setConfigSwapFile", setConfigSwapFile, lockWeb);
	dyn_resolver->addDyn("GET", "/cgi-bin/setConfigMultiBoot", setConfigMultiBoot, lockWeb);
}
#endif
#endif


