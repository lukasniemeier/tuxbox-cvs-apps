/*
	Neutrino-GUI  -   DBoxII-Project

	Homepage: http://dbox.cyberphoria.org/

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


#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <global.h>
#include <neutrino.h>

#include <driver/rcinput.h>

#include "color.h"
#include "motorcontrol.h"

#include "widget/menue.h"
#include "widget/messagebox.h"

#include "system/settings.h"

CMotorControl::CMotorControl()
{
	satfindpid = -1;
	
	frameBuffer = CFrameBuffer::getInstance();
	
	width = 420;
	hheight = g_Fonts->menu_title->getHeight();
	mheight = g_Fonts->menu->getHeight();
	height = hheight + (19 * mheight);
	if (height > 576) height = 576;
	x = ((720 - width) >> 1);
	y = (576 - height) >> 1;
	
	stepSize = 1; //default: 1 step
	stepMode = STEP_MODE_TIMED;
	installerMenue = false;
	motorPosition = 1;
	satellitePosition = 0;
	stepDelay = 10;
}

int CMotorControl::exec(CMenuTarget* parent, string)
{
	uint msg;
	uint data;
	bool istheend = false;
	
	if (!frameBuffer->getActive())
		return menu_return::RETURN_EXIT_ALL;
	
	if (parent)
		parent->hide();
		
	startSatFind();
		
	paint();
	paintMenu();
	paintStatus();

	while (!istheend)
	{

		unsigned long long timeoutEnd = g_RCInput->calcTimeoutEnd_MS(250);
		msg = CRCInput::RC_nokey;

		while (!(msg == CRCInput::RC_timeout) && (!(msg == CRCInput::RC_home)))
		{
			g_RCInput->getMsgAbsoluteTimeout(&msg, &data, &timeoutEnd);

			if (installerMenue)
			{
				switch(msg)
				{
					case CRCInput::RC_ok:
					case CRCInput::RC_0:
						printf("[motorcontrol] 0 key received... goto userMenue\n");
						installerMenue = false;
						paintMenu();
						paintStatus();
						break;
						
					case CRCInput::RC_1:
					case CRCInput::RC_right:
						printf("[motorcontrol] left/1 key received... drive/Step motor west, stepMode: %d\n", stepMode);
						motorStepWest();
						paintStatus();
						break;
					
					case CRCInput::RC_red:
					case CRCInput::RC_2:
						printf("[motorcontrol] 2 key received... halt motor\n");
						g_Zapit->sendMotorCommand(0xE0, 0x31, 0x60, 0, 0, 0);
						break;

					case CRCInput::RC_3:
					case CRCInput::RC_left:
						printf("[motorcontrol] right/3 key received... drive/Step motor east, stepMode: %d\n", stepMode);
						motorStepEast();
						paintStatus();
						break;
						
					case CRCInput::RC_4:
						printf("[motorcontrol] 4 key received... set west (soft) limit\n");
						g_Zapit->sendMotorCommand(0xE1, 0x31, 0x67, 0, 0, 0);
						break;
						
					case CRCInput::RC_5:
						printf("[motorcontrol] 5 key received... disable (soft) limits\n");
						g_Zapit->sendMotorCommand(0xE0, 0x31, 0x63, 0, 0, 0);
						break;
					
					case CRCInput::RC_6:
						printf("[motorcontrol] 6 key received... set east (soft) limit\n");
						g_Zapit->sendMotorCommand(0xE1, 0x31, 0x66, 0, 0, 0);
						break;
					
					case CRCInput::RC_7:
						printf("[motorcontrol] 7 key received... goto reference position\n");
						g_Zapit->sendMotorCommand(0xE0, 0x31, 0x6B, 1, 0, 0);
						satellitePosition = 0;
						paintStatus();
						break;
					
					case CRCInput::RC_8:
						printf("[motorcontrol] 8 key received... enable (soft) limits\n");
						g_Zapit->sendMotorCommand(0xE0, 0x31, 0x6A, 1, 0, 0);
						break;
					
					case CRCInput::RC_9:
						printf("[motorcontrol] 9 key received... (re)-calculate positions\n");
						g_Zapit->sendMotorCommand(0xE0, 0x31, 0x6F, 1, 0, 0);
						break;
					
					case CRCInput::RC_plus:
					case CRCInput::RC_up:
						printf("[motorcontrol] up key received... increase satellite position: %d\n", ++motorPosition);
						satellitePosition = 0;
						paintStatus();
						break;
					
					case CRCInput::RC_minus:
					case CRCInput::RC_down:
						if (motorPosition > 1) motorPosition--;
						printf("[motorcontrol] down key received... decrease satellite position: %d\n", motorPosition);
						satellitePosition = 0;
						paintStatus();
						break;
					
					case CRCInput::RC_blue:
						if (++stepMode > 2) 
							stepMode = 0;
						if (stepMode == STEP_MODE_OFF)
							satellitePosition = 0;
						printf("[motorcontrol] red key received... toggle stepmode on/off: %d\n", stepMode);
						paintStatus();
						break;
					
					default:
						//printf("[motorcontrol] message received...\n");
						if ((msg >= CRCInput::RC_WithData) && (msg < CRCInput::RC_WithData + 0x10000000)) 
							delete (unsigned char*) data;
						break;
				}
			}
			else
			{
				switch(msg)
				{
					case CRCInput::RC_ok:
					case CRCInput::RC_0:
						printf("[motorcontrol] 0 key received... goto installerMenue\n");
						installerMenue = true;
						paintMenu();
						paintStatus();
						break;
						
					case CRCInput::RC_1:
					case CRCInput::RC_right:
						printf("[motorcontrol] left/1 key received... drive/Step motor west, stepMode: %d\n", stepMode);
						motorStepWest();
						paintStatus();
						break;
					
					case CRCInput::RC_red:
					case CRCInput::RC_2:
						printf("[motorcontrol] 2 key received... halt motor\n");
						g_Zapit->sendMotorCommand(0xE0, 0x31, 0x60, 0, 0, 0);
						break;

					case CRCInput::RC_3:
					case CRCInput::RC_left:
						printf("[motorcontrol] right/3 key received... drive/Step motor east, stepMode: %d\n", stepMode);
						motorStepEast();
						paintStatus();
						break;
					
					case CRCInput::RC_green:
					case CRCInput::RC_5:
						printf("[motorcontrol] 5 key received... store present satellite number: %d\n", motorPosition);
						g_Zapit->sendMotorCommand(0xE0, 0x31, 0x6A, 1, motorPosition, 0);
						break;
					
					case CRCInput::RC_6:
						if (stepSize < 0x7F) stepSize++;
						printf("[motorcontrol] 6 key received... increase Step size: %d\n", stepSize);
						paintStatus();
						break;
					
					case CRCInput::RC_yellow:
					case CRCInput::RC_7:
						printf("[motorcontrol] 7 key received... goto satellite number: %d\n", motorPosition);
						g_Zapit->sendMotorCommand(0xE0, 0x31, 0x6B, 1, motorPosition, 0);
						satellitePosition = 0;
						paintStatus();
						break;
					
					case CRCInput::RC_9:
						if (stepSize > 1) stepSize--;
						printf("[motorcontrol] 9 key received... decrease Step size: %d\n", stepSize);
						paintStatus();
						break;
					
					case CRCInput::RC_plus:
					case CRCInput::RC_up:
						printf("[motorcontrol] up key received... increase satellite position: %d\n", ++motorPosition);
						satellitePosition = 0;
						paintStatus();
						break;
					
					case CRCInput::RC_minus:
					case CRCInput::RC_down:
						if (motorPosition > 1) motorPosition--;
						printf("[motorcontrol] down key received... decrease satellite position: %d\n", motorPosition);
						satellitePosition = 0;
						paintStatus();
						break;
					
					case CRCInput::RC_blue:
						if (++stepMode > 2) 
							stepMode = 0;
						if (stepMode == STEP_MODE_OFF)
							satellitePosition = 0;
						printf("[motorcontrol] red key received... toggle stepmode on/off: %d\n", stepMode);
						paintStatus();
						break;
					
					default:
						//printf("[motorcontrol] message received...\n");
						if ((msg >= CRCInput::RC_WithData) && (msg < CRCInput::RC_WithData + 0x10000000)) 
							delete (unsigned char*) data;
						break;
				}
			}
		}
		
		istheend = (msg == CRCInput::RC_home);
	}
	
	hide();

	return menu_return::RETURN_REPAINT;
}

void CMotorControl::motorStepWest(void)
{
	printf("[motorcontrol] motorStepWest\n");
	switch(stepMode)
	{
		case STEP_MODE_ON:
			g_Zapit->sendMotorCommand(0xE0, 0x31, 0x69, 1, (-1 * stepSize), 0);
			satellitePosition += stepSize;
			break;
		case STEP_MODE_TIMED:
			g_Zapit->sendMotorCommand(0xE0, 0x31, 0x69, 1, 40, 0);
			usleep(stepSize * stepDelay * 1000);
			g_Zapit->sendMotorCommand(0xE0, 0x31, 0x60, 0, 0, 0); //halt motor
			satellitePosition += stepSize;
			break;
		default:
			g_Zapit->sendMotorCommand(0xE0, 0x31, 0x69, 1, 40, 0);
	}
}	

void CMotorControl::motorStepEast(void)
{
	printf("[motorcontrol] motorStepEast\n");
	switch(stepMode)
	{
		case STEP_MODE_ON:
			g_Zapit->sendMotorCommand(0xE0, 0x31, 0x68, 1, (-1 * stepSize), 0);
			satellitePosition -= stepSize;
			break;
		case STEP_MODE_TIMED:
			g_Zapit->sendMotorCommand(0xE0, 0x31, 0x68, 1, 40, 0);
			usleep(stepSize * stepDelay * 1000);
			g_Zapit->sendMotorCommand(0xE0, 0x31, 0x60, 0, 0, 0); //halt motor
			satellitePosition -= stepSize;
			break;
		default:
			g_Zapit->sendMotorCommand(0xE0, 0x31, 0x68, 1, 40, 0);
	}
}

void CMotorControl::hide()
{
	frameBuffer->paintBackgroundBoxRel(x, y, width, height + 20);
	stopSatFind();
}

void CMotorControl::paintLine(int xpos, int * ypos, char * txt)
{
	*ypos += mheight;
	frameBuffer->paintBoxRel(xpos, *ypos - mheight, width - xpos, hheight, COL_MENUCONTENT);
	g_Fonts->menu->RenderString(xpos, *ypos, g_Fonts->menu->getRenderWidth(txt), txt, COL_MENUCONTENT);
}

void CMotorControl::paintLine(int xpos, int ypos, char * txt)
{
	frameBuffer->paintBoxRel(xpos, ypos - mheight, width - xpos, hheight, COL_MENUCONTENT);
	g_Fonts->menu->RenderString(xpos, ypos, g_Fonts->menu->getRenderWidth(txt), txt, COL_MENUCONTENT);
}

void CMotorControl::paintSeparator(int xpos, int * ypos, int width, char * txt)
{
	int stringwidth = 0;
	int stringstartposX = 0;
	
	*ypos += mheight;
	frameBuffer->paintHLineRel(xpos, width - 20, *ypos - (mheight >> 1), COL_MENUCONTENT + 3);
	frameBuffer->paintHLineRel(xpos, width - 20, *ypos - (mheight >> 1) + 1, COL_MENUCONTENT + 1);
	
	stringwidth = g_Fonts->menu->getRenderWidth(txt);
	stringstartposX = 0;
	stringstartposX = (xpos + (width >> 1)) - (stringwidth >> 1);
	frameBuffer->paintBoxRel(stringstartposX - 5, *ypos - mheight, stringwidth + 10, mheight, COL_MENUCONTENT);
	g_Fonts->menu->RenderString(stringstartposX, *ypos, stringwidth, txt, COL_MENUCONTENT);
}

void CMotorControl::paintStatus()
{
	char buf[256];
	char buf2[256];
	int xpos1, xpos2;
	
	xpos1 = x + 10;
	xpos2 = xpos1 + 10 + g_Fonts->menu->getRenderWidth("(a) Motor Position:");
	
	ypos = ypos_status;
	paintSeparator(xpos1, &ypos, width, "Motor Control Settings");
	//paintLine(xpos1, &ypos, "------ Motor Control Settings ------");
	
	paintLine(xpos1, &ypos, "(a) Motor Position:");
	sprintf(buf, "%d", motorPosition);
	paintLine(xpos2, ypos, buf);
	
	paintLine(xpos1, &ypos, "(b) Movement:");
	switch(stepMode)
	{
		case STEP_MODE_ON:
			strcpy(buf, "Step Mode");
			break;
		case STEP_MODE_OFF:
			strcpy(buf, "Drive Mode");
			break;
		case STEP_MODE_TIMED:
			strcpy(buf, "Timed Step Mode");
			break;
	}
	paintLine(xpos2, ypos, buf);
	
	paintLine(xpos1, &ypos, "(c) Step Size:");
	switch(stepMode)
	{
		case STEP_MODE_ON:
			sprintf(buf, "%d", stepSize);
			break;
		case STEP_MODE_OFF:
			strcpy(buf, "don't care");
			break;
		case STEP_MODE_TIMED:
			sprintf(buf, "%d", stepSize * stepDelay);
			strcat(buf, " milliseconds");
			break;
	}
	paintLine(xpos2, ypos, buf);
	
	paintSeparator(xpos1, &ypos, width, "Status");
	//paintLine(xpos1, &ypos, "---------------- Status ---------------");
	strcpy(buf, "Satellite Position (Step Mode): ");
	sprintf(buf2, "%d", satellitePosition);
	strcat(buf, buf2);
	paintLine(xpos1, &ypos, buf);
	
}

void CMotorControl::paint()
{
	ypos = y;
	frameBuffer->paintBoxRel(x, ypos, width, hheight, COL_MENUHEAD);
	g_Fonts->menu_title->RenderString(x + 10, ypos + hheight + 1, width, g_Locale->getText("motorcontrol.head").c_str(), COL_MENUHEAD);
	frameBuffer->paintBoxRel(x, ypos + hheight, width, height - hheight, COL_MENUCONTENT);

	ypos += hheight + (mheight >> 1) - 10;
	ypos_menue = ypos;
}

void CMotorControl::paintMenu()
{
	int xpos1, xpos2;
	
	ypos = ypos_menue;
	
	xpos1 = x + 10;
	xpos2 = xpos1 + 10 + g_Fonts->menu->getRenderWidth("(7/yellow)");
	paintLine(xpos1, &ypos, "(0/OK)");
	(installerMenue ? paintLine(xpos2, ypos, "User Menue") : paintLine(xpos2, ypos, "Installer Menue"));
	paintLine(xpos1, &ypos, "(1/right)");
	paintLine(xpos2, ypos, "Step/Drive Motor West (b,c)");
	paintLine(xpos1, &ypos, "(2/red)");
	paintLine(xpos2, ypos, "Halt Motor");
	paintLine(xpos1, &ypos, "(3/left)");
	paintLine(xpos2, ypos, "Step/Drive Motor East (b,c)");
	
	if (installerMenue)
	{
		paintLine(xpos1, &ypos, "(4)");
		paintLine(xpos2, ypos, "Set West (soft) Limit");
		paintLine(xpos1, &ypos, "(5)");
		paintLine(xpos2, ypos, "Disable (soft) Limits");
		paintLine(xpos1, &ypos, "(6)");
		paintLine(xpos2, ypos, "Set East (soft) Limit");
		paintLine(xpos1, &ypos, "(7)");
		paintLine(xpos2, ypos, "Goto Reference Position");
		paintLine(xpos1, &ypos, "(8)");
		paintLine(xpos2, ypos, "Enable (soft) Limits");
		paintLine(xpos1, &ypos, "(9)");
		paintLine(xpos2, ypos, "(Re)-Calculate Positions");
		paintLine(xpos1, &ypos, "(+/up)");
		paintLine(xpos2, ypos, "Increase Motor Position (a)");
		paintLine(xpos1, &ypos, "(-/down)");
		paintLine(xpos2, ypos, "Decrease Motor Position (a)");
		paintLine(xpos1, &ypos, "(blue)");
		paintLine(xpos2, ypos, "Switch Step/Drive Mode (b)");
	}
	else
	{
		paintLine(xpos1, &ypos, "(4)");
		paintLine(xpos2, ypos, "not defined");
		paintLine(xpos1, &ypos, "(5/green)");
		paintLine(xpos2, ypos, "Store Motor Position (a)");
		paintLine(xpos1, &ypos, "(6)");
		paintLine(xpos2, ypos, "Increase Step Size (c)");
		paintLine(xpos1, &ypos, "(7/yellow)");
		paintLine(xpos2, ypos, "Goto Motor Position (a)");
		paintLine(xpos1, &ypos, "(8)");
		paintLine(xpos2, ypos, "not defined");
		paintLine(xpos1, &ypos, "(9)");
		paintLine(xpos2, ypos, "Decrease Step Size (c)");
		paintLine(xpos1, &ypos, "(+/up)");
		paintLine(xpos2, ypos, "Increase Motor Position (a)");
		paintLine(xpos1, &ypos, "(-/down)");
		paintLine(xpos2, ypos, "Decrease Motor Position (a)");
		paintLine(xpos1, &ypos, "(blue)");
		paintLine(xpos2, ypos, "Switch Step/Drive Mode (b)");	
	}
	
	ypos_status = ypos;
}

void CMotorControl::startSatFind(void)
{
	
		if (satfindpid != -1)
		{
			kill(satfindpid, SIGKILL);
			waitpid(satfindpid, 0, 0);
			satfindpid = -1;
		}
		
		switch ((satfindpid = fork()))
		{
		case -1:
			printf("[motorcontrol] fork");
			break;
		case 0:
			printf("[motorcontrol] starting satfind...\n");
			if (execlp("/bin/satfind", "satfind", NULL) < 0)
				printf("[motorcontrol] execlp satfind failed.\n");		
			break;
		} /* switch */
}

void CMotorControl::stopSatFind(void)
{
	
	if (satfindpid != -1)
	{
		printf("[motorcontrol] killing satfind...\n");
		kill(satfindpid, SIGKILL);
		waitpid(satfindpid, 0, 0);
		satfindpid = -1;
	}
}




