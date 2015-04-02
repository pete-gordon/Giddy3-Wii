
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <malloc.h>
#include <math.h>

#include <gccore.h>
#include <wiiuse/wpad.h>

#include "gcmodplay.h"

#include "giddy3.h"
#include "render.h"
#include "samples.h"

extern Mtx GXmodelView2D;
extern Vector axis;

extern u8 sprites[], texts[], *mtexts;
extern u8 *msprites;

#define MAXTITLESTARS 200
#define STARSPEED 10

extern time_t gamestarttime, gameendtime;

extern BOOL audioavailable, tunePlaying, musicenabled;
extern int musicvol, sfxvol;
extern int musicvolopt, sfxvolopt, lrfade2;
extern struct what_is_giddy_doing gid;

struct tstar
{
  int x, y;
  int dx, dy;
  int f;
};

struct tstar tstars[MAXTITLESTARS];

extern int what_are_we_doing;
int menunum, mitems;
BOOL startit = FALSE;

int titlestate = 0, nexttstar = 0;

// giddy 3
int titx[] = { 34, 80, 129, 186, 240, 295 };
int tits[] = {  0,  1,   2,   2,   3,   4 };
int tity[6];
float tita[6];

// titles font
int tfw[] = { 214*256*4+  12*4,  6,    // !
              214*256*4+  18*4,  9,    // "
              214*256*4+  27*4, 11,    // #
              214*256*4+  38*4, 11,    // £
              214*256*4+  49*4, 11,    // %
              214*256*4+  60*4, 12,    // &
              214*256*4+  72*4,  5,    // '
              214*256*4+  77*4,  7,    // (
              214*256*4+  84*4,  7,    // )
              214*256*4+  91*4, 11,    // *
              214*256*4+ 102*4,  9,    // +
              214*256*4+ 111*4,  6,    // ,
              214*256*4+ 117*4, 10,    // -
              214*256*4+ 127*4,  6,    // .
              214*256*4+ 133*4, 11,    // /
              214*256*4+ 144*4, 11,    // 0
              214*256*4+ 155*4,  8,    // 1
              214*256*4+ 163*4, 11,    // 2
              214*256*4+ 174*4, 11,    // 3
              214*256*4+ 185*4, 11,    // 4
              214*256*4+ 196*4, 11,    // 5
              214*256*4+ 207*4, 11,    // 6
              214*256*4+ 218*4, 11,    // 7
              214*256*4+ 229*4, 11,    // 8
              214*256*4+ 240*4, 11,    // 9
              214*256*4+ 251*4,  5,    // :
              228*256*4+   0*4,  6,    // ;
              228*256*4+   6*4,  7,    // <
              228*256*4+  13*4,  7,    // =
              228*256*4+  20*4,  7,    // >
              228*256*4+  27*4, 11,    // ?
              228*256*4+  38*4, 11,    // @
              228*256*4+  49*4, 11,    // A
              228*256*4+  60*4, 11,    // B
              228*256*4+  71*4, 11,    // C
              228*256*4+  82*4, 11,    // D
              228*256*4+  93*4, 11,    // E
              228*256*4+ 104*4, 11,    // F
              228*256*4+ 115*4, 11,    // G
              228*256*4+ 126*4, 11,    // H
              228*256*4+ 137*4,  6,    // I
              228*256*4+ 143*4, 11,    // J
              228*256*4+ 154*4, 11,    // K
              228*256*4+ 165*4, 11,    // L
              228*256*4+ 176*4, 11,    // M
              228*256*4+ 187*4, 11,    // N
              228*256*4+ 198*4, 11,    // O
              228*256*4+ 209*4, 11,    // P
              228*256*4+ 220*4, 11,    // Q
              228*256*4+ 231*4, 11,    // R
              228*256*4+ 242*4, 11,    // S
              242*256*4+   0*4, 11,    // T
              242*256*4+  11*4, 11,    // U
              242*256*4+  22*4, 11,    // V
              242*256*4+  33*4, 11,    // W
              242*256*4+  44*4, 11,    // X
              242*256*4+  55*4, 11,    // Y
              242*256*4+  66*4, 11,    // Z
              242*256*4+  77*4,  7,    // [
              242*256*4+  84*4, 11,    //
              242*256*4+  95*4,  7,    // ]
              242*256*4+ 102*4, 18,    // ^
              242*256*4+ 120*4, 18,    // _
              242*256*4+ 138*4, 11 };  // `

int tfg[4*12];

extern int fadea, fadeadd, fadetype, lrfade2;
extern struct btex sprtt[], sprtv[];

char *straplines[] = { "REASONABLY SPECIAL EDITION",
                       "THE RETRO EGGSPERIENCE",
                       "- BETTER THAN SPROUTS -",
                       "WITH GARISH TITLE SCREEN",
                       "FULLY 3D (POSSIBLE LIE)",
                       "- REPROGRAMMED BY HAND -",
                       "WHAT THE FLIPPING EGG?",
                       "- BEARD CONTENT: LOW -",
                       "- HE IS THE EGG-MAN -" };

int nextstrapline=0;

int stx=0, sty=0, sstx=0, ssty=0;
float sta=0.0f, ssta=0;
int strapx, strapw, strapsw, strapy;

int titlepage=0, titlepage_timer;
int titlepage_in_type = 0, titlepage_out_type = 0;

int tpx, tpxo, tpw=0;
int tpy, tpyo, tph=0;
int tpalph;
float tpscale;

