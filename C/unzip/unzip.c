/* unzip.c -- IO for uncompress .zip files using zlib
 Version 1.1, February 14h, 2010
 part of the MiniZip project - ( http://www.winimage.com/zLibDll/minizip.html )

 Copyright (C) 1998-2010 Gilles Vollant (minizip) ( http://www.winimage.com/zLibDll/minizip.html )

 Modifications of Unzip for Zip64
 Copyright (C) 2007-2008 Even Rouault

 Modifications for Zip64 support on both zip and unzip
 Copyright (C) 2009-2010 Mathias Svensson ( http://result42.com )

 For more info read MiniZip_info.txt


 ------------------------------------------------------------------------------------
 Decryption code comes from crypt.c by Info-ZIP but has been greatly reduced in terms of
 compatibility with older software. The following is from the original crypt.c.
 Code woven in by Terry Thorsen 1/2003.

 Copyright (c) 1990-2000 Info-ZIP.  All rights reserved.

 See the accompanying file LICENSE, version 2000-Apr-09 or later
 (the contents of which are also included in zip.h) for terms of use.
 If, for some reason, all these files are missing, the Info-ZIP license
 also may be found at:  ftp://ftp.info-zip.org/pub/infozip/license.html

 crypt.c (full version) by Info-ZIP.      Last revised:  [see crypt.h]

 The encryption/decryption parts of this source code (as opposed to the
 non-echoing password parts) were originally written in Europe.  The
 whole source package can be freely distributed, including from the USA.
 (Prior to January 2000, re-export from the US was a violation of US law.)

 This encryption code is a direct transcription of the algorithm from
 Roger Schlafly, described by Phil Katz in the file appnote.txt.  This
 file (appnote.txt) is distributed with the PKZIP program (even in the
 version without encryption capabilities).

 ------------------------------------------------------------------------------------

 Changes in unzip.c

 2007-2008 - Even Rouault - Addition of cpl_unzGetCurrentFileZStreamPos
 2007-2008 - Even Rouault - Decoration of symbol names unz* -> cpl_unz*
 2007-2008 - Even Rouault - Remove old C style function prototypes
 2007-2008 - Even Rouault - Add unzip support for ZIP64

 Copyright (C) 2007-2008 Even Rouault


 Oct-2009 - Mathias Svensson - Removed cpl_* from symbol names (Even Rouault added them but since this is now moved to a new project (minizip64) I renamed them again).
 Oct-2009 - Mathias Svensson - Fixed problem if uncompressed size was > 4G and compressed size was <4G
 should only read the compressed/uncompressed size from the Zip64 format if
 the size from normal header was 0xFFFFFFFF
 Oct-2009 - Mathias Svensson - Applied some bug fixes from paches recived from Gilles Vollant
 Oct-2009 - Mathias Svensson - Applied support to unzip files with compression mathod BZIP2 (bzip2 lib is required)
 Patch created by Daniel Borca

 Jan-2010 - back to unzip and minizip 1.0 name scheme, with compatibility layer

 Copyright (C) 1998 - 2010 Gilles Vollant, Even Rouault, Mathias Svensson
 
 Copyright (C) 2011 - 2015 Taiki https://www.taiki.us/


 */


#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "zlib/zlib.h"
#include "unzip.h"

#ifdef STDC
#  include <stddef.h>
#  include <string.h>
#  include <stdlib.h>
#endif
#ifdef NO_ERRNO_H
extern int errno;
#else
#   include <errno.h>
#endif


#ifndef CASESENSITIVITYDEFAULT_NO
#  if !defined(unix) && !defined(CASESENSITIVITYDEFAULT_YES)
#    define CASESENSITIVITYDEFAULT_NO
#  endif
#endif


#ifndef UNZ_BUFSIZE
#define UNZ_BUFSIZE (16384)
#endif

#ifndef UNZ_MAXFILENAMEINZIP
#define UNZ_MAXFILENAMEINZIP (256)
#endif

#define SIZECENTRALDIRITEM (0x2e)
#define SIZEZIPLOCALHEADER (0x1e)


const char unz_copyright[] =
" unzip Copyright 1998-20xx Gilles Vollant - http://www.winimage.com/zLibDll";

/* unz_file_info_interntal contain internal info about a file in zipfile*/
typedef struct
{
	uint64_t offset_curfile;/* relative offset of local header 8 bytes */
} unz_file_info64_internal;


/* file_in_zip_read_info_s contain internal information about a file in zipfile,
 when reading and decompress it */
typedef struct
{
	char  *read_buffer;         /* internal buffer for compressed data */
	z_stream stream;            /* zLib stream structure for inflate */

	uint64_t pos_in_zipfile;       /* position in byte on the zipfile, for fseek*/
	uint64_t stream_initialised;   /* flag set if stream structure is initialised*/

	uint64_t offset_local_extrafield;/* offset of the local extra field */
	uint32_t  size_local_extrafield;/* size of the local extra field */
	uint64_t pos_local_extrafield;   /* position in the local extra field in read*/
	uint64_t total_out_64;

	uint64_t _crc32;                /* _crc32 of all data uncompressed */
	uint64_t crc32_wait;           /* _crc32 we must obtain after decompress all */
	uint64_t rest_read_compressed; /* number of byte to be decompressed */
	uint64_t rest_read_uncompressed;/*number of byte to be obtained after decomp*/
	zlib_filefunc64_32_def z_filefunc;
	voidpf filestream;        /* io structore of the zipfile */
	uint64_t compression_method;   /* compression method (0==store) */
	uint64_t byte_before_the_zipfile;/* byte before the zipfile, ( > 0 for sfx)*/
	int   raw;
} file_in_zip64_read_info_s;


/* unz64_s contain internal information about the zipfile
 */
typedef struct
{
	zlib_filefunc64_32_def z_filefunc;
	int is64bitOpenFunction;
	voidpf filestream;        /* io structore of the zipfile */
	unz_global_info64 gi;       /* public global information */
	uint64_t byte_before_the_zipfile;/* byte before the zipfile, ( > 0 for sfx)*/
	uint64_t num_file;             /* number of the current file in the zipfile*/
	uint64_t pos_in_central_dir;   /* pos of the current file in the central dir*/
	uint64_t current_file_ok;      /* flag about the usability of the current file*/
	uint64_t central_pos;          /* position of the beginning of the central dir*/

	uint64_t size_central_dir;     /* size of the central directory  */
	uint64_t offset_central_dir;   /* offset of start of central directory with
									respect to the starting disk number */

	unz_file_info64 cur_file_info; /* public info about the current file in zip*/
	unz_file_info64_internal cur_file_info_internal; /* private info about it*/
	file_in_zip64_read_info_s* pfile_in_zip_read; /* structure about the current
												   file if we are decompressing it */
	int encrypted;

	int isZip64;

} unz64_s;


/* ===========================================================================
 Read a byte from a gz_stream; update next_in and avail_in. Return EOF
 for end of file.
 IN assertion: the stream s has been sucessfully opened for reading.
 */


static int unz64local_getByte(const zlib_filefunc64_32_def* pzlib_filefunc_def, voidpf filestream, byte *pi)
{
	byte c;
	if(ZREAD64(*pzlib_filefunc_def, filestream, &c, 1) == 1)
	{
		*pi = c;
		return UNZ_OK;
	}
	else
	{
		if(ZERROR64(*pzlib_filefunc_def,filestream))
			return UNZ_ERRNO;
		else
			return UNZ_EOF;
	}
}

/* ===========================================================================
 Reads a long in LSB order from the given gz_stream. Sets
 */

static int unz64local_getShort (const zlib_filefunc64_32_def* pzlib_filefunc_def, voidpf filestream, uint16_t * pX)
{
	uint16_t x;
	byte i = 0;
	int err;

	if((err = unz64local_getByte(pzlib_filefunc_def, filestream, &i)) == UNZ_OK)
		x = i;

	if(err == UNZ_OK && (err = unz64local_getByte(pzlib_filefunc_def, filestream, &i)) == UNZ_OK)
		x += ((short) i) << 8;
	else
		x = 0;

	if(pX != NULL)
		*pX = x;

	return err;
}

static int unz64local_getLong (const zlib_filefunc64_32_def* pzlib_filefunc_def, voidpf filestream, uint32_t *pX)
{
	uint32_t x;
	byte i = 0;
	int err;

	if((err = unz64local_getByte(pzlib_filefunc_def, filestream, &i)) == UNZ_OK)
		x = i;

	if(err == UNZ_OK && (err = unz64local_getByte(pzlib_filefunc_def, filestream, &i)) == UNZ_OK)
		x += ((uint32_t) i) << 8;

	if(err == UNZ_OK && (err = unz64local_getByte(pzlib_filefunc_def, filestream, &i)) == UNZ_OK)
		x += ((uint32_t) i) << 16;

	if(err == UNZ_OK && (err = unz64local_getByte(pzlib_filefunc_def, filestream, &i)) == UNZ_OK)
		x += ((uint32_t) i) << 24;
	else
		x = 0;

	if(pX != NULL)
		*pX = x;

	return err;
}

