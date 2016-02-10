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

typedef struct libArchiveWrapper ARCHIVE;

typedef void* (*open_archive_ptr) (ARCHIVE * structure, char * path);
typedef void (*jump_back_begining_ptr) (ARCHIVE * structure);
typedef void (*close_archive_ptr) (ARCHIVE * structure);


struct libArchiveWrapper
{
	void * archive;
	void * cachedEntry;
	
	void * fileHandle;
	
	char ** fileList;
	uint nbFiles;
	
	struct
	{
		open_archive_ptr open_archive;
		jump_back_begining_ptr jump_back_begining;
		close_archive_ptr close_archive;
	} utils;
	
};

#include "libarchivewrapper.h"
#include "unarrwrapper.h"