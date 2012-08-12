#ifndef __setup_extra_h
#define __setup_extra_h

#include <setup_window.h>

class eOSDExpertSetup: public eSetupWindow
{
	eListBoxEntryMulti *timeout_infobar;
	eListBoxEntryMulti *timeout_volumebar;
	eListBoxEntryMulti *timeout_keypressed;

	void colorbuttonsChanged(bool);
	void reinitializeHTTPServer(bool);
	void fastZappingChanged(bool b);
	void init_eOSDExpertSetup();
	void fileToggle(bool newState, const char* filename);
	void selInfobarChanged(eListBoxEntryMenu* e);
	void selVolumebarChanged(eListBoxEntryMenu* e);
	void selChannelKeypressedInitDelayChanged(eListBoxEntryMenu* e);
public:
	eOSDExpertSetup();
};

#endif
