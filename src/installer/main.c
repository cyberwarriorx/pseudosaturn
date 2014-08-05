/*  Copyright 2014 Theo Berkau

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
    Program that installs PseudoSaturn firmware to flash.
*/

#include <string.h>
#include <stdio.h>
#include <iapetus.h>
#include "main.h"
#include "ps_rom.h"

font_struct main_font;

//////////////////////////////////////////////////////////////////////////////

u16 wait_for_press()
{
	for (;;)        
	{
		vdp_vsync(); 
		if (per[0].but_push_once)
			break;
	}
	return per[0].but_push_once;
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

	press=wait_for_press();

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

void backup_flash()
{
	int ret;
	volatile u16 *read_addr=(volatile u16 *)0x22000000;
	volatile u16 *write_addr=(volatile u16 *)0x20200000;
	u32 i;
	flash_info_struct flash_info;

	vdp_start_draw_list();
	vdp_end_draw_list();

	if ((ret = ar_init_flash_io(&flash_info)) != IAPETUS_ERR_OK)
	{
		if (!ar_handle_detect_error(ret))
		   return;
	}

	for (i = 0; i < 0x40000; i++)
		write_addr[i] = read_addr[i];

	vdp_printf(&main_font, 8, 3 * 8, 0xF, "Firmware ready to download. Set your");
	vdp_printf(&main_font, 8, 4 * 8, 0xF, "Commlink utility to download data from");
	vdp_printf(&main_font, 8, 5 * 8, 0xF, "LWRAM(0x00200000 to 0x00240000)");
	vdp_printf(&main_font, 8, 6 * 8, 0xF, "Press any button to exit.");

	commlink_start_service();
	wait_for_press();
	commlink_stop_service();
}

//////////////////////////////////////////////////////////////////////////////

int reflash_ar(font_struct *font, u8 *rom_addr, int ask_upload)
{
   int ret;
   volatile u16 *write_addr=(volatile u16 *)0x22000000;
   u16 *read_addr=(u16 *)((u32)rom_addr | 0x20000000);
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
				return 1;
      }

      vdp_printf(font, 8, 8, 0xF, "Detected cart succesfully");

		if (ask_upload)
		{
			vdp_printf(font, 8, 3 * 8, 0xF, "Please upload flash to 0x00200000 and");
			vdp_printf(font, 8, 4 * 8, 0xF, "then press 'A' to continue. ");
			vdp_printf(font, 8, 5 * 8, 0xF, "Press 'X' to exit.");

			commlink_start_service();

			for (;;)        
			{
				vdp_vsync(); 
				if (per[0].but_push_once & PAD_A)
					break;
				else if (per[0].but_push_once & PAD_X)
					return 1;
			}

			commlink_stop_service();

			if (strncmp((char *)read_addr, "SEGA SEGASATURN ", 16) != 0)
			{
				vdp_printf(font, 8, 7 * 8, 0xF, "Invalid or no ROM uploaded. ");
				vdp_printf(font, 8, 8 * 8, 0xF, "Press 'A' to try again.");

				for (;;)        
				{
					vdp_vsync(); 
					if (per[0].but_push_once & PAD_A)
					{
						vdp_clear_screen(font);
						goto start;
					}
				}
			}
		}

      vdp_printf(font, 8, 7 * 8, 0xF, "WARNING: Rewriting the flash may damage");
      vdp_printf(font, 8, 8 * 8, 0xF, "your AR. Press A+B+C to continue.");
      vdp_printf(font, 8, 9 * 8, 0xF, "Press 'X' to exit.");

      for (;;)        
      {
         vdp_vsync(); 
         if (per[0].but_push & PAD_A &&
             per[0].but_push & PAD_B &&
             per[0].but_push & PAD_C)
            break;
         else if (per[0].but_push_once & PAD_X)
            return 1;
      }

      vdp_printf(font, 8, 11 * 8, 0xF, "DO NOT TURN OFF YOUR SYSTEM");

      vdp_printf(font, 8, 12 * 8, 0xF, "Erasing flash...");
      ar_erase_flash_all(&flash_info);
      vdp_printf(font, 17 * 8, 12 * 8, 0xF, "OK");

      vdp_printf(font, 8, 13 * 8, 0xF, "Writing flash...");		
      font->transparent = FALSE;
      for (i = 0; i < flash_info.num_pages; i++)
      {
         vdp_printf(font, 17 * 8, 13 * 8, 0xF, "%d%%  ", (i+1) * 100 / 32);
         ar_write_flash(&flash_info, write_addr+(i*flash_info.page_size), read_addr+(i*flash_info.page_size), 1);
      }
      vdp_printf(font, 17 * 8, 13 * 8, 0xF, "OK  ");
      font->transparent = TRUE;
      vdp_printf(font, 8, 14 * 8, 0xF, "Verifying flash...");
      ret = ar_verify_write_flash(&flash_info, write_addr, read_addr, flash_info.num_pages);
      vdp_printf(font, 19 * 8, 14 * 8, 0xF, ret ? "OK" : "FAILED");

      if (ret)
      {
         vdp_printf(font, 8, 15 * 8, 0xF, "SUCCESS! Press reset to finish.");
         goto done;
      }

      vdp_printf(font, 8, 15 * 8, 0xF, "Failed flashing AR. Press a 'A' to");
      vdp_printf(font, 8, 16 * 8, 0xF, "retry or 'X' to exit");

      for (;;)        
      {
         vdp_vsync(); 
         if (per[0].but_push_once & PAD_A)
            break;
         else if (per[0].but_push_once & PAD_X)
            return 0;
      }
      vdp_clear_screen(font);
   }

