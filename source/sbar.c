/*
Copyright (C) 1996-1997 Id Software, Inc.

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

*/
// sbar.c -- status bar code

#include "quakedef.h"

extern void M_Print (int cx, int cy, char *str);

extern cvar_t scr_sbaralpha;
extern cvar_t scr_sbarscale;
extern cvar_t viewsize;

int			sb_updates;		// if >= vid.numpages, no update needed

cvar_t cl_killmedals = {"cl_killmedals", "0"};
cvar_t cl_plasmanade = {"cl_plasmanade", "2"};
cvar_t cl_nadenum = {"cl_nadenum", "5"};
cvar_t cl_round = {"cl_round", "0"};
cvar_t cl_life = {"cl_life", "1"};
cvar_t cl_slowmo = {"cl_slowmo", "5"};
cvar_t cl_respawn = {"cl_respawn", "0"};
cvar_t cl_activate = {"cl_activate", "0"};	//For slowmo
cvar_t cl_scope = {"cl_scope", "0"};
cvar_t cl_ww = {"cl_ww", "0"};		//World weapon print message
cvar_t cl_ch_red = {"cl_ch_red", "0"};
cvar_t cl_ch_blue = {"cl_ch_blue", "1"};
cvar_t cl_overlay = {"cl_overlay", "0"};
cvar_t hide_hud = {"hide_hud", "0"};

#define STAT_MINUS		10	// num frame for '-' stats digit
qpic_t		*sb_nums[2][11];
qpic_t		*sb_colon, *sb_slash;
qpic_t		*sb_ibar;
qpic_t		*sb_sbar;
qpic_t		*sb_scorebar;

qpic_t      *sb_weapons[7][8];   // 0 is active, 1 is owned, 2-5 are flashes
qpic_t      *sb_ammo[4];
qpic_t		*sb_sigil[4];
qpic_t		*sb_armor[3];
qpic_t		*sb_items[32];

qpic_t	*sb_faces[7][2];		// 0 is gibbed, 1 is dead, 2-6 are alive
							// 0 is static, 1 is temporary animation
qpic_t	*sb_face_invis;
qpic_t	*sb_face_quad;
qpic_t	*sb_face_invuln;
qpic_t	*sb_face_invis_invuln;

bool	sb_showscores;

int			sb_lines;			// scan lines to draw

qpic_t      *rsb_invbar[2];
qpic_t      *rsb_weapons[5];
qpic_t      *rsb_items[2];
qpic_t      *rsb_ammo[3];
qpic_t      *rsb_teambord;		// PGM 01/19/97 - team color border

//Weapon Icons
qpic_t		*AR;
qpic_t		*SMG;
qpic_t		*Sniper;
qpic_t		*RL;
qpic_t		*Needler;
qpic_t		*ppist;
qpic_t		*Pistol;
qpic_t		*Shottie;
//World weapons
qpic_t		*ARW;
qpic_t		*SMGW;
qpic_t		*SniperW;
qpic_t		*RLW;
qpic_t		*NeedlerW;
qpic_t		*ppistW;
qpic_t		*PistolW;
qpic_t		*ShottieW;

//Kill medals
qpic_t		*doublek;
qpic_t		*triple;
qpic_t		*killtrocity;
qpic_t		*killtacular;

//Grenade types
qpic_t		*plasma;
qpic_t		*frag;


//Health
qpic_t		*life90;
qpic_t		*lifered;
qpic_t		*health_bar_outline_blue;
qpic_t		*health_bar_outline_red;

//Ammo counter
qpic_t		*pistolb;
qpic_t		*shotgunb;
qpic_t		*arb;
qpic_t		*needleb;
qpic_t		*rocketb;
qpic_t		*smgb;
qpic_t		*sniperb;

//Slowmo graphic
qpic_t		*slomoclock[9];

//Grenade Numbers
qpic_t		*numbers[5];

//Crosshairs

//red
qpic_t		*arred;
qpic_t		*smgred;
qpic_t		*sniperred;
qpic_t		*rlred;
qpic_t		*pistolred;
qpic_t		*ppistred;
qpic_t		*needlerred;
qpic_t		*plriflered;
qpic_t		*shotgunred;
qpic_t		*swordred;

//blue
qpic_t		*arblue;
qpic_t		*smgblue;
qpic_t		*sniperblue;
qpic_t		*rlblue;
qpic_t		*pistolblue;
qpic_t		*ppistblue;
qpic_t		*needlerblue;
qpic_t		*plrifleblue;
qpic_t		*shotgunblue;
qpic_t		*swordblue;

qpic_t		*plasmabar;


//MED 01/04/97 added two more weapons + 3 alternates for grenade launcher
qpic_t      *hsb_weapons[7][5];   // 0 is active, 1 is owned, 2-5 are flashes
//MED 01/04/97 added array to simplify weapon parsing
int         hipweapons[4] = {HIT_LASER_CANNON_BIT,HIT_MJOLNIR_BIT,4,HIT_PROXIMITY_GUN_BIT};
//MED 01/04/97 added hipnotic items array
qpic_t      *hsb_items[2];

void Sbar_MiniDeathmatchOverlay (void);
void Sbar_DeathmatchOverlay (void);
void M_DrawPic (int x, int y, qpic_t *pic);

int cach;
int drawhealth_kicked;
int drawslowmo_kicked;
int drawww_kicked;
int drawnade_kicked;
int crosshairs_kicked;
int drawammo_kicked;
int drawmedal_kicked;
int drawweapon_kicked;

/*
===============
Sbar_ShowScores

Tab key down
===============
*/
void Sbar_ShowScores (void)
{
	if (sb_showscores)
		return;
	sb_showscores = true;
	sb_updates = 0;
}

/*
===============
Sbar_DontShowScores

Tab key up
===============
*/
void Sbar_DontShowScores (void)
{
	sb_showscores = false;
	sb_updates = 0;
}

/*
===============
Sbar_Changed
===============
*/
void Sbar_Changed (void)
{
	sb_updates = 0;	// update next frame
}

