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

#include "quakedef.h"
#include "net_dgrm.h"

char res_string[256];
CVAR (vid_vsync, 1, CVAR_ARCHIVE)
extern cvar_t	fov;
extern cvar_t	crosshair;
extern cvar_t	invert_camera;
extern cvar_t	pstv_rumble;
extern cvar_t	retrotouch;
extern cvar_t	scr_sbaralpha;
extern cvar_t	motioncam;
extern cvar_t	motion_horizontal_sensitivity;
extern cvar_t	motion_vertical_sensitivity;
extern cvar_t	psvita_front_sensitivity_x;
extern cvar_t	psvita_front_sensitivity_y;
extern cvar_t	psvita_touchmode;
extern cvar_t	gl_torchflares;
extern cvar_t	show_fps;
extern cvar_t	gl_fog;
extern cvar_t	gl_outline;
extern cvar_t	r_viewmodeloffset;
extern cvar_t	st_separation;
extern int scr_width;
extern int scr_height;
extern uint8_t is_uma0;
int cfg_width;
int cfg_height;
extern cvar_t gl_bilinear;
int m_state = m_none;
bool mm_rentry = false;

extern ModsList* mods;
extern int max_mod_idx;

void (*vid_menudrawfn)(void);
void (*vid_menukeyfn)(int key);

void M_Menu_Main_f (void);
	void M_Menu_SinglePlayer_f (void);
		void M_Menu_Load_f (void);
		void M_Menu_Save_f (void);
	void M_Menu_MultiPlayer_f (void);
		void M_Menu_Setup_f (void);
		void M_Menu_Net_f (void);
			void M_Menu_OnlineServerList_f (void);
	void M_Menu_Options_f (void);
		void M_Menu_Keys_f (void);
		void M_Menu_Video_f (void);
	void M_Menu_Help_f (void);
	void M_Menu_Quit_f (void);
void M_Menu_LanConfig_f (void);
void M_Matchmaking_f (void);
void M_Firefight_f (void);
void M_Menu_Search_f (void);
void M_Menu_ServerList_f (void);
void M_Menu_Pause_f (void);

void M_Main_Draw (void);
	void M_SinglePlayer_Draw (void);
		void M_Load_Draw (void);
		void M_Save_Draw (void);
	void M_MultiPlayer_Draw (void);
		void M_Setup_Draw (void);
		void M_Net_Draw (void);
			void M_OnlineServerList_Draw (void);
	void M_Options_Draw (void);
		void M_Keys_Draw (void);
		void M_Graphics_Draw (void);
		void M_Video_Draw (void);
		void M_Mods_Draw (void);
	void M_Help_Draw (void);
	void M_Quit_Draw (void);
void M_LanConfig_Draw (void);
void M_Matchmaking_Draw (void);
void M_Firefight_Draw (void);
void M_Search_Draw (void);
void M_ServerList_Draw (void);
void M_Pause_Draw (void);

void M_Main_Key (int key);
	void M_SinglePlayer_Key (int key);
		void M_Load_Key (int key);
		void M_Save_Key (int key);
	void M_MultiPlayer_Key (int key);
		void M_Setup_Key (int key);
		void M_Net_Key (int key);
			void M_OnlineServerList_Key (int key);
	void M_Options_Key (int key);
		void M_Keys_Key (int key);
		void M_Graphics_Key (int key);
		void M_Video_Key (int key);
		void M_Mods_key (int key);
	void M_Help_Key (int key);
	void M_Quit_Key (int key);
void M_LanConfig_Key (int key);
void M_Matchmaking_Key (int key);
void M_Firefight_Key (int key);
void M_Search_Key (int key);
void M_ServerList_Key (int key);
void M_Pause_Key (int key);

bool	m_entersound;		// play after drawing a frame, so caching
								// won't disrupt the sound
bool	m_recursiveDraw;

int			m_return_state;
bool	m_return_onerror;
char		m_return_reason [32];

int	m_multiplayer_cursor = 1;
#define StartingGame	(m_multiplayer_cursor == 1)
#define JoiningGame		(m_multiplayer_cursor == 0)
#define SerialConfig	(m_net_cursor == 0)
#define DirectConfig	(m_net_cursor == 1)
#define	IPXConfig		(m_net_cursor == 2)
#define	TCPIPConfig		(m_net_cursor == 3)

void M_ConfigureNetSubsystem(void);

int antialiasing = 2;
uint8_t netcheck_dialog_running = 0;

void SetResolution(int w, int h){
	char res_str[64];
	FILE *f = NULL;
	if (is_uma0) f = fopen("uma0:data/Quake/resolution.cfg", "wb");
	else f = fopen("ux0:data/Quake/resolution.cfg", "wb");
	sprintf(res_str, "%dx%d", w, h);
	fwrite(res_str, 1, strlen(res_str), f);
	fclose(f);
	cfg_width = w;
	cfg_height = h;
}

void SetAntiAliasing(int m){
	char res_str[64];
	FILE *f = NULL;
	if (is_uma0) f = fopen("uma0:data/Quake/antialiasing.cfg", "wb");
	else f = fopen("ux0:data/Quake/antialiasing.cfg", "wb");
	sprintf(res_str, "%d", m);
	fwrite(res_str, 1, strlen(res_str), f);
	fclose(f);
}

void M_DrawColorBar (int x, int y, int highlight)
{
    int i;
    int intense = highlight * 16 + (highlight < 8 ? 11 : 4);

    for (i = 0; i < 14; i++)
    {
        // take the approximate midpoint colour (handle backward ranges)
        int c = i * 16 + (i < 8 ? 8 : 7);

        // braw baseline colour (offset downwards a little so that it fits correctly
        Draw_Fill (x + i * 8, y + 4, 8, 8, c);
    }

    // draw the highlight rectangle
    Draw_Fill (x - 1 + highlight * 8, y + 3, 10, 10, 15);

    // redraw the highlighted color at brighter intensity
    Draw_Fill (x + highlight * 8, y + 4, 8, 8, intense);
}

/*
================
M_DrawCharacter

Draws one solid graphics character
================
*/
void M_DrawCharacter (int cx, int line, int num)
{
	Draw_Character ( cx, line, num);
}

void M_Print (int cx, int cy, char *str)
{
	while (*str)
	{
		M_DrawCharacter (cx, cy, (*str)+128);
		str++;
		cx += 8;
	}
}

void M_PrintCentered (int cy, char *str)
{
	int cx = (320*MENU_SCALE)/2 - strlen(str) * 4;
	
	while (*str)
	{
		M_DrawCharacter (cx, cy, (*str)+128);
		str++;
		cx += 8;
	}
}


void M_PrintWhite (int cx, int cy, char *str)
{
	while (*str)
	{
		M_DrawCharacter (cx, cy, *str);
		str++;
		cx += 8;
	}
}

void M_DrawTransPic (int x, int y, qpic_t *pic)
{
	Draw_TransPic (x, y, pic);
}

void M_DrawPic (int x, int y, qpic_t *pic)
{
	Draw_Pic (x, y, pic);
}

byte identityTable[256];
byte translationTable[256];

void M_BuildTranslationTable(int top, int bottom)
{
	int		j;
	byte	*dest, *source;

	for (j = 0; j < 256; j++)
		identityTable[j] = j;
	dest = translationTable;
	source = identityTable;
	memcpy (dest, source, 256);

	if (top < 128)	// the artists made some backwards ranges.  sigh.
		memcpy (dest + TOP_RANGE, source + top, 16);
	else
		for (j=0 ; j<16 ; j++)
			dest[TOP_RANGE+j] = source[top+15-j];

	if (bottom < 128)
		memcpy (dest + BOTTOM_RANGE, source + bottom, 16);
	else
		for (j=0 ; j<16 ; j++)
			dest[BOTTOM_RANGE+j] = source[bottom+15-j];
}


void M_DrawTransPicTranslate (int x, int y, qpic_t *pic)
{
	Draw_TransPicTranslate (x, y, pic, translationTable);
}


void M_DrawTextBox (int x, int y, int width, int lines)
{
	qpic_t	*p;
	int		cx, cy;
	int		n;

	// draw left side
	cx = x;
	cy = y;
	p = Draw_CachePic ("gfx/box_tl.lmp");
	M_DrawTransPic (cx, cy, p);
	p = Draw_CachePic ("gfx/box_ml.lmp");
	for (n = 0; n < lines; n++)
	{
		cy += 8;
		M_DrawTransPic (cx, cy, p);
	}
	p = Draw_CachePic ("gfx/box_bl.lmp");
	M_DrawTransPic (cx, cy+8, p);

	// draw middle
	cx += 8;
	while (width > 0)
	{
		cy = y;
		p = Draw_CachePic ("gfx/box_tm.lmp");
		M_DrawTransPic (cx, cy, p);
		p = Draw_CachePic ("gfx/box_mm.lmp");
		for (n = 0; n < lines; n++)
		{
			cy += 8;
			if (n == 1)
				p = Draw_CachePic ("gfx/box_mm2.lmp");
			M_DrawTransPic (cx, cy, p);
		}
		p = Draw_CachePic ("gfx/box_bm.lmp");
		M_DrawTransPic (cx, cy+8, p);
		width -= 2;
		cx += 16;
	}

	// draw right side
	cy = y;
	p = Draw_CachePic ("gfx/box_tr.lmp");
	M_DrawTransPic (cx, cy, p);
	p = Draw_CachePic ("gfx/box_mr.lmp");
	for (n = 0; n < lines; n++)
	{
		cy += 8;
		M_DrawTransPic (cx, cy, p);
	}
	p = Draw_CachePic ("gfx/box_br.lmp");
	M_DrawTransPic (cx, cy+8, p);
}

//=============================================================================

int m_save_demonum;

/*
================
M_ToggleMenu_f
================
*/
void M_ToggleMenu_f (void)
{
	m_entersound = true;

	if (key_dest == key_menu)
	{
		if (m_state != m_main)
		{
			M_Menu_Main_f ();
			return;
		}
		key_dest = key_game;
		m_state = m_none;
		return;
	}
	if (key_dest == key_console)
	{
		Con_ToggleConsole_f ();
	}
	else
	{
		if(sv.active)	//Enter pause menu
		{
			m_state = m_pause;
			key_dest = key_menu;
		}
		else
		{
			M_Menu_Main_f ();
		}
	}
}


int mvs(int cursorpos)
{
	return cursorpos*MVS;
}

//=============================================================================
/* MAIN MENU */


#define	MAIN_ITEMS	5
#define MAIN_MULTI_ITEMS 2

int	m_main_cursor;
int m_main_multi_cursor;
bool main_multi = false;
float main_percentwidth = 0.3;
float main_x_offset_percent = 0.1;

#define CURSOR_HEIGHT MVS

//main
#define main_pixel_height (MAIN_ITEMS + 1) * MVS
#define main_pixel_width PixWidth(main_percentwidth)
#define main_y_offset_pixels PixHeight(1)-main_pixel_height
#define main_x_offset_pixels PixWidth(main_x_offset_percent)

//main-multi submenu
#define multi_pixel_height (MAIN_MULTI_ITEMS) * MVS
#define multi_pixel_width 11*CHARZ

#define multi_x_offset_pixels main_x_offset_pixels+main_pixel_width


void M_Menu_Main_f (void)
{
	if (key_dest != key_menu)
	{
		m_save_demonum = cls.demonum;
		cls.demonum = -1;
	}
	key_dest = key_menu;
	m_state = m_main;
	m_entersound = true;
	main_multi = false;
}

void M_Main_Multi_Draw (void)
{
	//background
	Draw_Fill(multi_x_offset_pixels, main_y_offset_pixels, multi_pixel_width, multi_pixel_height, GREY);
	//cursor
	Draw_Fill(multi_x_offset_pixels, main_y_offset_pixels+mvs(m_main_multi_cursor), multi_pixel_width, CURSOR_HEIGHT, YELLOW);
	//menu items
	M_PrintWhite(multi_x_offset_pixels+TEXT_XMARGIN, main_y_offset_pixels+TEXT_YMARGIN+mvs(0), "Create");
	M_PrintWhite(multi_x_offset_pixels+TEXT_XMARGIN, main_y_offset_pixels+TEXT_YMARGIN+mvs(1), "Join");
}

void M_Main_Draw (void)
{
	int cursor_color = main_multi ? GREY : YELLOW;

	//background
	Draw_Fill(main_x_offset_pixels, main_y_offset_pixels, main_pixel_width, main_pixel_height, BG_COLOR); //blue bg
	Draw_Fill(main_x_offset_pixels, PixHeight(1)-MVS, main_pixel_width, MVS, BLACK); //black bar bottom
	M_PrintWhite(main_x_offset_pixels+main_pixel_width-(9*8), PixHeight(1)-MVS+TEXT_YMARGIN, "X Select"); //tip
	//cursor
	Draw_Fill(main_x_offset_pixels, main_y_offset_pixels+mvs(m_main_cursor), main_pixel_width, CURSOR_HEIGHT, cursor_color);
	//menu items
	M_PrintWhite(main_x_offset_pixels+TEXT_XMARGIN, main_y_offset_pixels+TEXT_YMARGIN+mvs(0), "Matchmaking");
	M_PrintWhite(main_x_offset_pixels+TEXT_XMARGIN, main_y_offset_pixels+TEXT_YMARGIN+mvs(1), "Firefight");
	M_PrintWhite(main_x_offset_pixels+TEXT_XMARGIN, main_y_offset_pixels+TEXT_YMARGIN+mvs(2), "Options");
	M_PrintWhite(main_x_offset_pixels+TEXT_XMARGIN, main_y_offset_pixels+TEXT_YMARGIN+mvs(3), "Spartan");
	M_PrintWhite(main_x_offset_pixels+TEXT_XMARGIN, main_y_offset_pixels+TEXT_YMARGIN+mvs(4), "Quit");

	if (main_multi)
		M_Main_Multi_Draw ();
}