BOOL pagerefresh = FALSE;

int menustaro=0, menustary=-1;
float menustara;

void tbindtexture( GXTexObj *texObj, u8 *texp )
{
	GX_InitTexObj( texObj, texp, 512, 256, GX_TF_RGBA8, GX_CLAMP, GX_CLAMP, GX_FALSE );
	GX_InitTexObjLOD( texObj, GX_NEAR, GX_NEAR, 0.0f, 0.0f, 0.0f, 0, 0, GX_ANISO_1 );
	GX_LoadTexObj( texObj, GX_TEXMAP0 );

  GX_SetTevOp( GX_TEVSTAGE0, GX_MODULATE );
  GX_SetVtxDesc( GX_VA_TEX0, GX_DIRECT );
}

static int txwidth( char *str )
{
  int w, i;

  for( w=0, i=0; str[i]; i++ )
  {
    if( ( str[i] < '!' ) || ( str[i] > '`' ) )
    {
      w += 11;
      continue;
    }

    w += tfw[(str[i]-'!')*2+1];
  }

  return w;
}

static int txchar( int x, int y, int c, int col )
{
  int cx, cy, w;
  int cr, cg, cb;
  u32 cmp;
  u8 *s, *d;

  cb = (col & 3) * 12;
  cg = ((col>>2) & 3) * 12;
  cr = ((col>>4) & 3) * 12;

  if( ( c < '!' ) || ( c > '`' ) )
    return 11;

  c = (c-'!')*2;
  w = tfw[c+1];
  s = &sprites[tfw[c]];
  d = &texts[(y*512+x)*4];

  for( cy=0; cy<14; cy++ )
  {
    for( cx=0; cx<w; cx++ )
    {
      cmp = (s[0]<<16)|(s[1]<<8)|s[2];

      switch( cmp )
      {
        case 0x000050:
          *(d++) = tfg[cr+cy]-12;
          *(d++) = tfg[cg+cy]-12;
          *(d++) = tfg[cb+cy]-12;
          *(d++) = s[3];
          s+=4;
          break;
        
        case 0x000060:
          *(d++) = tfg[cr+cy]+15;
          *(d++) = tfg[cg+cy]+15;
          *(d++) = tfg[cb+cy]+15;
          *(d++) = s[3];
          s+=4;
          break;
        
        case 0x000058:
          *(d++) = tfg[cr+cy];
          *(d++) = tfg[cg+cy];
          *(d++) = tfg[cb+cy];
          *(d++) = s[3];
          s+=4;
          break;
        
        default:
          *(d++) = *(s++);
          *(d++) = *(s++);
          *(d++) = *(s++);
          *(d++) = *(s++);
          break;
      }
    }
    s += 1024-(w*4);
    d += 2048-(w*4);
  }

  return w;
}

static void txprint( int x, int y, char *str, int col )
{
  int i, cx;

  for( cx=x, i=0; str[i]; i++ )
    cx += txchar( cx, y, str[i], col );
}

static void txcentre( int y, char *str, int col )
{
  txprint( (320-txwidth(str))/2, y, str, col );
}

#define NUM_IN_TYPES 4
#define NUM_OUT_TYPES 4
#define NUM_PAGES 6

void render_menustars( void )
{
  int x, y;

  if( menustary == -1 ) return;

  for( x=menustaro+16; x<304; x+=8 )
    render_sprite_scaled( &sprtt[5], x, menustary-5, FALSE, menustara, 0.3f + sin( menustara/32.1f )*0.2f );

  for( y=-5+(x-304); y<15; y+=8 )
    render_sprite_scaled( &sprtt[5], 304, menustary+y, FALSE, menustara, 0.3f + sin( menustara/32.1f )*0.2f );

  for( x=304-(y-15); x>16; x-=8 )
    render_sprite_scaled( &sprtt[5], x, menustary+15, FALSE, menustara, 0.3f + sin( menustara/32.1f )*0.2f );

  for( y=16-(16-x); y>-5; y-=8 )
    render_sprite_scaled( &sprtt[5], 16, menustary+y, FALSE, menustara, 0.3f + sin( menustara/32.1f )*0.2f );
}