/*
===============
Sbar_Init
===============
*/
void Sbar_Init (void)
{
	int		i;

	cach = 0;
	drawhealth_kicked = 0;
	drawslowmo_kicked = 0;
	drawww_kicked = 0;
	drawnade_kicked = 0;
	crosshairs_kicked = 0;
	drawammo_kicked = 0;
	drawmedal_kicked = 0;
	drawweapon_kicked = 0;
	for (i=0 ; i<10 ; i++)
	{
		sb_nums[0][i] = Draw_PicFromWad (va("num_%i",i));
		sb_nums[1][i] = Draw_PicFromWad (va("anum_%i",i));
	}

	sb_nums[0][10] = Draw_PicFromWad ("num_minus");
	sb_nums[1][10] = Draw_PicFromWad ("anum_minus");

	sb_colon = Draw_PicFromWad ("num_colon");
	sb_slash = Draw_PicFromWad ("num_slash");
	/*
	=================================================
	Weapon Icons for weapons.
	=================================================
	*/

/*
		//Ammo counter
		pistolb = Draw_CachePic ("gfx/ammo/pistolb.lmp");	//Pistolb
		shotgunb = Draw_CachePic ("gfx/ammo/shotgunb.lmp");	//shotgunb
		arb = Draw_CachePic ("gfx/ammo/arb.lmp");		//arb
		needleb = Draw_CachePic ("gfx/ammo/needleb.lmp");		//needleb
		rocketb = Draw_CachePic ("gfx/ammo/rocketb.lmp");		//rocketb
		smgb = Draw_CachePic ("gfx/ammo/arb.lmp");		//SMGb
		sniperb = Draw_CachePic ("gfx/ammo/sniperb.lmp");		//Sniperb
		*/

/*
		//World Weapons
		ARW = Draw_CachePic ("gfx/worldweapons/ar.lmp");	//Assault rifle
		SMGW = Draw_CachePic ("gfx/worldweapons/smg.lmp");	//SMG
		SniperW = Draw_CachePic ("gfx/worldweapons/sniper.lmp");	//Sniper Rifle
		RLW = Draw_CachePic ("gfx/worldweapons/RL.lmp");	//Rocket Launcher
		NeedlerW = Draw_CachePic ("gfx/worldweapons/Needler.lmp");	//Needler
		PistolW = Draw_CachePic ("gfx/worldweapons/Pistol.lmp");	//Pistol
		ShottieW = Draw_CachePic ("gfx/worldweapons/Shottie.lmp");	//Shottie
		ppistW = Draw_CachePic ("gfx/worldweapons/ppist.lmp");	//Plasma Pistol
*/
/*
		//Weapons
		AR = Draw_CachePic ("gfx/weapons/ar.lmp");	//Assault rifle
		SMG = Draw_CachePic ("gfx/weapons/smg.lmp");	//SMG
		Sniper = Draw_CachePic ("gfx/weapons/sniper.lmp");	//Sniper Rifle
		RL = Draw_CachePic ("gfx/weapons/RL.lmp");	//Rocket Launcher
		Needler = Draw_CachePic ("gfx/weapons/Needler.lmp");	//Needler
		Pistol = Draw_CachePic ("gfx/weapons/Pistol.lmp");	//Pistol
		Shottie = Draw_CachePic ("gfx/weapons/Shottie.lmp");	//Shottie
		ppist = Draw_CachePic ("gfx/weapons/ppist.lmp");	//Plasma Pistol
		*/


		//Health
		//life90 = Draw_CachePic ("gfx/healthb.lmp");
		//health_bar = Draw_CachePic ("gfx/healthback.lmp");
		//health_bar_red = Draw_CachePic ("gfx/healthbackr.lmp");

		//Kill medals
		//doublek = Draw_CachePic ("gfx/medals/double.lmp");
		//triple = Draw_CachePic ("gfx/medals/triple.lmp");
		//killtacular = Draw_CachePic ("gfx/medals/killtacular.lmp");
		//killtrocity = Draw_CachePic ("gfx/medals/killtrocity.lmp");


		//Nade types
		//plasma = Draw_CachePic ("gfx/plasmanade.lmp");
		//frag = Draw_CachePic ("gfx/fraggrenade.lmp");


		//Nade number counter
	//	numbers[0] = Draw_CachePic ("gfx/numbers/num_0.lmp");
	//	numbers[1] = Draw_CachePic ("gfx/numbers/num_1.lmp");
	//	numbers[2] = Draw_CachePic ("gfx/numbers/num_2.lmp");
	//	numbers[3] = Draw_CachePic ("gfx/numbers/num_3.lmp");
	//	numbers[4] = Draw_CachePic ("gfx/numbers/num_4.lmp");

/*
		//Clock counter
		slomoclock[0] = Draw_CachePic ("gfx/clock/clock1.lmp");
		slomoclock[1] = Draw_CachePic ("gfx/clock/clock2.lmp");
		slomoclock[2] = Draw_CachePic ("gfx/clock/clock3.lmp");
		slomoclock[3] = Draw_CachePic ("gfx/clock/clock4.lmp");
		slomoclock[4] = Draw_CachePic ("gfx/clock/clock5.lmp");
		slomoclock[5] = Draw_CachePic ("gfx/clock/clock6.lmp");
		slomoclock[6] = Draw_CachePic ("gfx/clock/clock7.lmp");
		slomoclock[7] = Draw_CachePic ("gfx/clock/clock8.lmp");
		slomoclock[8] = Draw_CachePic ("gfx/clock/clock9.lmp");
		*/

		//Client variables
		Cvar_RegisterVariable (&cl_killmedals);	//Kill medals
		Cvar_RegisterVariable (&cl_plasmanade); //Grenade types
		Cvar_RegisterVariable (&cl_nadenum);	//A counter for your grenades.
		Cvar_RegisterVariable (&cl_ww);	//World weapons
		Cvar_RegisterVariable (&cl_round);
		Cvar_RegisterVariable (&cl_life);
		Cvar_RegisterVariable (&cl_slowmo);
		Cvar_RegisterVariable (&cl_activate);
		Cvar_RegisterVariable (&cl_scope);
		Cvar_RegisterVariable (&cl_respawn);
		Cvar_RegisterVariable (&cl_ch_red);
		Cvar_RegisterVariable (&cl_ch_blue);
		Cvar_RegisterVariable (&cl_overlay);
		Cvar_RegisterVariable (&hide_hud);


		Cmd_AddCommand ("+showscores", Sbar_ShowScores);
		Cmd_AddCommand ("-showscores", Sbar_DontShowScores);

		sb_sbar = Draw_PicFromWad ("sbar");
		sb_ibar = Draw_PicFromWad ("ibar");
		sb_scorebar = Draw_PicFromWad ("scorebar");

	//MED 01/04/97 added new hipnotic weapons
		if (hipnotic)
		{
		  hsb_weapons[0][0] = Draw_PicFromWad ("inv_laser");
		  hsb_weapons[0][1] = Draw_PicFromWad ("inv_mjolnir");
		  hsb_weapons[0][2] = Draw_PicFromWad ("inv_gren_prox");
		  hsb_weapons[0][3] = Draw_PicFromWad ("inv_prox_gren");
		  hsb_weapons[0][4] = Draw_PicFromWad ("inv_prox");

		  hsb_weapons[1][0] = Draw_PicFromWad ("inv2_laser");
		  hsb_weapons[1][1] = Draw_PicFromWad ("inv2_mjolnir");
		  hsb_weapons[1][2] = Draw_PicFromWad ("inv2_gren_prox");
		  hsb_weapons[1][3] = Draw_PicFromWad ("inv2_prox_gren");
		  hsb_weapons[1][4] = Draw_PicFromWad ("inv2_prox");

		  for (i=0 ; i<5 ; i++)
		  {
			 hsb_weapons[2+i][0] = Draw_PicFromWad (va("inva%i_laser",i+1));
			 hsb_weapons[2+i][1] = Draw_PicFromWad (va("inva%i_mjolnir",i+1));
			 hsb_weapons[2+i][2] = Draw_PicFromWad (va("inva%i_gren_prox",i+1));
			 hsb_weapons[2+i][3] = Draw_PicFromWad (va("inva%i_prox_gren",i+1));
			 hsb_weapons[2+i][4] = Draw_PicFromWad (va("inva%i_prox",i+1));
		  }

		  hsb_items[0] = Draw_PicFromWad ("sb_wsuit");
		  hsb_items[1] = Draw_PicFromWad ("sb_eshld");
		}

}


//=============================================================================

// drawing routines are relative to the status bar location


/*
=============
Sbar_DrawPic
=============
*/
void Sbar_DrawPic (int x, int y, qpic_t *pic)
{
	Draw_Pic (x, y + 24, pic);
}

/*
=============
Sbar_DrawPicAlpha
=============
*/
void Sbar_DrawPicAlpha (int x, int y, qpic_t *pic, float alpha)
{
	Draw_AlphaPic (x, y + 24, pic, alpha);
}

