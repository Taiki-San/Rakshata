/* zip.c -- IO on .zip files using zlib
 Version 1.1, February 14h, 2010
 part of the MiniZip project - ( http://www.winimage.com/zLibDll/minizip.html )

 Copyright (C) 1998-2010 Gilles Vollant (minizip) ( http://www.winimage.com/zLibDll/minizip.html )

 Modifications for Zip64 support
 Copyright (C) 2009-2010 Mathias Svensson ( http://result42.com )

 For more info read MiniZip_info.txt

 Changes
 Oct-2009 - Mathias Svensson - Remove old C style function prototypes
 Oct-2009 - Mathias Svensson - Added Zip64 Support when creating new file archives
 Oct-2009 - Mathias Svensson - Did some code cleanup and refactoring to get better overview of some functions.
 Oct-2009 - Mathias Svensson - Added zipRemoveExtraInfoBlock to strip extra field data from its ZIP64 data
 It is used when recreting zip archive with RAW when deleting items from a zip.
 ZIP64 data is automaticly added to items that needs it, and existing ZIP64 data need to be removed.
 Oct-2009 - Mathias Svensson - Added support for BZIP2 as compression mode (bzip2 lib is required)
 Jan-2010 - back to unzip and minizip 1.0 name scheme, with compatibility layer

 */


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "zlib.h"
#include "zip.h"

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

#ifndef VERSIONMADEBY
# define VERSIONMADEBY   (0x0) /* platform depedent */
#endif

#ifndef Z_BUFSIZE
#define Z_BUFSIZE (64*1024) //(16384)
#endif

#ifndef Z_MAXFILENAMEINZIP
#define Z_MAXFILENAMEINZIP (256)
#endif

/*
 #define SIZECENTRALDIRITEM (0x2e)
 #define SIZEZIPLOCALHEADER (0x1e)
 */

/* I've found an old Unix (a SunOS 4.1.3_U1) without all SEEK_* defined.... */


// NOT sure that this work on ALL platform
#define MAKEULONG64(a, b) ((uint64_t)(((unsigned long)(a)) | ((uint64_t)((unsigned long)(b))) << 32))

#ifndef SEEK_CUR
#define SEEK_CUR    1
#endif

#ifndef SEEK_END
#define SEEK_END    2
#endif

#ifndef SEEK_SET
#define SEEK_SET    0
#endif

#ifndef DEF_MEM_LEVEL
#if MAX_MEM_LEVEL >= 8
#  define DEF_MEM_LEVEL 8
#else
#  define DEF_MEM_LEVEL  MAX_MEM_LEVEL
#endif
#endif
const char zip_copyright[] =" zip 1.01 Copyright 1998-2004 Gilles Vollant - http://www.winimage.com/zLibDll";


#define SIZEDATA_INDATABLOCK (4096-(4*4))

#define LOCALHEADERMAGIC    (0x04034b50)
#define CENTRALHEADERMAGIC  (0x02014b50)
#define ENDHEADERMAGIC      (0x06054b50)
#define ZIP64ENDHEADERMAGIC      (0x6064b50)
#define ZIP64ENDLOCHEADERMAGIC   (0x7064b50)

#define FLAG_LOCALHEADER_OFFSET (0x06)
#define CRC_LOCALHEADER_OFFSET  (0x0e)

#define SIZECENTRALHEADER (0x2e) /* 46 */

typedef struct linkedlist_datablock_internal_s
{
	struct linkedlist_datablock_internal_s* next_datablock;
	uint64_t  avail_in_this_block;
	uint64_t  filled_in_this_block;
	uint64_t  unused; /* for future use and alignement */
	unsigned char data[SIZEDATA_INDATABLOCK];
} linkedlist_datablock_internal;

typedef struct
{
	linkedlist_datablock_internal* first_block;
	linkedlist_datablock_internal* last_block;
} linkedlist_data;


typedef struct
{
	z_stream stream;            /* zLib stream structure for inflate */

	int  stream_initialised;    /* 1 is stream is initialised */
	uint32_t pos_in_buffered_data;  /* last written byte in buffered_data */

	uint64_t pos_local_header;     /* offset of the local header of the file
									currenty writing */
	char* central_header;       /* central header data for the current file */
	uint64_t size_centralExtra;
	uint64_t size_centralheader;   /* size of the central header for cur file */
	uint64_t size_centralExtraFree; /* Extra bytes allocated to the centralheader but that are not used */
	uint64_t flag;                 /* flag of the file currently writing */

	int  method;                /* compression method of file currenty wr.*/
	int  raw;                   /* 1 for directly writing raw data */
	byte buffered_data[Z_BUFSIZE];/* buffer contain compressed data to be writ*/
	uint64_t dosDate;
	uint64_t crc32;
	int  encrypt;
	int  zip64;               /* Add ZIP64 extened information in the extra field */
	uint64_t pos_zip64extrainfo;
	uint64_t totalCompressedData;
	uint64_t totalUncompressedData;
} curfile64_info;

typedef struct
{
	zlib_filefunc64_32_def z_filefunc;
	voidpf filestream;        /* io structore of the zipfile */
	linkedlist_data central_dir;/* datablock with central dir in construction*/
	int  in_opened_file_inzip;  /* 1 if a file in the zip is currently writ.*/
	curfile64_info ci;            /* info on the file curretly writing */

	uint64_t begin_pos;            /* position of the beginning of the zipfile */
	uint64_t add_position_when_writting_offset;
	uint64_t number_entry;

	char *globalcomment;

} zip64_internal;

static linkedlist_datablock_internal* allocate_new_datablock()
{
	linkedlist_datablock_internal* ldi;
	ldi = (linkedlist_datablock_internal*)
	malloc(sizeof(linkedlist_datablock_internal));
	if(ldi != NULL)
	{
		ldi->next_datablock = NULL;
		ldi->filled_in_this_block = 0;
		ldi->avail_in_this_block = SIZEDATA_INDATABLOCK;
	}
	return ldi;
}

static void free_datablock(linkedlist_datablock_internal* ldi)
{
	while (ldi != NULL)
	{
		linkedlist_datablock_internal* ldinext = ldi->next_datablock;
		free(ldi);
		ldi = ldinext;
	}
}

static void init_linkedlist(linkedlist_data* ll)
{
	ll->first_block = ll->last_block = NULL;
}

static void free_linkedlist(linkedlist_data* ll)
{
	free_datablock(ll->first_block);
	ll->first_block = ll->last_block = NULL;
}

static int add_data_in_datablock(linkedlist_data* ll, const void* buf, uint64_t len)
{
	linkedlist_datablock_internal* ldi;
	const unsigned char* from_copy;

	if(ll == NULL)
		return ZIP_INTERNALERROR;

	if(ll->last_block == NULL)
	{
		ll->first_block = ll->last_block = allocate_new_datablock();
		if(ll->first_block == NULL)
			return ZIP_INTERNALERROR;
	}

	ldi = ll->last_block;
	from_copy = (unsigned char*)buf;

	while (len > 0)
	{
		uint32_t copy_this;
		uint32_t i;
		unsigned char* to_copy;

		if(ldi->avail_in_this_block == 0)
		{
			ldi->next_datablock = allocate_new_datablock();
			if(ldi->next_datablock == NULL)
				return ZIP_INTERNALERROR;
			ldi = ldi->next_datablock;
			ll->last_block = ldi;
		}

		if(ldi->avail_in_this_block < len)
			copy_this = (uint32_t)ldi->avail_in_this_block;
		else
			copy_this = (uint32_t)len;

		to_copy = &(ldi->data[ldi->filled_in_this_block]);

		for (i = 0;i<copy_this;i++)
			*(to_copy+i)=*(from_copy+i);

		ldi->filled_in_this_block += copy_this;
		ldi->avail_in_this_block -= copy_this;
		from_copy += copy_this;
		len -= copy_this;
	}
	return ZIP_OK;
}



/****************************************************************************/

/* ===========================================================================
 Inputs a long in LSB order to the given file
 nbByte == 1, 2 ,4 or 8 (byte, short or long, uint64_t)
 */