static int unz64local_getLong64 (const zlib_filefunc64_32_def* pzlib_filefunc_def, voidpf filestream, uint64_t *pX)
{
	uint64_t x;
	byte i = 0;
	int err;

	if((err = unz64local_getByte(pzlib_filefunc_def, filestream, &i)) == UNZ_OK)
		x = i;

	if(err == UNZ_OK && (err = unz64local_getByte(pzlib_filefunc_def, filestream, &i)) == UNZ_OK)
		x += ((uint64_t) i) << 8;

	if(err == UNZ_OK && (err = unz64local_getByte(pzlib_filefunc_def, filestream, &i)) == UNZ_OK)
		x += ((uint64_t) i) << 16;

	if(err == UNZ_OK && (err = unz64local_getByte(pzlib_filefunc_def, filestream, &i)) == UNZ_OK)
		x += ((uint64_t) i) << 24;

	if(err == UNZ_OK && (err = unz64local_getByte(pzlib_filefunc_def, filestream, &i)) == UNZ_OK)
		x += ((uint64_t) i) << 32;

	if(err == UNZ_OK && (err = unz64local_getByte(pzlib_filefunc_def, filestream, &i)) == UNZ_OK)
		x += ((uint64_t) i) << 40;

	if(err == UNZ_OK && (err = unz64local_getByte(pzlib_filefunc_def, filestream, &i)) == UNZ_OK)
		x += ((uint64_t) i) << 48;

	if(err == UNZ_OK && (err = unz64local_getByte(pzlib_filefunc_def, filestream, &i)) == UNZ_OK)
		x += ((uint64_t) i) << 56;
	else
		x = 0;

	if(pX != NULL)
		*pX = x;

	return err;
}

/* My own strcmpi / strcasecmp */
static int strcmpcasenosensitive_internal (const char* fileName1, const char* fileName2)
{
	while(1)
	{
		char c1 = *(fileName1++), c2 = *(fileName2++);

		if((c1>='a') && (c1<='z'))
			c1 -= 0x20;

		if((c2>='a') && (c2<='z'))
			c2 -= 0x20;

		if(c1 == '\0')
			return (c2 == '\0' ? 0 : -1);

		if(c2 == '\0')
			return 1;

		if(c1 < c2)
			return -1;

		if(c1 > c2)
			return 1;
	}
}


#ifdef  CASESENSITIVITYDEFAULT_NO
#define CASESENSITIVITYDEFAULTVALUE 2
#else
#define CASESENSITIVITYDEFAULTVALUE 1
#endif

#ifndef STRCMPCASENOSENTIVEFUNCTION
#define STRCMPCASENOSENTIVEFUNCTION strcmpcasenosensitive_internal
#endif

/*
 Compare two filename (fileName1,fileName2).
 If iCaseSenisivity = 1, comparision is case sensitivity (like strcmp)
 If iCaseSenisivity = 2, comparision is not case sensitivity (like strcmpi
 or strcasecmp)
 If iCaseSenisivity = 0, case sensitivity is defaut of your operating system
 (like 1 on Unix, 2 on Windows)

 */
extern int  unzStringFileNameCompare (const char*  fileName1, const char*  fileName2, int iCaseSensitivity)
{
	if(iCaseSensitivity == 0)
		iCaseSensitivity = CASESENSITIVITYDEFAULTVALUE;

	if(iCaseSensitivity==1)
		return strcmp(fileName1,fileName2);

	return STRCMPCASENOSENTIVEFUNCTION(fileName1,fileName2);
}

#ifndef BUFREADCOMMENT
#define BUFREADCOMMENT (0x400)
#endif

/*
 Locate the Central directory of a zipfile (at the end, just before
 the global comment)
 */
static uint64_t unz64local_SearchCentralDir(const zlib_filefunc64_32_def* pzlib_filefunc_def, voidpf filestream)
{
	unsigned char* buf;
	uint64_t uSizeFile;
	uint64_t uBackRead;
	uint64_t uMaxBack = 0xffff; /* maximum size of global comment */
	uint64_t uPosFound = 0;

	if(ZSEEK64(*pzlib_filefunc_def,filestream,0,ZLIB_FILEFUNC_SEEK_END) != 0)
		return 0;


	uSizeFile = ZTELL64(*pzlib_filefunc_def,filestream);

	if(uMaxBack>uSizeFile)
		uMaxBack = uSizeFile;

	buf = (unsigned char*)malloc(BUFREADCOMMENT+4);
	if(buf == NULL)
		return 0;

	uBackRead = 4;
	while (uBackRead<uMaxBack)
	{
		uint64_t uReadSize;
		uint64_t uReadPos;
		int i;
		if(uBackRead+BUFREADCOMMENT>uMaxBack)
			uBackRead = uMaxBack;
		else
			uBackRead+=BUFREADCOMMENT;
		uReadPos = uSizeFile-uBackRead;

		uReadSize = ((BUFREADCOMMENT+4) < (uSizeFile-uReadPos)) ?
		(BUFREADCOMMENT+4) : (uint64_t)(uSizeFile-uReadPos);
		if(ZSEEK64(*pzlib_filefunc_def,filestream,uReadPos,ZLIB_FILEFUNC_SEEK_SET) != 0)
			break;

		if(ZREAD64(*pzlib_filefunc_def,filestream,buf,uReadSize) != uReadSize)
			break;

		for (i = (int) uReadSize - 3; i-- > 0; )
		{
			if(((*(buf+i)) == 0x50) && ((*(buf+i+1)) == 0x4b) &&
			   ((*(buf+i+2)) == 0x05) && ((*(buf+i+3)) == 0x06))
			{
				uPosFound = uReadPos + (uint) i;
				break;
			}
		}

		if(uPosFound != 0)
			break;
	}
	free(buf);
	return uPosFound;
}


/*
 Locate the Central directory 64 of a zipfile (at the end, just before
 the global comment)
 */

static uint64_t unz64local_SearchCentralDir64(const zlib_filefunc64_32_def* pzlib_filefunc_def, voidpf filestream)
{
	unsigned char* buf;
	uint64_t uSizeFile;
	uint64_t uBackRead;
	uint64_t uMaxBack = 0xffff; /* maximum size of global comment */
	uint64_t uPosFound = 0;
	uint32_t uL;
	uint64_t relativeOffset;

	if(ZSEEK64(*pzlib_filefunc_def,filestream,0,ZLIB_FILEFUNC_SEEK_END) != 0)
		return 0;

	uSizeFile = ZTELL64(*pzlib_filefunc_def,filestream);

	if(uMaxBack>uSizeFile)
		uMaxBack = uSizeFile;

	buf = (unsigned char*)malloc(BUFREADCOMMENT+4);
	if(buf == NULL)
		return 0;

	uBackRead = 4;
	while (uBackRead<uMaxBack)
	{
		uint64_t uReadSize;
		uint64_t uReadPos;
		int i;
		if(uBackRead+BUFREADCOMMENT>uMaxBack)
			uBackRead = uMaxBack;
		else
			uBackRead+=BUFREADCOMMENT;
		uReadPos = uSizeFile-uBackRead;

		uReadSize = ((BUFREADCOMMENT+4) < (uSizeFile-uReadPos)) ?
		(BUFREADCOMMENT+4) : (uint64_t)(uSizeFile-uReadPos);
		if(ZSEEK64(*pzlib_filefunc_def,filestream,uReadPos,ZLIB_FILEFUNC_SEEK_SET) != 0)
			break;

		if(ZREAD64(*pzlib_filefunc_def,filestream,buf,uReadSize) != uReadSize)
			break;

		for (i = (int)uReadSize-3; (i--) > 0;)
			if(((*(buf+i)) == 0x50) && ((*(buf+i+1)) == 0x4b) &&
			   ((*(buf+i+2)) == 0x06) && ((*(buf+i+3)) == 0x07))
			{
				uPosFound = uReadPos + (uint) i;
				break;
			}

		if(uPosFound != 0)
			break;
	}
	free(buf);
	if(uPosFound == 0)
		return 0;

	/* Zip64 end of central directory locator */
	if(ZSEEK64(*pzlib_filefunc_def,filestream, uPosFound,ZLIB_FILEFUNC_SEEK_SET) != 0)
		return 0;

	/* the signature, already checked */
	if(unz64local_getLong(pzlib_filefunc_def,filestream,&uL) != UNZ_OK)
		return 0;

	/* number of the disk with the start of the zip64 end of  central directory */
	if(unz64local_getLong(pzlib_filefunc_def,filestream,&uL) != UNZ_OK)
		return 0;
	if(uL != 0)
		return 0;

	/* relative offset of the zip64 end of central directory record */
	if(unz64local_getLong64(pzlib_filefunc_def,filestream,&relativeOffset) != UNZ_OK)
		return 0;

	/* total number of disks */
	if(unz64local_getLong(pzlib_filefunc_def,filestream,&uL) != UNZ_OK)
		return 0;
	if(uL != 1)
		return 0;

	/* Goto end of central directory record */
	if(ZSEEK64(*pzlib_filefunc_def,filestream, relativeOffset,ZLIB_FILEFUNC_SEEK_SET) != 0)
		return 0;

	/* the signature */
	if(unz64local_getLong(pzlib_filefunc_def,filestream,&uL) != UNZ_OK)
		return 0;

	if(uL != 0x06064b50)
		return 0;

	return relativeOffset;
}