void M_Main_Key (int key)
{
	switch (key)
	{
	case K_TRIANGLE:
		if (main_multi == true)
		{
			main_multi = false;
		}
		else
		{
			m_main_cursor = 4;
			M_Menu_Quit_f ();
		}
		break;
	case K_DOWNARROW:
		S_LocalSound ("misc/menuoption.wav");
		if (main_multi)
		{
			if (++m_main_multi_cursor >= MAIN_MULTI_ITEMS)
				m_main_multi_cursor = 0;
		}
		else
		{
			if (++m_main_cursor >= MAIN_ITEMS)
				m_main_cursor = 0;
		}
		
		break;

	case K_UPARROW:
		S_LocalSound ("misc/menuoption.wav");
		if (main_multi)
		{
			if (--m_main_multi_cursor < 0)
				m_main_multi_cursor = MAIN_MULTI_ITEMS - 1;
		}
		else
		{
			if (--m_main_cursor < 0)
				m_main_cursor = MAIN_ITEMS - 1;
		}
		break;

	case K_CROSS:
		if (m_main_cursor != 0) //submenu
		{
			S_LocalSound ("misc/menuoption.wav");
		}
		else
		{
			m_entersound = true;
		}

		if (main_multi)
		{
			switch (m_main_multi_cursor)
			{
			case 0:
				M_Matchmaking_f ();
				break;
			case 1:
				m_multiplayer_cursor = 0;
				M_Menu_Net_f ();
				break;
			}
		}
		else
		{
			switch (m_main_cursor)
			{
			case 0:	
				main_multi = true;
				break;
			case 1:
				M_Menu_Firefight_f ();
				break;

			case 2:
				M_Menu_Options_f ();
				break;

			case 3:
				M_Menu_Setup_f ();
				break;

			case 4:
				M_Menu_Quit_f ();
				break;
			}
		}
	}
}

//=============================================================================
/* SINGLE PLAYER MENU */

int	m_singleplayer_cursor;
#define	SINGLEPLAYER_ITEMS	3


void M_Menu_SinglePlayer_f (void)
{
	key_dest = key_menu;
	m_state = m_singleplayer;
	m_entersound = true;
}


void M_SinglePlayer_Draw (void)
{
	int		f;
	qpic_t	*p;

	M_DrawTransPic (16, 4, Draw_CachePic ("gfx/qplaque.lmp") );
	p = Draw_CachePic ("gfx/ttl_sgl.lmp");
	M_DrawPic ( (320-p->width)/2, 4, p);
	M_DrawTransPic (72, 32, Draw_CachePic ("gfx/sp_menu.lmp") );

	f = (int)(host_time * 10)%6;

	M_DrawTransPic (54, 32 + m_singleplayer_cursor * 20,Draw_CachePic( va("gfx/menudot%i.lmp", f+1 ) ) );
}


void M_SinglePlayer_Key (int key)
{
	switch (key)
	{
	case K_ENTER:
	case K_START:
	case K_TRIANGLE:
		M_Menu_Main_f ();
		break;

	case K_DOWNARROW:
		S_LocalSound ("misc/menuoption.wav");
		if (++m_singleplayer_cursor >= SINGLEPLAYER_ITEMS)
			m_singleplayer_cursor = 0;
		break;

	case K_UPARROW:
		S_LocalSound ("misc/menuoption.wav");
		if (--m_singleplayer_cursor < 0)
			m_singleplayer_cursor = SINGLEPLAYER_ITEMS - 1;
		break;

	case K_CROSS: // Cross
		m_entersound = true;

		switch (m_singleplayer_cursor)
		{
		case 0:
			key_dest = key_game;
			Cbuf_AddText ("disconnect\n");	// Ch0wW: Disconnect all the time to reset original NetQuake behaviour.
			Cbuf_AddText ("maxplayers 1\n");
			Cbuf_AddText ("map start\n");
			break;

		case 1:
			M_Menu_Load_f ();
			break;

		case 2:
			M_Menu_Save_f ();
			break;
		}

	}
}

//=============================================================================
/* LOAD/SAVE MENU */

int		load_cursor;		// 0 < load_cursor < MAX_SAVEGAMES

#define	MAX_SAVEGAMES		12
char	m_filenames[MAX_SAVEGAMES][SAVEGAME_COMMENT_LENGTH+1];
int		loadable[MAX_SAVEGAMES];

void M_ScanSaves (void)
{
	int		i, j;
	char	name[MAX_OSPATH];
	FILE	*f;
	int		version;

	for (i=0 ; i<MAX_SAVEGAMES ; i++)
	{
		strcpy (m_filenames[i], "--- UNUSED SLOT ---");
		loadable[i] = false;
		sprintf (name, "%s/s%i.sav", com_gamedir, i);
		f = fopen (name, "r");
		if (!f)
			continue;
		fscanf (f, "%i\n", &version);
		fscanf (f, "%79s\n", name);
		strncpy (m_filenames[i], name, sizeof(m_filenames[i])-1);

	// change _ back to space
		for (j=0 ; j<SAVEGAME_COMMENT_LENGTH ; j++)
			if (m_filenames[i][j] == '_')
				m_filenames[i][j] = ' ';
		loadable[i] = true;
		fclose (f);
	}
}

void M_Menu_Load_f (void)
{
	m_entersound = true;
	m_state = m_load;
	key_dest = key_menu;
	M_ScanSaves ();
}


void M_Menu_Save_f (void)
{
	if (!sv.active)
		return;
	if (cl.intermission)
		return;
	if (svs.maxclients != 1)
		return;
	m_entersound = true;
	m_state = m_save;
	key_dest = key_menu;
	M_ScanSaves ();
}


void M_Load_Draw (void)
{
	int		i;
	qpic_t	*p;

	p = Draw_CachePic ("gfx/p_load.lmp");
	M_DrawPic ( (320-p->width)/2, 4, p);

	for (i=0 ; i< MAX_SAVEGAMES; i++)
		M_Print (16, 32 + 8*i, m_filenames[i]);

// line cursor
	M_DrawCharacter (8, 32 + load_cursor*8, 12+((int)(realtime*4)&1));
}


void M_Save_Draw (void)
{
	int		i;
	qpic_t	*p;

	p = Draw_CachePic ("gfx/p_save.lmp");
	M_DrawPic ( (320-p->width)/2, 4, p);

	for (i=0 ; i<MAX_SAVEGAMES ; i++)
		M_Print (16, 32 + 8*i, m_filenames[i]);

// line cursor
	M_DrawCharacter (8, 32 + load_cursor*8, 12+((int)(realtime*4)&1));
}


void M_Load_Key (int k)
{
	switch (k)
	{
	case K_ENTER:
	case K_START:
	case K_TRIANGLE:
		M_Menu_SinglePlayer_f ();
		break;

	case K_CROSS: // Cross
		S_LocalSound ("misc/menuenter.wav");
		if (!loadable[load_cursor])
			return;
		m_state = m_none;
		key_dest = key_game;

	// Host_Loadgame_f can't bring up the loading plaque because too much
	// stack space has been used, so do it now
		SCR_BeginLoadingPlaque ();

	// issue the load command
		Cbuf_AddText (va ("load s%i\n", load_cursor) );
		return;

	case K_UPARROW:
	case K_LEFTARROW:
		S_LocalSound ("misc/menuoption.wav");
		load_cursor--;
		if (load_cursor < 0)
			load_cursor = MAX_SAVEGAMES-1;
		break;

	case K_DOWNARROW:
	case K_RIGHTARROW:
		S_LocalSound ("misc/menuoption.wav");
		load_cursor++;
		if (load_cursor >= MAX_SAVEGAMES)
			load_cursor = 0;
		break;
	}
}


void M_Save_Key (int k)
{
	switch (k)
	{
	case K_ENTER:
	case K_START:
	case K_TRIANGLE:
		M_Menu_SinglePlayer_f ();
		break;

	case K_CROSS:
		m_state = m_none;
		key_dest = key_game;
		Cbuf_AddText (va("save s%i\n", load_cursor));
		return;

	case K_UPARROW:
	case K_LEFTARROW:
		S_LocalSound ("misc/menuoption.wav");
		load_cursor--;
		if (load_cursor < 0)
			load_cursor = MAX_SAVEGAMES-1;
		break;

	case K_DOWNARROW:
	case K_RIGHTARROW:
		S_LocalSound ("misc/menuoption.wav");
		load_cursor++;
		if (load_cursor >= MAX_SAVEGAMES)
			load_cursor = 0;
		break;
	}
}

//=============================================================================
/* MULTIPLAYER MENU */

#define	MULTIPLAYER_ITEMS	2


void M_Menu_MultiPlayer_f (void)
{
	key_dest = key_menu;
	m_state = m_multiplayer;
	m_entersound = true;
}


void M_MultiPlayer_Draw (void)
{
	int		f;
	qpic_t	*p;
    int y_offset = 30;
	int y_cursor_offset = 32;
	int x_offset = 60;
	int x_text_offset = 15;
	
	//title and background
	M_DrawTransPic (16, 4, Draw_CachePic ("gfx/qplaque.lmp") );
	p = Draw_CachePic ("gfx/p_multi.lmp");
	M_DrawPic ( (320-p->width)/2, 4, p);
	// M_DrawTransPic (72, 32, Draw_CachePic ("gfx/mp_menu.lmp") );

	//menu items
	M_Print(x_offset+x_text_offset, y_offset+32, "Join Game");
	M_Print(x_offset+x_text_offset, y_offset+52, "Start Game");
	//M_Print(x_offset+x_text_offset, y_offset+72, "Edit Spartan");
	//cursor
	f = (int)(host_time * 10)%6;
	M_DrawTransPic (x_offset, y_offset+y_cursor_offset+(m_multiplayer_cursor*20), Draw_CachePic(va("gfx/menudot%i.lmp", f+1)));

	if (tcpipAvailable)
		return;
	M_PrintWhite ((320/2) - ((27*8)/2), 148, "No Communications Available");
}


void M_MultiPlayer_Key (int key)
{
	switch (key)
	{
	case K_ENTER:
	case K_START:
	case K_TRIANGLE:
		M_Matchmaking_f ();
		break;

	case K_DOWNARROW:
		S_LocalSound ("misc/menuoption.wav");
		if (++m_multiplayer_cursor >= MULTIPLAYER_ITEMS)
			m_multiplayer_cursor = 0;
		break;

	case K_UPARROW:
		S_LocalSound ("misc/menuoption.wav");
		if (--m_multiplayer_cursor < 0)
			m_multiplayer_cursor = MULTIPLAYER_ITEMS - 1;
		break;

	case K_CROSS: // Cross
		m_entersound = true;
		switch (m_multiplayer_cursor)
		{
		case 0:
			if (tcpipAvailable)
				M_Menu_Net_f ();
			break;

		case 1:
			if (tcpipAvailable)
				M_Menu_Net_f ();
			break;

		case 2:
			M_Menu_Setup_f ();
			break;
		}

	}
}

//=============================================================================
/* BENCHMARK MENU */

void M_Menu_Benchmark_f (void)
{
	key_dest = key_menu;
	m_state = m_benchmark;
	m_entersound = true;
}

extern int max_fps;
extern int min_fps;
extern int average_fps;
void M_Benchmark_Draw (void)
{
	char s[80],s1[80],s2[80];
	sprintf(s, "    Max FPS: %3d", max_fps);
	sprintf(s1, "    Min FPS: %3d", min_fps);
	sprintf(s2, "Average FPS: %3d", average_fps);
	M_Print(30, 20, "Benchmark results");
	M_Print (64, 40, s);
	M_Print (64, 48, s1);
	M_Print (64, 56, s2);
}


void M_Benchmark_Key (int key)
{
	switch (key)
	{
	case K_ENTER:
	case K_START:
	case K_TRIANGLE:
	case K_CIRCLE: // Circle
	case K_CROSS: // Cross
		M_Menu_Options_f ();
		break;
	}
}

//=============================================================================
/* SETUP MENU */

int		setup_cursor = 40;
int		setup_cursor_table[] = {40, 56, 80, 124};

char	setup_hostname[16];
char	setup_myname[16];
int		setup_oldtop;
int		setup_oldbottom;
int		setup_top;
int		setup_bottom;

#define	NUM_SETUP_CMDS	4

void M_Menu_Setup_f (void)
{
	key_dest = key_menu;
	m_state = m_setup;
	m_entersound = true;
	strcpy(setup_myname, cl_name.string);
	strcpy(setup_hostname, hostname.string);
	setup_top = setup_oldtop = ((int)cl_color.value) >> 4;
	setup_bottom = setup_oldbottom = ((int)cl_color.value) & 15;
}


void M_Setup_Draw (void)
{
	qpic_t	*p;
	int xoff = 60;
	int yoff = 70;
	int tbcolor = BG_COLOR-3;

	M_DrawTransPic (16, yoff+4, Draw_CachePic ("gfx/qplaque.lmp") );

	Draw_CenterWindow(0.66, 0.5, "Spartan Personalization");

	M_Print (xoff+64, yoff+40, "Hostname");
	//M_DrawTextBox (xoff+176, yoff+32, 16, 1);
	Draw_Fill(xoff+176,yoff+38, 16*8, 12, tbcolor);
	M_PrintWhite (xoff+184, yoff+40, setup_hostname);

	M_Print (xoff+64, yoff+56, "Spartan name");
	//M_DrawTextBox (xoff+176, yoff+48, 16, 1);
	Draw_Fill(xoff+176,yoff+54, 16*8, 12, tbcolor);
	M_PrintWhite (xoff+184, yoff+56, setup_myname);

    M_Print (xoff+64, yoff+80, "Spartan color");
    M_DrawColorBar (xoff+64, yoff+88, setup_bottom);

	M_PrintWhite (xoff+64, yoff+124, "Accept");

	M_DrawCharacter (xoff+56, yoff+setup_cursor_table [setup_cursor], 12+((int)(realtime*4)&1));

	if (setup_cursor == 0)
		M_DrawCharacter (xoff+184 + 8*strlen(setup_hostname), yoff+setup_cursor_table [setup_cursor], 10+((int)(realtime*4)&1));

	if (setup_cursor == 1)
		M_DrawCharacter (xoff+184 + 8*strlen(setup_myname), yoff+setup_cursor_table [setup_cursor], 10+((int)(realtime*4)&1));
}