void setup_titlepage( void )
{
  memset( &texts[0], 0, 512*240*4 );

  if( what_are_we_doing == WAWD_MENU )
  {
    switch( titlepage )
    {
      case 0:
        txcentre( 14, "START GAME",    menunum==0 ? 0x3d : 0x08 );
        txcentre( 32, "OPTIONS",       menunum==1 ? 0x3d : 0x08 );
        txcentre( 50, "CREDITS",       menunum==2 ? 0x3d : 0x08 );
        txcentre( 70, "QUIT",          menunum==3 ? 0x3d : 0x08 );
        menustary = menunum*18+114;
        if( menunum == 3 ) menustary+=2;
        mitems = 4;
        break;
      
      case 1:
        txcentre( 0, "OPTIONS", 0x1b );

        txprint(  22, 32, "SFX VOL", menunum==0 ? 0x3d : 0x08 );
        txprint( 128, 32, "[^_______`]", 0x00 );
        txprint( 135+(sfxvolopt*18), 32, "*", menunum==0 ? 0x3d : 0x08 );
        txprint(  22, 50, "MUSIC VOL", menunum==1 ? 0x3d : 0x08 );
        txprint( 128, 50, "[^_______`]", 0x00 );
        txprint( 135+(musicvolopt*18), 50, "*", menunum==1 ? 0x3d : 0x08 );
        
        txcentre( 98, "BACK",          menunum==2 ? 0x3d : 0x08 );

        switch( menunum )
        {
          case 0: menustary = 132; break;
          case 1: menustary = 150; break;
          case 2: menustary = 198; break;
        }

        mitems = 3; 
        break;
      
      case 2:
        txcentre(  28, "DESIGN, GRAPHICS & SFX:", 0x3d );
        txcentre(  42, "PHIL RUSTON", 0x1b );
        txcentre(  70, "SPECIAL EDITION PROGRAMMING:", 0x3d );
        txcentre(  84, "PETER GORDON", 0x1b );
        menustary = -1;
        mitems = 0;
        break;

      case 3:
        txcentre(   0, "MUSIC BY:", 0x3d );
        txcentre(  14, "LEE BEVAN", 0x1b );
        txcentre(  28, "DANIEL JOHANSSON", 0x1b );
        txcentre(  42, "JOGIER LILJEDAHL", 0x1b );
        txcentre(  56, "SAMI SAARNIO", 0x1b );
        txcentre(  84, "SPECIAL THANKS TO", 0x0f );
        txcentre(  98, "SPOT / UP ROUGH", 0x0f );
        menustary = -1;
        mitems = 0;
        break;

      case 4:
        txcentre(  28, "MORPHOS BUILD:", 0x3d );
        txcentre(  42, "ILKKA LEHTORANTA", 0x1b );
        txcentre(  70, "OS X BUILD:", 0x3d );
        txcentre(  84, "GABOR BUKOVICS", 0x1b );
        menustary = -1;
        mitems = 0;
        break;
    }

    tpx     = 0;
    tpxo    = 0;
    tpw     = 320;
    tpy     = 0;
    tpyo    = 0;
    tph     = 140;
    tpscale = 1.0f;
    tpalph  = 255;
    pagerefresh = TRUE;
    return;
  }

  switch( titlepage )
  {
    case 0:
      txcentre(   0, "* STORY TIME *", 0x3d );
      txcentre(  28, "AUGUST, SPACE YEAR 2011", 0x1b );
      txcentre(  42, "WE FIND THE WORLD IN THE", 0x1b );
      txcentre(  56, "GRIP OF APATHY AS AN", 0x1b );
      txcentre(  70, "ARMADA OF ALIEN", 0x1b );
      txcentre(  84, "SPACESHIPS HOVERS ABOVE", 0x1b );
      txcentre(  98, "EARTH'S CAPITAL CITIES..", 0x1b );
      break;

    case 1:
      txcentre(   0, "APATHY? YES INDEED.", 0x1d );
      txcentre(  14, "YOU SEE, SUCH INVASIONS", 0x1d );
      txcentre(  28, "HAD BECOME A REGULAR", 0x1d );
      txcentre(  42, "THING AROUND FRIDAY TEA", 0x1d );
      txcentre(  56, "TIMES, AND AS THE", 0x1d );
      txcentre(  70, "ALIENS WERE ONLY TWO", 0x1d );
      txcentre(  84, "INCHES TALL THEY DIDNT", 0x1d );
      txcentre(  98, "POSE MUCH OF A THREAT..", 0x1d );
      break;

    case 2:
      txcentre(   0, "FORTUNATELY FOR THIS", 0x17 );
      txcentre(  14,"STORY, THIS TIME THINGS", 0x17 );
      txcentre(  28, "WERE DIFFERENT. FROM", 0x17 );
      txcentre(  42, "THESE UFOS CAME DEADLY", 0x17 );
      txcentre(  56, "ROBOT STOMPERS ARMED", 0x17 );
      txcentre(  70, "WITH DARK GLASSES, A", 0x17 );
      txcentre(  84, "GENERAL BAD ATTITUDE", 0x17 );
      txcentre(  98, "AND WIBBLEWAVE RAY-GUNS.", 0x17 );
      break;
    
    case 3:
      txcentre(   0, "THE WORLD WAS CAUGHT", 0x3d );
      txcentre(  14, "ON THE HOP. ONLY ONE", 0x3d );
      txcentre(  28, "MAN COULD SAVE THE DAY,", 0x3d );
      txcentre(  42, "BUT THAT MAN WAS BUSY", 0x3d );
      txcentre(  56, "IRONING HIS PANTS.", 0x3d );
      txcentre(  70, "SO INSTEAD, GIDDY, THE", 0x3d );
      txcentre(  84, "EGG-SHAPED SUPERHERO", 0x3d );
      txcentre(  98, "DECIDED TO HAVE A GO.", 0x3d );
      break;

    case 4:
      txcentre(  0, "* REALITY TIME *", 0x1b );
      txcentre( 28, "WHAT ALL THIS OLD TOSH", 0x1d );
      txcentre( 42, "BOILS DOWN TO IS A 2D", 0x1d );
      txcentre( 56, "PLATFORM / PUZZLE GAME", 0x1d );
      txcentre( 70, "LIKE THOSE OF YORE..", 0x1d );
      txcentre( 98, "BUT WITH A DASH OF IRONY.", 0x3d );
      break;

    case 5:
      txcentre(  0, "USING ONLY THE POWER OF", 0x17 );
      txcentre( 14, "YOUR MIND AND FINGERS", 0x17 );
      txcentre( 28, "YOU MUST SOLVE A LOAD", 0x17 );
      txcentre( 42, "OF BRUTALLY CONTRIVED", 0x17 );
      txcentre( 56, "PUZZLES WHILST JUMPING", 0x17 );
      txcentre( 70, "ABOUT COLLECTING THINGS,", 0x17 );
      txcentre( 84, "AVOIDING BADDIES AND", 0x17 );
      txcentre( 98, "ERR.. BEING A GOOD EGG.", 0x17 );
      break;
  }

  switch( titlepage_in_type )
  {
    case 0:
      tpx     = 0;
      tpxo    = 0;
      tpw     = 320;
      tpy     = 140;
      tpyo    = 0;
      tph     = 0;
      tpscale = 1.0f;
      tpalph  = 255;
      break;

    case 1:
      tpx     = 0;
      tpxo    = 0;
      tpw     = 320;
      tpy     = 0;
      tpyo    = 0;
      tph     = 140;
      tpscale = 0.1f;
      tpalph  = 0;
      break;

    case 2:
      tpx     = 320;
      tpxo    = 0;
      tpw     = 0;
      tpy     = 0;
      tpyo    = 0;
      tph     = 140;
      tpscale = 1.0f;
      tpalph  = 255;
      break;

    case 3:
      tpx     = 0;
      tpxo    = 0;
      tpw     = 320;
      tpy     = 70;
      tpyo    = 0;
      tph     = 0;
      tpscale = 1.0f;
      tpalph  = 255;
      break;
  }

  pagerefresh = TRUE;
}

