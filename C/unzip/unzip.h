/* unzip.h -- IO for uncompress .zip files using zlib
 Version 1.1, February 14h, 2010
 part of the MiniZip project - ( http://www.winimage.com/zLibDll/minizip.html )

 Copyright (C) 1998-2010 Gilles Vollant (minizip) ( http://www.winimage.com/zLibDll/minizip.html )

 Modifications of Unzip for Zip64
 Copyright (C) 2007-2008 Even Rouault

 Modifications for Zip64 support on both zip and unzip
 Copyright (C) 2009-2010 Mathias Svensson ( http://result42.com )

 For more info read MiniZip_info.txt

 ---------------------------------------------------------------------------------

 Condition of use and distribution are the same than zlib :

 This software is provided 'as-is', without any express or implied
 warranty.  In no event will the authors be held liable for any damages
 arising from the use of this software.

 Permission is granted to anyone to use this software for any purpose,
 including commercial applications, and to alter it and redistribute it
 freely, subject to the following restrictions:

 1. The origin of this software must not be misrepresented; you must not
 claim that you wrote the original software. If you use this software
 in a product, an acknowledgment in the product documentation would be
 appreciated but is not required.
 2. Altered source versions must be plainly marked as such, and must not be
 misrepresented as being the original software.
 3. This notice may not be removed or altered from any source distribution.

 ---------------------------------------------------------------------------------

 Changes

 See header of unzip64.c

 */

#ifndef _unz64_H
#define _unz64_H

