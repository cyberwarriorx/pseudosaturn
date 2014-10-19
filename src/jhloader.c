/*  Copyright 2014 James Laird-Wah

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

#include <string.h>
#include <iapetus.h>

extern font_struct main_font;

static int emulate_bios_loadcd_init(void) {
   *(uint32_t*)0x6000278 = 0;
   *(uint32_t*)0x600027c = 0;
   cd_abort_file();
   cd_end_transfer();
   cd_reset_selector_all();
   cd_set_sector_size(SECT_2048);
   return 0;
}

static struct region_s {
   char id;
   const char *key;
} regions[] = {
   {1,   "JAPAN"},
   {2,   "TAIWAN and PHILIPINES"},
   {4,   "USA and CANADA"},
   {5,   "BRAZIL"},
   {6,   "KOREA"},
   {10,  "ASIA PAL area"},
   {12,  "EUROPE"},
   {13,  "LATIN AMERICA"},
   {0,   NULL}
};

static const char *get_region_string(void) {
   char region = *(volatile char*)0x20100033;
   struct region_s *r;
   for (r=regions; r->id; r++)
      if (r->id == region)
         return r->key;
   return NULL;
}

static int set_image_region(u8 *base)
{
   const char *str = get_region_string();
   if (!str)
   {
      vdp_print_text(&main_font, 2 * 8, 6 * 16, 15, "Invalid region set!");
      return -1;
   }

   // set region header
   memset(base + 0x40, ' ', 0x10);
   base[0x40] = str[0];

   // set region footer
   char *ptr = base + 0xe00;
   memset(ptr, ' ', 0x20);
   *(uint32_t*)ptr = 0xa00e0009;
   strcpy(ptr + 4, "For ");
   strcpy(ptr + 8, str);
   char *end = ptr + 4 + strlen(ptr + 4);
   *end = '.';

   vdp_printf(&main_font, 2 * 8, 5 * 16, 15, "Region: %s", str);
   return 0;
}

static int emulate_bios_loadcd_read(void)
{
   int ret, i;

   // doesn't matter where
   u8 *ptr = (u8*)0x6002000;

   ret = cd_read_sector(ptr, 150, SECT_2048, 2048*16);
   if (ret < 0)
      return ret;

   // BIOS doesn't like PSEUDO discs ;)
   memcpy(ptr, "SEGA SEGASATURN ", 16);

   ret = set_image_region(ptr);
   if (ret < 0)
      return ret;

   ret = cd_put_sector_data(0, 16);
   if (ret < 0)
      return ret;
   while (!(CDB_REG_HIRQ & HIRQ_DRDY)) {}

   for (i = 0; i < 2048 * 16; i+=4)
      CDB_REG_DATATRNS = *(uint32_t*)(ptr + i);

   if ((ret = cd_end_transfer()) != 0)
      return ret;

   while (!(CDB_REG_HIRQ & HIRQ_EHST)) {}

   *(uint16_t*)0x60003a0 = 1;

   return 0;
}


void jhload(void)
{
   int ret;
   vdp_print_text(&main_font, 2 * 8, 6 * 16, 15, "Initialising CD block...");
   if ((ret = emulate_bios_loadcd_init()) < 0)
   {
      vdp_printf(&main_font, 25 * 8, 6 * 16, 15, "Error %d.", ret);
      for (;;) {}
   }
   vdp_print_text(&main_font, 25 * 8, 6 * 16, 15, "Done.");

   vdp_print_text(&main_font, 2 * 8, 7 * 16, 15, "Reading CD data...");
   if ((ret = emulate_bios_loadcd_read()) < 0)
   {
      vdp_printf(&main_font, 19 * 8, 7 * 16, 15, "Error %d.", ret);
      for (;;) {}
   }
   vdp_print_text(&main_font, 19 * 8, 7 * 16, 15, "Done.");

   vdp_print_text(&main_font, 2 * 8, 8 * 16, 15, "Booting CD...");
   ret = bios_loadcd_boot();

   // if we came back, that's an error!
   // error -8: bad region
   // error -4: bad security code
   // error -1: bad headers
   vdp_printf(&main_font, 19 * 8, 8 * 16, 15, "Error %d.", ret);
   for (;;)
   {
      vdp_vsync();
      if (per[0].but_push_once & PAD_A)
         return;
   }
}
