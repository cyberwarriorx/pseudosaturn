#include "StdAfx.h"
#include <strsafe.h>
#include "ImgUtil.h"

ImgUtil::ImgUtil(CString filename)
{
   genLUT();
   this->filename = filename;
}

ImgUtil::~ImgUtil(void)
{
}

bool ImgUtil::isBitSet(int val, char bit)
{
   return (val & (1 << bit)) != 0;
}

int ImgUtil::orBit(int val, char bit)
{
   return val | (1 << bit);
}

int ImgUtil::notBit(int val, char bit)
{
   return ~(1 << bit) & val;
}

void ImgUtil::generateCRCTable(int *tbl)
{
   unsigned int val;
   int j;
   int val2;
   int i;
   short xor;

   for (i = 0; i < 256; i++)
   {
      val2 = 0;
      val = i & 0xFFFF;
      for (j = 0; j < 8; j++)
      {
         xor = isBitSet(val, 0);
         if ( isBitSet(val2, j) )
            val = orBit(val >> 1, 31);
         else
            val = notBit(val >> 1, 31);
         if ( xor )
            val ^= 0xD8018001;
      }
      *tbl = val;
      tbl++;
   }
}

void ImgUtil::generateECCTables(int *eccTable1, int *eccTable2)
{
   int data;
   int *buf_ptr;
   signed int i;
   int *buf2_ptr;
   signed int j;
   int k;
   int buf[256];
   int buf2[256];

   data = 1;
   buf[0] = 1;
	for (i = 1; i < 8; i++)
   {
      data *= 2;
      buf[i] = data;
   }
   for (i = 8; i < 255; i++)
   {
      data *= 2;
      if ( data & 0xF00 )
      {
         data |= 1;
         data = (data & 0xFF) ^ 0x1C;
      }
		buf[i] = data;
   }
   buf2_ptr = &buf2[1];
   for (j = 1; j < 256; j++)
   {
      buf_ptr = buf;
      for (k = 0; k < 255; k++)
      {
         int val = buf_ptr[0];
         buf_ptr++;
         if ( j == val )
         {
            *buf2_ptr = k;
            buf2_ptr++;
            break;
         }
      }
   }
   eccTable1[0] = eccTable2[0] = 0;
	for (i = 1; i < 256; i++)
   {
      eccTable1[i] = buf[(buf2[i] + 1) % 255];
      eccTable2[i] = buf[(buf2[i] + 230) % 255];
   }
}

void ImgUtil::genLUT()
{
   generateCRCTable(crc_table);
   generateECCTables(ecc_table1, ecc_table2);
}

DWORD ImgUtil::detectImage()
{
	CT2A ascii(filename);
	char *fn=ascii.m_psz;
	FILE *fp;
	char type[128];

   // See if it's a .cue file
   char *p = strrchr(fn, '.');

   if (_stricmp(p, ".cue") == 0)
   {
      // If cue file
      if (fopen_s(&fp, fn, "rt") != 0)
         return false;
		if (fscanf_s(fp, "FILE \"%[^\"]\" %s\r\n", imageFile, sizeof(imageFile), type, sizeof(type)) == EOF || _stricmp(type, "BINARY") != 0)
      {
         fclose(fp);
			return ERROR_BAD_FORMAT;
      }
   }
   else
      strcpy_s(imageFile, sizeof(imageFile), fn);

   // Go through data and establish sector size
   if (fopen_s(&fp, imageFile, "rb") != 0)
	{
		char file[MAX_PATH];
		
		strcpy_s(file, fn);
		char *p = strrchr(file, '\\');
		p[1] = '\0';

		// if file doesn't exist, strip off path and use current directory
		p = strrchr(imageFile, '\\');
		if (p)
		{
			strcat_s(file, p+1);
			if (fopen_s(&fp, file, "rb") != 0)
				return ERROR_FILE_NOT_FOUND;
			else
				strcpy_s(imageFile, file);
		}
		else
			return ERROR_FILE_NOT_FOUND;
	}

   unsigned char *buf=(unsigned char *)malloc(3000);

   if (buf == NULL)
	{
		fclose(fp);
		return ERROR_OUTOFMEMORY;
	}

   // Figure out what kind of file we're dealing with and how to handle it
   if (fp)
   {
      unsigned char sync[12] = { 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x00 };
      int sync_offset[2];
      int sync_count=0;

      fread((void *)buf, 3000, 1, fp);

      for (int i = 0; i < 3000; i++)
      {
         if (memcmp(buf+i, sync, 12) == 0)
         {
            // Sync
            sync_offset[sync_count] = i;
            sync_count++;

            if (sync_count >= 2)
               break;
         }
      }
      if (sync_count == 0)
      {
         // ISO
         sectorSize = 2048;
         generateECC = false;
      }
      else if (sync_count == 2 && sync_offset[1]-sync_offset[0] == 2352)
      {
         sectorSize = 2352;
			trackType = ST_MODE1;
         generateECC = true;
      }
      else
			return ERROR_UNSUPPORTED_TYPE;
      fclose(fp);
   }
   free(buf);

   return ERROR_SUCCESS;
}

