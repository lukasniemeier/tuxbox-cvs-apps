/*
 * $Id: text.h,v 1.1 2010/03/03 20:28:45 rhabarber1848 Exp $
 *
 * logomask - d-box2 linux project
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
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
*/

#ifndef __TEXT_H__

#define __TEXT_H__

#include "logomask.h"

extern int FSIZE_BIG;
extern int FSIZE_MED;
extern int FSIZE_SMALL;

FT_Error MyFaceRequester(FTC_FaceID face_id, FT_Library library, FT_Pointer request_data, FT_Face *aface);
void RenderString(char *string, int sx, int sy, int maxwidth, int layout, int size, int color);
int RenderChar(FT_ULong currentchar, int sx, int sy, int ex, int color);
int GetStringLen(unsigned char *string);

#endif