void M_Setup_Key (int k)
{
	int			l;

	switch (k)
	{
	case K_ENTER:
	case K_START:
	case K_TRIANGLE:
		M_Menu_Main_f ();
		break;

	case K_UPARROW:
		S_LocalSound ("misc/menuoption.wav");
		setup_cursor--;
		if (setup_cursor < 0)
			setup_cursor = NUM_SETUP_CMDS-1;
		break;

	case K_DOWNARROW:
		S_LocalSound ("misc/menuoption.wav");
		setup_cursor++;
		if (setup_cursor >= NUM_SETUP_CMDS)
			setup_cursor = 0;
		break;

	case K_LEFTARROW:
		if (setup_cursor < 2)
			return;
		S_LocalSound ("misc/menuoption.wav");
		if (setup_cursor == 2)
			setup_bottom = setup_bottom - 1;
		break;
	case K_RIGHTARROW:
		if (setup_cursor < 2)
			return;
forward:
		S_LocalSound ("misc/menuoption.wav");
		if (setup_cursor == 2)
			setup_bottom = setup_bottom + 1;
		break;

	case K_CROSS: // Cross
		if (setup_cursor == 0 || setup_cursor == 1 || setup_cursor == 2)
			return;

		// setup_cursor == 3 (OK)
		if (strcmp(cl_name.string, setup_myname) != 0)
			Cbuf_AddText ( va ("name \"%s\"\n", setup_myname) );
		if (strcmp(hostname.string, setup_hostname) != 0)
			Cvar_Set("hostname", setup_hostname);
		if (setup_top != setup_oldtop || setup_bottom != setup_oldbottom)
			Cbuf_AddText( va ("color %i %i\n", setup_top, setup_bottom) );
		m_entersound = true;
		M_Menu_Main_f ();
		break;

	case K_BACKSPACE:
		if (setup_cursor == 0)
		{
			if (strlen(setup_hostname))
				setup_hostname[strlen(setup_hostname)-1] = 0;
		}

		if (setup_cursor == 1)
		{
			if (strlen(setup_myname))
				setup_myname[strlen(setup_myname)-1] = 0;
		}
		break;

	default:
		if (k < 32 || k > 127)
			break;
		if (setup_cursor == 0)
		{
			l = strlen(setup_hostname);
			if (l < 15)
			{
				setup_hostname[l+1] = 0;
				setup_hostname[l] = k;
			}
		}
		if (setup_cursor == 1)
		{
			l = strlen(setup_myname);
			if (l < 15)
			{
				setup_myname[l+1] = 0;
				setup_myname[l] = k;
			}
		}
	}

	if (setup_top > 13)
		setup_top = 0;
	if (setup_top < 0)
		setup_top = 13;
	if (setup_bottom > 13)
		setup_bottom = 0;
	if (setup_bottom < 0)
		setup_bottom = 13;
}

//=============================================================================
/* NET MENU */

int	m_net_cursor;
int m_net_items;
int m_net_saveHeight;
int m_net_prevstate;

char *net_helpMessage [] =
{
/* .........1.........2.... */
  " Commonly used to play  ",
  " over the Internet, but ",
  " also used on a Local   ",
  " Area Network.          "
};

void M_Menu_Net_f (void)
{
	key_dest = key_menu;
	m_net_prevstate = m_state;
	m_state = m_net;
	m_entersound = true;
	m_net_items = 1;

	if (m_net_cursor >= m_net_items)
		m_net_cursor = 0;
	m_net_cursor--;
	M_Net_Key (K_DOWNARROW);
}


void M_Net_Draw (void)
{
	int		f;
	qpic_t	*p;

	M_DrawTransPic (16, 4, Draw_CachePic ("gfx/qplaque.lmp") );
	p = Draw_CachePic ("gfx/p_multi.lmp");
	M_DrawPic ( (320-p->width)/2, 4, p);

	f = 32;

	if (tcpipAvailable)
		p = Draw_CachePic ("gfx/netmen4.lmp");
	else
		p = Draw_CachePic ("gfx/dim_tcp.lmp");
	M_DrawTransPic (72, f, p);

	f = (320-26*8)/2;
	M_DrawTextBox (f, 134, 24, 4);
	f += 8;
	M_Print (f, 166, net_helpMessage[m_net_cursor*4+0]);

	f = (int)(host_time * 10)%6;
	M_DrawTransPic (54, 32 + m_net_cursor * 20,Draw_CachePic( va("gfx/menudot%i.lmp", f+1 ) ) );
}


void M_Net_Key (int k)
{
	M_Menu_LanConfig_f();
}

//=============================================================================
/* MODS MENU */

int		mods_cursor;

void M_Menu_Mods_f (void)
{
	key_dest = key_menu;
	m_state = m_mods;
	m_entersound = true;
}

ModsList* cur = NULL;

void M_Mods_Draw (void)
{
	float		r;
	qpic_t	*p;

	M_DrawTransPic (16, 4, Draw_CachePic ("gfx/qplaque.lmp") );

	M_Print (60, 10, "Select the mod to load");

	ModsList* ptr = mods;
	int j = 0;
	while (ptr != NULL){
		M_Print (60, 32 + j * 8, ptr->name);
		if (j == mods_cursor) cur = ptr;
		ptr = ptr->next;
		j++;
	}

	// cursor
	M_DrawCharacter (50, 32 + mods_cursor*8, 12+((int)(realtime*4)&1));
}


void M_Mods_Key (int k)
{
	switch (k)
	{
	case K_ENTER:
	case K_START:
	case K_TRIANGLE:
		M_Menu_Options_f ();
		break;

	case K_CROSS:
		m_entersound = true;
		char cmd[128];
		sprintf(cmd, "game %s\n", cur->name);
		Cbuf_AddText (cmd);
		M_Menu_Main_f ();
		return;

	case K_UPARROW:
		S_LocalSound ("misc/menuoption.wav");
		mods_cursor--;
		if (mods_cursor < 0)
			mods_cursor = max_mod_idx;
		break;

	case K_DOWNARROW:
		S_LocalSound ("misc/menuoption.wav");
		mods_cursor++;
		if (mods_cursor > max_mod_idx)
			mods_cursor = 0;
		break;

	}

	if (mods_cursor > max_mod_idx)
	{
		if (k == K_UPARROW)
			mods_cursor = max_mod_idx;
		else
			mods_cursor = 0;
	}

}

//=============================================================================
/* GRAPHICS MENU */

# define GRAPHICS_ITEMS 14

#define	SLIDER_RANGE 10

int	graphics_cursor;

int w_res[] = {480, 640, 720, 960};
int h_res[] = {272, 368, 408, 544};
int r_idx = -1;

void M_Menu_Graphics_f (void)
{
	key_dest = key_menu;
	m_state = m_graphics;
	m_entersound = true;
}

void M_AdjustSliders2 (int dir)
{
	S_LocalSound ("misc/menuoption.wav");
	
	switch (graphics_cursor)
	{
	case 0:  // bilinear filtering
		Cvar_ToggleValue(&gl_bilinear);
		break;
	case 1:	// gamma
		v_gamma.value -= dir * 0.05;
		if (v_gamma.value < 0.5)
			v_gamma.value = 0.5;
		if (v_gamma.value > 1)
			v_gamma.value = 1;
		Cvar_SetValue ("v_gamma", v_gamma.value);
		break;
	case 2: // overbright
		gl_overbright.value += dir;
		if (gl_overbright.value < 0)
			gl_overbright.value = 0;
		if (gl_overbright.value > 2)
			gl_overbright.value = 2;
		Cvar_SetValue ("gl_overbright", gl_overbright.value);
		break;
	case 3: // hud transparency
		scr_sbaralpha.value += dir * 0.1;
		if (scr_sbaralpha.value < 0)
			scr_sbaralpha.value = 0;
		if (scr_sbaralpha.value > 1)
			scr_sbaralpha.value = 1;
		Cvar_SetValue ("scr_sbaralpha", scr_sbaralpha.value);
		break;
	case 4:  // mirrors opacity
		r_mirroralpha.value += dir * 0.1;
		if (r_mirroralpha.value < 0)
			r_mirroralpha.value = 0;
		if (r_mirroralpha.value > 1)
			r_mirroralpha.value = 1;
		Cvar_SetValue ("r_mirroralpha", r_mirroralpha.value);
		break;
	case 5: // water opacity
		r_wateralpha.value += dir * 0.1;
		if (r_wateralpha.value < 0)
			r_wateralpha.value = 0;
		if (r_wateralpha.value > 1)
			r_wateralpha.value = 1;
		Cvar_SetValue ("r_wateralpha", r_wateralpha.value);
		break;
	case 6:	// dynamic torchflares
		Cvar_ToggleValue(&gl_torchflares);
		break;
	case 7:	// dynamic shadows
		Cvar_ToggleValue(&r_shadows);
		break;
	case 8:	// Fog
		if (gl_fog.value) Cvar_SetValue ("r_fullbright", 0);
		else Cvar_SetValue ("r_fullbright", 1);
		Cvar_ToggleValue (&gl_fog);
		break;
	case 9:  // cel shading
		gl_outline.value += dir;
		if (gl_outline.value > 6) gl_outline.value = 6;
		else if (gl_outline.value < 0) gl_outline.value = 0;
		Cvar_SetValue ("gl_outline",gl_outline.value);
		break;
	case 10:  // anaglyph 3d
		st_separation.value = st_separation.value == 0.5 ? 0.0 : 0.5;
		Cvar_SetValue ("st_separation",st_separation.value);
		break;
	case 11:	// antialiasing
		antialiasing += dir;
		if (antialiasing < 0) antialiasing = 8;
		else if (antialiasing > 8) antialiasing = 0;
		SetAntiAliasing(antialiasing);
		break;
	case 12:	// resolution
		if (r_idx == -1) {
			for (r_idx = 0; r_idx < 4; r_idx++) {
				if (cfg_width == w_res[r_idx]) break;
			}
		}
		r_idx += dir;
		if (r_idx > 3) r_idx = 0;
		else if (r_idx < 0) r_idx = 3;
		SetResolution(w_res[r_idx], h_res[r_idx]);
		break;
	case 13:
		Cvar_SetValue ("vid_vsync", !vid_vsync.value);
		break;
	case 14:	// performance test
		key_dest = key_benchmark;
		m_state = m_none;
		cls.demonum = m_save_demonum;
		Cbuf_AddText("benchmark demo1\n");
		break;
	default:
		break;
	}
}

void M_DrawSlider (int x, int y, float range)
{
	int	i;

	if (range < 0)
		range = 0;
	if (range > 1)
		range = 1;
	M_DrawCharacter (x-8, y, 128);
	for (i=0 ; i<SLIDER_RANGE ; i++)
		M_DrawCharacter (x + i*8, y, 129);
	M_DrawCharacter (x+i*8, y, 130);
	M_DrawCharacter (x + (SLIDER_RANGE-1)*8 * range, y, 131);
}

void M_DrawCheckbox (int x, int y, int on)
{
	M_Print (x, y, on ? "on" : "off");
}

void M_Graphics_Draw (void)
{
	float		r;
	qpic_t	*p;
	int xoff = 60;
	int yoff = 20;

	M_DrawTransPic (xoff+16, 4, Draw_CachePic ("gfx/qplaque.lmp") );
	// p = Draw_CachePic ("gfx/p_option.lmp");
	// M_DrawPic ( (320*MENU_SCALE-p->width)/2, 4, p);

	Draw_CenterWindow(0.8,0.8, "Graphics Options");
	
	M_Print (xoff+16, yoff+32, "    Bilinear Filtering");
	M_DrawCheckbox (xoff+220, yoff+32, gl_bilinear.value);
	
	M_Print (xoff+16, yoff+40, "            Brightness");
	r = (1.0 - v_gamma.value) / 0.5;
	M_DrawSlider (xoff+220, yoff+40, r);
	
	M_Print (xoff+16, yoff+48, "      Light Overbright");
	r = gl_overbright.value / 2;
	M_DrawSlider (xoff+220, yoff+48, r);
	
	M_Print (xoff+16, yoff+56, "      HUD Transparency");
	M_DrawSlider (xoff+220, yoff+56, scr_sbaralpha.value);
	
	M_Print (xoff+16, yoff+64, "       Mirrors Opacity");
	r = r_mirroralpha.value;
	M_DrawSlider (xoff+220, yoff+64, r);
	
	M_Print (xoff+16, yoff+72, "         Water Opacity");
	r = r_wateralpha.value;
	M_DrawSlider (xoff+220, yoff+72, r);
	
	M_Print (xoff+16, yoff+80, "        Dynamic Lights");
	M_DrawCheckbox (xoff+220, yoff+80, gl_torchflares.value);

	M_Print (xoff+16, yoff+88, "       Dynamic Shadows");
	M_DrawCheckbox (xoff+220, yoff+88, r_shadows.value);
	
	M_Print (xoff+16, yoff+96, "         Fog Rendering");
	M_DrawCheckbox (xoff+220, yoff+96, gl_fog.value);
	
	M_Print (xoff+16, yoff+104, "           Cel Shading");
	r = gl_outline.value / 6;
	M_DrawSlider (xoff+220, yoff+104, r);
	
	M_Print (xoff+16, yoff+112,"           Anaglyph 3D");
	M_DrawCheckbox (xoff+220, yoff+112, st_separation.value != 0);

	M_Print (xoff+16, yoff+120,"         Anti-Aliasing");
	switch (antialiasing) {
	case 1:
		M_Print (xoff+220, yoff+120, "MSAA 2x");
		break;
	case 2:
		M_Print (xoff+220, yoff+120, "MSAA 4x");
		break;
	case 3:
		M_Print (xoff+220, yoff+120, "SSAA 2x");
		break;
	case 4:
		M_Print (xoff+220, yoff+120, "SSAA 4x");
		break;
	case 5:
		M_Print (xoff+220, yoff+120, "MSAA 2x + SSAA 2x");
		break;
	case 6:
		M_Print (xoff+220, yoff+120, "MSAA 2x + SSAA 4x");
		break;
	case 7:
		M_Print (xoff+220, yoff+120, "MSAA 4x + SSAA 2x");
		break;
	case 8:
		M_Print (xoff+220, yoff+120, "MSAA 4x + SSAA 4x");
		break;
	default:
		M_Print (xoff+220, yoff+120, "Disabled");
		break;
	}
	
	char res_str[64];
	sprintf(res_str, "%dx%d", cfg_width, cfg_height);
	M_Print (xoff+16, yoff+128,"            Resolution");
	M_Print (xoff+220, yoff+128, res_str);

	M_Print (xoff+16, yoff+136,"                V-Sync");
	M_DrawCheckbox (xoff+220, yoff+136, vid_vsync.value);
	
	M_Print (xoff+16, yoff+152,"      Test Performance");
	
	// Warn users for reboot required
	if (graphics_cursor == 11 || graphics_cursor == 12) {
		M_PrintCentered (210, "Editing this option will require");
		M_PrintCentered (218, "  an app reboot to take effect  ");
	}
	
// cursor
	if (graphics_cursor == GRAPHICS_ITEMS) M_DrawCharacter (xoff+200, yoff+152, 12+((int)(realtime*4)&1));
	else M_DrawCharacter (xoff+200, yoff+32 + graphics_cursor*8, 12+((int)(realtime*4)&1));
}

