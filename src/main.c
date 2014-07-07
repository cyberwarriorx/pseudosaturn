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

#include <string.h>
#include <iapetus.h>
#include "cdload.h"
#include "main.h"

#define CD_WORKBUF 0x202C0000

u8 *sect_buffer = (u8 *)0x26002000;

font_struct main_font;

u8 *dir_tbl;

menu_item_struct main_menu[] = {
{ "Start Game" , &start_game, },
{ "Reflash AR" , &reflash_ar, },
{ "Credits" , &credits, }, 
{ "\0", NULL }
};

//////////////////////////////////////////////////////////////////////////////

void load_exec_binary(unsigned char *data, int size, u32 addr)
{
   void (*func)();

   //vdp_printf(&mainfont, 2 * 8 , 23 * 8, 15, "Loading binary to %08X(%d)", addr, size);
   memcpy((void *)addr, data, size);
   func = (void (*)())addr;
   //vdp_printf(&mainfont, 2 * 8 , 24 * 8, 15, "Data Loaded, executing %08X", func);
   func();
}

//////////////////////////////////////////////////////////////////////////////

void load_and_start(u32 start_addr, u32 ipsize)
{
   //vdp_printf(&mainfont, 2 * 8 , 22 * 8, 15, "load_and_start: startaddr = %08X", startaddr);
   if ((start_addr & 0x0F000000) == 0x06000000)
   {
      // Load code, etc. to lwram and execute
      //load_exec_binary(cdload, cdload_size, 0x00202000);
      file_struct file;
      int i, i2;
	
      if (cdfs_init((void *)CD_WORKBUF, 4096) != IAPETUS_ERR_OK)
      {
         vdp_print_text(&main_font, 2 * 8, 4 * 16, 15, "Couldn't initialize CD File System");
         for (;;) {}
      }

      vdp_print_text(&main_font, 2 * 8, 6 * 16, 15, "Opening file...");
      if ((cdfs_open (NULL, &file)) != IAPETUS_ERR_OK)
      {
         vdp_print_text(&main_font, 17 * 8, 6 * 16, 15, "Error.");
         for (;;) {}
      }
      vdp_print_text(&main_font, 17 * 8, 6 * 16, 15, "Done.");
      vdp_vsync();

      //   vdp_printf(&mainfont, 2 * 8, 4 * 16, 15, "dirtbl = %02X %02X %02X %02X", dirtbl[0], dirtbl[1], dirtbl[2], dirtbl[3]);
      //   vdp_printf(&mainfont, 2 * 8, 5 * 16, 15, "lba = %d", file.lba);

      vdp_print_text(&main_font, 2 * 8, 7 * 16, 15, "Reading file...");
      if (cdfs_read((void *)start_addr, 1, file.size, &file) != IAPETUS_ERR_OK)
      {
         vdp_print_text(&main_font, 17 * 8, 7 * 16, 15, "Error.");
         for (;;) {}
      }
      vdp_print_text(&main_font, 17 * 8, 7 * 16, 15, "Done.");
      vdp_vsync();

      //   vdp_printf(&mainfont, 2 * 8, 8 * 16, 15, "Start Address: %08X(%08X)", startaddr, *((u32 *)startaddr));

      // Delay for 3 seconds in case a button is hit
      vdp_print_text(&main_font, 2 * 8, 9 * 16, 15, "Starting game in X second(s)");
      vdp_print_text(&main_font, 2 * 8, 10 * 16, 15, "Hit X to enable debugger");
      vdp_print_text(&main_font, 19 * 8, 9 * 16, 0, "X");
      for (i = 3; i > 0; i--)
      {
         vdp_printf(&main_font, 19 * 8, 9 * 16, 15, "%d", i);

         for (i2 = 0; i2 < 60; i2++)
         {
            vdp_vsync();
            if (per[0].but_push_once & PAD_X)
            {
               // Enable Debugger
               debugger_start();
            }
         }
         vdp_printf(&main_font, 19 * 8, 9 * 16, 0, "%d", i);
      }

      // Execute ip code
      ipprog();
   }
   else
   {
      // Load code, etc. to hwram and execute
      load_exec_binary(cdload, cdload_size, 0x06082000);
   }
}

//////////////////////////////////////////////////////////////////////////////