static int zip64local_putValue (const zlib_filefunc64_32_def* pzlib_filefunc_def, voidpf filestream, uint64_t x, uint64_t nbByte)
{
	byte buf[8];
	uint64_t n;

	for (n = 0; n < nbByte; n++)
	{
		buf[n] = (unsigned char)(x & 0xff);
		x >>= 8;
	}
	if(x != 0)
	{     /* data overflow - hack for ZIP64 (X Roche) */
		for (n = 0; n < nbByte; n++)
		{
			buf[n] = 0xff;
		}
	}

	if(ZWRITE64(*pzlib_filefunc_def,filestream,buf,(uint64_t) nbByte) != nbByte)
		return ZIP_ERRNO;
	else
		return ZIP_OK;
}

static void zip64local_putValue_inmemory(void* dest, uint64_t x, int nbByte)
{
	byte * buf = (byte *) dest;

	for(byte n = 0; n < nbByte; n++)
	{
		buf[n] = (byte) (x & 0xff);
		x >>= 8;
	}

	//data overflow - hack for ZIP64
	if(x != 0)
		for (byte n = 0; n < nbByte; buf[n++] = 0xff);
}

/****************************************************************************/


static uint64_t zip64local_TmzDateToDosDate(const tm_zip* ptm)
{
	uint64_t year = (uint64_t) ptm->tm_year;

	if(year >= 1980)
		year -= 1980;
	else if(year >= 80)
		year -= 80;

	return (uint64_t) ((ptm->tm_mday + 32 * (ptm->tm_mon + 1) + 512 * year) << 16) | (ptm->tm_sec / 2 + 32 * ptm->tm_min + 2048 * (uint64_t) ptm->tm_hour);
}


/****************************************************************************/

static int zip64local_getByte(const zlib_filefunc64_32_def* pzlib_filefunc_def, voidpf filestream, byte * pi)
{
	byte c;

	if(ZREAD64(*pzlib_filefunc_def, filestream, &c, 1) == 1)
	{
		*pi = c;
		return ZIP_OK;
	}
	else
	{
		if(ZERROR64(*pzlib_filefunc_def,filestream))
			return ZIP_ERRNO;
		else
			return ZIP_EOF;
	}
}


/* ===========================================================================
 Reads a long in LSB order from the given gz_stream. Sets
 */
static int zip64local_getShort (const zlib_filefunc64_32_def* pzlib_filefunc_def, voidpf filestream, uint16_t * pX)
{
	uint16_t x;
	byte i = 0;
	int err;

	if((err = zip64local_getByte(pzlib_filefunc_def, filestream, &i)) == ZIP_OK)
		x = i;

	if(err == ZIP_OK && (err = zip64local_getByte(pzlib_filefunc_def, filestream, &i)) == ZIP_OK)
		x += ((short) i) << 8;
	else
		x = 0;

	if(pX != NULL)
		*pX = x;

	return err;
}

static int zip64local_getLong (const zlib_filefunc64_32_def* pzlib_filefunc_def, voidpf filestream, uint32_t* pX)
{
	uint32_t x;
	byte i = 0;
	int err;

	if((err = zip64local_getByte(pzlib_filefunc_def, filestream, &i)) == ZIP_OK)
		x = i;

	if(err == ZIP_OK && (err = zip64local_getByte(pzlib_filefunc_def, filestream, &i)) == ZIP_OK)
		x += ((uint32_t) i) << 8;

	if(err == ZIP_OK && (err = zip64local_getByte(pzlib_filefunc_def, filestream, &i)) == ZIP_OK)
		x += ((uint32_t) i) << 16;

	if(err == ZIP_OK && (err = zip64local_getByte(pzlib_filefunc_def, filestream, &i)) == ZIP_OK)
		x += ((uint32_t) i) << 24;
	else
		x = 0;

	if(pX != NULL)
		*pX = x;

	return err;
}

static int zip64local_getLong64 (const zlib_filefunc64_32_def* pzlib_filefunc_def, voidpf filestream, uint64_t *pX)
{
	uint64_t x;
	byte i = 0;
	int err;

	if((err = zip64local_getByte(pzlib_filefunc_def, filestream, &i)) == ZIP_OK)
		x = i;

	if(err == ZIP_OK && (err = zip64local_getByte(pzlib_filefunc_def, filestream, &i)) == ZIP_OK)
		x += ((uint64_t) i) << 8;

	if(err == ZIP_OK && (err = zip64local_getByte(pzlib_filefunc_def, filestream, &i)) == ZIP_OK)
		x += ((uint64_t) i) << 16;

	if(err == ZIP_OK && (err = zip64local_getByte(pzlib_filefunc_def, filestream, &i)) == ZIP_OK)
		x += ((uint64_t) i) << 24;

	if(err == ZIP_OK && (err = zip64local_getByte(pzlib_filefunc_def, filestream, &i)) == ZIP_OK)
		x += ((uint64_t) i) << 32;

	if(err == ZIP_OK && (err = zip64local_getByte(pzlib_filefunc_def, filestream, &i)) == ZIP_OK)
		x += ((uint64_t) i) << 40;

	if(err == ZIP_OK && (err = zip64local_getByte(pzlib_filefunc_def, filestream, &i)) == ZIP_OK)
		x += ((uint64_t) i) << 48;

	if(err == ZIP_OK && (err = zip64local_getByte(pzlib_filefunc_def, filestream, &i)) == ZIP_OK)
		x += ((uint64_t) i) << 56;
	else
		x = 0;

	if(pX != NULL)
		*pX = x;

	return err;
}

#ifndef BUFREADCOMMENT
#define BUFREADCOMMENT (0x400)
#endif
/*
	Locate the Central directory of a zipfile (at the end, just before
 the global comment)
 */

static uint64_t zip64local_SearchCentralDir(const zlib_filefunc64_32_def* pzlib_filefunc_def, voidpf filestream)
{
	byte* buf;
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
		uint64_t uReadSize, i;
		uint64_t uReadPos;
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

		for (i = uReadSize - 3; i-- > 0;)
			if(((*(buf+i)) == 0x50) && ((*(buf+i+1)) == 0x4b) &&
			   ((*(buf+i+2)) == 0x05) && ((*(buf+i+3)) == 0x06))
			{
				uPosFound = uReadPos+i;
				break;
			}

		if(uPosFound != 0)
			break;
	}
	free(buf);
	return uPosFound;
}

/*
 Locate the End of Zip64 Central directory locator and from there find the CD of a zipfile (at the end, just before
 the global comment)
 */

