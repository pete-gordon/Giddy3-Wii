
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <malloc.h>
#include <math.h>

#include <gccore.h>
#include <wiiuse/wpad.h>

#define DEFAULT_FIFO_SIZE (256 * 1024)

#include "giddy3.h"
#include "render.h"

#include "giddy3.h"
#include "render.h"
#include "specials.h"
#include "enemies.h"
#include "samples.h"
#include "titles.h"

#define SCROFF 140

time_t gamestarttime, gameendtime;

//char bollocks[128];
extern int what_are_we_doing, endingstate;

extern int lastsound, bincount;
extern int dragonmode, titlestate;

extern struct enemy *enemies[], *enemiesb[];
extern int stuffloop[];

extern int musicvol, sfxvol;

extern BOOL onthebin;
extern BOOL musicon;

BOOL ignorejump = FALSE;

int clevel, llevel, lastinv;
u32 frame;
Vector axis = (Vector){ 0, 0, 1 };

extern BOOL pleft,pright,pjump,spacehit,enterhit,paused;

BOOL strsneedupdate=FALSE;

extern struct infotext infos1[], infos2[], infos3[], infos4[], infos5[];
struct infotext *infos;
int winfo;

// RGBA Texture buffers
u8 ATTRIBUTE_ALIGN(32) blocks[512*256*4];
u8 ATTRIBUTE_ALIGN(32) sprites[512*256*4];
u8 ATTRIBUTE_ALIGN(32) wsprites[512*256*4];
u8 ATTRIBUTE_ALIGN(32) gsprites[512*256*4];
u8 ATTRIBUTE_ALIGN(32) wgsprites[512*256*4];
u8 ATTRIBUTE_ALIGN(32) psprites[512*256*4];
u8 ATTRIBUTE_ALIGN(32) wpsprites[512*256*4];
u8 ATTRIBUTE_ALIGN(32) texts[1024*256*4];
u8 ATTRIBUTE_ALIGN(32) tvtex[32*32*4];
u8 *mblocks;
u8 *msprites;
u8 *mwsprites;
u8 *mgsprites;
u8 *mwgsprites;
u8 *mpsprites;
u8 *mwpsprites;
u8 *mtexts;
u8 *screentex;
GXRModeObj *rmode;
void *gp_fifo = NULL;
GXColor bgcol = { 0, 0, 0, 0xff };
Mtx GXmodelView2D;

u8 blktrans[512];
u8 collisionarea[16*16*3*2];
u8 liftstoparea[16*16*3*2];

u8 strig[ST_LAST], inv[INV_LAST];

int fadea=255, fadeadd=-6, fadetype=0;
int lrfade1=255, lrfade2=255;

int infobulbwinfo = -1, infobulbalpha = 0;
float infobulbzoom;

struct invtext ivtexts[] = { { "Coins",                "" },
                             { "Air Horn",             "..An Air Horn.."                     },
                             { "Barrel of Beer",       "..A Barrel of Beer.."                },
                             { "Party Hits CD",        "..A Hideous Party Hits CD.."         },
                             { "Hose Pipe",            "..A Hose pipe.."                     },
                             { "Catapult",             "..A Catapult.."                      },
                             { "Spade",                "..A Spade.."                         },
                             { "Tub of Lard",          "..A Tub of Lard.."                   },
                             { "Bubble Gum",           "..Some Bubble Gum.."                 },
                             { "Control Box",          "..A Control Box.."                   },
                             { "Camera (Inc.Photo)",   "..Camera with photo.."               },
                             { "Large Cog",            "..A Large Cog.."                     },
                             { "Enormous Boot",        "..An Enormous Boot.."                },
                             { "Sturdy Plank",         "..A Sturdy Plank.."                  },
                             { "Scissors",             "..A Pair of Scissors.."              },
                             { "Turps",                "..Some Turps.."                      },
                             { "Candle Stick",         "..A Candle Stick.."                  },
                             { "Diamond",              "..A Diamond.."                       },
                             { "Lighted Candle",       "..A Lighted Candle.."                },
                             { "Balloon wreckage",     "..'Weather Balloon' Wreckage.." },
                             { "Lump of Carbon",       "..A Lump of Carbon.."                },
                             { "Electrical Toolkit",   "..An Electrical Toolkit.."           },
                             { "The A-Teams Tel No",   "..The A-Team's Phone Number.."       },
                             { "TNT Detonator",        "..An Explosives Detonator.."         },
                             { "HardHat",              "..A Hard Hat.."                      },
                             { "Mirror",               "..A Mirror.."                        },
                             { "Indigestion Pills",    "..Some Indigestion Tablets.."        },
                             { "Charged Battery",      "..A Charged Battery.."               },
                             { "Flat Battery",         "..A Flat Battery.."                  },
                             { "Digital Camera",       "..A Digital Camera.."                },
                             { "Printed Photo",        "..A Photo Of An Empty Room.."        },
                             { "Whackin' Great Bomb",  "..A Whackin' Great Bomb.."           },
                             { "Teleporter Watch",     "..An Alien Teleporter Watch.." },
                             { "Scotch mist?",         "" } };

u8 fgmap[30240], bgmap[3840], dmap[280], tmap[30240];

struct btex bt[256];

extern struct btex sprt1[], psprt1[], sprtg[];
extern struct btex sprt2[], psprt2[];
extern struct btex sprt3[], psprt3[];
extern struct btex sprt4[], psprt4[];
extern struct btex sprt5[], psprt5[];
struct btex *psprs, *sprs, *ps;

extern struct lift lifts1[], lifts2[], lifts3[], lifts4[], lifts5[];
struct lift *lifts;

extern struct spring springs1[], springs2[], springs3[], springs4[], springs5[];
struct spring *springs;

int num_nthings, num_bnthings, num_pthings, num_bpthings, num_gthings, num_bgthings;
struct thingy *bn_things[MAX_THINGIES]; // Normal things, behind the foreground
struct thingy *n_things[MAX_THINGIES];  // Normal things
struct thingy *bp_things[MAX_THINGIES]; // Puzzle things, behind the foreground
struct thingy *p_things[MAX_THINGIES];  // Puzzle things
struct thingy *bg_things[MAX_THINGIES]; // Global things, behind the foreground
struct thingy *g_things[MAX_THINGIES];  // Global things

struct star stars[MAX_STARS]; // Incidental stars
int nextstar = 0;

int ibubblepop[] = { 42, 43, 44, 45, 46 };
struct incidental incd[MAX_INCIDENTALS], bincd[MAX_INCIDENTALS];
int nextincd = 0, nextbincd = 0;

int edgecdlist[] = { 11,12,13,13, 14,14,14,15, 15,15,15,15, 15,15,14,14,
                     14,13,13,12, 11 };

//                            FGW  FGH  BGW  BGH  XWRP  YWRP  YOFF  /  /  IMX   IMY   IGX   IGY   SPLIT SPL YSCRTL
struct mapsz mapsizes[] = { { 400,  70,  36,  40, 0x12, 0xff, 0x00, 1, 3, 0x47, 0x0d, 0x50, 0x17,   474, 12, 130 },
                            { 240, 126,  36,  28, 0x10, 0x0e, 0x00, 1, 1, 0x00, 0x70, 0x03, 0x7a, 10000,  0,   0 },
                            { 430,  70,  36,  28, 0x0c, 0x30, 0x22, 1, 3, 0x02, 0x0a, 0x0c, 0x13, 10000,  0,   0 },
                            { 600,  28, 256,  15, 0xff, 0x80, 0x80, 1, 8, 0x49, 0x0c, 0x52, 0x0e, 10000,  0,   0 },
                            { 170, 158,  36,  90, 0x10, 0xff, 0x00, 1, 1, 0x00, 0x15, 0x09, 0x1f, 10000,  0,   0 } };

struct stickyscroll
{
  int miny, maxy, scry;
};

struct stickyscroll stsc1[] = { { -1, } };
struct stickyscroll stsc2[] = { { 1840, 1984, 1784 },
                                { 1376, 1536, 1336 },
                                {  928, 1088,  888 },
                                {  480,  640,  440 },
                                {    0,  204,    0 },
                                { -1, } };
struct stickyscroll stsc3[] = { { -1, } };
struct stickyscroll stsc4[] = { { -1, } };
struct stickyscroll stsc5[] = { { -1, } };
struct stickyscroll *stsc;

struct mapsz *mapi;

int fgx, fgy, bgx, bgy;

struct what_is_giddy_doing gid;
struct what_is_everyone_else_doing stuff;

extern s16 sintab[];
extern u8 tvborders[];
extern int hoptab[];

extern struct thingy n_things1[], g_things1[], p_things1[];
extern struct thingy n_things2[], g_things2[], p_things2[];
extern struct thingy n_things3[], g_things3[], p_things3[];
extern struct thingy n_things4[], g_things4[], p_things4[];
extern struct thingy n_things5[], g_things5[], p_things5[];

static void *xfb[2] = { NULL, NULL};
u32 fb=0;

int ibstate=0, ibalpha, ibwait=0, ibitem = -1;
float ibscale, ibrot;
float ibw[3] = {0.0f,0.0f,0.0f};
float ibtw[3] = {0.0f,0.0f,0.0f};
char *ibt[3] = { NULL, NULL, NULL };
int ibpos, ibadd=0, iboff, ibdest;

extern int bangframes[];
extern int trd_x, trd_y;

BOOL considertardis;

extern BOOL audioavailable;

void flip( void )
{
	GX_DrawDone();
	
	fb ^= 1;
	GX_SetZMode( GX_TRUE, GX_LEQUAL, GX_TRUE );
	GX_SetColorUpdate( GX_TRUE );
	GX_CopyDisp( xfb[fb], GX_TRUE );
	VIDEO_SetNextFramebuffer( xfb[fb] );
	VIDEO_Flush();
	VIDEO_WaitVSync();
}

void bindtexture( GXTexObj *texObj, u8 *texp )
{
	GX_InitTexObj( texObj, texp, 256, 256, GX_TF_RGBA8, GX_CLAMP, GX_CLAMP, GX_FALSE );
	GX_InitTexObjLOD( texObj, GX_NEAR, GX_NEAR, 0.0f, 0.0f, 0.0f, 0, 0, GX_ANISO_1 );
	GX_LoadTexObj( texObj, GX_TEXMAP0 );

  GX_SetTevOp( GX_TEVSTAGE0, GX_MODULATE );
  GX_SetVtxDesc( GX_VA_TEX0, GX_DIRECT );
}

void unbindtexture( void )
{
  GX_SetTevOp (GX_TEVSTAGE0, GX_PASSCLR);
  GX_SetVtxDesc (GX_VA_TEX0, GX_NONE);
}

void texturemangle( int w, int h, u8 *src, u8 *dest )
{
  int x, y, qw, qh, rw;

  qw = w/4;
  qh = h/4;
  rw = w*4;

  for (y = 0; y < qh; y++)
    for (x = 0; x < qw; x++)
    {
      int blockbase = (y * qw + x) * 8;

      u64 fieldA = *((u64 *)(&src[(y*4)*rw+x*16]));
      u64 fieldB = *((u64 *)(&src[(y*4)*rw+x*16+8]));

      ((u64 *) dest)[blockbase] = 
        ((fieldA & 0xFF00000000ULL) << 24) | ((fieldA & 0xFF00000000000000ULL) >> 8) | 
        ((fieldA & 0xFFULL) << 40) | ((fieldA & 0xFF000000ULL) << 8) | 
        ((fieldB & 0xFF00000000ULL) >> 8) | ((fieldB & 0xFF00000000000000ULL) >> 40) | 
        ((fieldB & 0xFFULL) << 8) | ((fieldB & 0xFF000000ULL) >> 24);
      ((u64 *) dest)[blockbase+4] =
        ((fieldA & 0xFFFF0000000000ULL) << 8) | ((fieldA & 0xFFFF00ULL) << 24) |
        ((fieldB & 0xFFFF0000000000ULL) >> 24) | ((fieldB & 0xFFFF00ULL) >> 8);

      fieldA = *((u64 *)(&src[(y*4+1)*rw+x*16]));
      fieldB = *((u64 *)(&src[(y*4+1)*rw+x*16+8]));

      ((u64 *) dest)[blockbase+1] = 
        ((fieldA & 0xFF00000000ULL) << 24) | ((fieldA & 0xFF00000000000000ULL) >> 8) | 
        ((fieldA & 0xFFULL) << 40) | ((fieldA & 0xFF000000ULL) << 8) | 
        ((fieldB & 0xFF00000000ULL) >> 8) | ((fieldB & 0xFF00000000000000ULL) >> 40) | 
        ((fieldB & 0xFFULL) << 8) | ((fieldB & 0xFF000000ULL) >> 24);
      ((u64 *) dest)[blockbase+5] =
        ((fieldA & 0xFFFF0000000000ULL) << 8) | ((fieldA & 0xFFFF00ULL) << 24) |
        ((fieldB & 0xFFFF0000000000ULL) >> 24) | ((fieldB & 0xFFFF00ULL) >> 8);

      fieldA = *((u64 *)(&src[(y*4+2)*rw+x*16]));
      fieldB = *((u64 *)(&src[(y*4+2)*rw+x*16+8]));

      ((u64 *) dest)[blockbase+2] = 
        ((fieldA & 0xFF00000000ULL) << 24) | ((fieldA & 0xFF00000000000000ULL) >> 8) | 
        ((fieldA & 0xFFULL) << 40) | ((fieldA & 0xFF000000ULL) << 8) | 
        ((fieldB & 0xFF00000000ULL) >> 8) | ((fieldB & 0xFF00000000000000ULL) >> 40) | 
        ((fieldB & 0xFFULL) << 8) | ((fieldB & 0xFF000000ULL) >> 24);
      ((u64 *) dest)[blockbase+6] =
        ((fieldA & 0xFFFF0000000000ULL) << 8) | ((fieldA & 0xFFFF00ULL) << 24) |
        ((fieldB & 0xFFFF0000000000ULL) >> 24) | ((fieldB & 0xFFFF00ULL) >> 8);

      fieldA = *((u64 *)(&src[(y*4+3)*rw+x*16]));
      fieldB = *((u64 *)(&src[(y*4+3)*rw+x*16+8]));

      ((u64 *) dest)[blockbase+3] = 
        ((fieldA & 0xFF00000000ULL) << 24) | ((fieldA & 0xFF00000000000000ULL) >> 8) | 
        ((fieldA & 0xFFULL) << 40) | ((fieldA & 0xFF000000ULL) << 8) | 
        ((fieldB & 0xFF00000000ULL) >> 8) | ((fieldB & 0xFF00000000000000ULL) >> 40) | 
        ((fieldB & 0xFFULL) << 8) | ((fieldB & 0xFF000000ULL) >> 24);
      ((u64 *) dest)[blockbase+7] =
        ((fieldA & 0xFFFF0000000000ULL) << 8) | ((fieldA & 0xFFFF00ULL) << 24) |
        ((fieldB & 0xFFFF0000000000ULL) >> 24) | ((fieldB & 0xFFFF00ULL) >> 8);
    }
}

void giddy_say( char *what )
{
  u8 *s, *d;
  int i, j, k, x, y, gx, gy, gw, gh;

  // First print the text
  memset( &texts[164*256*4], 0, 92*256*4 );
  d = &texts[(170*256+6)*4];

  gx = 0;
  gy = 0;
  gw = 0;
  gh = 6;

  for( i=0; what[i]; i++ )
  {
    if( what[i] == '\n' )
    {
      gx = 0;
      gy += 8;
      gh += 8;
      d = &texts[((170+gy)*256+6)*4];
      continue;
    }

    j = (what[i]-32)*6;

    x = j%192;
    y = ((j/192)*8+76)*256;

    s = &texts[(y+x)*4];
    for( y=0; y<(8*256*4); y+=(256*4) )
    {
      for( x=0; x<24; x++ )
        d[y+x] = s[y+x];
    }
    d += 24;
    gx += 6;
    if( gx > gw ) gw = gx;
  }

  // Do the top and bottom
  i = (164*256+6)*4;
  j = ((170+gh+6)*256+6)*4;
  for( x=0; x<gw; x++ )
  {
    for( y=1; y<6; y++ )
    {
      texts[i+0+1024*y] = 0xff; // R
      texts[i+1+1024*y] = 0xff; // G
      texts[i+2+1024*y] = 0xff; // B
      texts[i+3+1024*y] = 0xff; // A
      texts[j+0-1024*y] = 0xff; // R
      texts[j+1-1024*y] = 0xff; // G
      texts[j+2-1024*y] = 0xff; // B
      texts[j+3-1024*y] = 0xff; // A
    }
    texts[i++] =    0; texts[j++] =    0; // R
    texts[i++] =    0; texts[j++] =    0; // G
    texts[i++] =    0; texts[j++] =    0; // B
    texts[i++] = 0xff; texts[j++] = 0xff; // A
  }

  // Do the left and right
  i = (170*256)*4;
  j = (170*256+gw+11)*4;
  for( y=0; y<=gh; y++ )
  {
    for( x=1; x<6; x++ )
    {
      texts[i+0+4*x] = 0xff; // R
      texts[i+1+4*x] = 0xff; // G
      texts[i+2+4*x] = 0xff; // B
      texts[i+3+4*x] = 0xff; // A
      texts[j+0-4*x] = 0xff; // R
      texts[j+1-4*x] = 0xff; // G
      texts[j+2-4*x] = 0xff; // B
      texts[j+3-4*x] = 0xff; // A
    }
    texts[i+0] =    0; texts[j+0] =    0; // R
    texts[i+1] =    0; texts[j+1] =    0; // G
    texts[i+2] =    0; texts[j+2] =    0; // B
    texts[i+3] = 0xff; texts[j+3] = 0xff; // A
    i+=1024;
    j+=1024;
  }

  // Do the corners
  k = (164*256+gw+6)*4;
  j = 164*256*4;
  i = 128*256*4;
  gy = (gh+7)*256*4;
  for( y=0; y<6; y++ )
  {
    for( x=0; x<24; x++ )
    {
      texts[j+gy] = gsprites[i+(10*256*4)];     // bottom left
      texts[k+gy] = gsprites[i+((10*256+8)*4)]; // bottom right
      texts[k++]  = gsprites[i+(8*4)];          // top right
      texts[j++]  = gsprites[i++];              // top left
    }
    i += (256*4)-24;
    j += (256*4)-24;
    k += (256*4)-24;
  }

  // Do the stalk
  if( !gid.flipped )
  {
    i = (124*256+39)*4;
    if( gw > 48 )
      j = ((164+gh+12)*256+(gw/2)+18)*4;
    else
      j = ((164+gh+12)*256+(gw/2)-8)*4;
    for( y=0; y<13; y++ )
    {
      for( x=0; x<56; x++ )
        texts[j+x] = gsprites[i+x];
      i += 256*4;
      j += 256*4;
    }
  } else {
    i = (124*256+39)*4;
    if( gw > 48 )
      j = ((164+gh+12)*256+(gw/2)-5)*4;
    else
      j = ((164+gh+12)*256+(gw/2)+8)*4;
    for( y=0; y<13; y++ )
    {
      for( x=0; x<56; x+=4 )
      {
        texts[j-x+0] = gsprites[i+x+0];
        texts[j-x+1] = gsprites[i+x+1];
        texts[j-x+2] = gsprites[i+x+2];
        texts[j-x+3] = gsprites[i+x+3];
      }
      i += 256*4;
      j += 256*4;
    }
  }

  gid.speakw = gw + 12;
  gid.speakh = gh + 25;
  gid.speakfw = ((float)gw+12)/256.0f;
  gid.speakfh = ((float)gh+25)/256.0f;
  gid.speakc = 140;
  gid.speaka = 180.0f;
  gid.speaksc = 0.1f;
  gid.speakstate = 1;
  if( gid.speakw >= 80 )
  {
    gid.speakxo = 0;
  } else {
    gid.speakxo = (gid.flipped) ? -16 : 16;
  }
  
  strsneedupdate = TRUE;
}

void animate_giddyspeak( void )
{
  BOOL goon;

  switch( gid.speakstate )
  {
    case 1:
      goon = TRUE;
      if( gid.speaka > 0.0f )
      {
        gid.speaka -= 12.0f;
        if( gid.speaka < 0.0f ) gid.speaka = 0.0f;
        goon = FALSE;
      }
      if( gid.speaksc < 1.0f )
      {
        gid.speaksc += 0.06f;
        if( gid.speaksc > 1.0f ) gid.speaksc = 1.0f;
        goon = FALSE;
      }
      if( goon ) gid.speakstate++;
      break;
    
    case 2:
      if( paused )
      {
        gid.speakc = 0;
        break;
      }

      if( gid.speakc > 0 )
      {
        gid.speakc--;
        break;
      }

      gid.speakstate++;
      break;
    
    case 3:
      goon = TRUE;
      if( gid.speaka < 180.0f )
      {
        gid.speaka += 12.0f;
        if( gid.speaka > 180.0f ) gid.speaka = 180.0f;
        goon = FALSE;
      }
      if( gid.speaksc > 0.1f )
      {
        gid.speaksc -= 0.06f;
        if( gid.speaksc < 0.1f ) gid.speaksc = 0.1f;
        goon = FALSE;
      }
      if( goon ) gid.speakstate = 0;
      break;
  }
}

void render_giddyspeak( void )
{
  float tl, tt, tr, tb;
  float vl, vt, vr, vb;
	Mtx m,m1,m2, mv;
	GXTexObj texObj;

  if( gid.speakstate == 0 ) return;

  vl = -gid.speakw/2.0f;    tl =          0.0f;
  vt = -(gid.speakh+20.0f); tt = 164.0f/256.0f;
  vr = gid.speakw/2.0f;     tr =   gid.speakfw;
  vb = -20.0f;              tb = 164.0f/256.0f + gid.speakfh;

  bindtexture( &texObj, mtexts );

	guMtxIdentity( m1 );
  guMtxScaleApply( m1, m1, gid.speaksc, gid.speaksc, 1.0f );
	guMtxRotAxisDeg( m2, &axis, gid.speaka );
	guMtxConcat( m2, m1, m );
  guMtxTransApply( m, m, gid.px-fgx, gid.py-fgy, 0 );
	guMtxConcat( GXmodelView2D, m, mv );
	GX_LoadPosMtxImm( mv, GX_PNMTX0 );

  GX_Begin( GX_QUADS, GX_VTXFMT0, 4 );
    GX_Position3f32( vl+gid.speakxo, vt, 0.0f ); GX_Color4u8( 0xff, 0xff, 0xff, 0xff ); GX_TexCoord2f32( tl, tt );
    GX_Position3f32( vr+gid.speakxo, vt, 0.0f ); GX_Color4u8( 0xff, 0xff, 0xff, 0xff ); GX_TexCoord2f32( tr, tt );
    GX_Position3f32( vr+gid.speakxo, vb, 0.0f ); GX_Color4u8( 0xff, 0xff, 0xff, 0xff ); GX_TexCoord2f32( tr, tb );
    GX_Position3f32( vl+gid.speakxo, vb, 0.0f ); GX_Color4u8( 0xff, 0xff, 0xff, 0xff ); GX_TexCoord2f32( tl, tb );
  GX_End();
}

void set_ibstr( int which, char *txt )
{
  u8 *s, *d;
  int i, j, x, y;

  ibt[which] = txt;
  if( !txt )
  {
    ibw[which] = 0.0f;
    return;
  }

  d = &texts[((which*11)+128)*256*4];
  for( i=0; txt[i]; i++ )
  {
    j = (txt[i]-32)*8;

    x = j&255;
    y = ((j/256)*10+46)*256;

    s = &texts[(y+x)*4];
    for( y=0; y<(10*256*4); y+=(256*4) )
    {
      for( x=0; x<32; x++ )
        d[y+x] = s[y+x];
    }
    d += 32;
  } 

  ibw[which] = (float)i*8;
  ibtw[which] = ibw[which] / 256.0f;
  strsneedupdate = TRUE;
}

void update_ibstrs( void )
{
  if( !strsneedupdate ) return;
  strsneedupdate = FALSE;
  texturemangle( 256, 256, texts, mtexts );
	DCFlushRange( mtexts, 256*256*4 );
  GX_InvalidateTexAll();
}

void animate_invbox( void )
{
  BOOL animdone;
  switch( ibstate )
  {
    case 0:
      gid.usemode = 0;
      ibalpha = 0;
      ibrot   = 180.0f;
      ibscale = 0.2f;
      return;
    
    case 1:
      gid.usemode = 1;
    case 4:
      animdone = TRUE;
      if( ibalpha < 255 )
      {
        ibalpha+=14;
        if( ibalpha > 255 ) ibalpha = 255;
        animdone = FALSE;
      }
      if( ibrot < 360.0f )
      {
        ibrot += 8.0f;
        if( ibrot > 360.0f )
          ibrot = 360.0f;
        animdone = FALSE;
      }
      if( ibscale < 1.0f )
      {
        ibscale += 0.035f;
        if( ibscale > 1.0f )
          ibscale = 1.0f;
        animdone = FALSE;
      }
      if( animdone ) ibstate++;
      break;

    case 2:
      gid.usemode = 1;
      break;

    case 5:
      if( ibwait > 0 )
      {
        ibwait--;
        break;
      }
      ibdest = -81-ibw[1];
      if( ibadd == 0 ) ibadd = -4;
      ibwait = 15;
      ibstate++;
      break;
    
    case 6:
      if( ibwait > 0 )
      {
        ibwait--;
        break;
      }
      ibstate++;
      break;

    case 3:
      gid.usemode = 0;
    case 7:
      animdone = TRUE;
      if( ibalpha > 0 )
      {
        ibalpha-=14;
        if( ibalpha < 0 ) ibalpha = 0;
        animdone = FALSE;
      }
      if( ibrot < 520.0f )
      {
        ibrot += 8.0f;
        if( ibrot > 520.0f )
          ibrot = 520.0f;
        animdone = FALSE;
      }
      if( ibscale > 0.2f )
      {
        ibscale -= 0.035f;
        if( ibscale < 0.2f )
          ibscale = 0.2f;
        animdone = FALSE;
      }
      if( animdone )
      {
        ibstate = 0;
        ibalpha = 0;
        ibrot   = 180.0f;
        ibscale = 0.2f;
      }
      break;
  }
}

void reset_infos( void )
{
  int i;
  for( i=0; infos1[i].x!=-1; i++ )
    infos1[i].active = infos1[i].iactive;
  for( i=0; infos2[i].x!=-1; i++ )
    infos2[i].active = infos2[i].iactive;
  for( i=0; infos3[i].x!=-1; i++ )
    infos3[i].active = infos3[i].iactive;
  for( i=0; infos4[i].x!=-1; i++ )
    infos4[i].active = infos4[i].iactive;
  for( i=0; infos5[i].x!=-1; i++ )
    infos5[i].active = infos5[i].iactive;
}

void check_infos( void )
{
  int i;
  winfo = -1;
  for( i=0; infos[i].x!=-1; i++ )
  {
    if( !infos[i].active ) continue;
    if( (gid.px>=infos[i].x) &&
        (gid.px<(infos[i].x+infos[i].w)) &&
        (gid.py>=infos[i].y) &&
        (gid.py<(infos[i].y+infos[i].h)) )
    {
      winfo = i;
      if( ( clevel == 4 ) && ( winfo == 0 ) )
        triggermrtbubble();

      if( ( clevel == 4 ) && ( winfo == 5 ) )
        triggerbuilderbubble();

      if( ( clevel == 3 ) && ( winfo == 1 ) )
        triggermuldoonandskellybubble();

      if( ( clevel == 5 ) && ( winfo == 5 ) )
        triggercyclingalienbubble();
      return;
    }
  }
}

void reset_thingy_array( struct thingy * t )
{
  int i;
  for( i=0; t[i].ix!=-1; i++ )
  {
    t[i].active = t[i].iactive;
    t[i].x      = t[i].ix;
    t[i].y      = t[i].iy;
    t[i].frame  = 0;
    t[i].framecount = 0;
  }
}

void reset_thingies( void )
{
  reset_thingy_array( n_things1 );
  reset_thingy_array( p_things1 );
  reset_thingy_array( g_things1 );
  reset_thingy_array( n_things2 );
  reset_thingy_array( p_things2 );
  reset_thingy_array( g_things2 );
  reset_thingy_array( n_things3 );
  reset_thingy_array( p_things3 );
  reset_thingy_array( g_things3 );
  reset_thingy_array( n_things4 );
  reset_thingy_array( p_things4 );
  reset_thingy_array( g_things4 );
  reset_thingy_array( n_things5 );
  reset_thingy_array( p_things5 );
  reset_thingy_array( g_things5 );
}

void init_thingies( int levn )
{
  int i;
  struct thingy *nt, *gt, *pt;

  switch( levn )
  {
    case 1: nt = &n_things1[0]; gt = &g_things1[0]; pt = &p_things1[0]; break;
    case 2: nt = &n_things2[0]; gt = &g_things2[0]; pt = &p_things2[0]; break;
    case 3: nt = &n_things3[0]; gt = &g_things3[0]; pt = &p_things3[0]; break;
    case 4: nt = &n_things4[0]; gt = &g_things4[0]; pt = &p_things4[0]; break;
    case 5: nt = &n_things5[0]; gt = &g_things5[0]; pt = &p_things5[0]; break;
    default: return;
  }

  num_bnthings = 0;
  num_bpthings = 0;
  num_bgthings = 0;
  num_nthings = 0;
  num_pthings = 0;
  num_gthings = 0;
 
  for( i=0; nt[i].ix!=-1; i++ )
    if( nt[i].flags & THF_BEHIND )
      bn_things[num_bnthings++] = &nt[i];
    else
      n_things[num_nthings++] = &nt[i];

  for( i=0; pt[i].ix!=-1; i++ )
    if( pt[i].flags & THF_BEHIND )
      bp_things[num_bpthings++] = &pt[i];
    else
      p_things[num_pthings++] = &pt[i];

  for( i=0; gt[i].ix!=-1; i++ )
    if( gt[i].flags & THF_BEHIND )
      bg_things[num_bgthings++] = &gt[i];
    else
      g_things[num_gthings++] = &gt[i];
}