void start_game()
{
   int stage=1;
   u32 start_addr=0;
   u32 ip_size=0;
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
            // Stage 2: Check if CD Block is locked, and unlock if neccessary
            u16 disc_type;

            if (!is_cd_auth(&disc_type))
            {
               cd_auth();

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

            break;
         }
         case 3:
         {
            // Stage 3: Read first 16 sectors of disc
            if ((cd_read_sector(sect_buffer, 150, SECT_2048, 16 * 2048)) == 0)
            {
               // continue onto stage 4
               stage = 4;
            }
            else
            {
               vdp_printf(&main_font, 2 * 8, 4 * 8, 15, "Failed reading IP");
               stage = 0;
            }
 
            break;
         }
         case 4:
         {
            // Stage 4: Verify ip header and process ip
            if (strncmp((char *)sect_buffer, "SEGA SEGASATURN ", 16) == 0)
            {
               // Horray! We can boot it!
               vdp_printf(&main_font, 2 * 8, 4 * 8, 15, "Detected real saturn disc");
               stage = 5;
            }
            else if (strncmp((char *)sect_buffer, "PSEUDO SATURN   ", 16) == 0)
            {
               // Horray! We can boot it!
               vdp_printf(&main_font, 2 * 8, 4 * 8, 15, "Detected pseudo saturn disc");
               stage = 5;
            }
            else
            {
               // Treat it like a VCD/Photo CD and try running the vcd player
               if (mpeg_exist)
               {
                  debugger_start();
                  vdp_printf(&main_font, 2 * 8 , 21 * 8, 15, "Loading Potential VCD");

                  bios_get_mpeg_rom(0, 128, (u32)sect_buffer);
                  vdp_printf(&main_font, 2 * 8 , 22 * 8, 15, "Fetched ROM");
						
                  ip_size = *((u32 *)(sect_buffer+0xE0));
                  vdp_printf(&main_font, 2 * 8 , 23 * 8, 15, "IP Size = %08X", ip_size);
                  memcpy((void *)0x20200000, sect_buffer+ip_size, (128*2048)-ip_size);
                  ipprog();
               }
					else
					{
						vdp_printf(&main_font, 2 * 8, 4 * 8, 15, "Unrecognized cd");
						vdp_printf(&main_font, 2 * 8 , 5 * 8, 15, "%02X %02X %02X %02X %02X", sect_buffer[0], sect_buffer[1], sect_buffer[2], sect_buffer[3], sect_buffer[4]);
						stage = 0;
						for (;;)
						{
							vdp_vsync();
							if (per[0].but_push_once & PAD_A)
								return;
						}
					}
            }

            break;
         }
         case 5:
         {
            // Read First Program
            start_addr = *((u32 *)(sect_buffer+0xF0));
            ip_size =    *((u32 *)(sect_buffer+0xE0));

            load_and_start(start_addr, ip_size);
            return;
         }
         default: break;
      }
   }
}

//////////////////////////////////////////////////////////////////////////////

void reflash_ar()
{
   int ret;
   volatile u16 *write_addr=(volatile u16 *)0x22000000;
   u16 *read_addr=(u16 *)0x20200000;

   vdp_start_draw_list();
   vdp_end_draw_list();

   for (;;)
   {
start:
      if ((ret = ar_init_flash_io()) != IAPETUS_ERR_OK)
      {
         vdp_printf(&main_font, 8, 8, 0xF, "Error detecting cart. Err code = %d", ret);
         goto done;
      }

      vdp_printf(&main_font, 8, 8, 0xF, "Detected cart succesfully");

      vdp_printf(&main_font, 8, 3 * 8, 0xF, "Please upload flash to 0x00200000 and");
      vdp_printf(&main_font, 8, 4 * 8, 0xF, "then press 'A' to continue. Press 'X'");
      vdp_printf(&main_font, 8, 5 * 8, 0xF, "to exit.");

      commlink_start_service();

      for (;;)        
      {
         vdp_vsync(); 
         if (per[0].but_push_once & PAD_A)
            break;
         else if (per[0].but_push_once & PAD_X)
            return;
      }

      commlink_stop_service();

      if (strncmp((char *)read_addr, "SEGA SEGASATURN ", 16) != 0)
      {
         vdp_printf(&main_font, 8, 7 * 8, 0xF, "Invalid or no ROM uploaded. Press 'A'");
         vdp_printf(&main_font, 8, 8 * 8, 0xF, "to try again.");

         for (;;)        
         {
            vdp_vsync(); 
            if (per[0].but_push_once & PAD_A)
            {
               vdp_clear_screen(&main_font);
               goto start;
            }
         }
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
      vdp_printf(&main_font, 8, 12 * 8, 0xF, "Writing flash...");
      ar_write_flash(write_addr, read_addr, 1024); // fix me
      vdp_printf(&main_font, 17 * 8, 12 * 8, 0xF, "OK");
      vdp_printf(&main_font, 8, 13 * 8, 0xF, "Verifying flash...");
      ret = ar_verify_write_flash(write_addr, read_addr, 1024);
      vdp_printf(&main_font, 19 * 8, 13 * 8, 0xF, ret ? "OK" : "FAILED");

      if (ret)
      {
         vdp_printf(&main_font, 8, 14 * 8, 0xF, "SUCCESS! Press reset to finish.");
         goto done;
      }

      vdp_printf(&main_font, 8, 14 * 8, 0xF, "Failed flashing AR. Press a 'A' to");
      vdp_printf(&main_font, 8, 15 * 8, 0xF, "retry or 'X' to exit");

      for (;;)        
      {
         vdp_vsync(); 
         if (per[0].but_push_once & PAD_A)
            break;
         else if (per[0].but_push_once & PAD_X)
            return;
      }
      vdp_clear_screen(&main_font);
   }

done:
   for (;;)        
   {
      vdp_vsync(); 
      if (per[0].but_push_once & PAD_A)
         break;
   }
}

//////////////////////////////////////////////////////////////////////////////

void credits()
{
   vdp_printf(&main_font, 8, 8, 0xF, "Copyright 2011-2014 Cyber Warrior X");
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

int main()
{
   screen_settings_struct settings;
   int choice;

   // This should always be called at the begining of your program.
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

   // Display Main Menu
   for(;;)
   {
      commlink_start_service();
      choice = gui_do_menu(main_menu, &main_font, 0, 0, "Pseudo Saturn v0.829", MTYPE_CENTER, -1);
      main_font.transparent = 1;
      gui_clear_scr(&main_font);
   }

   return 0;
}

//////////////////////////////////////////////////////////////////////////////
