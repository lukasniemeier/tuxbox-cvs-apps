#include <fcntl.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <linux/fb.h>
#include <sys/ioctl.h>
#include <sys/mman.h>

#include <dbox/avia_gt_pig.h>
#include <dbox/avs_core.h>
#include <dbox/fp.h>
#include <ost/dmx.h>
#include <plugin.h>

#include <ft2build.h>
#include FT_FREETYPE_H
#include "config.h"

#define PAGESIZE 40*24

#define fixfontheight 21

//colortable

#define black	0x01
#define red		0x02
#define green	0x03
#define yellow	0x04
#define blue	0x05
#define magenta	0x06
#define cyan	0x07
#define white	0x08
#define transp	0x09
#define menu1	0x0A
#define menu2	0x0B
#define menu3	0x0C

//spacing attributes

#define alpha_black			0x00
#define alpha_red			0x01
#define alpha_green			0x02
#define alpha_yellow		0x03
#define alpha_blue			0x04
#define alpha_magenta		0x05
#define alpha_cyan			0x06
#define alpha_white			0x07
#define flash				0x08
#define steady				0x09
#define end_box				0x0A
#define start_box			0x0B
#define normal_size			0x0C
#define double_height		0x0D
#define double_width		0x0E
#define double_size			0x0F
#define mosaic_black		0x10
#define mosaic_red			0x11
#define mosaic_green		0x12
#define mosaic_yellow		0x13
#define mosaic_blue			0x14
#define mosaic_magenta		0x15
#define mosaic_cyan			0x16
#define mosaic_white		0x17
#define conceal				0x18
#define contiguous_mosaic	0x19
#define separated_mosaic	0x1A
#define esc					0x1B
#define black_background	0x1C
#define new_background		0x1D
#define hold_mosaic			0x1E
#define release_mosaic		0x1F

//rc codes

#define	RC1_0		0x5C00
#define	RC1_1		0x5C01
#define	RC1_2		0x5C02
#define	RC1_3		0x5C03
#define	RC1_4		0x5C04
#define	RC1_5		0x5C05
#define	RC1_6		0x5C06
#define	RC1_7		0x5C07
#define	RC1_8		0x5C08
#define	RC1_9		0x5C09
#define	RC1_STANDBY	0x5C0C
#define	RC1_UP		0x5C0E
#define	RC1_DOWN	0x5C0F
#define	RC1_PLUS	0x5C16
#define	RC1_MINUS	0x5C17
#define	RC1_HOME	0x5C20
#define	RC1_DBOX	0x5C27
#define	RC1_MUTE	0x5C28
#define	RC1_RED		0x5C2D
#define	RC1_RIGHT	0x5C2E
#define	RC1_LEFT	0x5C2F
#define	RC1_OK		0x5C30
#define	RC1_BLUE	0x5C3B
#define	RC1_YELLOW	0x5C52
#define	RC1_GREEN	0x5C55
#define	RC1_HELP	0x5C82
#define	RC_0		0x00
#define	RC_1		0x01
#define	RC_2		0x02
#define	RC_3		0x03
#define	RC_4		0x04
#define	RC_5		0x05
#define	RC_6		0x06
#define	RC_7		0x07
#define	RC_8		0x08
#define	RC_9		0x09
#define	RC_RIGHT	0x0A
#define	RC_LEFT		0x0B
#define	RC_UP		0x0C
#define	RC_DOWN		0x0D
#define	RC_OK		0x0E
#define	RC_MUTE		0x0F
#define	RC_STANDBY	0x10
#define	RC_GREEN	0x11
#define	RC_YELLOW	0x12
#define	RC_RED		0x13
#define	RC_BLUE		0x14
#define	RC_PLUS		0x15
#define	RC_MINUS	0x16
#define	RC_HELP		0x17
#define	RC_DBOX		0x18
#define	RC_HOME		0x1F

//functions

void ConfigMenu();
void CleanUp();
void PageInput(int Number);
void Prev100();
void Prev10();
void Next10();
void Next100();
void PageCatching();
void CatchNextPage(int Init);
void CatchPrevPage();
void GetNextPageOne();
void GetPrevPageOne();
void GetNextSubPage();
void GetPrevSubPage();
void SwitchZoomMode();
void SwitchScreenMode();
void SwitchTranspMode();
void SwitchHintMode();
void CreateLine25();
void CopyBB2FB();
void RenderCatchedPage();
void RenderCharFB(int Char, int Attribute);
void RenderCharBB(int Char, int Attribute);
void RenderPageNotFound();
void RenderPage();
void DecodePage();
void *CacheThread(void *arg);
int  Init();
int  GetVideotextPIDs();
int  GetRCCode();

//framebuffer stuff

unsigned char *lfb = 0;
struct fb_var_screeninfo var_screeninfo;
struct fb_fix_screeninfo fix_screeninfo;

//freetype stuff

FT_Library	library;
FT_Face		face;

//some data

int dmx, pig, avs, rc, fb;
int sx, ex, sy, ey;
int vtxtpid;
int PosX, PosY, StartX, StartY;
int current_page, current_subpage, page, subpage, lastpage, pageupdate, zap_subpage_manual;
int inputcounter;
int zoommode, screenmode, transpmode, hintmode, boxed;
int fontwidth, fontheight;
int catch_row, catch_col, catched_page;
int prev_100, prev_10, next_10, next_100;
int fnc_old, fnc_mode1, fnc_mode2, fnc_old1, fnc_old2;
int clear_page, clear_subpage;
int pids_found;
int SDT_ready;