void thingy_bounds_wh( struct thingy *t, struct btex *sl, int *x, int *y, int *w, int *h )
{
  struct btex *s;

  s = &sl[t->frames[t->frame]];
  if( t->flags & THF_CENTREPOS )
  {
    *x = t->x - s->hfw;
    *y = t->y - s->hfh;
    *w = s->fw;
    *h = s->fh;
    return;
  }

  *x = t->x;
  *y = t->y;
  *w = s->fw;
  *h = s->fh;
}

void thingy_bounds( struct thingy *t, struct btex *sl, int *left, int *top, int *right, int *bot )
{
  struct btex *s;

  s = &sl[t->frames[t->frame]];
  if( t->flags & THF_CENTREPOS )
  {
    *left  = t->x - s->hfw;
    *top   = t->y - s->hfh;
    *right = t->x + s->hfw;
    *bot   = t->y + s->hfh;
    return;
  }

  *left  = t->x;
  *top   = t->y;
  *right = t->x+s->fw;
  *bot   = t->y+s->fh;
}

BOOL load_packed( char *filename, u8 *buf, int len )
{
  int s,d,i,j;
  u8 *pb;
  int plen;
  FILE *f;

  f = fopen( filename, "rb" );
  if( !f ) return FALSE;

  fseek( f, 0, SEEK_END );
  plen = ftell( f );
  fseek( f, 0, SEEK_SET );

  pb = malloc( plen );
  if( !pb )
  {
    fclose( f );
    return FALSE;
  }

  fread( pb, plen, 1, f );
  fclose( f );

  s=d=0;
  while( s < plen )
  {
    if( d >= len ) break;

    if( pb[s] == 0xff )
    {
      s++;
      i = pb[s++];
      j = pb[s++];
      while( j > 0 )
      {
        if( d >= len ) { free( pb ); return TRUE; }
        buf[d++] = i;
        j--;
      }
      continue;
    }

    buf[d++] = pb[s++];
  }

  free( pb );
  return TRUE;
};

BOOL load_blocks( char *filename )
{
  FILE *f;

  f = fopen( filename, "rb" );
  if( !f ) return FALSE;

  fread( blocks, 16*16*4, 256, f );
  fclose( f );

  texturemangle( 256, 256, blocks, mblocks );
	DCFlushRange( mblocks, 256*256*4 );
  
  return TRUE;
}

BOOL load_sprites( char *filename )
{
  FILE *f;
  int i, j;

  f = fopen( filename, "rb" );
  if( !f ) return FALSE;

  fread( sprites, 16*16*4, 256, f );
  fclose( f );

  texturemangle( 256, 256, sprites, msprites );

  for( i=0; i<256*256*4; i+=4 )
  {
    j = sprites[i  ]+80; wsprites[i  ] = j>255 ? 255 : j;
    j = sprites[i+1]+80; wsprites[i+1] = j>255 ? 255 : j;
    j = sprites[i+2]+80; wsprites[i+2] = j>255 ? 255 : j;
    wsprites[i+3] = sprites[i+3];
  }

  texturemangle( 256, 256, wsprites, mwsprites );

  DCFlushRange( msprites,  256*256*4 );
  DCFlushRange( mwsprites, 256*256*4 );
  
  return TRUE;
}

BOOL load_puzzlesprites( char *filename )
{
  FILE *f;
  int i, j;

  f = fopen( filename, "rb" );
  if( !f ) return FALSE;

  fread( psprites, 16*16*4, 256, f );
  fclose( f );

  texturemangle( 256, 256, psprites, mpsprites );

  for( i=0; i<256*256*4; i+=4 )
  {
    j = psprites[i  ]+80; wpsprites[i  ] = j>255 ? 255 : j;
    j = psprites[i+1]+80; wpsprites[i+1] = j>255 ? 255 : j;
    j = psprites[i+2]+80; wpsprites[i+2] = j>255 ? 255 : j;
    wpsprites[i+3] = psprites[i+3];
  }

  texturemangle( 256, 256, wpsprites, mwpsprites );

  DCFlushRange( mpsprites,  256*256*4 );
  DCFlushRange( mwpsprites, 256*256*4 );
  
  return TRUE;
}

BOOL load_globalsprites( void )
{
  FILE *f;
  int i, j;

  f = fopen( "/apps/giddy3/hats/sprites.bin", "rb" );
  if( !f ) return FALSE;

  fread( gsprites, 16*16*4, 256, f );
  fclose( f );

  texturemangle( 256, 256, gsprites, mgsprites );

  for( i=0; i<256*256*4; i+=4 )
  {
    j = gsprites[i  ]+80; wgsprites[i  ] = j>255 ? 255 : j;
    j = gsprites[i+1]+80; wgsprites[i+1] = j>255 ? 255 : j;
    j = gsprites[i+2]+80; wgsprites[i+2] = j>255 ? 255 : j;
    wgsprites[i+3] = gsprites[i+3];
  }

  texturemangle( 256, 256, wgsprites, mwgsprites );

  DCFlushRange( mgsprites,  256*256*4 );
  DCFlushRange( mwgsprites, 256*256*4 );

  memset( &texts[128*256*4], 0, 128*256*4 );

  f = fopen( "/apps/giddy3/hats/texts.bin", "rb" );
  if( !f ) return FALSE;

  fread( texts, 256*128*4, 1, f );
  fclose( f );

  texturemangle( 256, 256, texts, mtexts );
  DCFlushRange( mtexts, 256*256*4 );

  return TRUE;
}

void initstars( void )
{
  int i;

  for( i=0; i<MAX_STARS; i++ )
    stars[i].frame = 23;
}

void initincidentals( void )
{
  int i;

  for( i=0; i<MAX_INCIDENTALS; i++ )
  {
    incd[i].numframes = 0;
    incd[i].frame = 0;
    bincd[i].numframes = 0;
    bincd[i].frame = 0;
  }
}

BOOL startincidental( int x, int y, int xm, int ym, int wtex, int *frames, int numframes, int speed )
{
  struct btex *sl, *s;
  u8 *texp;

  switch( wtex )
  {
    case SPRITEX:  sl = sprs;  texp = msprites;  break;
    case PSPRITEX: sl = psprs; texp = mpsprites; break;
    case GSPRITEX: sl = sprtg; texp = mgsprites; break;
    default: return FALSE;
  }

  // Is it even on the screen?
  s = &sl[frames[0]];
  if( ( (x+s->hfw) < (fgx-64) ) ||
      ( (x-s->hfw) > (fgx+384) ) ||
      ( (y+s->hfh) < (fgy-64) ) ||
      ( (y-s->hfh) > (fgy+264) ) )
    return FALSE;

  incd[nextincd].s          = sl;
  incd[nextincd].x          = x;
  incd[nextincd].y          = y;
  incd[nextincd].xm         = xm;
  incd[nextincd].ym         = ym;
  incd[nextincd].numframes  = numframes;
  incd[nextincd].frame      = 0;
  incd[nextincd].framecount = 0;
  incd[nextincd].framespeed = speed;
  incd[nextincd].frames     = frames;
  incd[nextincd].wtex       = wtex;
  incd[nextincd].texp       = texp;
  nextincd = (nextincd+1)%MAX_INCIDENTALS;
  return TRUE;
}

BOOL startbgincidental( int x, int y, int xm, int ym, int wtex, int *frames, int numframes, int speed )
{
  struct btex *sl, *s;
  u8 *texp;

  switch( wtex )
  {
    case SPRITEX:  sl = sprs;  texp = msprites;  break;
    case PSPRITEX: sl = psprs; texp = mpsprites; break;
    case GSPRITEX: sl = sprtg; texp = mgsprites; break;
    default: return FALSE;
  }

  // Is it even on the screen?
  s = &sl[frames[0]];
  if( ( (x+s->hfw) < (bgx-64) ) ||
      ( (x-s->hfw) > (bgx+384) ) ||
      ( (y+s->hfh) < (bgy-64) ) ||
      ( (y-s->hfh) > (bgy+264) ) )
    return FALSE;

  bincd[nextbincd].s          = sl;
  bincd[nextbincd].x          = x;
  bincd[nextbincd].y          = y;
  bincd[nextbincd].xm         = xm;
  bincd[nextbincd].ym         = ym;
  bincd[nextbincd].numframes  = numframes;
  bincd[nextbincd].frame      = 0;
  bincd[nextbincd].framecount = 0;
  bincd[nextbincd].framespeed = speed;
  bincd[nextbincd].frames     = frames;
  bincd[nextbincd].wtex       = wtex;
  bincd[nextbincd].texp       = texp;
  nextbincd = (nextbincd+1)%MAX_INCIDENTALS;
  return TRUE;
}

void setliftdeltas( int ln )
{
  int fx,fy,tx,ty;

  if( lifts[ln].fromstop == -1 )
  {
    lifts[ln].cx = lifts[ln].stops[0]<<8;
    lifts[ln].cy = lifts[ln].stops[1]<<8;
    return;
  }

  fx = lifts[ln].stops[lifts[ln].fromstop*2];
  fy = lifts[ln].stops[lifts[ln].fromstop*2+1];
  tx = lifts[ln].stops[lifts[ln].tostop*2];
  ty = lifts[ln].stops[lifts[ln].tostop*2+1];
  lifts[ln].cx = fx<<8;
  lifts[ln].cy = fy<<8;
  lifts[ln].dx = ((tx-fx)<<8)/lifts[ln].speed;
  lifts[ln].dy = ((ty-fy)<<8)/lifts[ln].speed;
}

int liftloopchan=-1, liftlooplift=-1;
void initlifts( void )
{
  int i;

  liftloopchan = -1;
  liftlooplift = -1;

  for( i=0; lifts[i].numstops!=-1; i++ )
  {
    if( lifts[i].type == LT_TRIGGER )
      lifts[i].fromstop = -1;
    else
      lifts[i].fromstop = 0;
    lifts[i].tostop = 1;
    setliftdeltas( i );
    lifts[i].timeout = lifts[i].itimeout;
    lifts[i].scale = 1.0f;
    lifts[i].dip = 0;

    switch( lifts[i].wtex )
    {
      case SPRITEX:  lifts[i].s = &sprs[lifts[i].sprite];  lifts[i].src = sprites;  lifts[i].texp = msprites;  break;
      case PSPRITEX: lifts[i].s = &psprs[lifts[i].sprite]; lifts[i].src = psprites; lifts[i].texp = mpsprites; break;
      case GSPRITEX: lifts[i].s = &sprtg[lifts[i].sprite]; lifts[i].src = gsprites; lifts[i].texp = mgsprites; break;
      default:       lifts[i].s = NULL;                    lifts[i].src = NULL;     lifts[i].texp = NULL;      break;
    }
  }
}

void initsprings( void )
{
  int i;

  for( i=0; springs[i].x!=-1; i++ )
  {
    springs[i].frame = 0;
    springs[i].rtime = 0;
    switch( springs[i].wtex )
    {
      case SPRITEX:  springs[i].sl = sprs;  springs[i].src = sprites;  springs[i].texp = msprites;  break;
      case PSPRITEX: springs[i].sl = psprs; springs[i].src = psprites; springs[i].texp = mpsprites; break;
      case GSPRITEX: springs[i].sl = sprtg; springs[i].src = gsprites; springs[i].texp = mgsprites; break;
      default:       springs[i].sl = NULL;  springs[i].src = NULL;     springs[i].texp = NULL;      break;
    }
  }
}

BOOL load_level( int levn, BOOL setgiddydir )
{
  char mkstr[64];
  int specialstart;
  BOOL waswarp = FALSE;

  stopallchannels();

  considertardis = FALSE;

  specialstart = levn & 0x8000;
  levn &= 0x7fff;

  mapi =  &mapsizes[levn-1];
  switch( levn )
  {
    case 1:
      sprs    = &sprt1[0];
      psprs   = &psprt1[0];
      lifts   = &lifts1[0];
      infos   = &infos1[0];
      springs = &springs1[0];
      stsc    = &stsc1[0];
      break;

    case 2:
      sprs    = &sprt2[0];
      psprs   = &psprt2[0];
      lifts   = &lifts2[0];
      infos   = &infos2[0];
      springs = &springs2[0];
      stsc    = &stsc2[0];
      break;

    case 3:
      sprs    = &sprt3[0];
      psprs   = &psprt3[0];
      lifts   = &lifts3[0];
      infos   = &infos3[0];
      springs = &springs3[0];
      stsc    = &stsc3[0];
      break;

    case 4:
      sprs    = &sprt4[0];
      psprs   = &psprt4[0];
      lifts   = &lifts4[0];
      infos   = &infos4[0];
      springs = &springs4[0];
      stsc    = &stsc4[0];
      break;

    case 5:
      sprs    = &sprt5[0];
      psprs   = &psprt5[0];
      lifts   = &lifts5[0];
      infos   = &infos5[0];
      springs = &springs5[0];
      stsc    = &stsc5[0];
      break;
  }

  initstars();
  initlifts();
  initsprings();
  initincidentals();
  sprintf( mkstr, "/apps/giddy3/onion%d/blocks.bin", levn );
  if( !load_blocks( mkstr ) ) return FALSE;

  sprintf( mkstr, "/apps/giddy3/onion%d/sprites.bin", levn );
  if( !load_sprites( mkstr ) ) return FALSE;

  sprintf( mkstr, "/apps/giddy3/onion%d/psprites.bin", levn );
  if( !load_puzzlesprites( mkstr ) ) return FALSE;

  sprintf( mkstr, "/apps/giddy3/onion%d/transtab.pak", levn );
  if( !load_packed( mkstr, blktrans, 512 ) ) return FALSE;

  sprintf( mkstr, "/apps/giddy3/onion%d/map1.pak", levn );
  if( !load_packed( mkstr, fgmap, mapi->fgw*mapi->fgh ) ) return FALSE;

  sprintf( mkstr, "/apps/giddy3/onion%d/map2.pak", levn );
  if( !load_packed( mkstr, bgmap, mapi->bgw*mapi->bgh ) ) return FALSE;

  sprintf( mkstr, "/apps/giddy3/onion%d/map3.pak", levn );
  if( !load_packed( mkstr, dmap, 20*14 ) ) return FALSE;

  sprintf( mkstr, "/apps/giddy3/onion%d/tune.mod", levn );
  playtune( mkstr );

  // Make the glow rods deadly
  if( levn == 2 )
  {
    blktrans[194+256] = 0xc;
    blktrans[195+256] = 0xc;
    blktrans[196+256] = 0xc;
  }

  init_thingies( levn );
  initenemies( enemies[levn-1] );
  initenemies( enemiesb[levn-1] );

  fgx = mapi->imapx * 16;
  fgy = mapi->imapy * 16;

  gid.px        = mapi->igidx * 16;
  gid.x         = gid.px<<8;
  gid.py        = mapi->igidy * 16;
  gid.y         = gid.py<<8;
  gid.movedisp  = 0;

  if( specialstart != 0 )
  {
    switch( levn )
    {
      case 1:
        gid.x = 6272<<8;
        gid.px = 6272;
        gid.y = 404<<8;
        gid.py = 404;
        fgx = gid.px-160;
        fgy = gid.py-SCROFF;
        break;
      
      case 4:
        gid.x = 3008<<8;
        gid.px = 3008;
        gid.y = 416<<8;
        gid.py = 416;
        fgx = gid.px-160;
        fgy = gid.py-SCROFF;
        break;
    }
  }

  gid.def = 2;
  if( setgiddydir )
  {
    gid.flipped = FALSE; // in original, it was bit 15 in giddef
    if( ( mapi->igidx - mapi->imapx ) >= 10 )
      gid.flipped = TRUE;
  }

  if( gid.watchwarp )
    waswarp = TRUE;

  gid.jumpcount  = 0;
  gid.jumping    = 0;
  gid.jumpstep   = 6;
  gid.jump       = FALSE;
  gid.jumplatch  = FALSE;
  gid.dazed      = FALSE;
  gid.usemode    = FALSE;
  gid.allowjump  = TRUE;
  gid.nogmxgrav  = FALSE;
  gid.teleport   = FALSE;
  gid.stargatewarp= FALSE;
  gid.watchwarp  = FALSE;
  gid.wwob       = 0.0f;
  gid.onlift     = -1;
  gid.speakc     = 0;
  gid.speakstate = 0;
  gid.rot        = 0;

  stuff.bossmode = FALSE;

  clevel = levn;
  llevel = levn;

  switch( levn )
  {
    case 1:
      initslug();
      initeel();
      initsprinkler();
      initwhackinggreatbomb();
      if( waswarp ) incidentalsound( SND_TELEPORT_IN, sfxvol );
      break;
    
    case 2:
      initjunkchute();
      initrecyclotron();
      initsludgemonster();
      initburstpipe();
      inittoxicgas();
      initpluggrabber();
      initprinter();
      break;

    case 3:
      inittardis();
      initmuldoonandskelly();
      initboulder();
      initdragon();
      initballoon();
      initseesaw();
      initfallingblocks();
      initstargate();
      initspecialfade();
      initboulder2();
      inittimerdoor();
      initlockblockzapper();
      initwallsteppingstones();
      initgummachine();
      break;

    case 4:
      initmrt();
      inittardis();
      initphonebox();
      initfactory();
      initbuilder();
      initcrusher();
      initninja();
      initspecialbin();
      break;
    
    case 5:
      inittripledoors();
      initbigassfan();
      initlaserbeam();
      initbigscreen();
      inittpbubs();
      initbiledude();
      initcyclingalien();
      initflashybuttons();
      initeggsterminatorproductionline();
      incidentalsound( SND_TELEPORT_IN, sfxvol );
      break;
  }

  return TRUE;
};