/*
 Open a Zip file. path contain the full pathname (by example,
 on a Windows NT computer "c:\\test\\zlib114.zip" or on an Unix computer
 "zlib/zlib114.zip".
 If the zipfile cannot be opened (file doesn't exist or in not valid), the
 return value is NULL.
 Else, the return value is a unzFile Handle, usable with other function
 of this unzip package.
 */
static unzFile unzOpenInternal (const void *path, zlib_filefunc64_32_def* pzlib_filefunc64_32_def, int is64bitOpenFunction)
{
	unz64_s us;
	unz64_s *s;
	uint64_t central_pos = 0;

	int err = UNZ_OK;

	if(pzlib_filefunc64_32_def == NULL)
		fill_fopen64_filefunc(&us.z_filefunc.zfile_func64);
	else
		us.z_filefunc = *pzlib_filefunc64_32_def;

	us.is64bitOpenFunction = is64bitOpenFunction;
	us.filestream = ZOPEN64(us.z_filefunc, path, ZLIB_FILEFUNC_MODE_READ | ZLIB_FILEFUNC_MODE_EXISTING);

	if(us.filestream == NULL)
		return NULL;

	central_pos = unz64local_SearchCentralDir64(&us.z_filefunc,us.filestream);
	if(central_pos)
	{
		uint32_t number_disk = 0;          /* number of the current dist, used for spaning ZIP, unsupported, always 0*/
		uint32_t number_disk_with_CD = 0;  /* number the the disk with central dir, used for spaning ZIP, unsupported, always 0*/
		uint64_t number_entry_CD = 0;      /* total number of entries in the central dir (same than number_entry on nospan) */

		us.isZip64 = 1;

		if(ZSEEK64(us.z_filefunc, us.filestream, central_pos,ZLIB_FILEFUNC_SEEK_SET) != 0)
			err = UNZ_ERRNO;

		/* the signature, already checked */
		if(unz64local_getLong(&us.z_filefunc, us.filestream, NULL) != UNZ_OK)
			err = UNZ_ERRNO;

		/* size of zip64 end of central directory record */
		if(unz64local_getLong64(&us.z_filefunc, us.filestream, NULL) != UNZ_OK)
			err = UNZ_ERRNO;

		/* version made by */
		if(unz64local_getShort(&us.z_filefunc, us.filestream, NULL) != UNZ_OK)
			err = UNZ_ERRNO;

		/* version needed to extract */
		if(unz64local_getShort(&us.z_filefunc, us.filestream, NULL) != UNZ_OK)
			err = UNZ_ERRNO;

		/* number of this disk */
		if(unz64local_getLong(&us.z_filefunc, us.filestream, &number_disk) != UNZ_OK)
			err = UNZ_ERRNO;

		/* number of the disk with the start of the central directory */
		if(unz64local_getLong(&us.z_filefunc, us.filestream,&number_disk_with_CD) != UNZ_OK)
			err = UNZ_ERRNO;

		/* total number of entries in the central directory on this disk */
		if(unz64local_getLong64(&us.z_filefunc, us.filestream,&us.gi.number_entry) != UNZ_OK)
			err = UNZ_ERRNO;

		/* total number of entries in the central directory */
		if(unz64local_getLong64(&us.z_filefunc, us.filestream,&number_entry_CD) != UNZ_OK)
			err = UNZ_ERRNO;

		if(number_entry_CD != us.gi.number_entry || number_disk_with_CD != 0 || number_disk != 0)
			err = UNZ_BADZIPFILE;

		/* size of the central directory */
		if(unz64local_getLong64(&us.z_filefunc, us.filestream,&us.size_central_dir) != UNZ_OK)
			err = UNZ_ERRNO;

		/* offset of start of central directory with respect to the
		 starting disk number */
		if(unz64local_getLong64(&us.z_filefunc, us.filestream,&us.offset_central_dir) != UNZ_OK)
			err = UNZ_ERRNO;

		us.gi.size_comment = 0;
	}
	else
	{
		central_pos = unz64local_SearchCentralDir(&us.z_filefunc,us.filestream);	//Checké, devrait fonctionner

		if(central_pos == 0)
			err = UNZ_ERRNO;

		us.isZip64 = 0;

		uint16_t number_disk, number_disk_with_CD, number_entry_CD;

		if(ZSEEK64(us.z_filefunc, us.filestream, central_pos, ZLIB_FILEFUNC_SEEK_SET) != 0)
			err = UNZ_ERRNO;

		/* the signature, already checked */
		if(unz64local_getLong(&us.z_filefunc, us.filestream, NULL) != UNZ_OK)
			err = UNZ_ERRNO;

		/* number of this disk */
		if(unz64local_getShort(&us.z_filefunc, us.filestream, &number_disk) != UNZ_OK)
			err = UNZ_ERRNO;

		/* number of the disk with the start of the central directory */
		if(unz64local_getShort(&us.z_filefunc, us.filestream, &number_disk_with_CD) != UNZ_OK)
			err = UNZ_ERRNO;

		/* total number of entries in the central dir on this disk */
		us.gi.number_entry = 0;
		if(unz64local_getShort(&us.z_filefunc, us.filestream, (uint16_t *) &(us.gi.number_entry)) != UNZ_OK)
			err = UNZ_ERRNO;

		/* total number of entries in the central dir */
		if(unz64local_getShort(&us.z_filefunc, us.filestream, &number_entry_CD) != UNZ_OK)
			err = UNZ_ERRNO;

		if(number_entry_CD != us.gi.number_entry || number_disk_with_CD != 0 || number_disk != 0)
			err = UNZ_BADZIPFILE;

		/* size of the central directory */
		us.size_central_dir = 0;
		if(unz64local_getLong(&us.z_filefunc, us.filestream, (uint32_t *) &(us.size_central_dir)) != UNZ_OK)
			err = UNZ_ERRNO;

		/* offset of start of central directory with respect to the starting disk number */
		us.offset_central_dir = 0;
		if(unz64local_getLong(&us.z_filefunc, us.filestream, (uint32_t *) &(us.offset_central_dir)) != UNZ_OK)
			err = UNZ_ERRNO;

		/* zipfile comment length */
		us.gi.size_comment = 0;
		if(unz64local_getShort(&us.z_filefunc, us.filestream, (uint16_t *) &us.gi.size_comment) != UNZ_OK)
			err = UNZ_ERRNO;
	}

	if(err == UNZ_OK && central_pos < us.offset_central_dir + us.size_central_dir)
		err = UNZ_BADZIPFILE;

	if(err != UNZ_OK)
	{
		ZCLOSE64(us.z_filefunc, us.filestream);
		return NULL;
	}

	us.byte_before_the_zipfile = central_pos - (us.offset_central_dir + us.size_central_dir);
	us.central_pos = central_pos;
	us.pfile_in_zip_read = NULL;
	us.encrypted = 0;

	s = (unz64_s*) malloc(sizeof(unz64_s));

	if(s != NULL)
	{
		*s = us;
		unzGoToFirstFile((unzFile) s);
	}

	return (unzFile) s;
}


extern unzFile  unzOpen2 (const char *path, zlib_filefunc_def* pzlib_filefunc32_def)
{
	if(pzlib_filefunc32_def != NULL)
	{
		zlib_filefunc64_32_def zlib_filefunc64_32_def_fill;
		fill_zlib_filefunc64_32_def_from_filefunc32(&zlib_filefunc64_32_def_fill,pzlib_filefunc32_def);
		return unzOpenInternal(path, &zlib_filefunc64_32_def_fill, 0);
	}
	else
		return unzOpenInternal(path, NULL, 0);
}

