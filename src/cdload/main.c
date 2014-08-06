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
#include "main.h"

#define CD_WORKBUF 0x202C0000

//char cdfsinit_msg[] = "Starting CDFSInit";
char cdfs_init_error_msg[] = "Couldn't initialize CD File System";
char opening_file_msg[] = "Opening file...";
char error_msg[] = "Error.";
char done_msg[] = "Done.";
char reading_file_msg[] = "Reading file...";
char starting_game_msg[] = "Starting game in X second(s)";
char debugger_msg[] = "Hit X to enable debugger";
char seconds_msg[] = "3";
font_struct main_font;

//////////////////////////////////////////////////////////////////////////////

int main()
{
   file_struct file;
   int start_addr;
   int i, i2;
   u8 *sect_buffer = (u8 *)0x26002000;
	
   start_addr = *((u32 *)(sect_buffer+0xF0));

   // Re-initialize peripherals since Pseudosaturn still has a hold of it
   per_init();

   // Setup the default 8x16 1BPP font
   main_font.data = font_8x8;
   main_font.width = 8;
   main_font.height = 8;
   main_font.bpp = 1;
   main_font.out = (u8 *)0x25E00000;
   vdp_set_font(SCREEN_RBG0, &main_font, 1);
   //vdp_print_text(&mainfont, 2 * 8, 3 * 16, 15, cdfsinit_msg);

   if (cdfs_init((void *)CD_WORKBUF, 4096) != IAPETUS_ERR_OK)
   {
      vdp_print_text(&main_font, 2 * 8, 5 * 16, 15, "Couldn't initialize CD File System");
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
#ifdef ENABLE_DEBUG
   // Delay for 3 seconds in case a button is hit
   vdp_print_text(&main_font, 2 * 8, 9 * 16, 15, "Starting game in X second(s)");
   vdp_print_text(&main_font, 2 * 8, 10 * 16, 15,  "Hit X to enable debugger");
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
#endif

   // Execute ip code
   ipprog();
   return 0;
}

//////////////////////////////////////////////////////////////////////////////
