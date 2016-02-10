/*********************************************************************************************
 **	__________         __           .__            __                 ________   _______   	**
 **	\______   \_____  |  | __  _____|  |__ _____ _/  |______   	___  _\_____  \  \   _  \  	**
 **	 |       _/\__  \ |  |/ / /  ___/  |  \\__  \\   __\__  \  	\  \/ //  ____/  /  /_\  \ 	**
 **	 |    |   \ / __ \|    <  \___ \|   Y  \/ __ \|  |  / __ \_  \   //       \  \  \_/   \	**
 **	 |____|_  /(____  /__|_ \/____  >___|  (____  /__| (____  /	  \_/ \_______ \ /\_____  /	**
 **	        \/      \/     \/     \/     \/     \/          \/ 	              \/ \/     \/ 	**
 **                                                                                          **
 **			This Source Code Form is subject to the terms of the Mozilla Public				**
 **			License, v. 2.0. If a copy of the MPL was not distributed with this				**
 **			file, You can obtain one at https://mozilla.org/MPL/2.0/.						**
 **                                                                                         **
 **                     			© Taiki 2011-2016                                       **
 **                                                                                          **
 *********************************************************************************************/

#include <archive.h>
#include <archive_entry.h>

//Utils
ARCHIVE * openArchiveFromFile(const char * path);
void _7zipJumpBackAtBegining(ARCHIVE * archive);

bool _7zipExtractOnefile(ARCHIVE * archive, const char* filename, const char* outputPath);
bool _7zipExtractToMem(ARCHIVE * archive, const char* filename, byte ** data, uint64_t * length);

bool fileExistInArchive(ARCHIVE * archive, const char * filename);
bool _7zipLocateFile(ARCHIVE * archive, void ** entryBackup, const char * filename);

void closeArchive(ARCHIVE * archive);