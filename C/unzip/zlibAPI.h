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

enum
{
	STRIP_PATH_NONE = 0,
	STRIP_PATH_ALL,
	STRIP_PATH_FIRST
};

//Unzip
int extractCurrentfile(unzFile zipFile, char* filenameInZip, char* outputPath, int extractWithoutPath, unsigned char* passwordPageCrypted);
bool extractOnefile(unzFile zipFile, char* filename, char* outputPath, bool extractWithoutPath, unsigned char* passwordPageCrypted);
bool listArchiveContent(unzFile uf, char *** filenameInzip, uint * nbFichiers);
bool extractToMem(unzFile zipFile, byte ** output, uint64_t * sizeOutput);

//zip
zipFile * createZip(const char * outfile);
bool addFileToZip(zipFile * zipFile, const char * filename, const char * inzipFilename);
void createDirInZip(zipFile * zipFile, const char * dirName);
bool addMemToZip(zipFile * zipFile, const char * filename, const byte * memoryChunk, const uint64_t chunckSize);
void closeZip(zipFile * zipFile);