BOOL cheaty_starty( int levn )
{
  switch( levn )
  {
    case 2:
      // Cheaty starty on level 2
      inv[INV_LARD] = 1;                 // Already got the lard
      g_things1[0].active = FALSE;        // Disable lard object
      strig[ST1_SLUG_MOVE]    = 1;       // Already used the barrel
      strig[ST1_SLUG_BATHING] = 1;
      p_things1[0].active = FALSE;        // Disable barrel object

      // Super cheaty
      //inv[INV_BUBBLEGUM] = 1;
      //inv[INV_BOOT] = 1;
      break;
    
    case 4:
      // Cheaty starty on level 4
      strig[ST2_SLUDGE_RELEASED] = 1;    // Already used the lard
      g_things1[0].active = FALSE;        // Disable lard object
      strig[ST1_SLUG_MOVE]    = 1;       // Already used the barrel
      strig[ST1_SLUG_BATHING] = 1;
      p_things1[0].active = FALSE;        // Disable barrel object
      inv[INV_ELECTRICALTOOLKIT] = 1;   // Already got the electrical toolkit
      g_things2[3].active = FALSE;
      gid.coins = 190;
      gid.fallpuffs = TRUE;
      break;

    case 3:
      // Cheaty starty on level 3
      strig[ST4_PHONEBOX_FIXED] = 1;    // Already used the electrical toolkit
      g_things2[3].active = FALSE;        // disable toolkit object
      strig[ST2_SLUDGE_RELEASED] = 1;    // Already used the lard
      g_things1[0].active = FALSE;        // Disable lard object
      strig[ST1_SLUG_MOVE]    = 1;       // Already used the barrel
      strig[ST1_SLUG_BATHING] = 1;
      p_things1[0].active = FALSE;        // Disable barrel object
      inv[INV_PLANK] = 1;                // Already got the sturdy plank
      g_things4[1].active = FALSE;
      break;
    
    case 0x8004: // Level 4, second time around
      // Cheaty starty on level 4
      strig[ST4_PHONEBOX_FIXED] = 1;    // Already used the electrical toolkit
      g_things2[3].active = FALSE;        // disable toolkit object
      strig[ST2_SLUDGE_RELEASED] = 1;    // Already used the lard
      g_things1[0].active = FALSE;        // Disable lard object
      strig[ST1_SLUG_MOVE]    = 1;       // Already used the barrel
      strig[ST1_SLUG_BATHING] = 1;
      p_things1[0].active = FALSE;        // Disable barrel object
      inv[INV_PLANK] = 1;                // Already got the sturdy plank
      g_things4[1].active = FALSE;
      inv[INV_DETONATOR] = 1;          // Already got the detonator
      g_things3[2].active = FALSE;
      break;
    
    case 0x8003: // Level 3, second time around
      // Cheaty starty on level 3
      strig[ST4_PHONEBOX_FIXED] = 1;    // Already used the electrical toolkit
      g_things2[3].active = FALSE;        // disable toolkit object
      strig[ST2_SLUDGE_RELEASED] = 1;    // Already used the lard
      g_things1[0].active = FALSE;        // Disable lard object
      strig[ST1_SLUG_MOVE]    = 1;       // Already used the barrel
      strig[ST1_SLUG_BATHING] = 1;
      p_things1[0].active = FALSE;        // Disable barrel object
      inv[INV_PLANK] = 1;                // Already got the sturdy plank
      g_things4[1].active = FALSE;
      strig[ST4_DETONATOR_PLACED] = 1; // Already used the detonator
      strig[ST4_FACTORY_BLOWN] = 1;
      g_things3[2].active = FALSE;
      inv[INV_LARGECOG] = 1;           // Already got the cog
      inv[INV_SCISSORS] = 1;           // Already got the scissors
      g_things3[0].active = FALSE;
      inv[INV_CATAPULT] = 1;           // Already got the catapult
      g_things3[4].active = FALSE;
      break;

    case 0x18004: // Level 4, third time around
      // Cheaty starty on level 4
      strig[ST4_PHONEBOX_FIXED] = 1;    // Already used the electrical toolkit
      g_things2[3].active = FALSE;        // disable toolkit object
      strig[ST2_SLUDGE_RELEASED] = 1;    // Already used the lard
      g_things1[0].active = FALSE;        // Disable lard object
      strig[ST1_SLUG_MOVE]    = 1;       // Already used the barrel
      strig[ST1_SLUG_BATHING] = 1;
      p_things1[0].active = FALSE;        // Disable barrel object
      inv[INV_PLANK] = 1;                // Already got the sturdy plank
      g_things4[1].active = FALSE;
      strig[ST4_DETONATOR_PLACED] = 1; // Already used the detonator
      strig[ST4_FACTORY_BLOWN] = 1;
      g_things3[2].active = FALSE;
      inv[INV_LARGECOG] = 1;           // Already got the cog
      inv[INV_SCISSORS] = 1;           // Already got the scissors
      g_things3[0].active = FALSE;
      g_things3[4].active = FALSE;     // Already used the catapult
      strig[ST3_BALLOON_POPPED] = 1;
      strig[ST3_GIVEN_BALLOON] = 1;
      inv[INV_ATEAMPHONENO] = 1;
      gid.coins = 231;
      break;

    case 0x18003: // Level 3, third time around
      // Cheaty starty on level 3
      strig[ST4_PHONEBOX_FIXED] = 1;    // Already used the electrical toolkit
      g_things2[3].active = FALSE;        // disable toolkit object
      strig[ST2_SLUDGE_RELEASED] = 1;    // Already used the lard
      g_things1[0].active = FALSE;        // Disable lard object
      strig[ST1_SLUG_MOVE]    = 1;       // Already used the barrel
      strig[ST1_SLUG_BATHING] = 1;
      p_things1[0].active = FALSE;        // Disable barrel object
      inv[INV_PLANK] = 1;                // Already got the sturdy plank
      g_things4[1].active = FALSE;
      strig[ST4_DETONATOR_PLACED] = 1; // Already used the detonator
      strig[ST4_FACTORY_BLOWN] = 1;
      g_things3[2].active = FALSE;
      inv[INV_LARGECOG] = 1;           // Already got the cog
      inv[INV_SCISSORS] = 1;           // Already got the scissors
      g_things3[0].active = FALSE;
      g_things3[4].active = FALSE;     // Already used the catapult
      strig[ST3_BALLOON_POPPED] = 1;
      strig[ST3_GIVEN_BALLOON] = 1;
      strig[ST4_ATEAM_CALLED] = 1;     // A-Teams phone number already used
      inv[INV_FLATBATTERY] = 1;        // Got the battery
      p_things4[43].active = FALSE;
//      strig[ST3_TORCHES_LIT] = 1;
      break;

    case 0x5: // Level 5
      // Cheaty starty on level 5
      strig[ST4_PHONEBOX_FIXED] = 1;    // Already used the electrical toolkit
      g_things2[3].active = FALSE;        // disable toolkit object
      strig[ST2_SLUDGE_RELEASED] = 1;    // Already used the lard
      g_things1[0].active = FALSE;        // Disable lard object
      strig[ST1_SLUG_MOVE]    = 1;       // Already used the barrel
      strig[ST1_SLUG_BATHING] = 1;
      p_things1[0].active = FALSE;        // Disable barrel object
      strig[ST4_DETONATOR_PLACED] = 1; // Already used the detonator
      strig[ST4_FACTORY_BLOWN] = 1;
      g_things3[2].active = FALSE;
      strig[ST3_SEESAW_SEESAWED] = 1;  // Already used the plank and scissors and that
      strig[ST3_PLANK_PLACED] = 1;
      g_things3[0].active = FALSE;
      g_things4[1].active = FALSE;
      inv[INV_AIRHORN] = 1;            // Already got the airhorn
      g_things3[9].active = FALSE;
      inv[INV_LARGECOG] = 1;           // Already got the cog
      g_things3[4].active = FALSE;     // Already used the catapult
      strig[ST3_BALLOON_POPPED] = 1;
      strig[ST3_GIVEN_BALLOON] = 1;
      strig[ST4_ATEAM_CALLED] = 1;     // A-Teams phone number already used
      inv[INV_FLATBATTERY] = 1;        // Got the battery
      p_things4[43].active = FALSE;
      inv[INV_CANDLESTICK] = 1;        // Got the candlestick
      g_things3[10].active = FALSE;
      break;

    case 0x10001: // Level 1, second time around
      // Cheaty starty on level 1
      strig[ST4_PHONEBOX_FIXED] = 1;    // Already used the electrical toolkit
      g_things2[3].active = FALSE;        // disable toolkit object
      strig[ST2_SLUDGE_RELEASED] = 1;    // Already used the lard
      g_things1[0].active = FALSE;        // Disable lard object
      strig[ST1_SLUG_MOVE]    = 1;       // Already used the barrel
      strig[ST1_SLUG_BATHING] = 1;
      p_things1[0].active = FALSE;        // Disable barrel object
      strig[ST4_DETONATOR_PLACED] = 1; // Already used the detonator
      strig[ST4_FACTORY_BLOWN] = 1;
      g_things3[2].active = FALSE;
      strig[ST3_SEESAW_SEESAWED] = 1;  // Already used the plank and scissors and that
      strig[ST3_PLANK_PLACED] = 1;
      g_things3[0].active = FALSE;
      g_things4[1].active = FALSE;
      inv[INV_AIRHORN] = 1;            // Already got the airhorn
      g_things3[9].active = FALSE;
      inv[INV_LARGECOG] = 1;           // Already got the cog
      g_things3[4].active = FALSE;     // Already used the catapult
      strig[ST3_BALLOON_POPPED] = 1;
      strig[ST3_GIVEN_BALLOON] = 1;
      strig[ST4_ATEAM_CALLED] = 1;     // A-Teams phone number already used
      inv[INV_FLATBATTERY] = 1;        // Got the battery
      p_things4[43].active = FALSE;
      inv[INV_CANDLESTICK] = 1;        // Got the candlestick
      g_things3[10].active = FALSE;
      inv[INV_CD] = 1;                 // Got the CD
      g_things5[7].active = FALSE;
      inv[INV_TURPS] = 1;              // Got the turps
      g_things5[6].active = FALSE;
      inv[INV_TELEPORTERWATCH] = 1;    // Got the teleporter watch
      g_things5[0].active = FALSE;
      break;

    case 0x8002: // Level 2, second time around
      // Cheaty starty on level 2
      strig[ST4_PHONEBOX_FIXED] = 1;    // Already used the electrical toolkit
      g_things2[3].active = FALSE;        // disable toolkit object
      strig[ST2_SLUDGE_RELEASED] = 1;    // Already used the lard
      g_things1[0].active = FALSE;        // Disable lard object
      strig[ST1_SLUG_MOVE]    = 1;       // Already used the barrel
      strig[ST1_SLUG_BATHING] = 1;
      p_things1[0].active = FALSE;        // Disable barrel object
      strig[ST4_DETONATOR_PLACED] = 1; // Already used the detonator
      strig[ST4_FACTORY_BLOWN] = 1;
      g_things3[2].active = FALSE;
      strig[ST3_SEESAW_SEESAWED] = 1;  // Already used the plank and scissors and that
      strig[ST3_PLANK_PLACED] = 1;
      g_things3[0].active = FALSE;
      g_things4[1].active = FALSE;
      inv[INV_AIRHORN] = 1;            // Already got the airhorn
      g_things3[9].active = FALSE;
      inv[INV_LARGECOG] = 1;           // Already got the cog
      g_things3[4].active = FALSE;     // Already used the catapult
      strig[ST3_BALLOON_POPPED] = 1;
      strig[ST3_GIVEN_BALLOON] = 1;
      strig[ST4_ATEAM_CALLED] = 1;     // A-Teams phone number already used
      inv[INV_FLATBATTERY] = 1;        // Got the battery
      p_things4[43].active = FALSE;
      inv[INV_CANDLESTICK] = 1;        // Got the candlestick
      g_things3[10].active = FALSE;
      inv[INV_TURPS] = 1;              // Got the turps
      g_things5[6].active = FALSE;
      inv[INV_TELEPORTERWATCH] = 1;    // Got the teleporter watch
      g_things5[0].active = FALSE;
      strig[ST1_COLIN_CHEERED_UP] = 1; // Used the CD
      g_things5[7].active = FALSE;
      inv[INV_LUMPOFCARBON] = 1;       // Got the carbon
      g_things1[7].active = FALSE;
      break;

    case 0x20004: // Level 4, fourth time around
      // Cheaty starty on level 4
      strig[ST4_PHONEBOX_FIXED] = 1;    // Already used the electrical toolkit
      g_things2[3].active = FALSE;        // disable toolkit object
      strig[ST2_SLUDGE_RELEASED] = 1;    // Already used the lard
      g_things1[0].active = FALSE;        // Disable lard object
      strig[ST1_SLUG_MOVE]    = 1;       // Already used the barrel
      strig[ST1_SLUG_BATHING] = 1;
      p_things1[0].active = FALSE;        // Disable barrel object
      strig[ST4_DETONATOR_PLACED] = 1; // Already used the detonator
      strig[ST4_FACTORY_BLOWN] = 1;
      g_things3[2].active = FALSE;
      strig[ST3_SEESAW_SEESAWED] = 1;  // Already used the plank and scissors and that
      strig[ST3_PLANK_PLACED] = 1;
      g_things3[0].active = FALSE;
      g_things4[1].active = FALSE;
      inv[INV_AIRHORN] = 1;            // Already got the airhorn
      g_things3[9].active = FALSE;
      g_things3[4].active = FALSE;     // Already used the catapult
      strig[ST3_BALLOON_POPPED] = 1;
      strig[ST3_GIVEN_BALLOON] = 1;
      strig[ST4_ATEAM_CALLED] = 1;     // A-Teams phone number already used
      inv[INV_FLATBATTERY] = 1;        // Got the battery
      p_things4[43].active = FALSE;
      inv[INV_CANDLESTICK] = 1;        // Got the candlestick
      g_things3[10].active = FALSE;
      inv[INV_TURPS] = 1;              // Got the turps
      g_things5[6].active = FALSE;
      inv[INV_TELEPORTERWATCH] = 1;    // Got the teleporter watch
      g_things5[0].active = FALSE;
      strig[ST1_COLIN_CHEERED_UP] = 1; // Used the CD
      g_things5[7].active = FALSE;
      inv[INV_LUMPOFCARBON] = 1;       // Got the carbon
      g_things1[7].active = FALSE;
      strig[ST2_RECYCLOTRON_REPAIRED] = 1; // Used the cog
      strig[ST2_CAMERA_COLLECTED] = 1;
      inv[INV_DIGITALCAMERA] = 1;      // Got the camera
      break;

    case 0x20003: // Level 3, fourth time around
      // Cheaty starty on level 3
      strig[ST4_PHONEBOX_FIXED] = 1;    // Already used the electrical toolkit
      g_things2[3].active = FALSE;        // disable toolkit object
      strig[ST2_SLUDGE_RELEASED] = 1;    // Already used the lard
      g_things1[0].active = FALSE;        // Disable lard object
      strig[ST1_SLUG_MOVE]    = 1;       // Already used the barrel
      strig[ST1_SLUG_BATHING] = 1;
      p_things1[0].active = FALSE;        // Disable barrel object
      strig[ST4_DETONATOR_PLACED] = 1; // Already used the detonator
      strig[ST4_FACTORY_BLOWN] = 1;
      g_things3[2].active = FALSE;
      strig[ST3_SEESAW_SEESAWED] = 1;  // Already used the plank and scissors and that
      strig[ST3_PLANK_PLACED] = 1;
      g_things3[0].active = FALSE;
      g_things4[1].active = FALSE;
      g_things3[4].active = FALSE;     // Already used the catapult
      strig[ST3_BALLOON_POPPED] = 1;
      strig[ST3_GIVEN_BALLOON] = 1;
      strig[ST4_ATEAM_CALLED] = 1;     // A-Teams phone number already used
      inv[INV_FLATBATTERY] = 1;        // Got the battery
      p_things4[43].active = FALSE;
      strig[ST1_COLIN_CHEERED_UP] = 1; // Used the CD
      g_things5[7].active = FALSE;
      strig[ST2_RECYCLOTRON_REPAIRED] = 1; // Used the cog
      strig[ST2_CAMERA_COLLECTED] = 1;
      inv[INV_DIGITALCAMERA] = 1;      // Got the camera
      g_things1[7].active = FALSE;     // Used the carbon
      strig[ST4_CARBON_PLACED] = 1;
      strig[ST4_DIAMOND_COLLECTED] = 1;
      inv[INV_DIAMOND] = 1;
      inv[INV_TELEPORTERWATCH] = 1;    // Got the teleporter watch
      g_things5[0].active = FALSE;
      inv[INV_AIRHORN] = 1;            // Already got the airhorn
      g_things3[9].active = FALSE;
      inv[INV_TURPS] = 1;              // Got the turps
      g_things5[6].active = FALSE;
      inv[INV_CANDLESTICK] = 1;        // Got the candlestick
      g_things3[10].active = FALSE;
      gid.coins = 240;
      inv[INV_COINS] = 1;
      break;

    case 0x10005: // Level 5, second time around
      // Cheaty starty on level 5
      strig[ST4_PHONEBOX_FIXED] = 1;    // Already used the electrical toolkit
      g_things2[3].active = FALSE;        // disable toolkit object
      strig[ST2_SLUDGE_RELEASED] = 1;    // Already used the lard
      g_things1[0].active = FALSE;        // Disable lard object
      strig[ST1_SLUG_MOVE]    = 1;       // Already used the barrel
      strig[ST1_SLUG_BATHING] = 1;
      p_things1[0].active = FALSE;        // Disable barrel object
      strig[ST4_DETONATOR_PLACED] = 1; // Already used the detonator
      strig[ST4_FACTORY_BLOWN] = 1;
      g_things3[2].active = FALSE;
      strig[ST3_SEESAW_SEESAWED] = 1;  // Already used the plank and scissors and that
      strig[ST3_PLANK_PLACED] = 1;
      g_things3[0].active = FALSE;
      g_things4[1].active = FALSE;
      g_things3[4].active = FALSE;     // Already used the catapult
      strig[ST3_BALLOON_POPPED] = 1;
      strig[ST3_GIVEN_BALLOON] = 1;
      strig[ST4_ATEAM_CALLED] = 1;     // A-Teams phone number already used
      inv[INV_FLATBATTERY] = 1;        // Got the battery
      p_things4[43].active = FALSE;
      strig[ST1_COLIN_CHEERED_UP] = 1; // Used the CD
      g_things5[7].active = FALSE;
      strig[ST2_RECYCLOTRON_REPAIRED] = 1; // Used the cog
      strig[ST2_CAMERA_COLLECTED] = 1;
      inv[INV_DIGITALCAMERA] = 1;      // Got the camera
      g_things1[7].active = FALSE;     // Used the carbon
      strig[ST4_CARBON_PLACED] = 1;
      strig[ST4_DIAMOND_COLLECTED] = 1;
      inv[INV_TELEPORTERWATCH] = 1;    // Got the teleporter watch
      g_things5[0].active = FALSE;
      g_things3[9].active = FALSE;     // Used the airhorn
      g_things5[6].active = FALSE;     // Used the turps
      g_things3[10].active = FALSE;    // Used the candlestick
      strig[ST3_DRAGON_HONKED] = 1;
      strig[ST3_DRAGON_DRUNK] = 1;
      strig[ST3_CANDLE_LIT] = 1;
      strig[ST3_CANDLE_COLLECTED] = 1; // Got the lit candle
      strig[ST3_TORCHES_LIT] = 1;      // .. and used it
      strig[ST3_DIAMOND_PLACED] = 1;   // Used the diamond
      inv[INV_HOSEPIPE] = 1;           // Got the hosepipe
      g_things3[17].active = FALSE;
      inv[INV_INDIGESTIONPILLS] = 1;   // Got the indigestion pills
      g_things3[19].active = FALSE;
      strig[ST3_GUM_BOUGHT] = 1;
      strig[ST3_GUM_COLLECTED] = 1;    // Got the gum
      inv[INV_BUBBLEGUM] = 1;
      break;

    case 0x20001: // Level 1, third time around
      // Cheaty starty on level 1
      strig[ST4_PHONEBOX_FIXED] = 1;    // Already used the electrical toolkit
      g_things2[3].active = FALSE;        // disable toolkit object
      strig[ST2_SLUDGE_RELEASED] = 1;    // Already used the lard
      g_things1[0].active = FALSE;        // Disable lard object
      strig[ST1_SLUG_MOVE]    = 1;       // Already used the barrel
      strig[ST1_SLUG_BATHING] = 1;
      p_things1[0].active = FALSE;        // Disable barrel object
      strig[ST4_DETONATOR_PLACED] = 1; // Already used the detonator
      strig[ST4_FACTORY_BLOWN] = 1;
      g_things3[2].active = FALSE;
      strig[ST3_SEESAW_SEESAWED] = 1;  // Already used the plank and scissors and that
      strig[ST3_PLANK_PLACED] = 1;
      g_things3[0].active = FALSE;
      g_things4[1].active = FALSE;
      g_things3[4].active = FALSE;     // Already used the catapult
      strig[ST3_BALLOON_POPPED] = 1;
      strig[ST3_GIVEN_BALLOON] = 1;
      strig[ST4_ATEAM_CALLED] = 1;     // A-Teams phone number already used
      p_things4[43].active = FALSE;
      strig[ST1_COLIN_CHEERED_UP] = 1; // Used the CD
      g_things5[7].active = FALSE;
      strig[ST2_RECYCLOTRON_REPAIRED] = 1; // Used the cog
      strig[ST2_CAMERA_COLLECTED] = 1;
      g_things1[7].active = FALSE;     // Used the carbon
      strig[ST4_CARBON_PLACED] = 1;
      strig[ST4_DIAMOND_COLLECTED] = 1;
      inv[INV_TELEPORTERWATCH] = 1;    // Got the teleporter watch
      g_things5[0].active = FALSE;
      g_things3[9].active = FALSE;     // Used the airhorn
      g_things5[6].active = FALSE;     // Used the turps
      g_things3[10].active = FALSE;    // Used the candlestick
      strig[ST3_DRAGON_HONKED] = 1;
      strig[ST3_DRAGON_DRUNK] = 1;
      strig[ST3_CANDLE_LIT] = 1;
      strig[ST3_CANDLE_COLLECTED] = 1; // Got the lit candle
      strig[ST3_TORCHES_LIT] = 1;      // .. and used it
      strig[ST3_DIAMOND_PLACED] = 1;   // Used the diamond
      inv[INV_HOSEPIPE] = 1;           // Got the hosepipe
      g_things3[17].active = FALSE;
      g_things3[19].active = FALSE;   // Used the indigestion pills
      strig[ST5_INDIGESTION_CURED] = 1;
      strig[ST3_GUM_BOUGHT] = 1;
      strig[ST3_GUM_COLLECTED] = 1;    // Got the gum
      inv[INV_BUBBLEGUM] = 1;
      strig[ST5_BATTERY_CHARGED] = 1;  // Charged the battery
      strig[ST5_BATTERY_COLLECTED] = 1; // And grabbed it
      inv[INV_CHARGEDBATTERY] = 1;
      strig[ST5_PHOTO_TAKEN] = 1;      // Got the photo
      inv[INV_CAMERAWITHPHOTO] = 1;
      break;

    case 0x10002: // Level 2, third time around
      // Cheaty starty on level 2
      strig[ST4_PHONEBOX_FIXED] = 1;    // Already used the electrical toolkit
      g_things2[3].active = FALSE;        // disable toolkit object
      strig[ST2_SLUDGE_RELEASED] = 1;    // Already used the lard
      g_things1[0].active = FALSE;        // Disable lard object
      strig[ST1_SLUG_MOVE]    = 1;       // Already used the barrel
      strig[ST1_SLUG_BATHING] = 1;
      p_things1[0].active = FALSE;        // Disable barrel object
      strig[ST4_DETONATOR_PLACED] = 1; // Already used the detonator
      strig[ST4_FACTORY_BLOWN] = 1;
      g_things3[2].active = FALSE;
      strig[ST3_SEESAW_SEESAWED] = 1;  // Already used the plank and scissors and that
      strig[ST3_PLANK_PLACED] = 1;
      g_things3[0].active = FALSE;
      g_things4[1].active = FALSE;
      g_things3[4].active = FALSE;     // Already used the catapult
      strig[ST3_BALLOON_POPPED] = 1;
      strig[ST3_GIVEN_BALLOON] = 1;
      strig[ST4_ATEAM_CALLED] = 1;     // A-Teams phone number already used
      p_things4[43].active = FALSE;
      strig[ST1_COLIN_CHEERED_UP] = 1; // Used the CD
      g_things5[7].active = FALSE;
      strig[ST2_RECYCLOTRON_REPAIRED] = 1; // Used the cog
      strig[ST2_CAMERA_COLLECTED] = 1;
      g_things1[7].active = FALSE;     // Used the carbon
      strig[ST4_CARBON_PLACED] = 1;
      strig[ST4_DIAMOND_COLLECTED] = 1;
      inv[INV_TELEPORTERWATCH] = 1;    // Got the teleporter watch
      g_things5[0].active = FALSE;
      g_things3[9].active = FALSE;     // Used the airhorn
      g_things5[6].active = FALSE;     // Used the turps
      g_things3[10].active = FALSE;    // Used the candlestick
      strig[ST3_DRAGON_HONKED] = 1;
      strig[ST3_DRAGON_DRUNK] = 1;
      strig[ST3_CANDLE_LIT] = 1;
      strig[ST3_CANDLE_COLLECTED] = 1; // Got the lit candle
      strig[ST3_TORCHES_LIT] = 1;      // .. and used it
      strig[ST3_DIAMOND_PLACED] = 1;   // Used the diamond
      g_things3[19].active = FALSE;   // Used the indigestion pills
      strig[ST5_INDIGESTION_CURED] = 1;
      strig[ST3_GUM_BOUGHT] = 1;
      strig[ST3_GUM_COLLECTED] = 1;    // Got the gum
      inv[INV_BUBBLEGUM] = 1;
      strig[ST5_BATTERY_CHARGED] = 1;  // Charged the battery
      strig[ST5_BATTERY_COLLECTED] = 1; // And grabbed it
      inv[INV_CHARGEDBATTERY] = 1;
      strig[ST5_PHOTO_TAKEN] = 1;      // Got the photo
      inv[INV_CAMERAWITHPHOTO] = 1;
      g_things3[17].active = FALSE;    // Got and used the hosepipe
      strig[ST1_HOSE_PLACED] = 1;
      strig[ST1_BOSS_BEATEN] = 1;
      strig[ST1_BOOT_COLLECTED] = 1;
      inv[INV_BOOT] = 1;
      break;

    case 0x30004: // Level 4, fifth time around
      // Cheaty starty on level 4
      strig[ST4_PHONEBOX_FIXED] = 1;    // Already used the electrical toolkit
      g_things2[3].active = FALSE;        // disable toolkit object
      strig[ST2_SLUDGE_RELEASED] = 1;    // Already used the lard
      g_things1[0].active = FALSE;        // Disable lard object
      strig[ST1_SLUG_MOVE]    = 1;       // Already used the barrel
      strig[ST1_SLUG_BATHING] = 1;
      p_things1[0].active = FALSE;        // Disable barrel object
      strig[ST4_DETONATOR_PLACED] = 1; // Already used the detonator
      strig[ST4_FACTORY_BLOWN] = 1;
      g_things3[2].active = FALSE;
      strig[ST3_SEESAW_SEESAWED] = 1;  // Already used the plank and scissors and that
      strig[ST3_PLANK_PLACED] = 1;
      g_things3[0].active = FALSE;
      g_things4[1].active = FALSE;
      g_things3[4].active = FALSE;     // Already used the catapult
      strig[ST3_BALLOON_POPPED] = 1;
      strig[ST3_GIVEN_BALLOON] = 1;
      strig[ST4_ATEAM_CALLED] = 1;     // A-Teams phone number already used
      p_things4[43].active = FALSE;
      strig[ST1_COLIN_CHEERED_UP] = 1; // Used the CD
      g_things5[7].active = FALSE;
      strig[ST2_RECYCLOTRON_REPAIRED] = 1; // Used the cog
      strig[ST2_CAMERA_COLLECTED] = 1;
      g_things1[7].active = FALSE;     // Used the carbon
      strig[ST4_CARBON_PLACED] = 1;
      strig[ST4_DIAMOND_COLLECTED] = 1;
      inv[INV_TELEPORTERWATCH] = 1;    // Got the teleporter watch
      g_things5[0].active = FALSE;
      g_things3[9].active = FALSE;     // Used the airhorn
      g_things5[6].active = FALSE;     // Used the turps
      g_things3[10].active = FALSE;    // Used the candlestick
      strig[ST3_DRAGON_HONKED] = 1;
      strig[ST3_DRAGON_DRUNK] = 1;
      strig[ST3_CANDLE_LIT] = 1;
      strig[ST3_CANDLE_COLLECTED] = 1; // Got the lit candle
      strig[ST3_TORCHES_LIT] = 1;      // .. and used it
      strig[ST3_DIAMOND_PLACED] = 1;   // Used the diamond
      g_things3[19].active = FALSE;   // Used the indigestion pills
      strig[ST5_INDIGESTION_CURED] = 1;
      strig[ST3_GUM_BOUGHT] = 1;
      strig[ST3_GUM_COLLECTED] = 1;    // Got the gum
      strig[ST5_BATTERY_CHARGED] = 1;  // Charged the battery
      strig[ST5_BATTERY_COLLECTED] = 1; // And grabbed it
      inv[INV_CHARGEDBATTERY] = 1;
      strig[ST5_PHOTO_TAKEN] = 1;      // Got the photo
      g_things3[17].active = FALSE;    // Got and used the hosepipe
      strig[ST1_HOSE_PLACED] = 1;
      strig[ST1_BOSS_BEATEN] = 1;
      strig[ST1_BOOT_COLLECTED] = 1;
      strig[ST2_JCS_BOOTED] = 1;
      strig[ST2_PLUG_GRABBED] = 1;  // Grabbed it!
      p_things2[105].active = FALSE;
      inv[INV_HARDHAT] = 1;  // got the hard hat
      g_things2[10].active = FALSE;
      strig[ST2_PIPE_SEALED] = 1; // gum used
      strig[ST2_PHOTO_PRINTED] = 1;
      strig[ST2_PHOTO_COLLECTED] = 1;
      inv[INV_PRINTEDPHOTO] = 1;
      break;

    case 0x20005: // Level 5, third time around
      // Cheaty starty on level 5
      strig[ST4_PHONEBOX_FIXED] = 1;    // Already used the electrical toolkit
      g_things2[3].active = FALSE;        // disable toolkit object
      strig[ST2_SLUDGE_RELEASED] = 1;    // Already used the lard
      g_things1[0].active = FALSE;        // Disable lard object
      strig[ST1_SLUG_MOVE]    = 1;       // Already used the barrel
      strig[ST1_SLUG_BATHING] = 1;
      p_things1[0].active = FALSE;        // Disable barrel object
      strig[ST4_DETONATOR_PLACED] = 1; // Already used the detonator
      strig[ST4_FACTORY_BLOWN] = 1;
      g_things3[2].active = FALSE;
      strig[ST3_SEESAW_SEESAWED] = 1;  // Already used the plank and scissors and that
      strig[ST3_PLANK_PLACED] = 1;
      g_things3[0].active = FALSE;
      g_things4[1].active = FALSE;
      g_things3[4].active = FALSE;     // Already used the catapult
      strig[ST3_BALLOON_POPPED] = 1;
      strig[ST3_GIVEN_BALLOON] = 1;
      strig[ST4_ATEAM_CALLED] = 1;     // A-Teams phone number already used
      p_things4[43].active = FALSE;
      strig[ST1_COLIN_CHEERED_UP] = 1; // Used the CD
      g_things5[7].active = FALSE;
      strig[ST2_RECYCLOTRON_REPAIRED] = 1; // Used the cog
      strig[ST2_CAMERA_COLLECTED] = 1;
      g_things1[7].active = FALSE;     // Used the carbon
      strig[ST4_CARBON_PLACED] = 1;
      strig[ST4_DIAMOND_COLLECTED] = 1;
      inv[INV_TELEPORTERWATCH] = 1;    // Got the teleporter watch
      g_things5[0].active = FALSE;
      g_things3[9].active = FALSE;     // Used the airhorn
      g_things5[6].active = FALSE;     // Used the turps
      g_things3[10].active = FALSE;    // Used the candlestick
      strig[ST3_DRAGON_HONKED] = 1;
      strig[ST3_DRAGON_DRUNK] = 1;
      strig[ST3_CANDLE_LIT] = 1;
      strig[ST3_CANDLE_COLLECTED] = 1; // Got the lit candle
      strig[ST3_TORCHES_LIT] = 1;      // .. and used it
      strig[ST3_DIAMOND_PLACED] = 1;   // Used the diamond
      g_things3[19].active = FALSE;   // Used the indigestion pills
      strig[ST5_INDIGESTION_CURED] = 1;
      strig[ST3_GUM_BOUGHT] = 1;
      strig[ST3_GUM_COLLECTED] = 1;    // Got the gum
      strig[ST5_BATTERY_CHARGED] = 1;  // Charged the battery
      strig[ST5_BATTERY_COLLECTED] = 1; // And grabbed it
      inv[INV_CHARGEDBATTERY] = 1;
      strig[ST5_PHOTO_TAKEN] = 1;      // Got the photo
      g_things3[17].active = FALSE;    // Got and used the hosepipe
      strig[ST1_HOSE_PLACED] = 1;
      strig[ST1_BOSS_BEATEN] = 1;
      strig[ST1_BOOT_COLLECTED] = 1;
      strig[ST2_JCS_BOOTED] = 1;
      strig[ST2_PLUG_GRABBED] = 1;  // Grabbed it!
      p_things2[105].active = FALSE;
      gid.hardhat = TRUE; // used the hardhat
      g_things2[10].active = FALSE;
      strig[ST2_PIPE_SEALED] = 1; // gum used
      strig[ST2_PHOTO_PRINTED] = 1;
      strig[ST2_PHOTO_COLLECTED] = 1;
      inv[INV_PRINTEDPHOTO] = 1;
      inv[INV_SPADE] = 1;   // got the spade
      g_things4[5].active = FALSE;
      g_things4[6].active = FALSE;
      break;

    case 0x30001: // Level 1, fourth time around
      // Cheaty starty on level 1
      strig[ST4_PHONEBOX_FIXED] = 1;    // Already used the electrical toolkit
      g_things2[3].active = FALSE;        // disable toolkit object
      strig[ST2_SLUDGE_RELEASED] = 1;    // Already used the lard
      g_things1[0].active = FALSE;        // Disable lard object
      strig[ST1_SLUG_MOVE]    = 1;       // Already used the barrel
      strig[ST1_SLUG_BATHING] = 1;
      p_things1[0].active = FALSE;        // Disable barrel object
      strig[ST4_DETONATOR_PLACED] = 1; // Already used the detonator
      strig[ST4_FACTORY_BLOWN] = 1;
      g_things3[2].active = FALSE;
      strig[ST3_SEESAW_SEESAWED] = 1;  // Already used the plank and scissors and that
      strig[ST3_PLANK_PLACED] = 1;
      g_things3[0].active = FALSE;
      g_things4[1].active = FALSE;
      g_things3[4].active = FALSE;     // Already used the catapult
      strig[ST3_BALLOON_POPPED] = 1;
      strig[ST3_GIVEN_BALLOON] = 1;
      strig[ST4_ATEAM_CALLED] = 1;     // A-Teams phone number already used
      p_things4[43].active = FALSE;
      strig[ST1_COLIN_CHEERED_UP] = 1; // Used the CD
      g_things5[7].active = FALSE;
      strig[ST2_RECYCLOTRON_REPAIRED] = 1; // Used the cog
      strig[ST2_CAMERA_COLLECTED] = 1;
      g_things1[7].active = FALSE;     // Used the carbon
      strig[ST4_CARBON_PLACED] = 1;
      strig[ST4_DIAMOND_COLLECTED] = 1;
      inv[INV_TELEPORTERWATCH] = 1;    // Got the teleporter watch
      g_things5[0].active = FALSE;
      g_things3[9].active = FALSE;     // Used the airhorn
      g_things5[6].active = FALSE;     // Used the turps
      g_things3[10].active = FALSE;    // Used the candlestick
      strig[ST3_DRAGON_HONKED] = 1;
      strig[ST3_DRAGON_DRUNK] = 1;
      strig[ST3_CANDLE_LIT] = 1;
      strig[ST3_CANDLE_COLLECTED] = 1; // Got the lit candle
      strig[ST3_TORCHES_LIT] = 1;      // .. and used it
      strig[ST3_DIAMOND_PLACED] = 1;   // Used the diamond
      g_things3[19].active = FALSE;   // Used the indigestion pills
      strig[ST5_INDIGESTION_CURED] = 1;
      strig[ST3_GUM_BOUGHT] = 1;
      strig[ST3_GUM_COLLECTED] = 1;    // Got the gum
      strig[ST5_BATTERY_CHARGED] = 1;  // Charged the battery
      strig[ST5_BATTERY_COLLECTED] = 1; // And grabbed it
      strig[ST5_PHOTO_TAKEN] = 1;      // Got the photo
      g_things3[17].active = FALSE;    // Got and used the hosepipe
      strig[ST1_HOSE_PLACED] = 1;
      strig[ST1_BOSS_BEATEN] = 1;
      strig[ST1_BOOT_COLLECTED] = 1;
      strig[ST2_JCS_BOOTED] = 1;
      strig[ST2_PLUG_GRABBED] = 1;  // Grabbed it!
      p_things2[105].active = FALSE;
      gid.hardhat = TRUE; // used the hardhat
      g_things2[10].active = FALSE;
      strig[ST2_PIPE_SEALED] = 1; // gum used
      strig[ST2_PHOTO_PRINTED] = 1;
      strig[ST2_PHOTO_COLLECTED] = 1;
      inv[INV_SPADE] = 1;   // got the spade
      g_things4[5].active = FALSE;
      g_things4[6].active = FALSE;
      strig[ST5_CCTV_DISABLED] = 1;
      strig[ST5_BATTERY_PLACED] = 1;
      strig[ST5_RED_PRESSED] = 1;
      strig[ST5_GREEN_PRESSED] = 1;
      strig[ST5_BLUE_PRESSED] = 1;
      inv[INV_MIRROR] = 1;
      g_things5[10].active = FALSE;
      break;

    case 0x30005: // Level 5, fourth time around
      // Cheaty starty on level 5
      strig[ST4_PHONEBOX_FIXED] = 1;    // Already used the electrical toolkit
      g_things2[3].active = FALSE;        // disable toolkit object
      strig[ST2_SLUDGE_RELEASED] = 1;    // Already used the lard
      g_things1[0].active = FALSE;        // Disable lard object
      strig[ST1_SLUG_MOVE]    = 1;       // Already used the barrel
      strig[ST1_SLUG_BATHING] = 1;
      p_things1[0].active = FALSE;        // Disable barrel object
      strig[ST4_DETONATOR_PLACED] = 1; // Already used the detonator
      strig[ST4_FACTORY_BLOWN] = 1;
      g_things3[2].active = FALSE;
      strig[ST3_SEESAW_SEESAWED] = 1;  // Already used the plank and scissors and that
      strig[ST3_PLANK_PLACED] = 1;
      g_things3[0].active = FALSE;
      g_things4[1].active = FALSE;
      g_things3[4].active = FALSE;     // Already used the catapult
      strig[ST3_BALLOON_POPPED] = 1;
      strig[ST3_GIVEN_BALLOON] = 1;
      strig[ST4_ATEAM_CALLED] = 1;     // A-Teams phone number already used
      p_things4[43].active = FALSE;
      strig[ST1_COLIN_CHEERED_UP] = 1; // Used the CD
      g_things5[7].active = FALSE;
      strig[ST2_RECYCLOTRON_REPAIRED] = 1; // Used the cog
      strig[ST2_CAMERA_COLLECTED] = 1;
      g_things1[7].active = FALSE;     // Used the carbon
      strig[ST4_CARBON_PLACED] = 1;
      strig[ST4_DIAMOND_COLLECTED] = 1;
      inv[INV_TELEPORTERWATCH] = 1;    // Got the teleporter watch
      g_things5[0].active = FALSE;
      g_things3[9].active = FALSE;     // Used the airhorn
      g_things5[6].active = FALSE;     // Used the turps
      g_things3[10].active = FALSE;    // Used the candlestick
      strig[ST3_DRAGON_HONKED] = 1;
      strig[ST3_DRAGON_DRUNK] = 1;
      strig[ST3_CANDLE_LIT] = 1;
      strig[ST3_CANDLE_COLLECTED] = 1; // Got the lit candle
      strig[ST3_TORCHES_LIT] = 1;      // .. and used it
      strig[ST3_DIAMOND_PLACED] = 1;   // Used the diamond
      g_things3[19].active = FALSE;   // Used the indigestion pills
      strig[ST5_INDIGESTION_CURED] = 1;
      strig[ST3_GUM_BOUGHT] = 1;
      strig[ST3_GUM_COLLECTED] = 1;    // Got the gum
      strig[ST5_BATTERY_CHARGED] = 1;  // Charged the battery
      strig[ST5_BATTERY_COLLECTED] = 1; // And grabbed it
      strig[ST5_PHOTO_TAKEN] = 1;      // Got the photo
      g_things3[17].active = FALSE;    // Got and used the hosepipe
      strig[ST1_HOSE_PLACED] = 1;
      strig[ST1_BOSS_BEATEN] = 1;
      strig[ST1_BOOT_COLLECTED] = 1;
      strig[ST2_JCS_BOOTED] = 1;
      strig[ST2_PLUG_GRABBED] = 1;  // Grabbed it!
      p_things2[105].active = FALSE;
      gid.hardhat = TRUE; // used the hardhat
      g_things2[10].active = FALSE;
      strig[ST2_PIPE_SEALED] = 1; // gum used
      strig[ST2_PHOTO_PRINTED] = 1;
      strig[ST2_PHOTO_COLLECTED] = 1;
      g_things4[5].active = FALSE;
      g_things4[6].active = FALSE;
      strig[ST5_CCTV_DISABLED] = 1;
      strig[ST5_BATTERY_PLACED] = 1;
      strig[ST5_RED_PRESSED] = 1;
      strig[ST5_GREEN_PRESSED] = 1;
      strig[ST5_BLUE_PRESSED] = 1;
      inv[INV_MIRROR] = 1;
      g_things5[10].active = FALSE;
      strig[ST1_BOMB_DUG] = 1;
      strig[ST1_BOMB_COLLECTED] = 1;
      inv[INV_BOMB] = 1;
      break;
  }

  return load_level( levn, TRUE );
}