/*
================
Sbar_DrawCharacter

Draws one solid graphics character
================
*/
void Sbar_DrawCharacter (int x, int y, int num)
{
	Draw_Character (x, y + 24, num);
}

/*
================
Sbar_DrawString
================
*/
void Sbar_DrawString (int x, int y, char *str)
{
	Draw_String (x, y + 24, str);
}

/*
=============
Sbar_itoa
=============
*/
int Sbar_itoa (int num, char *buf)
{
	char	*str;
	int		pow10;
	int		dig;

	str = buf;

	if (num < 0)
	{
		*str++ = '-';
		num = -num;
	}

	for (pow10 = 10 ; num >= pow10 ; pow10 *= 10)
	;

	do
	{
		pow10 /= 10;
		dig = num/pow10;
		*str++ = '0'+dig;
		num -= dig*pow10;
	} while (pow10 != 1);

	*str = 0;

	return str-buf;
}


/*
=============
Sbar_DrawNum
=============
*/
int num_kicked;
void Sbar_DrawNum (int x, int y, int num, int digits, int color)
{
	char			str[12];
	char			*ptr;
	int				l, frame;
	
	num = min(999,num); //johnfitz -- cap high values rather than truncating number

	l = Sbar_itoa (num, str);
	ptr = str;
	if (l > digits)
		ptr += (l-digits);
	if (l < digits)
		x += (digits-l)*24;

	while (*ptr)
	{
		if (*ptr == '-')
			frame = STAT_MINUS;
		else
			frame = *ptr -'0';

		Sbar_DrawPic (x,y,sb_nums[color][frame]); //johnfitz -- DrawTransPic is obsolete
		x += 24;
		ptr++;
	}
}

//=============================================================================

int		fragsort[MAX_SCOREBOARD];

char	scoreboardtext[MAX_SCOREBOARD][20];
int		scoreboardtop[MAX_SCOREBOARD];
int		scoreboardbottom[MAX_SCOREBOARD];
int		scoreboardcount[MAX_SCOREBOARD];
int		scoreboardlines;

/*
===============
Sbar_SortFrags
===============
*/
void Sbar_SortFrags (void)
{
	int		i, j, k;

// sort by frags
	scoreboardlines = 0;
	for (i=0 ; i<cl.maxclients ; i++)
	{
		if (cl.scores[i].name[0])
		{
			fragsort[scoreboardlines] = i;
			scoreboardlines++;
		}
	}

	for (i=0 ; i<scoreboardlines ; i++)
		for (j=0 ; j<scoreboardlines-1-i ; j++)
			if (cl.scores[fragsort[j]].frags < cl.scores[fragsort[j+1]].frags)
			{
				k = fragsort[j];
				fragsort[j] = fragsort[j+1];
				fragsort[j+1] = k;
			}
}

int	Sbar_ColorForMap (int m)
{
	return m < 128 ? m + 8 : m + 8;
}

/*
===============
Sbar_UpdateScoreboard
===============
*/
void Sbar_UpdateScoreboard (void)
{
	int		i, k;
	int		top, bottom;
	scoreboard_t	*s;

	Sbar_SortFrags ();

// draw the text
	memset (scoreboardtext, 0, sizeof(scoreboardtext));

	for (i=0 ; i<scoreboardlines; i++)
	{
		k = fragsort[i];
		s = &cl.scores[k];
		sprintf (&scoreboardtext[i][1], "%3i %s", s->frags, s->name);

		top = s->colors & 0xf0;
		bottom = (s->colors & 15) <<4;
		scoreboardtop[i] = Sbar_ColorForMap (top);
		scoreboardbottom[i] = Sbar_ColorForMap (bottom);
	}
}



/*
===============
Sbar_SoloScoreboard
===============
*/
void Sbar_SoloScoreboard (void)
{
	char	str[80];
	int		minutes, seconds, tens, units;
	int		l;

	// time
	minutes = cl.time / 60;
	seconds = cl.time - 60*minutes;
	tens = seconds / 10;
	units = seconds - 10*tens;
	sprintf (str,"Time:%3i:%i%i", minutes, tens, units);
	Sbar_DrawString (8, 8, str);

	// draw level name
	l = strlen (cl.levelname);
	Sbar_DrawString (232 - l*4, 8, cl.levelname);
}

/*
===============
Sbar_DrawScoreboard
===============
*/
void Sbar_DrawScoreboard (void)
{
	//Sbar_SoloScoreboard ();
	if (cl.gametype == 1 || cl.gametype == 3) //Slayer or Swat 
		Sbar_DeathmatchOverlay ();
}






void Sbar_DrawWeaponPic (void)
{
	if(!drawweapon_kicked)
	{
		AR = Draw_CachePic ("gfx/weapons/ar.lmp");	//Assault rifle
		SMG = Draw_CachePic ("gfx/weapons/smg.lmp");	//SMG
		Sniper = Draw_CachePic ("gfx/weapons/sniper.lmp");	//Sniper Rifle
		RL = Draw_CachePic ("gfx/weapons/RL.lmp");	//Rocket Launcher
		Needler = Draw_CachePic ("gfx/weapons/Needler.lmp");	//Needler
		Pistol = Draw_CachePic ("gfx/weapons/Pistol.lmp");	//Pistol
		Shottie = Draw_CachePic ("gfx/weapons/Shottie.lmp");	//Shottie
		ppist = Draw_CachePic ("gfx/weapons/ppist.lmp");	//Plasma Pistol
		drawweapon_kicked = 1;
	}
	int dif, dify;
	dif = 20;
	dify = 10;

	//Assault rifle
	if ((int)cl.stats[STAT_ACTIVEWEAPON] == IT_NAILGUN)
	{
		M_DrawTransPic ((CANVAS_WIDTH-XPADDING-AR->width), YPADDING, AR);
	}
	//Shotgun
	if ((int)cl.stats[STAT_ACTIVEWEAPON] == IT_SUPER_SHOTGUN)
	{
		M_DrawTransPic ((CANVAS_WIDTH-XPADDING-Shottie->width), YPADDING, Shottie);
	}
	//plasma Pistol
	if ((int)cl.stats[STAT_ACTIVEWEAPON] == IT_LIGHTNING)
	{
		M_DrawTransPic ((CANVAS_WIDTH-XPADDING-ppist->width), YPADDING, ppist);
	}
	//Needler
	if ((int)cl.stats[STAT_ACTIVEWEAPON] == IT_SUPER_LIGHTNING)
	{
		M_DrawTransPic ((CANVAS_WIDTH-XPADDING-Needler->width), YPADDING, Needler);
	}
	//Rocket Launcher
	if ((int)cl.stats[STAT_ACTIVEWEAPON] == IT_ROCKET_LAUNCHER)
	{
		M_DrawTransPic ((CANVAS_WIDTH-XPADDING-RL->width), YPADDING, RL);
	}
	//Pistol
	if ((int)cl.stats[STAT_ACTIVEWEAPON] == IT_SHOTGUN)
	{
		M_DrawTransPic ((CANVAS_WIDTH-XPADDING-Pistol->width), YPADDING, Pistol);
	}
	//SMG
	if ((int)cl.stats[STAT_ACTIVEWEAPON] == IT_SUPER_NAILGUN)
	{
		M_DrawTransPic ((CANVAS_WIDTH-XPADDING-SMG->width), YPADDING, SMG);
	}
	//Sniper
	if ((int)cl.stats[STAT_ACTIVEWEAPON] == IT_GRENADE_LAUNCHER)
	{
		M_DrawTransPic ((CANVAS_WIDTH-XPADDING-Sniper->width), YPADDING, Sniper);
	}
}