extern unzFile  unzOpen2_64 (const void *path,
							 zlib_filefunc64_def* pzlib_filefunc_def)
{
	if(pzlib_filefunc_def != NULL)
	{
		zlib_filefunc64_32_def zlib_filefunc64_32_def_fill;
		zlib_filefunc64_32_def_fill.zfile_func64 = *pzlib_filefunc_def;
		zlib_filefunc64_32_def_fill.ztell32_file = NULL;
		zlib_filefunc64_32_def_fill.zseek32_file = NULL;
		return unzOpenInternal(path, &zlib_filefunc64_32_def_fill, 1);
	}
	else
		return unzOpenInternal(path, NULL, 1);
}

extern unzFile  unzOpen (const char *path)
{
	return unzOpenInternal(path, NULL, 0);
}

extern unzFile  unzOpen64 (const void *path)
{
	return unzOpenInternal(path, NULL, 1);
}

/*
 Close a ZipFile opened with unzOpen.
 If there is files inside the .Zip opened with unzOpenCurrentFile (see later),
 these files MUST be closed with unzCloseCurrentFile before call unzClose.
 return UNZ_OK if there is no problem. */
extern int  unzClose (unzFile file)
{
	unz64_s* s;
	if(file == NULL)
		return UNZ_PARAMERROR;
	s = (unz64_s*)file;

	if(s->pfile_in_zip_read != NULL)
		unzCloseCurrentFile(file);

	ZCLOSE64(s->z_filefunc, s->filestream);
	free(s);
	return UNZ_OK;
}


/*
 Write info about the ZipFile in the *pglobal_info structure.
 No preparation of the structure is needed
 return UNZ_OK if there is no problem. */
extern int  unzGetGlobalInfo64 (unzFile file, unz_global_info64* pglobal_info)
{
	unz64_s* s;
	if(file == NULL)
		return UNZ_PARAMERROR;
	s = (unz64_s*)file;
	*pglobal_info=s->gi;
	return UNZ_OK;
}

extern int  unzGetGlobalInfo (unzFile file, unz_global_info* pglobal_info32)
{
	unz64_s* s;
	if(file == NULL)
		return UNZ_PARAMERROR;
	s = (unz64_s*)file;
	/* to do : check if number_entry is not truncated */
	pglobal_info32->number_entry = (uint64_t)s->gi.number_entry;
	pglobal_info32->size_comment = s->gi.size_comment;
	return UNZ_OK;
}
/*
 Translate date/time from Dos format to tm_unz (readable more easilty)
 */
static void unz64local_DosDateToTmuDate (uint64_t ulDosDate, tm_unz* ptm)
{
	uint64_t uDate;
	uDate = (uint64_t)(ulDosDate>>16);
	ptm->tm_mday = (uint32_t)(uDate&0x1f);
	ptm->tm_mon =  (uint32_t)((((uDate)&0x1E0)/0x20)-1);
	ptm->tm_year = (uint32_t)(((uDate&0x0FE00)/0x0200)+1980);

	ptm->tm_hour = (uint32_t) ((ulDosDate &0xF800)/0x800);
	ptm->tm_min =  (uint32_t) ((ulDosDate&0x7E0)/0x20);
	ptm->tm_sec =  (uint32_t) (2*(ulDosDate&0x1f));
}

/*
 Get Info about the current file in the zipfile, with internal only info
 */
static int unz64local_GetCurrentFileInfoInternal (unzFile file, unz_file_info64 *pfile_info, unz_file_info64_internal *pfile_info_internal, char *szFileName, uint64_t fileNameBufferSize, void *extraField, uint64_t extraFieldBufferSize, char *szComment, uint64_t commentBufferSize)
{
	unz64_s* s;
	unz_file_info64 file_info;
	unz_file_info64_internal file_info_internal;
	int err = UNZ_OK;
	uint32_t uMagic, uL;
	long lSeek = 0;

	if(file == NULL)
		return UNZ_PARAMERROR;

	s = (unz64_s*)file;
	if(ZSEEK64(s->z_filefunc, s->filestream, s->pos_in_central_dir + s->byte_before_the_zipfile, ZLIB_FILEFUNC_SEEK_SET) != 0)
		err = UNZ_ERRNO;

	memset(&file_info, 0, sizeof(file_info));

	/* we check the magic */
	if(err == UNZ_OK)
	{
		if(unz64local_getLong(&s->z_filefunc, s->filestream,&uMagic) != UNZ_OK)
			err = UNZ_ERRNO;
		else if(uMagic != 0x02014b50)
			err = UNZ_BADZIPFILE;
	}

	if(unz64local_getShort(&s->z_filefunc, s->filestream, (uint16_t *) &file_info.version) != UNZ_OK)
		err = UNZ_ERRNO;

	if(unz64local_getShort(&s->z_filefunc, s->filestream, (uint16_t *) &file_info.version_needed) != UNZ_OK)
		err = UNZ_ERRNO;

	if(unz64local_getShort(&s->z_filefunc, s->filestream, (uint16_t *) &file_info.flag) != UNZ_OK)
		err = UNZ_ERRNO;

	if(unz64local_getShort(&s->z_filefunc, s->filestream, (uint16_t *) &file_info.compression_method) != UNZ_OK)
		err = UNZ_ERRNO;

	if(unz64local_getLong(&s->z_filefunc, s->filestream, (uint32_t *) &file_info.dosDate) != UNZ_OK)
		err = UNZ_ERRNO;

	unz64local_DosDateToTmuDate(file_info.dosDate,&file_info.tmu_date);

	if(unz64local_getLong(&s->z_filefunc, s->filestream, (uint32_t *) &file_info.crc) != UNZ_OK)
		err = UNZ_ERRNO;

	if(unz64local_getLong(&s->z_filefunc, s->filestream, &uL) != UNZ_OK)
		err = UNZ_ERRNO;
	file_info.compressed_size = uL;

	if(unz64local_getLong(&s->z_filefunc, s->filestream,&uL) != UNZ_OK)
		err = UNZ_ERRNO;
	file_info.uncompressed_size = uL;

	if(unz64local_getShort(&s->z_filefunc, s->filestream, (uint16_t *) &file_info.size_filename) != UNZ_OK)
		err = UNZ_ERRNO;

	if(unz64local_getShort(&s->z_filefunc, s->filestream, (uint16_t *) &file_info.size_file_extra) != UNZ_OK)
		err = UNZ_ERRNO;

	if(unz64local_getShort(&s->z_filefunc, s->filestream, (uint16_t *) &file_info.size_file_comment) != UNZ_OK)
		err = UNZ_ERRNO;

	if(unz64local_getShort(&s->z_filefunc, s->filestream, (uint16_t *) &file_info.disk_num_start) != UNZ_OK)
		err = UNZ_ERRNO;

	if(unz64local_getShort(&s->z_filefunc, s->filestream, (uint16_t *) &file_info.internal_fa) != UNZ_OK)
		err = UNZ_ERRNO;

	if(unz64local_getLong(&s->z_filefunc, s->filestream, (uint32_t *) &file_info.external_fa) != UNZ_OK)
		err = UNZ_ERRNO;

	// relative offset of local header
	if(unz64local_getLong(&s->z_filefunc, s->filestream,&uL) != UNZ_OK)
		err = UNZ_ERRNO;

	file_info_internal.offset_curfile = uL;

	lSeek += file_info.size_filename;
	if(err == UNZ_OK && szFileName != NULL)
	{
		uint64_t uSizeRead;
		if(file_info.size_filename < fileNameBufferSize)
		{
			*(szFileName+file_info.size_filename)='\0';
			uSizeRead = file_info.size_filename;
		}
		else
			uSizeRead = fileNameBufferSize;

		if(file_info.size_filename > 0 && fileNameBufferSize > 0 && ZREAD64(s->z_filefunc, s->filestream, szFileName, uSizeRead) != uSizeRead)
			err = UNZ_ERRNO;

		lSeek -= uSizeRead;
	}
	else
	{
		ZSEEK64(s->z_filefunc, s->filestream, file_info.size_filename, ZLIB_FILEFUNC_SEEK_CUR);
	}

	// Read extrafield
	if(err == UNZ_OK && extraField != NULL)
	{
		uint64_t uSizeRead;
		if(file_info.size_file_extra < extraFieldBufferSize)
			uSizeRead = file_info.size_file_extra;
		else
			uSizeRead = extraFieldBufferSize;

		if(lSeek != 0)
		{
			if(ZSEEK64(s->z_filefunc, s->filestream, (uint64_t) lSeek, ZLIB_FILEFUNC_SEEK_CUR) == 0)
				lSeek = 0;
			else
				err = UNZ_ERRNO;
		}

		if(file_info.size_file_extra > 0 && extraFieldBufferSize > 0 && ZREAD64(s->z_filefunc, s->filestream,extraField, (uint64_t) uSizeRead) != uSizeRead)
			err = UNZ_ERRNO;

		lSeek += file_info.size_file_extra - (uint64_t)uSizeRead;
	}
	else
		lSeek += file_info.size_file_extra;


	if(err == UNZ_OK && file_info.size_file_extra != 0)
	{
		uint64_t acc = 0;

		// since lSeek now points to after the extra field we need to move back
		lSeek -= file_info.size_file_extra;

		if(lSeek != 0)
		{
			if(ZSEEK64(s->z_filefunc, s->filestream, (uint64_t) lSeek, ZLIB_FILEFUNC_SEEK_CUR) == 0)
				lSeek = 0;
			else
				err = UNZ_ERRNO;
		}

		while(acc < file_info.size_file_extra)
		{
			uint16_t headerId, dataSize;

			if(unz64local_getShort(&s->z_filefunc, s->filestream,&headerId) != UNZ_OK)
				err = UNZ_ERRNO;

			if(unz64local_getShort(&s->z_filefunc, s->filestream,&dataSize) != UNZ_OK)
				err = UNZ_ERRNO;

			/* ZIP64 extra fields */
			if(headerId == 0x0001)
			{
				uint32_t uL2;

				if(file_info.uncompressed_size == UINT32_MAX)
				{
					if(unz64local_getLong64(&s->z_filefunc, s->filestream,&file_info.uncompressed_size) != UNZ_OK)
						err = UNZ_ERRNO;
				}

				if(file_info.compressed_size == UINT32_MAX)
				{
					if(unz64local_getLong64(&s->z_filefunc, s->filestream,&file_info.compressed_size) != UNZ_OK)
						err = UNZ_ERRNO;
				}

				if(file_info_internal.offset_curfile == UINT32_MAX)
				{
					/* Relative Header offset */
					if(unz64local_getLong64(&s->z_filefunc, s->filestream,&file_info_internal.offset_curfile) != UNZ_OK)
						err = UNZ_ERRNO;
				}

				if(file_info.disk_num_start == UINT32_MAX)
				{
					/* Disk Start Number */
					if(unz64local_getLong(&s->z_filefunc, s->filestream, &uL2) != UNZ_OK)
						err = UNZ_ERRNO;
				}
			}
			else
			{
				if(ZSEEK64(s->z_filefunc, s->filestream,dataSize,ZLIB_FILEFUNC_SEEK_CUR) != 0)
					err = UNZ_ERRNO;
			}

			acc += 2 + 2 + dataSize;
		}
	}

	if(err == UNZ_OK && szComment != NULL)
	{
		uint64_t uSizeRead;
		if(file_info.size_file_comment<commentBufferSize)
		{
			*(szComment+file_info.size_file_comment)='\0';
			uSizeRead = file_info.size_file_comment;
		}
		else
			uSizeRead = commentBufferSize;

		if(lSeek != 0)
		{
			if(ZSEEK64(s->z_filefunc, s->filestream, (uint64_t) lSeek,ZLIB_FILEFUNC_SEEK_CUR) != 0)
				err = UNZ_ERRNO;
		}

		if(file_info.size_file_comment > 0 && commentBufferSize > 0 && ZREAD64(s->z_filefunc, s->filestream,szComment,uSizeRead) != uSizeRead)
				err = UNZ_ERRNO;
	}

	if(err == UNZ_OK && pfile_info != NULL)
		*pfile_info=file_info;

	if(err == UNZ_OK && pfile_info_internal != NULL)
		*pfile_info_internal=file_info_internal;

	return err;
}