BOOL start_game( void )
{
  int i;

  paused = FALSE;

  gid.coins = 0;
  gid.lives = 3;
  gid.energy = 3;
  gid.dieanim = FALSE;
  gid.spoonedit = FALSE;
  gid.rot = 0.0f;
  gid.jumping = 0;
  gid.flashing = 0;
  gid.onlift = -1;
  gid.onspring = -1;
  gid.fallpuffs = FALSE;
  gid.lockx = -1;
  gid.hardhat = FALSE;
	gid.savedtheday = FALSE;

  lastinv = 0;

  reset_thingies();
  reset_infos();

	winfo = -1;
	infobulbwinfo = -1;

  for( i=0; i<ST_LAST; i++ )
    strig[i] = 0;

  for( i=0; i<INV_LAST; i++ )
    inv[i] = 0;

  mtexts     = &texts[256*256*4];

  if( !load_globalsprites() ) return FALSE;

//  if( !cheaty_starty( 0x30001 ) ) return FALSE;
  if( !load_level( 1, TRUE ) ) return FALSE;

  time( &gamestarttime );

  giddy_say( "Come on, time to wibble." );
  return TRUE;
}

void addthingytoxcolis( struct thingy *t, struct btex *sl, u8 *src, int cx, int cy )
{
  int tl, tt, tr, tb, xp, yp, xl, cpx, cpy, cpxl;
  int ccx, ccy;
  struct btex *s;

  if( !t->active ) return;

  if( (t->flags&THF_BLOCKWALK) == 0 )
    return;

  thingy_bounds( t, sl, &tl, &tt, &tr, &tb );

  ccx = cx<<4;
  ccy = cy<<4;

  if( ( tl >= (ccx+32) ) ||
      ( tt >= (ccy+48) ) ||
      ( tr < ccx ) ||
      ( tb < ccy ) )
    return;

  s = &sl[t->frames[t->frame]];

  yp = 0;
  if( tt < ccy ) yp = ccy-tt;
  xl = 0;
  if( tl < ccx ) xl = ccx-tl;

  cpy = 0;
  if( tt > ccy ) cpy = tt-ccy;
  cpxl = 0;
  if( tl > ccx ) cpxl = tl-ccx;

  for( ; yp<s->fh; yp++, cpy++ )
  {
    if( cpy >= 48 ) break;
    for( xp=xl, cpx=cpxl; xp<s->fw; xp++, cpx++ )
    {
      if( cpx >= 32 ) break;
      if( src[(yp*256+xp)*4+3] != 0 )
        collisionarea[cpy*32+cpx] = 0xff;
    }
  }  
}

void makecolisareax( int offset )
{
  int mapo, x, y, b, bpx, bpy, bx, by, i;

  mapo = ((gid.y-0xa00)>>12)*mapi->fgw + ((gid.x-offset)>>12);

  for( y=0; y<3; y++ )
  {
    for( x=0; x<2; x++ )
    {
      i = fgmap[y*mapi->fgw+x+mapo];
      b = blktrans[i];
      if( ( blktrans[i+256]&1 ) != 0 ) b = 0;
      
      bpy = (b/16) * 16 * 256 * 4;
      b = (b&15)*16*4+3;
      for( by=0; by<16; by++, bpy+=256*4 )
      {
        for( bx=0,bpx=b; bx<16; bx++, bpx+=4 )
          collisionarea[((y*16)+by)*32+(x*16+bx)] = (blocks[bpy+bpx]&0xff)?0xff:0x00;
      }
    }
  }

  x = (gid.x-offset)>>12;
  y = (gid.y-0xa00)>>12;

  // Add in any solid objects in the area
  for( i=0; i<num_nthings; i++ )
    addthingytoxcolis( n_things[i], sprs, sprites, x, y );
  for( i=0; i<num_bnthings; i++ )
    addthingytoxcolis( bn_things[i], sprs, sprites, x, y );
  for( i=0; i<num_pthings; i++ )
    addthingytoxcolis( p_things[i], psprs, psprites, x, y );
  for( i=0; i<num_bpthings; i++ )
    addthingytoxcolis( bp_things[i], psprs, psprites, x, y );
  for( i=0; i<num_gthings; i++ )
    addthingytoxcolis( g_things[i], sprtg, gsprites, x, y );
  for( i=0; i<num_bgthings; i++ )
    addthingytoxcolis( bg_things[i], sprtg, gsprites, x, y );
}

void addthingytoycolis( struct thingy *t, struct btex *sl, u8 *src, int cx, int cy )
{
  int tl, tt, tr, tb, xp, yp, xl, cpx, cpy, cpxl;
  int ccx, ccy;
  u8 bm;
  struct btex *s;

  if( !t->active ) return;

  if( gid.mainchryoffset > 0 )
  {
    if( (t->flags&THF_BLOCKJUMP) == 0 )
      return;
  } else {
    if( (t->flags&THF_BLOCKFALL) == 0 )
      return;
  }

  thingy_bounds( t, sl, &tl, &tt, &tr, &tb );

  if( ( (tb-tt) > 8 ) && ( (t->flags&THF_BLOCKWALK) == 0 ) )
    tb = tt+8;

  ccx = cx<<4;
  ccy = cy<<4;

  if( ( tl >= (ccx+48) ) ||
      ( tt >= (ccy+32) ) ||
      ( tr < ccx ) ||
      ( tb < ccy ) )
    return;

  s = &sl[t->frames[t->frame]];
  src += (s->fy*256+s->fx)*4;

  yp = 0;
  if( tt < ccy ) yp = ccy-tt;
  xl = 0;
  if( tl < ccx ) xl = ccx-tl;

  cpy = 0;
  if( tt > ccy ) cpy = tt-ccy;
  cpxl = 0;
  if( tl > ccx ) cpxl = tl-ccx;

  bm = 0xff;
  if( t->flags&THF_CONVEYLEFT  ) bm = 0xfd;
  if( t->flags&THF_CONVEYRIGHT ) bm = 0xfe;

  switch( clevel )
  {
    case 3:
      if( t == p_things[6] ) bm = 0xf8;
      if( t == p_things[20] ) bm = 0xf7;
      break;

    case 4:
      if( t == p_things[39] ) bm = 0xfb;
      if( t == p_things[8] ) bm = 0xf4;
      break;
    
    case 5:
      if( t == p_things[7] ) bm = 0xf5;
      if( t == p_things[5] ) bm = 0xf6;
      if( t == p_things[3] ) bm = 0xfa;
      if( t == n_things[0] ) bm = 0xf9;
      break;
  }

  for( ; yp<s->fh; yp++, cpy++ )
  {
    if( cpy >= 32 ) break;
    for( xp=xl, cpx=cpxl; xp<s->fw; xp++, cpx++ )
    {
      if( cpx >= 48 ) break;
      if( src[(yp*256+xp)*4+3] != 0 )
        collisionarea[cpy*48+cpx] = bm;
    }
  }  
}

void addlifttoycolis( int ln, int cx, int cy, int lx, int ly, int stop )
{
  int tl, tt, tr, tb, xp, yp, xl, cpx, cpy, cpxl;
  u8 *src;
  struct btex *s;

  if( gid.mainchryoffset > 0 )
    return;

  if( !lifts[ln].s ) return;

  s = lifts[ln].s;

  tl = lx - s->hfw;
  tt = (ly+lifts[ln].bobo+lifts[ln].dip+lifts[ln].surface) - s->hfh;
  tr = tl + s->fw;
  tb = tt + lifts[ln].depth;

//  if( ( (tb-tt) > 8 ) && ( (t->flags&THF_BLOCKWALK) == 0 ) )
//    tb = tt+8;

  if( ( tl >= ((cx+3)<<4) ) ||
      ( tt >= ((cy+2)<<4) ) ||
      ( tr < (cx<<4) ) ||
      ( tb < (cy<<4) ) )
    return;

  src = &lifts[ln].src[((s->fy+lifts[ln].surface)*256+s->fx)*4];

  yp = 0;
  if( tt < (cy<<4) ) yp = (cy<<4)-tt;
  xl = 0;
  if( tl < (cx<<4) ) xl = (cx<<4)-tl;

  cpy = 0;
  if( tt > (cy<<4) ) cpy = tt-(cy<<4);
  cpxl = 0;
  if( tl > (cx<<4) ) cpxl = tl-(cx<<4);

  for( ; yp<lifts[ln].depth; yp++, cpy++ )
  {
    if( cpy >= 32 ) break;
    for( xp=xl, cpx=cpxl; xp<s->fw; xp++, cpx++ )
    {
      if( cpx >= 48 ) break;
      if( src[(yp*256+xp)*4+3] != 0 )
      {
        collisionarea[cpy*48+cpx] = ln+1;
        liftstoparea[cpy*48+cpx]  = stop;
      }
    }
  }  
}

void addspringtoycolis( int sn, int cx, int cy )
{
  int tl, tt, tr, tb, xp, yp, xl, cpx, cpy, cpxl;
  int so[] = { 0, 7, 14, 19 };
  int offs;
  u8 *src;
  struct btex *s;

  if( gid.mainchryoffset > 0 )
    return;

  s = &springs[sn].sl[springs[sn].frames[springs[sn].frame]];
  src = &springs[sn].src[(s->fy*256+s->fx)*4];

  tl = springs[sn].x;
  tt = springs[sn].y;
  tr = tl + s->fw;
  tb = tt + 8;

  if( ( tl >= ((cx+3)<<4) ) ||
      ( tt >= ((cy+2)<<4) ) ||
      ( tr < (cx<<4) ) ||
      ( tb < (cy<<4) ) )
    return;

  yp = 0;
  if( tt < (cy<<4) ) yp = (cy<<4)-tt;
  xl = 0;
  if( tl < (cx<<4) ) xl = (cx<<4)-tl;

  cpy = 0;
  if( tt > (cy<<4) ) cpy = tt-(cy<<4);
  cpxl = 0;
  if( tl > (cx<<4) ) cpxl = tl-(cx<<4);

  offs = so[springs[sn].frame];
  if( yp < so[springs[sn].frame] )
  {
    offs = so[springs[sn].frame]-yp;
    yp = so[springs[sn].frame];
  }
  cpy += offs;

  for( ; yp<(so[springs[sn].frame]+14); yp++, cpy++ )
  {
    if( cpy >= 32 ) break;
    for( xp=xl, cpx=cpxl; xp<s->fw; xp++, cpx++ )
    {
      if( cpx >= 48 ) break;
      if( src[(yp*256+xp)*4+3] != 0 )
        collisionarea[cpy*48+cpx] = sn+0x80;
    }
  }  
}

void addtardistoycolis( int cx, int cy )
{
  int tl, tt, tr, tb, xp, yp, xl, cpx, cpy, cpxl;

  if( ( clevel != 3 ) && ( clevel != 4 ) )
    return;

  if( !considertardis )
    return;

  if( gid.mainchryoffset > 0 )
    return;

  tl = trd_x>>8;
  tt = (trd_y>>8)-8;
  tr = tl + 62;
  tb = tt + 8;

  if( ( tl >= ((cx+3)<<4) ) ||
      ( tt >= ((cy+2)<<4) ) ||
      ( tr < (cx<<4) ) ||
      ( tb < (cy<<4) ) )
    return;

  yp = 0;
  if( tt < (cy<<4) ) yp = (cy<<4)-tt;
  xl = 0;
  if( tl < (cx<<4) ) xl = (cx<<4)-tl;

  cpy = 0;
  if( tt > (cy<<4) ) cpy = tt-(cy<<4);
  cpxl = 0;
  if( tl > (cx<<4) ) cpxl = tl-(cx<<4);

  for( ; yp<8; yp++, cpy++ )
  {
    if( cpy >= 32 ) break;
    for( xp=xl, cpx=cpxl; xp<62; xp++, cpx++ )
    {
      if( cpx >= 48 ) break;
      collisionarea[cpy*48+cpx] = 0xfc;
    }
  }  
}

void makecolisareay( int offset, BOOL inclifts, BOOL inctardis )
{
  int mapo, i, j, x, y, b, bpx, bpy, bx, by;

  if( inclifts )
    memset( liftstoparea, 0, 16*16*3*2 );

  mapo = ((gid.y-offset)>>12)*mapi->fgw + ((gid.x-0x1000)>>12);

  for( y=0; y<2; y++ )
  {
    for( x=0; x<3; x++ )
    {
      i = fgmap[y*mapi->fgw+x+mapo];
      b = blktrans[i];
      if( ( gid.mainchryoffset > 0 ) && ( ( blktrans[i+256]&1 ) != 0 ) )
        b = 0;

      bpy = (b/16) * 16 * 256 * 4;
      b = (b&15)*16*4+3;
      for( by=0; by<16; by++, bpy+=256*4 )
      {
        for( bx=0,bpx=b; bx<16; bx++, bpx+=4 )
          collisionarea[((y*16)+by)*48+(x*16+bx)] = (blocks[bpy+bpx]&0xff)?0xff:0x00;
      }
    }
  }

  x = (gid.x-0x1000)>>12;
  y = (gid.y-offset)>>12;

  // Add in any solid objects in the area
  for( i=0; i<num_nthings; i++ )
    addthingytoycolis( n_things[i], sprs, sprites, x, y );
  for( i=0; i<num_bnthings; i++ )
    addthingytoycolis( bn_things[i], sprs, sprites, x, y );
  for( i=0; i<num_pthings; i++ )
    addthingytoycolis( p_things[i], psprs, psprites, x, y );
  for( i=0; i<num_bpthings; i++ )
    addthingytoycolis( bp_things[i], psprs, psprites, x, y );
  for( i=0; i<num_gthings; i++ )
    addthingytoycolis( g_things[i], sprtg, gsprites, x, y );
  for( i=0; i<num_bgthings; i++ )
    addthingytoycolis( bg_things[i], sprtg, gsprites, x, y );
  for( i=0; springs[i].x!=-1; i++ )
    addspringtoycolis( i, x, y );
  if( inclifts )
  {
    for( i=0; lifts[i].numstops!=-1; i++ )
    {
      if( lifts[i].fromstop == -1 )
      {
        for( j=0; j<lifts[i].numstops; j++ )
          addlifttoycolis( i, x, y, lifts[i].stops[j*2], lifts[i].stops[j*2+1], j );
      } else {
        addlifttoycolis( i, x, y, lifts[i].cx>>8, lifts[i].cy>>8, 0 );
      }
    }
  }
  if( inctardis )
    addtardistoycolis( x, y );
}

int testupcolis( void )
{
  int coff, i, j, k, ov, bov;

  makecolisareay( 0x1000, FALSE, FALSE );

  coff = ((gid.py&0xf)+16)*48 + (gid.px&0xf) + 6;
  bov = 0; // biggest area of overlap
  for( i=0; i<21; i++ )  // scan 21 lines
  {
    ov = 0; // this lines overlap
    j = coff - edgecdlist[i]*48; // Origin offset - 10y
    if( collisionarea[j] != 0 )
    {
      for( k=0; k<10; k++ )
      {
        ov++; // at least one pixel overlap
        j+=48; // go up one pixel
        if( collisionarea[j] == 0 ) break;
      }
      
      if( ov > bov ) bov = ov; // Compare this lines col with max
    }
    coff++;
  }

  return bov;
}

int dotestdowncolis( int *lifthit, int *liftstophit, int *convey, int *springhit, int *trig )
{
  int coff, i, j, k, ov, bov, wtard;

  gid.y += 0x300;
  gid.py += 3;
  makecolisareay( 0, lifthit != NULL, TRUE );

  wtard = gid.ontardis == ON_TARDIS_STEP_INSIDE ? ON_TARDIS_STEP_INSIDE : ON_TARDIS_STEP_OUTSIDE;
  if( gid.ontardis != INSIDE_TARDIS )
    gid.ontardis = NOT_ON_TARDIS;

  *convey = 0;

  if( lifthit ) *lifthit = -1;
  if( springhit ) *springhit = -1;
  coff = (gid.py&0xf)*48 + (gid.px&0xf) + 6;
  bov = 0; // biggest area of overlap
  for( i=0; i<21; i++ )  // scan 21 lines
  {
    ov = 0; // this lines overlap
    j = coff + edgecdlist[i]*48; // Origin offset - 10x
    if( collisionarea[j] != 0 )
    {
      switch( collisionarea[j] )
      {
        case 0xf4:
        case 0xf5:
        case 0xf6:
        case 0xf7:
        case 0xf8:
        case 0xf9:
        case 0xfa:
        case 0xfb:
          *trig = collisionarea[j];
          break;
        case 0xfc: if( gid.ontardis == NOT_ON_TARDIS ) gid.ontardis = wtard; break;
        case 0xfd: *convey = 1; break;
        case 0xfe: *convey = 2; break;
        case 0xff: break;
        default:
          if( ( collisionarea[j] >= 0x80 ) && ( collisionarea[j] < 0x90 ) )
          {
            if( springhit ) *springhit = collisionarea[j]-0x80;
            break;
          }

          if( lifthit )     *lifthit     = collisionarea[j]-1;
          if( liftstophit ) *liftstophit = liftstoparea[j];
          break;
      }
      for( k=0; k<10; k++ )
      {
        ov++; // at least one pixel overlap
        j-=48; // go up one pixel
        if( collisionarea[j] == 0 ) break;
        switch( collisionarea[j] )
        {
          case 0xf4:
          case 0xf5:
          case 0xf6:
          case 0xf7:
          case 0xf8:
          case 0xf9:
          case 0xfa:
          case 0xfb:
            *trig = collisionarea[j];
            break;
          case 0xfc: if( gid.ontardis == NOT_ON_TARDIS ) gid.ontardis = wtard; break;
          case 0xfd: *convey = 1; break;
          case 0xfe: *convey = 2; break;
          case 0xff: break;
          default:
            if( ( collisionarea[j] >= 0x80 ) && ( collisionarea[j] < 0x90 ) )
            {
              if( springhit ) *springhit = collisionarea[j]-0x80;
              break;
            }
            if( lifthit )     *lifthit = collisionarea[j]-1;
            if( liftstophit ) *liftstophit = liftstoparea[j];
            break;
        }
      }
      
      if( ov > bov ) bov = ov; // Compare this lines col with max
    }
    coff++;
  }

  gid.y -= 0x300;
  gid.py -= 3;
  return bov;
}

int testdowncolis( int *convey, int *trig )
{
  int bov, glf, gls;

  *trig = 0;

  bov = dotestdowncolis( &glf, &gls, convey, &gid.onspring, trig );

  if( ( glf != -1 ) && ( glf != gid.onlift ) )
  {
    if( lifts[glf].fromstop == -1 )
    {
      lifts[glf].fromstop = gls;
      lifts[glf].tostop   = (gls+1)%lifts[glf].numstops;
      setliftdeltas( glf );
    }
    gid.lox = gid.x - lifts[glf].cx;
    if( lifts[glf].type == LT_BUBBLE )
    {
      gid.loy = gid.py - (lifts[glf].cy>>8)+lifts[glf].bobo+lifts[glf].dip;
//      if( gid.loy > lifts[glf].surface-30 )
//        gid.loy = lifts[glf].surface-30;
    } else {
      gid.loy = -((18+lifts[glf].s->hfh)-lifts[glf].surface);
//      gid.loy = lifts[glf].surface-30;
    }
    gid.onlift = glf;
  }

  return bov;
}

int testrightcolis( void )
{
  int coff, i, j, k, ov, bov;

  makecolisareax( 0 );

  coff = ((gid.py-10)&15)*32 + (gid.px&15);

  bov = 0; // biggest area of overlap
  for( i=0; i<21; i++ )  // scan 21 lines
  {
    ov = 0; // this lines overlap
    j = coff + edgecdlist[i]; // Origin offset - 10x
    if( collisionarea[j] != 0 )
    {
      for( k=0; k<edgecdlist[i]; k++ )
      {
        ov++; // at least one pixel overlap
        j--; // go left one pixel
        if( collisionarea[j] == 0 ) break;
      }
      
      if( ov > bov ) bov = ov; // Compare this lines col with max
    }
    coff+=32;
  }
  return bov;
}

BOOL testleftcolis( void )
{
  int coff, i, j, k, ov, bov;

  makecolisareax( 0x1000 );

  coff = ((gid.py-10)&15)*32 + (gid.px&15) + 16;

  bov = 0; // biggest area of overlap
  for( i=0; i<21; i++ )  // scan 21 lines
  {
    ov = 0; // this lines overlap
    j = coff - edgecdlist[i]; // Origin offset - 10x
    if( collisionarea[j] != 0 )
    {
      for( k=0; k<edgecdlist[i]; k++ )
      {
        ov++; // at least one pixel overlap
        j++; // go left one pixel
        if( collisionarea[j] == 0 ) break;
      }
      
      if( ov > bov ) bov = ov; // Compare this lines col with max
    }
    coff+=32;
  }
  return bov;
}

void jumprotate( void )
{
  if( gid.rot == 0 ) return;

  gid.def = 1;
  gid.rot -= 12;
  if( gid.rot < 0 ) gid.rot = 0;
}

void gidanim( BOOL flipped )
{
  s16 gidanimframes[] = { 1, 2, 3, 4, 5, 4, 3, 2 };

  gid.flipped = flipped;
  if( gid.rot == 0 )
  {
    gid.framedelay++;
    if( gid.framedelay >= 5 )
    {
      gid.framedelay = 0;
      gid.framestep++;
      if( gid.framestep >= 8 )
        gid.framestep = 0;
      if( gid.framestep == 0 ) lpactionsound( SND_GIDDYSTEP1, sfxvol );
      if( gid.framestep == 4 ) lpactionsound( SND_GIDDYSTEP2, sfxvol );
    }

    gid.def = gidanimframes[gid.framestep];
  }
}

void scroll_x( void )
{
  int off, liml, limr;

  if( stuff.bossmode )
  {
    liml = stuff.bossmodeleft;
    limr = stuff.bossmoderight-320;
  } else {
    liml = 0;
    limr = mapi->fgw*16 - 320;
    if( clevel == 4 )
      limr = 7200;
  }

  off = (fgx+160)-gid.px;
  if( off < -6 ) off = -6;
  if( off > 6 ) off = 6;

  fgx -= off;
  if( fgx < liml ) fgx = liml;
  if( fgx > limr ) fgx = limr;
}

void scroll_y( void )
{
  int off, limt, limb;
  int i, targy;

  limt = mapi->yscrolltoplimit;
  limb = mapi->fgh*16 - 240;

  targy = (gid.py-SCROFF)-stuff.quake;

//  if( gid.onlift == -1 )
  {
    for( i=0; stsc[i].miny!=-1; i++ )
    {
      if( ( gid.py >= stsc[i].miny ) &&
          ( gid.py <= stsc[i].maxy ) )
        targy = stsc[i].scry-stuff.quake;
    }
  }

  if( stuff.quake > 0 )
    stuff.quake--;

  off = fgy-targy;
  if( off < -6 ) off = -6;
  if( off > 6 ) off = 6;

  fgy -= off;
  if( fgy < limt ) fgy = limt;
  if( fgy > limb ) fgy = limb;
}

/*
  mov si,offset sintab
  mov bx,[rumbleindex]
  or bx,bx
  jz ridxok
  mov ax,2000h
  imul [word si+bx]
  mov ax,dx
  cwde
  sub [fmapposy],eax
  mov [rumbled],eax
  add bx,20
  cmp bx,180*2
  jb ridxok
  xor bx,bx
ridxok: mov [rumbleindex],bx
*/


void scrollhandler( void )
{
  scroll_x();
  if( gid.x < 0x1600 ) { gid.x = 0x1600; gid.px = 0x16; }
  if( gid.x > (mapi->fgw<<12) ) { gid.x = mapi->fgw<<12; gid.px = gid.x>>8; }

  scroll_y();

  bgx = fgx>>mapi->xdiv;
  bgy = (fgy+(mapi->yoff<<4))>>mapi->ydiv;
}

/*
;---------------------------------------------------------------------------

  xor ebx,ebx
  mov bx,[bmapxlimit]
  shl ebx,12
  mov eax,[fmapposx]  ;calc background parallax pos
  mov cl,[Bmapxdiv]
  shr eax,cl
xmlmod: cmp eax,ebx
  jb bmlok
  sub eax,ebx
  jmp xmlmod
bmlok:  mov [bmapposx],eax

  mov ax,[bmapyoffset]
  cwde
  shl eax,12
  add eax,[fmapposy]
  mov cl,[Bmapydiv]
  shr eax,cl
  xor ebx,ebx
  mov bx,[bmapylimit]
  shl ebx,12
ymlmod: cmp eax,ebx
  jb bmylok
  sub eax,ebx
  jmp ymlmod
bmylok: mov [bmapposy],eax

scrh_end:
  ret

ENDP
*/

void giddyhit( void )
{
  if( ( gid.flashing > 0 ) ||
      ( gid.dieanim ) ||
      ( gid.usemode ) )
    return;

  if( gid.energy == 0 )
  {
    gid.lives--;
    gid.dieanim = TRUE;
    gid.def = 6;
    gid.dy = gid.y;
    gid.mainchryoffset = (3<<8);
    actionsound( SND_LOSELIFE, sfxvol );
    actionsound( SND_GIDDYHITECHO, sfxvol );
  } else {
    gid.energy--;
    actionsound( SND_GIDDYHIT, sfxvol );
  }

  gid.flashing = 120;
  gid.redang = 0.0f;
}

int slowsink( void )
{
  return blktrans[fgmap[((gid.y+0x1000)>>12)*mapi->fgw+(gid.x>>12)]+256]&0x0c;
}

