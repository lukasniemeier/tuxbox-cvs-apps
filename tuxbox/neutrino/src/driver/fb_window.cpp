/*
 * $Header: /cvs/tuxbox/apps/tuxbox/neutrino/src/driver/fb_window.cpp,v 1.1 2003/02/22 23:17:13 thegoodguy Exp $
 *
 * abstract fb_window class - d-box2 linux project
 *
 * (C) 2003 by thegoodguy <thegoodguy@berlios.de>
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
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 */

#include "fb_window.h"

#include "fontrenderer.h"
#include "framebuffer.h"

class CPrivateData
{
 public:
	CFrameBuffer * frameBuffer;
	void         * Background;
};

CFBWindow::CFBWindow(const int _x, const int _y, const int _dx, const int _dy)
{
	x  = _x ;
	y  = _y ;
	dx = _dx;
	dy = _dy;

	private_data = (void *) new CPrivateData;
	((CPrivateData *)private_data)->frameBuffer = CFrameBuffer::getInstance();
	((CPrivateData *)private_data)->Background = (void*) new unsigned char [_dx * _dy];
	if (((CPrivateData *)private_data)->Background != NULL)
		((CPrivateData *)private_data)->frameBuffer->SaveScreen(_x, _y, _dx, _dy, (unsigned char *)((CPrivateData *)private_data)->Background);

}

CFBWindow::~CFBWindow(void)
{
	if (private_data != NULL)
	{
		if (((CPrivateData *)private_data)->Background != NULL)
			((CPrivateData *)private_data)->frameBuffer->RestoreScreen(x, y, dx, dy, (unsigned char *)((CPrivateData *)private_data)->Background);
		
		delete ((CPrivateData *)private_data);
		private_data = NULL;
	}
}

void CFBWindow::paintBoxRel(const int _x, const int _y, const int _dx, const int _dy, const color_t _col)
{
	((CPrivateData *)private_data)->frameBuffer->paintBoxRel(x + _x, y + _y, _dx, _dy, _col);
}

bool CFBWindow::paintIcon(const std::string _filename, const int _x, const int _y, const color_t _offset)
{
	((CPrivateData *)private_data)->frameBuffer->paintIcon(_filename, x + _x, y + _y, _offset);
}

void CFBWindow::RenderString(const font_t _font, const int _x, const int _y, const int _width, const std::string _text, const unsigned char _color, const int _boxheight, const bool _utf8_encoded)
{
	((Font *)_font)->RenderString(x + _x, y + _y, _width, _text, _color, _boxheight, _utf8_encoded);
}
