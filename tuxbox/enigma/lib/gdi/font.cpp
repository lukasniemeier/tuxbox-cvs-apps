#include "font.h"

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <pthread.h>
#include <sys/types.h>
#include <unistd.h>

// use this for init Freetype...
#include <ft2build.h>
#include FT_FREETYPE_H

#include "config.h"

#include <core/base/eerror.h>
#include <core/gdi/lcd.h>
#include <core/gdi/grc.h>
#include <core/system/elock.h>
#include <core/system/init.h>

#include <map>

fontRenderClass *fontRenderClass::instance;
static eLock ftlock;
static FTC_Font cache_current_font=0;

struct fntColorCacheKey
{
	gRGB start, end;
	fntColorCacheKey(const gRGB &start, const gRGB &end)
		: start(start), end(end)
	{
	}
	bool operator <(const fntColorCacheKey &c) const
	{
		if (start < c.start)
			return 1;
		else if (start == c.start)
			return end < c.end;
		return 0;
	}
};

std::map<fntColorCacheKey,gLookup> colorcache;

static gLookup &getColor(const gPalette &pal, const gRGB &start, const gRGB &end)
{
	fntColorCacheKey key(start, end);
	std::map<fntColorCacheKey,gLookup>::iterator i=colorcache.find(key);
	if (i != colorcache.end())
		return i->second;
	gLookup &n=colorcache.insert(std::pair<fntColorCacheKey,gLookup>(key,gLookup())).first->second;
	eDebug("[FONT] creating new font color cache entry %02x%02x%02x%02x .. %02x%02x%02x%02x", start.a, start.r, start.g, start.b,
		end.a, end.r, end.g, end.b);
	n.build(16, pal, start, end);
/*	for (int i=0; i<16; i++)
		eDebugNoNewLine("%02x|%02x%02x%02x%02x ", (int)n.lookup[i], pal.data[n.lookup[i]].a, pal.data[n.lookup[i]].r, pal.data[n.lookup[i]].g, pal.data[n.lookup[i]].b);
	eDebug("");*/
	return n;
}

fontRenderClass *fontRenderClass::getInstance()
{
	return instance;
}

FT_Error myFTC_Face_Requester(FTC_FaceID	face_id,
															FT_Library	library,
															FT_Pointer	request_data,
															FT_Face*		aface)
{
	return ((fontRenderClass*)request_data)->FTC_Face_Requester(face_id, aface);
}


FT_Error fontRenderClass::FTC_Face_Requester(FTC_FaceID	face_id, FT_Face* aface)
{
	fontListEntry *font=(fontListEntry *)face_id;
	if (!font)
		return -1;
	
	eDebug("[FONT] FTC_Face_Requester (%s)", font->face);

	int error;
	if ((error=FT_New_Face(library, font->filename, 0, aface)))
	{
		eDebug(" failed: %s", strerror(error));
		return error;
	}
	FT_Select_Charmap(*aface, ft_encoding_unicode);
	return 0;
}																																																																

FTC_FaceID fontRenderClass::getFaceID(const char *face)
{
	for (fontListEntry *f=font; f; f=f->next)
	{
		if (!strcmp(f->face, face))
			return (FTC_FaceID)f;
	}
	return 0;
}

FT_Error fontRenderClass::getGlyphBitmap(FTC_Image_Desc *font, FT_ULong glyph_index, FTC_SBit *sbit)
{
	FT_Error res=FTC_SBit_Cache_Lookup(sbitsCache, font, glyph_index, sbit);
	return res;
}

const char* fontRenderClass::AddFont(const char *filename)
{
	eDebugNoNewLine("[FONT] adding font %s...", filename);
	fflush(stdout);
	int error;
	fontListEntry *n=new fontListEntry;

	FT_Face face;
	eLocker lock(ftlock);

	if ((error=FT_New_Face(library, filename, 0, &face)))
		eFatal(" failed: %s", strerror(error));

	strcpy(n->filename=new char[strlen(filename)+1], filename);
	strcpy(n->face=new char[strlen(face->family_name)+strlen(face->style_name)+2], face->family_name);
	if (face->style_name[0]!=' ')
		strcat(n->face, " ");
	strcat(n->face, face->style_name);
	FT_Done_Face(face);

	n->next=font;
	eDebug("OK (%s)", n->face);
	font=n;

	return n->face;
}

fontRenderClass::fontListEntry::~fontListEntry()
{
	delete[] filename;
	delete[] face;
}

