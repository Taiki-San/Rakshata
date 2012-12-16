/*
   miniunz.c
   Version 1.1, February 14h, 2010
   sample part of the MiniZip project - ( http://www.winimage.com/zLibDll/minizip.html )

         Copyright (C) 1998-2010 Gilles Vollant (minizip) ( http://www.winimage.com/zLibDll/minizip.html )

         Modifications of Unzip for Zip64
         Copyright (C) 2007-2008 Even Rouault

         Modifications for Zip64 support on both zip and unzip
         Copyright (C) 2009-2010 Mathias Svensson ( http://result42.com )
*/

/*Stuff used by miniunzip*/
#include "unzip.h"
#define MAXFILENAME (256)

int do_extract_onefile(unzFile uf, const char* filename, int opt_extract_without_path, int opt_overwrite, const char* password, unsigned char* pageOutputCrypted);
int do_extract(unzFile uf, char* input, int opt_extract_without_path, int opt_overwrite, const char* password);
int do_extract_currentfile(unzFile uf, const int* popt_extract_without_path, int* popt_overwrite, const char* password, char* pageOutputCrypted);
void change_file_date(const char *filename, uLong dosdate, tm_unz tmu_date);
void Display64BitsSize(ZPOS64_T n, int size_char);
int makedir (char *newdir);
int mymkdir(const char* dirname);