void M_Graphics_Key (int k)
{
	switch (k)
	{
	case K_ENTER:
	case K_START:
	case K_TRIANGLE:
		M_Menu_Options_f ();
		break;

	case K_CROSS:
		m_entersound = true;
		M_AdjustSliders2 (1);
		return;

	case K_UPARROW:
		S_LocalSound ("misc/menuoption.wav");
		graphics_cursor--;
		if (graphics_cursor < 0)
			graphics_cursor = GRAPHICS_ITEMS;
		break;

	case K_DOWNARROW:
		S_LocalSound ("misc/menuoption.wav");
		graphics_cursor++;
		if (graphics_cursor > GRAPHICS_ITEMS)
			graphics_cursor = 0;
		break;

	case K_LEFTARROW:
		M_AdjustSliders2 (-1);
		break;

	case K_RIGHTARROW:
		M_AdjustSliders2 (1);
		break;
	}
}

//=============================================================================
/* OPTIONS MENU */

#define	OPTIONS_ITEMS 23

int	options_cursor;

void M_Menu_Options_f (void)
{
	key_dest = key_menu;
	m_state = m_options;
	m_entersound = true;
}

char *w_pos[] = {"Center", "Right", "Left", "Hidden"};
int w_pos_idx = -1;

void M_AdjustSliders (int dir)
{
	S_LocalSound ("misc/menuoption.wav");

	switch (options_cursor)
	{
	case 5:	// screen size
		viewsize.value += dir * 10;
		if (viewsize.value < 30)
			viewsize.value = 30;
		if (viewsize.value > 120)
			viewsize.value = 120;
		Cvar_SetValue ("viewsize", viewsize.value);
		break;
	case 6:	// camera sensitivity
		sensitivity.value += dir * 0.5;
		if (sensitivity.value < 1)
			sensitivity.value = 1;
		if (sensitivity.value > 11)
			sensitivity.value = 11;
		Cvar_SetValue ("sensitivity", sensitivity.value);
		break;
	case 7:	// invert camera
		Cvar_ToggleValue (&invert_camera);
		break;
	case 8:	// music volume
		bgmvolume.value += dir * 0.1;
		if (bgmvolume.value < 0)
			bgmvolume.value = 0;
		if (bgmvolume.value > 1)
			bgmvolume.value = 1;
		Cvar_SetValue ("bgmvolume", bgmvolume.value);
		break;
	case 9:	// sfx volume
		volume.value += dir * 0.1;
		if (volume.value < 0)
			volume.value = 0;
		if (volume.value > 1)
			volume.value = 1;
		Cvar_SetValue ("volume", volume.value);
		break;
	case 10:	// retrotouch
		Cvar_SetValue ("retrotouch", !retrotouch.value);
		break;
	case 11:	// motion camera
		Cvar_SetValue ("motioncam", !motioncam.value);
		break;
	case 12:	// motion camera sensitivity horizontal
		motion_horizontal_sensitivity.value += dir * 0.5;
		if (motion_horizontal_sensitivity.value < 0)
			motion_horizontal_sensitivity.value = 0;
		if (motion_horizontal_sensitivity.value > 10)
			motion_horizontal_sensitivity.value = 10;
		Cvar_SetValue ("motion_horizontal_sensitivity", motion_horizontal_sensitivity.value);
		break;
	case 13:	// motion camera sensitivity vertical
		motion_vertical_sensitivity.value += dir * 0.5;
		if (motion_vertical_sensitivity.value < 0)
			motion_vertical_sensitivity.value = 0;
		if (motion_vertical_sensitivity.value > 10)
			motion_vertical_sensitivity.value = 10;
		Cvar_SetValue ("motion_vertical_sensitivity", motion_vertical_sensitivity.value);
		break;
	case 14:	// rumble
		Cvar_SetValue ("pstv_rumble", !pstv_rumble.value);
		break;
	case 15:	// show fps
		Cvar_SetValue ("show_fps", !show_fps.value);
		break;
	case 16:	// crosshair
		crosshair.value += dir;
		if (crosshair.value > 2) crosshair.value = 0;
		else if (crosshair.value < 0) crosshair.value = 2;
		Cvar_SetValue ("crosshair", crosshair.value);
		break;
	case 17:	// show weapon
		w_pos_idx += dir;
		if (w_pos_idx > 3) w_pos_idx = 0;
		if (w_pos_idx < 0) w_pos_idx = 3;
		switch (w_pos_idx) {
			case 1:
				Cvar_SetValue ("r_drawviewmodel", 1);
				Cvar_SetValue ("r_viewmodeloffset", 8);
				break;
			case 2:
				Cvar_SetValue ("r_drawviewmodel", 1);
				Cvar_SetValue ("r_viewmodeloffset", -8);
				break;
			case 3:
				Cvar_SetValue ("r_drawviewmodel", 0);
				break;
			default:
				Cvar_SetValue ("r_drawviewmodel", 1);
				Cvar_SetValue ("r_viewmodeloffset", 0);
				break;
		}
		break;
	case 18:	// field of view
		fov.value += dir * 5;
		if (fov.value > 130) fov.value = 130;
		else if (fov.value < 75) fov.value = 75;
		Cvar_SetValue ("fov",fov.value);
		break;
	case 19:	// smooth animations
		Cvar_ToggleValue(&r_interpolate_model_animation);
		Cvar_ToggleValue(&r_interpolate_model_transform);
		break;
	case 20:	// specular mode
		Cvar_SetValue ("gl_xflip", !gl_xflip.value);
		break;
	case 21: //touchscreen x value
		psvita_front_sensitivity_x.value += dir * 0.05;
		if (psvita_front_sensitivity_x.value > 1)
			psvita_front_sensitivity_x.value = 1;
		else if (psvita_front_sensitivity_x.value < 0)
			psvita_front_sensitivity_x.value = 0;
		Cvar_SetValue ("psvita_front_sensitivity_x", psvita_front_sensitivity_x.value);
		break;
	case 22: //touchscreen y value
		psvita_front_sensitivity_y.value += dir * 0.05;
		if (psvita_front_sensitivity_y.value > 1)
			psvita_front_sensitivity_y.value = 1;
		else if (psvita_front_sensitivity_y.value < 0)
			psvita_front_sensitivity_y.value = 0;
		Cvar_SetValue ("psvita_front_sensitivity_y", psvita_front_sensitivity_y.value);
		break;
	case 23: //enable touchscreen aiming
		Cvar_SetValue ("psvita_touchmode", !psvita_touchmode.value);
		break;
	default:
		break;
	}
}

void M_Options_Draw (void)
{
	float		r;
	qpic_t	*p;
	int xoff = 60;
	int yoff = 20;


	M_DrawTransPic (xoff+16, 4, Draw_CachePic ("gfx/qplaque.lmp") );

	Draw_CenterWindow(0.8,0.8, "General Options");

	// p = Draw_CachePic ("gfx/p_option.lmp");
	// M_DrawPic ( (320*MENU_SCALE-p->width)/2, 4, p);

	M_Print (xoff+16, yoff+32, "     Controls Settings");
	M_Print (xoff+16, yoff+40, "     Graphics Settings");
	M_Print (xoff+16, yoff+48, "          Open Console");
	M_Print (xoff+16, yoff+56, "        Open Mods Menu");
	M_Print (xoff+16, yoff+64, "     Reset to defaults");

	M_Print (xoff+16, yoff+72, "           Screen size");
	r = (viewsize.value - 30) / (120 - 30);
	M_DrawSlider (xoff+220, yoff+72, r);
	
	M_Print (xoff+16, yoff+80, "    Camera Sensitivity");
	r = (sensitivity.value - 1)/10;
	M_DrawSlider (xoff+220, yoff+80, r);
	
	M_Print (xoff+16, yoff+88, "         Invert Camera");
	M_DrawCheckbox (xoff+220, yoff+88, invert_camera.value);

	M_Print (xoff+16, yoff+96,"          Music Volume");
	r = bgmvolume.value;
	M_DrawSlider (xoff+220, yoff+96, r);
	
	M_Print (xoff+16, yoff+104,"          Sound Volume");
	r = volume.value;
	M_DrawSlider (xoff+220, yoff+104, r);
	
	M_Print (xoff+16, yoff+112,"        Use Retrotouch");
	M_DrawCheckbox (xoff+220, yoff+112, retrotouch.value);

	M_Print (xoff+16, yoff+120,"         Use Gyroscope");
	M_DrawCheckbox (xoff+220, yoff+120, motioncam.value);
	
	M_Print (xoff+16, yoff+128,"    Gyro X Sensitivity");
	r = motion_horizontal_sensitivity.value/10;
	M_DrawSlider (xoff+220, yoff+128, r);

	M_Print (xoff+16, yoff+136,"    Gyro Y Sensitivity");
	r = motion_vertical_sensitivity.value/10;
	M_DrawSlider (xoff+220, yoff+136, r);

	M_Print (xoff+16, yoff+144,"         Rumble Effect");
	M_DrawCheckbox (xoff+220, yoff+144, pstv_rumble.value);
	
	M_Print (xoff+16, yoff+152,"        Show Framerate");
	M_DrawCheckbox (xoff+220, yoff+152, show_fps.value);
	
	M_Print (xoff+16, yoff+160,"        Show Crosshair");
	if (crosshair.value == 0) M_Print (xoff+220, yoff+160, "Off");
	else if (crosshair.value == 1) M_Print (xoff+220, yoff+160, "Original");
	else M_Print (xoff+220, yoff+160, "Custom");
	
	M_Print (xoff+16, yoff+168,"       Weapon Position");
	if (w_pos_idx == -1) {
		if (!r_drawviewmodel.value) w_pos_idx = 3;
		else if (r_viewmodeloffset.value < 0) w_pos_idx = 2;
		else if (r_viewmodeloffset.value > 0) w_pos_idx = 1;
		else w_pos_idx = 0;
	}
	M_Print (xoff+220, yoff+168, w_pos[w_pos_idx]);
	
	M_Print (xoff+16, yoff+176,"         Field of View");
	r = (fov.value - 75) / 55;
	M_DrawSlider (xoff+220, yoff+176, r);
	
	M_Print (xoff+16, yoff+184,"     Smooth Animations");
	M_DrawCheckbox (xoff+220, yoff+184, r_interpolate_model_animation.value);
	
	M_Print (xoff+16, yoff+192,"         Specular Mode");
	M_DrawCheckbox (xoff+220, yoff+192, gl_xflip.value);

	M_Print (xoff+16, yoff+200,"   Touch X Sensitivity");
	r = psvita_front_sensitivity_x.value;
	M_DrawSlider (xoff+220, yoff+200, r);

	M_Print (xoff+16, yoff+208,"   Touch Y Sensitivity");
	r = psvita_front_sensitivity_y.value;
	M_DrawSlider (xoff+220, yoff+208, r);

	M_Print (xoff+16, yoff+216,"   Touch Screen Aiming");
	M_DrawCheckbox (xoff+220, yoff+216, psvita_touchmode.value);

	M_DrawCharacter (xoff+200, yoff+32+options_cursor*8, 12+((int)(realtime*4)&1));
}