fontRenderClass::fontRenderClass(): fb(fbClass::getInstance())
{
	instance=this;
	eDebug("[FONT] initializing core...");
	{
		if (FT_Init_FreeType(&library))
		{
			eDebug("[FONT] initializing failed.");
			return;
		}
	}
	eDebug("[FONT] loading fonts...");
	fflush(stdout);
	font=0;
	
	int maxbytes=4*1024*1024;
	eDebug("[FONT] Intializing font cache, using max. %dMB...", maxbytes/1024/1024);
	fflush(stdout);
	{
		if (FTC_Manager_New(library, 8, 8, maxbytes, myFTC_Face_Requester, this, &cacheManager))
		{
			eDebug("[FONT] initializing font cache failed!");
			return;
		}
		if (!cacheManager)
		{
			eDebug("[FONT] initializing font cache manager error.");
			return;
		}
		if (FTC_SBit_Cache_New(cacheManager, &sbitsCache))
		{
			eDebug("[FONT] initializing font cache sbit failed!");
			return;
		}
		if (FTC_Image_Cache_New(cacheManager, &imageCache))
		{
			eDebug("[FONT] initializing font cache imagecache failed!");
		}
	}
	return;
}

fontRenderClass::~fontRenderClass()
{
	ftlock.lock();
//	auskommentiert weil freetype und enigma die kritische masse des suckens ueberschreiten. 
//	FTC_Manager_Done(cacheManager);
//	FT_Done_FreeType(library);
}

Font *fontRenderClass::getFont(const char *face, int size, int tabwidth)
{
	FTC_FaceID id=getFaceID(face);
	if (!id)
		eDebug("face %s does not exist!", face);
	if (!id)
		return 0;
	return new Font(this, id, size, tabwidth);
}

Font::Font(fontRenderClass *render, FTC_FaceID faceid, int isize, int tw): tabwidth(tw)
{
	renderer=render;
	font.font.face_id=faceid;
	font.font.pix_width	= isize;
	font.font.pix_height = isize;
	font.image_type = ftc_image_grays;
	height=isize;
	if (tabwidth==-1)
		tabwidth=8*isize;
	ref=0;
//	font.image_type |= ftc_image_flag_autohinted;
}

FT_Error Font::getGlyphBitmap(FT_ULong glyph_index, FTC_SBit *sbit)
{
	return renderer->getGlyphBitmap(&font, glyph_index, sbit);
}

Font::~Font()
{
}

void Font::lock()
{
	ref++;
}

void Font::unlock()
{
	ref--;
	if (!ref)
		delete this;
}

int eTextPara::appendGlyph(FT_UInt glyphIndex, int flags)
{
	FTC_SBit glyph;
	if (current_font->getGlyphBitmap(glyphIndex, &glyph))
	{
		return 1;
	}
	if ((flags&GS_MYWRAP) && (cursor.x()+ glyph->xadvance) >= area.right())
	{
		int cnt = 0;
		glyphString::iterator i(glyphs.end());
		--i;
		while (i != glyphs.begin())
		{
			if (i->flags&(GS_ISSPACE|GS_ISFIRST))
				break;
			cnt++;
			--i;
		} 
		if (i != glyphs.begin() && ((i->flags&(GS_ISSPACE|GS_ISFIRST))==GS_ISSPACE) && (++i != glyphs.end()))		// skip space
		{
			int linelength=cursor.x()-i->x;
			i->flags|=GS_ISFIRST;
			ePoint offset=ePoint(i->x, i->y);
			newLine();
			offset-=cursor;
			while (i != glyphs.end())		// rearrange them into the next line
			{
				i->x-=offset.x();
				i->y-=offset.y();
				i->bbox->moveBy(-offset.x(), -offset.y());
				++i;
			}
			cursor+=ePoint(linelength, 0);	// put the cursor after that line
		} else
		{
	    if (cnt)
			{
				newLine();
				flags|=GS_ISFIRST;
			}
		}
	}
	int xadvance=glyph->xadvance, kern=0;
	if (previous && use_kerning)
	{
		FT_Vector delta;
		FT_Get_Kerning(current_face, previous, glyphIndex, ft_kerning_default, &delta);
		kern=delta.x>>6;
	}

  eRect* bbox = new eRect();
	bbox->setLeft( (flags&GS_ISFIRST|glyphs.empty()?cursor.x():cursor.x()-1) + glyph->left );	
	bbox->setTop( cursor.y() - glyph->top );
	bbox->setWidth( glyph->width );
	bbox->setHeight( glyph->height );
//	eDebug("GS_ISFIRST=%i, bbox left = %i, top = %i, right = %i, bottom = %i", flags&GS_ISFIRST, bbox->left(), bbox->top(), bbox->right(), bbox->bottom() );

	pGlyph ng;
	ng.x=cursor.x()+kern;
	xadvance+=kern;
	ng.y=cursor.y();
	ng.w=xadvance;
	ng.font=current_font;
	ng.font->lock();
	ng.glyph_index=glyphIndex;
	ng.flags=flags;
	ng.bbox=bbox;
	glyphs.push_back(ng);

	cursor+=ePoint(xadvance, 0);
	previous=glyphIndex;
	return 0;
}