BOOL animate_titlepage_in( void )
{
  switch( titlepage_in_type )
  {
    case 0:
      if( tph < 140 )
      {
        tph+=2;
        tpy-=2;
        break;
      }
      return TRUE;
    
    case 1:
      if( tpalph < 255 )
      {
        tpalph+=3;
        if( tpalph > 255 )
          tpalph = 255;
      }

      if( tpscale < 1.0f )
      {
        tpscale += 0.9f/80.0f;
        if( tpscale > 1.0f )
          tpscale = 1.0f;
      } else {
        return TRUE;
      }
      break;
    
    case 2:
      if( tpw < 320 )
      {
        tpw += 4;
        tpx -= 4;
        break;
      }
      return TRUE;
    
    case 3:
      if( tph < 140 )
      {
        tph+=2;
        tpy--;
        break;
      }
      return TRUE;
  }
  return FALSE;
}

BOOL animate_titlepage_out( void )
{
  switch( titlepage_out_type )
  {
    case 0:
      if( tph > 0 )
      {
        tpyo+=2;
        tph-=2;
        return FALSE;
      }
      break;
    
    case 1:
      if( tpalph > 0 )
      {
        tpalph-=3;
        if( tpalph < 0 )
          tpalph = 0;
      }

      if( tpscale > 0.1f )
      {
        tpscale -= 0.9f/80.0f;
        return FALSE;
      }
      break;

    case 2:
      if( tpw > 0 )
      {
        tpxo+=4;
        tpw-=4;
        return FALSE;
      }
      break;
    
    case 3:
      if( tph > 0 )
      {
        tpy++;
        tpyo+=2;
        tph-=2;
        return FALSE;
      }
      break;
  }

  titlepage_in_type = (titlepage_in_type+1)%NUM_IN_TYPES;
  titlepage_out_type = (titlepage_out_type+1)%NUM_OUT_TYPES;
  titlepage = (titlepage+1)%NUM_PAGES;
  return TRUE;
}

void render_titlepage( void )
{
  Mtx m, mv;
	GXTexObj texObj;

  if( ( tpw == 0 ) || ( tph == 0 ) )
    return;

  tbindtexture( &texObj, mtexts );

	guMtxIdentity( m );
  guMtxScaleApply( m, m, tpscale, tpscale, 1.0f );
	guMtxTransApply( m, m, 0.0f, 100.0f, 0 );
	guMtxConcat( GXmodelView2D, m, mv );
	GX_LoadPosMtxImm( mv, GX_PNMTX0 );

  GX_Begin( GX_QUADS, GX_VTXFMT0, 4 );
    GX_Position3f32( tpx,     tpy,     0.0f ); GX_Color4u8( 0xff, 0xff, 0xff, 0xff ); GX_TexCoord2f32(       tpxo/512.0f,       tpyo/256.0f );
    GX_Position3f32( tpx+tpw, tpy,     0.0f ); GX_Color4u8( 0xff, 0xff, 0xff, 0xff ); GX_TexCoord2f32( (tpxo+tpw)/512.0f,       tpyo/256.0f );
    GX_Position3f32( tpx+tpw, tpy+tph, 0.0f ); GX_Color4u8( 0xff, 0xff, 0xff, 0xff ); GX_TexCoord2f32( (tpxo+tpw)/512.0f, (tpyo+tph)/256.0f );
    GX_Position3f32( tpx,     tpy+tph, 0.0f ); GX_Color4u8( 0xff, 0xff, 0xff, 0xff ); GX_TexCoord2f32(       tpxo/512.0f, (tpyo+tph)/256.0f );
  GX_End();

  unbindtexture();
}