void M_Options_Key (int k)
{
	switch (k)
	{
	case K_ENTER:
	case K_START:
	case K_TRIANGLE:
		if (sv.active) { //user is in pause menu
			M_Menu_Pause_f();
		} else {
			M_Menu_Main_f ();
		}
		break;

	case K_CROSS:
		m_entersound = true;
		switch (options_cursor)
		{
		case 0: // Controls Settings
			M_Menu_Keys_f ();
			break;
		case 1: // Graphics Settings
			M_Menu_Graphics_f ();
			break;
		case 2: // Open Console
			m_state = m_none;
			Con_ToggleConsole_f ();
			break;
		case 3: // Open Mods Menu
			M_Menu_Mods_f ();
			break;
		case 4: // Reset to defaults
			Cbuf_AddText ("exec default.cfg\n");
			IN_ResetInputs();
			viewsize.value = 120;
			v_gamma.value = 1;
			sensitivity.value = 3;
			invert_camera.value = 0;
			bgmvolume.value = 1.0;
			volume.value = 0.7f;
			retrotouch.value = 0;
			pstv_rumble.value = 1.0f;
			show_fps.value = 0;
			r_drawviewmodel.value = 1;
			crosshair.value = 0;
			fov.value = 90;
			gl_fog.value = 0;
			gl_torchflares.value = 1;
			r_shadows.value = 1;
			r_interpolate_model_animation.value = 1;
			r_interpolate_model_transform.value = 1;
			r_mirroralpha.value = 0.8f;
			r_wateralpha.value = 1.0f;
			gl_xflip.value = 0;
			motioncam.value = 0;
			vid_vsync.value = 1;
			motion_horizontal_sensitivity.value = 3;
			motion_vertical_sensitivity.value = 3;
			psvita_front_sensitivity_x.value = 0.25;
			psvita_front_sensitivity_y.value = 0.25;
			psvita_touchmode.value = 1;
			scr_sbaralpha.value = 0.5f;
			gl_outline.value = 0;
			st_separation.value = 0;
			w_pos_idx = 0;
			r_viewmodeloffset.value = 0;
			gl_overbright.value = 0;
			Cvar_SetValue ("viewsize", viewsize.value);
			Cvar_SetValue ("v_gamma", v_gamma.value);
			Cvar_SetValue ("sensitivity", sensitivity.value);
			Cvar_SetValue ("invert_camera", invert_camera.value);
			Cvar_SetValue ("bgmvolume", bgmvolume.value);
			Cvar_SetValue ("volume", volume.value);
			Cvar_SetValue ("retrotouch", retrotouch.value);
			Cvar_SetValue ("pstv_rumble", pstv_rumble.value);
			Cvar_SetValue ("show_fps", show_fps.value);
			Cvar_SetValue ("r_drawviewmodel", r_drawviewmodel.value);
			Cvar_SetValue ("crosshair", crosshair.value);
			Cvar_SetValue ("fov", fov.value);
			Cvar_SetValue ("gl_fog", gl_fog.value);
			Cvar_SetValue ("gl_torchflares", gl_torchflares.value);
			Cvar_SetValue ("r_shadows", r_shadows.value);
			Cvar_SetValue ("r_interpolate_model_animation", r_interpolate_model_animation.value);
			Cvar_SetValue ("r_interpolate_model_transform", r_interpolate_model_transform.value);
			Cvar_SetValue ("r_mirroralpha", r_mirroralpha.value);
			Cvar_SetValue ("r_wateralpha", r_wateralpha.value);
			Cvar_SetValue ("gl_xflip", gl_xflip.value);
			Cvar_SetValue ("motioncam", motioncam.value);
			Cvar_SetValue ("motion_horizontal_sensitivity", motion_horizontal_sensitivity.value);
			Cvar_SetValue ("motion_vertical_sensitivity", motion_vertical_sensitivity.value);
			Cvar_SetValue ("psvita_front_sensitivity_x", psvita_front_sensitivity_x.value);
			Cvar_SetValue ("psvita_front_sensitivity_y", psvita_front_sensitivity_y.value);
			Cvar_SetValue ("psvita_touchmode", psvita_touchmode.value);
			Cvar_SetValue ("scr_sbaralpha", scr_sbaralpha.value);
			Cvar_SetValue ("vid_vsync", vid_vsync.value);
			Cvar_SetValue ("gl_outline", gl_outline.value);
			Cvar_SetValue ("r_viewmodeloffset", r_viewmodeloffset.value);
			Cvar_SetValue ("st_separation",st_separation.value);
			Cvar_SetValue ("gl_overbright",gl_overbright.value);
			SetResolution(960, 544);
			antialiasing = 2;
			r_idx = -1;
			SetAntiAliasing(antialiasing);
            Cbuf_AddText ("gl_texturemode GL_LINEAR\n");
			break;
		default: // All other settings
			M_AdjustSliders (1);
			break;
		}
		return;

	case K_UPARROW:
		S_LocalSound ("misc/menuoption.wav");
		options_cursor--;
		if (options_cursor < 0)
			options_cursor = OPTIONS_ITEMS;
		break;

	case K_DOWNARROW:
		S_LocalSound ("misc/menuoption.wav");
		options_cursor++;
		if (options_cursor > OPTIONS_ITEMS)
			options_cursor = 0;
		break;

	case K_LEFTARROW:
		M_AdjustSliders (-1);
		break;

	case K_RIGHTARROW:
		M_AdjustSliders (1);
		break;
	}
}

//=============================================================================
/* KEYS MENU */

#define BIND_BINDABLE 0
#define BIND_SEPARATOR 1

char *bindnames[][2] =
{
	{"+attack", 		"Attack"},
	{"impulse 10", 		"Cycle weapons"},
	{"impulse 13", 		"Pickup / Reload"},
	{"impulse 27", 		"Change Grenade"},
	{"impulse 28", 		"Throw Grenade"},
	{"impulse 29", 		"Melee"},
	{"impulse 11", 		"Zoom"},
	{"+jump", 			"Jump / Swim up"},
	{"+forward", 		"Forward"},
	{"+back", 			"Back"},
	{"+moveleft", 		"Left" },
	{"+moveright", 		"Right" }
};

#define	NUMCOMMANDS	(sizeof(bindnames)/sizeof(bindnames[0]))

int		keys_cursor;
int		bind_grab;

void M_Menu_Keys_f (void)
{
	key_dest = key_menu;
	m_state = m_keys;
	m_entersound = true;
}


void M_FindKeysForCommand (char *command, int *twokeys)
{
	int		count;
	int		j;
	int		l;
	char	*b;

	twokeys[0] = twokeys[1] = -1;
	l = strlen(command);
	count = 0;

	for (j=0 ; j<256 ; j++)
	{
		b = keybindings[j];
		if (!b)
			continue;
		if (!strncmp (b, command, l) )
		{
			twokeys[count] = j;
			count++;
			if (count == 2)
				break;
		}
	}
}

void M_UnbindCommand (char *command)
{
	int		j;
	int		l;
	char	*b;

	l = strlen(command);

	for (j=0 ; j<256 ; j++)
	{
		b = keybindings[j];
		if (!b)
			continue;
		if (!strncmp (b, command, l) )
			Key_SetBinding (j, "");
	}
}


void M_Keys_Draw (void)
{
	int		i, l;
	int		keys[2];
	char	*name;
	int		x, y;
	int xoff = 60;
	int yoff = 40;
	qpic_t	*p;

	// p = Draw_CachePic ("gfx/ttl_cstm.lmp");
	// M_DrawPic ( (320*MENU_SCALE-p->width)/2, 4, p);
	Draw_CenterWindow(0.8,0.8, "Control Options");

	if (bind_grab)
		M_Print (xoff+12, 32, "Press a key or button for this action");
	else
		M_Print (xoff+18, 32, "Cross to change, Select to clear");

// search for known bindings
	for (i=0 ; i<NUMCOMMANDS ; i++)
	{
		y = 48 + 8*i;

		M_Print (xoff+16, yoff+y, bindnames[i][1]);

		l = strlen (bindnames[i][0]);

		M_FindKeysForCommand (bindnames[i][0], keys);

		if (keys[0] == -1)
		{
			M_Print (xoff+140, yoff+y, "???");
		}
		else
		{
			name = Key_KeynumToString (keys[0]);
			M_Print (xoff+140, yoff+y, name);
			x = strlen(name) * 8;
			if (keys[1] != -1)
			{
				M_Print (xoff+140 + x + 8, yoff+y, "or");
				M_Print (xoff+140 + x + 32, yoff+y, Key_KeynumToString (keys[1]));
			}
		}
	}

	if (bind_grab)
		M_DrawCharacter (xoff+130, yoff+48 + keys_cursor*8, '=');
	else
		M_DrawCharacter (xoff+130, yoff+48 + keys_cursor*8, 12+((int)(realtime*4)&1));
}


void M_Keys_Key (int k)
{
	char	cmd[80];
	int		keys[2];

	if (bind_grab)
	{	// defining a key
		S_LocalSound ("misc/menuoption.wav");
		if (k == K_START) // Start cancels
		{
			bind_grab = false;
		}
		else if (k != '`')
		{
			sprintf (cmd, "bind \"%s\" \"%s\"\n", Key_KeynumToString (k), bindnames[keys_cursor][0]);
			Cbuf_InsertText (cmd);
		}

		bind_grab = false;
		return;
	}

	switch (k)
	{
	case K_ENTER:
	case K_START:
	case K_TRIANGLE:
		M_Menu_Options_f ();
		break;

	case K_LEFTARROW:
	case K_UPARROW:
		S_LocalSound ("misc/menuoption.wav");
		keys_cursor--;
		if (keys_cursor < 0)
			keys_cursor = NUMCOMMANDS-1;
		break;

	case K_DOWNARROW:
	case K_RIGHTARROW:
		S_LocalSound ("misc/menuoption.wav");
		keys_cursor++;
		if (keys_cursor >= NUMCOMMANDS)
			keys_cursor = 0;
		break;

	case K_CROSS:		// go into bind mode
		M_FindKeysForCommand (bindnames[keys_cursor][0], keys);
		S_LocalSound ("misc/menuenter.wav");
		if (keys[1] != -1)
			M_UnbindCommand (bindnames[keys_cursor][0]);
		bind_grab = true;
		break;

	case K_SELECT:				// delete bindings
		S_LocalSound ("misc/menuenter.wav");
		M_UnbindCommand (bindnames[keys_cursor][0]);
		break;
	}
}

//=============================================================================
/* VIDEO MENU */

void M_Menu_Video_f (void)
{
	key_dest = key_menu;
	m_state = m_video;
	m_entersound = true;
}


void M_Video_Draw (void)
{
	(*vid_menudrawfn) ();
}


void M_Video_Key (int key)
{
	(*vid_menukeyfn) (key);
}

//=============================================================================
/* HELP MENU */

int		help_page;
#define	NUM_HELP_PAGES	6


void M_Menu_Help_f (void)
{
	key_dest = key_menu;
	m_state = m_help;
	m_entersound = true;
	help_page = 0;
}



void M_Help_Draw (void)
{
	M_DrawPic (0, 0, Draw_CachePic ( va("gfx/help%i.lmp", help_page)) );
}


void M_Help_Key (int key)
{
	switch (key)
	{
	case K_ENTER:
	case K_START:
	case K_TRIANGLE:
		M_Menu_Main_f ();
		break;

	case K_UPARROW:
	case K_RIGHTARROW:
		m_entersound = true;
		if (++help_page >= NUM_HELP_PAGES)
			help_page = 0;
		break;

	case K_DOWNARROW:
	case K_LEFTARROW:
		m_entersound = true;
		if (--help_page < 0)
			help_page = NUM_HELP_PAGES-1;
		break;
	}

}

//=============================================================================
/* QUIT MENU */

int		msgNumber;
int		m_quit_prevstate;
bool	wasInMenus;

#ifndef	_WIN32
char *quitMessage [] =
{
/* .........1.........2.... */
  "    Ready to retire     ",
  "         Chief?         ",
  "                        ",
  "   O NO         X YES   ",

  " Milord, methinks that  ",
  "   thou art a lowly     ",
  " quitter. Is this true? ",
  "                        ",

  " Do I need to bust your ",
  "  face open for trying  ",
  "        to quit?        ",
  "                        ",

  " Man, I oughta smack you",
  "   for trying to quit!  ",
  "     Press X to get     ",
  "      smacked out.      ",

  " What, you want to stop ",
  "   playing VitaQuake?   ",
  "     Press X or O to    ",
  "   return to LiveArea.  ",

  " Press X to quit like a ",
  "   big loser in life.   ",
  "  Return back to stay   ",
  "  proud and successful! ",

  "   If you press X to    ",
  "  quit, I will summon   ",
  "  Satan all over your   ",
  "      memory card!      ",

  "  Um, Asmodeus dislikes ",
  " his children trying to ",
  " quit. Press X to return",
  "   to your Tinkertoys.  ",

  "  If you quit now, I'll ",
  "  throw a blanket-party ",
  "   for you next time!   ",
  "                        "
};
#endif

void M_Menu_Quit_f (void)
{
	if (m_state == m_quit)
		return;
	wasInMenus = (key_dest == key_menu);
	key_dest = key_menu;
	m_quit_prevstate = m_state;
	m_state = m_quit;
	m_entersound = true;
	msgNumber = 0; //rand()&7;
}


void M_Quit_Key (int key)
{
	switch (key)
	{
	case K_TRIANGLE:
	case K_CIRCLE:
	case K_SELECT:
	case 'n':
	case 'N':
		if (wasInMenus)
		{
			m_state = m_quit_prevstate;
			m_entersound = true;
		}
		else
		{
			key_dest = key_game;
			m_state = m_none;
		}
		break;

	case K_CROSS:
		key_dest = key_console;
		Host_Quit_f ();
		break;

	default:
		break;
	}

}


void M_Quit_Draw (void)
{
	if (wasInMenus)
	{
		m_state = m_quit_prevstate;
		m_recursiveDraw = true;
		M_Draw ();
		m_state = m_quit;
	}
	int xoff = 80;
	int yoff = 20;

	Draw_OffCenterWindow(0, -20, 0.5, 0.2, "Quit");
	M_Print (xoff+64, yoff+84,  quitMessage[msgNumber*4+0]);
	M_Print (xoff+64, yoff+92,  quitMessage[msgNumber*4+1]);
	M_Print (xoff+64, yoff+100, quitMessage[msgNumber*4+2]);
	M_PrintWhite (xoff+64, yoff+116, quitMessage[msgNumber*4+3]);
}


//=============================================================================
/* LAN CONFIG MENU */

int		lanConfig_cursor = 2;
#define NUM_LANCONFIG_CMDS	4
#define LAN_HEIGHT_P 0.5
#define LAN_WIDTH_P 0.55
#define LAN_HEIGHT PixHeight(LAN_HEIGHT_P)
#define LAN_WIDTH PixWidth(0.66)

int 	lanConfig_port;
char	lanConfig_portname[6];
char	lanConfig_joinname[22];

//int m_lan_prevstate;

void M_Menu_LanConfig_f (void)
{
	key_dest = key_menu;
	//m_lan_prevstate = m_state;
	m_state = m_lanconfig;
	m_entersound = true;
	if (lanConfig_cursor == -1)
	{
		if (JoiningGame && TCPIPConfig)
			lanConfig_cursor = NUM_LANCONFIG_CMDS;
	}
	if (StartingGame && lanConfig_cursor >= 3)
		lanConfig_cursor = 1;
	lanConfig_port = DEFAULTnet_hostport;
	sprintf(lanConfig_portname, "%u", lanConfig_port);

	m_return_onerror = false;
	m_return_reason[0] = 0;
	mm_rentry = true;
}

char protocol[64];
uint8_t proto_idx = 0;

