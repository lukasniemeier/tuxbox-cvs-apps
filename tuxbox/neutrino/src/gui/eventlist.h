#ifndef EVENTLIST_HPP
#define EVENTLIST_HPP
//
// $Id: eventlist.h,v 1.4 2001/09/18 20:20:26 field Exp $
//
// $Log: eventlist.h,v $
// Revision 1.4  2001/09/18 20:20:26  field
// Eventlist in den Infov. verschoben (gelber Knopf), Infov.-Anzeige auf Knoepfe
// vorbereitet
//
// Revision 1.3  2001/09/18 14:58:20  field
// Eventlist verbessert
//
// Revision 1.2  2001/09/18 11:34:42  fnbrd
// Some changes.
//
// Revision 1.1  2001/09/18 10:50:30  fnbrd
// Eventlist, quick'n dirty
//
//

#include "../driver/framebuffer.h"
#include "../driver/fontrenderer.h"
#include "../driver/rcinput.h"
#include "../daemonc/remotecontrol.h"
#include "../helpers/infoviewer.h"
#include "../helpers/settings.h"
#include "menue.h"
#include "color.h"

#include <string>
#include <vector>

using namespace std;

class EventList
{
  private:
    struct event {
//      long long id;
      string datetimeduration;
      string name;
//      time_t starttime;
//      unsigned lengthInSeconds;
    };
    void removeAllEvents(void);
    void readEvents(const std::string& channelname); // I really don't like handling names
    unsigned int		selected;
    int                 current_event;
    unsigned int		liststart;
    unsigned int		listmaxshow;
    unsigned int		numwidth;
    int			fheight; // Fonthoehe Channellist-Inhalt
    int			theight; // Fonthoehe Channellist-Titel

    int			key;
    string			name;
    vector<event*>	evtlist;

    int 			width;
    int 			height;
    int 			x;
    int 			y;

    void paintItem(int pos);
    void paint();
    void paintHead();
    void hide();

  public:
    EventList();
    ~EventList();
    void exec(const string& channelname);
};


#endif // EVENTLIST_HPP








