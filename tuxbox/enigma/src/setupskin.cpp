/*
 * enigma_setup.cpp
 *
 * Copyright (C) 2002 Felix Domke <tmbinc@tuxbox.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 *
 * $Id: setupskin.cpp,v 1.4 2002/06/23 15:51:24 Ghostrider Exp $
 */

#include "setupskin.h"

#include <dirent.h>
#include "config.h"

#include <core/gui/ebutton.h>
#include <core/gui/emessage.h>
#include <core/system/econfig.h>

extern eString getInfo(const char *file, const char *info);

void eSkinSetup::loadSkins()
{
	const char *skinPath = DATADIR "/enigma/skins/";
	struct dirent **namelist;
	char *current_skin="";
	eConfig::getInstance()->getKey("/ezap/ui/skin", current_skin);

	int n = scandir(skinPath, &namelist, 0, alphasort);

	if (n<0)
	{
		eDebug("error reading skin directory");
		eMessageBox msg("error reading skin directory", "error");
		msg.show();
		msg.exec();
		msg.hide();
		return;
	}
	for(int count=0;count<n;count++)
	{
		eString	fileName=eString(skinPath) + eString(namelist[count]->d_name);

		if (fileName.find(".info") != -1)
		{
			eString esml=getInfo(fileName.c_str(), "esml");
			eString name=getInfo(fileName.c_str(), "name");
			if (esml.size() && name.size())
			{
				eListBoxEntrySkin *s=new eListBoxEntrySkin(lskins, name, esml);
				if (esml == current_skin)
				{
					eDebug("got current");
					lskins->setCurrent(s);
				}
			}
		}
		free(namelist[count]);
  }
  free(namelist);
}

void eSkinSetup::accept()
{
	skinSelected(lskins->getCurrent());
}

void eSkinSetup::skinSelected(eListBoxEntrySkin *skin)
{
	if (!skin)
	{
		close(1);
		return;
	}

	eConfig::getInstance()->setKey("/ezap/ui/skin", skin->getESML().c_str());

	close(0);
}

eSkinSetup::eSkinSetup()
{
	baccept=new eButton(this);
	baccept->setName("accept");
	breject=new eButton(this);
	breject->setName("reject");
	lskins=new eListBox<eListBoxEntrySkin>(this, eSkin::getActive()->queryValue("fontsize", 20));
	lskins->setName("skins");
	CONNECT(baccept->selected, eSkinSetup::accept);
	CONNECT(breject->selected, eSkinSetup::reject);
	CONNECT(lskins->selected, eSkinSetup::skinSelected);
	
	setFocus(lskins);

	eSkin *skin=eSkin::getActive();
	if (skin->build(this, "setup.skins"))
		eFatal("skin load of \"setup.skins\" failed");

	loadSkins();
}

eSkinSetup::~eSkinSetup()
{
}

int eSkinSetup::keyDown(int rc)
{
	switch(rc)
	{
	case eRCInput::RC_RIGHT:
		focusNext(eWidget::focusDirE);
		return 1;
	case eRCInput::RC_DOWN:
		focusNext(eWidget::focusDirS);
		return 1;
	case eRCInput::RC_LEFT:
		focusNext(eWidget::focusDirW);
		return 1;
	case eRCInput::RC_UP:
		focusNext(eWidget::focusDirN);
		return 1;
	}
}
