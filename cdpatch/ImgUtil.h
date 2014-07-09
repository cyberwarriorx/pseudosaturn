#pragma once

enum { ST_MODE1=1, ST_MODE2_F1, ST_MODE2_F2 };

class ImgUtil
{
private:
   CString filename;
   char imageFile[MAX_PATH];
   int sectorSize;
   int trackType;

   bool generateECC;
   int crc_table[256];
   int ecc_table1[1104];
   int ecc_table2[256];

   bool isBitSet(int val, char bit);
   int orBit(int val, char bit);
   int notBit(int val, char bit);
   void generateCRCTable(int *crctable );
   void generateECCTables(int *ecc_table1, int *ecc_table2);
   void genLUT();
   void generateMode1EDC(int *a1, int *a2);
   void setQ( unsigned char *a1, unsigned char *a2 );
   void generateMode1ECC(int *a1, int *a2, unsigned char *a3);
public:
   ImgUtil(CString filename);
   ~ImgUtil(void);
   DWORD detectImage();
   DWORD patch( int sector, int offset, unsigned char *data, int size );
	void error(HWND hwnd, LPTSTR func, DWORD error_code);
};