void ImgUtil::generateMode1EDC(int *tbl, int *sectorData)
{
   int result;
   int i, j;

   result = -551682158;
	for (i = 3; i < 516; i++)
   {
      result ^= sectorData[i];
		for (j = 0; j < 4; j++)
         result = tbl[result & 0xFF] ^ (result >> 8) & 0xFFFFFF;
   }
   sectorData[i] = result;
	sectorData[i+1] = 0;
	sectorData[i+2] = 0;
}

void ImgUtil::setQ( unsigned char *tbl, unsigned char *a2 )
{
   a2[0] = tbl[4 * a2[0]];
   a2[1] = tbl[4 * a2[1]];
}

void ImgUtil::generateMode1ECC(int *tbl1, int *tbl2, unsigned char *sectorData)
{
	int i,j,k;
	unsigned short *userPtr;
   unsigned short *userPtr2;
	unsigned short *userEndPtr;
	unsigned short *eccPtr1;
   unsigned short *eccPtr2;
	int a, b;

   userPtr = (unsigned short *)(sectorData + 12);
   eccPtr1 = (unsigned short *)(sectorData + 2076);
   eccPtr2 = (unsigned short *)(sectorData + 2162);
   for (i = 0; i < 43; i++)
   {
      userPtr2 = userPtr;
      a = b = 0;
      for (j = 0; j < 24; j++)
      {
         a ^= userPtr2[0];
         b ^= userPtr2[0];
         setQ((unsigned char *)tbl1, (unsigned char *)&b);
         userPtr2 += 43;
      }
      setQ((unsigned char *)tbl1, (unsigned char *)&b);
      b ^= a;
      setQ((unsigned char *)tbl2, (unsigned char *)&b);
      eccPtr1[0] = b;
      eccPtr2[0] = b ^ a;
      eccPtr1++;
      eccPtr2++;
      userPtr++;
   }
	userPtr = (unsigned short *)(sectorData + 12);
   userEndPtr = (unsigned short *)(sectorData + 2248);
   eccPtr1 = (unsigned short *)(sectorData + 2248);
   eccPtr2 = (unsigned short *)(sectorData + 2300);

	for (i = 0; i < 26; i++)
   {
      userPtr2 = userPtr;
		a = b = 0;
      for ( k = 0; k < 43; k++ )
      {
         a ^= userPtr2[0];
         b ^= userPtr2[0];
         setQ((unsigned char *)tbl1, (unsigned char *)&b);
         userPtr2 += 44;
         if ( userPtr2 > userEndPtr )
            userPtr2 -= 1118;
      }
      setQ((unsigned char *)tbl1, (unsigned char *)&b);
      b ^= a;
      setQ((unsigned char *)tbl2, (unsigned char *)&b);
      eccPtr1[0] = b;
      eccPtr2[0] = b ^ a;
      eccPtr1++;
      eccPtr2++;
      userPtr += 43;
   }
}

DWORD ImgUtil::patch( int sector, int offset, unsigned char *data, int size )
{
   unsigned char *sectorData = new unsigned char[sectorSize];
   FILE *fp;
      
   if (fopen_s(&fp, imageFile, "r+b") != 0)
	{
		delete sectorData;
	   return ERROR_FILE_NOT_FOUND;
	}

   fseek(fp, sector * sectorSize, SEEK_SET);
   fread((void *)sectorData, 1, sectorSize, fp);

   // If mode 1, 2352 byte sector offset by 16 to allow for sync header, etc.
   if (trackType == ST_MODE1 && sectorSize == 2352)
      memcpy(sectorData+16+offset, data, size);
   else
      memcpy(sectorData+offset, data, size);

   // Recalculate ecc, etc.
   if (generateECC)
   {
      generateMode1EDC(crc_table, (int *)sectorData);
      generateMode1ECC(ecc_table1, ecc_table2, sectorData);
   }

   fseek(fp, sector * sectorSize, SEEK_SET);
   fwrite(sectorData, 1, sectorSize, fp);
   fclose(fp);
	delete sectorData;
	return ERROR_SUCCESS;
}

void ImgUtil::error(HWND hwnd, LPTSTR func, DWORD error_code) 
{ 
	LPVOID msg_buf;
	LPVOID disp_buf;

	FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER | 
		FORMAT_MESSAGE_FROM_SYSTEM |
		FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL,
		error_code,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPTSTR) &msg_buf,
		0, NULL );

	disp_buf = (LPVOID)LocalAlloc(LMEM_ZEROINIT, 
		(lstrlen((LPCTSTR)msg_buf) + lstrlen((LPCTSTR)func) + 40) * sizeof(TCHAR)); 
	StringCchPrintf((LPTSTR)disp_buf, 
		LocalSize(disp_buf) / sizeof(TCHAR),
		TEXT("Error %s: %s"), 
		func, msg_buf); 
	MessageBox(hwnd, (LPCTSTR)disp_buf, TEXT("Error"), MB_OK); 

	LocalFree(msg_buf);
	LocalFree(disp_buf);
}