static uint64_t zip64local_SearchCentralDir64(const zlib_filefunc64_32_def* pzlib_filefunc_def, voidpf filestream)
{
	unsigned char* buf;
	uint64_t uSizeFile;
	uint64_t uBackRead;
	uint64_t uMaxBack = 0xffff; /* maximum size of global comment */
	uint64_t uPosFound = 0;
	uint32_t entry;
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
		uint64_t uReadSize, uReadPos;

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

		for(uint64_t i = uReadSize - 3; i-- > 0;)
		{
			// Signature "0x07064b50" Zip64 end of central directory locater
			if(((*(buf+i)) == 0x50) && ((*(buf+i+1)) == 0x4b) && ((*(buf+i+2)) == 0x06) && ((*(buf+i+3)) == 0x07))
			{
				uPosFound = uReadPos + i;
				break;
			}
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
	if(zip64local_getLong(pzlib_filefunc_def,filestream, &entry) != ZIP_OK)
		return 0;

	/* number of the disk with the start of the zip64 end of  central directory */
	if(zip64local_getLong(pzlib_filefunc_def,filestream, &entry) != ZIP_OK || entry != 0)
		return 0;

	/* relative offset of the zip64 end of central directory record */
	if(zip64local_getLong64(pzlib_filefunc_def,filestream, &relativeOffset) != ZIP_OK)
		return 0;

	/* total number of disks */
	if(zip64local_getLong(pzlib_filefunc_def,filestream, &entry) != ZIP_OK || entry != 1)
		return 0;

	/* Goto Zip64 end of central directory record */
	if(ZSEEK64(*pzlib_filefunc_def,filestream, relativeOffset,ZLIB_FILEFUNC_SEEK_SET) != 0)
		return 0;

	/* the signature of 'Zip64 end of central directory'*/
	if(zip64local_getLong(pzlib_filefunc_def,filestream, &entry) != ZIP_OK || entry != 0x06064b50)
		return 0;

	return relativeOffset;
}

int LoadCentralDirectoryRecord(zip64_internal* pziinit)
{
	int err = ZIP_OK;
	uint64_t byte_before_the_zipfile = 0;	/* byte before the zipfile, ( > 0 for sfx)*/

	uint64_t size_central_dir = 0;		/* size of the central directory  */
	uint64_t offset_central_dir = 0;	/* offset of start of central directory */
	uint64_t central_pos = 0;
	uint64_t number_entry_CD = 0;		/* total number of entries in the central dir (same than number_entry on nospan) */

	uint16_t VersionMadeBy, VersionNeeded;
	uint16_t size_comment = 0;

	bool hasZIP64Record = false;

	// check first if we find a ZIP64 record
	central_pos = zip64local_SearchCentralDir64(&pziinit->z_filefunc,pziinit->filestream);
	if(central_pos > 0)
		hasZIP64Record = true;

	else if(central_pos == 0)
		central_pos = zip64local_SearchCentralDir(&pziinit->z_filefunc,pziinit->filestream);

	/* disable to allow appending to empty ZIP archive
	 if(central_pos == 0)
	 err = ZIP_ERRNO;
	 */

	if(hasZIP64Record)
	{
		uint32_t number_disk;			/* number of the current dist, used for spaning ZIP, unsupported, always 0*/
		uint32_t number_disk_with_CD;	/* number the the disk with central dir, used for spaning ZIP, unsupported, always 0*/
		uint64_t number_entry, sizeEndOfCentralDirectory;

		if(ZSEEK64(pziinit->z_filefunc, pziinit->filestream, central_pos, ZLIB_FILEFUNC_SEEK_SET) != 0)
			err = ZIP_ERRNO;

		/* the signature, already checked */
		if(zip64local_getLong(&pziinit->z_filefunc, pziinit->filestream, NULL) != ZIP_OK)
			err = ZIP_ERRNO;

		/* size of zip64 end of central directory record */
		if(zip64local_getLong64(&pziinit->z_filefunc, pziinit->filestream, &sizeEndOfCentralDirectory) != ZIP_OK)
			err = ZIP_ERRNO;

		/* version made by */
		if(zip64local_getShort(&pziinit->z_filefunc, pziinit->filestream, &VersionMadeBy) != ZIP_OK)
			err = ZIP_ERRNO;

		/* version needed to extract */
		if(zip64local_getShort(&pziinit->z_filefunc, pziinit->filestream, &VersionNeeded) != ZIP_OK)
			err = ZIP_ERRNO;

		/* number of this disk */
		if(zip64local_getLong(&pziinit->z_filefunc, pziinit->filestream, &number_disk) != ZIP_OK)
			err = ZIP_ERRNO;

		/* number of the disk with the start of the central directory */
		if(zip64local_getLong(&pziinit->z_filefunc, pziinit->filestream, &number_disk_with_CD) != ZIP_OK)
			err = ZIP_ERRNO;

		/* total number of entries in the central directory on this disk */
		if(zip64local_getLong64(&pziinit->z_filefunc, pziinit->filestream, &number_entry) != ZIP_OK)
			err = ZIP_ERRNO;

		/* total number of entries in the central directory */
		if(zip64local_getLong64(&pziinit->z_filefunc, pziinit->filestream, &number_entry_CD) != ZIP_OK)
			err = ZIP_ERRNO;

		if(number_entry_CD != number_entry || number_disk_with_CD != 0 || number_disk != 0)
			err = ZIP_BADZIPFILE;

		/* size of the central directory */
		if(zip64local_getLong64(&pziinit->z_filefunc, pziinit->filestream, &size_central_dir) != ZIP_OK)
			err = ZIP_ERRNO;

		/* offset of start of central directory with respect to the
		 starting disk number */
		if(zip64local_getLong64(&pziinit->z_filefunc, pziinit->filestream, &offset_central_dir) != ZIP_OK)
			err = ZIP_ERRNO;
	}
	else
	{
		uint16_t number_disk;			/* number of the current dist, used for spaning ZIP, unsupported, always 0*/
		uint16_t number_disk_with_CD;	/* number the the disk with central dir, used for spaning ZIP, unsupported, always 0*/
		uint16_t number_entry;

		// Read End of central Directory info
		if(ZSEEK64(pziinit->z_filefunc, pziinit->filestream, central_pos,ZLIB_FILEFUNC_SEEK_SET) != 0)
			err = ZIP_ERRNO;

		/* the signature, already checked */
		if(zip64local_getLong(&pziinit->z_filefunc, pziinit->filestream, NULL) != ZIP_OK)
			err = ZIP_ERRNO;

		/* number of this disk */
		if(zip64local_getShort(&pziinit->z_filefunc, pziinit->filestream, &number_disk) != ZIP_OK)
			err = ZIP_ERRNO;

		/* number of the disk with the start of the central directory */
		if(zip64local_getShort(&pziinit->z_filefunc, pziinit->filestream, &number_disk_with_CD) != ZIP_OK)
			err = ZIP_ERRNO;

		/* total number of entries in the central dir on this disk */
		if(zip64local_getShort(&pziinit->z_filefunc, pziinit->filestream, &number_entry) != ZIP_OK)
			err = ZIP_ERRNO;

		/* total number of entries in the central dir */
		if(zip64local_getShort(&pziinit->z_filefunc, pziinit->filestream, (uint16_t *) &number_entry_CD) != ZIP_OK)
			err = ZIP_ERRNO;

		if(err == ZIP_OK && (number_entry_CD != number_entry || number_disk_with_CD != 0 || number_disk != 0))
			err = ZIP_BADZIPFILE;

		/* size of the central directory */
		if(zip64local_getLong(&pziinit->z_filefunc, pziinit->filestream, (uint32_t *) &size_central_dir) != ZIP_OK)
			err = ZIP_ERRNO;

		/* offset of start of central directory with respect to the starting disk number */
		if(zip64local_getLong(&pziinit->z_filefunc, pziinit->filestream, (uint32_t *) &offset_central_dir) != ZIP_OK)
			err = ZIP_ERRNO;

		/* zipfile global comment length */
		if(zip64local_getShort(&pziinit->z_filefunc, pziinit->filestream, &size_comment) != ZIP_OK)
			err = ZIP_ERRNO;
	}

	if(err == ZIP_OK && central_pos < offset_central_dir + size_central_dir)
		err = ZIP_BADZIPFILE;

	if(err != ZIP_OK)
	{
		ZCLOSE64(pziinit->z_filefunc, pziinit->filestream);
		return ZIP_ERRNO;
	}

	if(size_comment > 0)
	{
		pziinit->globalcomment = (char*)malloc(size_comment+1);
		if(pziinit->globalcomment)
		{
			size_comment = ZREAD64(pziinit->z_filefunc, pziinit->filestream, pziinit->globalcomment,size_comment);
			pziinit->globalcomment[size_comment] = 0;
		}
	}

	byte_before_the_zipfile = central_pos - (offset_central_dir+size_central_dir);
	pziinit->add_position_when_writting_offset = byte_before_the_zipfile;

	uint64_t size_central_dir_to_read = size_central_dir;
	size_t buf_size = SIZEDATA_INDATABLOCK;
	void* buf_read = (void*)malloc(buf_size);
	if(ZSEEK64(pziinit->z_filefunc, pziinit->filestream, offset_central_dir + byte_before_the_zipfile, ZLIB_FILEFUNC_SEEK_SET) != 0)
		err = ZIP_ERRNO;

	while ((size_central_dir_to_read > 0) && (err == ZIP_OK))
	{
		uint64_t read_this = SIZEDATA_INDATABLOCK;
		if(read_this > size_central_dir_to_read)
			read_this = size_central_dir_to_read;

		if(ZREAD64(pziinit->z_filefunc, pziinit->filestream,buf_read,(uint64_t)read_this) != read_this)
			err = ZIP_ERRNO;

		if(err == ZIP_OK)
			err = add_data_in_datablock(&pziinit->central_dir,buf_read, (uint64_t)read_this);

		size_central_dir_to_read-=read_this;
	}
	free(buf_read);

	pziinit->begin_pos = byte_before_the_zipfile;
	pziinit->number_entry = number_entry_CD;

	if(ZSEEK64(pziinit->z_filefunc, pziinit->filestream, offset_central_dir + byte_before_the_zipfile, ZLIB_FILEFUNC_SEEK_SET) != 0)
		err = ZIP_ERRNO;

	return err;
}

/************************************************************/
extern zipFile  zipOpen3 (const void *pathname, int append, zipcharpc* globalcomment, zlib_filefunc64_32_def* pzlib_filefunc64_32_def)
{
	zip64_internal ziinit;
	zip64_internal* zi;
	int err = ZIP_OK;

	ziinit.z_filefunc.zseek32_file = NULL;
	ziinit.z_filefunc.ztell32_file = NULL;
	if(pzlib_filefunc64_32_def == NULL)
		fill_fopen64_filefunc(&ziinit.z_filefunc.zfile_func64);
	else
		ziinit.z_filefunc = *pzlib_filefunc64_32_def;

	ziinit.filestream = ZOPEN64(ziinit.z_filefunc,
								pathname,
								(append == APPEND_STATUS_CREATE) ?
								(ZLIB_FILEFUNC_MODE_READ | ZLIB_FILEFUNC_MODE_WRITE | ZLIB_FILEFUNC_MODE_CREATE) :
								(ZLIB_FILEFUNC_MODE_READ | ZLIB_FILEFUNC_MODE_WRITE | ZLIB_FILEFUNC_MODE_EXISTING));

	if(ziinit.filestream == NULL)
		return NULL;

	if(append == APPEND_STATUS_CREATEAFTER)
		ZSEEK64(ziinit.z_filefunc,ziinit.filestream,0,SEEK_END);

	ziinit.begin_pos = ZTELL64(ziinit.z_filefunc,ziinit.filestream);
	ziinit.in_opened_file_inzip = 0;
	ziinit.ci.stream_initialised = 0;
	ziinit.number_entry = 0;
	ziinit.add_position_when_writting_offset = 0;
	init_linkedlist(&(ziinit.central_dir));



	zi = (zip64_internal*)malloc(sizeof(zip64_internal));
	if(zi == NULL)
	{
		ZCLOSE64(ziinit.z_filefunc,ziinit.filestream);
		return NULL;
	}

	/* now we add file in a zipfile */
	ziinit.globalcomment = NULL;
	if(append == APPEND_STATUS_ADDINZIP)
	{
		// Read and Cache Central Directory Records
		err = LoadCentralDirectoryRecord(&ziinit);
	}

	if(globalcomment)
	{
		*globalcomment = ziinit.globalcomment;
	}

	if(err != ZIP_OK)
	{
		free(ziinit.globalcomment);
		free(zi);

		return NULL;
	}

	*zi = ziinit;
	return (zipFile)zi;
}

extern zipFile  zipOpen2 (const char *pathname, int append, zipcharpc* globalcomment, zlib_filefunc_def* pzlib_filefunc32_def)
{
	if(pzlib_filefunc32_def != NULL)
	{
		zlib_filefunc64_32_def zlib_filefunc64_32_def_fill;
		fill_zlib_filefunc64_32_def_from_filefunc32(&zlib_filefunc64_32_def_fill,pzlib_filefunc32_def);
		return zipOpen3(pathname, append, globalcomment, &zlib_filefunc64_32_def_fill);
	}
	else
		return zipOpen3(pathname, append, globalcomment, NULL);
}

extern zipFile  zipOpen2_64 (const void *pathname, int append, zipcharpc* globalcomment, zlib_filefunc64_def* pzlib_filefunc_def)
{
	if(pzlib_filefunc_def != NULL)
	{
		zlib_filefunc64_32_def zlib_filefunc64_32_def_fill;
		zlib_filefunc64_32_def_fill.zfile_func64 = *pzlib_filefunc_def;
		zlib_filefunc64_32_def_fill.ztell32_file = NULL;
		zlib_filefunc64_32_def_fill.zseek32_file = NULL;
		return zipOpen3(pathname, append, globalcomment, &zlib_filefunc64_32_def_fill);
	}
	else
		return zipOpen3(pathname, append, globalcomment, NULL);
}

extern zipFile zipOpen (const char* pathname, int append)
{
	return zipOpen3((const void*)pathname,append,NULL,NULL);
}

extern zipFile  zipOpen64 (const void* pathname, int append)
{
	return zipOpen3(pathname,append,NULL,NULL);
}

int Write_LocalFileHeader(zip64_internal* zi, const char* filename, uint32_t size_extrafield_local, const void* extrafield_local)
{
	/* write the local header */
	int err;
	uint32_t size_filename = (uint32_t)strlen(filename);
	uint32_t size_extrafield = size_extrafield_local;

	err = zip64local_putValue(&zi->z_filefunc,zi->filestream,(uint64_t)LOCALHEADERMAGIC, 4);

	if(err == ZIP_OK)
	{
		if(zi->ci.zip64)
			err = zip64local_putValue(&zi->z_filefunc,zi->filestream,(uint64_t)45,2);/* version needed to extract */
		else
			err = zip64local_putValue(&zi->z_filefunc,zi->filestream,(uint64_t)20,2);/* version needed to extract */
	}

	if(err == ZIP_OK)
		err = zip64local_putValue(&zi->z_filefunc,zi->filestream,(uint64_t)zi->ci.flag,2);

	if(err == ZIP_OK)
		err = zip64local_putValue(&zi->z_filefunc,zi->filestream,(uint64_t)zi->ci.method,2);

	if(err == ZIP_OK)
		err = zip64local_putValue(&zi->z_filefunc,zi->filestream,(uint64_t)zi->ci.dosDate,4);

	// CRC / Compressed size / Uncompressed size will be filled in later and rewritten later
	if(err == ZIP_OK)
		err = zip64local_putValue(&zi->z_filefunc,zi->filestream,(uint64_t)0,4); /* crc 32, unknown */
	if(err == ZIP_OK)
	{
		if(zi->ci.zip64)
			err = zip64local_putValue(&zi->z_filefunc,zi->filestream,(uint64_t)0xFFFFFFFF,4); /* compressed size, unknown */
		else
			err = zip64local_putValue(&zi->z_filefunc,zi->filestream,(uint64_t)0,4); /* compressed size, unknown */
	}
	if(err == ZIP_OK)
	{
		if(zi->ci.zip64)
			err = zip64local_putValue(&zi->z_filefunc,zi->filestream,(uint64_t)0xFFFFFFFF,4); /* uncompressed size, unknown */
		else
			err = zip64local_putValue(&zi->z_filefunc,zi->filestream,(uint64_t)0,4); /* uncompressed size, unknown */
	}

	if(err == ZIP_OK)
		err = zip64local_putValue(&zi->z_filefunc,zi->filestream,(uint64_t)size_filename,2);

	if(zi->ci.zip64)
	{
		size_extrafield += 20;
	}

	if(err == ZIP_OK)
		err = zip64local_putValue(&zi->z_filefunc,zi->filestream,(uint64_t)size_extrafield,2);

	if((err == ZIP_OK) && (size_filename > 0))
	{
		if(ZWRITE64(zi->z_filefunc,zi->filestream,filename,size_filename)!=size_filename)
			err = ZIP_ERRNO;
	}

	if((err == ZIP_OK) && (size_extrafield_local > 0))
	{
		if(ZWRITE64(zi->z_filefunc, zi->filestream, extrafield_local, size_extrafield_local) != size_extrafield_local)
			err = ZIP_ERRNO;
	}


	if(err == ZIP_OK && (zi->ci.zip64))
	{
		// write the Zip64 extended info
		short HeaderID = 1;
		short DataSize = 16;
		uint64_t CompressedSize = 0;
		uint64_t UncompressedSize = 0;

		// Remember position of Zip64 extended info for the local file header. (needed when we update size after done with file)
		zi->ci.pos_zip64extrainfo = ZTELL64(zi->z_filefunc,zi->filestream);

		err = zip64local_putValue(&zi->z_filefunc, zi->filestream, (uint64_t) HeaderID, 2);

		if(err == ZIP_OK)
			err = zip64local_putValue(&zi->z_filefunc, zi->filestream, (uint64_t) DataSize, 2);

		if(err == ZIP_OK)
			err = zip64local_putValue(&zi->z_filefunc, zi->filestream, (uint64_t) UncompressedSize, 8);

		if(err == ZIP_OK)
			err = zip64local_putValue(&zi->z_filefunc, zi->filestream, (uint64_t) CompressedSize, 8);
	}

	return err;
}

/*
 NOTE.
 When writing RAW the ZIP64 extended information in extrafield_local and extrafield_global needs to be stripped
 before calling this function it can be done with zipRemoveExtraInfoBlock

 It is not done here because then we need to realloc a new buffer since parameters are 'const' and I want to minimize
 unnecessary allocations.
 */
extern int  zipOpenNewFileInZip4_64 (zipFile file, const char* filename, const zip_fileinfo* zipfi,
									 const void* extrafield_local, uint32_t size_extrafield_local,
									 const void* extrafield_global, uint32_t size_extrafield_global,
									 const char* comment, int method, int level, int raw,
									 int windowBits,int memLevel, int strategy,
									 const char* password, uint64_t crcForCrypting,
									 uint64_t versionMadeBy, uint64_t flagBase, int zip64)
{
	zip64_internal* zi;
	uint32_t size_filename;
	uint32_t size_comment;
	uint32_t i;
	int err = ZIP_OK;

	if(password != NULL)
		return ZIP_PARAMERROR;

	if(file == NULL)
		return ZIP_PARAMERROR;

	if(method != 0 && method != Z_DEFLATED)
		return ZIP_PARAMERROR;

	zi = (zip64_internal*) file;
	if(zi->in_opened_file_inzip == 1)
	{
		err = zipCloseFileInZip (file);
		if(err != ZIP_OK)
			return err;
	}

	if(filename == NULL)
		filename = "-";

	if(comment == NULL)
		size_comment = 0;
	else
		size_comment = (uint32_t)strlen(comment);

	size_filename = (uint32_t)strlen(filename);

	if(zipfi == NULL)
		zi->ci.dosDate = 0;
	else
	{
		if(zipfi->dosDate != 0)
			zi->ci.dosDate = zipfi->dosDate;
		else
			zi->ci.dosDate = zip64local_TmzDateToDosDate(&zipfi->tmz_date);
	}

	zi->ci.flag = flagBase;
	if(level == 8 || level == 9)
		zi->ci.flag |= 2;

	else if(level == 2)
		zi->ci.flag |= 4;

	else if(level == 1)
		zi->ci.flag |= 6;

	if(password != NULL)
		zi->ci.flag |= 1;

	zi->ci.crc32 = 0;
	zi->ci.method = method;
	zi->ci.encrypt = 0;
	zi->ci.stream_initialised = 0;
	zi->ci.pos_in_buffered_data = 0;
	zi->ci.raw = raw;
	zi->ci.pos_local_header = ZTELL64(zi->z_filefunc,zi->filestream);

	zi->ci.size_centralheader = SIZECENTRALHEADER + size_filename + size_extrafield_global + size_comment;
	zi->ci.size_centralExtraFree = 32; // Extra space we have reserved in case we need to add ZIP64 extra info data

	zi->ci.central_header = (char*)malloc((uint32_t)zi->ci.size_centralheader + zi->ci.size_centralExtraFree);

	zi->ci.size_centralExtra = size_extrafield_global;
	zip64local_putValue_inmemory(zi->ci.central_header,(uint64_t)CENTRALHEADERMAGIC,4);
	/* version info */
	zip64local_putValue_inmemory(zi->ci.central_header+4,(uint64_t)versionMadeBy,2);
	zip64local_putValue_inmemory(zi->ci.central_header+6,(uint64_t)20,2);
	zip64local_putValue_inmemory(zi->ci.central_header+8,(uint64_t)zi->ci.flag,2);
	zip64local_putValue_inmemory(zi->ci.central_header+10,(uint64_t)zi->ci.method,2);
	zip64local_putValue_inmemory(zi->ci.central_header+12,(uint64_t)zi->ci.dosDate,4);
	zip64local_putValue_inmemory(zi->ci.central_header+16,(uint64_t)0,4); /*crc*/
	zip64local_putValue_inmemory(zi->ci.central_header+20,(uint64_t)0,4); /*compr size*/
	zip64local_putValue_inmemory(zi->ci.central_header+24,(uint64_t)0,4); /*uncompr size*/
	zip64local_putValue_inmemory(zi->ci.central_header+28,(uint64_t)size_filename,2);
	zip64local_putValue_inmemory(zi->ci.central_header+30,(uint64_t)size_extrafield_global,2);
	zip64local_putValue_inmemory(zi->ci.central_header+32,(uint64_t)size_comment,2);
	zip64local_putValue_inmemory(zi->ci.central_header+34,(uint64_t)0,2); /*disk nm start*/

	if(zipfi == NULL)
		zip64local_putValue_inmemory(zi->ci.central_header+36,(uint64_t)0,2);
	else
		zip64local_putValue_inmemory(zi->ci.central_header+36,(uint64_t)zipfi->internal_fa,2);

	if(zipfi == NULL)
		zip64local_putValue_inmemory(zi->ci.central_header+38,(uint64_t)0,4);
	else
		zip64local_putValue_inmemory(zi->ci.central_header+38,(uint64_t)zipfi->external_fa,4);

	if(zi->ci.pos_local_header >= 0xffffffff)
		zip64local_putValue_inmemory(zi->ci.central_header+42,(uint64_t)0xffffffff,4);
	else
		zip64local_putValue_inmemory(zi->ci.central_header+42,(uint64_t)zi->ci.pos_local_header - zi->add_position_when_writting_offset,4);

	for (i = 0;i<size_filename;i++)
		*(zi->ci.central_header+SIZECENTRALHEADER+i) = *(filename+i);

	for (i = 0;i<size_extrafield_global;i++)
		*(zi->ci.central_header+SIZECENTRALHEADER+size_filename+i) =
		*(((const char*)extrafield_global)+i);

	for (i = 0;i<size_comment;i++)
		*(zi->ci.central_header+SIZECENTRALHEADER+size_filename+
		  size_extrafield_global+i) = *(comment+i);
	if(zi->ci.central_header == NULL)
		return ZIP_INTERNALERROR;

	zi->ci.zip64 = zip64;
	zi->ci.totalCompressedData = 0;
	zi->ci.totalUncompressedData = 0;
	zi->ci.pos_zip64extrainfo = 0;

	err = Write_LocalFileHeader(zi, filename, size_extrafield_local, extrafield_local);

	zi->ci.stream.avail_in = (uint32_t)0;
	zi->ci.stream.avail_out = (uint32_t)Z_BUFSIZE;
	zi->ci.stream.next_out = zi->ci.buffered_data;
	zi->ci.stream.total_in = 0;
	zi->ci.stream.total_out = 0;
	zi->ci.stream.data_type = Z_BINARY;

	if(err == ZIP_OK && zi->ci.method == Z_DEFLATED && !zi->ci.raw)
	{
		if(zi->ci.method == Z_DEFLATED)
		{
			zi->ci.stream.zalloc = (alloc_func)0;
			zi->ci.stream.zfree = (free_func)0;
			zi->ci.stream.opaque = (voidpf)0;

			if(windowBits > 0)
				windowBits = -windowBits;

			err = deflateInit2(&zi->ci.stream, level, Z_DEFLATED, windowBits, memLevel, strategy);

			if(err == Z_OK)
				zi->ci.stream_initialised = Z_DEFLATED;
		}
	}

	if(err == Z_OK)
		zi->in_opened_file_inzip = 1;

	return err;
}

extern int  zipOpenNewFileInZip4 (zipFile file, const char* filename, const zip_fileinfo* zipfi,
								  const void* extrafield_local, uint32_t size_extrafield_local,
								  const void* extrafield_global, uint32_t size_extrafield_global,
								  const char* comment, int method, int level, int raw,
								  int windowBits,int memLevel, int strategy,
								  const char* password, uint64_t crcForCrypting,
								  uint64_t versionMadeBy, uint64_t flagBase)
{
	return zipOpenNewFileInZip4_64 (file, filename, zipfi,
									extrafield_local, size_extrafield_local,
									extrafield_global, size_extrafield_global,
									comment, method, level, raw,
									windowBits, memLevel, strategy,
									password, crcForCrypting, versionMadeBy, flagBase, 0);
}

extern int  zipOpenNewFileInZip3 (zipFile file, const char* filename, const zip_fileinfo* zipfi,
								  const void* extrafield_local, uint32_t size_extrafield_local,
								  const void* extrafield_global, uint32_t size_extrafield_global,
								  const char* comment, int method, int level, int raw,
								  int windowBits,int memLevel, int strategy,
								  const char* password, uint64_t crcForCrypting)
{
	return zipOpenNewFileInZip4_64 (file, filename, zipfi,
									extrafield_local, size_extrafield_local,
									extrafield_global, size_extrafield_global,
									comment, method, level, raw,
									windowBits, memLevel, strategy,
									password, crcForCrypting, VERSIONMADEBY, 0, 0);
}

extern int  zipOpenNewFileInZip3_64(zipFile file, const char* filename, const zip_fileinfo* zipfi,
									const void* extrafield_local, uint32_t size_extrafield_local,
									const void* extrafield_global, uint32_t size_extrafield_global,
									const char* comment, int method, int level, int raw,
									int windowBits,int memLevel, int strategy,
									const char* password, uint64_t crcForCrypting, int zip64)
{
	return zipOpenNewFileInZip4_64 (file, filename, zipfi,
									extrafield_local, size_extrafield_local,
									extrafield_global, size_extrafield_global,
									comment, method, level, raw,
									windowBits, memLevel, strategy,
									password, crcForCrypting, VERSIONMADEBY, 0, zip64);
}

extern int  zipOpenNewFileInZip2(zipFile file, const char* filename, const zip_fileinfo* zipfi,
								 const void* extrafield_local, uint32_t size_extrafield_local,
								 const void* extrafield_global, uint32_t size_extrafield_global,
								 const char* comment, int method, int level, int raw)
{
	return zipOpenNewFileInZip4_64 (file, filename, zipfi,
									extrafield_local, size_extrafield_local,
									extrafield_global, size_extrafield_global,
									comment, method, level, raw,
									-MAX_WBITS, DEF_MEM_LEVEL, Z_DEFAULT_STRATEGY,
									NULL, 0, VERSIONMADEBY, 0, 0);
}

extern int  zipOpenNewFileInZip2_64(zipFile file, const char* filename, const zip_fileinfo* zipfi,
									const void* extrafield_local, uint32_t size_extrafield_local,
									const void* extrafield_global, uint32_t size_extrafield_global,
									const char* comment, int method, int level, int raw, int zip64)
{
	return zipOpenNewFileInZip4_64 (file, filename, zipfi,
									extrafield_local, size_extrafield_local,
									extrafield_global, size_extrafield_global,
									comment, method, level, raw,
									-MAX_WBITS, DEF_MEM_LEVEL, Z_DEFAULT_STRATEGY,
									NULL, 0, VERSIONMADEBY, 0, zip64);
}

extern int  zipOpenNewFileInZip64 (zipFile file, const char* filename, const zip_fileinfo* zipfi,
								   const void* extrafield_local, uint32_t size_extrafield_local,
								   const void*extrafield_global, uint32_t size_extrafield_global,
								   const char* comment, int method, int level, int zip64)
{
	return zipOpenNewFileInZip4_64 (file, filename, zipfi,
									extrafield_local, size_extrafield_local,
									extrafield_global, size_extrafield_global,
									comment, method, level, 0,
									-MAX_WBITS, DEF_MEM_LEVEL, Z_DEFAULT_STRATEGY,
									NULL, 0, VERSIONMADEBY, 0, zip64);
}

extern int  zipOpenNewFileInZip (zipFile file, const char* filename, const zip_fileinfo* zipfi,
								 const void* extrafield_local, uint32_t size_extrafield_local,
								 const void*extrafield_global, uint32_t size_extrafield_global,
								 const char* comment, int method, int level)
{
	return zipOpenNewFileInZip4_64 (file, filename, zipfi,
									extrafield_local, size_extrafield_local,
									extrafield_global, size_extrafield_global,
									comment, method, level, 0,
									-MAX_WBITS, DEF_MEM_LEVEL, Z_DEFAULT_STRATEGY,
									NULL, 0, VERSIONMADEBY, 0, 0);
}

static int zip64FlushWriteBuffer(zip64_internal* zi)
{
	int err = ZIP_OK;

	if(ZWRITE64(zi->z_filefunc,zi->filestream,zi->ci.buffered_data,zi->ci.pos_in_buffered_data) != zi->ci.pos_in_buffered_data)
		err = ZIP_ERRNO;

	zi->ci.totalCompressedData += zi->ci.pos_in_buffered_data;
	zi->ci.totalUncompressedData += zi->ci.stream.total_in;
	zi->ci.stream.total_in = 0;
	zi->ci.pos_in_buffered_data = 0;

	return err;
}

extern int zipWriteInFileInZip (zipFile file, const void* buf, uint64_t len)
{
	zip64_internal* zi;
	int err = ZIP_OK;

	if(file == NULL)
		return ZIP_PARAMERROR;
	zi = (zip64_internal*)file;

	if(zi->in_opened_file_inzip == 0)
		return ZIP_PARAMERROR;

	zi->ci.crc32 = crc32(zi->ci.crc32,buf,(uint32_t)len);
	zi->ci.stream.next_in = (Bytef*)buf;
	zi->ci.stream.avail_in = len;

	while(err == ZIP_OK && zi->ci.stream.avail_in > 0)
	{
		if(zi->ci.stream.avail_out == 0)
		{
			if(zip64FlushWriteBuffer(zi) == ZIP_ERRNO)
				err = ZIP_ERRNO;
			zi->ci.stream.avail_out = (uint32_t)Z_BUFSIZE;
			zi->ci.stream.next_out = zi->ci.buffered_data;
		}


		if(err != ZIP_OK)
			break;

		if((zi->ci.method == Z_DEFLATED) && (!zi->ci.raw))
		{
			uint64_t uTotalOutBefore = zi->ci.stream.total_out;
			err=deflate(&zi->ci.stream,  Z_NO_FLUSH);
			if(uTotalOutBefore > zi->ci.stream.total_out)
			{
				int bBreak = 0;
				bBreak++;
			}

			zi->ci.pos_in_buffered_data += (uint32_t) (zi->ci.stream.total_out - uTotalOutBefore);
		}
		else
		{
			uint32_t copy_this,i;
			if(zi->ci.stream.avail_in < zi->ci.stream.avail_out)
				copy_this = zi->ci.stream.avail_in;
			else
				copy_this = zi->ci.stream.avail_out;

			for (i = 0; i < copy_this; i++)
				*(((char*)zi->ci.stream.next_out)+i) =
				*(((const char*)zi->ci.stream.next_in)+i);
			{
				zi->ci.stream.avail_in -= copy_this;
				zi->ci.stream.avail_out-= copy_this;
				zi->ci.stream.next_in+= copy_this;
				zi->ci.stream.next_out+= copy_this;
				zi->ci.stream.total_in+= copy_this;
				zi->ci.stream.total_out+= copy_this;
				zi->ci.pos_in_buffered_data += copy_this;
			}
		}
	}
	return err;
}

extern int  zipCloseFileInZipRaw (zipFile file, uint64_t uncompressed_size, uint64_t crc32)
{
	return zipCloseFileInZipRaw64 (file, uncompressed_size, crc32);
}

extern int  zipCloseFileInZipRaw64 (zipFile file, uint64_t uncompressed_size, uint64_t crc32)
{
	zip64_internal* zi;
	uint64_t compressed_size, invalidValue = 0xffffffff;
	uint16_t datasize = 0;
	int err = ZIP_OK;

	if(file == NULL)
		return ZIP_PARAMERROR;
	zi = (zip64_internal*)file;

	if(zi->in_opened_file_inzip == 0)
		return ZIP_PARAMERROR;
	zi->ci.stream.avail_in = 0;

	if((zi->ci.method == Z_DEFLATED) && (!zi->ci.raw))
	{
		while (err == ZIP_OK)
		{
			uint64_t uTotalOutBefore;
			if(zi->ci.stream.avail_out == 0)
			{
				if(zip64FlushWriteBuffer(zi) == ZIP_ERRNO)
					err = ZIP_ERRNO;
				else
				{
					zi->ci.stream.avail_out = (uint32_t)Z_BUFSIZE;
					zi->ci.stream.next_out = zi->ci.buffered_data;
				}
			}

			if(err == ZIP_OK)
			{
				uTotalOutBefore = zi->ci.stream.total_out;
				err = deflate(&zi->ci.stream, Z_FINISH);
				zi->ci.pos_in_buffered_data += (uint32_t)(zi->ci.stream.total_out - uTotalOutBefore);
			}
		}
	}
	else if((zi->ci.method == Z_BZIP2ED) && (!zi->ci.raw))
	{
		err = ZIP_PARAMERROR;
	}

	if(err == Z_STREAM_END)
		err = ZIP_OK; /* this is normal */

	if((zi->ci.pos_in_buffered_data > 0) && (err == ZIP_OK))
	{
		if(zip64FlushWriteBuffer(zi)==ZIP_ERRNO)
			err = ZIP_ERRNO;
	}

	if((zi->ci.method == Z_DEFLATED) && (!zi->ci.raw))
	{
		int tmp_err = deflateEnd(&zi->ci.stream);
		if(err == ZIP_OK)
			err = tmp_err;
		zi->ci.stream_initialised = 0;
	}

	if(!zi->ci.raw)
	{
		crc32 = (uint64_t)zi->ci.crc32;
		uncompressed_size = zi->ci.totalUncompressedData;
	}
	compressed_size = zi->ci.totalCompressedData;

	// update Current Item crc and sizes,
	if(compressed_size >= 0xffffffff || uncompressed_size >= 0xffffffff || zi->ci.pos_local_header >= 0xffffffff)
	{
		/*version Made by*/
		zip64local_putValue_inmemory(zi->ci.central_header+4,(uint64_t)45,2);
		/*version needed*/
		zip64local_putValue_inmemory(zi->ci.central_header+6,(uint64_t)45,2);

	}

	zip64local_putValue_inmemory(zi->ci.central_header+16,crc32,4); /*crc*/


	if(compressed_size >= 0xffffffff)
		zip64local_putValue_inmemory(zi->ci.central_header+20, invalidValue,4); /*compr size*/
	else
		zip64local_putValue_inmemory(zi->ci.central_header+20, compressed_size,4); /*compr size*/

	/// set internal file attributes field
	if(zi->ci.stream.data_type == Z_ASCII)
		zip64local_putValue_inmemory(zi->ci.central_header+36,(uint64_t)Z_ASCII,2);

	if(uncompressed_size >= 0xffffffff)
		zip64local_putValue_inmemory(zi->ci.central_header+24, invalidValue,4); /*uncompr size*/
	else
		zip64local_putValue_inmemory(zi->ci.central_header+24, uncompressed_size,4); /*uncompr size*/

	// Add ZIP64 extra info field for uncompressed size
	if(uncompressed_size >= 0xffffffff)
		datasize += 8;

	// Add ZIP64 extra info field for compressed size
	if(compressed_size >= 0xffffffff)
		datasize += 8;

	// Add ZIP64 extra info field for relative offset to local file header of current file
	if(zi->ci.pos_local_header >= 0xffffffff)
		datasize += 8;

	if(datasize > 0)
	{
		char* p = NULL;

		if((uint64_t)(datasize + 4) > zi->ci.size_centralExtraFree)
		{
			// we can not write more data to the buffer that we have room for.
			return ZIP_BADZIPFILE;
		}

		p = zi->ci.central_header + zi->ci.size_centralheader;

		// Add Extra Information Header for 'ZIP64 information'
		zip64local_putValue_inmemory(p, 0x0001, 2); // HeaderID
		p += 2;
		zip64local_putValue_inmemory(p, datasize, 2); // DataSize
		p += 2;

		if(uncompressed_size >= 0xffffffff)
		{
			zip64local_putValue_inmemory(p, uncompressed_size, 8);
			p += 8;
		}

		if(compressed_size >= 0xffffffff)
		{
			zip64local_putValue_inmemory(p, compressed_size, 8);
			p += 8;
		}

		if(zi->ci.pos_local_header >= 0xffffffff)
		{
			zip64local_putValue_inmemory(p, zi->ci.pos_local_header, 8);
			//			p += 8; //Unread afterward
		}

		// Update how much extra free space we got in the memory buffer
		// and increase the centralheader size so the new ZIP64 fields are included
		// ( 4 below is the size of HeaderID and DataSize field )
		zi->ci.size_centralExtraFree -= datasize + 4;
		zi->ci.size_centralheader += datasize + 4;

		// Update the extra info size field
		zi->ci.size_centralExtra += datasize + 4;
		zip64local_putValue_inmemory(zi->ci.central_header+30,(uint64_t)zi->ci.size_centralExtra,2);
	}

	if(err == ZIP_OK)
		err = add_data_in_datablock(&zi->central_dir, zi->ci.central_header, (uint64_t)zi->ci.size_centralheader);

	free(zi->ci.central_header);

	if(err == ZIP_OK)
	{
		// Update the LocalFileHeader with the new values.

		uint64_t cur_pos_inzip = ZTELL64(zi->z_filefunc,zi->filestream);

		if(ZSEEK64(zi->z_filefunc,zi->filestream, zi->ci.pos_local_header + 14,ZLIB_FILEFUNC_SEEK_SET) != 0)
			err = ZIP_ERRNO;

		if(err == ZIP_OK)
			err = zip64local_putValue(&zi->z_filefunc,zi->filestream,crc32,4); /* crc 32, unknown */

		if(uncompressed_size >= 0xffffffff)
		{
			if(zi->ci.pos_zip64extrainfo > 0)
			{
				// Update the size in the ZIP64 extended field.
				if(ZSEEK64(zi->z_filefunc,zi->filestream, zi->ci.pos_zip64extrainfo + 4,ZLIB_FILEFUNC_SEEK_SET) != 0)
					err = ZIP_ERRNO;

				if(err == ZIP_OK) /* compressed size, unknown */
					err = zip64local_putValue(&zi->z_filefunc, zi->filestream, uncompressed_size, 8);

				if(err == ZIP_OK) /* uncompressed size, unknown */
					err = zip64local_putValue(&zi->z_filefunc, zi->filestream, compressed_size, 8);
			}
		}
		else
		{
			if(err == ZIP_OK) /* compressed size, unknown */
				err = zip64local_putValue(&zi->z_filefunc,zi->filestream,compressed_size,4);

			if(err == ZIP_OK) /* uncompressed size, unknown */
				err = zip64local_putValue(&zi->z_filefunc,zi->filestream,uncompressed_size,4);
		}

		if(ZSEEK64(zi->z_filefunc,zi->filestream, cur_pos_inzip,ZLIB_FILEFUNC_SEEK_SET) != 0)
			err = ZIP_ERRNO;
	}

	zi->number_entry ++;
	zi->in_opened_file_inzip = 0;

	return err;
}

extern int  zipCloseFileInZip (zipFile file)
{
	return zipCloseFileInZipRaw (file,0,0);
}

int Write_Zip64EndOfCentralDirectoryLocator(zip64_internal* zi, uint64_t zip64eocd_pos_inzip)
{
	int err = ZIP_OK;
	uint64_t pos = zip64eocd_pos_inzip - zi->add_position_when_writting_offset;

	err = zip64local_putValue(&zi->z_filefunc,zi->filestream,(uint64_t)ZIP64ENDLOCHEADERMAGIC,4);

	/*num disks*/
	if(err == ZIP_OK) /* number of the disk with the start of the central directory */
		err = zip64local_putValue(&zi->z_filefunc,zi->filestream,(uint64_t)0,4);

	/*relative offset*/
	if(err == ZIP_OK) /* Relative offset to the Zip64EndOfCentralDirectory */
		err = zip64local_putValue(&zi->z_filefunc,zi->filestream, pos,8);

	/*total disks*/ /* Do not support spawning of disk so always say 1 here*/
	if(err == ZIP_OK) /* number of the disk with the start of the central directory */
		err = zip64local_putValue(&zi->z_filefunc,zi->filestream,(uint64_t)1,4);

	return err;
}

int Write_Zip64EndOfCentralDirectoryRecord(zip64_internal* zi, uint64_t size_centraldir, uint64_t centraldir_pos_inzip)
{
	int err = ZIP_OK;

	uint64_t Zip64DataSize = 44;

	err = zip64local_putValue(&zi->z_filefunc,zi->filestream,(uint64_t)ZIP64ENDHEADERMAGIC,4);

	if(err == ZIP_OK) /* size of this 'zip64 end of central directory' */
		err = zip64local_putValue(&zi->z_filefunc,zi->filestream,(uint64_t)Zip64DataSize,8); // why uint64_t of this ?

	if(err == ZIP_OK) /* version made by */
		err = zip64local_putValue(&zi->z_filefunc,zi->filestream,(uint64_t)45,2);

	if(err == ZIP_OK) /* version needed */
		err = zip64local_putValue(&zi->z_filefunc,zi->filestream,(uint64_t)45,2);

	if(err == ZIP_OK) /* number of this disk */
		err = zip64local_putValue(&zi->z_filefunc,zi->filestream,(uint64_t)0,4);

	if(err == ZIP_OK) /* number of the disk with the start of the central directory */
		err = zip64local_putValue(&zi->z_filefunc,zi->filestream,(uint64_t)0,4);

	if(err == ZIP_OK) /* total number of entries in the central dir on this disk */
		err = zip64local_putValue(&zi->z_filefunc, zi->filestream, zi->number_entry, 8);

	if(err == ZIP_OK) /* total number of entries in the central dir */
		err = zip64local_putValue(&zi->z_filefunc, zi->filestream, zi->number_entry, 8);

	if(err == ZIP_OK) /* size of the central directory */
		err = zip64local_putValue(&zi->z_filefunc,zi->filestream,(uint64_t)size_centraldir,8);

	if(err == ZIP_OK) /* offset of start of central directory with respect to the starting disk number */
	{
		uint64_t pos = centraldir_pos_inzip - zi->add_position_when_writting_offset;
		err = zip64local_putValue(&zi->z_filefunc,zi->filestream, (uint64_t)pos,8);
	}
	return err;
}
int Write_EndOfCentralDirectoryRecord(zip64_internal* zi, uint64_t size_centraldir, uint64_t centraldir_pos_inzip)
{
	int err = ZIP_OK;

	/*signature*/
	err = zip64local_putValue(&zi->z_filefunc,zi->filestream,(uint64_t)ENDHEADERMAGIC,4);

	if(err == ZIP_OK) /* number of this disk */
		err = zip64local_putValue(&zi->z_filefunc,zi->filestream,(uint64_t)0,2);

	if(err == ZIP_OK) /* number of the disk with the start of the central directory */
		err = zip64local_putValue(&zi->z_filefunc,zi->filestream,(uint64_t)0,2);

	if(err == ZIP_OK) /* total number of entries in the central dir on this disk */
	{
		{
			if(zi->number_entry >= 0xFFFF)
				err = zip64local_putValue(&zi->z_filefunc,zi->filestream,(uint64_t)0xffff,2); // use value in ZIP64 record
			else
				err = zip64local_putValue(&zi->z_filefunc,zi->filestream,(uint64_t)zi->number_entry,2);
		}
	}

	if(err == ZIP_OK) /* total number of entries in the central dir */
	{
		if(zi->number_entry >= 0xFFFF)
			err = zip64local_putValue(&zi->z_filefunc,zi->filestream,(uint64_t)0xffff,2); // use value in ZIP64 record
		else
			err = zip64local_putValue(&zi->z_filefunc,zi->filestream,(uint64_t)zi->number_entry,2);
	}

	if(err == ZIP_OK) /* size of the central directory */
		err = zip64local_putValue(&zi->z_filefunc,zi->filestream,(uint64_t)size_centraldir,4);

	if(err == ZIP_OK) /* offset of start of central directory with respect to the starting disk number */
	{
		uint64_t pos = centraldir_pos_inzip - zi->add_position_when_writting_offset;
		if(pos >= 0xffffffff)
		{
			err = zip64local_putValue(&zi->z_filefunc,zi->filestream, (uint64_t)0xffffffff,4);
		}
		else
			err = zip64local_putValue(&zi->z_filefunc,zi->filestream, (uint64_t)(centraldir_pos_inzip - zi->add_position_when_writting_offset),4);
	}

	return err;
}

int Write_GlobalComment(zip64_internal* zi, const char* global_comment)
{
	int err = ZIP_OK;
	uint32_t size_global_comment = 0;

	if(global_comment != NULL)
		size_global_comment = (uint32_t)strlen(global_comment);

	err = zip64local_putValue(&zi->z_filefunc,zi->filestream,(uint64_t)size_global_comment,2);

	if(err == ZIP_OK && size_global_comment > 0)
	{
		if(ZWRITE64(zi->z_filefunc,zi->filestream, global_comment, size_global_comment) != size_global_comment)
			err = ZIP_ERRNO;
	}
	return err;
}

extern int  zipClose (zipFile file, const char* global_comment)
{
	zip64_internal* zi;
	int err = 0;
	uint64_t size_centraldir = 0;
	uint64_t centraldir_pos_inzip;
	uint64_t pos;

	if(file == NULL)
		return ZIP_PARAMERROR;

	zi = (zip64_internal*)file;

	if(zi->in_opened_file_inzip == 1)
	{
		err = zipCloseFileInZip (file);
	}

	if(global_comment == NULL)
		global_comment = zi->globalcomment;

	centraldir_pos_inzip = ZTELL64(zi->z_filefunc,zi->filestream);

	if(err == ZIP_OK)
	{
		linkedlist_datablock_internal* ldi = zi->central_dir.first_block;
		while (ldi != NULL)
		{
			if((err == ZIP_OK) && (ldi->filled_in_this_block > 0))
			{
				if(ZWRITE64(zi->z_filefunc,zi->filestream, ldi->data, ldi->filled_in_this_block) != ldi->filled_in_this_block)
					err = ZIP_ERRNO;
			}

			size_centraldir += ldi->filled_in_this_block;
			ldi = ldi->next_datablock;
		}
	}
	free_linkedlist(&(zi->central_dir));

	pos = centraldir_pos_inzip - zi->add_position_when_writting_offset;
	if(pos >= 0xffffffff)
	{
		uint64_t Zip64EOCDpos = ZTELL64(zi->z_filefunc,zi->filestream);
		Write_Zip64EndOfCentralDirectoryRecord(zi, size_centraldir, centraldir_pos_inzip);

		Write_Zip64EndOfCentralDirectoryLocator(zi, Zip64EOCDpos);
	}

	if(err == ZIP_OK)
		err = Write_EndOfCentralDirectoryRecord(zi, size_centraldir, centraldir_pos_inzip);

	if(err == ZIP_OK)
		err = Write_GlobalComment(zi, global_comment);

	if(ZCLOSE64(zi->z_filefunc,zi->filestream) != 0)
		if(err == ZIP_OK)
			err = ZIP_ERRNO;

	free(zi->globalcomment);
	free(zi);

	return err;
}

extern int zipRemoveExtraInfoBlock (char* pData, int* dataLen, short sHeader)
{
	char* p = pData;
	uint size = 0;
	char* pNewHeader;
	char* pTmp;
	short header;
	uint16_t dataSize;

	int retVal = ZIP_OK;

	if(pData == NULL || *dataLen < 4)
		return ZIP_PARAMERROR;

	pNewHeader = malloc((uint) *dataLen * sizeof(char));
	pTmp = pNewHeader;

	while(p < (pData + *dataLen))
	{
		header = *(short*)p;
		dataSize = (uint16_t) *(((short*)p)+1);

		if( header == sHeader ) // Header found.
		{
			p += dataSize + 4; // skip it. do not copy to temp buffer
		}
		else if((int16_t) dataSize >= 0)
		{
			// Extra Info block should not be removed, So copy it to the temp buffer.
			memcpy(pTmp, p, dataSize + 4);
			p += dataSize + 4;
			size += dataSize + 4;
		}

	}

	if(size < (uint32_t) *dataLen)
	{
		// clean old extra info block.
		memset(pData, 0, (uint32_t) *dataLen);
		
		// copy the new extra info block over the old
		if(size > 0)
			memcpy(pData, pNewHeader, (uint32_t) size);
		
		// set the new extra info size
		*dataLen = (int) size;
		
		retVal = ZIP_OK;
	}
	else
		retVal = ZIP_ERRNO;
	
	free(pNewHeader);
	
	return retVal;
}