done:
   for (;;)        
   {
      vdp_vsync(); 
      if (per[0].but_push_once & PAD_A)
         break;
   }

	return 1;
}

//////////////////////////////////////////////////////////////////////////////

void install_ps_disc()
{
	if (!reflash_ar(&main_font, (u8 *)ps_rom, 0))
		bios_run_cd_player();
}

//////////////////////////////////////////////////////////////////////////////

void install_ps_commlink()
{
	if (!reflash_ar(&main_font, (u8 *)0x00200000, 1))
		bios_run_cd_player();
}

//////////////////////////////////////////////////////////////////////////////

void credits()
{
	vdp_printf(&main_font, 8, 8, 0xF, "Copyright 2014 Cyber Warrior X");
	vdp_printf(&main_font, 8, 2 * 8, 15, "http://www.cyberwarriorx.com");
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

menu_item_struct main_menu[] = {
	{ "Backup AR to Commlink" , &backup_flash, },
	{ "Inst Firmware from Disc" , &install_ps_disc, },
	{ "Inst Firmware from Comm" , &install_ps_commlink, },
	{ "Credits" , &credits, }, 
	{ "\0", NULL }
};

//////////////////////////////////////////////////////////////////////////////

void installer_init()
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

   // Setup the default 8x16 1BPP font
   main_font.data = font_8x8;
   main_font.width = 8;
   main_font.height = 8;
   main_font.bpp = 1;
   main_font.out = (u8 *)0x25E00000;
   vdp_set_font(SCREEN_RBG0, &main_font, 1);

   // Display everything
   vdp_disp_on();

	if (ud_detect() == IAPETUS_ERR_OK)
		cl_set_service_func(ud_check);
}

int main()
{
   installer_init();

   // Display Main Menu
   for(;;)
   {
      commlink_start_service();
      gui_do_menu(main_menu, &main_font, 0, 0, "Pseudo Saturn Installer v" INSTALLER_VERSION, MTYPE_CENTER, -1);

      main_font.transparent = 1;
      gui_clear_scr(&main_font);
   }

   return 0;
}

//////////////////////////////////////////////////////////////////////////////