void reset_titles( void )
{
  int i;
  float a;

  stopallchannels();
  menustary = -1;

  for( i=0; i<MAXTITLESTARS; i++ )
    tstars[i].x = -1;

  a = 0.0f;
  for( i=0; i<6; i++ )
  {
    tita[i] = a;
    tity[i] = -28;
    a -= 0.4f;
  }

  for( i=0; i<12; i++ )
  {
    tfg[i   ] = ((240-8)/12)*i+8;
    tfg[i+12] = ((240-70)/12)*i+70;
    tfg[i+24] = ((240-140)/12)*i+140;
    tfg[i+36] = ((240-210)/12)*i+210;
  }

  fadea = 255;
  fadeadd = -4;
  fadetype = 0;

  load_globalsprites();

  memset( &texts[0], 0, 512*256*4 );
  mtexts = &texts[512*256*4];
}

void animate_tstars( void )
{
  int i;

  for( i=0; i<MAXTITLESTARS; i++ )
  {
    if( tstars[i].x == -1 )
      continue;
    
    tstars[i].x += tstars[i].dx;
    tstars[i].y += tstars[i].dy;
    tstars[i].f++;
    if( tstars[i].f > (5*STARSPEED) )
      tstars[i].x = -1;
  }
}

void stbg_move( void )
{
  stx -= 4;
  if( stx <= -240 ) stx = 0;
  sty -= 6;
  if( sty <= -240 ) sty = 0;
  sta += 1.0f;
  if( sta >= 360.0f ) sta -= 360.0f;

  sstx -= 3;
  if( sstx <= -120 ) sstx = 0;
  ssty -= 4;
  if( ssty <= -120 ) ssty = 0;
  ssta += 0.4f;
  if( ssta >= 360.0f ) ssta -= 360.0f;
}

void title_timing( void )
{
  int i;

  menustaro = (menustaro+1)&7;
  menustara += 3;

  animate_tstars();

  stbg_move();

  switch( titlestate )
  {
    case 1:
      for( i=0; i<6; i++ )
      {
        tita[i] += 0.03f;
        if( tita[i] > 3.14159265f/2.0f )
        {
          tita[i] = 3.14159265f/2.0f;
          if( i == 5 ) titlestate++;
        }
        tity[i] = -80 + sin(tita[i])*176;
      }
      break;
    
    case 2:
      if( strapsw < strapw )
      {
        strapsw+=2;
        for( i=0; i<3; i++ )
        {
          tstars[nexttstar].x = (strapx+strapsw)<<8;
          tstars[nexttstar].y = (strapy+(rand()%14))<<8;
          tstars[nexttstar].dx = ((rand()%3)-4)<<8;
          tstars[nexttstar].dy = ((rand()%18)-9)<<7;
          tstars[nexttstar].f = 0;
          nexttstar = (nexttstar+1)%MAXTITLESTARS;
        }
        break;
      }
      titlestate++;
      break;

    case 3:
      if( ( tity[2] < 64 ) && ( strapy > 76 ) )
      {
        strapy -= 3;
        if( strapy < 76 ) strapy = 76;
      }

      for( i=0; i<6; i++ )
      {
        if( tity[i] > 40 )
        {
          tity[i]-=4;
          if( tity[i] < 40 ) tity[i] = 40;
        } else {
          if( ( i == 5 ) && ( strapy <= 76 ) ) titlestate++;
        }
        
        if( tity[i] > 64 )
          break;
      }
      break;
    
    case 4:
      setup_titlepage();
      titlestate++;
      break;
    
    case 5:
      if( animate_titlepage_in() )
      {
        titlepage_timer = 520;
        titlestate++;
      }
      break;
    
    case 6:
      if( titlepage_timer > 0 )
      {
        titlepage_timer--;
        break;
      }
      titlestate++;
      break;
    
    case 7:
      if( animate_titlepage_out() )
        titlestate = 4;
      break;
    
    case 8:
      break;
  }
}

void render_star( int x, int y, int alpha, float scale, float ang )
{
  Mtx m, mv;
	guMtxIdentity( m );
  guMtxScaleApply( m, m, scale, scale, 1.0f );
	guMtxTransApply( m, m, x, y, 0 );
	guMtxConcat( GXmodelView2D, m, mv );
	GX_LoadPosMtxImm( mv, GX_PNMTX0 );

  GX_Begin( GX_QUADS, GX_VTXFMT0, 4 );
    GX_Position3f32( -53.0f, -50.0f, 0.0f ); GX_Color4u8( 0xff, 0xff, 0xff, alpha ); GX_TexCoord2f32(   0.0f/256.0f,   0.0f/256.0f );
    GX_Position3f32(  53.0f, -50.0f, 0.0f ); GX_Color4u8( 0xff, 0xff, 0xff, alpha ); GX_TexCoord2f32( 106.0f/256.0f,   0.0f/256.0f );
    GX_Position3f32(  53.0f,  50.0f, 0.0f ); GX_Color4u8( 0xff, 0xff, 0xff, alpha ); GX_TexCoord2f32( 106.0f/256.0f, 100.0f/256.0f );
    GX_Position3f32( -53.0f,  50.0f, 0.0f ); GX_Color4u8( 0xff, 0xff, 0xff, alpha ); GX_TexCoord2f32(   0.0f/256.0f, 100.0f/256.0f );
  GX_End();
}

void render_stars( void )
{
  int x, y;

  for( y=ssty/2; y<265; y+=60 )
    for( x=sstx/2; x<348; x+=60 )
      render_star( x, y, 160, 0.5f, sta );

  for( y=sty/2; y<290; y+=120 )
    for( x=stx/2; x<376; x+=120 )
      render_star( x, y, 160, 1.0f, sta );
}