void make_giddy_do_things( void )
{
  s16 oldx, oldy;
  int i;

  oldy = gid.y;
  oldx = gid.x;
  //fmapposy+=rumbled;
  //rumbled=0;

  if( gid.fallpuffs )
  {
    if( ( (frame&3) == 0 ) && ( gid.mainchryoffset < 0 ) )
      startincidental( gid.px, gid.py, 0, -1, GSPRITEX, bangframes, 5, 2 );
  }

  if( gid.spoonedit )
  {
    gid.spooneditwobble += 0.1f;
    if( gid.spooneditwobble > 6.2831853f ) gid.spooneditwobble -= 6.2831853f;
    switch( gid.spooneditstate )
    {
      case 0: // Fade it
        if( gid.spooneditwobblefactor > 0.0f )
        {
          gid.spooneditwobblefactor -= 0.08f;
          if( gid.spooneditwobblefactor < 0.0f )
            gid.spooneditwobblefactor = 0.0f;
        }

        if( gid.spooneditfade < 255 )
        {
          gid.spooneditfade += 4;
          if( gid.spooneditfade > 255 ) gid.spooneditfade = 255;
          break;
        }

        gid.spooneditstate++;
        break;

      case 1: // Show it
        if( gid.spooneditwobblefactor > 0.0f )
        {
          gid.spooneditwobblefactor -= 0.08f;
          if( gid.spooneditwobblefactor < 0.0f )
            gid.spooneditwobblefactor = 0.0f;
        }

        if( gid.spooneditwait > 0 )
        {
          gid.spooneditwait--;
          break;
        }

        gid.spooneditstate++;
        break;
      
      case 2: // Fade out
        fadetype = 0;
        if( fadea < 255 )
        {
          fadea += 4;
          if( fadea > 255 ) fadea = 255;
          break;
        }

        if( lrfade2 != 255 )
          break;

        gid.spooneditstate++;  // 3 = done!
        fadeadd = -6;
        break;
    }
    return;
  }

  if( gid.dieanim )
  {
    gid.dy -= gid.mainchryoffset;

    if( ((gid.dy>>8)-fgy) > 256 )
    {
      if( gid.lives < 0 )
      {
        gid.spoonedit = TRUE;
        gid.spooneditstate = 0;
        gid.spooneditfade = 0;
        gid.spooneditwobblefactor = 12.0f;
        gid.spooneditwait = 120;
        gid.spooneditwobble = 0.0f;
        return;
      }

      gid.energy = 3;
      gid.dieanim = FALSE;
      gid.mainchryoffset = -0x20;
      gid.def = 1;
    } else {
      gid.mainchryoffset -= 0x20;
      if( gid.mainchryoffset < -(6<<8) )
        gid.mainchryoffset = -(6<<8);
      gid.def = ((frame>>3)&1)+6;
      return;
    }
  }

  if( gid.flashing > 0 )
  {
    gid.flashing--;
    gid.redang += 0.15f;
    if( gid.redang > 3.14159265f ) gid.redang -= 3.14159265f;
  }

  if( gid.watchwarp )
  {
    gid.rot += 4;
    gid.wwob += 0.09f;
    gid.sgwsc += 0.03f;
    if( gid.sgwsc > 2.0f )
    {
      gid.wwa -= 3;
      if( gid.wwa < 0 ) gid.wwa = 0;
    }
    if( ( llevel != gid.wwl ) &&
        ( gid.sgwsc > 4.0f ) )
    {
      llevel = gid.wwl;
      fadea = 0;
      fadeadd = 8;
      fadetype = 0;
      gid.stopuntilfade = TRUE;
    }
    return;
  }

  if( gid.stargatewarp )
  {
    if( gid.px < 6654 ) { gid.px++; gid.x+=0x100; }
    if( gid.px > 6654 ) { gid.px--; gid.x-=0x100; }
    if( gid.py <  136 ) { gid.py++; gid.y+=0x100; }
    if( gid.py >  136 ) { gid.py--; gid.y-=0x100; }
    gid.rot+=12;
    if( gid.sgwsc > 0.0f )
      gid.sgwsc -= 0.02f;

    if( ( llevel != 5 ) &&
        ( gid.sgwsc < 0.4f ) )
    {
      llevel = 5;
      fadea = 0;
      fadeadd = 8;
      fadetype = 0;
      gid.stopuntilfade = TRUE;
    }
    return;
  }

  if( gid.dazed )
  {
    // Giddy is dazed
    return;
  }

  if( ( pjump ) && ( !gid.jumping ) && ( !gid.usemode ) )
	{
		if( ignorejump )
    {
			ignorejump = FALSE;
	  } else {
      gid.jump = TRUE;
      pjump = FALSE;
    }
	}

  if( !gid.usemode ) // Can't move if in use mode
  {
    if( ( !gid.stopuntilfade ) ||
        ( fadeadd == 0 ) )
    {
      if( ( !gid.nogmxgrav ) &&
          ( !gid.jumplatch ) &&
          ( gid.jump ) &&
          ( !gid.jumping ) &&
          ( gid.allowjump ) &&
          ( gid.ontardis != INSIDE_TARDIS ) )
      {
        gid.jumpcount++;
        gid.jumping = 1;
        gid.rot = 358;
        considertardis = TRUE;
        gid.ontardis = NOT_ON_TARDIS;
        actionsound( SND_JUMP, sfxvol );
      }

      if( gid.jumping )
      {
        gid.mainchryoffset = ((sintab[gid.jumping+90]*0xb)>>8)&0xff00;
          
        jumprotate();
        gid.jumping += gid.jumpstep;
        if( gid.jumping >= 180 )
        {
          gid.jumpstep = 6;
          gid.framestep = 0;
          gid.jumping = 0;
          gid.jump = FALSE;
          gid.mainchryoffset = 0xfc00;
        }
      }

      if( ( !gid.nogmxgrav ) &&
          ( !gid.teleport ) )
      {
        // Inertial X motion control
        if( pright )
        {
          gid.movedisp+=2;
          if( gid.movedisp > 8 ) gid.movedisp = 8;
        }

        if( pleft )
        {
          gid.movedisp-=2;
          if( gid.movedisp < -8 ) gid.movedisp = -8;
        }
      }
    }
  } else {

    if( ( pleft ) && ( ibadd == 0 ) && ( ibitem != -1 ) )
    {
      do { ibitem=(ibitem+1)%INV_LAST; } while( inv[ibitem] == 0 );
      set_ibstr( 2, ivtexts[ibitem].use );
      iboff = ibw[1] + 81;
      ibdest = -(strlen(ivtexts[ibitem].use)*4) - iboff;
      ibadd = -4;
      actionsound( SND_USE, sfxvol );
    }

    if( ( pright ) && ( ibadd == 0 ) && ( ibitem != -1 ) )
    {
      do { ibitem=(ibitem+INV_LAST-1)%INV_LAST; } while( inv[ibitem] == 0 );
      set_ibstr( 2, ibt[1] );
      set_ibstr( 1, ivtexts[ibitem].use );
      iboff = ibw[1]+81;
      ibpos -= iboff;
      ibdest = -strlen(ivtexts[ibitem].use)*4;
      ibadd = 4;
      actionsound( SND_USE, sfxvol );
    }
  }

  if( gid.ontardis == INSIDE_TARDIS )
  {
    gid.x = trd_x+(26<<8);
    gid.movedisp = 0;
    gid.mainchryoffset = 0;
    return;
  }

  if( gid.lockx != -1 )
  {
    gid.x = gid.lockx;
    gid.px = gid.x>>8;
    gid.y = gid.locky;
    gid.py = gid.y>>8;
    return;
  }

  // Dampen the motion
  if( gid.movedisp > 0 ) gid.movedisp--;
  if( gid.movedisp < 0 ) gid.movedisp++;

  if( gid.movedisp > 0 )
  {
    if( gid.def != 6 )
      gid.flipped = FALSE;
    if( ( testrightcolis() == 0 ) && ( gid.movedisp > 1 ) )
    {
      gid.x += ((gid.movedisp<<7)&0xffffff00);
      gid.px = gid.x>>8;
      if( stuff.bossmode )
      {
        if( gid.x > (stuff.bossmoderight<<8) )
        {
          gid.x = (stuff.bossmoderight<<8);
        } else {
          i = testrightcolis();
          if( i != 0 )
            gid.x -= (i<<8);
          else
            gidanim( FALSE );
        }
      } else {
        if( gid.onlift != -1 ) gid.lox += ((gid.movedisp<<7)&0xffffff00);
        i = testrightcolis();
        if( i != 0 )
          gid.x -= (i<<8);
        else
          gidanim( FALSE );
      }
      gid.px = gid.x>>8;
    }

  } else if( gid.movedisp < 0 ) {

    if( gid.def != 6 )
      gid.flipped = TRUE;
    if( ( testleftcolis() == 0 ) && ( gid.movedisp < -1 ) )
    {
      gid.x += ((gid.movedisp<<7)&0xffffff00);
      gid.px = gid.x>>8;
      if( stuff.bossmode )
      {
        if( gid.x < (stuff.bossmodeleft<<8) )
        {
          gid.x = (stuff.bossmodeleft<<8);
        } else {
          i = testleftcolis();
          if( i != 0 )
          {
            gid.x += (i<<8);
            if( ( gid.x&0xffffff00 ) < ( oldx&0xffffff00 ) )
              gidanim( TRUE );
          } else {
            gidanim( TRUE );
          }
        }
      } else {
        if( gid.onlift != -1 ) gid.lox += ((gid.movedisp<<7)&0xffffff00);
        i = testleftcolis();
        if( i != 0 )
        {
          gid.x += (i<<8);
          if( ( gid.x&0xffffff00 ) < ( oldx&0xffffff00 ) )
            gidanim( TRUE );
        } else {
          gidanim( TRUE );
        }
      }
      gid.px = gid.x>>8;
    }
  }

  gid.y -= (int)gid.mainchryoffset;  // down?
  gid.py = gid.y>>8;

  if( gid.mainchryoffset <= 0 )
  {
    int convey, triggr;

    i = testdowncolis( &convey, &triggr );
    if( i == 0 )
    {
      i = slowsink();
      if( i != 0 )
      {
        gid.y += gid.mainchryoffset;
        gid.y += 0x100;
        gid.py = gid.y>>8;
        gid.mainchryoffset = 0xff80;
        if(i&0x04) giddyhit();
      }
    } else {
      gid.mainchryoffset = 0xfe80;
      gid.y -= (i<<8);
      gid.py = gid.y>>8;
      gid.fallpuffs = FALSE;
      switch( gid.ontardis )
      {
        case ON_TARDIS_STEP_OUTSIDE:
          if( gid.x >= (trd_x+(44<<8)) )
            gid.ontardis = ON_TARDIS_STEP_INSIDE;
          break;
        
        case ON_TARDIS_STEP_INSIDE:
          if( gid.x < (trd_x+(26<<8)) )
          {
            gid.ontardis = INSIDE_TARDIS;
            incidentalsound( SND_TARDIS, sfxvol );
          }
          break;
        
        case NOT_ON_TARDIS:
          considertardis = FALSE;
          break;
      }
    }

    switch( triggr )
    {
      case 0xfb:
        triggerfactory();
        break;
      
      case 0xf4:
        specialbinstand();
        break;

      case 0xf5:
        if( p_things5[7].frames[0] == 22 )
        {
          p_things5[7].frames[0] = 23;
          gid.y += (4<<8);
          gid.py += 4;
        }
        if( !strig[ST5_BLUE_PRESSED] )
        {
          actionsound( SND_CLICKYCLICK, sfxvol );
          strig[ST5_BLUE_PRESSED] = 1;
          p_things5[8].numframes = 2;
          p_things5[23].active = TRUE;
          initbigassfan();
        }
        break;

      case 0xf6:
        if( p_things5[5].frames[0] == 22 )
        {
          p_things5[5].frames[0] = 23;
          gid.y += (4<<8);
          gid.py += 4;
        }
        if( !strig[ST5_RED_PRESSED] )
        {
          actionsound( SND_CLICKYCLICK, sfxvol );
          strig[ST5_RED_PRESSED] = 1;
          p_things5[6].numframes = 2;
          p_things5[21].active = TRUE;
          initbigassfan();
        }
        break;

      case 0xfa:
        if( p_things5[3].frames[0] == 22 )
        {
          p_things5[3].frames[0] = 23;
          gid.y += (4<<8);
          gid.py += 4;
        }
        if( !strig[ST5_GREEN_PRESSED] )
        {
          actionsound( SND_CLICKYCLICK, sfxvol );
          strig[ST5_GREEN_PRESSED] = 1;
          p_things5[4].numframes = 2;
          p_things5[22].active = TRUE;
          initbigassfan();
        }
        break;
      
      case 0xf9:
        gid.sgwsc = 1.0f;
        gid.wwa = 255;
        gid.watchwarp = TRUE;
        gid.wwl = 1;
        incidentalsound( SND_TELEPORT_OUT, sfxvol );
        break;
      
      case 0xf8:
        if( p_things3[6].y == 901 )
        {
          p_things3[6].y = 905;
          gid.y += (4<<8);
          gid.py += 4;
          triggertimerdoor();
        }
        break;
      
      case 0xf7:
        if( p_things3[20].y == 1061 )
        {
          p_things3[20].y = 1065;
          gid.y += (4<<8);
          gid.py += 4;
          triggertimerdoor();
        }
        break;

      default:
        if( p_things5[3].frames[0] == 23 )
          p_things5[3].frames[0] = 22;
        if( p_things5[5].frames[0] == 23 )
          p_things5[5].frames[0] = 22;
        if( p_things5[7].frames[0] == 23 )
          p_things5[7].frames[0] = 22;
        if( p_things3[6].y == 905 )
          p_things3[6].y = 901;
        if( p_things3[20].y == 1065 )
          p_things3[20].y = 1061;
        if( onthebin ) specialbinleave();
        break;
    }

    switch( convey )
    {
      case 1:
        gid.x -= 0x100;
        gid.px--;
        break;
      
      case 2:
        gid.x += 0x100;
        gid.px++;
        break;
    }

    if( gid.onspring != -1)
    {
      if( springs[gid.onspring].frame < 3 )
      {
        springs[gid.onspring].frame++;
        springs[gid.onspring].rtime = 5;
      } else {
        if( gid.jumping == 0 )
        {
          incidentalsound( SND_SPRING, sfxvol );
          gid.jumpcount++;
          gid.jumping = 1;
          gid.rot = 358;
          gid.y -= 0x1400;
          gid.py -= 0x14;
        }
      }
    }

    gid.allowjump = (i!=0);
    if( gid.onlift != -1 ) gid.allowjump = TRUE;
    gid.mainchryoffset -= 0x80;
    if( gid.mainchryoffset < -(6*256) )
      gid.mainchryoffset = -(6*256);
  }

  if( gid.mainchryoffset > 0 )
  {
    i = testupcolis();
    if( i != 0 )
    {
      gid.y += gid.mainchryoffset;
      gid.py = gid.y>>8;
    }

    gid.onspring = -1;

    // cheatfly
  }

/*
       and [word giddef],0cfffh
       mov al,[gidflicker]
       or al,al
       jz ngflk
       dec [gidflicker]
       cmp [gidflicker],97
       jb gflik
       or [giddef],2000h
       jmp ngflk
gflik: and al,2
       jz ngflk
       or [word giddef],01000h

ngflk: mov ax,[gidback]
       or [giddef],ax
       mov [gidback],0
*/
}

void render_tvborders( void )
{
	GXTexObj texObj;

  GX_LoadPosMtxImm(GXmodelView2D,GX_PNMTX0);

	GX_InitTexObj( &texObj, &tvtex[0], 32, 32, GX_TF_RGBA8, GX_CLAMP, GX_CLAMP, GX_FALSE );
	GX_InitTexObjLOD( &texObj, GX_NEAR, GX_NEAR, 0.0f, 0.0f, 0.0f, 0, 0, GX_ANISO_1 );
	GX_LoadTexObj( &texObj, GX_TEXMAP0 );

  GX_SetTevOp( GX_TEVSTAGE0, GX_MODULATE );
  GX_SetVtxDesc( GX_VA_TEX0, GX_DIRECT );

  GX_Begin( GX_QUADS, GX_VTXFMT0, 4*4 );
    GX_Position3f32(  -0.5f,  -0.5f, 0.0f ); GX_Color4u8( 0xff, 0xff, 0xff, 0xff ); GX_TexCoord2f32( 0.0f, 0.0f );
    GX_Position3f32(  15.5f,  -0.5f, 0.0f ); GX_Color4u8( 0xff, 0xff, 0xff, 0xff ); GX_TexCoord2f32( 0.5f, 0.0f );
    GX_Position3f32(  15.5f,  15.5f, 0.0f ); GX_Color4u8( 0xff, 0xff, 0xff, 0xff ); GX_TexCoord2f32( 0.5f, 0.5f );
    GX_Position3f32(  -0.5f,  15.5f, 0.0f ); GX_Color4u8( 0xff, 0xff, 0xff, 0xff ); GX_TexCoord2f32( 0.0f, 0.5f );

    GX_Position3f32( 304.5f,  -0.5f, 0.0f ); GX_Color4u8( 0xff, 0xff, 0xff, 0xff ); GX_TexCoord2f32( 0.5f, 0.0f );
    GX_Position3f32( 320.5f,  -0.5f, 0.0f ); GX_Color4u8( 0xff, 0xff, 0xff, 0xff ); GX_TexCoord2f32( 1.0f, 0.0f );
    GX_Position3f32( 320.5f,  15.5f, 0.0f ); GX_Color4u8( 0xff, 0xff, 0xff, 0xff ); GX_TexCoord2f32( 1.0f, 0.5f );
    GX_Position3f32( 304.5f,  15.5f, 0.0f ); GX_Color4u8( 0xff, 0xff, 0xff, 0xff ); GX_TexCoord2f32( 0.5f, 0.5f );
                                                                                                                
    GX_Position3f32(  -0.5f, 224.5f, 0.0f ); GX_Color4u8( 0xff, 0xff, 0xff, 0xff ); GX_TexCoord2f32( 0.0f, 0.5f );
    GX_Position3f32(  15.5f, 224.5f, 0.0f ); GX_Color4u8( 0xff, 0xff, 0xff, 0xff ); GX_TexCoord2f32( 0.5f, 0.5f );
    GX_Position3f32(  15.5f, 240.5f, 0.0f ); GX_Color4u8( 0xff, 0xff, 0xff, 0xff ); GX_TexCoord2f32( 0.5f, 1.0f );
    GX_Position3f32(  -0.5f, 240.5f, 0.0f ); GX_Color4u8( 0xff, 0xff, 0xff, 0xff ); GX_TexCoord2f32( 0.0f, 1.0f );
                                                                                                                
    GX_Position3f32( 304.5f, 224.5f, 0.0f ); GX_Color4u8( 0xff, 0xff, 0xff, 0xff ); GX_TexCoord2f32( 0.5f, 0.5f );
    GX_Position3f32( 320.5f, 224.5f, 0.0f ); GX_Color4u8( 0xff, 0xff, 0xff, 0xff ); GX_TexCoord2f32( 1.0f, 0.5f );
    GX_Position3f32( 320.5f, 240.5f, 0.0f ); GX_Color4u8( 0xff, 0xff, 0xff, 0xff ); GX_TexCoord2f32( 1.0f, 1.0f );
    GX_Position3f32( 304.5f, 240.5f, 0.0f ); GX_Color4u8( 0xff, 0xff, 0xff, 0xff ); GX_TexCoord2f32( 0.5f, 1.0f );
  GX_End();

  unbindtexture();
}

void render_background( void )
{
  int o, xo, yo, bx, by, x, y, splity, splo, nump;
  struct btex *tx;
	GXTexObj texObj;

  bindtexture( &texObj, mblocks );

	GX_Begin( GX_QUADS, GX_VTXFMT0, (256/16)*(320/16)*4 );
  for( y=0, yo=0; y<=240; y+=16, yo++ )
  {
    for( x=0, xo=0; x<320; x+=16, xo++ )
    {
      tx = &bt[dmap[yo*20+xo]];
      GX_Position3f32( (f32)(x   ), (f32)(y   ), 0.0f ); GX_Color4u8( 0xff, 0xff, 0xff, 0xff ); GX_TexCoord2f32( tx->x,       tx->y );
      GX_Position3f32( (f32)(x+16), (f32)(y   ), 0.0f ); GX_Color4u8( 0xff, 0xff, 0xff, 0xff ); GX_TexCoord2f32( tx->x+tx->w, tx->y );
      GX_Position3f32( (f32)(x+16), (f32)(y+16), 0.0f ); GX_Color4u8( 0xff, 0xff, 0xff, 0xff ); GX_TexCoord2f32( tx->x+tx->w, tx->y+tx->h );
      GX_Position3f32( (f32)(x   ), (f32)(y+16), 0.0f ); GX_Color4u8( 0xff, 0xff, 0xff, 0xff ); GX_TexCoord2f32( tx->x,       tx->y+tx->h );
    }
  }
  GX_End();

  splity = mapi->splitpos-fgy;
  if( ( clevel == 1 ) && ( gid.y < (370<<8) ) )
    splity = 100000;

  xo = -(bgx&15);
  yo = -(bgy&15);

  splo = 0;
  nump = 0;

  by = (bgy>>4)%mapi->ywrap;
  for( y=yo; y<240; y+=16 )
  {
    bx = (bgx>>4)%mapi->xwrap;

    if( ( splo == 0 ) && ( y >= splity ) )
      splo = mapi->sply;

    for( x=xo; x<336; x+=16 )
    {
      if( bgmap[(by+splo)*mapi->bgw+bx] > 0 )
        nump+=4;
      bx = (bx+1)%mapi->xwrap;
    }

    by=(by+1)%mapi->ywrap;
    if( ( clevel == 1 ) && ( splo == 0 ) && ( by > 15 ) && ( gid.y < (390<<8) ) )
      by = 15;
    if( ( clevel == 4 ) && ( by > 14 ) )
      by = 14;
  }

  if( nump == 0 )
  {
    unbindtexture();
    return;
  }

  splo = 0;
  by = (bgy>>4)%mapi->ywrap;

  GX_Begin( GX_QUADS, GX_VTXFMT0, nump );
  for( y=yo; y<240; y+=16 )
  {
    bx = (bgx>>4)%mapi->xwrap;

    if( ( splo == 0 ) && ( y >= splity ) )
      splo = mapi->sply;

    for( x=xo; x<336; x+=16 )
    {
      o = bgmap[(by+splo)*mapi->bgw+bx];
      if( o > 0 )
      {
        tx = &bt[o];
        GX_Position3f32( (f32)(x   ), (f32)(y   ), 0.0f ); GX_Color4u8( 0xff, 0xff, 0xff, 0xff ); GX_TexCoord2f32( tx->x,       tx->y );
        GX_Position3f32( (f32)(x+16), (f32)(y   ), 0.0f ); GX_Color4u8( 0xff, 0xff, 0xff, 0xff ); GX_TexCoord2f32( tx->x+tx->w, tx->y );
        GX_Position3f32( (f32)(x+16), (f32)(y+16), 0.0f ); GX_Color4u8( 0xff, 0xff, 0xff, 0xff ); GX_TexCoord2f32( tx->x+tx->w, tx->y+tx->h );
        GX_Position3f32( (f32)(x   ), (f32)(y+16), 0.0f ); GX_Color4u8( 0xff, 0xff, 0xff, 0xff ); GX_TexCoord2f32( tx->x,       tx->y+tx->h );
      }
      bx = (bx+1)%mapi->xwrap;
    }

    by=(by+1)%mapi->ywrap;
    if( ( clevel == 1 ) && ( splo == 0 ) && ( by > 15 ) && ( gid.y < (390<<8) ) )
      by = 15;
    if( ( clevel == 4 ) && ( by > 14 ) )
      by = 14;
  }

  GX_End();

  unbindtexture();
}

void render_foreground_zone( int fx, int fy, int fw, int fh )
{
  int x, y, xc, yc, b, nump;
  struct btex *tx;
	GXTexObj texObj;

  x = (fx*16)-fgx;
  y = (fy*16)-fgy;
  if( ( x >= 320 ) || ( (x+(fw*16)) < 0 ) ) return;
  if( ( y >= 240 ) || ( (y+(fh*16)) < 0 ) ) return;

  nump = 0;
  for( yc=0; yc<fh; yc++ )
  {
    x = (fx*16)-fgx;
    for( xc=0; xc<fw; xc++ )
    {
      b = fgmap[(yc+fy)*mapi->fgw+xc+fx];
      if( b ) nump += 4;
      x+=16;
    }
    y+=16;
  }

  if( nump == 0 ) return;

  bindtexture( &texObj, mblocks);

	GX_LoadPosMtxImm(GXmodelView2D,GX_PNMTX0);

  y = (fy*16)-fgy;

  GX_Begin( GX_QUADS, GX_VTXFMT0, nump );
  for( yc=0; yc<fh; yc++ )
  {
    x = (fx*16)-fgx;
    for( xc=0; xc<fw; xc++ )
    {
      b = fgmap[(yc+fy)*mapi->fgw+xc+fx];
      if( b )
      {
        tx = &bt[b];
        GX_Position3f32( (f32)(x   ), (f32)(y   ), 0.0f ); GX_Color4u8( 0xff, 0xff, 0xff, 0xff ); GX_TexCoord2f32( tx->x,       tx->y );
        GX_Position3f32( (f32)(x+16), (f32)(y   ), 0.0f ); GX_Color4u8( 0xff, 0xff, 0xff, 0xff ); GX_TexCoord2f32( tx->x+tx->w, tx->y );
        GX_Position3f32( (f32)(x+16), (f32)(y+16), 0.0f ); GX_Color4u8( 0xff, 0xff, 0xff, 0xff ); GX_TexCoord2f32( tx->x+tx->w, tx->y+tx->h );
        GX_Position3f32( (f32)(x   ), (f32)(y+16), 0.0f ); GX_Color4u8( 0xff, 0xff, 0xff, 0xff ); GX_TexCoord2f32( tx->x,       tx->y+tx->h );
      }
      x+=16;
    }
    y+=16;
  }
  GX_End();
}

void render_foreground( void )
{
  int xo, yo, bx, by, x, y, nump;
  struct btex *tx;
	GXTexObj texObj;

  xo = -(fgx&15);
  yo = -(fgy&15);
  by = (fgy>>4) * mapi->fgw;

  nump = 0;
  for( y=yo; y<240; y+=16 )
  {
    bx = fgx>>4;
    for( x=xo; x<336; x+=16 )
    {
      if( fgmap[by+bx] != 0 ) nump += 4;
      bx++;
    }
    by+=mapi->fgw;
  }

  if( nump == 0 ) return;

  by = (fgy>>4) * mapi->fgw;

  bindtexture( &texObj, mblocks );

  GX_Begin( GX_QUADS, GX_VTXFMT0, nump );
  for( y=yo; y<240; y+=16 )
  {
    bx = fgx>>4;
    for( x=xo; x<336; x+=16 )
    {
      if( fgmap[by+bx] != 0 )
      {
        tx = &bt[fgmap[by+bx]];
        GX_Position3f32( (f32)(x   ), (f32)(y   ), 0.0f ); GX_Color4u8( 0xff, 0xff, 0xff, 0xff ); GX_TexCoord2f32( tx->x,       tx->y );
        GX_Position3f32( (f32)(x+16), (f32)(y   ), 0.0f ); GX_Color4u8( 0xff, 0xff, 0xff, 0xff ); GX_TexCoord2f32( tx->x+tx->w, tx->y );
        GX_Position3f32( (f32)(x+16), (f32)(y+16), 0.0f ); GX_Color4u8( 0xff, 0xff, 0xff, 0xff ); GX_TexCoord2f32( tx->x+tx->w, tx->y+tx->h );
        GX_Position3f32( (f32)(x   ), (f32)(y+16), 0.0f ); GX_Color4u8( 0xff, 0xff, 0xff, 0xff ); GX_TexCoord2f32( tx->x,       tx->y+tx->h );
      }
      bx++;
    }
    by+=mapi->fgw;
  }
  GX_End();

  unbindtexture();
}

void render_sprite_scaled( struct btex *tx, int x, int y, BOOL flipx, float rot, float scale )
{
  float sw, sh;
	Mtx m,m1,m2, mv;

  if( tx->w == 0.0f ) return;  // Dummy sprite?

  sw = tx->hwx;
  sh = tx->hhx;

  if( !flipx ) rot = -rot;

	guMtxIdentity( m1 );
  guMtxScaleApply( m1, m1, scale, scale, 1.0f );
	guMtxRotAxisDeg( m2, &axis, rot );
	guMtxConcat( m2, m1, m );
	guMtxTransApply( m, m, x, y, 0 );
	guMtxConcat( GXmodelView2D, m, mv );
	GX_LoadPosMtxImm( mv, GX_PNMTX0 );

  GX_Begin( GX_QUADS, GX_VTXFMT0, 4 );
  if( flipx )
  {
    GX_Position3f32( -sw, -sh, 0.0f ); GX_Color4u8( 0xff, 0xff, 0xff, 0xff ); GX_TexCoord2f32( tx->x+tx->w, tx->y );
    GX_Position3f32(  sw, -sh, 0.0f ); GX_Color4u8( 0xff, 0xff, 0xff, 0xff ); GX_TexCoord2f32( tx->x      , tx->y );
    GX_Position3f32(  sw,  sh, 0.0f ); GX_Color4u8( 0xff, 0xff, 0xff, 0xff ); GX_TexCoord2f32( tx->x      , tx->y+tx->h );
    GX_Position3f32( -sw,  sh, 0.0f ); GX_Color4u8( 0xff, 0xff, 0xff, 0xff ); GX_TexCoord2f32( tx->x+tx->w, tx->y+tx->h );
  } else {
    GX_Position3f32( -sw, -sh, 0.0f ); GX_Color4u8( 0xff, 0xff, 0xff, 0xff ); GX_TexCoord2f32( tx->x,       tx->y );
    GX_Position3f32(  sw, -sh, 0.0f ); GX_Color4u8( 0xff, 0xff, 0xff, 0xff ); GX_TexCoord2f32( tx->x+tx->w, tx->y );
    GX_Position3f32(  sw,  sh, 0.0f ); GX_Color4u8( 0xff, 0xff, 0xff, 0xff ); GX_TexCoord2f32( tx->x+tx->w, tx->y+tx->h );
    GX_Position3f32( -sw,  sh, 0.0f ); GX_Color4u8( 0xff, 0xff, 0xff, 0xff ); GX_TexCoord2f32( tx->x,       tx->y+tx->h );
  }
  GX_End();
}

void render_sprite_scaleda( struct btex *tx, int x, int y, BOOL flipx, float rot, float scale, int alpha )
{
  float sw, sh;
	Mtx m,m1,m2, mv;

  if( tx->w == 0.0f ) return;  // Dummy sprite?

  sw = tx->hwx;
  sh = tx->hhx;

  if( !flipx ) rot = -rot;

	guMtxIdentity( m1 );
  guMtxScaleApply( m1, m1, scale, scale, 1.0f );
	guMtxRotAxisDeg( m2, &axis, rot );
	guMtxConcat( m2, m1, m );
	guMtxTransApply( m, m, x, y, 0 );
	guMtxConcat( GXmodelView2D, m, mv );
	GX_LoadPosMtxImm( mv, GX_PNMTX0 );

  GX_Begin( GX_QUADS, GX_VTXFMT0, 4 );
  if( flipx )
  {
    GX_Position3f32( -sw, -sh, 0.0f ); GX_Color4u8( 0xff, 0xff, 0xff, alpha ); GX_TexCoord2f32( tx->x+tx->w, tx->y );
    GX_Position3f32(  sw, -sh, 0.0f ); GX_Color4u8( 0xff, 0xff, 0xff, alpha ); GX_TexCoord2f32( tx->x      , tx->y );
    GX_Position3f32(  sw,  sh, 0.0f ); GX_Color4u8( 0xff, 0xff, 0xff, alpha ); GX_TexCoord2f32( tx->x      , tx->y+tx->h );
    GX_Position3f32( -sw,  sh, 0.0f ); GX_Color4u8( 0xff, 0xff, 0xff, alpha ); GX_TexCoord2f32( tx->x+tx->w, tx->y+tx->h );
  } else {
    GX_Position3f32( -sw, -sh, 0.0f ); GX_Color4u8( 0xff, 0xff, 0xff, alpha ); GX_TexCoord2f32( tx->x,       tx->y );
    GX_Position3f32(  sw, -sh, 0.0f ); GX_Color4u8( 0xff, 0xff, 0xff, alpha ); GX_TexCoord2f32( tx->x+tx->w, tx->y );
    GX_Position3f32(  sw,  sh, 0.0f ); GX_Color4u8( 0xff, 0xff, 0xff, alpha ); GX_TexCoord2f32( tx->x+tx->w, tx->y+tx->h );
    GX_Position3f32( -sw,  sh, 0.0f ); GX_Color4u8( 0xff, 0xff, 0xff, alpha ); GX_TexCoord2f32( tx->x,       tx->y+tx->h );
  }
  GX_End();
}