void eTextPara::calc_bbox()
{
	boundBox.setLeft( 32000 );
	boundBox.setTop( 32000 );
	boundBox.setRight( -32000 );         // for each glyph image, compute its bounding box, translate it,
	boundBox.setBottom( -32000 );
	// and grow the string bbox

	for (	glyphString::iterator i(glyphs.begin()); i != glyphs.end(); ++i)
	{
		if ( i->bbox->left() < boundBox.left() )
			boundBox.setLeft( i->bbox->left() );
		if ( i->bbox->top() < boundBox.top() )
			boundBox.setTop( i->bbox->top() );
		if ( i->bbox->right() > boundBox.right() )
			boundBox.setRight( i->bbox->right() );
		if ( i->bbox->bottom() > boundBox.bottom() )
			boundBox.setBottom( i->bbox->bottom() );
	}
//	eDebug("boundBox left = %i, top = %i, right = %i, bottom = %i", boundBox.left(), boundBox.top(), boundBox.right(), boundBox.bottom() );
	bboxValid=1;
}

void eTextPara::newLine()
{
	if (maximum.width()<cursor.x())
		maximum.setWidth(cursor.x());
	cursor.setX(left);
	int linegap=current_face->size->metrics.height-(current_face->size->metrics.ascender+current_face->size->metrics.descender);
	cursor+=ePoint(0, (current_face->size->metrics.ascender+current_face->size->metrics.descender+linegap*1/2)>>6);
	if (maximum.height()<cursor.y())
		maximum.setHeight(cursor.y());
	previous=0;
}

static eLock refcntlck;

eTextPara::~eTextPara()
{
	clear();
	if (refcnt>=0)
		eFatal("verdammt man der war noch gelockt :/\n");
}

void eTextPara::destroy()
{
	eLocker lock(refcntlck);

	if (!refcnt--)
		delete this;
}

eTextPara *eTextPara::grab()
{
	eLocker lock(refcntlck);

	refcnt++;
	return this;
}

void eTextPara::setFont(const gFont &font)
{
	if (refcnt)
		eFatal("mod. after lock");
	setFont(fontRenderClass::getInstance()->getFont(font.family.c_str(), font.pointSize));
}

void eTextPara::setFont(Font *fnt)
{
	if (refcnt)
		eFatal("mod. after lock");
	if (!fnt)
		return;
	if (current_font && !current_font->ref)
		delete current_font;
	current_font=fnt;
	eLocker lock(ftlock);

	if (FTC_Manager_Lookup_Size(fontRenderClass::instance->cacheManager, &current_font->font.font, &current_face, &current_font->size)<0)
	{
		eDebug("FTC_Manager_Lookup_Size failed!");
		return;
	}
	cache_current_font=&current_font->font.font;
	previous=0;
	if (cursor.y()==-1)
	{
		cursor=ePoint(area.x(), area.y()+(current_face->size->metrics.ascender>>6));
		left=cursor.x();
	}
	use_kerning=FT_HAS_KERNING(current_face);
}

int eTextPara::renderString(const eString &string, int rflags)
{
	eLocker lock(ftlock);

	if (refcnt)
		eFatal("mod. after lock");

	if (!current_font)
		return -1;

	glyphs.reserve(glyphs.size()+string.length());

	if (&current_font->font.font != cache_current_font)
	{
		if (FTC_Manager_Lookup_Size(fontRenderClass::instance->cacheManager, &current_font->font.font, &current_face, &current_font->size)<0)
		{
			eDebug("FTC_Manager_Lookup_Size failed!");
			return -1;
		}
		cache_current_font=&current_font->font.font;
	}
	
	std::string::const_iterator p(string.begin());

	while(p != string.end())
	{
		int isprintable=1;
		int flags=0;
	
		if (rflags&RS_WRAP)
			 flags|=GS_MYWRAP;
		
		if (!(rflags&RS_DIRECT))
		{
			switch (*p)
			{
			case '\t':
				isprintable=0;
				cursor+=ePoint(current_font->tabwidth, 0);
				cursor-=ePoint(cursor.x()%current_font->tabwidth, 0);
				break;
			case '\n':
				isprintable=0;
				newLine();
				flags|=GS_ISFIRST;
				break;
			case '\r':
				isprintable=0;
				break;
			case ' ':
				flags|=GS_ISSPACE;
			default:
				break;
			}
		}
		if (isprintable)
		{
			FT_UInt index;

			index=(rflags&RS_DIRECT)? *p : FT_Get_Char_Index(current_face, *p);

			if (!index)
				; // eDebug("unicode %d ('%c') not present", uc, uc);
			else
				appendGlyph(index, flags);
		}
		p++;
	}
	bboxValid=false;
	calc_bbox();
	return 0;
}