int oldsheilds;
int sheildrechargeplaying = 0;
int sheildbleepplaying = 0;
int nullplaying = 0;
int sheilds_channel;

void Sbar_DrawHealth(void)
{
	if(!drawhealth_kicked)
	{
		life90 = Draw_CachePic ("gfx/healthb.lmp");
		lifered = Draw_CachePic ("gfx/healthr.lmp");
		drawhealth_kicked = 1;
	}
	if(cl.stats <= 0)
		return;
	int sheilds, i, lasthealth;
	float b;
	sheilds = (int)cl.stats[STAT_HEALTH];
	lasthealth = cl.stats[STAT_HEALTH];
	b = 0;
	i = 0;
	for(i=0; i != lasthealth; i++)
	{
			if((int)cl.stats[STAT_HEALTH] <= 30)
			{
				int hbrstart = (CANVAS_WIDTH-health_bar_outline_red->width)/2 + 8;
				M_DrawPic (((int)(hbrstart+(b-lifered->width)/2)), YPADDING + 4, lifered ); //+4 y due to border of healthbar 
			}
			else
			{
				int hbstart = (CANVAS_WIDTH-health_bar_outline_blue->width)/2 + 8;
				M_DrawTransPic (((int)(hbstart+(b)/2)), YPADDING + 4, life90); //+4 y due to border of healthbar 
			}	
		b += 1.55;
		lasthealth=cl.stats[STAT_HEALTH];
	}

	if ((int)deathmatch.value < 3)
	{
		if (sheilds > oldsheilds && !sheildrechargeplaying)
		{
			S_LocalSound("player/sheild_recharge.wav");
			sheildrechargeplaying = 1;
		}
		if (sheilds > oldsheilds && !nullplaying)
		{
			S_LocalSound("player/null.wav");
			nullplaying = 1;
		}

		if (sheilds < 40 && !sheildbleepplaying && sheilds > 20)
		{
			S_LocalSound("player/sheilds_low_bleep.wav");
			sheildbleepplaying = 1;
		}

		if (sheilds == 40 && sheildrechargeplaying)
		{
			sheildrechargeplaying = 0;
			S_StopSound (cl.viewentity, 1);
		}

		if (sheilds == 100 && nullplaying)
		{
			nullplaying = 0;
			S_StopSound (cl.viewentity, 1);
		}


		if (sheilds > 40 && sheildbleepplaying)
		{
			sheildbleepplaying = 0;
			S_StopSound (cl.viewentity, 0);
		}

		oldsheilds = sheilds;
	}
}
/*
===================================
Displays weapons that are on the ground.
===================================
*/
void Sbar_DrawWW (void)
{
	int dify =  200;
	if (!drawww_kicked)
	{
		drawww_kicked = 1;
		ARW = Draw_CachePic ("gfx/worldweapons/ar.lmp");	//Assault rifle
		SMGW = Draw_CachePic ("gfx/worldweapons/smg.lmp");	//SMG
		SniperW = Draw_CachePic ("gfx/worldweapons/sniper.lmp");	//Sniper Rifle
		RLW = Draw_CachePic ("gfx/worldweapons/RL.lmp");	//Rocket Launcher
		NeedlerW = Draw_CachePic ("gfx/worldweapons/Needler.lmp");	//Needler
		PistolW = Draw_CachePic ("gfx/worldweapons/Pistol.lmp");	//Pistol
		ShottieW = Draw_CachePic ("gfx/worldweapons/Shottie.lmp");	//Shottie
		ppistW = Draw_CachePic ("gfx/worldweapons/ppist.lmp");	//Plasma Pistol

	}
	//Assault rifle
	if ((int)cl_ww.value == 1)
	{
		M_DrawTransPic ( (CANVAS_WIDTH-YPADDING-ARW->width)/2, 55+dify, ARW);
	}
	//plasma Pistol
	if ((int)cl_ww.value == 2)
	{
		M_DrawTransPic( (CANVAS_WIDTH-YPADDING-ppistW->width), 55+dify, ppistW );
	}
	//Shottie
	if ((int)cl_ww.value == 3)
	{
		M_DrawTransPic ( (CANVAS_WIDTH-YPADDING-ShottieW->width), 55+dify, ShottieW );
	}
	//Needler
	if ((int)cl_ww.value == 4)
	{
		M_DrawTransPic ( (CANVAS_WIDTH-YPADDING-NeedlerW->width), 55+dify, NeedlerW );
	}
	//Rocket Launcher
	if ((int)cl_ww.value == 5)
	{
		M_DrawTransPic ( (CANVAS_WIDTH-YPADDING-RLW->width), 55+dify, RLW );
	}
	//SMG
	if ((int)cl_ww.value == 7)
	{
		M_DrawTransPic ( (CANVAS_WIDTH-YPADDING-SMGW->width), 55+dify, SMGW );
	}
	//Pistol
	if ((int)cl_ww.value == 6)
	{
		M_DrawTransPic ( (CANVAS_WIDTH-YPADDING-PistolW->width), 55+dify, PistolW );
	}
	//Sniper
	if ((int)cl_ww.value == 8)
	{
		M_DrawTransPic ( (CANVAS_WIDTH-YPADDING-SniperW->width), 55+dify, SniperW );
	}
}