#ifdef __cplusplus
extern "C" {
#endif

#ifndef _ZLIB_H
#include "zlib/zlib.h"
#endif

#ifndef  _ZLIBIOAPI_H
#include "ioapi.h"
#endif

#define Z_BZIP2ED 12

#if defined(STRICTUNZIP) || defined(STRICTZIPUNZIP)
	/* like the STRICT of WIN32, we define a pointer that cannot be converted
	 from (void*) without cast */
	typedef struct TagunzFile__ { int unused; } unzFile__;
	typedef unzFile__ *unzFile;
#else
	typedef voidp unzFile;
#endif


#define UNZ_OK                          (0)
#define UNZ_END_OF_LIST_OF_FILE         (-100)
#define UNZ_ERRNO                       (Z_ERRNO)
#define UNZ_EOF                         (0)
#define UNZ_PARAMERROR                  (-102)
#define UNZ_BADZIPFILE                  (-103)
#define UNZ_INTERNALERROR               (-104)
#define UNZ_CRCERROR                    (-105)

	/* tm_unz contain date/time info */
	typedef struct tm_unz_s
	{
		uint32_t tm_sec;            /* seconds after the minute - [0,59] */
		uint32_t tm_min;            /* minutes after the hour - [0,59] */
		uint32_t tm_hour;           /* hours since midnight - [0,23] */
		uint32_t tm_mday;           /* day of the month - [1,31] */
		uint32_t tm_mon;            /* months since January - [0,11] */
		uint32_t tm_year;           /* years - [1980..2044] */
	} tm_unz;

	/* unz_global_info structure contain global data about the ZIPfile
	 These data comes from the end of central dir */
	typedef struct unz_global_info64_s
	{
		uint64_t number_entry;         /* total number of entries in
										the central dir on this disk */
		uint64_t size_comment;         /* size of the global comment of the zipfile */
	} unz_global_info64;

	typedef struct unz_global_info_s
	{
		uint64_t number_entry;         /* total number of entries in
										the central dir on this disk */
		uint64_t size_comment;         /* size of the global comment of the zipfile */
	} unz_global_info;

	/* unz_file_info contain information about a file in the zipfile */
	typedef struct unz_file_info64_s
	{
		uint64_t version;              /* version made by                 2 bytes */
		uint64_t version_needed;       /* version needed to extract       2 bytes */
		uint64_t flag;                 /* general purpose bit flag        2 bytes */
		uint64_t compression_method;   /* compression method              2 bytes */
		uint64_t dosDate;              /* last mod file date in Dos fmt   4 bytes */
		uint64_t crc;                  /* crc-32                          4 bytes */
		uint64_t compressed_size;      /* compressed size                 8 bytes */
		uint64_t uncompressed_size;    /* uncompressed size               8 bytes */
		uint64_t size_filename;        /* filename length                 2 bytes */
		uint64_t size_file_extra;      /* extra field length              2 bytes */
		uint64_t size_file_comment;    /* file comment length             2 bytes */

		uint64_t disk_num_start;       /* disk number start               2 bytes */
		uint64_t internal_fa;          /* internal file attributes        2 bytes */
		uint64_t external_fa;          /* external file attributes        4 bytes */

		tm_unz tmu_date;
	} unz_file_info64;

	typedef struct unz_file_info_s
	{
		uint64_t version;              /* version made by                 2 bytes */
		uint64_t version_needed;       /* version needed to extract       2 bytes */
		uint64_t flag;                 /* general purpose bit flag        2 bytes */
		uint64_t compression_method;   /* compression method              2 bytes */
		uint64_t dosDate;              /* last mod file date in Dos fmt   4 bytes */
		uint64_t crc;                  /* crc-32                          4 bytes */
		uint64_t compressed_size;      /* compressed size                 4 bytes */
		uint64_t uncompressed_size;    /* uncompressed size               4 bytes */
		uint64_t size_filename;        /* filename length                 2 bytes */
		uint64_t size_file_extra;      /* extra field length              2 bytes */
		uint64_t size_file_comment;    /* file comment length             2 bytes */

		uint64_t disk_num_start;       /* disk number start               2 bytes */
		uint64_t internal_fa;          /* internal file attributes        2 bytes */
		uint64_t external_fa;          /* external file attributes        4 bytes */

		tm_unz tmu_date;
	} unz_file_info;

	extern int  unzStringFileNameCompare (const char* fileName1, const char* fileName2, int iCaseSensitivity);
	/*
	 Compare two filename (fileName1,fileName2).
	 If iCaseSenisivity = 1, comparision is case sensitivity (like strcmp)
	 If iCaseSenisivity = 2, comparision is not case sensitivity (like strcmpi
	 or strcasecmp)
	 If iCaseSenisivity = 0, case sensitivity is defaut of your operating system
	 (like 1 on Unix, 2 on Windows)
	 */


	extern unzFile  unzOpen (const char *path);
	extern unzFile  unzOpen64 (const void *path);
	/*
  Open a Zip file. path contain the full pathname (by example,
	 on a Windows XP computer "c:\\zlib\\zlib113.zip" or on an Unix computer
	 "zlib/zlib113.zip".
	 If the zipfile cannot be opened (file don't exist or in not valid), the
	 return value is NULL.
	 Else, the return value is a unzFile Handle, usable with other function
	 of this unzip package.
	 the "64" function take a const void* pointer, because the path is just the
	 value passed to the open64_file_func callback.
	 Under Windows, if UNICODE is defined, using fill_fopen64_filefunc, the path
	 is a pointer to a wide unicode string (LPCTSTR is LPCWSTR), so const char*
	 does not describe the reality
	 */


	extern unzFile  unzOpen2 (const char *path, zlib_filefunc_def* pzlib_filefunc_def);
	/*
	 Open a Zip file, like unzOpen, but provide a set of file low level API
	 for read/write the zip file (see ioapi.h)
	 */

	extern unzFile  unzOpen2_64 (const void *path, zlib_filefunc64_def* pzlib_filefunc_def);
	/*
	 Open a Zip file, like unz64Open, but provide a set of file low level API
	 for read/write the zip file (see ioapi.h)
	 */

	extern int  unzClose (unzFile file);
	/*
  Close a ZipFile opened with unzOpen.
  If there is files inside the .Zip opened with unzOpenCurrentFile (see later),
	 these files MUST be closed with unzCloseCurrentFile before call unzClose.
  return UNZ_OK if there is no problem. */

	extern int  unzGetGlobalInfo (unzFile file,
								  unz_global_info *pglobal_info);

	extern int  unzGetGlobalInfo64 (unzFile file,
									unz_global_info64 *pglobal_info);
	/*
  Write info about the ZipFile in the *pglobal_info structure.
  No preparation of the structure is needed
  return UNZ_OK if there is no problem. */


	extern int  unzGetGlobalComment (unzFile file,
									 char *szComment,
									 uint64_t uSizeBuf);
	/*
  Get the global comment string of the ZipFile, in the szComment buffer.
  uSizeBuf is the size of the szComment buffer.
  return the number of byte copied or an error code <0
	 */


	/***************************************************************************/
	/* Unzip package allow you browse the directory of the zipfile */

	extern int  unzGoToFirstFile (unzFile file);
	/*
  Set the current file of the zipfile to the first file.
  return UNZ_OK if there is no problem
	 */

	extern int unzGoToNextFile (unzFile file);
	/*
  Set the current file of the zipfile to the next file.
  return UNZ_OK if there is no problem
  return UNZ_END_OF_LIST_OF_FILE if the actual file was the latest.
	 */

	extern int  unzLocateFile (unzFile file,
							   const char *szFileName,
							   int iCaseSensitivity);
	/*
  Try locate the file szFileName in the zipfile.
  For the iCaseSensitivity signification, see unzStringFileNameCompare

  return value :
  UNZ_OK if the file is found. It becomes the current file.
  UNZ_END_OF_LIST_OF_FILE if the file is not found
	 */


	/* ****************************************** */
	/* Ryan supplied functions */
	/* unz_file_info contain information about a file in the zipfile */
	typedef struct unz_file_pos_s
	{
		uint64_t pos_in_zip_directory;   /* offset in zip file directory */
		uint64_t num_of_file;            /* # of file */
	} unz_file_pos;

	extern int  unzGetFilePos(
							  unzFile file,
							  unz_file_pos* file_pos);

	extern int  unzGoToFilePos(
							   unzFile file,
							   unz_file_pos* file_pos);

	typedef struct unz64_file_pos_s
	{
		uint64_t pos_in_zip_directory;   /* offset in zip file directory */
		uint64_t num_of_file;            /* # of file */
	} unz64_file_pos;

	extern int  unzGetFilePos64(
								unzFile file,
								unz64_file_pos* file_pos);

	extern int  unzGoToFilePos64(
								 unzFile file,
								 const unz64_file_pos* file_pos);

	/* ****************************************** */

	extern int  unzGetCurrentFileInfo64 (unzFile file,
										 unz_file_info64 *pfile_info,
										 char *szFileName,
										 uint64_t fileNameBufferSize,
										 void *extraField,
										 uint64_t extraFieldBufferSize,
										 char *szComment,
										 uint64_t commentBufferSize);

	extern int  unzGetCurrentFileInfo (unzFile file,
									   unz_file_info *pfile_info,
									   char *szFileName,
									   uint64_t fileNameBufferSize,
									   void *extraField,
									   uint64_t extraFieldBufferSize,
									   char *szComment,
									   uint64_t commentBufferSize);
	/*
  Get Info about the current file
  if pfile_info != NULL, the *pfile_info structure will contain somes info about
	 the current file
  if szFileName != NULL, the filemane string will be copied in szFileName
	 (fileNameBufferSize is the size of the buffer)
  if extraField != NULL, the extra field information will be copied in extraField
	 (extraFieldBufferSize is the size of the buffer).
	 This is the Central-header version of the extra field
  if szComment != NULL, the comment string of the file will be copied in szComment
	 (commentBufferSize is the size of the buffer)
	 */


	/** Addition for GDAL : START */

	extern uint64_t  unzGetCurrentFileZStreamPos64 (unzFile file);

	/** Addition for GDAL : END */


	/***************************************************************************/
	/* for reading the content of the current zipfile, you can open it, read data
	 from it, and close it (you can close it before reading all the file)
	 */

	extern int  unzOpenCurrentFile (unzFile file);
	/*
  Open for reading data the current file in the zipfile.
  If there is no error, the return value is UNZ_OK.
	 */

	extern int  unzOpenCurrentFileInternal (unzFile file,
									 int* method,
									 int* level,
									 int raw);
	/*
  Same than unzOpenCurrentFile, but open for read raw the file (not uncompress)
	 if raw==1
  *method will receive method of compression, *level will receive level of
	 compression
  note : you can set level parameter as NULL (if you did not want known level,
	 but you CANNOT set method parameter as NULL
	 */


	extern int  unzCloseCurrentFile (unzFile file);
	/*
  Close the file in zip opened with unzOpenCurrentFile
  Return UNZ_CRCERROR if all the file was read but the CRC is not good
	 */

	extern int  unzReadCurrentFile (unzFile file,
									voidp buf,
									unsigned len);
	/*
  Read bytes from the current file (opened by unzOpenCurrentFile)
  buf contain buffer where data must be copied
  len the size of buf.

  return the number of byte copied if somes bytes are copied
  return 0 if the end of file was reached
  return <0 with error code if there is an error
	 (UNZ_ERRNO for IO error, or zLib error for uncompress error)
	 */

	extern z_off_t  unztell (unzFile file);

	extern uint64_t  unztell64 (unzFile file);
	/*
  Give the current position in uncompressed data
	 */

	extern int  unzeof (unzFile file);
	/*
  return 1 if the end of file was reached, 0 elsewhere
	 */

	extern int  unzGetLocalExtrafield (unzFile file,
									   voidp buf,
									   unsigned len);
	/*
  Read extra field from the current file (opened by unzOpenCurrentFile)
  This is the local-header version of the extra field (sometimes, there is
	 more info in the local-header version than in the central-header)

  if buf == NULL, it return the size of the local extra field

  if buf != NULL, len is the size of the buffer, the extra header is copied in
	 buf.
  the return value is the number of bytes copied in buf, or (if <0)
	 the error code
	 */

	/***************************************************************************/

	/* Get the current file offset */
	extern uint64_t  unzGetOffset64 (unzFile file);

	/* Set the current file offset */
	extern int  unzSetOffset64 (unzFile file, uint64_t pos);
	extern int  unzSetOffset (unzFile file, uint64_t pos);



#ifdef __cplusplus
}
#endif

#endif /* _unz64_H */
