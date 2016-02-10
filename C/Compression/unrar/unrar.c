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

#include <unarr.h>

bool openRarArchiveFromFile(ARCHIVE * archive, const char * path)
{
	archive->fileHandle = ar_open_file(path);
	if(archive->fileHandle == NULL)
	{
		logR("Couldn't open RAR archive's file");
		return false;
	}
	
	archive->archive = ar_open_rar_archive(archive->fileHandle);
	if(archive->archive == NULL)
	{
		logR("Couldn't open RAR archive");
		return false;
	}
	
	return true;
}

void _closeArchive(ARCHIVE * archive)
{
	ar_close_archive(archive->archive);
	ar_close(archive->fileHandle);
}

bool isEOF(ARCHIVE * archive)
{
	return ar_at_eof(archive->archive);
}

bool moveToNextFile(ARCHIVE * archive)
{
	bool output = ar_parse_entry(archive->archive) && !isEOF(archive);

	if(output && archive->currentEntryLength != UINT64_MAX)
		archive->currentEntryLength = UINT64_MAX;
	
	return output;
}

int readData(ARCHIVE * archive, rawData * buffer, uint64_t length)
{
	if(archive->currentEntryLength == UINT64_MAX)
	{
		archive->currentEntryLength = (uint64_t) archive->utils.get_size(archive);
		if((int64_t) archive->currentEntryLength < 0)
		{
			archive->currentEntryLength = UINT64_MAX;
			return -1;
		}
	}
	
	if(length > archive->currentEntryLength)
		length = archive->currentEntryLength;

	if(!ar_entry_uncompress(archive->archive, buffer, length))
		return -1;

	archive->currentEntryLength -= length;
	return (int) length;
}

void jumpBackAtBegining(ARCHIVE * archive)
{
	ar_seek(archive->archive, 0, SEEK_SET);
}

size_t getSize(ARCHIVE * archive)
{
	return ar_entry_get_size(archive->archive);
}

const char * getName(ARCHIVE * archive)
{
	return ar_entry_get_name(archive->archive);
}

void configureUnarrJumptable(ARCHIVE * archive)
{
	archive->utils.open_archive = openRarArchiveFromFile;
	archive->utils.jump_back_begining = jumpBackAtBegining;
	archive->utils.move_next_file = moveToNextFile;
	archive->utils.get_size = getSize;
	archive->utils.get_name = getName;
	archive->utils.is_EOF = isEOF;
	archive->utils.read_data = readData;
	archive->utils.cleanup_entry = NULL;
	archive->utils.close_archive = _closeArchive;
	archive->utils.get_error_string = NULL;
}