#include "epgwindow.h"
#include "enigma_event.h"

#include <algorithm>
#include <iomanip>

#include <core/driver/rc.h>
#include <core/gui/eskin.h>
#include <core/dvb/edvb.h>


eListBoxEntryEPG::eListBoxEntryEPG(const eit_event_struct* evt, eListBox<eListBoxEntryEPG> *listbox)
		:eListBoxEntryTextStream((eListBox<eListBoxEntryTextStream>*)listbox), event(evt)	
{	
		for (ePtrList<Descriptor>::iterator d(event.descriptor); d != event.descriptor.end(); ++d)
		{
			Descriptor *descriptor=*d;
			if (descriptor->Tag()==DESCR_SHORT_EVENT)
			{
				tm* t = localtime(&event.start_time);
 		
				text 	<< std::setw(2) << t->tm_mday << '.' << t->tm_mon+1 << ", "
							<< std::setw(2) << std::setfill('0') << t->tm_hour << ':' << std::setw(2) << t->tm_min << "  "
 							<< ((ShortEventDescriptor*)descriptor)->event_name;

				return;
			}
		}
		text << "no_name";
}

void eEPGWindow::fillEPGList()
{
	setText(eString("EPG - ")+current->service_name);
	eDebug("get EventMap for onid: %02x, sid: %02x\n", current->original_network_id, current->service_id);
	const eventMap* evt = eEPGCache::getInstance()->getEventMap(current->original_network_id, current->service_id);
	eventMap::const_iterator It;
	for (It = evt->begin(); It != evt->end(); It++)
		new eListBoxEntryEPG(*It->second, &list);
}

void eEPGWindow::entrySelected(eListBoxEntryEPG *entry)
{
	if (!entry)
		close(0);
	else
	{	
		int ret;
		hide();
		eEventDisplay ei(eDVB::getInstance()->service->service_name.c_str(), 0, &entry->event);
		ei.show();
		while(ret = ei.exec())
		{
			eListBoxEntryEPG* tmp;

			if (ret==-1)
				tmp=list.goPrev();
			else if (ret == 1)
				tmp=list.goNext();

			if (tmp)
				ei.setEvent(&tmp->event);					
		}
		ei.hide();
		show();
	}
}

eEPGWindow::eEPGWindow(eService* service)
:current(service),eListBoxWindow<eListBoxEntryEPG>("Select Service...", 16, eSkin::getActive()->queryValue("fontsize", 20), 600)
{
	move(ePoint(50, 50));
	list.setActiveColor(eSkin::getActive()->queryScheme("eServiceSelector.highlight"));
	CONNECT(list.selected, eEPGWindow::entrySelected);
	fillEPGList();
}
