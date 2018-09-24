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

/*
    Program that attempts to run code off custom cd's
    by Cyber Warrior X. Based on code by Charles MacDonald.
*/

#include <stdio.h>
#include <string.h>
#include <iapetus.h>
#include "cheat.h"
#include "cdload.h"
#include "main.h"

#ifdef ENABLE_CHEATS
#define ROM_CHEAT_LIST 0x02020000
#define CHEAT_LIST 0x00200000
#endif

font_struct main_font;

u8 *dir_tbl;

void jhload(void);

menu_item_struct main_menu[] = {
{ "Start Game" , &start_game, },
#ifdef ENABLE_CHEATS
{ "Select Cheats", &select_cheats, },
#endif
{ "Reflash AR" , &reflash_ar, },
{ "Credits" , &credits, }, 
{ "\0", NULL }
};

//////////////////////////////////////////////////////////////////////////////

int cd_set_sector_size(int size);

int cd_move_sector_data_cd_auth(u8 dst_filter, u16 sector_pos, u8 sel_num, u16 num_sectors)
{
	int ret;
	cd_cmd_struct cd_cmd1;
	cd_cmd_struct cd_cmd2;
	cd_cmd_struct cd_cmd_rs;
	int i;
	cd_stat_struct cd_status;
	u16 auth;

	cd_cmd1.CR1 = 0x6600 | dst_filter;
	cd_cmd1.CR2 = sector_pos;
	cd_cmd1.CR3 = sel_num << 8;
	cd_cmd1.CR4 = num_sectors;

	cd_cmd2.CR1 = 0xE000;
	cd_cmd2.CR2 = 0x0000;
	cd_cmd2.CR3 = 0x0000;
	cd_cmd2.CR4 = 0x0000;

	ret = cd_exec_command(0, &cd_cmd1, &cd_cmd_rs);

	// Clear hirq flags
	CDB_REG_HIRQ = ~(HIRQ_DCHG | HIRQ_EFLS);

	// Authenticate disc
	if ((ret = cd_exec_command(HIRQ_EFLS, &cd_cmd2, &cd_cmd_rs)) != 0)
		return ret;

	// wait till operation is finished
	while (!(CDB_REG_HIRQ & HIRQ_EFLS)) {}

	// Wait until drive has finished seeking
	for (;;)
	{
		// wait a bit
		for (i = 0; i < 100000; i++) { }

		if (cd_get_stat(&cd_status) != 0) continue;

		if (cd_status.status == STATUS_PAUSE) break;
		else if (cd_status.status == STATUS_FATAL) return IAPETUS_ERR_UNKNOWN;
	}

	// Was Authentication successful?
	if (!is_cd_auth(&auth))
		return IAPETUS_ERR_AUTH;

	return IAPETUS_ERR_OK;
}

//////////////////////////////////////////////////////////////////////////////

int jhl_cd_hack()
{
	int ret;
	int i, j;
	u16 type;

	if ((ret = cd_set_sector_size(SECT_2352)) != 0)
		return ret;

	// Write 150x2352 sectors
	if ((ret = cd_put_sector_data(0, 150)) != 0)
		return ret;

	// copy data
	for (j = 0; j < 150; j++)
	{
		for (i = 0; i < (2352/4); i++)
			CDB_REG_DATATRNS = 0x00020002;
	}

	if ((ret = cd_end_transfer()) != 0)
		return ret;

	while (!(CDB_REG_HIRQ & HIRQ_EHST)) {}

	ret = cd_move_sector_data_cd_auth(0, 0, 0, 50);

	ret = is_cd_auth(&type);
	//vdp_printf(&main_font, 8, 5 * 8, 0xF, "is_cd_auth = %d, type = %d", ret, type);

	while (!(CDB_REG_HIRQ & HIRQ_ECPY)) {}

	if ((ret = cd_end_transfer()) != 0)
		return ret;

	if ((ret = cd_set_sector_size(SECT_2048)) != 0)
		return ret;

	if ((ret = cd_reset_selector_all()) != 0)
		return ret;

	return ret;
}

//////////////////////////////////////////////////////////////////////////////

BOOL is_audio_cd()
{
   u32 toc[102];

   // Check TOC
   cd_get_toc(toc);

   vdp_printf(&main_font, 2 * 8, 5 * 8, 15, "%08X -> %02X -> %d", toc[0], ((toc[0] & 0xFF000000) >> 24),(((toc[0] & 0xFF000000) >> 24) == 0x01) ? TRUE : FALSE );
   return (((toc[0] & 0xFF000000) >> 24) == 0x01) ? TRUE : FALSE;
}

