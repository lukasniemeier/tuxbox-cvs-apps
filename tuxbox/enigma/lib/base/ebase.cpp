#include "ebase.h"
#include <pthread.h>
#include <fcntl.h>
#include <unistd.h>

void *eThread::wrapper(void *ptr)
{
	((eThread*)ptr)->thread();
	pthread_exit(0);
}

eThread::eThread()
{
	thread_id=pthread_create(&the_thread, 0, wrapper, this);
}

eThread::~eThread()
{
	printf("waiting for thread shutdown");
	pthread_join(thread_id, 0);
}

eSocketNotifier::eSocketNotifier(eMainloop *context, int fd, int requested): context(*context), requested(requested)
{
	state=0;
}

eSocketNotifier::~eSocketNotifier()
{
	stop();
}

void eSocketNotifier::start()
{
	if (state)
		stop();
	context.addSocketNotifier(this);
	state=1;
}

void eSocketNotifier::stop()
{
	if (state)
		context.removeSocketNotifier(this);
	state=0;
}

					// timer
void eTimer::start(long msek, bool singleShot)
{
	if (!bActive)
	{	
		bActive = true;
		bSingleShot = singleShot;
		interval = msek;
  	gettimeofday(&nextActivation, 0);		

		nextActivation += msek;
		context.addTimer(this);
	}
}

void eTimer::stop()
{
	if (bActive)
	{
		bActive=false;
		context.removeTimer(this);
	}
}

void eTimer::changeInterval(long msek)
{
	if (bActive)
		context.removeTimer(this);	

	nextActivation -= interval;
	interval = msek;
	nextActivation += interval;

	if (bActive)
		context.addTimer(this);
}

void eTimer::activate()   // Internal Funktion... called from eApplication
{
	/*emit*/ timeout();
	printf("Timer emitted\n");
	context.removeTimer(this);
	if (!bSingleShot)
	{
		nextActivation += interval;
		context.addTimer(this);
	}
	else
		bActive=false;
}
						// mainloop

void eMainloop::addSocketNotifier(eSocketNotifier *sn)
{
	notifiers.insert(std::pair<int,eSocketNotifier*> (sn->getFD(), sn));
}

void eMainloop::removeSocketNotifier(eSocketNotifier *sn)
{
	notifiers.erase(sn->getFD());
}

void eMainloop::processOneEvent()
{
	std::vector<pollfd> pfd;
	printf("wir haben FDs:");
//	pfd.clear();
	for (std::map<int,eSocketNotifier*>::iterator i(notifiers.begin()); i!=notifiers.end(); ++i)
	{
		pollfd p;
		p.fd=i->first;
		p.events=i->second->getRequested();
		p.revents=0;
		
		pfd.push_back(p);
		printf("%d (%x)", i->first, i->second->getRequested());
	}

			// process pending timers...
	long usec;

	while (!TimerList.empty() && (usec = timeout_usec( (*TimerList.begin())->getNextActivation() ) ) <= 0 )
		(*TimerList.begin())->activate();

	if (!TimerList.empty())
		printf("Next Timer in %d\n", usec);
	else
		usec=-1;

	int ret=poll(&(*pfd.begin()), pfd.size(), usec);
	if (ret>0)
	{
		printf("bin aussem poll raus und da war was");
		for (std::vector<pollfd>::iterator i(pfd.begin()); i != pfd.end(); ++i)
		{
			if (i->revents)
			{
				notifiers[i->fd]->activate(i->revents);
				if (!--ret)		// shortcut
					break;
			}
		}
	} else if (ret<0)
	{
		printf("poll made error");
	}
		// das ist noch doof hier
	while (!TimerList.empty() && (timeout_usec( (*TimerList.begin())->getNextActivation() ) ) <= 0 )
		(*TimerList.begin())->activate();
}

int eMainloop::exec()
{
		if (!loop_level)
		{
			app_quit_now = false;
			enter_loop();
		}
}

void eMainloop::enter_loop()
{
	loop_level++;

	// Status der vorhandenen Loop merken
	bool old_exit_loop = app_exit_loop;
	
	app_exit_loop = false;

	while (!app_exit_loop && !app_quit_now)
	{
		processOneEvent();
	}	

	// wiederherstellen der vorherigen app_exit_loop
	app_exit_loop = old_exit_loop;

	loop_level--;

	if (!loop_level)
	{
			// do something here on exit the last loop
	}
}

void eMainloop::exit_loop()  // call this to leave the current loop
{
	app_exit_loop = true;	
}

void eMainloop::quit()   // call this to leave all loops
{
	app_quit_now = true;
}

int eMessagePump::send(void *data, int len)
{
	return ::write(fd[1], data, len)<0;
}

int eMessagePump::recv(void *data, int len)
{
	unsigned char*dst=(unsigned char*)data;
	while (len)
	{
		int r=::read(fd[0], dst, len);
		if (r<0)
			return r;
		dst+=r;
		len-=r;
	}
	return 0;
}

eApplication* eApp = 0;