void render_giddy( void )
{
  float sw, sh, x, y, ca, cb, ga;
  struct btex *tx;
  u8 *whichtex;
  float rot;
	Mtx m,m1,m2, mv;
	Vector axis = (Vector){ 0, 0, 1 };
	GXTexObj texObj;

  if( gid.dieanim )
  {
    y = (gid.dy>>8)-fgy;
    rot = 0.0f;
    if( ( clevel == 4 ) && ( gid.hardhat ) )
    {
      whichtex = msprites;
      tx = &sprs[gid.def+36];
    } else {
      whichtex = mgsprites;
      tx = &sprtg[gid.def];
    }
  } else {
    if( gid.ontardis == INSIDE_TARDIS )
      return;

    y = gid.py-fgy;
    rot = gid.rot;
    tx = &sprs[gid.def];
    whichtex = msprites;
  }

  x = gid.px-fgx;

  if( tx->w == 0.0f ) return;  // Dummy sprite?

  bindtexture( &texObj, whichtex );

  sw = tx->hwx;
  sh = tx->hhx;

  if( !gid.flipped ) rot = -rot;

	guMtxIdentity( m1 );
  if( gid.stargatewarp )
    guMtxScaleApply( m1, m1, gid.sgwsc, gid.sgwsc, 1.0f );
  if( gid.watchwarp )
  {
    guMtxScaleApply( m1, m1,
                     gid.sgwsc + sin( gid.wwob ) * gid.sgwsc * 0.4f,
                     gid.sgwsc + sin( gid.wwob*0.8f ) * gid.sgwsc * 0.4f,
                     1.0f );
  }
	guMtxRotAxisDeg( m2, &axis, rot );
	guMtxConcat( m2, m1, m );
	guMtxTransApply( m, m, x, y, 0 );
	guMtxConcat( GXmodelView2D, m, mv );
	GX_LoadPosMtxImm( mv, GX_PNMTX0 );

  ga = 255;
  if( gid.watchwarp ) ga = gid.wwa;
  if( ( gid.flashing ) && ( !gid.dieanim ) )
  {
    ca = ((int)(sin(gid.redang)*63.0f))+192;
    cb = ((int)(sin(gid.redang)*160.0f)); if( cb < 0 ) cb = 0;
  } else if( gid.watchwarp ) {
    ca = ((int)(sin(gid.wwob*1.2f)*95.0f))+160;
    cb = ((int)(sin(gid.wwob*0.9f)*95.0f))+160;
  } else {
    ca = 255;
    cb = 255;
  }

  if( ( clevel == 4 ) && ( gid.hardhat ) && ( !gid.dieanim ) )
  {
    float hsb, hy;
 
    hsb = 24.0f - sh;
    hy = tx->y+(24.0f/256.0f);

  	GX_Begin( GX_QUADS, GX_VTXFMT0, 8 );
    if( gid.flipped )
    {
        GX_Position3f32( -sw, -sh, 0.0f ); GX_Color4u8( ca, cb, cb, ga ); GX_TexCoord2f32( tx->x+tx->w, 232.0f/256.0f );
        GX_Position3f32(  sw, -sh, 0.0f ); GX_Color4u8( ca, cb, cb, ga ); GX_TexCoord2f32( tx->x      , 232.0f/256.0f );
        GX_Position3f32(  sw, hsb, 0.0f ); GX_Color4u8( ca, cb, cb, ga ); GX_TexCoord2f32( tx->x      ,          1.0f );
        GX_Position3f32( -sw, hsb, 0.0f ); GX_Color4u8( ca, cb, cb, ga ); GX_TexCoord2f32( tx->x+tx->w,          1.0f );

        GX_Position3f32( -sw, hsb, 0.0f ); GX_Color4u8( ca, cb, cb, ga ); GX_TexCoord2f32( tx->x+tx->w, hy          );
        GX_Position3f32(  sw, hsb, 0.0f ); GX_Color4u8( ca, cb, cb, ga ); GX_TexCoord2f32( tx->x      , hy          );
        GX_Position3f32(  sw,  sh, 0.0f ); GX_Color4u8( ca, cb, cb, ga ); GX_TexCoord2f32( tx->x      , tx->y+tx->h );
        GX_Position3f32( -sw,  sh, 0.0f ); GX_Color4u8( ca, cb, cb, ga ); GX_TexCoord2f32( tx->x+tx->w, tx->y+tx->h );
    } else {
        GX_Position3f32( -sw, -sh, 0.0f ); GX_Color4u8( ca, cb, cb, ga ); GX_TexCoord2f32( tx->x,       232.0f/256.0f );
        GX_Position3f32(  sw, -sh, 0.0f ); GX_Color4u8( ca, cb, cb, ga ); GX_TexCoord2f32( tx->x+tx->w, 232.0f/256.0f );
        GX_Position3f32(  sw, hsb, 0.0f ); GX_Color4u8( ca, cb, cb, ga ); GX_TexCoord2f32( tx->x+tx->w,          1.0f );
        GX_Position3f32( -sw, hsb, 0.0f ); GX_Color4u8( ca, cb, cb, ga ); GX_TexCoord2f32( tx->x,                1.0f );

        GX_Position3f32( -sw, hsb, 0.0f ); GX_Color4u8( ca, cb, cb, ga ); GX_TexCoord2f32( tx->x,       hy          );
        GX_Position3f32(  sw, hsb, 0.0f ); GX_Color4u8( ca, cb, cb, ga ); GX_TexCoord2f32( tx->x+tx->w, hy          );
        GX_Position3f32(  sw,  sh, 0.0f ); GX_Color4u8( ca, cb, cb, ga ); GX_TexCoord2f32( tx->x+tx->w, tx->y+tx->h );
        GX_Position3f32( -sw,  sh, 0.0f ); GX_Color4u8( ca, cb, cb, ga ); GX_TexCoord2f32( tx->x,       tx->y+tx->h );
    }
    GX_End();
    unbindtexture();
    return;
  }

  GX_Begin( GX_QUADS, GX_VTXFMT0, 4 );
  if( gid.flipped )
  {
    GX_Position3f32( -sw, -sh, 0.0f ); GX_Color4u8( ca, cb, cb, ga ); GX_TexCoord2f32( tx->x+tx->w, tx->y       );
    GX_Position3f32(  sw, -sh, 0.0f ); GX_Color4u8( ca, cb, cb, ga ); GX_TexCoord2f32( tx->x      , tx->y       );
    GX_Position3f32(  sw,  sh, 0.0f ); GX_Color4u8( ca, cb, cb, ga ); GX_TexCoord2f32( tx->x      , tx->y+tx->h );
    GX_Position3f32( -sw,  sh, 0.0f ); GX_Color4u8( ca, cb, cb, ga ); GX_TexCoord2f32( tx->x+tx->w, tx->y+tx->h );
  } else {
    GX_Position3f32( -sw, -sh, 0.0f ); GX_Color4u8( ca, cb, cb, ga ); GX_TexCoord2f32( tx->x      , tx->y       );
    GX_Position3f32(  sw, -sh, 0.0f ); GX_Color4u8( ca, cb, cb, ga ); GX_TexCoord2f32( tx->x+tx->w, tx->y       );
    GX_Position3f32(  sw,  sh, 0.0f ); GX_Color4u8( ca, cb, cb, ga ); GX_TexCoord2f32( tx->x+tx->w, tx->y+tx->h );
    GX_Position3f32( -sw,  sh, 0.0f ); GX_Color4u8( ca, cb, cb, ga ); GX_TexCoord2f32( tx->x      , tx->y+tx->h );
  }
  GX_End();
  unbindtexture();
}

void render_sprite( struct btex *tx, int x, int y, BOOL flipx, float rot )
{
  float sw, sh;
	Mtx m, mv;

  if( tx->w == 0.0f ) return;  // Dummy sprite?

  sw = tx->hwx;
  sh = tx->hhx;

  if( !flipx ) rot = -rot;

	guMtxRotAxisDeg( m, &axis, rot );
	guMtxTransApply( m, m, x, y, 0 );
	guMtxConcat( GXmodelView2D, m, mv );
	GX_LoadPosMtxImm( mv, GX_PNMTX0 );

  GX_Begin( GX_QUADS, GX_VTXFMT0, 4 );
  if( flipx )
  {
    GX_Position3f32( -sw, -sh, 0.0f ); GX_Color4u8( 0xff, 0xff, 0xff, 0xff ); GX_TexCoord2f32( tx->x+tx->w, tx->y );
    GX_Position3f32(  sw, -sh, 0.0f ); GX_Color4u8( 0xff, 0xff, 0xff, 0xff ); GX_TexCoord2f32( tx->x      , tx->y );
    GX_Position3f32(  sw,  sh, 0.0f ); GX_Color4u8( 0xff, 0xff, 0xff, 0xff ); GX_TexCoord2f32( tx->x      , tx->y+tx->h );
    GX_Position3f32( -sw,  sh, 0.0f ); GX_Color4u8( 0xff, 0xff, 0xff, 0xff ); GX_TexCoord2f32( tx->x+tx->w, tx->y+tx->h );
  } else {
    GX_Position3f32( -sw, -sh, 0.0f ); GX_Color4u8( 0xff, 0xff, 0xff, 0xff ); GX_TexCoord2f32( tx->x,       tx->y );
    GX_Position3f32(  sw, -sh, 0.0f ); GX_Color4u8( 0xff, 0xff, 0xff, 0xff ); GX_TexCoord2f32( tx->x+tx->w, tx->y );
    GX_Position3f32(  sw,  sh, 0.0f ); GX_Color4u8( 0xff, 0xff, 0xff, 0xff ); GX_TexCoord2f32( tx->x+tx->w, tx->y+tx->h );
    GX_Position3f32( -sw,  sh, 0.0f ); GX_Color4u8( 0xff, 0xff, 0xff, 0xff ); GX_TexCoord2f32( tx->x,       tx->y+tx->h );
  }
  GX_End();
}

void render_sprite_a( struct btex *tx, int x, int y, BOOL flipx, float rot, int alpha )
{
  float sw, sh;
	Mtx m, mv;

  if( tx->w == 0.0f ) return;  // Dummy sprite?

  sw = tx->hwx;
  sh = tx->hhx;

  if( !flipx ) rot = -rot;

	guMtxRotAxisDeg( m, &axis, rot );
	guMtxTransApply( m, m, x, y, 0 );
	guMtxConcat( GXmodelView2D, m, mv );
	GX_LoadPosMtxImm( mv, GX_PNMTX0 );

  GX_Begin( GX_QUADS, GX_VTXFMT0, 4 );
  if( flipx )
  {
    GX_Position3f32( -sw, -sh, 0.0f ); GX_Color4u8( 0xff, 0xff, 0xff, alpha ); GX_TexCoord2f32( tx->x+tx->w, tx->y );
    GX_Position3f32(  sw, -sh, 0.0f ); GX_Color4u8( 0xff, 0xff, 0xff, alpha ); GX_TexCoord2f32( tx->x      , tx->y );
    GX_Position3f32(  sw,  sh, 0.0f ); GX_Color4u8( 0xff, 0xff, 0xff, alpha ); GX_TexCoord2f32( tx->x      , tx->y+tx->h );
    GX_Position3f32( -sw,  sh, 0.0f ); GX_Color4u8( 0xff, 0xff, 0xff, alpha ); GX_TexCoord2f32( tx->x+tx->w, tx->y+tx->h );
  } else {
    GX_Position3f32( -sw, -sh, 0.0f ); GX_Color4u8( 0xff, 0xff, 0xff, alpha ); GX_TexCoord2f32( tx->x,       tx->y );
    GX_Position3f32(  sw, -sh, 0.0f ); GX_Color4u8( 0xff, 0xff, 0xff, alpha ); GX_TexCoord2f32( tx->x+tx->w, tx->y );
    GX_Position3f32(  sw,  sh, 0.0f ); GX_Color4u8( 0xff, 0xff, 0xff, alpha ); GX_TexCoord2f32( tx->x+tx->w, tx->y+tx->h );
    GX_Position3f32( -sw,  sh, 0.0f ); GX_Color4u8( 0xff, 0xff, 0xff, alpha ); GX_TexCoord2f32( tx->x,       tx->y+tx->h );
  }
  GX_End();
}

void render_sprite_tl( struct btex *tx, int x, int y, BOOL flipx )
{
  float sw, sh;

  if( tx->w == 0.0f ) return;  // Dummy sprite?

  sw = tx->fw;
  sh = tx->fh;

	GX_LoadPosMtxImm(GXmodelView2D,GX_PNMTX0);
  GX_Begin( GX_QUADS, GX_VTXFMT0, 4 );
  if( flipx )
  {
    GX_Position3f32( x   , y   , 0.0f ); GX_Color4u8( 0xff, 0xff, 0xff, 0xff ); GX_TexCoord2f32( tx->x+tx->w, tx->y       );
    GX_Position3f32( x+sw, y   , 0.0f ); GX_Color4u8( 0xff, 0xff, 0xff, 0xff ); GX_TexCoord2f32( tx->x,       tx->y       );
    GX_Position3f32( x+sw, y+sh, 0.0f ); GX_Color4u8( 0xff, 0xff, 0xff, 0xff ); GX_TexCoord2f32( tx->x,       tx->y+tx->h );
    GX_Position3f32( x   , y+sh, 0.0f ); GX_Color4u8( 0xff, 0xff, 0xff, 0xff ); GX_TexCoord2f32( tx->x+tx->w, tx->y+tx->h );
  } else {
    GX_Position3f32( x   , y   , 0.0f ); GX_Color4u8( 0xff, 0xff, 0xff, 0xff ); GX_TexCoord2f32( tx->x,       tx->y       );
    GX_Position3f32( x+sw, y   , 0.0f ); GX_Color4u8( 0xff, 0xff, 0xff, 0xff ); GX_TexCoord2f32( tx->x+tx->w, tx->y       );
    GX_Position3f32( x+sw, y+sh, 0.0f ); GX_Color4u8( 0xff, 0xff, 0xff, 0xff ); GX_TexCoord2f32( tx->x+tx->w, tx->y+tx->h );
    GX_Position3f32( x   , y+sh, 0.0f ); GX_Color4u8( 0xff, 0xff, 0xff, 0xff ); GX_TexCoord2f32( tx->x,       tx->y+tx->h );
  }
  GX_End();
}

void render_sprite_tl_stretch( struct btex *tx, int x, int y, float sw, float sh, BOOL flipx )
{
  if( tx->w == 0.0f ) return;  // Dummy sprite?

	GX_LoadPosMtxImm(GXmodelView2D,GX_PNMTX0);
  GX_Begin( GX_QUADS, GX_VTXFMT0, 4 );
  if( flipx )
  {
    GX_Position3f32( x   , y   , 0.0f ); GX_Color4u8( 0xff, 0xff, 0xff, 0xff ); GX_TexCoord2f32( tx->x+tx->w, tx->y       );
    GX_Position3f32( x+sw, y   , 0.0f ); GX_Color4u8( 0xff, 0xff, 0xff, 0xff ); GX_TexCoord2f32( tx->x,       tx->y       );
    GX_Position3f32( x+sw, y+sh, 0.0f ); GX_Color4u8( 0xff, 0xff, 0xff, 0xff ); GX_TexCoord2f32( tx->x,       tx->y+tx->h );
    GX_Position3f32( x   , y+sh, 0.0f ); GX_Color4u8( 0xff, 0xff, 0xff, 0xff ); GX_TexCoord2f32( tx->x+tx->w, tx->y+tx->h );
  } else {
    GX_Position3f32( x   , y   , 0.0f ); GX_Color4u8( 0xff, 0xff, 0xff, 0xff ); GX_TexCoord2f32( tx->x,       tx->y       );
    GX_Position3f32( x+sw, y   , 0.0f ); GX_Color4u8( 0xff, 0xff, 0xff, 0xff ); GX_TexCoord2f32( tx->x+tx->w, tx->y       );
    GX_Position3f32( x+sw, y+sh, 0.0f ); GX_Color4u8( 0xff, 0xff, 0xff, 0xff ); GX_TexCoord2f32( tx->x+tx->w, tx->y+tx->h );
    GX_Position3f32( x   , y+sh, 0.0f ); GX_Color4u8( 0xff, 0xff, 0xff, 0xff ); GX_TexCoord2f32( tx->x,       tx->y+tx->h );
  }
  GX_End();
}

void render_sprite_tl_clipy( struct btex *tx, int x, int y, int clipy, BOOL flipx )
{
  float sw, sh, th;

  if( tx->w == 0.0f ) return;  // Dummy sprite?
  if( y >= clipy ) return;

  sw = tx->fw;
  sh = tx->fh;
  th = tx->h;

  if( ( y+sh ) >= clipy )
  {
    sh = clipy-y;
    th = sh/256.0f;
  }

	GX_LoadPosMtxImm(GXmodelView2D,GX_PNMTX0);
  GX_Begin( GX_QUADS, GX_VTXFMT0, 4 );
  if( flipx )
  {
    GX_Position3f32( x   , y   , 0.0f ); GX_Color4u8( 0xff, 0xff, 0xff, 0xff ); GX_TexCoord2f32( tx->x+tx->w, tx->y    );
    GX_Position3f32( x+sw, y   , 0.0f ); GX_Color4u8( 0xff, 0xff, 0xff, 0xff ); GX_TexCoord2f32( tx->x,       tx->y    );
    GX_Position3f32( x+sw, y+sh, 0.0f ); GX_Color4u8( 0xff, 0xff, 0xff, 0xff ); GX_TexCoord2f32( tx->x,       tx->y+th );
    GX_Position3f32( x   , y+sh, 0.0f ); GX_Color4u8( 0xff, 0xff, 0xff, 0xff ); GX_TexCoord2f32( tx->x+tx->w, tx->y+th );
  } else {
    GX_Position3f32( x   , y   , 0.0f ); GX_Color4u8( 0xff, 0xff, 0xff, 0xff ); GX_TexCoord2f32( tx->x,       tx->y    );
    GX_Position3f32( x+sw, y   , 0.0f ); GX_Color4u8( 0xff, 0xff, 0xff, 0xff ); GX_TexCoord2f32( tx->x+tx->w, tx->y    );
    GX_Position3f32( x+sw, y+sh, 0.0f ); GX_Color4u8( 0xff, 0xff, 0xff, 0xff ); GX_TexCoord2f32( tx->x+tx->w, tx->y+th );
    GX_Position3f32( x   , y+sh, 0.0f ); GX_Color4u8( 0xff, 0xff, 0xff, 0xff ); GX_TexCoord2f32( tx->x,       tx->y+th );
  }
  GX_End();
}

void render_sprite_tl_clipyb( struct btex *tx, int x, int y, int clipy, BOOL flipx )
{
  float sw, sh, tt, th;

  if( tx->w == 0.0f ) return;  // Dummy sprite?
  if( (y+tx->fh) <= clipy ) return;

  sw = tx->fw;
  sh = tx->fh;
  tt = tx->y;
  th = tx->h;

  if( y < clipy )
  {
    sh -= clipy-y;
    th -= (clipy-y)/256.0f;
    tt += (clipy-y)/256.0f;
    y = clipy;
  }

	GX_LoadPosMtxImm(GXmodelView2D,GX_PNMTX0);
  GX_Begin( GX_QUADS, GX_VTXFMT0, 4 );
  if( flipx )
  {
    GX_Position3f32( x   , y   , 0.0f ); GX_Color4u8( 0xff, 0xff, 0xff, 0xff ); GX_TexCoord2f32( tx->x+tx->w, tt    );
    GX_Position3f32( x+sw, y   , 0.0f ); GX_Color4u8( 0xff, 0xff, 0xff, 0xff ); GX_TexCoord2f32( tx->x,       tt    );
    GX_Position3f32( x+sw, y+sh, 0.0f ); GX_Color4u8( 0xff, 0xff, 0xff, 0xff ); GX_TexCoord2f32( tx->x,       tt+th );
    GX_Position3f32( x   , y+sh, 0.0f ); GX_Color4u8( 0xff, 0xff, 0xff, 0xff ); GX_TexCoord2f32( tx->x+tx->w, tt+th );
  } else {
    GX_Position3f32( x   , y   , 0.0f ); GX_Color4u8( 0xff, 0xff, 0xff, 0xff ); GX_TexCoord2f32( tx->x,       tt    );
    GX_Position3f32( x+sw, y   , 0.0f ); GX_Color4u8( 0xff, 0xff, 0xff, 0xff ); GX_TexCoord2f32( tx->x+tx->w, tt    );
    GX_Position3f32( x+sw, y+sh, 0.0f ); GX_Color4u8( 0xff, 0xff, 0xff, 0xff ); GX_TexCoord2f32( tx->x+tx->w, tt+th );
    GX_Position3f32( x   , y+sh, 0.0f ); GX_Color4u8( 0xff, 0xff, 0xff, 0xff ); GX_TexCoord2f32( tx->x,       tt+th );
  }
  GX_End();
}

void render_sprite_offs( struct btex *tx, int x, int y, int xo, int yo, BOOL flipx, float rot )
{
  float sw, sh;
	Mtx m, mv;

  if( tx->w == 0.0f ) return;  // Dummy sprite?

  sw = tx->hwx;
  sh = tx->hhx;

  if( !flipx ) rot = -rot;

	guMtxRotAxisDeg( m, &axis, rot );
	guMtxTransApply( m, m, x, y, 0 );
	guMtxConcat( GXmodelView2D, m, mv );
	GX_LoadPosMtxImm( mv, GX_PNMTX0 );

  GX_Begin( GX_QUADS, GX_VTXFMT0, 4 );
  if( flipx )
  {
    GX_Position3f32( xo-sw, yo-sh, 0.0f ); GX_Color4u8( 0xff, 0xff, 0xff, 0xff ); GX_TexCoord2f32( tx->x+tx->w, tx->y       );
    GX_Position3f32( xo+sw, yo-sh, 0.0f ); GX_Color4u8( 0xff, 0xff, 0xff, 0xff ); GX_TexCoord2f32( tx->x,       tx->y       );
    GX_Position3f32( xo+sw, yo+sh, 0.0f ); GX_Color4u8( 0xff, 0xff, 0xff, 0xff ); GX_TexCoord2f32( tx->x,       tx->y+tx->h );
    GX_Position3f32( xo-sw, yo+sh, 0.0f ); GX_Color4u8( 0xff, 0xff, 0xff, 0xff ); GX_TexCoord2f32( tx->x+tx->w, tx->y+tx->h );
  } else {
    GX_Position3f32( xo-sw, yo-sh, 0.0f ); GX_Color4u8( 0xff, 0xff, 0xff, 0xff ); GX_TexCoord2f32( tx->x,       tx->y       );
    GX_Position3f32( xo+sw, yo-sh, 0.0f ); GX_Color4u8( 0xff, 0xff, 0xff, 0xff ); GX_TexCoord2f32( tx->x+tx->w, tx->y       );
    GX_Position3f32( xo+sw, yo+sh, 0.0f ); GX_Color4u8( 0xff, 0xff, 0xff, 0xff ); GX_TexCoord2f32( tx->x+tx->w, tx->y+tx->h );
    GX_Position3f32( xo-sw, yo+sh, 0.0f ); GX_Color4u8( 0xff, 0xff, 0xff, 0xff ); GX_TexCoord2f32( tx->x,       tx->y+tx->h );
  }
  GX_End();
}

void render_nthing( struct thingy *t )
{
  int x, y;
  struct btex *s;
  u8 *texp;
	GXTexObj texObj;

  if( !t->active ) return;

  x = t->x;
  y = t->y;

  if( t->flags & THF_HOP ) y -= hoptab[frame%24];

  s = &sprs[t->frames[t->frame]];

  if( ( x < (fgx-s->fw) ) ||
      ( x > (fgx+319+s->fw) ) ||
      ( y < (fgy-s->fh) ) ||
      ( y > (fgy+239+s->fh) ) )
    return;
 
  if( ( t->flags & THF_FLASH ) && ( ( frame % 20 ) < 3 ) )
    texp = mwsprites;
  else
    texp = msprites;

  bindtexture( &texObj, texp );

  if( t->flags & THF_CENTREPOS )
  {
    render_sprite( s, x-fgx, y-fgy, t->flipped, 0 );
    unbindtexture();
    return;
  }

  render_sprite_tl( s, x-fgx, y-fgy, t->flipped );
  unbindtexture();
};

void render_pthing( struct thingy *t )
{
  int x, y;
  struct btex *s;
  u8 *texp;
	GXTexObj texObj;

  if( !t->active ) return;

  x = t->x;
  y = t->y;

  if( t->flags & THF_HOP ) y -= hoptab[frame%24];

  s = &psprs[t->frames[t->frame]];

  if( ( x < (fgx-s->fw) ) ||
      ( x > (fgx+319+s->fw) ) ||
      ( y < (fgy-s->fh) ) ||
      ( y > (fgy+239+s->fh) ) )
    return;
 
  if( ( t->flags & THF_FLASH ) && ( ( frame % 20 ) < 3 ) )
    texp = mwpsprites;
  else
    texp = mpsprites;

  bindtexture( &texObj, texp );

  if( t->flags & THF_CENTREPOS )
  {
    render_sprite( s, x-fgx, y-fgy, t->flipped, 0 );
    unbindtexture();
    return;
  }

  render_sprite_tl( s, x-fgx, y-fgy, t->flipped );
  unbindtexture();
};


void render_gthing( struct thingy *t )
{
  int x, y;
  struct btex *s;
  u8 *texp;
	GXTexObj texObj;

  if( !t->active ) return;

  x = t->x;
  y = t->y;

  if( t->flags & THF_HOP ) y -= hoptab[frame%24];

  s = &sprtg[t->frames[t->frame]];

  if( ( x < (fgx-s->fw) ) ||
      ( x > (fgx+319+s->fw) ) ||
      ( y < (fgy-s->fh) ) ||
      ( y > (fgy+239+s->fh) ) )
    return;
 
  if( ( t->flags & THF_FLASH ) && ( ( frame % 20 ) < 3 ) )
    texp = mwgsprites;
  else
    texp = mgsprites;

  bindtexture( &texObj, texp );

  if( t->flags & THF_CENTREPOS )
  {
    render_sprite( s, x-fgx, y-fgy, t->flipped, 0 );
    unbindtexture();
    return;
  }

  render_sprite_tl( s, x-fgx, y-fgy, t->flipped );
  unbindtexture();
};

void render_infobulb( void )
{
	GXTexObj texObj;

  bindtexture( &texObj, mgsprites );
	if( infobulbalpha > 0 )
    render_sprite_scaleda( &sprtg[110], 245, 23, FALSE, 0, infobulbzoom, infobulbalpha );
  unbindtexture();

	if( winfo == -1 ) 
	{
		infobulbwinfo = -1;
		return;
  }

  if( infobulbwinfo != winfo )
	{
		infobulbalpha = 255;
		infobulbzoom = 1.0f;
		infobulbwinfo = winfo;
	}

  bindtexture( &texObj, mgsprites );

  render_sprite_tl( &sprtg[(frame&0x08)?110:5], 236, 10, FALSE );

  unbindtexture();
}

void ring_o_stars( int x, int y, float ang, float astep, int num, float speed )
{
  int i;

  for( i=0; i<num; i++ )
  {
    stars[nextstar].x          = (x<<8);
    stars[nextstar].y          = (y<<8);
    stars[nextstar].dx         = (int)(cos(ang)*speed);
    stars[nextstar].dy         = (int)(sin(ang)*speed);
    stars[nextstar].framecount = 0;
    stars[nextstar].framespeed = 5;
    stars[nextstar].frame      = 18;
    nextstar = (nextstar+1)%MAX_STARS;
    ang += astep;
  }
}