/*
 Write info about the ZipFile in the *pglobal_info structure.
 No preparation of the structure is needed
 return UNZ_OK if there is no problem.
 */
extern int  unzGetCurrentFileInfo64 (unzFile file,
									 unz_file_info64 * pfile_info,
									 char * szFileName, uint64_t fileNameBufferSize,
									 void *extraField, uint64_t extraFieldBufferSize,
									 char* szComment,  uint64_t commentBufferSize)
{
	return unz64local_GetCurrentFileInfoInternal(file,pfile_info,NULL,
												 szFileName,fileNameBufferSize,
												 extraField,extraFieldBufferSize,
												 szComment,commentBufferSize);
}

extern int  unzGetCurrentFileInfo (unzFile file,
								   unz_file_info * pfile_info,
								   char * szFileName, uint64_t fileNameBufferSize,
								   void *extraField, uint64_t extraFieldBufferSize,
								   char* szComment,  uint64_t commentBufferSize)
{
	int err;
	unz_file_info64 file_info64;
	err = unz64local_GetCurrentFileInfoInternal(file,&file_info64,NULL,
												szFileName,fileNameBufferSize,
												extraField,extraFieldBufferSize,
												szComment,commentBufferSize);
	if((err == UNZ_OK) && (pfile_info != NULL))
	{
		pfile_info->version = file_info64.version;
		pfile_info->version_needed = file_info64.version_needed;
		pfile_info->flag = file_info64.flag;
		pfile_info->compression_method = file_info64.compression_method;
		pfile_info->dosDate = file_info64.dosDate;
		pfile_info->crc = file_info64.crc;

		pfile_info->size_filename = file_info64.size_filename;
		pfile_info->size_file_extra = file_info64.size_file_extra;
		pfile_info->size_file_comment = file_info64.size_file_comment;

		pfile_info->disk_num_start = file_info64.disk_num_start;
		pfile_info->internal_fa = file_info64.internal_fa;
		pfile_info->external_fa = file_info64.external_fa;

		pfile_info->tmu_date = file_info64.tmu_date,


		pfile_info->compressed_size = (uint64_t)file_info64.compressed_size;
		pfile_info->uncompressed_size = (uint64_t)file_info64.uncompressed_size;

	}
	return err;
}
/*
 Set the current file of the zipfile to the first file.
 return UNZ_OK if there is no problem
 */
extern int  unzGoToFirstFile (unzFile file)
{
	unz64_s* s;

	if(file == NULL)
		return UNZ_PARAMERROR;

	s = (unz64_s*)file;
	s->pos_in_central_dir=s->offset_central_dir;
	s->num_file = 0;

	int err = unz64local_GetCurrentFileInfoInternal(file,&s->cur_file_info, &s->cur_file_info_internal, NULL, 0, NULL, 0, NULL, 0);

	s->current_file_ok = err == UNZ_OK;

	return err;
}

/*
 Set the current file of the zipfile to the next file.
 return UNZ_OK if there is no problem
 return UNZ_END_OF_LIST_OF_FILE if the actual file was the latest.
 */
extern int  unzGoToNextFile (unzFile  file)
{
	unz64_s* s;
	int err;

	if(file == NULL)
		return UNZ_PARAMERROR;
	s = (unz64_s*)file;
	if(!s->current_file_ok)
		return UNZ_END_OF_LIST_OF_FILE;
	if(s->gi.number_entry != 0xffff)    /* 2^16 files overflow hack */
		if(s->num_file+1==s->gi.number_entry)
			return UNZ_END_OF_LIST_OF_FILE;

	s->pos_in_central_dir += SIZECENTRALDIRITEM + s->cur_file_info.size_filename +
	s->cur_file_info.size_file_extra + s->cur_file_info.size_file_comment;
	s->num_file++;
	err = unz64local_GetCurrentFileInfoInternal(file,&s->cur_file_info,
												&s->cur_file_info_internal,
												NULL,0,NULL,0,NULL,0);
	s->current_file_ok = (err == UNZ_OK);
	return err;
}


/*
 Try locate the file szFileName in the zipfile.
 For the iCaseSensitivity signification, see unzStringFileNameCompare

 return value :
 UNZ_OK if the file is found. It becomes the current file.
 UNZ_END_OF_LIST_OF_FILE if the file is not found
 */
extern int  unzLocateFile (unzFile file, const char *szFileName, int iCaseSensitivity)
{
	unz64_s* s;
	int err;

	/* We remember the 'current' position in the file so that we can jump
	 * back there if we fail.
	 */
	unz_file_info64 cur_file_infoSaved;
	unz_file_info64_internal cur_file_info_internalSaved;
	uint64_t num_fileSaved;
	uint64_t pos_in_central_dirSaved;


	if(file == NULL)
		return UNZ_PARAMERROR;

	if(strlen(szFileName) >= UNZ_MAXFILENAMEINZIP)
		return UNZ_PARAMERROR;

	s = (unz64_s*)file;
	if(!s->current_file_ok)
		return UNZ_END_OF_LIST_OF_FILE;

	/* Save the current state */
	num_fileSaved = s->num_file;
	pos_in_central_dirSaved = s->pos_in_central_dir;
	cur_file_infoSaved = s->cur_file_info;
	cur_file_info_internalSaved = s->cur_file_info_internal;

	bool firstPass = true;
	do
	{
		char szCurrentFileName[UNZ_MAXFILENAMEINZIP+1];
		if((err = unzGetCurrentFileInfo64(file, NULL, szCurrentFileName, sizeof(szCurrentFileName) - 1, NULL, 0, NULL, 0)) == UNZ_OK)
		{
			if(unzStringFileNameCompare(szCurrentFileName, szFileName, iCaseSensitivity) == 0)
				return UNZ_OK;

			err = unzGoToNextFile(file);
		}

		if(firstPass && err == UNZ_END_OF_LIST_OF_FILE)
		{
			firstPass = false;
			err = unzGoToFirstFile(file);
		}
	} while (err == UNZ_OK);

	/* We failed, so restore the state of the 'current file' to where we
	 * were.
	 */
	s->num_file = num_fileSaved;
	s->pos_in_central_dir = pos_in_central_dirSaved;
	s->cur_file_info = cur_file_infoSaved;
	s->cur_file_info_internal = cur_file_info_internalSaved;
	return err;
}


