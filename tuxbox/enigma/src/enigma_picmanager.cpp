#include <lib/base/i18n.h>
#include <lib/gui/elabel.h>
#include <lib/gui/enumber.h>
#include <lib/gui/ebutton.h>
#include <lib/gui/echeckbox.h>
#include <lib/gui/eskin.h>
#include <lib/system/econfig.h>
#include <lib/gui/numberactions.h>
#include <lib/system/info.h>
#include <lib/system/init.h>
#include <lib/system/init_num.h>
#include "enigma_picmanager.h"

ePicViewerSettings::ePicViewerSettings():eWindow(0)
{
	int slideshowtimeout = 5;
	eConfig::getInstance()->getKey("/picviewer/slideshowtimeout", slideshowtimeout);
	int sortpictures = 1;
	eConfig::getInstance()->getKey("/picviewer/sortpictures", sortpictures);
	int wraparound = 1;
	eConfig::getInstance()->getKey("/picviewer/wraparound", wraparound);
	int startwithselectedpic = 0;
	eConfig::getInstance()->getKey("/picviewer/startwithselectedpic", startwithselectedpic);
	int includesubdirs = 0;
	eConfig::getInstance()->getKey("/picviewer/includesubdirs", includesubdirs);
	int showbusysign = 0;
	eConfig::getInstance()->getKey("/picviewer/showbusysign", showbusysign);
	int format169 = 0;
	eConfig::getInstance()->getKey("/picviewer/format169", format169);

	int fd = eSkin::getActive()->queryValue("fontsize", 20);

	setText(_("Slide Viewer Settings"));
	cmove(ePoint(100, 80));

	int y = 10, dy = 35, h = fd + 6;


	eLabel *l = new eLabel(this);
	l->setText(_("Timeout"));
	l->move(ePoint(10, y));
	l->resize(eSize(100, h));

	timeout = new eListBox<eListBoxEntryText>(this, l);
	timeout->loadDeco();
	timeout->setFlags(eListBox<eListBoxEntryText>::flagNoUpDownMovement);
	timeout->move(ePoint(110, y));
	timeout->resize(eSize(35, 34));
	eListBoxEntryText* entries[30];
	for (int i = 0; i < 30; i++)
	{
		eString num = eString().sprintf("%d", i + 1);
		entries[i] = new eListBoxEntryText(timeout, num.c_str(), (void *)new eString(num.c_str()));
	}
	timeout->setCurrent(entries[slideshowtimeout - 1]);
	timeout->setHelpText(_("Select slideshow timeout (left, right)"));

	y += dy;

	sort = new eCheckbox(this, sortpictures, 1);
	sort->setText(_("Sort slides"));
	sort->move(ePoint(10, y));
	sort->resize(eSize(300, h));
	sort->setHelpText(_("Sort slides alphabetically"));

	y += dy;

	wrap = new eCheckbox(this, wraparound, 1);
	wrap->setText(_("Wrap around"));
	wrap->move(ePoint(10, y));
	wrap->resize(eSize(300, h));
	wrap->setHelpText(_("Wrap around at end of slideshow"));

	y += dy;

	start = new eCheckbox(this, startwithselectedpic, 1);
	start->setText(_("Start with selected slide"));
	start->move(ePoint(10, y));
	start->resize(eSize(300, h));
	start->setHelpText(_("Start slideshow with selected slide"));

	y += dy;

	subdirs = new eCheckbox(this, includesubdirs, 1);
	subdirs->setText(_("Include subdirectories"));
	subdirs->move(ePoint(10, y));
	subdirs->resize(eSize(300, h));
	subdirs->setHelpText(_("Include slides of subdirectories in slideshow"));

	y += dy;

	busy = new eCheckbox(this, showbusysign, 1);
	busy->setText(_("Show busy sign"));
	busy->move(ePoint(10, y));
	busy->resize(eSize(300, h));
	busy->setHelpText(_("Show busy sign while preprocessing slide"));

	y += dy;

	format_169 = new eCheckbox(this, format169, 1);
	format_169->setText(_("Show in 16:9 format"));
	format_169->move(ePoint(10, y));
	format_169->resize(eSize(300, h));
	format_169->setHelpText(_("Always show slides in 16:9 format"));

	y += dy + 20;

	ok = new eButton(this);
	ok->setText(_("save"));
	ok->setShortcut("green");
	ok->setShortcutPixmap("green");
	ok->move(ePoint(10, y));
	ok->resize(eSize(130, h));
	ok->setHelpText(_("Close window and save entries"));
	ok->loadDeco();
	CONNECT(ok->selected, ePicViewerSettings::okPressed);

	abort = new eButton(this);
	abort->loadDeco();
	abort->setText(_("abort"));
	abort->setShortcut("red");
	abort->setShortcutPixmap("red");
	abort->move(ePoint(130 + 10 + 10, y));
	abort->resize(eSize(130, h));
	abort->setHelpText(_("Close window (no changes are saved)"));
	CONNECT(abort->selected, ePicViewerSettings::abortPressed);

	y = y + 40 + 2 * h;
	cresize(eSize(350, y));

	statusbar = new eStatusBar(this);
	statusbar->move(ePoint(0, clientrect.height() - 2 * h));
	statusbar->resize(eSize( clientrect.width(), 2 * h));
	statusbar->loadDeco();
}

ePicViewerSettings::~ePicViewerSettings()
{
}

void ePicViewerSettings::fieldSelected(int *number)
{
	focusNext(eWidget::focusDirNext);
}

void ePicViewerSettings::okPressed()
{
	eConfig::getInstance()->setKey("/picviewer/slideshowtimeout", atoi(((eString *)timeout->getCurrent()->getKey())->c_str()));
	eConfig::getInstance()->setKey("/picviewer/sortpictures", (int)sort->isChecked());
	eConfig::getInstance()->setKey("/picviewer/wraparound", (int)wrap->isChecked());
	eConfig::getInstance()->setKey("/picviewer/startwithselectedpic", (int)start->isChecked());
	eConfig::getInstance()->setKey("/picviewer/includesubdirs", (int)subdirs->isChecked());
	eConfig::getInstance()->setKey("/picviewer/showbusysign", (int)busy->isChecked());
	eConfig::getInstance()->setKey("/picviewer/format169", (int)format_169->isChecked());

	close(1);
}

void ePicViewerSettings::abortPressed()
{
	close(0);
}