void M_LanConfig_Draw (void)
{
	int LAN_YOFF = (PixHeight(1)-LAN_HEIGHT-MVS)/2 - 20;
	int		lanConfig_cursor_table [] = {LAN_YOFF+72, LAN_YOFF+92, LAN_YOFF+112, LAN_YOFF+144, LAN_YOFF+158};
	// qpic_t	*p;
	int		basex;
	char	*startJoin;
	
	if (key_dest == key_menu)
	{
		m_state = m_net_prevstate;
		m_recursiveDraw = true;
		M_Draw ();
		m_state = m_lanconfig;
	}
	if (StartingGame)
		startJoin = "New Game";
	else
		startJoin = "Join Game";

	if (proto_idx == 0)
		sprintf(protocol, "TCP/IP");
	else
		sprintf(protocol, "AdHoc");

	M_DrawTransPic (16, 4, Draw_CachePic ("gfx/qplaque.lmp") );
	Draw_CenterWindow(LAN_WIDTH_P, LAN_HEIGHT_P, va ("%s - %s", startJoin, protocol));
	
	// p = Draw_CachePic ("gfx/p_multi.lmp");
	basex = (320*MENU_SCALE-LAN_WIDTH)/2 + 30;
	// M_DrawPic (basex, 4, p);
	
	// M_PrintWhite (basex+9*CHARZ, LAN_YOFF+32, );
	basex += 8;

	M_PrintWhite (basex, LAN_YOFF+52, "Address:");
	M_PrintWhite (basex+9*CHARZ, LAN_YOFF+52, my_tcpip_address);

	M_PrintWhite (basex, lanConfig_cursor_table[0], "Port");
	M_DrawTextBox (basex+8*8, lanConfig_cursor_table[0]-8, 6, 1);
	M_Print (basex+9*CHARZ, lanConfig_cursor_table[0], lanConfig_portname);
	M_PrintWhite (basex, lanConfig_cursor_table[1], "Protocol");
	M_Print (basex+9*CHARZ, lanConfig_cursor_table[1], protocol);
	if (JoiningGame)
	{
		M_Print (basex, lanConfig_cursor_table[2], "Search for local games...");
		M_Print (basex, LAN_YOFF+128, "Join game at:");
		M_DrawTextBox (basex+8, lanConfig_cursor_table[3]-8, 22, 1);
		M_Print (basex+16, lanConfig_cursor_table[3], lanConfig_joinname);
		M_Print (basex, LAN_YOFF+158, "Join an online server");
	}
	else
	{
		//M_DrawTextBox (basex, lanConfig_cursor_table[2]-8, 2, 1);
		M_PrintWhite (basex+8, lanConfig_cursor_table[2], "Accept");
	}

	M_DrawCharacter (basex-8, lanConfig_cursor_table [lanConfig_cursor], 12+((int)(realtime*4)&1));

	if (lanConfig_cursor == 0)
		M_DrawCharacter (basex+9*CHARZ + 8*strlen(lanConfig_portname), lanConfig_cursor_table [0], 10+((int)(realtime*4)&1));

	if (lanConfig_cursor == 3)
		M_DrawCharacter (basex+16 + 8*strlen(lanConfig_joinname), lanConfig_cursor_table [3], 10+((int)(realtime*4)&1));

	if (*m_return_reason)
		M_PrintWhite (basex, LAN_YOFF+168, m_return_reason);
}


void M_LanConfig_Key (int key)
{
	int		l;

	switch (key)
	{
	case K_ENTER:
	case K_START:
	case K_TRIANGLE:
		if (m_multiplayer_cursor == 0)
		{
			M_Menu_Main_f ();
		}
		else
		{
			M_Matchmaking_f ();
		}
		break;

	case K_UPARROW:
		S_LocalSound ("misc/menuoption.wav");
		lanConfig_cursor--;
		if (lanConfig_cursor < 0)
			if (JoiningGame)
				lanConfig_cursor = NUM_LANCONFIG_CMDS;
			else
				lanConfig_cursor = NUM_LANCONFIG_CMDS-1;
		break;

	case K_DOWNARROW:
		S_LocalSound ("misc/menuoption.wav");
		lanConfig_cursor++;
		if (lanConfig_cursor > NUM_LANCONFIG_CMDS)
			lanConfig_cursor = 0;
		break;

	case K_CROSS:
		if (lanConfig_cursor == 0)
			break;

		m_entersound = true;

		M_ConfigureNetSubsystem ();
		
		if (lanConfig_cursor == 1)
		{
			Datagram_Shutdown();
			proto_idx = (proto_idx + 1) % 2;
			if (proto_idx == 1) { // Setup AdHoc
				// Start sceNetAdhoc and sceNetAdhocctl
				sceNetAdhocInit();
				SceNetAdhocctlAdhocId adhocId;
				memset(&adhocId, 0, sizeof(SceNetAdhocctlAdhocId));
				adhocId.type = SCE_NET_ADHOCCTL_ADHOCTYPE_RESERVED;
				memcpy(&adhocId.data[0], "SOLI00001", SCE_NET_ADHOCCTL_ADHOCID_LEN);
				sceNetAdhocctlInit(&adhocId);
	
				SceNetCheckDialogParam param;
				sceNetCheckDialogParamInit(&param);
				SceNetAdhocctlGroupName groupName;
				memset(groupName.data, 0, SCE_NET_ADHOCCTL_GROUPNAME_LEN);
				param.groupName = &groupName;
				memcpy(&param.npCommunicationId.data, "SOLI00001", 9);
				param.npCommunicationId.term = '\0';
				param.npCommunicationId.num = 0;
				param.mode = SCE_NETCHECK_DIALOG_MODE_PSP_ADHOC_CONN;
				param.timeoutUs = 0;
	
				int res = sceNetCheckDialogInit(&param);
				if (res >= 0) {
					netcheck_dialog_running = 1;
				}
			} else Datagram_Init();
		}

		if (lanConfig_cursor == 2)
		{
			if (StartingGame)
			{
				M_Matchmaking_f ();
				break;
			}
			M_Menu_Search_f();
			break;
		}

		if (lanConfig_cursor == 3)
		{
			m_return_state = m_state;
			m_return_onerror = true;
			key_dest = key_game;
			m_state = m_none;
			Cbuf_AddText ( va ("connect \"%s\"\n", lanConfig_joinname) );
			break;
		}

		if (lanConfig_cursor == 4)
		{
			M_Menu_OnlineServerList_f ();
			break;
		}

		break;

	case K_BACKSPACE:
		if (lanConfig_cursor == 0)
		{
			if (strlen(lanConfig_portname))
				lanConfig_portname[strlen(lanConfig_portname)-1] = 0;
		}

		if (lanConfig_cursor == 3)
		{
			if (strlen(lanConfig_joinname))
				lanConfig_joinname[strlen(lanConfig_joinname)-1] = 0;
		}
		break;

	default:
		if (key < 32 || key > 127)
			break;

		if (lanConfig_cursor == 3)
		{
			l = strlen(lanConfig_joinname);
			if (l < 21)
			{
				lanConfig_joinname[l+1] = 0;
				lanConfig_joinname[l] = key;
			}
		}

		if (key < '0' || key > '9')
			break;
		if (lanConfig_cursor == 0)
		{
			l = strlen(lanConfig_portname);
			if (l < 5)
			{
				lanConfig_portname[l+1] = 0;
				lanConfig_portname[l] = key;
			}
		}
	}

	if (StartingGame && lanConfig_cursor == 3)
		if (key == K_UPARROW)
			lanConfig_cursor = 2;
		else
			lanConfig_cursor = 0;

	l =  atoi(lanConfig_portname);
	if (l > 65535)
		l = lanConfig_port;
	else
		lanConfig_port = l;
	sprintf(lanConfig_portname, "%u", lanConfig_port);
}

//=============================================================================
/* ONLINE SERVERLIST MENU */

int		onlineServerList_cursor = 0;

void M_Menu_OnlineServerList_f (void)
{
	key_dest = key_menu;
	m_state = m_onlineserverlist;
	m_entersound = true;
	m_return_onerror = false;
	m_return_reason[0] = 0;
}

#define NUM_SERVERS 5

void M_OnlineServerList_Draw (void)
{
	qpic_t	*p;
	int		basex;

	M_DrawTransPic (16, 4, Draw_CachePic ("gfx/qplaque.lmp") );
	p = Draw_CachePic ("gfx/p_multi.lmp");
	basex = (320-p->width)/2;
	M_DrawPic (basex, 4, p);

	M_Print (basex, 32, "EU Official Server (Shareware Only)");
	M_Print (basex, 40, "EU Official Server (Deatmatch Maps)");
	M_Print (basex, 48, "NCTech Spaceball1 Server");
	M_Print (basex, 56, "Shmack Practice Mode Server");
	M_Print (basex, 64, "Clan HDZ DM Server");

	M_DrawCharacter (basex-8, 32+onlineServerList_cursor*8, 12+((int)(realtime*4)&1));

	if (*m_return_reason)
		M_PrintWhite (basex, 148, m_return_reason);
}


void M_OnlineServerList_Key (int key)
{
	int		l;

	switch (key)
	{
	case K_ENTER:
	case K_START:
	case K_TRIANGLE:
		M_Menu_MultiPlayer_f ();
		break;

	case K_UPARROW:
		S_LocalSound ("misc/menuoption.wav");
		onlineServerList_cursor--;
		if (onlineServerList_cursor < 0)
			onlineServerList_cursor = NUM_SERVERS-1;
		break;

	case K_DOWNARROW:
		S_LocalSound ("misc/menuoption.wav");
		onlineServerList_cursor++;
		if (onlineServerList_cursor >= NUM_SERVERS)
			onlineServerList_cursor = 0;
		break;

	case K_CROSS:
		m_return_state = m_state;
		m_return_onerror = true;
		key_dest = key_game;
		m_state = m_none;
		Cbuf_AddText ("stopdemo\n");

		if (onlineServerList_cursor == 0) Cbuf_AddText ("connect 212.24.100.151\n");
		if (onlineServerList_cursor == 1) Cbuf_AddText ("connect 212.24.100.151:27000\n");
		if (onlineServerList_cursor == 2) Cbuf_AddText ("connect quake.nctech.ca\n");
		if (onlineServerList_cursor == 3) Cbuf_AddText ("connect quake.shmack.net\n");
		if (onlineServerList_cursor == 4) Cbuf_AddText ("connect dm.clanhdz.com\n");

		break;
	}
}

//=============================================================================
/* GAME OPTIONS MENU */

typedef struct
{
	char	*name;
	char	*description;
} level_t;

level_t		levels[] =
{
	{"citadel", "Citadel"}, //0
	{"narsp", "Narrows"},
	{"Longest", "Longest"},
	{"chill", "Chill Out"},
	{"pri2", "Prisoner"},
	{"base", "Minibase"},
	{"plaza", "Plaza"},
	{"spider", "Spiderweb"},
	{"bloody", "Blood Gutch"},
	{"lockout", "Lockout"},
	{"lock", "Lockout2"},

	{"construction", "Construction"}, //11
	{"fire", "Fire!"} //12
};



typedef struct
{
	char	*description;
	int		firstLevel;
	int		levels;
} episode_t;

episode_t	episodes[] =
{
	{"Slayer Maps", 0, 11}
};

episode_t	episodes_ff[] =
{
	{"Firefight Maps", 11, 2}
};


void M_Menu_Pause_f (void)
{
	key_dest = key_menu;
	m_state = m_pause;
	m_entersound = true;
}

#define	NUM_PAUSEOPTIONS	4

#define P_HEIGHT 			mvs(4)
#define P_YOFF				PixHeight(0.3)

#define P_WIDTH 			PixWidth(0.4)
#define P_XOFF				(PixWidth(1)-P_WIDTH)/2

int		pause_cursor;

//pause
void M_Pause_Draw (void)
{	

	//background
	Draw_WindowPix(P_XOFF, P_YOFF, P_WIDTH, P_HEIGHT, "Pause");
	//cursor
	Draw_Fill(P_XOFF, P_YOFF+mvs(pause_cursor), P_WIDTH, MVS, YELLOW);
	//menu items
	M_Print(P_XOFF+TEXT_XMARGIN, P_YOFF+mvs(0)+TEXT_YMARGIN, "Add Bot");
	M_Print(P_XOFF+TEXT_XMARGIN, P_YOFF+mvs(1)+TEXT_YMARGIN, "Remove Bot");
	M_Print(P_XOFF+TEXT_XMARGIN, P_YOFF+mvs(2)+TEXT_YMARGIN, "Options");
	M_Print(P_XOFF+TEXT_XMARGIN, P_YOFF+mvs(3)+TEXT_YMARGIN, "Disconnect");
}

void M_Pause_Key (int key)
{
	switch (key)
	{
	case K_ENTER:
	case K_START:
	case K_TRIANGLE:
		key_dest = key_game;
		m_state = m_none;
		break;

	case K_UPARROW:
		S_LocalSound ("misc/menuoption.wav");
		pause_cursor--;
		if (pause_cursor < 0)
			pause_cursor = NUM_PAUSEOPTIONS-1;
		break;

	case K_DOWNARROW:
		S_LocalSound ("misc/menuoption.wav");
		pause_cursor++;
		if (pause_cursor >= NUM_PAUSEOPTIONS)
			pause_cursor = 0;
		break;

	case K_CROSS:
		S_LocalSound ("misc/menuenter.wav");
		if(pause_cursor == 0)
		{
			Cbuf_AddText ("impulse 100\n");
		}
		if(pause_cursor == 1)
		{
			Cbuf_AddText ("impulse 102\n");
		}
		if(pause_cursor == 2)
		{
			M_Menu_Options_f ();
		}
		if(pause_cursor == 3)
		{
			Cbuf_AddText ("disconnect\n");
			M_Menu_Main_f();
			//S_StopAllSounds(true);
			//Cbuf_AddText  ("play music/Solitude_MainTheme_Low.wav");
		}
		break;
	}
}

int	startepisode;
int	startlevel;
int maxplayers;

void M_Matchmaking_f (void)
{
	key_dest = key_menu;
	m_state = m_gameoptions;
	m_entersound = true;
	if (maxplayers == 0)
		maxplayers = svs.maxclients;
	if (maxplayers < 2)
		maxplayers = svs.maxclientslimit;
	//default MP settings
	Cvar_SetValue ("skill", 0);
	Cvar_SetValue ("deathmatch", 1);
	startepisode = 0;
	if (!mm_rentry)
		startlevel = rand() % episodes[startepisode].levels;
	mm_rentry = false;
}