void movelifts( void )
{
  int i, j, oldx, oldy;
  int fx,fy,tx,ty;
  float ang, scc;
  BOOL moveon;

  for( i=0; lifts[i].numstops!=-1; i++ )
  {
    oldx = lifts[i].cx;
    oldy = lifts[i].cy;
    if( lifts[i].type == LT_FRETURN )
      lifts[i].bobo = ((int)(sin( ((float)frame)/6.0f )*lifts[i].bob));
    else
      lifts[i].bobo = ((int)(sin( ((float)frame)/16.0f )*lifts[i].bob));

    if( gid.onlift == i )
    {
      if( lifts[i].dip < lifts[i].dipmax )
        lifts[i].dip+=3;
    } else {
      if( lifts[i].dip > 0 )
        lifts[i].dip-=3;
    }

    if( lifts[i].fromstop == -1 )
      continue;

    fx = lifts[i].stops[lifts[i].fromstop*2];
    fy = lifts[i].stops[lifts[i].fromstop*2+1];
    tx = lifts[i].stops[lifts[i].tostop*2];
    ty = lifts[i].stops[lifts[i].tostop*2+1];

    if( lifts[i].timeout > 0 )
    {
      lifts[i].timeout--;
      if( gid.onlift != i )
      {
        if( lifts[i].type == LT_BTRIGGR )
        {
          if( lifts[i].fromstop == 0 )
          {
            lifts[i].timeout = lifts[i].itimeout;
            if( ( liftloopchan != -1 ) && ( liftlooplift == i ) )
              stopchannel( liftloopchan );
            continue;
          }
        }

        if( lifts[i].type == LT_TRIGGER )
        {
          lifts[i].fromstop = -1;
          lifts[i].timeout = lifts[i].itimeout;
          if( ( liftloopchan != -1 ) && ( liftlooplift == i ) )
            stopchannel( liftloopchan );
          continue;
        }
      }
    } else {
      moveon = FALSE;

      lifts[i].cx += lifts[i].dx;
      lifts[i].cy += lifts[i].dy;

      if( lifts[i].dy == 0 )
      {
        if( lifts[i].dx > 0 )
        {
          if( (lifts[i].cx>>8) >= tx )
            moveon = TRUE;
        } else {
          if( (lifts[i].cx>>8) <= tx )
            moveon = TRUE;
        }
      } else {
        if( lifts[i].dy > 0 )
        {
          if( (lifts[i].cy>>8) >= ty )
            moveon = TRUE;
        } else {
          if( (lifts[i].cy>>8) <= ty )
            moveon = TRUE;
        }
      }

      if( moveon )
      {
        if( ( (lifts[i].cx>>8) > (fgx-64) ) &&
            ( (lifts[i].cx>>8) < (fgx+384) ) &&
            ( (lifts[i].cy>>8) > (fgy-64) ) &&
            ( (lifts[i].cy>>8) < (fgy+304) ) )
        {
          if( lifts[i].sndstop != -1 )
            incidentalsound( lifts[i].sndstop, sfxvol );
        }

        if( ( liftloopchan != -1 ) && ( liftlooplift == i ) )
          stopchannel( liftloopchan );

        switch( lifts[i].type )
        {
          case LT_BUBBLE:
            {
              BOOL anydraw;
              lifts[i].cx = lifts[i].stops[0]<<8;
              lifts[i].cy = lifts[i].stops[1]<<8;
              lifts[i].timeout = 60;
              anydraw = FALSE;
              ang = 0.0f;
              for( j=0; j<8; j++, ang+=(3.14159265/4.0f) )
                anydraw |= startincidental( lifts[i].stops[2] + ((int)(sin(ang)*18.0f)), lifts[i].stops[3] + ((int)(cos(ang)*18.0f)), 0, 0, SPRITEX, ibubblepop, 5, 1 );
              if( gid.onlift == i ) gid.onlift = -1;
              if( anydraw ) incidentalsound( SND_BUBBLEPOP, sfxvol );
            }
            break;
          
          case LT_TRIGGER:
            if( gid.onlift != i )
            {
              lifts[i].fromstop = -1;
              continue;
            }

          default:
            lifts[i].timeout = lifts[i].itimeout;

            lifts[i].fromstop = lifts[i].tostop;
            lifts[i].tostop = (lifts[i].tostop+1)%lifts[i].numstops;
            setliftdeltas( i );
            break;
        }
      } else {
        if( ( lifts[i].sndloop != -1 ) && ( liftloopchan == -1 ) )
        {
          if( ( (lifts[i].cx>>8) > (fgx-64) ) &&
              ( (lifts[i].cx>>8) < (fgx+384) ) &&
              ( (lifts[i].cy>>8) > (fgy-64) ) &&
              ( (lifts[i].cy>>8) < (fgy+304) ) )
          {
            liftlooplift = i;
            ambientloop( lifts[i].sndloop, sfxvol, &liftloopchan );
          }
        }
      }
    }

    if( lifts[i].type == LT_BUBBLE )
    {
      scc = ((float)(lifts[i].stops[1]-(lifts[i].cy>>8))) / 64.0f;
      if( scc > 1.0f ) scc = 1.0f;
      lifts[i].scale = scc;
    }

    if( gid.onlift == i )
    {
      if( gid.mainchryoffset <= 0 )
      {
        gid.x = lifts[i].cx + gid.lox;
        gid.y = lifts[i].cy + ((gid.loy+lifts[i].bobo+lifts[i].dip)<<8);
        gid.px = gid.x>>8;
        gid.py = gid.y>>8;
      } else {
        gid.onlift = -1;
      }
      j = ((lifts[i].s->fw/2)+9)<<8;
      if( ( gid.lox < -j ) || ( gid.lox > j ) )
        gid.onlift = -1;
    }
  }
}

void animatesprings( void )
{
  int i;
  for( i=0; springs[i].x!=-1; i++ )
  {
    if( springs[i].rtime > 0 )
    {
      springs[i].rtime--;
      continue;
    }
    if( ( i != gid.onspring ) && ( springs[i].frame != 0 ) )
      springs[i].frame = 0;
  }
}

void movestars( void )
{
  int i;
 
  for( i=0; i<MAX_STARS; i++ )
  {
    if( stars[i].frame < 23 )
    {
      stars[i].x += stars[i].dx;
      stars[i].y += stars[i].dy;
      if( stars[i].framecount < stars[i].framespeed )
      {
        stars[i].framecount++;
        continue;
      }
      stars[i].frame++;
      stars[i].framecount = 0;
    }
  }
}

void timeincidentals( void )
{
  int i;
 
  for( i=0; i<MAX_INCIDENTALS; i++ )
  {
    if( incd[i].frame < incd[i].numframes )
    {
      incd[i].x += incd[i].xm;
      incd[i].y += incd[i].ym;
      if( incd[i].framecount < incd[i].framespeed )
      {
        incd[i].framecount++;
        continue;
      }
      incd[i].frame++;
      incd[i].framecount = 0;
    }
  }

  for( i=0; i<MAX_INCIDENTALS; i++ )
  {
    if( bincd[i].frame < bincd[i].numframes )
    {
      bincd[i].x += bincd[i].xm;
      bincd[i].y += bincd[i].ym;
      if( bincd[i].framecount < bincd[i].framespeed )
      {
        bincd[i].framecount++;
        continue;
      }
      bincd[i].frame++;
      bincd[i].framecount = 0;
    }
  }
}

void drawlifts( BOOL behind )
{
  int i, j;
  u8 *lasttexp;
	GXTexObj texObj;

  lasttexp = NULL;
  for( i=0; lifts[i].numstops!=-1; i++ )
  {
    if( behind != lifts[i].behind ) continue;

    if( lifts[i].texp != lasttexp )
    {
      if( lasttexp ) unbindtexture();
      bindtexture( &texObj, lifts[i].texp );
      lasttexp = lifts[i].texp;
    }

    if( lifts[i].fromstop == -1 )
    {
      for( j=0; j<lifts[i].numstops; j++ )
      {
        if( lifts[i].s ) render_sprite_scaled( lifts[i].s, lifts[i].stops[j*2]-fgx, (lifts[i].stops[j*2+1]-fgy)+lifts[i].bobo+lifts[i].dip, FALSE, 0, lifts[i].scale );
      }
      continue;
    }

    switch( lifts[i].type )
    {
      case LT_BUBBLE:
        render_sprite_scaled( &sprs[((frame>>4)&1)+40], (lifts[i].cx>>8)-fgx, ((lifts[i].cy>>8)-fgy)+lifts[i].bobo+lifts[i].dip, FALSE, 0, lifts[i].scale );
        break;
      
      case LT_FRETURN:
        if( lifts[i].s ) render_sprite_scaled( lifts[i].s+((frame>>1)&1), (lifts[i].cx>>8)-fgx, ((lifts[i].cy>>8)-fgy)+lifts[i].bobo+lifts[i].dip, FALSE, 0, lifts[i].scale );
        break;
      
      case LT_GIRDER:
        {
          int x, y, k;
          x = (lifts[i].cx>>8)-fgx;
          y = ((lifts[i].cy>>8)-fgy)+lifts[i].bobo+lifts[i].dip;
          for( k=(y-57); k > -16; k -=16 )
            render_sprite_tl( &sprt4[32], x-3, k, FALSE );
          render_sprite_tl( &sprt4[31], x-10, y-41, FALSE );
          if( lifts[i].s ) render_sprite_scaled( lifts[i].s, x, y, FALSE, 0, lifts[i].scale );
        }
        break;

      default:
        if( lifts[i].s ) render_sprite_scaled( lifts[i].s, (lifts[i].cx>>8)-fgx, ((lifts[i].cy>>8)-fgy)+lifts[i].bobo+lifts[i].dip, FALSE, 0, lifts[i].scale );
        break;
    }
  }

  if( lasttexp )
    unbindtexture();
}

void drawsprings( void )
{
  int i;
	GXTexObj texObj;

  for( i=0; springs[i].x!=-1; i++ )
  {
    bindtexture( &texObj, springs[i].texp );
    render_sprite_tl( &springs[i].sl[springs[i].frames[springs[i].frame]], springs[i].x-fgx, springs[i].y-fgy, FALSE );
    unbindtexture();
  }
}

void drawstars( void )
{
  int i;
	GXTexObj texObj;

  bindtexture( &texObj, mgsprites );
  for( i=0; i<MAX_STARS; i++ )
  {
    if( stars[i].frame < 23 )
      render_sprite( &sprtg[stars[i].frame], (stars[i].x>>8)-fgx, (stars[i].y>>8)-fgy, FALSE, 0 );
  }
  unbindtexture();
}

void drawincidentals( void )
{
  int i;
	GXTexObj texObj;

  for( i=0; i<MAX_INCIDENTALS; i++ )
  {
    if( incd[i].frame < incd[i].numframes )
    {
      bindtexture( &texObj, incd[i].texp );
      render_sprite( &incd[i].s[incd[i].frames[incd[i].frame]], incd[i].x-fgx, incd[i].y-fgy, FALSE, 0 );
      unbindtexture();
    }
  }
}

void drawbgincidentals( void )
{
  int i;
	GXTexObj texObj;

  for( i=0; i<MAX_INCIDENTALS; i++ )
  {
    if( bincd[i].frame < bincd[i].numframes )
    {
      bindtexture( &texObj, bincd[i].texp );
      render_sprite( &bincd[i].s[bincd[i].frames[bincd[i].frame]], bincd[i].x-bgx, bincd[i].y-bgy, FALSE, 0 );
      unbindtexture();
    }
  }
}

void render_coincount( void )
{
  int x;
	GXTexObj texObj;

  bindtexture( &texObj, mgsprites );
  render_sprite_tl( &sprtg[29], 263, 12, FALSE );
  x = 286;
  if( gid.coins >= 100 ) { render_sprite_tl( &sprtg[30+(gid.coins/100)], x, 18, FALSE ); x+=11; }
  if( gid.coins >= 10 )  { render_sprite_tl( &sprtg[30+((gid.coins/10)%10)], x, 18, FALSE ); x+=11; }
  render_sprite_tl( &sprtg[30+(gid.coins%10)], x, 18, FALSE );
  unbindtexture();
}

void render_lives( void )
{
  int i, j;
	GXTexObj texObj;

  j = gid.lives;
  if( j < 0 ) j = 0;

  bindtexture( &texObj, msprites );
  if( ( clevel == 4 ) && ( gid.hardhat ) )
  {
    render_sprite_tl( &sprs[27], 4,  8, FALSE );
    render_sprite_tl( &sprs[28], 4, 32, FALSE );
  } else {
    render_sprite_tl( &sprs[6], 4, 8, FALSE );
  }
  unbindtexture();

  bindtexture( &texObj, mgsprites );
  render_sprite_tl( &sprtg[30+(j%10)], 52, 9, FALSE );

  for( i=0; i<gid.energy; i++ )
    render_sprite_tl( &sprtg[18], 50+i*20, 21, FALSE );
  unbindtexture();
}

void render_invbox( void )
{
  float tl, tt, tr, tb;
  float vl, vt, vr, vb;
  int x2;
	Mtx m,m1,m2, mv;
	GXTexObj texObj;

  bindtexture( &texObj, mtexts );
	guMtxIdentity( m1 );
  guMtxScaleApply( m1, m1, ibscale, ibscale, 1.0f );
	guMtxRotAxisDeg( m2, &axis, ibrot );
	guMtxConcat( m2, m1, m );
	guMtxTransApply( m, m, 160.0f, 28.0f, 0 );
	guMtxConcat( GXmodelView2D, m, mv );
	GX_LoadPosMtxImm( mv, GX_PNMTX0 );

  GX_Begin( GX_QUADS, GX_VTXFMT0, 4 );
    GX_Position3f32( -92, -22.5f, 0.0f ); GX_Color4u8( 0xff, 0xff, 0xff, ibalpha ); GX_TexCoord2f32(     0.0f,        0.0f );
    GX_Position3f32(  92, -22.5f, 0.0f ); GX_Color4u8( 0xff, 0xff, 0xff, ibalpha ); GX_TexCoord2f32( 0.71875f,        0.0f );
    GX_Position3f32(  92,  22.5f, 0.0f ); GX_Color4u8( 0xff, 0xff, 0xff, ibalpha ); GX_TexCoord2f32( 0.71875f, 0.17578125f );
    GX_Position3f32( -92,  22.5f, 0.0f ); GX_Color4u8( 0xff, 0xff, 0xff, ibalpha ); GX_TexCoord2f32(     0.0f, 0.17578125f );
  GX_End();

  if( ibw[0] > 0.0f )
  {
    vl = -ibw[0]/2.0f; tl = 0.0f;
    vt = -10.0f;       tt = (128.0f/256.0f);
    vr = ibw[0]/2.0f;  tr = ibtw[0];
    vb = 0.0f;         tb = tt + (10.0f/256.0f);
    GX_Begin( GX_QUADS, GX_VTXFMT0, 4 );
      GX_Position3f32( vl, vt, 0.0f ); GX_Color4u8( 0xff, 0xff, 0xff, ibalpha ); GX_TexCoord2f32( tl, tt );
      GX_Position3f32( vr, vt, 0.0f ); GX_Color4u8( 0xff, 0xff, 0xff, ibalpha ); GX_TexCoord2f32( tr, tt );
      GX_Position3f32( vr, vb, 0.0f ); GX_Color4u8( 0xff, 0xff, 0xff, ibalpha ); GX_TexCoord2f32( tr, tb );
      GX_Position3f32( vl, vb, 0.0f ); GX_Color4u8( 0xff, 0xff, 0xff, ibalpha ); GX_TexCoord2f32( tl, tb );
    GX_End();
  }

  if( ( ibw[1] > 0.0f ) &&
      ( ibpos < 81 ) &&
      ( (ibpos+ibw[1]) > -81 ) )
  {
    if( ibpos >= -81 )
    { 
      vl = ibpos; tl = 0.0f;
    } else {
      vl = -81.0f; tl = (-81.0f-(float)ibpos)/256.0f;
    }

    if( (ibpos+ibw[1]) < 81 )
    {
      vr = ibpos+ibw[1]; tr = ibtw[1];
    } else {
      vr = 81.0f; tr = (ibw[1] - (float)((ibpos+ibw[1])-81)) / 256.0f;
    }

    vt = 0.0f; tt = (139.0f/256.0f);
    vb = 10.0f; tb = tt + (10.0f/256.0f);
    GX_Begin( GX_QUADS, GX_VTXFMT0, 4 );
      GX_Position3f32( vl, vt, 0.0f ); GX_Color4u8( 0xff, 0xff, 0xff, ibalpha ); GX_TexCoord2f32( tl, tt );
      GX_Position3f32( vr, vt, 0.0f ); GX_Color4u8( 0xff, 0xff, 0xff, ibalpha ); GX_TexCoord2f32( tr, tt );
      GX_Position3f32( vr, vb, 0.0f ); GX_Color4u8( 0xff, 0xff, 0xff, ibalpha ); GX_TexCoord2f32( tr, tb );
      GX_Position3f32( vl, vb, 0.0f ); GX_Color4u8( 0xff, 0xff, 0xff, ibalpha ); GX_TexCoord2f32( tl, tb );
    GX_End();
  }

  x2 = ibpos+iboff;
  if( ( ibw[2] > 0.0f ) &&
      ( x2 < 81 ) &&
      ( (x2+ibw[2]) > -81 ) )
  {
    if( x2 >= -81 )
    { 
      vl = x2; tl = 0.0f;
    } else {
      vl = -81.0f; tl = (-81.0f-(float)x2)/256.0f;
    }

    if( (x2+ibw[2]) < 81 )
    {
      vr = x2+ibw[2]; tr = ibtw[2];
    } else {
      vr = 81.0f; tr = (ibw[2] - (float)((x2+ibw[2])-81)) / 256.0f;
    }

    vt = 0.0f; tt = (150.0f/256.0f);
    vb = 10.0f; tb = tt + (10.0f/256.0f);
    GX_Begin( GX_QUADS, GX_VTXFMT0, 4 );
      GX_Position3f32( vl, vt, 0.0f ); GX_Color4u8( 0xff, 0xff, 0xff, ibalpha ); GX_TexCoord2f32( tl, tt );
      GX_Position3f32( vr, vt, 0.0f ); GX_Color4u8( 0xff, 0xff, 0xff, ibalpha ); GX_TexCoord2f32( tr, tt );
      GX_Position3f32( vr, vb, 0.0f ); GX_Color4u8( 0xff, 0xff, 0xff, ibalpha ); GX_TexCoord2f32( tr, tb );
      GX_Position3f32( vl, vb, 0.0f ); GX_Color4u8( 0xff, 0xff, 0xff, ibalpha ); GX_TexCoord2f32( tl, tb );
    GX_End();
  }

  unbindtexture();
}

void special_animations( void )
{
  switch( clevel )
  {
    case 1:
      if( !strig[ST1_TREE_1UP] )
      {
        if( ( gid.py <= 270 ) &&
            ( gid.px >= 1320 ) &&
            ( gid.px < 1400 ) )
          strig[ST1_TREE_1UP] = 1;
        break;
      }

      if( ( g_things[1]->active ) &&
          ( g_things[1]->y < 184 ) )
        g_things[1]->y+=6;
      break;
    
    case 4:
      if( !strig[ST4_POLE_1UP] )
      {
        if( ( gid.py <= 278 ) &&
            ( gid.px >= 3280 ) &&
            ( gid.px < 3440 ) )
          strig[ST4_POLE_1UP] = 1;
        break;
      }

      if( ( g_things4[9].active ) &&
          ( g_things4[9].y < 232 ) )
      {
        g_things4[9].y+=6;
        if( g_things4[9].y > 232 )
          g_things4[9].y = 232;
      }
      break;
  }
}

void animate_thingy( struct thingy *t )
{
  if( ( !t->active ) || ( t->numframes < 2 ) )
    return;
  
  if( t->framecount < t->frametime )
  {
    t->framecount++;
    return;
  }

  t->framecount = 0;
  t->frame = (t->frame+1)%t->numframes;
}

void you_now_have( char *what )
{
  set_ibstr( 0, "You now have" );
  set_ibstr( 1, what );
  set_ibstr( 2, NULL );
  ibpos = 82;

  if( (strlen(what)*8) > 194 )
  {
    ibadd  = -2;
    ibwait = 160;
    ibdest = -(strlen(what)*8+82);
  } else {
    ibadd = -4;
    ibwait = 60;
    ibdest = -strlen(what)*4;
  }

  ibstate = 4;
}

void collect_nthingy( struct thingy *t )
{
  int tl, tt, tr, tb;

  if( ( !t->active ) || ( (t->flags&(THF_COLLECTABLE|THF_DEADLY)) == 0 ) ) return;

  thingy_bounds( t, sprs, &tl, &tt, &tr, &tb );
  if( ( (gid.px-12) >= tr ) ||
      ( (gid.py-12) >= tb ) ||
      ( (gid.px+12) < tl )  ||
      ( (gid.py+12) < tt ) )
    return;

  if( t->flags & THF_COLLECTABLE )
  {
    t->active = FALSE;
    return;
  }

  // Must be deadly
  giddyhit();
}

void collect_pthingy( struct thingy *t )
{
  int tl, tt, tr, tb;

  if( ( !t->active ) || ( (t->flags&(THF_COLLECTABLE|THF_DEADLY)) == 0 ) ) return;

  thingy_bounds( t, psprs, &tl, &tt, &tr, &tb );
  if( ( (gid.px-12) >= tr ) ||
      ( (gid.py-12) >= tb ) ||
      ( (gid.px+12) < tl )  ||
      ( (gid.py+12) < tt ) )
    return;

  if( t->flags & THF_COLLECTABLE )
  {
    switch( clevel )
    {
      case 1:
        switch( t->frames[0] )
        {
          case 24: // Whacking great bomb
            ring_o_stars( (tr-tl)/2+tl, (tb-tt)/2+tt, 0.0f, 3.14159265f/4.0f, 8, 720.f );
            inv[INV_BOMB] = 1;
            you_now_have( ivtexts[INV_BOMB].pickup );
            strig[ST1_BOMB_COLLECTED] = 1;
            actionsound( SND_ITEMGET, sfxvol );
            break;
            
          case 31: // Boot
            ring_o_stars( (tr-tl)/2+tl, (tb-tt)/2+tt, 0.0f, 3.14159265f/4.0f, 8, 720.f );
            inv[INV_BOOT] = 1;
            strig[ST1_BOOT_COLLECTED] = 1;
            you_now_have( ivtexts[INV_BOOT].pickup );
            actionsound( SND_ITEMGET, sfxvol );
            break;

          case 35: // Barrel
            ring_o_stars( (tr-tl)/2+tl, (tb-tt)/2+tt, 0.0f, 3.14159265f/4.0f, 8, 720.f );
            inv[INV_BARREL] = 1;
            you_now_have( ivtexts[INV_BARREL].pickup );
            actionsound( SND_ITEMGET, sfxvol );
            break;
        }
        break;
      
      case 2:
        switch( t->frames[0] )
        {
          case 69: // Control Box
            ring_o_stars( (tr-tl)/2+tl, (tb-tt)/2+tt, 0.0f, 3.14159265f/4.0f, 8, 720.f );
            inv[INV_CONTROLBOX] = 1;
            you_now_have( ivtexts[INV_CONTROLBOX].pickup );
            actionsound( SND_ITEMGET, sfxvol );
            break;
          
          case 74: // Printed photo
            ring_o_stars( (tr-tl)/2+tl, (tb-tt)/2+tt, 0.0f, 3.14159265f/4.0f, 8, 720.f );
            inv[INV_PRINTEDPHOTO] = 1;
            you_now_have( ivtexts[INV_PRINTEDPHOTO].pickup );
            actionsound( SND_ITEMGET, sfxvol );
            break;
        }
        break;
      
      case 4:
        switch( t->frames[0] )
        {
          case 27: // Coggy the cog
            ring_o_stars( (tr-tl)/2+tl, (tb-tt)/2+tt, 0.0f, 3.14159265f/4.0f, 8, 720.f );
            inv[INV_LARGECOG] = 1;
            you_now_have( ivtexts[INV_LARGECOG].pickup );
            actionsound( SND_ITEMGET, sfxvol );
            break;
          
          case 36: // Battery
            ring_o_stars( (tr-tl)/2+tl, (tb-tt)/2+tt, 0.0f, 3.14159265f/4.0f, 8, 720.f );
            inv[INV_FLATBATTERY] = 1;
            you_now_have( ivtexts[INV_FLATBATTERY].pickup );
            actionsound( SND_ITEMGET, sfxvol );
            break;
        }
        break;
      
      case 5:
        switch( t->frames[0] )
        {
          case 57: // Charged battery
            ring_o_stars( (tr-tl)/2+tl, (tb-tt)/2+tt, 0.0f, 3.14159265f/4.0f, 8, 720.f );
            p_things5[19].active = FALSE;
            strig[ST5_BATTERY_COLLECTED] = 1;
            inv[INV_CHARGEDBATTERY] = 1;
            you_now_have( ivtexts[INV_CHARGEDBATTERY].pickup );
            actionsound( SND_ITEMGET, sfxvol );
            break;
        }
        break;
    }

    t->active = FALSE;
    return;
  }

  // Must be deadly
  giddyhit();
}

void collect_gthingy( struct thingy *t )
{
  int tl, tt, tr, tb;

  if( ( !t->active ) || ( (t->flags&(THF_COLLECTABLE|THF_DEADLY)) == 0 ) ) return;

  thingy_bounds( t, sprtg, &tl, &tt, &tr, &tb );
  if( ( (gid.px-12) >= tr ) ||
      ( (gid.py-12) >= tb ) ||
      ( (gid.px+12) < tl )  ||
      ( (gid.py+12) < tt ) )
    return;

  if( t->flags & THF_COLLECTABLE )
  {
    switch( t->frames[0] )
    {
      case 10: // Coin?
        ring_o_stars( t->x, t->y, -3.14159265f/8.0f, -((3.14159265f/8.0f)*6.0f)/4.0f, 5, 720.0f );
        gid.coins++;
        inv[INV_COINS] = 1;
        playsound( C_COIN, SND_COIN, sfxvol );
        break;
      
      case 41: // Plank
        ring_o_stars( (tr-tl)/2+tl, (tb-tt)/2+tt, 0.0f, 3.14159265f/4.0f, 8, 720.f );
        inv[INV_PLANK] = 1;
        you_now_have( ivtexts[INV_PLANK].pickup );
        actionsound( SND_ITEMGET, sfxvol );
        break;

      case 64: // Hosepipe
        ring_o_stars( (tr-tl)/2+tl, (tb-tt)/2+tt, 0.0f, 3.14159265f/4.0f, 8, 720.f );
        inv[INV_HOSEPIPE] = 1;
        you_now_have( ivtexts[INV_HOSEPIPE].pickup );
        actionsound( SND_ITEMGET, sfxvol );
        break;        

      case 65: // Air Horn
        ring_o_stars( (tr-tl)/2+tl, (tb-tt)/2+tt, 0.0f, 3.14159265f/4.0f, 8, 720.f );
        inv[INV_AIRHORN] = 1;
        you_now_have( ivtexts[INV_AIRHORN].pickup );
        actionsound( SND_ITEMGET, sfxvol );
        break;

      case 66: // Candle
        ring_o_stars( (tr-tl)/2+tl, (tb-tt)/2+tt, 0.0f, 3.14159265f/4.0f, 8, 720.f );
        inv[INV_CANDLESTICK] = 1;
        you_now_have( ivtexts[INV_CANDLESTICK].pickup );
        actionsound( SND_ITEMGET, sfxvol );
        break;
      
      case 72: // Plunger
        ring_o_stars( (tr-tl)/2+tl, (tb-tt)/2+tt, 0.0f, 3.14159265f/4.0f, 8, 720.f );
        inv[INV_DETONATOR] = 1;
        you_now_have( ivtexts[INV_DETONATOR].pickup );
        actionsound( SND_ITEMGET, sfxvol );
        break;

      case 73: // Diamond
        ring_o_stars( (tr-tl)/2+tl, (tb-tt)/2+tt, 0.0f, 3.14159265f/4.0f, 8, 720.f );
        inv[INV_DIAMOND] = 1;
        you_now_have( ivtexts[INV_DIAMOND].pickup );
        strig[ST4_DIAMOND_COLLECTED] = 1;
        actionsound( SND_ITEMGET, sfxvol );
        break;        

      case 74: // Hard hat
        ring_o_stars( (tr-tl)/2+tl, (tb-tt)/2+tt, 0.0f, 3.14159265f/4.0f, 8, 720.f );
        inv[INV_HARDHAT] = 1;
        you_now_have( ivtexts[INV_HARDHAT].pickup );
        actionsound( SND_ITEMGET, sfxvol );
        break;        

      case 75: // Mirror
        ring_o_stars( (tr-tl)/2+tl, (tb-tt)/2+tt, 0.0f, 3.14159265f/4.0f, 8, 720.f );
        inv[INV_MIRROR] = 1;
        you_now_have( ivtexts[INV_MIRROR].pickup );
        actionsound( SND_ITEMGET, sfxvol );
        break;        

      case 76: // Turps
        ring_o_stars( (tr-tl)/2+tl, (tb-tt)/2+tt, 0.0f, 3.14159265f/4.0f, 8, 720.f );
        inv[INV_TURPS] = 1;
        you_now_have( ivtexts[INV_TURPS].pickup );
        actionsound( SND_ITEMGET, sfxvol );
        break;        
      
      case 77: // Electrical toolkit
        ring_o_stars( (tr-tl)/2+tl, (tb-tt)/2+tt, 0.0f, 3.14159265f/4.0f, 8, 720.f );
        inv[INV_ELECTRICALTOOLKIT] = 1;
        you_now_have( ivtexts[INV_ELECTRICALTOOLKIT].pickup );
        actionsound( SND_ITEMGET, sfxvol );
        break;
      
      case 70: // Lit candle
      case 78:
        ring_o_stars( (tr-tl)/2+tl, (tb-tt)/2+tt, 0.0f, 3.14159265f/4.0f, 8, 720.f );
        inv[INV_LIGHTEDCANDLE] = 1;
        strig[ST3_CANDLE_COLLECTED] = 1;
        g_things3[12].active = FALSE;
        g_things3[13].active = FALSE;
        you_now_have( ivtexts[INV_LIGHTEDCANDLE].pickup );
        actionsound( SND_ITEMGET, sfxvol );
        break;

      case 80: // Lump of carbon
        ring_o_stars( (tr-tl)/2+tl, (tb-tt)/2+tt, 0.0f, 3.14159265f/4.0f, 8, 720.f );
        inv[INV_LUMPOFCARBON] = 1;
        you_now_have( ivtexts[INV_LUMPOFCARBON].pickup );
        actionsound( SND_ITEMGET, sfxvol );
        break;
      
      case 81: // Scissors
        ring_o_stars( (tr-tl)/2+tl, (tb-tt)/2+tt, 0.0f, 3.14159265f/4.0f, 8, 720.f );
        inv[INV_SCISSORS] = 1;
        you_now_have( ivtexts[INV_SCISSORS].pickup );
        actionsound( SND_ITEMGET, sfxvol );
        break;
      
      case 82: // Bubble gum
        ring_o_stars( (tr-tl)/2+tl, (tb-tt)/2+tt, 0.0f, 3.14159265f/4.0f, 8, 720.f );
        inv[INV_BUBBLEGUM] = 1;
        you_now_have( ivtexts[INV_BUBBLEGUM].pickup );
        strig[ST3_GUM_COLLECTED] = 1;
        actionsound( SND_ITEMGET, sfxvol );
        break;

      case 83: // Camera
        ring_o_stars( (tr-tl)/2+tl, (tb-tt)/2+tt, 0.0f, 3.14159265f/4.0f, 8, 720.f );
        inv[INV_DIGITALCAMERA] = 1;
        you_now_have( ivtexts[INV_DIGITALCAMERA].pickup );
        strig[ST2_CAMERA_COLLECTED] = 1;
        actionsound( SND_ITEMGET, sfxvol );
        break;

      case 84: // Lard
        ring_o_stars( (tr-tl)/2+tl, (tb-tt)/2+tt, 0.0f, 3.14159265f/4.0f, 8, 720.f );
        inv[INV_LARD] = 1;
        you_now_have( ivtexts[INV_LARD].pickup );
        actionsound( SND_ITEMGET, sfxvol );
        break;
      
      case 86: // Chalkie
        ring_o_stars( (tr-tl)/2+tl, (tb-tt)/2+tt, 0.0f, 3.14159265f/4.0f, 8, 720.f );
        inv[INV_INDIGESTIONPILLS] = 1;
        you_now_have( ivtexts[INV_INDIGESTIONPILLS].pickup );
        actionsound( SND_ITEMGET, sfxvol );
        break;

      case 87: // Catapult
        ring_o_stars( (tr-tl)/2+tl, (tb-tt)/2+tt, 0.0f, 3.14159265f/4.0f, 8, 720.f );
        inv[INV_CATAPULT] = 1;
        you_now_have( ivtexts[INV_CATAPULT].pickup );
        actionsound( SND_ITEMGET, sfxvol );
        break;

      case 88: // CD
        ring_o_stars( (tr-tl)/2+tl, (tb-tt)/2+tt, 0.0f, 3.14159265f/4.0f, 8, 720.f );
        inv[INV_CD] = 1;
        you_now_have( ivtexts[INV_CD].pickup );
        actionsound( SND_ITEMGET, sfxvol );
        break;
      
      case 92:
      case 93: // Spade
        ring_o_stars( (tr-tl)/2+tl, (tb-tt)/2+tt, 0.0f, 3.14159265f/4.0f, 8, 720.f );
        inv[INV_SPADE] = 1;
        g_things4[5].active = FALSE;
        g_things4[6].active = FALSE;
        you_now_have( ivtexts[INV_SPADE].pickup );
        actionsound( SND_ITEMGET, sfxvol );
        break;

      case 97: // Weather balloon wreckage
        ring_o_stars( (tr-tl)/2+tl, (tb-tt)/2+tt, 0.0f, 3.14159265f/4.0f, 8, 720.f );
        inv[INV_BALLOONWRECKAGE] = 1;
        you_now_have( ivtexts[INV_BALLOONWRECKAGE].pickup );
        actionsound( SND_ITEMGET, sfxvol );
        break;

      case 98: // Teleporter watch
        ring_o_stars( (tr-tl)/2+tl, (tb-tt)/2+tt, 0.0f, 3.14159265f/4.0f, 8, 720.f );
        inv[INV_TELEPORTERWATCH] = 1;
        you_now_have( ivtexts[INV_TELEPORTERWATCH].pickup );
        actionsound( SND_ITEMGET, sfxvol );
        break;

      case 99: // 1UP
        ring_o_stars( t->x, t->y, -3.14159265f/8.0f, -((3.14159265f/8.0f)*6.0f)/4.0f, 5, 720.0f );
        gid.lives++;
        giddy_say( "Huzzah! Eggstra life!" );
        actionsound( SND_1UP, sfxvol );
        break;
      
      case 100: // Burger
      case 102: // Ice cream
      case 103: // Fairy cake
      case 104: // Radish
      case 105: // Cheese
        if( gid.energy >= 3 ) return;
        ring_o_stars( t->x, t->y, -3.14159265f/8.0f, -((3.14159265f/8.0f)*6.0f)/4.0f, 5, 720.0f );
        gid.energy++;
        giddy_say( "Chomp!" );
        actionsound( SND_CHOMP, sfxvol );
        break;
    }

    t->active = FALSE;
    return;
  }

  // Must be deadly
  giddyhit();
}