/*
 ///////////////////////////////////////////
 // Contributed by Ryan Haksi (mailto://cryogen@infoserve.net)
 // I need random access
 //
 // Further optimization could be realized by adding an ability
 // to cache the directory in memory. The goal being a single
 // comprehensive file read to put the file I need in a memory.
 */

/*
 typedef struct
 {
 uint64_t pos_in_zip_directory;   // offset in file
 uint64_t num_of_file;            // # of file
 } unz_file_pos;
 */

extern int  unzGetFilePos64(unzFile file, unz64_file_pos*  file_pos)
{
	unz64_s* s;

	if(file == NULL || file_pos == NULL)
		return UNZ_PARAMERROR;
	s = (unz64_s*)file;
	if(!s->current_file_ok)
		return UNZ_END_OF_LIST_OF_FILE;

	file_pos->pos_in_zip_directory  = s->pos_in_central_dir;
	file_pos->num_of_file           = s->num_file;

	return UNZ_OK;
}

extern int  unzGetFilePos(unzFile file, unz_file_pos* file_pos)
{
	unz64_file_pos file_pos64;
	int err = unzGetFilePos64(file,&file_pos64);
	if(err == UNZ_OK)
	{
		file_pos->pos_in_zip_directory = (uint64_t)file_pos64.pos_in_zip_directory;
		file_pos->num_of_file = (uint64_t)file_pos64.num_of_file;
	}
	return err;
}

extern int  unzGoToFilePos64(unzFile file, const unz64_file_pos* file_pos)
{
	unz64_s* s;
	int err;

	if(file == NULL || file_pos == NULL)
		return UNZ_PARAMERROR;
	s = (unz64_s*)file;

	/* jump to the right spot */
	s->pos_in_central_dir = file_pos->pos_in_zip_directory;
	s->num_file           = file_pos->num_of_file;

	/* set the current file */
	err = unz64local_GetCurrentFileInfoInternal(file,&s->cur_file_info,
												&s->cur_file_info_internal,
												NULL,0,NULL,0,NULL,0);
	/* return results */
	s->current_file_ok = (err == UNZ_OK);
	return err;
}

extern int  unzGoToFilePos(unzFile file, unz_file_pos* file_pos)
{
	unz64_file_pos file_pos64;
	if(file_pos == NULL)
		return UNZ_PARAMERROR;

	file_pos64.pos_in_zip_directory = file_pos->pos_in_zip_directory;
	file_pos64.num_of_file = file_pos->num_of_file;
	return unzGoToFilePos64(file,&file_pos64);
}

/*
 // Unzip Helper Functions - should be here?
 ///////////////////////////////////////////
 */

/*
 Read the local header of the current zipfile
 Check the coherency of the local header and info in the end of central
 directory about this file
 store in *piSizeVar the size of extra info in local header
 (filename and size of extra field data)
 */
static int unz64local_CheckCurrentFileCoherencyHeader (unz64_s* s, uint32_t* piSizeVar, uint64_t * poffset_local_extrafield, uint32_t  * psize_local_extrafield)
{
	uint16_t sData, sFlags, size_filename, size_extra_field;
	uint32_t uMagic, uData;
	int err = UNZ_OK;

	*piSizeVar = 0;
	*poffset_local_extrafield = 0;
	*psize_local_extrafield = 0;

	if(ZSEEK64(s->z_filefunc, s->filestream,s->cur_file_info_internal.offset_curfile + s->byte_before_the_zipfile, ZLIB_FILEFUNC_SEEK_SET) != 0)
		return UNZ_ERRNO;

	if(err == UNZ_OK)
	{
		if(unz64local_getLong(&s->z_filefunc, s->filestream, &uMagic) != UNZ_OK)
			err = UNZ_ERRNO;
		else if(uMagic != 0x04034b50)
			err = UNZ_BADZIPFILE;
	}

	if(unz64local_getShort(&s->z_filefunc, s->filestream, &sData) != UNZ_OK)
		err = UNZ_ERRNO;

	if(unz64local_getShort(&s->z_filefunc, s->filestream, &sFlags) != UNZ_OK)
		err = UNZ_ERRNO;

	if(unz64local_getShort(&s->z_filefunc, s->filestream, &sData) != UNZ_OK)
		err = UNZ_ERRNO;

	else if(err == UNZ_OK && sData != s->cur_file_info.compression_method)
		err = UNZ_BADZIPFILE;

	if(err == UNZ_OK && s->cur_file_info.compression_method != 0 && s->cur_file_info.compression_method != Z_DEFLATED)
		err = UNZ_BADZIPFILE;

	if(unz64local_getLong(&s->z_filefunc, s->filestream, &uData) != UNZ_OK) /* date/time */
		err = UNZ_ERRNO;

	if(unz64local_getLong(&s->z_filefunc, s->filestream, &uData) != UNZ_OK) /* crc */
		err = UNZ_ERRNO;
	else if(err == UNZ_OK && uData != s->cur_file_info.crc && (sFlags & 8) == 0)
		err = UNZ_BADZIPFILE;

	if(unz64local_getLong(&s->z_filefunc, s->filestream,&uData) != UNZ_OK) /* size compr */
		err = UNZ_ERRNO;
	else if(uData != 0xFFFFFFFF && err == UNZ_OK && uData != s->cur_file_info.compressed_size && (sFlags & 8) == 0)
		err = UNZ_BADZIPFILE;

	if(unz64local_getLong(&s->z_filefunc, s->filestream,&uData) != UNZ_OK) /* size uncompr */
		err = UNZ_ERRNO;
	else if(uData != 0xFFFFFFFF && err == UNZ_OK && uData != s->cur_file_info.uncompressed_size && (sFlags & 8) == 0)
		err = UNZ_BADZIPFILE;

	if(unz64local_getShort(&s->z_filefunc, s->filestream, &size_filename) != UNZ_OK)
		err = UNZ_ERRNO;
	else if(err == UNZ_OK && size_filename != s->cur_file_info.size_filename)
		err = UNZ_BADZIPFILE;

	*piSizeVar += (uint32_t)size_filename;

	if(unz64local_getShort(&s->z_filefunc, s->filestream, &size_extra_field) != UNZ_OK)
		err = UNZ_ERRNO;

	*poffset_local_extrafield= s->cur_file_info_internal.offset_curfile +
	SIZEZIPLOCALHEADER + size_filename;
	*psize_local_extrafield = (uint32_t)size_extra_field;

	*piSizeVar += (uint32_t)size_extra_field;

	return err;
}

/*
 Open for reading data the current file in the zipfile.
 If there is no error and the file is opened, the return value is UNZ_OK.
 */