void start_game()
{
   int stage=1;
   BOOL mpeg_exist;

   commlink_stop_service();

   main_font.transparent = 0;

   if (cd_init() != IAPETUS_ERR_OK)
   {
      vdp_printf(&main_font, 2 * 8, 3 * 8, 15, "Couldn't initialize CD Block");
      for (;;)
      {
         vdp_vsync();
         if (per[0].but_push_once & PAD_A)
            return;
      }
   }

	mpeg_exist = is_mpeg_card_present();
   //if (mpegexist)
   //   vdp_printf(&mainfont, 2 * 8, 3 * 8, 15, "MPEG Card detected");

   for(;;)                                
   {
      vdp_vsync();

      switch (stage)
      {
         case 1:
         {
            // Stage 1: Get CD Block status. Make sure disc is inserted
            if (is_cd_present())
            {
               vdp_printf(&main_font, 2 * 8, 4 * 8, 15, "Loading Disc...   ");

               if (is_audio_cd())
               {
                  vdp_printf(&main_font, 2 * 8, 4 * 8, 15, "Found Audio Disc");
                  bios_run_cd_player();
               }

               // continue onto stage 2
               stage = 2;
            }
            else
            {
               // Display an error message letting the user know that they need to insert a disc
               vdp_printf(&main_font, 2 * 8, 4 * 8, 15, "Please insert disc");
            }
               
            break;
         }
         case 2:
         {
            // Stage 2: Check if CD Block is locked, and unlock if necessary
            u16 disc_type;

            if (!is_cd_auth(&disc_type))
            {
               jhl_cd_hack();

               if (!is_cd_auth(&disc_type))
               {
                  // Run CD Player
                  bios_run_cd_player();
               }
               else
                  stage = 3;
            }
            else
               stage = 3;

            // try BIOS-based boot
            jhload();
          
	    return;
         }
         default: break;
      }
   }
}

//////////////////////////////////////////////////////////////////////////////

#ifdef ENABLE_CHEATS

void select_cheats()
{
   // Display Main Menu
   for(;;)
   {      
      int choice = ct_games_do_menu(&main_font);

      main_font.transparent = 1;
      gui_clear_scr(&main_font);
      if (choice == -1)
         break;
   }
}

#endif

//////////////////////////////////////////////////////////////////////////////

u16 wait_for_press(u16 mask)
{	
	for (;;)        
	{
		vdp_vsync(); 
		if (per[0].but_push_once & mask)
			break;
	}
	return per[0].but_push_once & mask;
}

//////////////////////////////////////////////////////////////////////////////

BOOL ar_handle_detect_error(int err)
{
	char text[128];
	u16 press;
	u16 vendor_id, device_id;
	if (err == IAPETUS_ERR_HWNOTFOUND)
		sprintf(text, "HW not found.");
	else if(err == IAPETUS_ERR_UNSUPPORTED)
		sprintf(text, "HW not supported.");
	else
		sprintf(text, "Unknown error.");
	vdp_printf(&main_font, 8, 8, 0xF, "Error detecting cart. %s", text);
	ar_get_product_id(&vendor_id, &device_id);
	vdp_printf(&main_font, 8, 16, 0xF, "HW ID: %04X %04X", vendor_id, device_id);

	press=wait_for_press(-1);

	if (err != IAPETUS_ERR_UNSUPPORTED)
		return FALSE;

	if (press & PAD_A && press & PAD_B && press & PAD_C)
	{
		vdp_vsync();
		vdp_clear_screen(&main_font);
		return TRUE;
	}

	return FALSE;
}

//////////////////////////////////////////////////////////////////////////////