int gameoptions_cursor_table[] = {40, 56, 64, 72, 80, 88, 96, 112, 120};
#define	NUM_GAMEOPTIONS	7

#define MM_WIDTH_PERCENT 0.4
#define MM_WIDTH_PIX PixWidth(MM_WIDTH_PERCENT)
#define MM_HEIGHT_PERCENT 1
#define MM_HEIGHT_PIX PixHeight(MM_HEIGHT_PERCENT)

#define MM_GAMEOPS_HEIGHT mvs(7)
#define MM_HEADER_HEIGHT mvs(3)
#define MM_FOOTER_HEIGHT mvs(1)

#define MM_XOFF 0

#define MM_BG BG_COLOR
#define MM_FG BG_COLOR + 1

#define MM_GAMEOPS_OPT_XOFF TEXT_XMARGIN + 8 * CHARZ

int		gameoptions_cursor = 6;


void M_Matchmaking_Draw (void)
{
	//background
	Draw_Fill(MM_XOFF, 0, MM_WIDTH_PIX, MM_HEIGHT_PIX, MM_BG);
	Draw_Fill(MM_XOFF, MM_HEADER_HEIGHT, MM_WIDTH_PIX, MM_GAMEOPS_HEIGHT, MM_FG);
	Draw_Fill(MM_XOFF, MM_HEIGHT_PIX-mvs(1), MM_WIDTH_PIX, MM_FOOTER_HEIGHT, BLACK);
	Draw_Fill(MM_XOFF+MM_WIDTH_PIX, 0, 1, MM_HEIGHT_PIX, BLACK+1);
	//cursor
	Draw_Fill(MM_XOFF, MM_HEADER_HEIGHT+mvs(gameoptions_cursor), MM_WIDTH_PIX, MVS, YELLOW);

	//header
	M_PrintWhite (MM_XOFF+TEXT_XMARGIN, MM_HEADER_HEIGHT-mvs(1)+TEXT_YMARGIN, "MATCHMAKING");
	//======================== 0
		//force update protocol char[]
		if (proto_idx == 0) sprintf(protocol, "TCP/IP");
		else sprintf(protocol, "AdHoc");
	M_Print (MM_XOFF+TEXT_XMARGIN, MM_HEADER_HEIGHT+TEXT_YMARGIN+mvs(0), "NETWORK");
		M_PrintWhite (MM_XOFF+TEXT_XMARGIN+MM_GAMEOPS_OPT_XOFF, MM_HEADER_HEIGHT+TEXT_YMARGIN+mvs(0), protocol);
	//======================== 1
	M_Print (MM_XOFF+TEXT_XMARGIN, MM_HEADER_HEIGHT+TEXT_YMARGIN+mvs(1), "GAME");
		switch((int)deathmatch.value)
		{
			case 0:
				M_PrintWhite  (MM_XOFF+TEXT_XMARGIN+MM_GAMEOPS_OPT_XOFF, MM_HEADER_HEIGHT+TEXT_YMARGIN+mvs(1), "Co-op");
				break;
			case 1:
				M_PrintWhite (MM_XOFF+TEXT_XMARGIN+MM_GAMEOPS_OPT_XOFF, MM_HEADER_HEIGHT+TEXT_YMARGIN+mvs(1), "Slayer");
				if((int)deathmatch.value > 1000)
					Cvar_SetValue ("fraglimit", 50);
				Cvar_SetValue ("deathmatch", 1);
				Cbuf_AddText ("deathmatch 1\n");
				Cvar_SetValue ("coop", 0);
				Cbuf_AddText ("coop 0\n");
				Cvar_SetValue ("teamplay", 0);
				break;
			case 3:
				M_PrintWhite (MM_XOFF+TEXT_XMARGIN+MM_GAMEOPS_OPT_XOFF, MM_HEADER_HEIGHT+TEXT_YMARGIN+mvs(1), "Swat");
				if((int)deathmatch.value > 1000)
					Cvar_SetValue ("fraglimit", 50);
				Cvar_SetValue ("deathmatch", 3);
				Cbuf_AddText ("deathmatch 3\n");
				Cvar_SetValue ("coop", 0);
				Cvar_SetValue ("teamplay", 0);
				break;
		}
	//======================== 2
	M_Print (MM_XOFF+TEXT_XMARGIN, MM_HEADER_HEIGHT+TEXT_YMARGIN+mvs(2), "KILLS");
		if (fraglimit.value == 0)
			M_PrintWhite (MM_XOFF+TEXT_XMARGIN+MM_GAMEOPS_OPT_XOFF, MM_HEADER_HEIGHT+TEXT_YMARGIN+mvs(2), "Unlimited");
		else if (fraglimit.value < 1000)
			M_PrintWhite (MM_XOFF+TEXT_XMARGIN+MM_GAMEOPS_OPT_XOFF, MM_HEADER_HEIGHT+TEXT_YMARGIN+mvs(2), va("%i", (int)fraglimit.value));
		else
			M_PrintWhite (MM_XOFF+TEXT_XMARGIN+MM_GAMEOPS_OPT_XOFF, MM_HEADER_HEIGHT+TEXT_YMARGIN+mvs(2), "Rounds");
	//======================== 3
	M_Print (MM_XOFF+TEXT_XMARGIN, MM_HEADER_HEIGHT+TEXT_YMARGIN+mvs(3), "TIME");
		if (timelimit.value == 0)
			M_PrintWhite (MM_XOFF+TEXT_XMARGIN+MM_GAMEOPS_OPT_XOFF, MM_HEADER_HEIGHT+TEXT_YMARGIN+mvs(3), "Unlimited");
		else
			M_PrintWhite (MM_XOFF+TEXT_XMARGIN+MM_GAMEOPS_OPT_XOFF, MM_HEADER_HEIGHT+TEXT_YMARGIN+mvs(3), va("%i Minutes", (int)timelimit.value));
	//======================== 4
	M_Print (MM_XOFF+TEXT_XMARGIN, MM_HEADER_HEIGHT+TEXT_YMARGIN+mvs(4), "BOT SKILL");
		if (skill.value == 0)
			M_PrintWhite (MM_XOFF+TEXT_XMARGIN+MM_GAMEOPS_OPT_XOFF, MM_HEADER_HEIGHT+TEXT_YMARGIN+mvs(4), "Easy");
		else if (skill.value == 1)
			M_PrintWhite (MM_XOFF+TEXT_XMARGIN+MM_GAMEOPS_OPT_XOFF, MM_HEADER_HEIGHT+TEXT_YMARGIN+mvs(4), "Normal");
		else if (skill.value == 2)
			M_PrintWhite (MM_XOFF+TEXT_XMARGIN+MM_GAMEOPS_OPT_XOFF, MM_HEADER_HEIGHT+TEXT_YMARGIN+mvs(4), "Hard");
		else
			M_PrintWhite (MM_XOFF+TEXT_XMARGIN+MM_GAMEOPS_OPT_XOFF, MM_HEADER_HEIGHT+TEXT_YMARGIN+mvs(4), "Legendary");
	//======================== 5
	M_Print (MM_XOFF+TEXT_XMARGIN, MM_HEADER_HEIGHT+TEXT_YMARGIN+mvs(5), "MAP");
		M_PrintWhite (MM_XOFF+TEXT_XMARGIN+MM_GAMEOPS_OPT_XOFF, MM_HEADER_HEIGHT+TEXT_YMARGIN+mvs(5), levels[episodes[startepisode].firstLevel + startlevel].description);
	//========================= 6
	M_PrintWhite (MM_XOFF+TEXT_XMARGIN, MM_HEADER_HEIGHT+TEXT_YMARGIN+mvs(6), "START GAME");
	//========================= Draw image
	qpic_t *randmap = Draw_CachePicSafe("gfx/maps/random.lmp", "");
	M_DrawTransPic(MM_XOFF+(MM_WIDTH_PIX-randmap->width)/2, MM_HEADER_HEIGHT+MM_GAMEOPS_HEIGHT+(((MM_HEIGHT_PIX-MM_FOOTER_HEIGHT)-(MM_HEADER_HEIGHT+MM_GAMEOPS_HEIGHT))-randmap->height)/2, randmap);
	qpic_t *mappic = Draw_CachePicSafe(va("gfx/maps/%s.lmp", levels[episodes[startepisode].firstLevel + startlevel].name), "gfx/maps/random.lmp");
	M_DrawTransPic(MM_XOFF+(MM_WIDTH_PIX-mappic->width)/2, MM_HEADER_HEIGHT+MM_GAMEOPS_HEIGHT+(((MM_HEIGHT_PIX-MM_FOOTER_HEIGHT)-(MM_HEADER_HEIGHT+MM_GAMEOPS_HEIGHT))-mappic->height)/2, mappic);
	//========================= Footer
	M_PrintWhite (MM_WIDTH_PIX-23*CHARZ, MM_HEIGHT_PIX-MM_FOOTER_HEIGHT+TEXT_YMARGIN, "0 Back X Select/Change");
}

void M_Matchmaking_Change (int dir)
{
	int epcount, levcount;
	switch (gameoptions_cursor)
	{
	case 1: //game mode
		Cvar_SetValue ("deathmatch", (int)deathmatch.value + dir);
		if ((int)deathmatch.value < 1) //rollover
		{
			Cvar_SetValue ("deathmatch", 3);
			Cbuf_AddText ("deathmatch 3\n");
		}
		else if((int)deathmatch.value > 3) //rollover
		{
			Cvar_SetValue ("deathmatch", 1);
			Cbuf_AddText ("deathmatch 1\n");
		}
		else if ((int)deathmatch.value == 2) //firefight
		{
			Cvar_SetValue ("deathmatch", (int)deathmatch.value + dir);
			Cbuf_AddText ( va ("deathmatch %u\n", (int)deathmatch.value + dir) );
		}

		if((int)deathmatch.value == 1) //slayer
		{
			if((int)fraglimit.value > 1000)
				Cbuf_AddText ("fraglimit 50\n");
			Cbuf_AddText ("deathmatch 1\n");
			Cvar_SetValue ("deathmatch", 1);
			Cvar_SetValue ("coop", 0);
			Cvar_SetValue ("teamplay", 0);
			startepisode = 0;
		}
		else if((int)deathmatch.value == 3) //Swat
		{
			if((int)fraglimit.value > 1000)
				Cbuf_AddText ("fraglimit 50\n");
			Cbuf_AddText ("deathmatch 3\n");
			Cvar_SetValue ("deathmatch", 3);
			Cvar_SetValue ("coop", 0);
			Cvar_SetValue ("teamplay", 0);
			startepisode = 0;
		}
		else
		{
			startepisode = 0;
			startlevel = 0;
		}

		if(sv.active)
			Cbuf_AddText ("disconnect\n");
		break;

	case 2: //game mode frags
		Cvar_SetValue ("fraglimit", fraglimit.value + dir*10);
		if (fraglimit.value > 100)
			Cvar_SetValue ("fraglimit", 0);
		if (fraglimit.value < 0)
			Cvar_SetValue ("fraglimit", 100);
		break;
	case 3: //time set
		Cvar_SetValue ("timelimit", timelimit.value + dir*5);
		if (timelimit.value > 60)
			Cvar_SetValue ("timelimit", 0);
		if (timelimit.value < 0)
			Cvar_SetValue ("timelimit", 60);
		break;
	case 4: //difficulty
		Cvar_SetValue ("skill", skill.value + dir);
		if (skill.value > 3)
			Cvar_SetValue ("skill", 0);
		if (skill.value < 0)
			Cvar_SetValue ("skill", 3);
		break;
	case 5: //map
		startlevel += dir;
		levcount = episodes[startepisode].levels;

		if (startlevel < 0)
			startlevel = levcount - 1;

		if (startlevel >= levcount)
			startlevel = 0;
		break;
	}
}

void M_Matchmaking_Key (int key)
{
	switch (key)
	{
	case K_ENTER:
	case K_START:
	case K_CIRCLE:
	case K_TRIANGLE:
		M_Menu_Main_f ();
		break;

	case K_UPARROW:
		S_LocalSound ("misc/menuoption.wav");
		gameoptions_cursor--;
		if (gameoptions_cursor < 0)
			gameoptions_cursor = NUM_GAMEOPTIONS-1;
		break;

	case K_DOWNARROW:
		S_LocalSound ("misc/menuoption.wav");
		gameoptions_cursor++;
		if (gameoptions_cursor >= NUM_GAMEOPTIONS)
			gameoptions_cursor = 0;
		break;

	case K_LEFTARROW:
		if (gameoptions_cursor == 6 || gameoptions_cursor == 0)
			break;
		S_LocalSound ("misc/menuoption.wav");
		M_Matchmaking_Change (-1);
		break;

	case K_RIGHTARROW:
		if (gameoptions_cursor == 6 || gameoptions_cursor == 0)
			break;
		S_LocalSound ("misc/menuoption.wav");
		M_Matchmaking_Change (1);
		break;

	case K_CROSS:
		S_LocalSound ("misc/menuenter.wav");
		if (gameoptions_cursor == 6)
		{
			if (sv.active)
				Cbuf_AddText ("disconnect\n");
			S_LocalSound ("misc/menuback.wav");
			SCR_BeginLoadingPlaque ();
			Cbuf_AddText ("listen 0\n");	// so host_netport will be re-examined
			Cbuf_AddText ("chase_active 0\n");
			Cbuf_AddText ( va ("maxplayers %u\n", 8) );
			Cbuf_AddText ( va ("deathmatch %u\n", (int)deathmatch.value) );
			Cbuf_AddText ( va ("map %s\n", levels[episodes[startepisode].firstLevel + startlevel].name) );

			return;
		}
		else if (gameoptions_cursor == 0)
		{
			m_multiplayer_cursor = 1;
			M_Menu_Net_f ();
		}

		M_Matchmaking_Change (1);
		break;
	}
}