void Sbar_DrawAmmoBetter (void)
{
	int ammo_ar;
	int line0, line1, line2, i, b, ii, bb, iii, bbb;

	ammo_ar = 32;
		if(!drawammo_kicked)
		{
			//Ammo counter
			pistolb = Draw_CachePic ("gfx/ammo/pistolb.lmp");	//Pistolb
			shotgunb = Draw_CachePic ("gfx/ammo/shotgunb.lmp");	//shotgunb
			arb = Draw_CachePic ("gfx/ammo/arb.lmp");		//arb
			needleb = Draw_CachePic ("gfx/ammo/needleb.lmp");		//needleb
			rocketb = Draw_CachePic ("gfx/ammo/rocketb.lmp");		//rocketb
			smgb = Draw_CachePic ("gfx/ammo/arb.lmp");		//SMGb
			sniperb = Draw_CachePic ("gfx/ammo/sniperb.lmp");		//Sniperb
			//plasmabar = Draw_CachePic ("gfx/plasmabar.lmp");

			drawammo_kicked = 1;
		}
	int diff_ar, ammowidth;
	line0 = YPADDING+29;
	line1 = YPADDING+36;
	line2 = YPADDING+43;

	switch(cl.stats[STAT_ACTIVEWEAPON])
	{

		case IT_NAILGUN:
			ammowidth = 16 * (arb->width+6);
			for(i = 0; i < (int)cl.stats[STAT_AMMO] && i < 16; i++)
			{
				M_DrawTransPic(b+(CANVAS_WIDTH-ammowidth-10), line1, arb);
				b+= 6;
			}
			for(ii = 16; ii < (int)cl.stats[STAT_AMMO]; ii++)
			{
				M_DrawTransPic(bb+(CANVAS_WIDTH-ammowidth-10), line2, arb);
				bb+= 6;
			}
		break;
		case IT_SUPER_NAILGUN:
			ammowidth = 20 * (smgb->width+6);
			for(i = 0; i < (int)cl.stats[STAT_AMMO] && i < 20; i++)
			{
				M_DrawTransPic(b+(CANVAS_WIDTH-ammowidth-10), line0, smgb);
				b+= 6;
			}
			for(ii = 20; ii < (int)cl.stats[STAT_AMMO] && ii < 40; ii++)
			{
				M_DrawTransPic(bb+(CANVAS_WIDTH-ammowidth-10), line1, smgb);
				bb+= 6;
			}
			for(iii = 40; iii < (int)cl.stats[STAT_AMMO]; iii++)
			{
				M_DrawTransPic(bbb+(CANVAS_WIDTH-ammowidth-10), line2, smgb);
				bbb+= 6;
			}
		break;
		case IT_SUPER_LIGHTNING:
			ammowidth = 15 * (needleb->width+6);
			for(i = 0; i < (int)cl.stats[STAT_AMMO]; i++)
			{
				M_DrawTransPic(b+(CANVAS_WIDTH-ammowidth-10), line2, needleb);
				b+= 6;
			}
		break;
		case IT_GRENADE_LAUNCHER:
			ammowidth = 2 * (sniperb->width+60);
			for(i = 0; i < (int)cl.stats[STAT_AMMO] && i < 2; i++)
			{
				M_DrawTransPic(b+(CANVAS_WIDTH-ammowidth), line0, sniperb);
				b+= 70;
			}
			for(ii = 2; ii < (int)cl.stats[STAT_AMMO]; ii++)
			{
				M_DrawTransPic(bb+(CANVAS_WIDTH-ammowidth), line1, sniperb);
				bb+= 70;
			}
		break;
		case IT_ROCKET_LAUNCHER:
			ammowidth = 2 * (rocketb->width+60);
			for(i = 0; i < (int)cl.stats[STAT_AMMO]; i++)
			{
				M_DrawTransPic(b+(CANVAS_WIDTH-ammowidth), line1, rocketb);
				b+= 70;
			}
		break;
		case IT_SUPER_SHOTGUN:
			ammowidth = 8 * (shotgunb->width+12);
			for(i = 0; i < (int)cl.stats[STAT_AMMO]; i++)
			{
				M_DrawTransPic(b+(CANVAS_WIDTH-ammowidth-20), line1, shotgunb);
				b+= 12;
			}
		break;
		case IT_SHOTGUN:
			ammowidth = 12 * (pistolb->width+8);
			for(i = 0; i < (int)cl.stats[STAT_AMMO]; i++)
			{
				M_DrawPic(b+(CANVAS_WIDTH-ammowidth-10), line1, pistolb);
				b+= 8;
			}
		break;
		case IT_LIGHTNING: //plasma ?
			for(i = 0; i < (int)cl.stats[STAT_ARMOR]; i++)
			{
				if((int)cl.stats[STAT_ARMOR] > 80)
				{
					ammowidth = 35 * lifered->width;
					M_DrawTransPic(b+(CANVAS_WIDTH-ammowidth-10), line1, lifered);
				}
				else
				{
					ammowidth = 35 * life90->width;
					M_DrawTransPic(b+(CANVAS_WIDTH-ammowidth-10), line1, life90);
				}
				b+=1;
			}
		break;
	}
	char up[4];
	//plasma pistol
	if((int)cl.stats[STAT_ACTIVEWEAPON] == IT_LIGHTNING)
	{
		sprintf (up,"%d", cl.stats[STAT_AMMO]);
		Sbar_DrawString (CANVAS_WIDTH-100, YPADDING, up);
	}
	//needler and pistol
	else if((int)cl.stats[STAT_ACTIVEWEAPON] == IT_SUPER_LIGHTNING | cl.stats[STAT_ACTIVEWEAPON] == IT_SHOTGUN)
	{
		sprintf (up,"%d", (int)cl.stats[STAT_ARMOR]);
		Sbar_DrawString (CANVAS_WIDTH-100, YPADDING, up);
	}
	else
	{
		sprintf (up,"%d", (int)cl.stats[STAT_ARMOR]);
		Sbar_DrawString (CANVAS_WIDTH-80, YPADDING, up);
	}
}

void Sbar_DrawNade (void)
{
	int dif, dif2;
	dif= 20;
	dif2= -20;
	if(!drawnade_kicked)
	{
			//Nade types
		plasma = Draw_CachePic ("gfx/plasmanade.lmp");
		frag = Draw_CachePic ("gfx/fraggrenade.lmp");
		//Nade number counter
		numbers[0] = Draw_CachePic ("gfx/numbers/num_0.lmp");
		numbers[1] = Draw_CachePic ("gfx/numbers/num_1.lmp");
		numbers[2] = Draw_CachePic ("gfx/numbers/num_2.lmp");
		numbers[3] = Draw_CachePic ("gfx/numbers/num_3.lmp");
		numbers[4] = Draw_CachePic ("gfx/numbers/num_4.lmp");
		drawnade_kicked = 1;
	}

	/*
	===================================
	Displays what Grenade you currently have.
	===================================
	*/
	//For Plasma Nades.
	if ((int)cl_plasmanade.value == 1)
	{
		M_DrawTransPic (XPADDING, YPADDING, plasma);
	}
	//These are Actually Frag Grenades.
	else if ((int)cl_plasmanade.value == 2)
	{
		M_DrawTransPic(XPADDING, YPADDING, frag);
	}
	else
	{
		M_DrawTransPic(XPADDING, YPADDING, frag);
	}

	/*
	===================================
	Grenade counter numbers
	===================================
	*/
	int offs = 30;
	if ((int)cl_nadenum.value == 1)
	{
		M_DrawTransPic (XPADDING+offs, YPADDING+offs, numbers[0]);
	}
	if ((int)cl_nadenum.value == 2)
	{
		M_DrawTransPic (XPADDING+offs, YPADDING+offs, numbers[1] );
	}
		if ((int)cl_nadenum.value == 3)
	{
		M_DrawTransPic (XPADDING+offs, YPADDING+offs, numbers[2] );
	}
	if ((int)cl_nadenum.value == 4)
	{
		M_DrawTransPic (XPADDING+offs, YPADDING+offs, numbers[3] );
	}
	if ((int)cl_nadenum.value == 5)
	{
		M_DrawTransPic (XPADDING+offs, YPADDING+offs, numbers[4] );
	}
}

void Sbar_DrawFirefight (void)
{
/*
=============================================
Firefight Rounds
===================================================
*/
	char up[35];
	int round;
	round = (int)cl_round.value;
	if ((int)deathmatch.value == 2)
	{
		sprintf (up,"ROUND [%d]", round);
		Sbar_DrawString (CANVAS_WIDTH-(CANVAS_WIDTH*0.1), 430, up);
	}
/*
=============================================
Firefight Lives
===================================================
*/
	round = (int)cl_life.value;
	if ((int)deathmatch.value == 2)
	{
		sprintf (up,"LIVES: %d", round);
		Sbar_DrawString (CANVAS_WIDTH-(CANVAS_WIDTH*0.1), 450, up);
	}
}

void Sbar_DrawRespawn (void)
{
	char respawnstr[35];
	int round = 8;
	round = (int)cl_respawn.value;
	sprintf (respawnstr,"You will respawn in %d seconds", round);
	size_t len = strlen(respawnstr) * 8; //8 is character glyph width
	int canwidth = 320*MENU_SCALE;
	int canheight = 200*MENU_SCALE;
	int x = canwidth/2 - len/2;
	int y = canheight - (canheight*0.4);

	Sbar_DrawString (x, y, respawnstr);
}


