/*********************************************************************************************
 **	__________         __           .__            __                 ________   _______   	**
 **	\______   \_____  |  | __  _____|  |__ _____ _/  |______   	___  _\_____  \  \   _  \  	**
 **	 |       _/\__  \ |  |/ / /  ___/  |  \\__  \\   __\__  \  	\  \/ //  ____/  /  /_\  \ 	**
 **	 |    |   \ / __ \|    <  \___ \|   Y  \/ __ \|  |  / __ \_  \   //       \  \  \_/   \	**
 **	 |____|_  /(____  /__|_ \/____  >___|  (____  /__| (____  /	  \_/ \_______ \ /\_____  /	**
 **	        \/      \/     \/     \/     \/     \/          \/ 	              \/ \/     \/ 	**
 **                                                                                         **
 **			This Source Code Form is subject to the terms of the Mozilla Public				**
 **			License, v. 2.0. If a copy of the MPL was not distributed with this				**
 **			file, You can obtain one at https://mozilla.org/MPL/2.0/.						**
 **                                                                                         **
 **                     			© Taiki 2011-2016                                       **
 **                                                                                         **
 *********************************************************************************************/

#include <archive.h>
#include <archive_entry.h>

#if (TARGET_OS_SIMULATOR)
	#define DISABLE_LIBARCHIVE
#endif

#ifdef DISABLE_LIBARCHIVE

void configureLibarchiveJumptable(ARCHIVE * archive)
{
	
}

#else
struct archive * getArchive()
{
	//Create the libarchive archive
	struct archive * archive = archive_read_new();
	
	//Add support for the format we want to support
	//RAR support of libarchive is incomplete, and thus we don't use it
	//	archive_read_support_format_rar(archive);
	archive_read_support_format_ar(archive);
	archive_read_support_format_cab(archive);
	archive_read_support_format_cpio(archive);
	archive_read_support_format_gnutar(archive);
	archive_read_support_format_iso9660(archive);
	archive_read_support_format_lha(archive);
	archive_read_support_format_mtree(archive);
	archive_read_support_format_raw(archive);
	archive_read_support_format_tar(archive);
	archive_read_support_format_7zip(archive);
	archive_read_support_filter_all(archive);
	
	return archive;
}

bool openLibArchiveFromFile(ARCHIVE * archive, const char * path)
{
	archive->fileHandle = fopen(path, "rb");
	if(archive->fileHandle == NULL)
	{
		logR("Couldn't open the file!");
		return false;
	}
	
	//Create the libarchive archive
	archive->archive = getArchive();
	
	//Open the actual file
	if(archive_read_open_FILE(archive->archive, archive->fileHandle) != ARCHIVE_OK)
	{
		logR("Couldn't open the archive! Error message: %s", archive_error_string(archive->archive));
		archive->utils.close_archive(archive);
		fclose(archive->fileHandle);
		return false;
	}
	
	return true;
}

void closeLibArchive(ARCHIVE * archive)
{
	if(archive->archive != NULL)
		archive_read_free(archive->archive);
	
	fclose(archive->fileHandle);
}

bool libArchiveMoveToNextFile(ARCHIVE * archive)
{
	if(archive == NULL)
		return false;
	
	struct archive_entry * entry = NULL;
	
	int retValue = archive_read_next_header(archive->archive, &entry);
	
	if(retValue == ARCHIVE_OK && entry != NULL)
	{
		archive->workingEntry = entry;
		return true;
	}
	
	if(retValue == ARCHIVE_EOF)
		archive->isEOF |= true;
	
	return false;
}

bool libArchiveIsEOF(ARCHIVE * archive)
{
	return archive->isEOF;
}

void libArchiveJumpBackAtBegining(ARCHIVE * archive)
{
	archive_read_free(archive->archive);
	archive->workingEntry = NULL;
	
	rewind(archive->fileHandle);
	archive->archive = getArchive();
	
	if(archive_read_open_FILE(archive->archive, archive->fileHandle) != ARCHIVE_OK)
	{
		logR("Couldn't rewind the archive! Error message: %s", archive_error_string(archive->archive));
		archive->archive = NULL;
	}
	else
		archive->isEOF = false;
}

size_t libarchiveGetSize(ARCHIVE * archive)
{
	if(archive->workingEntry == NULL)
		return 0;
	
	int64_t value = archive_entry_size(archive->workingEntry);
	if(value < 0)
		value = 0;
	
	return (size_t) value;
}

const char * libarchiveGetName(ARCHIVE * archive)
{
	if(archive->workingEntry != NULL)
		return archive_entry_pathname(archive->workingEntry);
	
	return NULL;
}

int libarchiveReadData(ARCHIVE * archive, rawData * buffer, uint64_t length)
{
	if(archive == NULL || archive->workingEntry == NULL)
		return -1;
	
	return archive_read_data(archive->archive, buffer, sizeof(buffer));
}

const char * libarchiveErrorString(ARCHIVE * archive)
{
	return archive_error_string(archive->archive);
}

void configureLibarchiveJumptable(ARCHIVE * archive)
{
	archive->utils.open_archive = openLibArchiveFromFile;
	archive->utils.jump_back_begining = libArchiveJumpBackAtBegining;
	archive->utils.move_next_file = libArchiveMoveToNextFile;
	archive->utils.get_size = libarchiveGetSize;
	archive->utils.get_name = libarchiveGetName;
	archive->utils.is_EOF = libArchiveIsEOF;
	archive->utils.read_data = libarchiveReadData;
	archive->utils.cleanup_entry = NULL;
	archive->utils.close_archive = closeLibArchive;
	archive->utils.get_error_string = libarchiveErrorString;
}

#endif