extern int  unzOpenCurrentFileInternal (unzFile file, int* method, int* level, int raw)
{
	int err = UNZ_OK;
	uint32_t iSizeVar;
	unz64_s* s;
	file_in_zip64_read_info_s* pfile_in_zip_read_info;
	uint64_t offset_local_extrafield;  /* offset of the local extra field */
	uint32_t  size_local_extrafield;    /* size of the local extra field */

	if(file == NULL)
		return UNZ_PARAMERROR;

	s = (unz64_s*) file;

	if(!s->current_file_ok)
		return UNZ_PARAMERROR;

	if(s->pfile_in_zip_read != NULL)
		unzCloseCurrentFile(file);

	if(unz64local_CheckCurrentFileCoherencyHeader(s, &iSizeVar, &offset_local_extrafield, &size_local_extrafield) != UNZ_OK)
		return UNZ_BADZIPFILE;

	pfile_in_zip_read_info = malloc(sizeof(file_in_zip64_read_info_s));
	if(pfile_in_zip_read_info == NULL)
		return UNZ_INTERNALERROR;

	pfile_in_zip_read_info->read_buffer = malloc(UNZ_BUFSIZE);
	pfile_in_zip_read_info->offset_local_extrafield = offset_local_extrafield;
	pfile_in_zip_read_info->size_local_extrafield = size_local_extrafield;
	pfile_in_zip_read_info->pos_local_extrafield = 0;
	pfile_in_zip_read_info->raw = raw;

	if(pfile_in_zip_read_info->read_buffer == NULL)
	{
		free(pfile_in_zip_read_info);
		return UNZ_INTERNALERROR;
	}

	pfile_in_zip_read_info->stream_initialised = 0;

	if(method != NULL)
		*method = (int)s->cur_file_info.compression_method;

	if(level != NULL)
	{
		*level = 6;
		switch (s->cur_file_info.flag & 0x06)
		{
			case 6 : *level = 1; break;
			case 4 : *level = 2; break;
			case 2 : *level = 9; break;
		}
	}

	if(s->cur_file_info.compression_method != 0 && s->cur_file_info.compression_method != Z_DEFLATED)
#ifndef __APPLE__
		err = UNZ_BADZIPFILE;
#else
	;
#endif
	pfile_in_zip_read_info->crc32_wait=s->cur_file_info.crc;
	pfile_in_zip_read_info->_crc32 = 0;
	pfile_in_zip_read_info->total_out_64 = 0;
	pfile_in_zip_read_info->compression_method = s->cur_file_info.compression_method;
	pfile_in_zip_read_info->filestream=s->filestream;
	pfile_in_zip_read_info->z_filefunc=s->z_filefunc;
	pfile_in_zip_read_info->byte_before_the_zipfile=s->byte_before_the_zipfile;

	pfile_in_zip_read_info->stream.total_out = 0;

	if(s->cur_file_info.compression_method == Z_BZIP2ED && !raw)
	{
		pfile_in_zip_read_info->raw=1;
	}
	else if(s->cur_file_info.compression_method == Z_DEFLATED && !raw)
	{
		pfile_in_zip_read_info->stream.zalloc = (alloc_func)0;
		pfile_in_zip_read_info->stream.zfree = (free_func)0;
		pfile_in_zip_read_info->stream.opaque = (voidpf)0;
		pfile_in_zip_read_info->stream.next_in = 0;
		pfile_in_zip_read_info->stream.avail_in = 0;

		if((err = inflateInit2(&pfile_in_zip_read_info->stream, -MAX_WBITS)) == Z_OK)
			pfile_in_zip_read_info->stream_initialised = Z_DEFLATED;
		else
		{
			free(pfile_in_zip_read_info);
			return err;
		}

		/* windowBits is passed < 0 to tell that there is no zlib header.
		 * Note that in this case inflate *requires* an extra "dummy" byte
		 * after the compressed stream in order to complete decompression and
		 * return Z_STREAM_END.
		 * In unzip, i don't wait absolutely Z_STREAM_END because I known the
		 * size of both compressed and uncompressed data
		 */
	}

	pfile_in_zip_read_info->rest_read_compressed = s->cur_file_info.compressed_size;
	pfile_in_zip_read_info->rest_read_uncompressed = s->cur_file_info.uncompressed_size;
	pfile_in_zip_read_info->pos_in_zipfile = s->cur_file_info_internal.offset_curfile + SIZEZIPLOCALHEADER + iSizeVar;
	pfile_in_zip_read_info->stream.avail_in = (uint32_t)0;

	s->pfile_in_zip_read = pfile_in_zip_read_info;
	s->encrypted = 0;

	return UNZ_OK;
}

extern int  unzOpenCurrentFile (unzFile file)
{
	return unzOpenCurrentFileInternal(file, NULL, NULL, 0);
}

/** Addition for GDAL : START */

extern uint64_t  unzGetCurrentFileZStreamPos64( unzFile file)
{
	unz64_s* s;
	file_in_zip64_read_info_s* pfile_in_zip_read_info;
	s = (unz64_s*)file;
	if(file == NULL)
		return 0; //UNZ_PARAMERROR;
	pfile_in_zip_read_info=s->pfile_in_zip_read;
	if(pfile_in_zip_read_info == NULL)
		return 0; //UNZ_PARAMERROR;
	return pfile_in_zip_read_info->pos_in_zipfile +
	pfile_in_zip_read_info->byte_before_the_zipfile;
}

/** Addition for GDAL : END */

/*
 Read bytes from the current file.
 buf contain buffer where data must be copied
 len the size of buf.

 return the number of byte copied if somes bytes are copied
 return 0 if the end of file was reached
 return <0 with error code if there is an error
 (UNZ_ERRNO for IO error, or zLib error for uncompress error)
 */
extern int  unzReadCurrentFile  (unzFile file, voidp buf, unsigned len)
{
	int err = UNZ_OK, iRead = 0;
	unz64_s* s;
	file_in_zip64_read_info_s* pfile_in_zip_read_info;

	if(file == NULL)
		return UNZ_PARAMERROR;

	s = (unz64_s*)file;
	pfile_in_zip_read_info = s->pfile_in_zip_read;

	if(pfile_in_zip_read_info == NULL)
		return UNZ_PARAMERROR;

	if(pfile_in_zip_read_info->read_buffer == NULL)
		return UNZ_END_OF_LIST_OF_FILE;

	if(len == 0)
		return 0;

	pfile_in_zip_read_info->stream.next_out = (Bytef*)buf;

	pfile_in_zip_read_info->stream.avail_out = (uint32_t)len;

	if((len>pfile_in_zip_read_info->rest_read_uncompressed) && (!(pfile_in_zip_read_info->raw)))
		pfile_in_zip_read_info->stream.avail_out = (uint32_t)pfile_in_zip_read_info->rest_read_uncompressed;

	if(len > pfile_in_zip_read_info->rest_read_compressed + pfile_in_zip_read_info->stream.avail_in && pfile_in_zip_read_info->raw)
		pfile_in_zip_read_info->stream.avail_out = (uint32_t) pfile_in_zip_read_info->rest_read_compressed + pfile_in_zip_read_info->stream.avail_in;

	while (pfile_in_zip_read_info->stream.avail_out > 0)
	{
		if(pfile_in_zip_read_info->stream.avail_in == 0 && pfile_in_zip_read_info->rest_read_compressed > 0)
		{
			uint32_t uReadThis = UNZ_BUFSIZE;

			if(pfile_in_zip_read_info->rest_read_compressed < uReadThis)
				uReadThis = (uint32_t)pfile_in_zip_read_info->rest_read_compressed;

			if(uReadThis == 0)
				return UNZ_EOF;

			if(ZSEEK64(pfile_in_zip_read_info->z_filefunc, pfile_in_zip_read_info->filestream, pfile_in_zip_read_info->pos_in_zipfile + pfile_in_zip_read_info->byte_before_the_zipfile, ZLIB_FILEFUNC_SEEK_SET) != 0)
				return UNZ_ERRNO;

			if(ZREAD64(pfile_in_zip_read_info->z_filefunc, pfile_in_zip_read_info->filestream, pfile_in_zip_read_info->read_buffer, uReadThis) != uReadThis)
				return UNZ_ERRNO;

			pfile_in_zip_read_info->pos_in_zipfile += uReadThis;
			pfile_in_zip_read_info->rest_read_compressed -= uReadThis;
			pfile_in_zip_read_info->stream.next_in = (Bytef*)pfile_in_zip_read_info->read_buffer;
			pfile_in_zip_read_info->stream.avail_in = (uint32_t)uReadThis;
		}

		if(pfile_in_zip_read_info->compression_method == 0 || pfile_in_zip_read_info->raw)
		{
			uint32_t uDoCopy,i;

			if((pfile_in_zip_read_info->stream.avail_in == 0) && (pfile_in_zip_read_info->rest_read_compressed == 0))
				return (iRead == 0) ? UNZ_EOF : (int) iRead;

			if(pfile_in_zip_read_info->stream.avail_out <
			   pfile_in_zip_read_info->stream.avail_in)
				uDoCopy = pfile_in_zip_read_info->stream.avail_out;
			else
				uDoCopy = pfile_in_zip_read_info->stream.avail_in;

			for (i = 0;i<uDoCopy;i++)
				*(pfile_in_zip_read_info->stream.next_out+i) =
				*(pfile_in_zip_read_info->stream.next_in+i);

			pfile_in_zip_read_info->total_out_64 = pfile_in_zip_read_info->total_out_64 + uDoCopy;

			pfile_in_zip_read_info->_crc32 = crc32(pfile_in_zip_read_info->_crc32,
												   pfile_in_zip_read_info->stream.next_out,
												   uDoCopy);
			pfile_in_zip_read_info->rest_read_uncompressed-=uDoCopy;
			pfile_in_zip_read_info->stream.avail_in -= uDoCopy;
			pfile_in_zip_read_info->stream.avail_out -= uDoCopy;
			pfile_in_zip_read_info->stream.next_out += uDoCopy;
			pfile_in_zip_read_info->stream.next_in += uDoCopy;
			pfile_in_zip_read_info->stream.total_out += uDoCopy;
			iRead += uDoCopy;
		}
		else if(pfile_in_zip_read_info->compression_method == Z_BZIP2ED)
		{
			//Not implemented
		}
		else
		{
			uint64_t uTotalOutBefore,uTotalOutAfter;
			const Bytef *bufBefore;
			uint64_t uOutThis;
			int flush = Z_SYNC_FLUSH;

			uTotalOutBefore = pfile_in_zip_read_info->stream.total_out;
			bufBefore = pfile_in_zip_read_info->stream.next_out;

			err = inflate(&pfile_in_zip_read_info->stream,flush);

			if(err >= 0 && pfile_in_zip_read_info->stream.msg != NULL)
				err = Z_DATA_ERROR;

			uTotalOutAfter = pfile_in_zip_read_info->stream.total_out;
			uOutThis = uTotalOutAfter-uTotalOutBefore;

			pfile_in_zip_read_info->total_out_64 = pfile_in_zip_read_info->total_out_64 + uOutThis;
			pfile_in_zip_read_info->_crc32 = crc32(pfile_in_zip_read_info->_crc32, bufBefore, (uint32_t)(uOutThis));
			pfile_in_zip_read_info->rest_read_uncompressed -= uOutThis;

			iRead += (uTotalOutAfter - uTotalOutBefore);

			if(err == Z_STREAM_END)
				return iRead ? iRead : UNZ_EOF;

			if(err != Z_OK)
				break;
		}
	}

	return err == Z_OK ? iRead : err;
}