void Sbar_DrawGametype (void)
{
	int canwidth = 320*MENU_SCALE;
	int canheight = 200*MENU_SCALE;
	int x = canwidth-(canwidth*0.15);
	int y = canheight - (canheight*0.3);
	//set canvas mode
	GL_SetCanvas(CANVAS_MENU_STRETCH);
	if (deathmatch.value == 1)
		Sbar_DrawString (x, y, "Slayer");
	if (deathmatch.value == 2)
		Sbar_DrawString (x, y, "Firefight");
	if (deathmatch.value == 3)
		Sbar_DrawString (x, y, "Swat");
	//set canvas mode
	GL_SetCanvas(CANVAS_DEFAULT);
}

void Sbar_DrawMedal ()
{
	/*
	===================================
	Kill medals
	===================================
	*/
	if(!drawmedal_kicked)
	{
			//Kill medals
		doublek = Draw_CachePic ("gfx/medals/double.lmp");
		triple = Draw_CachePic ("gfx/medals/triple.lmp");
		killtacular = Draw_CachePic ("gfx/medals/killtacular.lmp");
		killtrocity = Draw_CachePic ("gfx/medals/killtrocity.lmp");
		drawmedal_kicked = 1;
	}

	if (cl_killmedals.value == 1)
	{
		M_DrawTransPic  (XPADDING, 335, doublek );
	}
	if (cl_killmedals.value == 2)
	{
		M_DrawTransPic  (XPADDING, 335, triple );
	}
	if (cl_killmedals.value == 3)
	{
		M_DrawTransPic  (XPADDING, 335, killtacular );
	}
	if (cl_killmedals.value == 4)
	{
		M_DrawTransPic (XPADDING, 335, killtrocity );
	}
}

void Sbar_DrawSlowmo (void)
{
	if(!drawslowmo_kicked)
	{
		slomoclock[0] = Draw_CachePic ("gfx/clock/clock1.lmp");
		slomoclock[1] = Draw_CachePic ("gfx/clock/clock2.lmp");
		slomoclock[2] = Draw_CachePic ("gfx/clock/clock3.lmp");
		slomoclock[3] = Draw_CachePic ("gfx/clock/clock4.lmp");
		slomoclock[4] = Draw_CachePic ("gfx/clock/clock5.lmp");
		slomoclock[5] = Draw_CachePic ("gfx/clock/clock6.lmp");
		slomoclock[6] = Draw_CachePic ("gfx/clock/clock7.lmp");
		slomoclock[7] = Draw_CachePic ("gfx/clock/clock8.lmp");
		slomoclock[8] = Draw_CachePic ("gfx/clock/clock9.lmp");
		drawslowmo_kicked = 1;
	}

	/*
	=============================================
	Slowmo timer
	===================================================
	*/
	int round;
	round = (int)cl_slowmo.value;
	M_DrawTransPic (XPADDING, CANVAS_HEIGHT-YPADDING, slomoclock[round] );
}

int crosshairs_kicked;
void Sbar_CrossHairs (void)
{
	GL_SetCanvas(CANVAS_CROSSHAIR);
	if(!crosshairs_kicked)
	{
		//Red Crosshairs
		arred = Draw_CachePic ("gfx/CH/ch_rar.lmp");
		smgred = Draw_CachePic ("gfx/CH/ch_rsmg.lmp");
		sniperred = Draw_CachePic ("gfx/CH/ch_rsniper.lmp");
		rlred = Draw_CachePic ("gfx/CH/ch_rrl.lmp");
		pistolred = Draw_CachePic ("gfx/CH/ch_rpistol.lmp");
		ppistred = Draw_CachePic ("gfx/CH/ch_rppist.lmp");
		needlerred = Draw_CachePic ("gfx/CH/ch_rneedler.lmp");
		shotgunred = Draw_CachePic ("gfx/CH/ch_rshottie.lmp");
		swordred = Draw_CachePic ("gfx/CH/ch_rsword.lmp");

		//Blue Crosshairs
		arblue = Draw_CachePic ("gfx/CH/ch_ar.lmp");
		smgblue = Draw_CachePic ("gfx/CH/ch_smg.lmp");
		sniperblue = Draw_CachePic ("gfx/CH/ch_sniper.lmp");
		rlblue = Draw_CachePic ("gfx/CH/ch_rl.lmp");
		pistolblue = Draw_CachePic ("gfx/CH/ch_pistol.lmp");
		ppistblue = Draw_CachePic ("gfx/CH/ch_ppist.lmp");
		needlerblue = Draw_CachePic ("gfx/CH/ch_needler.lmp");
		shotgunblue = Draw_CachePic ("gfx/CH/ch_shottie.lmp");
		swordblue = Draw_CachePic ("gfx/CH/ch_sword.lmp");

		crosshairs_kicked = 1;
	}
	/* Red Crosshairs */
	if(cl_ch_red.value == 1)		//Red CrossHair
	{
		switch(cl.stats[STAT_ACTIVEWEAPON])
		{
			case IT_NAILGUN:
				M_DrawPic (0-arred->width/2, 0-arred->height/2, arred);
			break;
			case IT_SUPER_NAILGUN:
				M_DrawPic (0-smgred->width/2, 0-smgred->height/2, smgred);
			break;
			case IT_GRENADE_LAUNCHER:
				M_DrawPic (0-sniperred->width/2, 0-sniperred->height/2, sniperred);
			break;
			case IT_ROCKET_LAUNCHER:
				M_DrawPic (0-rlred->width/2, 0-rlred->height/2, rlred);
			break;
			case IT_SHOTGUN:
				M_DrawPic (0-pistolred->width/2, 0-pistolred->height/2, pistolred);
			break;
			case IT_EXTRA_WEAPON:	//Needs red pick
				M_DrawPic (0-needlerred->width/2, 0-needlerred->height/2, needlerred);
			break;
			case IT_AXE:	//Needs red pick
				M_DrawPic (0-swordred->width/2, 0-swordred->height/2, swordred);
			break;
		}
	}
	/* Blue Crosshairs */
	else if (cl_ch_blue.value == 1)
	{
		switch(cl.stats[STAT_ACTIVEWEAPON])
		{
			case IT_NAILGUN:
				M_DrawPic (0-arblue->width/2, 0-arblue->height/2, arblue);
			break;
			case IT_SUPER_NAILGUN:
				M_DrawPic (0-smgblue->width/2, 0-smgblue->height/2, smgblue);
			break;
			case IT_GRENADE_LAUNCHER:
				M_DrawPic (0-sniperblue->width/2, 0-sniperblue->height/2, sniperblue);
			break;
			case IT_ROCKET_LAUNCHER:
				M_DrawPic (0-rlblue->width/2, 0-rlblue->height/2, rlblue);
			break;
			case IT_SHOTGUN:
				M_DrawPic (0-pistolblue->width/2, 0-pistolblue->height/2, pistolblue);
			break;
			case IT_EXTRA_WEAPON:
				M_DrawPic (0-needlerblue->width/2, 0-needlerblue->height/2, needlerblue);
			break;
			case IT_AXE:	//Needs pick
				M_DrawPic (0-swordblue->width/2, 0-swordblue->height/2, swordblue);
			break;
		}
	}
	GL_SetCanvas(CANVAS_DEFAULT);
}

