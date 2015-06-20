/*
   miniunz.c
   Version 1.1, February 14h, 2010
   sample part of the MiniZip project - ( http://www.winimage.com/zLibDll/minizip.html )

		Copyright (C) 1998-2010 Gilles Vollant (minizip) ( http://www.winimage.com/zLibDll/minizip.html )

		Modifications of Unzip for Zip64
		Copyright (C) 2007-2008 Even Rouault

		Modifications for Zip64 support on both zip and unzip
		Copyright (C) 2009-2010 Mathias Svensson ( http://result42.com )
 
		Major modifications by Taiki
		Copyright (C) 2011-2015 Taiki ( http://www.taiki.us/ )
*/

#include "unzip.h"
#include "zip.h"

//Unzip
int doExtract(unzFile uf, char *input, char *output_path, bool extractWithoutPath);
int doExtractCurrentfile(unzFile uf, char* filename_inzip, char* output_path, const bool* extractWithoutPath, unsigned char* passwordPageCrypted);
bool doExtractOnefile(unzFile uf, char* filename, char* output_path, bool extractWithoutPath, unsigned char* passwordPageCrypted);

//zip
zipFile * createZip(const char * outfile);
bool addFileToZip(zipFile * zipFile, const char * filename);
bool addMemToZip(zipFile * zipFile, const char * filename, const byte * memoryChunk, const uint64_t chunckSize);
bool addDirToZip(zipFile * zipFile, const char * dirName);
void closeZip(zipFile * zipFile);