void eTextPara::blit(gPixmapDC &dc, const ePoint &offset, const gRGB &background, const gRGB &foreground)
{
	eLocker lock(ftlock);

	if (&current_font->font.font != cache_current_font)
	{
		if (FTC_Manager_Lookup_Size(fontRenderClass::instance->cacheManager, &current_font->font.font, &current_face, &current_font->size)<0)
		{
			eDebug("FTC_Manager_Lookup_Size failed!");
			return;
		}
		cache_current_font=&current_font->font.font;
	}

	gPixmap &target=dc.getPixmap();

	if (target.bpp != 8)
		eFatal("eTextPara::blit - can't render into %d bpp buffer", target.bpp);

	register int opcode;
	gColor *lookup=0;
		
	if (target.clut.data)
	{
		lookup=getColor(target.clut, background, foreground).lookup;
		opcode=0;
	} else
	{
		opcode=1;
	}
		
	eRect clip(0, 0, target.x, target.y);
	clip&=dc.getClip();

	int buffer_stride=target.stride;

	for (glyphString::iterator i(glyphs.begin()); i != glyphs.end(); ++i)
	{
		static FTC_SBit glyph_bitmap;
		if (fontRenderClass::instance->getGlyphBitmap(&i->font->font, i->glyph_index, &glyph_bitmap))
			continue;
		int rx=i->x+glyph_bitmap->left + offset.x();
		int ry=i->y-glyph_bitmap->top  + offset.y();
		__u8 *d=(__u8*)(target.data)+buffer_stride*ry+rx;
		__u8 *s=glyph_bitmap->buffer;
		register int sx=glyph_bitmap->width;
		int sy=glyph_bitmap->height;
		if ((sy+ry) > clip.bottom())
			sy=clip.bottom()-ry+1;
		if ((sx+rx) > clip.right())
			sx=clip.right()-rx+1;
		if (sx>0)
			for (int ay=0; ay<sy; ay++)
			{
				register __u8 *td=d;
				register int ax;
				if (!opcode)
				{
					for (ax=0; ax<sx; ax++)
					{	
						register int b=(*s++)>>4;
						if(b)
							*td++=lookup[b];
						else
							td++;
					}
				} else
				{
					for (ax=0; ax<sx; ax++)
					{	
						register int b=*s++;
						*td++^=b;
					}
				}
				s+=glyph_bitmap->pitch-ax;
				d+=buffer_stride;
			}
	}
}

void eTextPara::realign(int dir)	// der code hier ist ein wenig merkwuerdig.
{
	glyphString::iterator begin(glyphs.begin()), c(glyphs.begin()), end(glyphs.begin()), last;
	if (dir==dirLeft)
		return;
	while (c != glyphs.end())
	{
		int linelength=0;
		int numspaces=0, num=0;
		begin=end;
		
			// zeilenende suchen
		do {
			last=end;
			++end;
		} while ((end != glyphs.end()) && (!(end->flags&GS_ISFIRST)));
			// end zeigt jetzt auf begin der naechsten zeile
			
		for (c=begin; c!=end; ++c)
		{
				// space am zeilenende skippen
			if ((c==last) && (c->flags&GS_ISSPACE))
				continue;

			if (c->flags&GS_ISSPACE)
				numspaces++;
			linelength+=c->w;;
			num++;
		}
		if (!num)		// line mit nur einem space
			continue;

		switch (dir)
		{
		case dirRight:
		case dirCenter:
		{
			int offset=area.width()-linelength;
			if (dir==dirCenter)
				offset/=2;
			while (begin != end)
			{
				begin->x+=offset;
				begin->bbox->moveBy(offset,0);
				++begin;
			}
			break;
		}
		case dirBlock:
		{
			if (end == glyphs.end())		// letzte zeile linksbuendig lassen
				continue;
			int spacemode;
			if (numspaces)
				spacemode=1;
			else
				spacemode=0;
			if ((!spacemode) && (num<2))
				break;
			int off=(area.width()-linelength)*256/(spacemode?numspaces:(num-1));
			int curoff=0;
			while (begin != end)
			{
				int doadd=0;
				if ((!spacemode) || (begin->flags&GS_ISSPACE))
					doadd=1;
				begin->x+=curoff>>8;
				begin->bbox->moveBy(curoff>>8,0);
				if (doadd)
					curoff+=off;
				++begin;
			}
			break;
		}
		}
	}
	bboxValid=false;
}

void eTextPara::clear()
{
	eLocker lock(ftlock);

	for (glyphString::iterator i(glyphs.begin()); i!=glyphs.end(); ++i)
	{
		i->font->unlock();
		delete i->bbox;
	}
	glyphs.clear();
}

eAutoInitP0<fontRenderClass> init_fontRenderClass(1, "Font Render Class");
