/*  Copyright 2015 Theo Berkau

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
#include <string.h>
#include <fcntl.h>
#include "main.h"

extern u8 logo[];
extern u32 logo_size;

void do_logo()
{
	int ret;
	int i,j;
	u16 pal[256];
	u8 crc_table[] = { 0xE3, 0xCC, 0x8A, 0xD3, 0xC5, 0xDF, 0x8A, 0xDA, 0xCB, 0xC3, 0xCE, 0x8A, 0xCC, 0xC5, 0xD8, 0x8A, 0xDE, 0xC2, 0xC3, 0xD9, 0x86, 0x8A, 0xD3, 0xC5, 0xDF, 0x8A, 0xDD, 0xCF, 0xD8, 0xCF, 0x8A, 0xD8, 0xC3, 0xDA, 0xDA, 0xCF, 0xCE, 0x8A, 0xC5, 0xCC, 0xCC, 0x8B, 0xAA };
	u8 *img_buffer=(u8 *)0x06002000;

	vdp_disp_on();

	img_struct img;
	if ((ret = pcx_load(logo, logo_size, &img, img_buffer)) != IAPETUS_ERR_OK)
	{
		vdp_printf(&main_font, 2 * 8, 1 * 8, 15, "Error loading pcx data = %d", ret);
		wait_for_press(-1);
	}

	// Convert palette
	for (i = 0; i < 256; i++)
	{
		unsigned char *p=&img.palette[i*3];
		pal[i] = RGB555(p[0] >> 3, p[1] >> 3, p[2] >> 3);
	}

	// Make sure we're faded out
	vdp_fade_out(SCREEN_RBG0, 0, 255);
	vdp_disp_on();
	for (i = 0; i < img.height; i++)
	   memcpy((void *)0x25E00000+(i*512), img.data+(i*img.width), img.width);
	j=8*10;
	for (i = 0; i < 20; i++)
		j += main_font.drawchar(&main_font, j, 25 * 8, 1, crc_table[i]^0xAA);
	j=8*10;
	for (i = 22; i < sizeof(crc_table)-1; i++)
		j += main_font.drawchar(&main_font, j, 26 * 8, 1, crc_table[i]^0xAA);
	vdp_set_palette(CRM5_2048, pal, 256);

	vdp_printf(&main_font, 8, 23 * 8, 1, "Copyright 2011-2015 Pseudo Saturn Team");

	vdp_fade_in(SCREEN_RBG0, 0, 2);

	// Wait for a few seconds or something
	for (i = 0; i < 2*60; i++) { vdp_vsync(); }

	vdp_fade_out(SCREEN_RBG0, 0, 2);
	vdp_disp_off();
	vdp_set_default_palette();

	vdp_clear_screen(&main_font);
	vdp_disable_color_offset(SCREEN_RBG0);
}
