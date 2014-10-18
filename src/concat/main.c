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

#include <stdio.h>
#include <stdlib.h>

unsigned char *read_file(const char *filename, long *size)
{
	FILE *fp;
	unsigned char *buf;
	if ((fp = fopen(filename, "rb")) == NULL)
	{
		printf("Error opening %s", filename);
		return NULL;
	}

    fseek(fp, 0, SEEK_END);
	*size = ftell(fp);
    fseek(fp, 0, SEEK_SET);
	if ((buf = malloc(*size)) == NULL)
	{
		printf("Error allocating buffer\n");
		fclose(fp);
		return NULL;
	}

	fread((void *)buf, 1, *size, fp);
	fclose(fp);
	return buf;
}

int main(int argc, char *argv[])
{
	char *array_name="array";
	FILE *fp;
	long size, size2;
	unsigned char *buf, *buf2;
	long i, j;

	if (argc != 4)
	{
		printf("CONCAT by Cyber Warrior X\n");
		printf("usage: concat <source binary file 1> <source binary file 2> <destination source file>");
		exit(1);
	}

	buf = read_file(argv[1], &size);
	buf2 = read_file(argv[2], &size2);

	if ((fp = fopen(argv[3], "wb")) == NULL)
	{
		printf("Error opening %s for writing", argv[3]);
		exit(1);
	}

	fwrite((void *)buf, 1, size, fp);
	fwrite((void *)buf2, 1, size2, fp);
	fclose(fp);
}