void M_Menu_Firefight_f (void)
{
	key_dest = key_menu;
	m_state = m_firefight;
	m_entersound = true;
	//default firefight settings
	Cvar_SetValue ("skill", 0);
	Cvar_SetValue ("deathmatch", 2);
	startepisode = 0;
	startlevel = 0;
}

int firefight_cursor_table[] = {40, 56, 64, 72, 80, 88, 96, 112, 120};
#define	NUM_FIREFIGHT_OPTIONS	3
int		firefight_cursor;


void M_Firefight_Draw (void)
{
	qpic_t	*p, *gt, *bg, *bar, *skillp, *mapfire, *mapconstruction, *maprandom;
	int		x, chg;

	mapfire = Draw_CachePic ("gfx/maps/fire.lmp");
	mapconstruction = Draw_CachePic ("gfx/maps/construction.lmp");
	bg = Draw_CachePic ("gfx/MENU/matchmakingback.lmp");
	bar = Draw_CachePic ("gfx/MENU/menubarmp.lmp");

	chg=106;
	int xmenustart = 30;

	M_DrawTransPic(xmenustart-16, 0, bg);
	M_DrawTransPic (xmenustart-16, 166-chg+(firefight_cursor*20),bar);


	//========================
	//  OPT: Start game
	//=======================
	M_Print (xmenustart, 168-chg, "Start Game");
	//========================
	//  OPT: difficulty
	//=======================
	M_Print (xmenustart, 188-chg, "Skill");
	if (skill.value == 0)
		M_PrintWhite (xmenustart+60, 188-chg, "Easy");
	else if (skill.value == 1)
		M_PrintWhite (xmenustart+60, 188-chg, "Normal");
	else if (skill.value == 2)
		M_PrintWhite (xmenustart+60, 188-chg, "Hard");
	else
		M_PrintWhite (xmenustart+60, 188-chg, "Legendary");
	//========================
	//  OPT: map
	//=======================
	M_Print (xmenustart, 208-chg, "Map");
	M_PrintWhite (xmenustart+40, 208-chg, levels[episodes_ff[startepisode].firstLevel + startlevel].description);

	//Draw image
	if(levels[episodes_ff[startepisode].firstLevel + startlevel].name == "fire")
    	 M_DrawTransPic(xmenustart,300-chg, mapfire);
   	else if(levels[episodes_ff[startepisode].firstLevel + startlevel].name == "construction")
    	 M_DrawTransPic(xmenustart,300-chg, mapconstruction);
   	else
   		M_DrawTransPic(xmenustart,300-chg, maprandom);
}

void M_Firefight_Change (int dir)
{
	int epcount, levcount;
	switch (firefight_cursor)
	{
	case 1: //difficulty
		Cvar_SetValue ("skill", skill.value + dir);
		if (skill.value > 3)
			Cvar_SetValue ("skill", 0);
		if (skill.value < 0)
			Cvar_SetValue ("skill", 3);
		break;
	case 2: //map
		startlevel += dir;
		levcount = episodes_ff[startepisode].levels;

		if (startlevel < 0)
			startlevel = levcount - 1;

		if (startlevel >= levcount)
			startlevel = 0;
		break;
	}
}

void M_Firefight_Key (int key)
{
	switch (key)
	{
	case K_ENTER:
	case K_START:
	case K_TRIANGLE:
		M_Menu_Main_f ();
		break;

	case K_UPARROW:
		S_LocalSound ("misc/menuoption.wav");
		firefight_cursor--;
		if (firefight_cursor < 0)
			firefight_cursor = NUM_FIREFIGHT_OPTIONS-1;
		break;

	case K_DOWNARROW:
		S_LocalSound ("misc/menuoption.wav");
		firefight_cursor++;
		if (firefight_cursor >= NUM_FIREFIGHT_OPTIONS)
			firefight_cursor = 0;
		break;

	case K_LEFTARROW:
		if (firefight_cursor == 0)
			break;
		S_LocalSound ("misc/menuoption.wav");
		M_Firefight_Change (-1);
		break;

	case K_RIGHTARROW:
		if (firefight_cursor == 0)
			break;
		S_LocalSound ("misc/menuoption.wav");
		M_Firefight_Change (1);
		break;

	case K_CROSS:
		S_LocalSound ("misc/menuenter.wav");
		if (firefight_cursor == 0) //on start options
		{
			if (sv.active)
				Cbuf_AddText ("disconnect\n");
			SCR_BeginLoadingPlaque (); //#TODO: fix loading screen not working.
			Cbuf_AddText ("timelimit 60\n");
			Cbuf_AddText ("fraglimit 9999\n");
			Cbuf_AddText ("listen 0\n");	// so host_netport will be re-examined
			Cbuf_AddText ("chase_active 0\n");
			Cbuf_AddText ( va ("maxplayers %u\n", 1) );
			Cbuf_AddText ( va ("deathmatch %u\n", (int)deathmatch.value) );
			Cbuf_AddText ( va ("map %s\n", levels[episodes_ff[startepisode].firstLevel + startlevel].name) );

			return;
		}
		break;
	}
}

//=============================================================================
/* SEARCH MENU */

bool	searchComplete = false;
double		searchCompleteTime;

void M_Menu_Search_f (void)
{
	key_dest = key_menu;
	m_state = m_search;
	m_entersound = false;
	slistSilent = true;
	slistLocal = false;
	searchComplete = false;
	NET_Slist_f();

}


void M_Search_Draw (void)
{
	qpic_t	*p;
	int x;

	p = Draw_CachePic ("gfx/p_multi.lmp");
	M_DrawPic ( (320-p->width)/2, 4, p);
	x = (320/2) - ((12*8)/2) + 4;
	M_DrawTextBox (x-8, 32, 12, 1);
	M_Print (x, 40, "Searching...");

	if(slistInProgress)
	{
		NET_Poll();
		return;
	}

	if (! searchComplete)
	{
		searchComplete = true;
		searchCompleteTime = realtime;
	}

	if (hostCacheCount)
	{
		M_Menu_ServerList_f ();
		return;
	}

	M_PrintWhite ((320/2) - ((22*8)/2), 64, "No Quake servers found");
	if ((realtime - searchCompleteTime) < 3.0)
		return;

	M_Menu_LanConfig_f ();
}


void M_Search_Key (int key)
{
}

//=============================================================================
/* SLIST MENU */

int		slist_cursor;
bool slist_sorted;

void M_Menu_ServerList_f (void)
{
	key_dest = key_menu;
	m_state = m_slist;
	m_entersound = true;
	slist_cursor = 0;
	m_return_onerror = false;
	m_return_reason[0] = 0;
	slist_sorted = false;
}


void M_ServerList_Draw (void)
{
	int		n;
	char	string [64];
	qpic_t	*p;

	if (!slist_sorted)
	{
		if (hostCacheCount > 1)
		{
			int	i,j;
			hostcache_t temp;
			for (i = 0; i < hostCacheCount; i++)
				for (j = i+1; j < hostCacheCount; j++)
					if (strcmp(hostcache[j].name, hostcache[i].name) < 0)
					{
						memcpy(&temp, &hostcache[j], sizeof(hostcache_t));
						memcpy(&hostcache[j], &hostcache[i], sizeof(hostcache_t));
						memcpy(&hostcache[i], &temp, sizeof(hostcache_t));
					}
		}
		slist_sorted = true;
	}

	p = Draw_CachePic ("gfx/p_multi.lmp");
	M_DrawPic ( (320-p->width)/2, 4, p);
	for (n = 0; n < hostCacheCount; n++)
	{
		if (hostcache[n].maxusers)
			sprintf(string, "%-15.15s %-15.15s %2u/%2u\n", hostcache[n].name, hostcache[n].map, hostcache[n].users, hostcache[n].maxusers);
		else
			sprintf(string, "%-15.15s %-15.15s\n", hostcache[n].name, hostcache[n].map);
		M_Print (16, 32 + 8*n, string);
	}
	M_DrawCharacter (0, 32 + slist_cursor*8, 12+((int)(realtime*4)&1));

	if (*m_return_reason)
		M_PrintWhite (16, 148, m_return_reason);
}


void M_ServerList_Key (int k)
{
	switch (k)
	{
	case K_ENTER:
	case K_START:
	case K_TRIANGLE:
		M_Menu_LanConfig_f ();
		break;

	case K_SPACE:
		M_Menu_Search_f ();
		break;

	case K_UPARROW:
	case K_LEFTARROW:
		S_LocalSound ("misc/menuoption.wav");
		slist_cursor--;
		if (slist_cursor < 0)
			slist_cursor = hostCacheCount - 1;
		break;

	case K_DOWNARROW:
	case K_RIGHTARROW:
		S_LocalSound ("misc/menuoption.wav");
		slist_cursor++;
		if (slist_cursor >= hostCacheCount)
			slist_cursor = 0;
		break;

	case K_CROSS:
		S_LocalSound ("misc/menuenter.wav");
		m_return_state = m_state;
		m_return_onerror = true;
		slist_sorted = false;
		key_dest = key_game;
		m_state = m_none;
		Cbuf_AddText ( va ("connect \"%s\"\n", hostcache[slist_cursor].cname) );
		break;

	default:
		break;
	}

}

//=============================================================================
/* Menu Subsystem */


void M_Init (void)
{
	Cmd_AddCommand ("togglemenu", M_ToggleMenu_f);

	Cmd_AddCommand ("menu_main", M_Menu_Main_f);
	Cmd_AddCommand ("menu_singleplayer", M_Menu_SinglePlayer_f);
	Cmd_AddCommand ("menu_load", M_Menu_Load_f);
	Cmd_AddCommand ("menu_save", M_Menu_Save_f);
	Cmd_AddCommand ("menu_multiplayer", M_Menu_MultiPlayer_f);
	Cmd_AddCommand ("menu_setup", M_Menu_Setup_f);
	Cmd_AddCommand ("menu_options", M_Menu_Options_f);
	Cmd_AddCommand ("menu_keys", M_Menu_Keys_f);
	Cmd_AddCommand ("menu_video", M_Menu_Video_f);
	Cmd_AddCommand ("help", M_Menu_Help_f);
	Cmd_AddCommand ("menu_quit", M_Menu_Quit_f);
	Cmd_AddCommand ("menu_pause", M_Menu_Pause_f);

}


void M_Draw (void)
{
	if (m_state == m_none || key_dest != key_menu)
		return;

	if (!m_recursiveDraw)
	{
		scr_copyeverything = 1;

		if (scr_con_current)
		{
			Draw_ConsoleBackground ();
			VID_UnlockBuffer ();
			S_ExtraUpdate ();
			VID_LockBuffer ();
		}
		else
			Draw_FadeScreen ();

		scr_fullupdate = 0;
	}
	else
	{
		m_recursiveDraw = false;
	}

	GL_SetCanvas (CANVAS_MENU_STRETCH); //johnfitz

	switch (m_state)
	{
	case m_none:
		break;

	case m_main:
		M_Main_Draw ();
		break;

	case m_singleplayer:
		M_SinglePlayer_Draw ();
		break;

	case m_load:
		M_Load_Draw ();
		break;

	case m_save:
		M_Save_Draw ();
		break;

	case m_multiplayer:
		M_MultiPlayer_Draw ();
		break;

	case m_setup:
		M_Setup_Draw ();
		break;

	case m_net:
		M_Net_Draw ();
		break;

	case m_options:
		M_Options_Draw ();
		break;
		
	case m_graphics:
		M_Graphics_Draw();
		break;
		
	case m_mods:
		M_Mods_Draw();
		break;

	case m_keys:
		M_Keys_Draw ();
		break;

	case m_video:
		M_Video_Draw ();
		break;

	case m_help:
		M_Help_Draw ();
		break;

	case m_quit:
		M_Quit_Draw ();
		break;

	case m_lanconfig:
		M_LanConfig_Draw ();
		break;

	case m_gameoptions:
		M_Matchmaking_Draw ();
		break;

	case m_search:
		M_Search_Draw ();
		break;

	case m_slist:
		M_ServerList_Draw ();
		break;

	case m_onlineserverlist:
		M_OnlineServerList_Draw();
		break;

	case m_benchmark:
		M_Benchmark_Draw();
		break;
	case m_pause:
		M_Pause_Draw();
		break;
	case m_firefight:
		M_Firefight_Draw();
		break;
	}

	if (m_entersound)
	{
		S_LocalSound ("misc/menuenter.wav");
		m_entersound = false;
	}

	VID_UnlockBuffer ();
	S_ExtraUpdate ();
	VID_LockBuffer ();
}


void M_Keydown (int key)
{
	switch (m_state)
	{
	case m_none:
		return;

	case m_main:
		M_Main_Key (key);
		return;

	case m_singleplayer:
		M_SinglePlayer_Key (key);
		return;

	case m_load:
		M_Load_Key (key);
		return;

	case m_save:
		M_Save_Key (key);
		return;

	case m_multiplayer:
		M_MultiPlayer_Key (key);
		return;

	case m_setup:
		M_Setup_Key (key);
		return;

	case m_net:
		M_Net_Key (key);
		return;

	case m_options:
		M_Options_Key (key);
		return;
		
	case m_graphics:
		M_Graphics_Key (key);
		return;

	case m_mods:
		M_Mods_Key (key);
		return;

	case m_keys:
		M_Keys_Key (key);
		return;

	case m_video:
		M_Video_Key (key);
		return;

	case m_help:
		M_Help_Key (key);
		return;

	case m_quit:
		M_Quit_Key (key);
		return;

	case m_lanconfig:
		M_LanConfig_Key (key);
		return;

	case m_gameoptions:
		M_Matchmaking_Key (key);
		return;

	case m_search:
		M_Search_Key (key);
		break;

	case m_slist:
		M_ServerList_Key (key);
		return;

	case m_onlineserverlist:
		M_OnlineServerList_Key (key);
		break;

	case m_benchmark:
		M_Benchmark_Key (key);
		break;
	case m_pause:
		M_Pause_Key(key);
		break;
	case m_firefight:
		M_Firefight_Key (key);
		return;
	}
}


void M_ConfigureNetSubsystem(void)
{
// enable/disable net systems to match desired config

	Cbuf_AddText ("stopdemo\n");

	//if (IPXConfig || TCPIPConfig)
		net_hostport = lanConfig_port;
}