void reflash_ar()
{
   int ret;
   volatile u16 *write_addr=(volatile u16 *)0x22000000;
   u16 *read_addr=(u16 *)0x20200000;
	flash_info_struct flash_info;
	int i;

   vdp_start_draw_list();
   vdp_end_draw_list();

   for (;;)
   {
start:
      if ((ret = ar_init_flash_io(&flash_info)) != IAPETUS_ERR_OK)
      {
			if (!ar_handle_detect_error(ret))
            goto done;
      }

      vdp_printf(&main_font, 8, 8, 0xF, "Detected cart succesfully");

      vdp_printf(&main_font, 8, 3 * 8, 0xF, "Please upload flash to 0x00200000 and");
      vdp_printf(&main_font, 8, 4 * 8, 0xF, "then press 'A' to continue. Press 'X'");
      vdp_printf(&main_font, 8, 5 * 8, 0xF, "to exit.");

      commlink_start_service();

		if (wait_for_press(PAD_A | PAD_X) & PAD_X)
			return;

      commlink_stop_service();

      if (strncmp((char *)read_addr, "SEGA SEGASATURN ", 16) != 0)
      {
         vdp_printf(&main_font, 8, 7 * 8, 0xF, "Invalid or no ROM uploaded.");
         vdp_printf(&main_font, 8, 8 * 8, 0xF, "Press 'A' to try again.");

			wait_for_press(PAD_A);
			vdp_clear_screen(&main_font);
			goto start;
      }

      vdp_printf(&main_font, 8, 7 * 8, 0xF, "WARNING: Rewriting the flash may damage");
      vdp_printf(&main_font, 8, 8 * 8, 0xF, "your AR. Press A+B+C to continue. Press");
      vdp_printf(&main_font, 8, 9 * 8, 0xF, "'X' to exit.");

      for (;;)        
      {
         vdp_vsync(); 
         if (per[0].but_push & PAD_A &&
             per[0].but_push & PAD_B &&
             per[0].but_push & PAD_C)
            break;
         else if (per[0].but_push_once & PAD_X)
            return;
      }

      vdp_printf(&main_font, 8, 11 * 8, 0xF, "DO NOT TURN OFF YOUR SYSTEM");

		vdp_printf(&main_font, 8, 12 * 8, 0xF, "Erasing flash...");
		ar_erase_flash_all(&flash_info);
		vdp_printf(&main_font, 17 * 8, 12 * 8, 0xF, "OK");

		vdp_printf(&main_font, 8, 13 * 8, 0xF, "Writing flash...");		
		main_font.transparent = FALSE;
		for (i = 0; i < flash_info.num_pages; i++)
		{
			vdp_printf(&main_font, 17 * 8, 13 * 8, 0xF, "%d%%  ", (i+1) * 100 / flash_info.num_pages);
			ar_write_flash(&flash_info, write_addr+(i*flash_info.page_size), read_addr+(i*flash_info.page_size), 1);
		}
		vdp_printf(&main_font, 17 * 8, 13 * 8, 0xF, "OK  ");
		main_font.transparent = TRUE;
      vdp_printf(&main_font, 8, 14 * 8, 0xF, "Verifying flash...");
      ret = ar_verify_write_flash(&flash_info, write_addr, read_addr, flash_info.num_pages);
      vdp_printf(&main_font, 19 * 8, 14 * 8, 0xF, ret ? "OK" : "FAILED");

      if (ret)
      {
         vdp_printf(&main_font, 8, 15 * 8, 0xF, "SUCCESS! Press reset to finish.");
         goto done;
      }

      vdp_printf(&main_font, 8, 15 * 8, 0xF, "Failed flashing AR. Press a 'A' to");
      vdp_printf(&main_font, 8, 16 * 8, 0xF, "retry or 'X' to exit");

		if (wait_for_press(PAD_A | PAD_X) & PAD_X)
			return;

      vdp_clear_screen(&main_font);
   }

done:
	wait_for_press(PAD_A);
}

//////////////////////////////////////////////////////////////////////////////

void credits()
{
   vdp_printf(&main_font, 8, 8, 0xF, "Copyright 2011-2015 Pseudo Saturn Team");
   vdp_printf(&main_font, 8, 2 * 8, 15, "http://github.com/cyberwarriorx/pseudosaturn");
   vdp_printf(&main_font, 8, 10 * 8, 15, "Press any button to go back");

   for (;;)        
   {
      vdp_vsync(); 
      if (per[0].but_push_once)
         break;
   }
   vdp_clear_screen(&main_font);
}

//////////////////////////////////////////////////////////////////////////////

void ps_init()
{
	screen_settings_struct settings;

	init_iapetus(RES_320x224);

	// Setup a screen for us draw on
	settings.is_bitmap = TRUE;
	settings.bitmap_size = BG_BITMAP512x256;
	settings.transparent_bit = 0;
	settings.color = BG_256COLOR;
	settings.special_priority = 0;
	settings.special_color_calc = 0;
	settings.extra_palette_num = 0;
	settings.map_offset = 0;
	settings.rotation_mode = 0;
	settings.parameter_addr = 0x25E60000;
	vdp_rbg0_init(&settings);

	// Use the default palette
	vdp_set_default_palette();

	// Setup the default 8x8 1BPP font
	main_font.data = font_8x8;
	main_font.width = 8;
	main_font.height = 8;
	main_font.bpp = 1;
	main_font.out = (u8 *)0x25E00000;
	vdp_set_font(SCREEN_RBG0, &main_font, 1);
	//con_init(&main_font, 0, 0, 320, 224);

	do_logo();

	// Display everything
	vdp_disp_on();

	if (ud_detect() == IAPETUS_ERR_OK)
		cl_set_service_func(ud_check);
}

//////////////////////////////////////////////////////////////////////////////

int main()
{
	ps_init();

   // Display Main Menu
   for(;;)
   {
      commlink_start_service();
      gui_do_menu(main_menu, &main_font, 0, 0, "Pseudo Saturn v" PSEUDOSATURN_VERSION, MTYPE_CENTER, -1);

      main_font.transparent = 1;
      gui_clear_scr(&main_font);
   }

   return 0;
}

//////////////////////////////////////////////////////////////////////////////
