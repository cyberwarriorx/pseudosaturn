/*  Copyright 2011-2014 Theo Berkau

    This file is part of PseudoSaturn.

    PseudoSaturn is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    PseudoSaturn is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with PseudoSaturn; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#include <iapetus.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include "codes.h"
#include "main.h"

extern vdp2_settings_struct vdp2_settings;

//////////////////////////////////////////////////////////////////////////////

void vdp_clear_line(font_struct *font, int x, int y, int width)
{
	char text[128];
	memset(text, ' ', width);
	text[width] = '\0';
	font->transparent = FALSE;
	vdp_print_text(font, x, y, 0xF, text);
	font->transparent = TRUE;
}

typedef struct  
{
	char name[34];
	int num_mc;
	int num_cheats;
	int num_cheats_on;
} game_struct;

typedef struct  
{
	char name[40];
	struct
	{
		u32 addr;
		u16 code;
	} codes[16];
   u16 enable;
	u8 num_codes;
} cheat_struct;

//////////////////////////////////////////////////////////////////////////////

static inline void align_addr_word(u32 *offset)
{
   if (offset[0] & 0x1)
      offset[0] += 1;
}

//////////////////////////////////////////////////////////////////////////////

static inline void align_addr_long(u32 *offset)
{
   for (;;)
   {
      if (offset[0] & 0x3)
         offset[0] += 1;
      else
         break;
   }
}

//////////////////////////////////////////////////////////////////////////////

static inline void align_buf(u8 **buf, u32 *offset)
{
	if ((u32)buf[0] & 0x2)
	{
		buf[0]-=2;
		offset[0]+=2;
	}
}

//////////////////////////////////////////////////////////////////////////////

void ct_menu_init(font_struct *font, char *title, int maxlen, int nummenu, int *x, int *y, int *menux, int *menuy)
{
	int win_width, win_height;

	// Setup Priorities
	VDP2_REG_PRIR = 0x0002;
	VDP2_REG_PRISA = 0x0101;
	VDP2_REG_PRISB = 0x0101;
	VDP2_REG_PRISC = 0x0101;
	VDP2_REG_PRISD = 0x0101;

	// Make sure current screen is clear
	gui_window_init();

	win_width = 6 + 8 + 6 + (maxlen * font->width);
	win_height = 5+5+5+8+nummenu*8;

	*x = ((vdp2_settings.screen_width - win_width) >> 1);
	*y = ((vdp2_settings.screen_height - win_height) >> 1);

	gui_window_draw(*x, *y, win_width, win_height, TRUE, 0, RGB16(26, 26, 25) | 0x8000);

	// Draw title
	vdp_printf(font, x[0]+6, y[0]+5, 15, "%s", title);

	*menux = x[0] + 6;
	*menuy = y[0] + 5 + 8 + 5;
}

//////////////////////////////////////////////////////////////////////////////

void ct_menu_clear(font_struct *font)
{
	gui_clear_scr(font);

	vdp_start_draw_list();
	vdp_end_draw_list();
}

//////////////////////////////////////////////////////////////////////////////

int ct_get_master_codes(u8 *buf, cheat_struct *cheats)
{
	int i;
	u32 offset=0;

	align_buf(&buf, &offset);

	strcpy(cheats->name, "");
	offset += strlen((char *)buf+offset)+1;
	align_addr_word(&offset);
	cheats->num_codes = buf[offset+1];
	offset+=2;

	for (i = 0; i < cheats->num_codes; i++)
	{
		align_addr_long(&offset);

		// Address
		cheats->codes[i].addr = *((u32 *)(buf+offset));
		offset += 4;
		// Data
		cheats->codes[i].code = *((u16 *)(buf+offset));
		offset += 2;
	}
	cheats->enable = 1;

	return 1;
}

//////////////////////////////////////////////////////////////////////////////

void ct_get_cheat_info(u8 *buf, cheat_struct *cheat)
{
	u32 offset=0;
	u32 len;
	int i;

	align_buf(&buf, &offset);

	len = strlen((char *)buf+offset)+1;
	strcpy(cheat->name,(char *)buf+offset);

	offset+=len;
	cheat->num_codes = buf[offset];

	for (i = 0; i < cheat->num_codes; i++)
	{
		align_addr_long(&offset);
		cheat->codes[i].addr = *((u32 *)(buf+offset));
		cheat->codes[i].code = *((u16 *)(buf+offset+4));
		offset += 8;
	}
	cheat->enable = *((u16 *)(buf+offset-2));
}

//////////////////////////////////////////////////////////////////////////////

int ct_get_num_master_codes(u8 *buf, u32 *offset)
{
	int num_codes;

	align_addr_word(offset);

	num_codes = buf[offset[0]+1];
	offset[0] += 2;
	if (num_codes != 0)
	{
		align_addr_long(offset);

		// Address
		offset[0] += 4;
		// Data
		offset[0] += 2;

		if (num_codes > 1)
		{
			align_addr_long(offset);

			// Address
			offset[0] += 4;
			// Data
			offset[0] += 2;
		}
	}

	return num_codes;
}

//////////////////////////////////////////////////////////////////////////////

int ct_get_cheats_ptr(u8 **cheats, u8 *buf, int max_cheats)
{
	int num_cheats;
	u32 offset=0;

	align_buf(&buf, &offset);

	// Skip game name
	offset += strlen((char *)buf+offset)+1;
	// Skip Master code
	ct_get_num_master_codes(buf, &offset);

	num_cheats=0;

	if (buf[offset] != 0xFE)
	{
		for (;;)
		{
			int i;
			int len;
			int num_codes;

			cheats[num_cheats] = buf+offset;
			len = strlen((char *)buf+offset)+1;
			num_codes = buf[offset+len];
			offset+=len+1;

			for (i = 0; i < num_codes; i++)
			{
				align_addr_long(&offset);
				offset += 8;
			}
		   num_cheats++;

			if (buf[offset] == 0xFE || num_cheats >= max_cheats)
				break;
		}
	}

	return num_cheats;
}

//////////////////////////////////////////////////////////////////////////////

int ct_get_num_cheats(u8 *buf, u32 *offset, int *num_cheats_on)
{
	int num_cheats = 0;
	if (buf[offset[0]] != 0xFE)
	{
		num_cheats = 0;
		for (;;)
		{
			int i;
			int num_codes;
			offset[0] += strlen((char *)buf+offset[0])+1;
			num_codes = buf[offset[0]];
			offset[0]+=1;

			for (i = 0; i < num_codes; i++)
			{
				align_addr_long(offset);
				offset[0] += 4;
				offset[0] += 2;
				offset[0] += 2;
			}

			if (num_cheats_on)
			{
				u8 *ptr=buf+offset[0]-2;				
				*num_cheats_on = (ptr[0] << 8) | ptr[1];
			}

			if (buf[offset[0]] == 0xFE)
				break;
		}
		num_cheats++;
	}

	return num_cheats;
}

//////////////////////////////////////////////////////////////////////////////

void ct_get_game_info(u8 *buf, game_struct *game)
{
	u32 offset = 0;

	strcpy(game->name, (char *)buf+offset); 
	offset += strlen((char *)buf+offset)+1;
	
	game->num_mc = ct_get_num_master_codes(buf, &offset);
	game->num_cheats = ct_get_num_cheats(buf, &offset, &game->num_cheats_on);
}

//////////////////////////////////////////////////////////////////////////////

int ct_get_games_ptr(u8 **games, u8 *buf, int max_games)
{
	int num_games = 0;
	u32 offset=0;

	for (;;)
	{
		if (buf[offset] == 0xFF || offset >= cheatlist_size)
			break;

		games[num_games] = buf+offset;
		offset += strlen((char *)buf+offset)+1;

		ct_get_num_master_codes(buf, &offset);
		ct_get_num_cheats(buf, &offset, NULL);

		offset += 2;

		num_games++;

		if (num_games >= max_games)
			break;
	}

	return num_games;
}

//////////////////////////////////////////////////////////////////////////////

void vdp_printf_center(font_struct *font, int x, int y, int maxlen, int color, int bg_color, char *format, ...)
{
	char string[128];
	va_list arg;

	va_start(arg, format);
	vsprintf(string, format, arg);
	x += ((maxlen-strlen(string))/2)*8;
	vdp_print_text(font, x+1, y+1, bg_color, string);
	vdp_print_text(font, x, y, color, string);
	va_end(arg);
}

//////////////////////////////////////////////////////////////////////////////

void refresh_cheat_list(font_struct *font, int menux, int menuy, int maxlen, u8 **buf, int scroll_pos, int nummenu, int num_codes)
{
	int i,num;
	int off=0; 
	menux += 1 * 8;
	if (scroll_pos == 0)
	{
		vdp_clear_line(font, menux, menuy, maxlen);
		vdp_printf_center(font, menux, menuy, maxlen, 0xF, 0x10, "-- master code --");
		menuy += 1 * 8;
		off++;
	}

	num = num_codes-scroll_pos+1-off;

	if (num > nummenu)
		num = nummenu-off;

	buf += off-1+scroll_pos;
	for (i = 0; i < num; i++)
	{
		cheat_struct cheat;
		vdp_clear_line(font, menux, menuy, maxlen);
		ct_get_cheat_info(buf[i], &cheat);
		vdp_printf_center(font, menux, menuy, maxlen, 0xF, 0x10, cheat.name);
		menuy += 8;
	}

	if (num_codes-scroll_pos < (nummenu-1))
	{
		vdp_clear_line(font, menux, menuy, maxlen);
		vdp_printf_center(font, menux, menuy, maxlen, 0xF, 0x10, "--- new code ---");
	}
}

//////////////////////////////////////////////////////////////////////////////

void refresh_game_list(font_struct *font, int menux, int menuy, int maxlen, u8 **buf, int nummenu)
{
	int i;
	menux += 1 * 8;
	for (i = 0; i < nummenu; i++)
	{
		game_struct game;
		vdp_clear_line(font, menux, menuy + (i * 8), maxlen);
		ct_get_game_info(buf[i], &game);
		vdp_printf_center(font, menux, menuy + (i * 8), maxlen, 0xF, 0x10, game.name);
	}
}

//////////////////////////////////////////////////////////////////////////////

int ct_start_game(font_struct *font, int comms_enable, u8 *ptr, u8 **cheat_ptr)
{
	int cursel=0;
	int menux, menuy;
	int maxlen=38;
	int nummenu=3;
	int x, y;
	game_struct game;

	ct_menu_init(font, comms_enable ? "Start Game Options (Comms Enable)" : "Start Game Options", maxlen, nummenu, &x, &y, &menux, &menuy);

	// Add Selected Menu Item(should always be first item) sprite to draw list
	vdp_printf(font, menux, menuy+(cursel * 8), 0xF, ">");

	vdp_printf_center(font, menux, menuy, maxlen, 0xF, 0x10, "START GAME WITH SELECTED CHEATS");
	vdp_printf_center(font, menux, menuy + (8 * 1), maxlen, 0xF, 0x10, "START GAME WITH NO CHEATS ENABLED");
	vdp_printf_center(font, menux, menuy + (8 * 2), maxlen, 0xF, 0x10, "CANCEL - RETURN TO PREVIOUS MENU");

	ct_get_game_info(ptr, &game);
	vdp_printf_center(font, x, menuy + (8 * 5), maxlen, 0xF, 0x10, "%s", game.name);
	vdp_printf_center(font, x, menuy + (8 * 6), maxlen, 0xF, 0x10, "%04d CHEATS ARE ON", game.num_cheats_on);

	vdp_printf_center(font, x, vdp2_settings.screen_height-1-(8*3), maxlen, 0xF, 0x10, "UP/DOWN: HIGHLIGHT OPTION");
	vdp_printf_center(font, x, vdp2_settings.screen_height-1-(8*2), maxlen, 0xF, 0x10, "A:SELECTS  B:CANCEL & EXIT");
	vdp_printf_center(font, x, vdp2_settings.screen_height-1-(8*1), maxlen, 0xF, 0x10, "C: EXIT TO CD PLAYER ");

	for (;;)
	{
		vdp_vsync(); 

		// poll joypad(if menu item is selected, return)
		if (per[0].but_push_once & PAD_UP)
		{
			vdp_printf(font, menux, menuy + (cursel * 8), 0, ">");

			if (cursel > 1)
				cursel--;

			vdp_printf(font, menux, menuy + (cursel * 8), 0xF, ">");
		}
		else if (per[0].but_push_once & PAD_DOWN)
		{
			vdp_printf(font, menux, menuy + (cursel * 8), 0x0, ">");

			if (cursel < 3)
				cursel++;

			vdp_printf(font, menux, menuy + (cursel * 8), 0xF, ">");
		}

		if (per[0].but_push_once & PAD_A)
		{
			// Select

			// Load Game

			// Apply codes here
			switch(cursel)
			{
			   case 0:
					start_game();
					break;
				case 1:
					start_game();
					break;
				case 2:
					ct_menu_clear(font);
					return -1;
			}
		}
		else if (per[0].but_push_once & PAD_B)
		{
			ct_menu_clear(font);
			return -1;
		}
		else if (per[0].but_push_once & PAD_C)
			bios_run_cd_player();
	}
}

//////////////////////////////////////////////////////////////////////////////

int ct_cheats_do_menu(font_struct *font, u8 *ptr)
{
   int cursel=0;
   int menux, menuy;
   int maxlen=32;
	int nummenu=11;
   int x, y;
	u8 *cheats_off[1024];
   int num_cheats;
   int scroll_pos=0;

	ct_menu_init(font, "SELECT CHEATS", maxlen, nummenu, &x, &y, &menux, &menuy);

   num_cheats = ct_get_cheats_ptr(cheats_off, ptr, 1024);

	if (num_cheats+2 < nummenu)
		nummenu = num_cheats+2;

   // Add Selected Menu Item(should always be first item) sprite to draw list
   vdp_printf(font, menux, menuy+(cursel * 8), 0xF, ">");
	refresh_cheat_list(font, menux, menuy, maxlen, cheats_off, 0, nummenu, num_cheats);

	vdp_printf_center(font, x, vdp2_settings.screen_height-1-(8*2), maxlen, 0xF, 0x10, "A:ON/OFF  B:EXIT  C:EDIT  Z:DELETE");
	vdp_printf_center(font, x, vdp2_settings.screen_height-1-(8*1), maxlen, 0xF, 0x10, "START:START GAME  X:START WITH COMMS");

   for (;;)
   {
      vdp_vsync(); 

      // poll joypad(if menu item is selected, return)
      if (per[0].but_push_once & PAD_UP)
      {
         vdp_printf(font, menux, menuy + (cursel * 8), 0, ">");

         if (cursel > 2)
            cursel--;
         else
         {
            if (scroll_pos > 0)
            {
               scroll_pos--;
					refresh_cheat_list(font, menux, menuy, maxlen, cheats_off, scroll_pos, nummenu, num_cheats);
            }
            else
            {
               if (cursel != 0)
                  cursel--;
            }
         }

         vdp_printf(font, menux, menuy + (cursel * 8), 0xF, ">");
      }
      else if (per[0].but_push_once & PAD_DOWN)
      {
         vdp_printf(font, menux, menuy + (cursel * 8), 0x0, ">");

         if (cursel < (nummenu - 3))
            cursel++;
         else
         {
				if (scroll_pos+nummenu < num_cheats+2)
            {
               scroll_pos++;
					refresh_cheat_list(font, menux, menuy, maxlen, cheats_off, scroll_pos, nummenu, num_cheats);
            }
            else
            {
               if (cursel != (nummenu - 1))
                  cursel++;
            }
         }

         vdp_printf(font, menux, menuy + (cursel * 8), 0xF, ">");
      }

      if (per[0].but_push_once & PAD_A)
      {
			// ON/OFF
      }
      else if (per[0].but_push_once & PAD_B)
      {
			ct_menu_clear(font);
         return -1;
      }
		else if (per[0].but_push_once & PAD_C)
		{
			// EDIT
		}
		else if (per[0].but_push_once & PAD_X)
		{
			// Start with Comms
		}
		else if (per[0].but_push_once & PAD_Z)
		{
			// Delete
		}
		else if (per[0].but_push_once & PAD_START)
		{
			// Start Game
			ct_menu_clear(font);

			ct_start_game(font, FALSE, ptr, cheats_off);
		}
   }
}

//////////////////////////////////////////////////////////////////////////////

int ct_games_do_menu(font_struct *font)
{
   int cursel=0;
   int maxlen=34;
	int nummenu=15;
   int x, y;
	int menux, menuy;
   u8 *games_off[1024];
   int num_games;
   int scroll_pos=0;
	char text[64];

	ct_menu_init(font, "SELECT GAME", maxlen, nummenu, &x, &y, &menux, &menuy);

   num_games = ct_get_games_ptr(games_off, cheatlist, 1024);

   // Add Selected Menu Item(should always be first item) sprite to draw list
   vdp_printf(font, menux, menuy+(cursel * 8), 0xF, ">");

	refresh_game_list(font, menux, menuy, maxlen, games_off, nummenu);

	strcpy(text, "A:SELECTS  B:EXIT  Z:DELETE");
	x = ((40-strlen(text))/2)*8;
	y = vdp2_settings.screen_height-1-(8*2);
	vdp_printf(font, x, y, 0xF, text);

   for (;;)
   {
      vdp_vsync(); 

      // poll joypad(if menu item is selected, return)
      if (per[0].but_push_once & PAD_UP)
      {
         vdp_printf(font, menux, menuy + (cursel * 8), 0, ">");

         if (cursel > 2)
            cursel--;
         else
         {
            if (scroll_pos > 0)
				{
					scroll_pos--;
					refresh_game_list(font, menux, menuy, maxlen, games_off+scroll_pos, nummenu);
				}
            else
            {
               if (cursel != 0)
                  cursel--;
            }
         }

         vdp_printf(font, menux, menuy + (cursel * 8), 0xF, ">");
      }
      else if (per[0].but_push_once & PAD_DOWN)
      {
         vdp_printf(font, menux, menuy + (cursel * 8), 0x0, ">");

         if (cursel < (nummenu - 3))
            cursel++;
         else
         {
            if (scroll_pos+1+nummenu < num_games)
            {
               scroll_pos++;
					refresh_game_list(font, menux, menuy, maxlen, games_off+scroll_pos, nummenu);
            }
            else
            {
               if (cursel != (nummenu - 1))
                  cursel++;
            }
         }

         vdp_printf(font, menux, menuy + (cursel * 8), 0xF, ">");
      }

      if (per[0].but_push_once & PAD_A)
      {
         gui_clear_scr(font);

         vdp_start_draw_list();
         vdp_end_draw_list();
			
         ct_cheats_do_menu(font, games_off[scroll_pos+cursel]);
         return cursel;
      }
      else if (per[0].but_push_once & PAD_B)
      {
         gui_clear_scr(font);

         vdp_start_draw_list();
         vdp_end_draw_list();
         return -1;
      }
		else if (per[0].but_push_once & PAD_Z)
		{
			// Remove current selected from list
		}
   }
}

//////////////////////////////////////////////////////////////////////////////

