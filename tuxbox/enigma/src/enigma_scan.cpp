/*
 * enigma_scan.cpp
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
 * $Id: enigma_scan.cpp,v 1.9 2002/10/15 23:31:29 Ghostrider Exp $
 */

#include <enigma_scan.h>

#include <satconfig.h>
#include <scan.h>
#include <lib/dvb/edvb.h>
#include <lib/dvb/frontend.h>
#include <lib/gui/ewindow.h>
#include <lib/gui/eskin.h>
#include <lib/gui/elabel.h>
#include <lib/base/i18n.h>

eZapScan::eZapScan()
	:eListBoxWindow<eListBoxEntryMenu>(_("Channels"), 4, 220)
{
	move(ePoint(150, 136));
	CONNECT((new eListBoxEntryMenu(&list, _("[back]")))->selected, eZapScan::sel_close);
	CONNECT((new eListBoxEntryMenu(&list, _("Transponder scan")))->selected, eZapScan::sel_scan);	
	if ( eFrontend::getInstance()->Type() == eFrontend::feSatellite )  // only when a sat box is avail we shows a satellite config
		CONNECT((new eListBoxEntryMenu(&list, _("Satellites...")))->selected, eZapScan::sel_satconfig);	
	CONNECT((new eListBoxEntryMenu(&list, _("Bouquets...")))->selected, eZapScan::sel_bouquet);	
}

eZapScan::~eZapScan()
{
}

void eZapScan::sel_close()
{
	close(0);
}

void eZapScan::sel_scan()
{
	TransponderScan setup(LCDTitle, LCDElement);
	hide();
	setup.exec();
	show();
}

void eZapScan::sel_bouquet()
{
	eDVB::getInstance()->settings->sortInChannels();
}

void eZapScan::sel_satconfig()
{
	hide();
	eSatelliteConfigurationManager satconfig;
	satconfig.setLCD(LCDTitle, LCDElement);
	satconfig.show();
	satconfig.exec();
	satconfig.hide();
	show();
}