FILE *conf;

pthread_t thread_id;
void *thread_result;

unsigned short RCCode;

struct _pid_table
{
	int	 vtxt_pid;
	int	 service_id;
	int	 service_name_len;
	char service_name[24];
}pid_table[128];

//buffers

unsigned char  backbuffer[720*576];
unsigned char  timestring[8];
unsigned char  page_char[PAGESIZE];
unsigned short page_atrb[PAGESIZE];	// ??????:h:c:bbbb:ffff -> ?=reserved, h=double height, c=charset, b=background, f=forground

//cachetables

unsigned char *cachetable[0x900][0x80];
unsigned char subpagetable[0x900];

//colormap

unsigned short rd[] = {0x01<<8, 0xFF<<8, 0x00<<8, 0xFF<<8, 0x00<<8, 0xFF<<8, 0x00<<8, 0xFF<<8, 0x00<<8, 0x00<<8, 0x00<<8, 0x00<<8};
unsigned short gn[] = {0x01<<8, 0x00<<8, 0xFF<<8, 0xFF<<8, 0x00<<8, 0x00<<8, 0xFF<<8, 0xFF<<8, 0x00<<8, 0x20<<8, 0x10<<8, 0x20<<8};
unsigned short bl[] = {0x01<<8, 0x00<<8, 0x00<<8, 0x00<<8, 0xFF<<8, 0xFF<<8, 0xFF<<8, 0xFF<<8, 0x00<<8, 0x40<<8, 0x20<<8, 0x40<<8};
unsigned short tr[] = {0x0000 , 0x0000 , 0x0000 , 0x0000 , 0x0000 , 0x0000 , 0x0000 , 0x0000 , 0xFFFF , 0x0000 , 0x0000 , 0x0A00 };
struct fb_cmap colormap = {1, 12, rd, gn, bl, tr};

//hamming table

unsigned char dehamming[]=
{
	0x01, 0xFF, 0x01, 0x01, 0xFF, 0x00, 0x01, 0xFF, 0xFF, 0x02, 0x01, 0xFF, 0x0A, 0xFF, 0xFF, 0x07,
	0xFF, 0x00, 0x01, 0xFF, 0x00, 0x00, 0xFF, 0x00, 0x06, 0xFF, 0xFF, 0x0B, 0xFF, 0x00, 0x03, 0xFF,
	0xFF, 0x0C, 0x01, 0xFF, 0x04, 0xFF, 0xFF, 0x07, 0x06, 0xFF, 0xFF, 0x07, 0xFF, 0x07, 0x07, 0x07,
	0x06, 0xFF, 0xFF, 0x05, 0xFF, 0x00, 0x0D, 0xFF, 0x06, 0x06, 0x06, 0xFF, 0x06, 0xFF, 0xFF, 0x07,
	0xFF, 0x02, 0x01, 0xFF, 0x04, 0xFF, 0xFF, 0x09, 0x02, 0x02, 0xFF, 0x02, 0xFF, 0x02, 0x03, 0xFF,
	0x08, 0xFF, 0xFF, 0x05, 0xFF, 0x00, 0x03, 0xFF, 0xFF, 0x02, 0x03, 0xFF, 0x03, 0xFF, 0x03, 0x03,
	0x04, 0xFF, 0xFF, 0x05, 0x04, 0x04, 0x04, 0xFF, 0xFF, 0x02, 0x0F, 0xFF, 0x04, 0xFF, 0xFF, 0x07,
	0xFF, 0x05, 0x05, 0x05, 0x04, 0xFF, 0xFF, 0x05, 0x06, 0xFF, 0xFF, 0x05, 0xFF, 0x0E, 0x03, 0xFF,
	0xFF, 0x0C, 0x01, 0xFF, 0x0A, 0xFF, 0xFF, 0x09, 0x0A, 0xFF, 0xFF, 0x0B, 0x0A, 0x0A, 0x0A, 0xFF,
	0x08, 0xFF, 0xFF, 0x0B, 0xFF, 0x00, 0x0D, 0xFF, 0xFF, 0x0B, 0x0B, 0x0B, 0x0A, 0xFF, 0xFF, 0x0B,
	0x0C, 0x0C, 0xFF, 0x0C, 0xFF, 0x0C, 0x0D, 0xFF, 0xFF, 0x0C, 0x0F, 0xFF, 0x0A, 0xFF, 0xFF, 0x07,
	0xFF, 0x0C, 0x0D, 0xFF, 0x0D, 0xFF, 0x0D, 0x0D, 0x06, 0xFF, 0xFF, 0x0B, 0xFF, 0x0E, 0x0D, 0xFF,
	0x08, 0xFF, 0xFF, 0x09, 0xFF, 0x09, 0x09, 0x09, 0xFF, 0x02, 0x0F, 0xFF, 0x0A, 0xFF, 0xFF, 0x09,
	0x08, 0x08, 0x08, 0xFF, 0x08, 0xFF, 0xFF, 0x09, 0x08, 0xFF, 0xFF, 0x0B, 0xFF, 0x0E, 0x03, 0xFF,
	0xFF, 0x0C, 0x0F, 0xFF, 0x04, 0xFF, 0xFF, 0x09, 0x0F, 0xFF, 0x0F, 0x0F, 0xFF, 0x0E, 0x0F, 0xFF,
	0x08, 0xFF, 0xFF, 0x05, 0xFF, 0x0E, 0x0D, 0xFF, 0xFF, 0x0E, 0x0F, 0xFF, 0x0E, 0x0E, 0xFF, 0x0E
};
