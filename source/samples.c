/*
** Sample data
*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <malloc.h>
#include <math.h>

#include <gccore.h>
#include "ptplay/ptplay.h"

#include "giddy3.h"
#include "samples.h"

extern BOOL audioavailable;
extern BOOL musicenabled;
extern int what_are_we_doing;

#define AUDIO_BUFFERS 3
#define FREQ 48000
#define SNDBUFFERLEN ((FREQ/50)*2*2*2)

struct sample
{
  s32 len;
  s16 *data;
};

struct sndchan
{
  s16 *sample;
  s32 len, pos, delta, volume;
  s32 loops;
};

s32 chanmix[SNDBUFFERLEN/4];

int mixitplease[AUDIO_BUFFERS], nextmixit, lastplayed=-1;
static vu32 curr_audio = 0;
static u8 audioBuf[AUDIO_BUFFERS][SNDBUFFERLEN] ATTRIBUTE_ALIGN(32);
BOOL tunePlaying = FALSE;
pt_mod_s *tune = NULL;

struct sndchan sfxchan[C_LAST];
struct sample smp[SND_LAST];

int musicvol = (MIX_MAX_VOLUME*6)/8, musicvolopt = 6;
int sfxvol   = MIX_MAX_VOLUME, sfxvolopt = 8;

static void mixcallback(void *usrdata,u8 *stream,u32 length)
{
  int i, j;
  s16 *out;
  s32 v;

  out = (s16 *)stream;

	if( ( tune ) && ( tunePlaying ) )
    PtRender( tune, (s8 *)&stream[0], (s8 *)&stream[2], 4, length/4, 2, 16, 2 );
  else
    memset( stream, 0, length );

  memset( chanmix, 0, SNDBUFFERLEN );
  for( i=0; i<C_LAST; i++ )
  {
    if( ( sfxchan[i].sample ) && ( sfxchan[i].delta ) )
    {
      for( j=0; j<length/4; j++ )
      {
        if( (sfxchan[i].pos>>8) >= sfxchan[i].len )
        {
          if( sfxchan[i].loops != 0 )
          {
            sfxchan[i].pos = 0;
            if( sfxchan[i].loops > 0 )
              sfxchan[i].loops--;
          } else {
            sfxchan[i].sample = NULL;
            sfxchan[i].delta = 0;
            break;
          }
        }
           
        v = sfxchan[i].sample[sfxchan[i].pos>>8] * sfxchan[i].volume;
        chanmix[j] += v;

        sfxchan[i].pos += sfxchan[i].delta;
      }
    }
  }

  for( i=0, j=0; i<length/4; i++ )
  {
    chanmix[i] >>= 8;
    v = (out[j] + chanmix[i])>>1;
    if( v > 32767 ) v = 32767;
    if( v < -32768 ) v = -32768;
    out[j++] = v;
    v = (out[j] + chanmix[i])>>1;
    if( v > 32767 ) v = 32767;
    if( v < -32768 ) v = -32768;
    out[j++] = v;
  }

	DCFlushRange(stream,length);
}

static void dmaCallback()
{
	AUDIO_StopDMA();
	AUDIO_InitDMA( (u32)audioBuf[curr_audio], SNDBUFFERLEN );
	DCFlushRange( audioBuf[curr_audio], SNDBUFFERLEN );
	AUDIO_StartDMA();

  if( mixitplease[curr_audio] )
    return;

  if( lastplayed != -1 )
    mixitplease[lastplayed] = 1;
  lastplayed = curr_audio;
	curr_audio = (curr_audio+1)%AUDIO_BUFFERS;
}

void initsounds( void )
{
  int i;

  for( i=0; i<C_LAST; i++ )
  {
    sfxchan[i].sample = NULL;
    sfxchan[i].delta = 0;
  }

  for( i=0; i<SND_LAST; i++ )
  {
    smp[i].data = NULL;
    smp[i].len = 0;
  }

	AUDIO_Init( NULL );
  audioavailable = TRUE;

	tunePlaying = FALSE;

	AUDIO_SetDSPSampleRate( AI_SAMPLERATE_48KHZ );
	for( i=0; i<AUDIO_BUFFERS; i++ )
		mixitplease[i] = 0;
	nextmixit = 0;
}

void stopmusic( void )
{
	if(!tunePlaying) return;

	AUDIO_StopDMA();
	AUDIO_RegisterDMACallback(NULL);

	curr_audio = 0;
	tunePlaying = FALSE;
}

void mixaudio( void )
{
  while( mixitplease[nextmixit] )
  {
    mixitplease[nextmixit] = 0;
    mixcallback( NULL, (u8*)audioBuf[nextmixit], SNDBUFFERLEN );
    nextmixit = (nextmixit+1)%AUDIO_BUFFERS;
  }

  if( !tune ) return;

  if( ( musicenabled ) || ( what_are_we_doing != WAWD_GAME ) )
  {
    if( tune->mastervolume < musicvol )
    {
      tune->mastervolume += 3;
      if( tune->mastervolume > musicvol )
        tune->mastervolume = musicvol;
    }
  } else {
    if( tune->mastervolume > 0 )
    {
      if( tune->mastervolume < 3 )
        tune->mastervolume = 0;
      else
        tune->mastervolume -= 3;
    }
  }
}

void startmusic( void )
{
	int i;

	if( tunePlaying ) return;

	for( i=0; i<AUDIO_BUFFERS; i++ )
		mixitplease[i] = 1;
	nextmixit = 0;

  mixaudio();

	curr_audio = 0;
  lastplayed = -1;
	tunePlaying = TRUE;

	AUDIO_RegisterDMACallback( dmaCallback );
	AUDIO_InitDMA((u32)audioBuf[curr_audio], SNDBUFFERLEN );
	AUDIO_StartDMA();
  lastplayed = curr_audio;
	curr_audio++;
}

void killtune( void )
{
	if( tunePlaying ) stopmusic();
	if( tune )
	{
    free( tune );
		tune = NULL;
	}
}

void playtune( char *fname )
{
  u8 *buf;
  int buflen;
  FILE *f;

  if( !audioavailable ) return;

  killtune();

  f = fopen( fname, "rb" );
  if( !f ) return;

  fseek( f, 0, SEEK_END);
  buflen = ftell(f);
  fseek( f, 0, SEEK_SET );

  buf = malloc( buflen );
  if( !buf )
  {
    fclose( f );
    return;
  }

  fread( buf, buflen, 1, f );
  fclose( f );

  tune = PtInit( buf, buflen, 48000 );
  if( !tune ) return;

  tune->mastervolume = musicvol;

	startmusic();
}

void setmusicvol( void )
{
  if( !tune ) return;
  tune->mastervolume = musicvol;
}

BOOL loadsounds( void )
{
  int i;
  FILE *f;
  char mkstr[64];
  s32 len;

  if( !audioavailable ) return TRUE;

  for( i=0; i<SND_LAST; i++ )
  {
    sprintf( mkstr, "/apps/giddy3/hats/fx%02x.raw", i+1 );
    f = fopen( mkstr, "rb" );
    fseek( f, 0, SEEK_END );
    len = ftell( f );
    fseek( f, 0, SEEK_SET );
    if( len < 2 )
    {
      fclose( f );
      continue;
    }
    smp[i].data = malloc( len );
    if( !smp[i].data )
    {
      fclose( f );
      return FALSE;
    }
    fread( smp[i].data, len, 1, f );
    fclose( f );
    smp[i].len = len/2;
  }

  return TRUE;
}

void freesounds( void )
{
  int i;

  if( !audioavailable ) return;

  for( i=0; i<SND_LAST; i++ )
  {
    if( smp[i].data ) free(smp[i].data);
    smp[i].data = NULL;
    smp[i].len = 0;
  }
}

int lastsound=-1;

void playsound( int chan, int sound, int volume )
{
  lastsound = sound;
  if( !audioavailable ) return;
  if( volume < 1 ) return;
  if( volume > 256 ) volume = 256;

  sfxchan[chan].sample = smp[sound].data;
  sfxchan[chan].len    = smp[sound].len;
  sfxchan[chan].pos    = 0;
  sfxchan[chan].delta  = (22050<<8)/48000;
  sfxchan[chan].loops  = 0;
  sfxchan[chan].volume = volume;
}

void loopsound( int chan, int sound, int volume )
{
  lastsound = sound;
  if( !audioavailable ) return;
  if( volume < 1 )
    return;
  if( volume > 256 ) volume = 256;

  sfxchan[chan].sample = smp[sound].data;
  sfxchan[chan].len    = smp[sound].len;
  sfxchan[chan].pos    = 0;
  sfxchan[chan].delta  = (22050<<8)/48000;
  sfxchan[chan].loops  = -1;
  sfxchan[chan].volume = volume;
}

void nloopsound( int chan, int sound, int volume, int loops )
{
  lastsound = sound;
  if( !audioavailable ) return;
  if( volume < 1 )
    return;
  if( volume > 256 ) volume = 256;

  sfxchan[chan].sample = smp[sound].data;
  sfxchan[chan].len    = smp[sound].len;
  sfxchan[chan].pos    = 0;
  sfxchan[chan].delta  = (22050<<8)/48000;
  sfxchan[chan].loops  = loops;
  sfxchan[chan].volume = volume;
}

static int amsc = 0;
static int *amchp[2] = { NULL, NULL };
void ambientloop( int sound, int volume, int *chp )
{
  amsc = (amsc+1)%((C_AMBIENTLAST-C_AMBIENT1)+1);
  if( amchp[amsc] )
    *amchp[amsc] = -1;
  amchp[amsc] = chp;
  if( chp )
    *chp = C_AMBIENT1+amsc;
  loopsound( C_AMBIENT1+amsc, sound, volume );
}

void stopchannel( int chan )
{
  int i;

  if( !audioavailable ) return;

  for( i=C_AMBIENT1; i<=C_AMBIENTLAST; i++ )
    if( ( chan == i ) && ( amchp[i-C_AMBIENT1] ) ) { *amchp[i-C_AMBIENT1] = -1; amchp[i-C_AMBIENT1] = NULL; }

  sfxchan[chan].sample = NULL;
  sfxchan[chan].delta = 0;
}

void stopallchannels( void )
{
  int i;

  for( i=0; i<C_LAST; i++ )
    stopchannel( i );
}

void setvol( int chan, int volume )
{
  sfxchan[chan].volume = volume;
}

static int esc = 0;
void enemysound( int sound, int volume )
{
  esc = (esc+1)%((C_ENEMYLAST-C_ENEMY1)+1);
  playsound( C_ENEMY1+esc, sound, volume );
}

static int asc = 0;
void actionsound( int sound, int volume )
{
  asc = (asc+1)%((C_ACTIONLAST-C_ACTION1)+1);
  playsound( C_ACTION1+asc, sound, volume );
}

void lpactionsound( int sound, int volume )
{
  int j;
  for( j=0; j<=(C_ACTIONLAST-C_ACTION1); j++ )
  {
    asc = (asc+1)%((C_ACTIONLAST-C_ACTION1)+1);
    if( sfxchan[C_ACTION1+asc].sample == NULL )
      break;
  }

  if( j > (C_ACTIONLAST-C_ACTION1) ) return;
  playsound( C_ACTION1+asc, sound, volume );
}

static int isc = 0;
void incidentalsound( int sound, int volume )
{
  isc = (isc+1)%((C_INCIDENTALLAST-C_INCIDENTAL1)+1);
  playsound( C_INCIDENTAL1+isc, sound, volume );
}

int incidentalloop( int sound, int volume )
{
  isc = (isc+1)%((C_INCIDENTALLAST-C_INCIDENTAL1)+1);
  loopsound( C_INCIDENTAL1+isc, sound, volume );
  return C_INCIDENTAL1+isc;
}

int incidentalloops( int sound, int volume, int loops )
{
  isc = (isc+1)%((C_INCIDENTALLAST-C_INCIDENTAL1)+1);
  nloopsound( C_INCIDENTAL1+isc, sound, volume, loops );
  return C_INCIDENTAL1+isc;
}