void render_tstars( void )
{
  int i;

  for( i=0; i<MAXTITLESTARS; i++ )
  {
    if( tstars[i].x == -1 )
      continue;

    render_sprite_a( &sprtt[(tstars[i].f/STARSPEED)+5], tstars[i].x>>8, tstars[i].y>>8, FALSE, 0.0f, 128 );
  }
}
    
void render_strapline( float w, int y )
{
  Mtx m, mv;
	GXTexObj texObj;

  if( w < 1.0f )
    return;

	guMtxIdentity( m );
	guMtxTransApply( m, m, strapx, y, 0 );
	guMtxConcat( GXmodelView2D, m, mv );
	GX_LoadPosMtxImm( mv, GX_PNMTX0 );

  tbindtexture( &texObj, mtexts );

  GX_Begin( GX_QUADS, GX_VTXFMT0, 4 );
    GX_Position3f32( 0.0f,  0.0f, 0.0f ); GX_Color4u8( 0xff, 0xff, 0xff, 0xff ); GX_TexCoord2f32( 0.0f/512.0f, 240.0f/256.0f );
    GX_Position3f32(    w,  0.0f, 0.0f ); GX_Color4u8( 0xff, 0xff, 0xff, 0xff ); GX_TexCoord2f32(    w/512.0f, 240.0f/256.0f );
    GX_Position3f32(    w, 14.0f, 0.0f ); GX_Color4u8( 0xff, 0xff, 0xff, 0xff ); GX_TexCoord2f32(    w/512.0f, 254.0f/256.0f );
    GX_Position3f32( 0.0f, 14.0f, 0.0f ); GX_Color4u8( 0xff, 0xff, 0xff, 0xff ); GX_TexCoord2f32( 0.0f/512.0f, 254.0f/256.0f );
  GX_End();

  unbindtexture();
}

void go_menus( void )
{
  int i;

  if( titlestate == 0 ) return;

  what_are_we_doing = WAWD_MENU;
  titlepage = 0;
  menunum = 0;

  for( i=0; i<6; i++ )
  {
    tity[i] = 40;
    tita[i] = 3.14159265f/2.0f;
  }
  strapy = 76;
  strapsw = strapw;

  setup_titlepage();
  titlestate = 8;
}

void menu_up( void )
{
  if( startit ) return;
  if( mitems == 0 ) return;
  if( menunum > 0 )
  {
    menunum--;
    setup_titlepage();
    actionsound( SND_CLICKYCLICK, sfxvol );
  }
}

void menu_down( void )
{
  if( startit ) return;
  if( mitems == 0 ) return;
  if( menunum < (mitems-1) )
  {
    menunum++;
    setup_titlepage();
    actionsound( SND_CLICKYCLICK, sfxvol );
  }
}

int testsamps[] = { SND_JUMP, SND_COIN, SND_SPRING, SND_VANHONK, SND_USE, SND_USEFAIL };

void menu_left( void )
{
  if( startit ) return;
  switch( titlepage )
  {
    case 1:
      switch( menunum )
      {
        case 0:
          if( sfxvolopt > 0 )
          {
            sfxvolopt--;
            sfxvol = (sfxvolopt * MIX_MAX_VOLUME)/8;
            if( sfxvol )
              playsound( 0, testsamps[rand()%6], sfxvol );
            else
              stopallchannels();
            setup_titlepage();
          }
          break;
        
        case 1:
          if( musicvolopt > 0 )
          {
            musicvolopt--;
            musicvol = (musicvolopt * MIX_MAX_VOLUME)/8;
            setmusicvol();
            setup_titlepage();
          }
          break;
      }
      break;
  }
}

void menu_right( void )
{
  if( startit ) return;
  switch( titlepage )
  {
    case 1:
      switch( menunum )
      {
        case 0:
          if( sfxvolopt < 8)
          {
            sfxvolopt++;
            sfxvol = (sfxvolopt * MIX_MAX_VOLUME)/8;
            if( sfxvol )
              playsound( 0, testsamps[rand()%6], sfxvol );
            else
              stopallchannels();
            setup_titlepage();
          }
          break;
        
        case 1:
          if( musicvolopt < 8 )
          {
            musicvolopt++;
            musicvol = (musicvolopt * MIX_MAX_VOLUME)/8;
            setmusicvol();
            setup_titlepage();
          }
          break;
      }
      break;
  }
}

BOOL menu_do( void )
{
  if( startit ) return FALSE;
  switch( titlepage )
  {
    case 0:
      switch( menunum )
      {
        case 0: // Start
          fadea = 0;
          fadeadd = 4;
          fadetype = 0;
          startit = TRUE;
          actionsound( SND_BUBBLEPOP, sfxvol );
          break;
        
        case 1: // Options
          menunum = 0;
          titlepage = 1;
          setup_titlepage();
          actionsound( SND_BUBBLEPOP, sfxvol );
          break;
        
        case 2: // Credits
          titlepage = 2; 
          setup_titlepage();
          actionsound( SND_BUBBLEPOP, sfxvol );
          break;

        case 3: // Quit
          return TRUE;
      }
      break;
    
    case 1:
      switch( menunum )
      {
        case 0: // sfx vol
          break;
        
        case 1: // music vol
          break;
        
        case 2: // back
          actionsound( SND_BUBBLEPOP, sfxvol );
          menunum = 1;
          titlepage = 0;
          setup_titlepage();
          save_options();
          break;
      }
      break;
    
    case 2:
		case 3:
      titlepage++;
      setup_titlepage();
      actionsound( SND_BUBBLEPOP, sfxvol );
      break;

    case 4:
      titlepage = 0;
      setup_titlepage();
      actionsound( SND_BUBBLEPOP, sfxvol );
      break;
  }

  return FALSE;
}