void timing( void )
{
  int i;

  if( fadeadd > 0 )
  {
    fadea += fadeadd;
    if( fadea > 255 )
    {
      fadea = 255;
      fadeadd = 0;
    }
  } else if( fadeadd < 0 ) {
    fadea += fadeadd;
    if( fadea < 0 )
    {
      fadea = 0;
      fadeadd = 0;
    }
  }

  switch( what_are_we_doing )
  {
    case WAWD_TITLES:
    case WAWD_MENU:
      title_timing();
      return;

	  case WAWD_ENDING:
			ending_timing();
		  return;
	}

  if( paused )
  {
    animate_giddyspeak();
    return;
  }

	if( infobulbalpha > 0 )
	{
		infobulbalpha-=7;
		if( infobulbalpha < 0 )
			infobulbalpha = 0;
		infobulbzoom += 0.08f;
	}

	if( enterhit )
  {
    if( winfo != -1 )
    {
      if( gid.speakstate == 0 )
      {
        giddy_say( infos[winfo].txt );
        actionsound( SND_SPEECHBUB, sfxvol );
      } else {
        gid.speakc = 0;
      }
    }
    enterhit = FALSE;
  }

  if( ( pjump ) && ( gid.usemode ) )
  {
    pjump = FALSE;
    spacehit = TRUE;
    ignorejump = TRUE;
  }

  if( spacehit )
  {
    switch( ibstate )
    {
      case 0:
        if( ( gid.jumping ) || ( !gid.allowjump ) || ( pjump ) )
          break;
        set_ibstr( 0, "** Use **" );
        for( i=0; i<INV_LAST; i++ )
          if( inv[(i+lastinv)%INV_LAST] ) break;
        if( i<INV_LAST )
        {
          ibitem = (i+lastinv)%INV_LAST;
          set_ibstr( 1, ivtexts[ibitem].use );
          set_ibstr( 2, NULL );
          ibpos = 82;
          ibdest = -strlen(ivtexts[ibitem].use)*4;
          ibadd = -4;
          ibstate++;
        } else {
          ibitem = -1;
          set_ibstr( 1, "Scotch mist?" );
          set_ibstr( 2, NULL );
          ibpos = 82;
          ibdest = -12*4;
          ibadd = -4;
          ibstate++;
        }
        actionsound( SND_USE, sfxvol );
        break;

      case 2:
        if( ibadd != 0 ) break;

        lastinv = ibitem;

        ibstate++;

        if( ( ibitem >= 0 ) && ( inv[ibitem] != 0 ) )
        {
          BOOL complain;

          complain = TRUE;
          switch( ibitem )
          {
            case INV_COINS:
              if( ( clevel == 4 ) && ( winfo == 3 ) )
              {
                giddy_say( "Who ya gonna call?" );
                actionsound( SND_USEFAIL, sfxvol );
                complain = FALSE;
                break;
              }

              if( triggergummachine() ) complain = FALSE;
              break;

            case INV_BARREL:
              if( ( clevel == 1 ) && ( winfo == 2 ) )
              {
                giddy_say( "I fear it would only\n"
                           "fuel his depression." );
                actionsound( SND_USEFAIL, sfxvol );
                complain = FALSE;
                break;
              }

              if( triggerslug() ) complain = FALSE;
              break;
            
            case INV_LARD:
              if( triggersludgemonster() ) complain = FALSE;
              break;
            
            case INV_BUBBLEGUM:
              if( triggerburstpipe() ) complain = FALSE;
              break;
            
            case INV_ELECTRICALTOOLKIT:
              if( triggerphonebox() ) complain = FALSE;
              break;
            
            case INV_BOOT:
              if( triggerjunkchuteswitcheroo() ) complain = FALSE;
              break;
            
            case INV_DETONATOR:
              if( placedetonator() ) complain = FALSE;
              break;
            
            case INV_CATAPULT:
              if( ( clevel == 3 ) && ( winfo == 2 ) )
              {
                giddy_say( "Thats not very nice!" );
                actionsound( SND_USEFAIL, sfxvol );
                complain = FALSE;
                break;
              }

              if( triggerballoon() ) complain = FALSE;
              break;
            
            case INV_BALLOONWRECKAGE:
              if( triggermuldoonandskelly() ) complain = FALSE;
              break;
            
            case INV_ATEAMPHONENO:
              if( triggerateamvan() ) complain = FALSE;
              break;
            
            case INV_PLANK:
              if( ( clevel == 3 ) && ( winfo == 2 ) )
              {
                giddy_say( "Thats not very nice!" );
                actionsound( SND_USEFAIL, sfxvol );
                complain = FALSE;
                break;
              }

              if( placeplank() ) complain = FALSE;
              break;
            
            case INV_SCISSORS:
              if( triggerseesaw() ) complain = FALSE;
              break;
            
            case INV_CHARGEDBATTERY:
              if( triggertripledoors() ) complain = FALSE;
              break;
            
            case INV_TELEPORTERWATCH:
              complain = FALSE;
              if( clevel == 5 )
              {
                giddy_say( "Apparently, it only\n"
                           "works from outside." );
                actionsound( SND_USEFAIL, sfxvol );
                break;
              }

              gid.sgwsc = 1.0f;
              gid.wwa = 255;
              gid.wwl = 5;
              gid.watchwarp = TRUE;
              gid.def = 1;
              incidentalsound( SND_TELEPORT_OUT, sfxvol );
              break;
            
            case INV_DIGITALCAMERA:
              if( triggerbigscreen1() ) complain = FALSE;
              break;

            case INV_PRINTEDPHOTO:
              if( triggerbigscreen2() ) complain = FALSE;
              break;

            case INV_INDIGESTIONPILLS:
              if( triggerbiledude() ) complain = FALSE;
              break;
            
            case INV_CD:
              if( triggereel() ) complain = FALSE;
              break;
            
            case INV_LARGECOG:
              if( triggerrecyclotron() ) complain = FALSE;
              break;
            
            case INV_LUMPOFCARBON:
              if( triggercrusher() ) complain = FALSE;
              break;

            case INV_AIRHORN:
              if( honkatdragon() ) complain = FALSE;
              break;
            
            case INV_TURPS:
              if( drinkfordragon() ) complain = FALSE;
              break;
            
            case INV_CANDLESTICK:
              if( triggerdragon() ) complain = FALSE;
              break;
            
            case INV_LIGHTEDCANDLE:
              if( triggertorches() ) complain = FALSE;
              break;
            
            case INV_DIAMOND:
              if( triggerlockblockzapper() ) complain = FALSE;
              break;
            
            case INV_FLATBATTERY:
              if( ( clevel == 5 ) && ( winfo == 0 ) )
              {
                giddy_say( "Err, the battery is flat." );
                actionsound( SND_USEFAIL, sfxvol );
                complain = FALSE;
                break;
              }

              if( triggercyclingalien() ) complain = FALSE;
              break;
            
            case INV_HOSEPIPE:
              if( triggersprinkler() ) complain = FALSE;
              break;
            
            case INV_CONTROLBOX:
              if( triggerpluggrabber() ) complain = FALSE;
              break;
            
            case INV_CAMERAWITHPHOTO:
              if( triggerprinter() ) complain = FALSE;
              break;
            
            case INV_HARDHAT:
              if( triggerbuilder() ) complain = FALSE;
              break;
            
            case INV_MIRROR:
              if( triggerlaserbeam() ) complain = FALSE;
              break;
            
            case INV_SPADE:
              if( triggerwhackinggreatbomb() ) complain = FALSE;
              break;
            
            case INV_BOMB:
              if( triggereggsterminatorproductionline() ) complain = FALSE;
              break;
          }

          if( complain )
          {
            if( winfo == -1 )
              giddy_say( "What? Where?" );
            else
              giddy_say( "Perhaps not." );
            actionsound( SND_USEFAIL, sfxvol );
          }
        }
        break;
    }
    spacehit = FALSE;
  }

  frame++;

  if( ibadd != 0 )
  {
    ibpos += ibadd;
    if( ibadd < 0 )
    {
      if( ibpos <= ibdest )
      {
        ibpos = ibdest;
        ibadd = 0;
        if( ibt[2] != NULL )
        {
          set_ibstr( 1, ibt[2] );
          set_ibstr( 2, NULL );
          ibpos += iboff;
        }
      }
    } else {
      if( ibpos >= ibdest )
      {
        ibpos = ibdest;
        ibadd = 0;
      }
    }
  }

  animate_giddyspeak();
  movestars();
  timeincidentals();
  special_animations();
  animatesprings();
  make_giddy_do_things();

  if( ( clevel == 1 ) &&
      ( gid.x >= (2848<<8) ) &&
      ( strig[ST1_HOSE_PLACED] == 1 ) &&
      ( strig[ST1_BOSS_BEATEN] == 0 ) &&
      ( stuff.bossmode == 0 ) )
    triggerpotatoboss();

  animateenemies( enemies[clevel-1] );
  animateenemies( enemiesb[clevel-1] );
  movelifts();
  triggerfallingblocks();
  dospecialfade();

  for( i=0; i<num_nthings; i++ )
  {
    animate_thingy( n_things[i] );
    collect_nthingy( n_things[i] );
  }
  for( i=0; i<num_bnthings; i++ )
  {
    animate_thingy( bn_things[i] );
    collect_nthingy( bn_things[i] );
  }
  for( i=0; i<num_pthings; i++ )
  {
    animate_thingy( p_things[i] );
    collect_pthingy( p_things[i] );
  }
  for( i=0; i<num_bpthings; i++ )
  {
    animate_thingy( bp_things[i] );
    collect_pthingy( bp_things[i] );
  }
  for( i=0; i<num_gthings; i++ )
  {
    animate_thingy( g_things[i] );
    collect_gthingy( g_things[i] );
  }
  for( i=0; i<num_bgthings; i++ )
  {
    animate_thingy( bg_things[i] );
    collect_gthingy( bg_things[i] );
  }

  switch( clevel )
  {
    case 1:
      animateslug();
      animateeel();
      animatesprinkler();
      break;
    
    case 2:
      animatejunkchute();
      animatesludgemonster();
      animateburstpipe();
      animatetoxicgas();
      animatepluggrabber();
      animaterecyclotron();
      animateprinter();
      break;
    
    case 3:
      animatetardis();
      animatemuldoonandskelly();
      animateboulder();
      animatedragon();
      animateballoon();
      animateseesaw();
      animatefallingblocks();
      animatestargate();
      animateboulder2();
      animatetimerdoor();
      animatelockblockzapper();
      animatewallsteppingstones();
      animategummachine();
      break;
    
    case 4:
      animatemrt();
      animatetardis();
      animatefactory();
      animatebuilder();
      animatecrusher();
      animateninja();
      animatespecialbin();
      break;
    
    case 5:
      animatetripledoors();
      animatebigassfan();
      animatelaserbeam();
      animatebigscreen();
      animatetpbubs();
      animatebiledude();
      animatecyclingalien();
      animateeggsterminatorproductionline();
      break;
  }

  scrollhandler();
  animate_invbox();
  check_infos();

  switch( clevel )
  {
    case 1:
      if( ( gid.px > 6304 ) &&
          ( gid.py < 420 ) &&
          ( clevel == llevel ) )
      {
        llevel = 2;
        fadea = 0;
        fadeadd = 8;
        fadetype = 0;
        gid.stopuntilfade = TRUE;
      }
      break;
    
    case 2:
      if( ( gid.px < 32 ) &&
          ( gid.py >= 1120 ) &&
          ( clevel == llevel ) )
      {
        llevel = 0x8001;
        fadea = 0;
        fadetype = 0;
        fadeadd = 8;
        gid.stopuntilfade = TRUE;
      }

      if( ( gid.px >= 3648 ) &&
          ( gid.px < 3696 ) &&
          ( gid.py < 440 ) &&
          ( clevel == llevel ) )
      {
        llevel = 4;
        fadea = 0;
        fadetype = 0;
        fadeadd = 8;
        gid.fallpuffs = TRUE;
      }
      break;
  }

  return;
}

void video_pre_init( void )
{
	// Initialise the video system
	VIDEO_Init();

	rmode = VIDEO_GetPreferredMode(NULL);
	VIDEO_Configure (rmode);
	xfb[0] = (u32 *)MEM_K0_TO_K1(SYS_AllocateFramebuffer(rmode));
	xfb[1] = (u32 *)MEM_K0_TO_K1(SYS_AllocateFramebuffer(rmode));

	// Initialise the console, required for printf
	console_init(xfb[0],20,20,rmode->fbWidth,rmode->xfbHeight,rmode->fbWidth*VI_DISPLAY_PIX_SZ);

	VIDEO_SetNextFramebuffer(xfb[0]);
	VIDEO_SetBlack(FALSE);
	VIDEO_Flush();
	VIDEO_WaitVSync();
	if(rmode->viTVMode&VI_NON_INTERLACE) VIDEO_WaitVSync();
}

BOOL render_init( void )
{
	f32 yscale;
	u32 xfbHeight;
	Mtx perspective;
  int x, y, i;
  float sc;

  i=0; sc = 1.0f/16.0f;
  for( y=0; y<16; y++ )
    for( x=0; x<16; x++ )
    {
      bt[i].x = ((float)x) * sc;
      bt[i].y = ((float)y) * sc;
      bt[i].w = 1.0f * sc;
      bt[i].h = 1.0f * sc;
      i++;
    }

	WPAD_Init();
	WPAD_SetDataFormat( 0, WPAD_FMT_BTNS );

	VIDEO_SetNextFramebuffer(xfb[fb]);
	VIDEO_SetBlack(FALSE);
	VIDEO_Flush();
	VIDEO_WaitVSync();
	if(rmode->viTVMode&VI_NON_INTERLACE) VIDEO_WaitVSync();

	gp_fifo = (u8 *) memalign(32,DEFAULT_FIFO_SIZE);

	GX_Init (gp_fifo, DEFAULT_FIFO_SIZE);

	// clears the bg to color and clears the z buffer
	GXColor background = { 0, 0, 0, 0xff };
	GX_SetCopyClear (background, 0x00ffffff);

	// other gx setup
	yscale = GX_GetYScaleFactor(rmode->efbHeight,rmode->xfbHeight);
	xfbHeight = GX_SetDispCopyYScale(yscale);
	GX_SetScissor(0,0,rmode->fbWidth,rmode->efbHeight);
	GX_SetDispCopySrc(0,0,rmode->fbWidth,rmode->efbHeight);
	GX_SetDispCopyDst(rmode->fbWidth,xfbHeight);
	GX_SetCopyFilter(rmode->aa,rmode->sample_pattern,GX_TRUE,rmode->vfilter);
	GX_SetFieldMode(rmode->field_rendering,((rmode->viHeight==2*rmode->xfbHeight)?GX_ENABLE:GX_DISABLE));

	if (rmode->aa)
		GX_SetPixelFmt(GX_PF_RGB565_Z16, GX_ZC_LINEAR);
	else
		GX_SetPixelFmt(GX_PF_RGB8_Z24, GX_ZC_LINEAR);

	GX_SetDispCopyGamma(GX_GM_1_0);
 

	// setup the vertex descriptor
	// tells the flipper to expect direct data
	GX_ClearVtxDesc();
		GX_InvVtxCache ();
		GX_InvalidateTexAll();

	GX_SetVtxDesc(GX_VA_TEX0, GX_NONE);
	GX_SetVtxDesc(GX_VA_POS, GX_DIRECT);
	GX_SetVtxDesc (GX_VA_CLR0, GX_DIRECT);


		GX_SetVtxAttrFmt (GX_VTXFMT0, GX_VA_POS, GX_POS_XYZ, GX_F32, 0);
		GX_SetVtxAttrFmt (GX_VTXFMT0, GX_VA_CLR0, GX_CLR_RGBA, GX_RGBA8, 0);
		GX_SetVtxAttrFmt(GX_VTXFMT0, GX_VA_TEX0, GX_TEX_ST, GX_F32, 0);
		GX_SetZMode (GX_FALSE, GX_LEQUAL, GX_TRUE);

	GX_SetNumChans(1);
	GX_SetNumTexGens(1);
	GX_SetTevOp (GX_TEVSTAGE0, GX_PASSCLR);
	GX_SetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0);
	GX_SetTexCoordGen(GX_TEXCOORD0, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY);

	guMtxIdentity(GXmodelView2D);
	guMtxTransApply (GXmodelView2D, GXmodelView2D, 0.0F, 0.0F, -50.0F);
	GX_LoadPosMtxImm(GXmodelView2D,GX_PNMTX0);

	guOrtho(perspective,0,240,0,320,0,300);
	GX_LoadProjectionMtx(perspective, GX_ORTHOGRAPHIC);

	GX_SetViewport(0,0,rmode->fbWidth,rmode->efbHeight,0,1);
	GX_SetBlendMode(GX_BM_BLEND, GX_BL_SRCALPHA, GX_BL_INVSRCALPHA, GX_LO_CLEAR);
	GX_SetAlphaUpdate(GX_TRUE);
	
	GX_SetCullMode(GX_CULL_NONE);

  mblocks    = &blocks[256*256*4];
  msprites   = &sprites[256*256*4];
  mwsprites  = &wsprites[256*256*4];
  mgsprites  = &gsprites[256*256*4];
  mwgsprites = &wgsprites[256*256*4];
  mpsprites  = &psprites[256*256*4];
  mwpsprites = &wpsprites[256*256*4];
  mtexts     = &texts[256*256*4];
  screentex  = &texts[512*256*4];

  texturemangle( 32, 32, &tvborders[0], &tvtex[0] );
	DCFlushRange( &tvtex[0], 32*32*4 );

  return TRUE;
}

void render_shut( void )
{
/*
  if( moozak )
  {
    Mix_FreeMusic( moozak );
    moozak = NULL;
  }
*/
}

void render_fade( void )
{
  lrfade2 = lrfade1;
  lrfade1 = fadea;
  if( fadea == 0 ) return;

	GX_LoadPosMtxImm(GXmodelView2D,GX_PNMTX0);

  if( fadetype == 1 )
  {
    GX_Begin( GX_QUADS, GX_VTXFMT0, 4 );
  		GX_Position3f32(   0.0f,   0.0f, 0.0f ); GX_Color4u8( 255, 255, 255, fadea );
  		GX_Position3f32( 320.0f,   0.0f, 0.0f ); GX_Color4u8( 255, 255, 255, fadea );
  		GX_Position3f32( 320.0f, 240.0f, 0.0f ); GX_Color4u8( 255, 255, 255, fadea );
  		GX_Position3f32(   0.0f, 240.0f, 0.0f ); GX_Color4u8( 255, 255, 255, fadea );
    GX_End();
    return;
  }

  GX_Begin( GX_QUADS, GX_VTXFMT0, 4 );
    GX_Position3f32(   0.0f,   0.0f, 0.0f ); GX_Color4u8(   0,   0,   0, fadea );
    GX_Position3f32( 320.0f,   0.0f, 0.0f ); GX_Color4u8(   0,   0,   0, fadea );
    GX_Position3f32( 320.0f, 240.0f, 0.0f ); GX_Color4u8(   0,   0,   0, fadea );
    GX_Position3f32(   0.0f, 240.0f, 0.0f ); GX_Color4u8(   0,   0,   0, fadea );
  GX_End();
}

void render_spoonedit( void )
{
  float extrax, extray;
  Mtx m,mv;
	GXTexObj texObj;

  if( !gid.spoonedit ) return;

  extrax = sin(gid.spooneditwobble)*gid.spooneditwobblefactor*2.0f;
  extray = cos(gid.spooneditwobble+3.14159265f)*gid.spooneditwobblefactor;

  bindtexture( &texObj, mgsprites );

	guMtxIdentity( m );
	guMtxTransApply( m, m, 160.0f, 100.0f, 0 );
	guMtxConcat( GXmodelView2D, m, mv );
	GX_LoadPosMtxImm( mv, GX_PNMTX0 );

  GX_Begin( GX_QUADS, GX_VTXFMT0, 4 );
    GX_Position3f32( -130.0f-extrax, -18.0f-extray, 0.0f ); GX_Color4u8( 0xff, 0xff, 0xff, gid.spooneditfade ); GX_TexCoord2f32( 101.0f/256.0f,         0.0f );
    GX_Position3f32(  130.0f+extrax, -18.0f-extray, 0.0f ); GX_Color4u8( 0xff, 0xff, 0xff, gid.spooneditfade ); GX_TexCoord2f32( 231.0f/256.0f,         0.0f );
    GX_Position3f32(  130.0f+extrax,  18.0f+extray, 0.0f ); GX_Color4u8( 0xff, 0xff, 0xff, gid.spooneditfade ); GX_TexCoord2f32( 231.0f/256.0f, 17.6f/256.0f );
    GX_Position3f32( -130.0f-extrax,  18.0f+extray, 0.0f ); GX_Color4u8( 0xff, 0xff, 0xff, gid.spooneditfade ); GX_TexCoord2f32( 101.0f/256.0f, 17.6f/256.0f );
  GX_End();

  unbindtexture();
}

extern struct wasp wasps1[];

extern float pk_ang;

BOOL render( void )
{
  int i;//, j, mx, my;
	GXTexObj texObj;
//  int bmx, bmy;
 
  if( gid.savedtheday )
  {
    killtune();
    what_are_we_doing = WAWD_ENDING;
		endingstate = 0;
    return FALSE;
  }

  if( ( gid.spoonedit ) && ( gid.spooneditstate == 3 ) )
  {
    killtune();
    titlestate = 0;
    what_are_we_doing = WAWD_TITLES;
    return FALSE;
  }

  if( ( llevel != clevel ) && ( fadeadd == 0 ) && ( lrfade2 == 255 ) )
  {
    killtune();
    load_level( llevel, FALSE );
    gid.ontardis = NOT_ON_TARDIS;
    fadeadd = -8;
    gid.stopuntilfade = FALSE;
    gid.mainchryoffset = 0;
    GX_InvVtxCache();
    GX_InvalidateTexAll();
  }

  update_ibstrs();

	GX_LoadPosMtxImm(GXmodelView2D,GX_PNMTX0);
  render_background();

  switch( clevel )
  {
    case 4:
      renderfactory();
      renderateamvan();
      break;
  }

  drawbgincidentals();

  for( i=0; i<num_bpthings; i++ )
    render_pthing( bp_things[i] );

  for( i=0; i<num_bgthings; i++ )
    render_gthing( bg_things[i] );

  for( i=0; i<num_bnthings; i++ )
    render_nthing( bn_things[i] );

  drawlifts( TRUE );

  switch( clevel )
  {
    case 1:
      renderslug();
      renderboss();
      break;

    case 2:
      renderrecyclotron();
      rendersludgemonster();
      rendertoxicgas();
      renderpluggrabberbg();
      break;
    
    case 3:
      renderseesawbg();
      break;
    
    case 4:
      rendercrusher();
      renderninjabg();
      renderspecialbin();
      break;
    
    case 5:
      rendertripledoorsbg();
      renderbigscreen();
      rendertpbubs();
      rendereggsterminatorproductionlinebg();
      break;
  }

  renderenemies( enemiesb[clevel-1] );

	GX_LoadPosMtxImm(GXmodelView2D,GX_PNMTX0);
  render_foreground();

  for( i=0; i<num_pthings; i++ )
    render_pthing( p_things[i] );

  for( i=0; i<num_gthings; i++ )
    render_gthing( g_things[i] );

  for( i=0; i<num_nthings; i++ )
    render_nthing( n_things[i] );

  drawincidentals();
  drawlifts( FALSE );
  drawsprings();
  drawstars();

  switch( clevel )
  {
    case 1:
      rendereel();
      rendershootyfmissiles();
      rendersprinkler();
      break;
    
    case 2:
      renderjunkchute();
      rendersludge();
      renderburstpipe();
      renderpluggrabber();
      renderprinter();
      break;
    
    case 3:
      rendertardis();
      rendermuldoonandskelly();
      renderboulder();
      renderdragon();
      renderballoon();
      renderseesaw();
      renderfallingblocks();
      renderstargate();
      renderboulder2();
      renderlockblockzapper();
      rendergummachine();
      break;
    
    case 4:
      rendermrt();
      rendertardis();
      renderbuilder();
      renderninja();
      break;
    
    case 5:
      rendertripledoors();
      renderbigassfan();
      renderlaserbeam();
      renderbiledude();
      rendercyclingalien();
      rendereggsterminatorproductionline();
      break;
  }

  renderenemies( enemies[clevel-1] );

  render_giddy();
  redrawtardisdoor();

  switch( clevel )
  {
    case 1:
      render_foreground_zone( 392, 18, 8, 10 );
      bindtexture( &texObj, msprites );
      for( i=21; i<=26; i++ )
        render_nthing( n_things[i] );
      unbindtexture();
      break;

    case 3:
      hidefallingblocks();
      bindtexture( &texObj, mpsprites );
      for( i=2; i<=15; i++ )
        render_nthing( n_things[i] );
      unbindtexture();
      break;

    case 4:
      renderfactorybits();
      break;
  }

  render_lives();
  render_coincount();
  render_infobulb();
  render_invbox();

  render_giddyspeak();

  render_spoonedit();

  render_tvborders();
  unbindtexture();
  render_fade();

  updatebigscreentex();

  return FALSE;
}