/*
===============
Sbar_DrawInventory
===============
*/
int h_flash;
float h_flash_timer;
void Sbar_DrawInventory (void)
{
	if(!cach)
	{
		health_bar_outline_red = Draw_CachePic ("gfx/healthbackr.lmp");
		health_bar_outline_blue = Draw_CachePic ("gfx/healthback.lmp");
		cach=1;
	}
	if((int)hide_hud.value)
		return;
	if((int)cl.stats[STAT_HEALTH] <= 0)
		return;

	if((int)cl.stats[STAT_HEALTH] > 0)
	{

		if((int)deathmatch.value == 2) {
			Sbar_DrawFirefight();
		}
		if((int)deathmatch.value == 3)
		{
			Sbar_DrawSlowmo();
		}
		else
		{
			Sbar_DrawNade();
			Sbar_DrawHealth();
		}
		Sbar_CrossHairs ();
		Sbar_DrawMedal ();
		Sbar_DrawGametype();
		Sbar_DrawWeaponPic();
		Sbar_DrawAmmoBetter();
		Sbar_DrawWW();

		if ((int)deathmatch.value != 3)
		{
			if ((int)cl.stats[STAT_HEALTH] <= 30)
			{
				M_DrawTransPic ( (CANVAS_WIDTH-health_bar_outline_red->width)/2, YPADDING, health_bar_outline_red);
				//if(h_flash == 1 && h_flash_timer <= 0.5f)
				//{
				//	M_DrawTransPic ( (CANVAS_WIDTH-health_bar->width)/*/2*/, 16, health_bar);
				//	h_flash = 0;
				//	if(h_flash_timer <= 0)
				//		h_flash_timer = 1;
				//}
				//else
				//{
					//M_DrawTransPic ( (CANVAS_WIDTH-health_bar_red->width)/*/2*/, 16, health_bar_red);
				//	h_flash = 1;
				//}
				//h_flash_timer -= 0.01;
			}
			else
			{
				M_DrawTransPic ( (CANVAS_WIDTH-health_bar_outline_blue->width)/2, YPADDING, health_bar_outline_blue);
			}
		}
	}
}


void Sbar_DrawScope (void)
{
qpic_t *scope;

scope = Draw_CachePic ("gfx/ch/scope.lmp");

//Draw_Pic (0, 0, scope);
}




/*
===============
Sbar_DrawFrags
===============
*/
// void Sbar_DrawFrags (void)
// {
// 	int				i, k, l;
// 	int				top, bottom;
// 	int				x, y, f;
// 	int				xofs;
// 	char			num[12];
// 	scoreboard_t	*s;

// 	Sbar_SortFrags ();

//     // draw the text
// 	l = scoreboardlines <= 4 ? scoreboardlines : 4;

// 	x = 23;
// 	if (cl.gametype == GAME_DEATHMATCH)
// 		xofs = 0;
// 	else
// 		xofs = (vid.width - 320)>>1;
// 	y = vid.height - 24 - 23;

// 	for (i=0 ; i<l ; i++)
// 	{
// 		k = fragsort[i];
// 		s = &cl.scores[k];
// 		if (!s->name[0])
// 			continue;

// 	// draw background
// 		top = s->colors & 0xf0;
// 		bottom = (s->colors & 15)<<4;
// 		top = Sbar_ColorForMap (top);
// 		bottom = Sbar_ColorForMap (bottom);

// 		Draw_Fill (xofs + x*8 + 10, y, 28, 4, top);
// 		Draw_Fill (xofs + x*8 + 10, y+4, 28, 3, bottom);

// 	// draw number
// 		f = s->frags;
// 		sprintf (num, "%3i",f);

// 		Sbar_DrawCharacter ( (x+1)*8 , -24, num[0]);
// 		Sbar_DrawCharacter ( (x+2)*8 , -24, num[1]);
// 		Sbar_DrawCharacter ( (x+3)*8 , -24, num[2]);

// 		if (k == cl.viewentity - 1)
// 		{
// 			Sbar_DrawCharacter (x*8+2, -24, 16);
// 			Sbar_DrawCharacter ( (x+4)*8-4, -24, 17);
// 		}
// 		x+=4;
// 	}
// }

//=============================================================================


/*
===============
Sbar_DrawFace
===============
*/
// void Sbar_DrawFace (void)
// {
// 	int	f, anim;

// // PGM 01/19/97 - team color drawing
// // PGM 03/02/97 - fixed so color swatch only appears in CTF modes
// 	if (rogue &&
//         (cl.maxclients != 1) &&
//         (teamplay.value>3) &&
//         (teamplay.value<7))
// 	{
// 		int				top, bottom;
// 		int				xofs;
// 		char			num[12];
// 		scoreboard_t	*s;

// 		s = &cl.scores[cl.viewentity - 1];
// 		// draw background
// 		top = s->colors & 0xf0;
// 		bottom = (s->colors & 15)<<4;
// 		top = Sbar_ColorForMap (top);
// 		bottom = Sbar_ColorForMap (bottom);

// 		if (cl.gametype == GAME_DEATHMATCH)
// 			xofs = 113;
// 		else
// 			xofs = ((vid.width - 320)>>1) + 113;

// 		Sbar_DrawPic (112, 0, rsb_teambord);
// 		Draw_Fill (xofs, /*vid.height-*/24+3, 22, 9, top); //johnfitz -- sbar coords are now relative
// 		Draw_Fill (xofs, /*vid.height-*/24+12, 22, 9, bottom); //johnfitz -- sbar coords are now relative

// 		// draw number
// 		f = s->frags;
// 		sprintf (num, "%3i",f);

// 		if (top==8)
// 		{
// 			if (num[0] != ' ')
// 				Sbar_DrawCharacter(109, 3, 18 + num[0] - '0');
// 			if (num[1] != ' ')
// 				Sbar_DrawCharacter(116, 3, 18 + num[1] - '0');
// 			if (num[2] != ' ')
// 				Sbar_DrawCharacter(123, 3, 18 + num[2] - '0');
// 		}
// 		else
// 		{
// 			Sbar_DrawCharacter ( 109, 3, num[0]);
// 			Sbar_DrawCharacter ( 116, 3, num[1]);
// 			Sbar_DrawCharacter ( 123, 3, num[2]);
// 		}

// 		return;
// 	}
// 	/*
// 	========================================
// 	Health bar edges
// 	========================================
// 	*/

// 	Sbar_DrawGametype();
// 	/*
// 	if (deathmatch.value != 3)
// 	{
// 		if (cl.stats[STAT_HEALTH] <= 30)
// 		{
// 			M_DrawPic ( (324-health_bar_red->width)/2, 16, health_bar_red);
// 		}
// 		else
// 		{
// 			M_DrawPic ( (324-health_bar->width)/2, 16, health_bar);
// 		}
// 		Sbar_DrawHealth();
// 	}

// 	Sbar_DrawWW();

// 	if (deathmatch.value == 3)
// 	{
// 		Sbar_DrawSlowmo();
// 	}
// 	*/
// 	//if (cl_activate.value == 1)
// 	//V_ParseDamage();

// 	/*
// 	Sbar_DrawFirefight();

// 	if (deathmatch.value != 3)
// 		Sbar_DrawNade();

// 	if (cl_scope.value == 1)
// 		Sbar_DrawScope();
// 	*/
// 	//Sbar_DrawWeapon();

// 	//Sbar_DrawAmmo();

// 	//Sbar_DrawMedal();
// }