BOOL render_titles( void )
{
  int i;
	GXTexObj texObj;

  if( titlestate == 0 )
  {
    musicenabled = TRUE;
    startit = FALSE;
    reset_titles();
    if( !load_sprites( "/apps/giddy3/hats/titlestuff.bin" ) ) return TRUE;

    strapw = txwidth( straplines[nextstrapline] );
    strapx = (320-strapw)/2;
    strapy = 96+60;
    strapsw = 0;
    txprint( 0, 240, straplines[nextstrapline], 0x31 );
    nextstrapline    = (nextstrapline+1)%8;
    pagerefresh = TRUE;

    playtune( "/apps/giddy3/hats/ttune.mod" );

    titlestate = 1;
    GX_InvVtxCache();
    GX_InvalidateTexAll();
  }

  if( ( startit ) && ( lrfade2 == 255 ) )
  {
    killtune();
    start_game();
    what_are_we_doing = WAWD_GAME;
    fadeadd = -4;
    return FALSE;
  }

  if( pagerefresh )
  {
    texturemangle( 512, 256, texts, mtexts );
  	DCFlushRange( mtexts, 512*256*4 );
    pagerefresh = FALSE;
  }

	GX_LoadPosMtxImm(GXmodelView2D,GX_PNMTX0);
  GX_Begin( GX_QUADS, GX_VTXFMT0, 4 );
    GX_Position3f32(   0.0f,   0.0f, 0.0f ); GX_Color4u8(   6, 187, 203, 255 );
    GX_Position3f32( 320.0f,   0.0f, 0.0f ); GX_Color4u8(   6, 187, 203, 255 );
    GX_Position3f32( 320.0f, 240.0f, 0.0f ); GX_Color4u8(   6, 187, 203, 255 );
    GX_Position3f32(   0.0f, 240.0f, 0.0f ); GX_Color4u8(   6, 187, 203, 255 );
  GX_End();

  bindtexture( &texObj, msprites );
  render_stars();

  for( i=0; i<6; i++ )
    render_sprite_scaled( &sprtt[tits[i]], titx[i], tity[i], FALSE, cos(tita[i])*60.0f, 1.0f+cos(tita[i])*2.0f );

  unbindtexture();

  render_strapline( strapsw, strapy );
  render_titlepage();

  bindtexture( &texObj, msprites );
  render_tstars();
  render_menustars();
  unbindtexture();

  render_tvborders();
  render_fade();

  return FALSE;
}

/******* ENDING STUFF ************/

int endingstate, bgcupframe, cupy;
int endparttimer;
float enz, ena, enzd, enad;
unsigned int gametime;

void start_ending( void )
{
	int i;

	musicenabled = TRUE;

	time( &gameendtime );

	gametime = (unsigned int)difftime( gameendtime, gamestarttime );

  load_globalsprites();

  memset( &texts[0], 0, 512*256*4 );
  mtexts = &texts[512*256*4];

  fadea = 255;
  fadeadd = -4;
  fadetype = 0;

	endingstate = 0;
	bgcupframe = 0;
	cupy = 287;

  for( i=0; i<12; i++ )
  {
    tfg[i   ] = ((240-8)/12)*i+8;
    tfg[i+12] = ((240-70)/12)*i+70;
    tfg[i+24] = ((240-140)/12)*i+140;
    tfg[i+36] = ((240-210)/12)*i+210;
  }

  playtune( "/apps/giddy3/hats/ctune.mod" );

  GX_InvVtxCache();
  GX_InvalidateTexAll();
}

void render_bgcups( void )
{
  int x, y, f;

	f = (bgcupframe>>2)&7;

  for( y=ssty/2; y<265; y+=60 )
    for( x=sstx/2; x<348; x+=60 )
		  render_sprite_scaleda( &sprtv[f], x, y, FALSE, 0.0f, 0.5f, 80 );

  for( y=sty/2; y<290; y+=120 )
    for( x=stx/2; x<376; x+=120 )
		  render_sprite_scaleda( &sprtv[f], x, y, FALSE, 0.0f, 1.0f, 80 );

	render_sprite_scaleda( &sprtv[f], 160, cupy, FALSE, 0.0f, 1.0f, 255 );
}

