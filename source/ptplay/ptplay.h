
#ifndef _PTREPLAY_H
#define _PTREPLAY_H

/*
**	protracker 2.3a player
**	(C) 2001, 2003 Ronald Hof and Timm S. Mueller
*/

#ifdef	__VBCC__
#define M_PI				3.14159265358979323846  /* pi */
#endif

#define PT_MOD_UNSUPPORTED		-1
#define PT_MOD_UNKNOWN			0
#define PT_MOD_PROTRACKER		1
#define PT_MOD_SOUNDTRACKER	2
#define PT_MOD_SOUNDFX			3

#define MODF_DOSONGEND		0x0001		/* perform song-end detection */
#define MODF_ALLOWPANNING	0x0002		/* allow mod to use command $8 for panning */
#define MODF_ALLOWFILTER	0x0004		/* allow mod to set lowpass filter */
#define MODF_SONGEND			0x0008		/* songend occured */

typedef struct
{
	u8	name[32];
	s8 *data;
	s32 length, repeat, replen, repend;		/* <<14, includes oversampling precision */
	s32 ft;
	s32 volume;
	s32 pad;
}pt_sample_s;	/* 32 bytes */


/* -> u8 <- cutoff, loopcount, retrig, delay	*/

typedef struct
{
	s32 sample, prevsample, sp, per, freq, oldsp;
	s32 dp, basevol, prevvol, vol, dvol, pan, ft;
	u32 flags;
	s32 dtp, tptarget, tpsrc;
	s32 retrig, cutoff, delay;
	s32 arp, arpbase;
	s32 vibspeed, vibdepth, avibspeed, avibindex;
	s32 trespeed, tredepth, atrespeed, atreindex;
	s32 vibwave, tremwave, glissando, funkoffset, funkcnt;
	s32 delaysample;
	s32 loopstart, loopcount, loopflg;
} pt_channel_s;

typedef struct
{
	s32 period, sample;
	u8 effect, efd1, efd2, efboth;
//	s32 pad;
} pt_patterndata_s;	/* 16 bytes */

typedef struct
{
	pt_patterndata_s data[64][4];
}pt_pattern_s;

typedef struct
{
	char	name[23];
	u8	modformat;
	u16	numpat, length;
	pt_sample_s		sample[32];
	pt_channel_s	chan[4];
	pt_pattern_s	*pattern;
	u8	pos[128];
	u32 flags;
	s32	freq;
	s32	mastervolume;
	u8	spos, ppos, speed, ciaspeed;
	s32	vbllen, bpos;
	u8	tick, patdelay, skiptopos, filter;
	u32	songloopcount;
	f32 flta, fltb;
	f32 fltpi[4];
	f32 fltpo[4];
	u32	PlayLength;	// in secs
}pt_mod_s;


#define	PTV_TEST_FREQUENCY	50

pt_mod_s *PtInit(u8 *buf, u32 bufsize, u32 freq );
void PtRender( pt_mod_s *mod, s8 * buf, s8 * buf2, s32 bufmodulo, s32 numsmp, s32 scale, s32 depth, s32 channels);

#endif