/*
===============
Sbar_Draw
===============
*/
void Sbar_Draw (void)
{
	if (scr_con_current == vid.height)
		return;		// console is full screen

	/*
	if (sb_updates >= vid.numpages)
		return;
	*/

	scr_copyeverything = 1;

	sb_updates++;
	
	GL_SetCanvas (CANVAS_DEFAULT); //johnfitz
	
	if (sb_lines && vid.width > 320)
		Draw_TileClear (0, vid.height - sb_lines, vid.width, sb_lines);

	GL_SetCanvas (CANVAS_SBAR); //johnfitz

	if (sb_showscores || cl.stats[STAT_HEALTH] <= 0)
	{
		Sbar_DrawScoreboard ();
		sb_updates = 0;
		if (cl.stats[STAT_HEALTH] <= 0)
		{
			GL_SetCanvas(CANVAS_MENU_STRETCH);
			Sbar_DrawRespawn();
			GL_SetCanvas(CANVAS_SBAR);
		}
	}

	GL_SetCanvas (CANVAS_DEFAULT);
	//Sbar_DrawFace();
	if (viewsize.value < 130 && !sb_showscores && cl.stats[STAT_HEALTH] > 0)
    	{
		Sbar_DrawInventory ();
		Sbar_MiniDeathmatchOverlay();
	}
}

//=============================================================================

/*
==================
Sbar_IntermissionNumber

==================
*/
void Sbar_IntermissionNumber (int x, int y, int num, int digits, int color)
{
	char			str[12];
	char			*ptr;
	int				l, frame;

	l = Sbar_itoa (num, str);
	ptr = str;
	if (l > digits)
		ptr += (l-digits);
	if (l < digits)
		x += (digits-l)*24;

	while (*ptr)
	{
		if (*ptr == '-')
			frame = STAT_MINUS;
		else
			frame = *ptr -'0';

		Draw_Pic (x,y,sb_nums[color][frame]);
		x += 24;
		ptr++;
	}
}

/*
==================
Sbar_DeathmatchOverlay

==================
*/
void Sbar_DeathmatchOverlay (void)
{
	qpic_t			*pic;
	int				i, k, l;
	int				top, bottom;
	int				x, y, f;
	char			num[12];
	scoreboard_t	*s;

	GL_SetCanvas (CANVAS_MENU); //johnfitz

	scr_copyeverything = 1;
	scr_fullupdate = 0;

// scores
	Sbar_SortFrags ();

// draw the text
	l = scoreboardlines;

	x = 80;
	y = 40;
	Draw_Fill ( x+12, 28, 136, 10, 0);
	M_PrintWhite (x+20, 29,"  LEADERBOARD");

	for (i=0 ; i<l ; i++)
	{
		k = fragsort[i];
		s = &cl.scores[k];
		if (!s->name[0])
			continue;

		// draw background
		bottom = (s->colors & 15)<<4;
		bottom = Sbar_ColorForMap (bottom);

		Draw_Fill ( x+12, y, 32, 8, 1); //score num bg
		Draw_Fill ( x+52, y, 96, 8, bottom); //player name bg

		// draw number
		f = s->frags;
		sprintf (num, "%3i",f);

		Draw_Character ( x+8 , y, num[0]);
		Draw_Character ( x+16 , y, num[1]);
		Draw_Character ( x+24 , y, num[2]);

		if (k == cl.viewentity - 1)
			Draw_Character ( x - 8, y, 12);

#if 0
{
	int				total;
	int				n, minutes, tens, units;

	// draw time
		total = cl.completed_time - s->entertime;
		minutes = (int)total/60;
		n = total - minutes*60;
		tens = n/10;
		units = n%10;

		sprintf (num, "%3i:%i%i", minutes, tens, units);

		Draw_String ( x+48 , y, num);
}
#endif

	// draw name
		M_PrintWhite (x+64, y, s->name);

		y += 10;
	}
	
	GL_SetCanvas (CANVAS_SBAR); //johnfitz
}

/*
==================
Sbar_DeathmatchOverlay

==================
*/
void Sbar_MiniDeathmatchOverlay (void)
{
	int				i, k, l;
	int				bottom;
	int				x, y, f;
	char			num[12];
	scoreboard_t	*s;

	scr_copyeverything = 1;
	scr_fullupdate = 0;
	//set canvas mode
	GL_SetCanvas(CANVAS_MENU_STRETCH);
	// scores
	Sbar_SortFrags ();

	// draw the text
	l = scoreboardlines;
	if (l > 2)
	{
		l = 2;
	}

	x = 400;
	y = 250;

	for (i=0 ; i<l ; i++) //only show two lines
	{
		k = fragsort[i];
		s = &cl.scores[k];
		if (!s->name[0])
			continue;

		// draw background
		bottom = (s->colors & 15) <<4;
		bottom = Sbar_ColorForMap (bottom);

		Draw_Fill ( x, y, 70, 10, bottom);

	   // draw number
		f = s->frags;
		sprintf (num, "%3i",f);

		Draw_Character ( x+40, y+1, num[0]);
		Draw_Character ( x+48, y+1, num[1]);
		Draw_Character ( x+56 , y+1, num[2]);

		if (k == cl.viewentity - 1)
			Draw_Character ( x - 8, y, 13);

		//Draw_String (x+32, y, s->name);

		y += 16;
	}
	//set canvas mode
	GL_SetCanvas(CANVAS_DEFAULT);
}

/*
==================
Sbar_IntermissionOverlay

==================
*/
void Sbar_IntermissionOverlay (void)
{
	qpic_t	*pic;
	int		dig;
	int		num;

	scr_copyeverything = 1;
	scr_fullupdate = 0;

	if (cl.gametype == 1 || cl.gametype == 3) //Slayer or Swat
	{
		Sbar_DeathmatchOverlay ();
		return;
	}

	GL_SetCanvas (CANVAS_MENU); //johnfitz

	//muff@yakko.globalnet.co.uk
	//dead easy stuff really
	pic = Draw_CachePic ("gfx/complete.lmp");
	Draw_Pic (64, 24, pic);

	pic = Draw_CachePic ("gfx/inter.lmp");
	Draw_TransPic (0, 56, pic);

	dig = cl.completed_time/60;
	Sbar_IntermissionNumber (152, 64, dig, 3, 0); //johnfitz -- was 160
	num = cl.completed_time - dig*60;

	Draw_Pic (224,64,sb_colon); //johnfitz -- was 234
	Draw_Pic (240,64,sb_nums[0][num/10]); //johnfitz -- was 246
	Draw_Pic (264,64,sb_nums[0][num%10]); //johnfitz -- was 266

	Sbar_IntermissionNumber (152, 104, cl.stats[STAT_SECRETS], 3, 0); //johnfitz -- was 160
	Draw_Pic (224,104,sb_slash); //johnfitz -- was 232
	Sbar_IntermissionNumber (240, 104, cl.stats[STAT_TOTALSECRETS], 3, 0); //johnfitz -- was 248

	Sbar_IntermissionNumber (152, 144, cl.stats[STAT_MONSTERS], 3, 0); //johnfitz -- was 160
	Draw_Pic (224,144,sb_slash); //johnfitz -- was 232
	Sbar_IntermissionNumber (240, 144, cl.stats[STAT_TOTALMONSTERS], 3, 0); //johnfitz -- was 248
	//end of muff

}


/*
==================
Sbar_FinaleOverlay

==================
*/
void Sbar_FinaleOverlay (void)
{
	qpic_t	*pic;

	scr_copyeverything = 1;
	
	GL_SetCanvas (CANVAS_MENU); //johnfitz

	pic = Draw_CachePic ("gfx/finale.lmp");
	Draw_TransPic ( (CANVAS_MENU_WIDTH-pic->width)/2, 16, pic);
}