void ending_timing( void )
{
	char tmp[64];
  stbg_move();
	bgcupframe++;

	switch( endingstate )
	{
		case 1:
      memset( &texts[0], 0, 512*256*4 );
			txcentre(  14+14, "** CONGRATULATIONS **", 0x3d );
			txcentre(  42+14, "YOU'VE SAVED THE WORLD", 0x17 );
			txcentre(  56+14, "AND IT ONLY TOOK YOU:", 0x17 );
			sprintf( tmp, "%u MINS AND %02u SECONDS", gametime/60, gametime%60 );
			txcentre(  84+14, tmp, 0x17 );
			txcentre( 112+14, "ADDITIONALLY, YOU MADE A", 0x0c );
			txcentre( 126+14, "HEALTHY PROFIT OF:", 0x0c );
			sprintf( tmp, "%d COINS", gid.coins );
			txcentre( 154+14, tmp, 0x0c );
			txcentre( 182+14, "SPEND IT WISELY NOW!", 0x0c );
			pagerefresh = 1;
			enz = 0.01f;
			ena = 360.0f;
			
			enzd = (1.0f-enz) / 60.0f;
			enad = (0.0f-ena) / 60.0f;

			endparttimer = 240;
			endingstate++;
			break;
		
		case 2:
	  case 6:
			enz += enzd;
		  ena += enad;

			if( enz >= 1.0f )
		  {
				enz = 1.0f;
        ena = 0.0f;
				endingstate++;
		  }
			break;
	  
		case 3:
		case 8:
			if( endparttimer > 0 )
		  {
			  endparttimer--;
				break;
			}

			endingstate++;
			break;

    case 4:
			enz -= enzd;
		  ena += enad;
			if( enz <= 0.01f )
		  {
				enz = 0.01f;
				ena = 0.0f;
				endingstate++;
			}
			break;

    case 5:
      memset( &texts[0], 0, 512*256*4 );
		  txcentre( 14, "YOU ARE HEREBY AWARDED THE", 0x31 );
			txcentre( 28, "GOLDEN EGG OF SPLENDIDNESS:", 0x31 );
			pagerefresh = 1;
			enz = 0.01f;
			ena = 360.0f;
			
			enzd = (1.0f-enz) / 60.0f;
			enad = (0.0f-ena) / 60.0f;

			endparttimer = 900;
			endingstate++;
      break;

    case 7:
    	if( cupy > 140 )
  		{
    		cupy--;
				break;
			}

      endingstate++;
		  break;

    case 9:
   		fadea = 0;
	    fadeadd = 4;
	    fadetype = 0;
			endingstate++;
			break;
	  
		case 10:
			enz -= enzd;
		  ena += enad;
			if( enz <= 0.01f )
		  {
				enz = 0.01f;
				ena = 0.0f;
			}

			if( lrfade2 < 255 )
				break;

		  stopmusic();

		  titlestate = 0;
			what_are_we_doing = WAWD_TITLES;
			break;

		case 11:
			fadea = 0;
		  fadeadd = 4;
			fadetype = 0;
			endingstate++;
			break;

    case 12:
			if( lrfade2 < 255 )
			  break;

		  stopmusic();

		  titlestate = 0;
			what_are_we_doing = WAWD_TITLES;
			break;
  }
}

BOOL render_ending( void )
{
	Mtx m,m1,m2, mv;
	GXTexObj texObj;

	if( endingstate == 0 )
  {
    if( !load_sprites( "/apps/giddy3/hats/victorystuff.bin" ) ) return TRUE;
		start_ending();
    endingstate = 1;
		return FALSE;
  }

  if( pagerefresh )
  {
    texturemangle( 512, 256, texts, mtexts );
  	DCFlushRange( mtexts, 512*256*4 );
    pagerefresh = FALSE;
  }

	GX_LoadPosMtxImm(GXmodelView2D,GX_PNMTX0);
  GX_Begin( GX_QUADS, GX_VTXFMT0, 4 );
    GX_Position3f32(   0.0f,   0.0f, 0.0f ); GX_Color4u8(   6, 187, 203, 255 );
    GX_Position3f32( 320.0f,   0.0f, 0.0f ); GX_Color4u8(   6, 187, 203, 255 );
    GX_Position3f32( 320.0f, 240.0f, 0.0f ); GX_Color4u8(   6, 187, 203, 255 );
    GX_Position3f32(   0.0f, 240.0f, 0.0f ); GX_Color4u8(   6, 187, 203, 255 );
  GX_End();

  bindtexture( &texObj, msprites );
  render_bgcups();
	unbindtexture();

  tbindtexture( &texObj, mtexts );

	guMtxIdentity( m1 );
  guMtxScaleApply( m1, m1, enz, enz, 1.0f );
	guMtxRotAxisDeg( m2, &axis, ena );
	guMtxConcat( m2, m1, m );
	guMtxTransApply( m, m, 160.0f, 120.0f, 0 );
	guMtxConcat( GXmodelView2D, m, mv );
	GX_LoadPosMtxImm( mv, GX_PNMTX0 );

  GX_Begin( GX_QUADS, GX_VTXFMT0, 4 );
    GX_Position3f32( -160.0f, -120.0f, 0.0f ); GX_Color4u8( 0xff, 0xff, 0xff, 0xff ); GX_TexCoord2f32(   0.0f/512.0f,   0.0f/256.0f );
    GX_Position3f32(  160.0f, -120.0f, 0.0f ); GX_Color4u8( 0xff, 0xff, 0xff, 0xff ); GX_TexCoord2f32( 320.0f/512.0f,   0.0f/256.0f );
    GX_Position3f32(  160.0f,  120.0f, 0.0f ); GX_Color4u8( 0xff, 0xff, 0xff, 0xff ); GX_TexCoord2f32( 320.0f/512.0f, 240.0f/256.0f );
    GX_Position3f32( -160.0f,  120.0f, 0.0f ); GX_Color4u8( 0xff, 0xff, 0xff, 0xff ); GX_TexCoord2f32(   0.0f/512.0f, 240.0f/256.0f );
  GX_End();

  unbindtexture();

	render_tvborders();
	render_fade();

	return FALSE;
}