/*
 Give the current position in uncompressed data
 */

extern z_off_t  unztell (unzFile file)
{
	unz64_s* s;
	file_in_zip64_read_info_s* pfile_in_zip_read_info;

	if(file == NULL)
		return UNZ_PARAMERROR;

	s = (unz64_s*) file;
	pfile_in_zip_read_info = s->pfile_in_zip_read;

	if(pfile_in_zip_read_info == NULL)
		return UNZ_PARAMERROR;

	return (z_off_t) pfile_in_zip_read_info->stream.total_out;
}

extern uint64_t unztell64 (unzFile file)
{
	unz64_s* s;
	file_in_zip64_read_info_s* pfile_in_zip_read_info;

	if(file == NULL)
		return (uint64_t)-1;

	s = (unz64_s*)file;
	pfile_in_zip_read_info=s->pfile_in_zip_read;

	if(pfile_in_zip_read_info == NULL)
		return (uint64_t) - 1;

	return pfile_in_zip_read_info->total_out_64;
}


/*
 return 1 if the end of file was reached, 0 elsewhere
 */
extern int  unzeof (unzFile file)
{
	unz64_s* s;
	file_in_zip64_read_info_s* pfile_in_zip_read_info;
	if(file == NULL)
		return UNZ_PARAMERROR;
	s = (unz64_s*)file;
	pfile_in_zip_read_info=s->pfile_in_zip_read;

	if(pfile_in_zip_read_info == NULL)
		return UNZ_PARAMERROR;

	if(pfile_in_zip_read_info->rest_read_uncompressed == 0)
		return 1;
	else
		return 0;
}

/*
 Read extra field from the current file (opened by unzOpenCurrentFile)
 This is the local-header version of the extra field (sometimes, there is
 more info in the local-header version than in the central-header)

 if buf == NULL, it return the size of the local extra field that can be read

 if buf != NULL, len is the size of the buffer, the extra header is copied in
 buf.
 the return value is the number of bytes copied in buf, or (if <0)
 the error code
 */
extern int  unzGetLocalExtrafield (unzFile file, voidp buf, unsigned len)
{
	unz64_s* s;
	file_in_zip64_read_info_s* pfile_in_zip_read_info;
	uint32_t read_now;
	uint64_t size_to_read;

	if(file == NULL)
		return UNZ_PARAMERROR;
	s = (unz64_s*)file;
	pfile_in_zip_read_info=s->pfile_in_zip_read;

	if(pfile_in_zip_read_info == NULL)
		return UNZ_PARAMERROR;

	size_to_read = (pfile_in_zip_read_info->size_local_extrafield -
					pfile_in_zip_read_info->pos_local_extrafield);

	if(buf == NULL)
		return (int)size_to_read;

	if(len > size_to_read)
		read_now = (uint32_t) size_to_read;
	else
		read_now = (uint32_t) len;

	if(read_now == 0)
		return 0;

	if(ZSEEK64(pfile_in_zip_read_info->z_filefunc,
			   pfile_in_zip_read_info->filestream,
			   pfile_in_zip_read_info->offset_local_extrafield +
			   pfile_in_zip_read_info->pos_local_extrafield,
			   ZLIB_FILEFUNC_SEEK_SET) != 0)
		return UNZ_ERRNO;

	if(ZREAD64(pfile_in_zip_read_info->z_filefunc,
			   pfile_in_zip_read_info->filestream,
			   buf,read_now)!=read_now)
		return UNZ_ERRNO;

	return (int) read_now;
}

/*
 Close the file in zip opened with unzOpenCurrentFile
 Return UNZ_CRCERROR if all the file was read but the CRC is not good
 */
extern int  unzCloseCurrentFile (unzFile file)
{
	int err = UNZ_OK;

	unz64_s* s;
	file_in_zip64_read_info_s* pfile_in_zip_read_info;
	if(file == NULL)
		return UNZ_PARAMERROR;
	s = (unz64_s*)file;
	pfile_in_zip_read_info=s->pfile_in_zip_read;

	if(pfile_in_zip_read_info == NULL)
		return UNZ_PARAMERROR;


	if((pfile_in_zip_read_info->rest_read_uncompressed == 0) &&
	   (!pfile_in_zip_read_info->raw))
	{
		if(pfile_in_zip_read_info->_crc32 != pfile_in_zip_read_info->crc32_wait)
			err = UNZ_CRCERROR;
	}


	free(pfile_in_zip_read_info->read_buffer);
	pfile_in_zip_read_info->read_buffer = NULL;
	if(pfile_in_zip_read_info->stream_initialised == Z_DEFLATED)
		inflateEnd(&pfile_in_zip_read_info->stream);

	pfile_in_zip_read_info->stream_initialised = 0;
	free(pfile_in_zip_read_info);

	s->pfile_in_zip_read=NULL;

	return err;
}


/*
 Get the global comment string of the ZipFile, in the szComment buffer.
 uSizeBuf is the size of the szComment buffer.
 return the number of byte copied or an error code <0
 */
extern int unzGetGlobalComment (unzFile file, char * szComment, uint64_t uSizeBuf)
{
	unz64_s* s;
	uint64_t uReadThis;

	if(file == NULL)
		return UNZ_PARAMERROR;

	s = (unz64_s*)file;

	uReadThis = uSizeBuf;
	if(uReadThis>s->gi.size_comment)
		uReadThis = s->gi.size_comment;

	if(ZSEEK64(s->z_filefunc, s->filestream, s->central_pos + 22, ZLIB_FILEFUNC_SEEK_SET) != 0)
		return UNZ_ERRNO;

	if(uReadThis > 0)
	{
		*szComment = '\0';
		if(ZREAD64(s->z_filefunc, s->filestream, szComment, uReadThis) != uReadThis)
			return UNZ_ERRNO;
	}

	if(szComment != NULL && uSizeBuf > s->gi.size_comment)
		*(szComment + s->gi.size_comment) = '\0';

	return uReadThis;
}

/* Additions by RX '2004 < Unused*/
extern uint64_t  unzGetOffset64(unzFile file)
{
	unz64_s* s;

	if(file == NULL)
		return 0; //UNZ_PARAMERROR;

	s = (unz64_s*) file;

	if(!s->current_file_ok)
		return 0;

	if(s->gi.number_entry != 0 && s->gi.number_entry != 0xffff && s->num_file == s->gi.number_entry)
		return 0;

	return s->pos_in_central_dir;
}

extern int unzSetOffset64(unzFile file, uint64_t pos)
{
	unz64_s* s;
	int err;
	
	if(file == NULL)
		return UNZ_PARAMERROR;

	s = (unz64_s*) file;
	
	s->pos_in_central_dir = pos;
	s->num_file = s->gi.number_entry;      /* hack */
	err = unz64local_GetCurrentFileInfoInternal(file,&s->cur_file_info, &s->cur_file_info_internal, NULL, 0, NULL, 0, NULL, 0);
	s->current_file_ok = (err == UNZ_OK);
	return err;
}
