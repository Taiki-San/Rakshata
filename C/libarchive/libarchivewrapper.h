/*********************************************************************************************
 **	__________         __           .__            __                 ________   _______   	**
 **	\______   \_____  |  | __  _____|  |__ _____ _/  |______   	___  _\_____  \  \   _  \  	**
 **	 |       _/\__  \ |  |/ / /  ___/  |  \\__  \\   __\__  \  	\  \/ //  ____/  /  /_\  \ 	**
 **	 |    |   \ / __ \|    <  \___ \|   Y  \/ __ \|  |  / __ \__ \   //       \  \  \_/   \	**
 **	 |____|_  /(____  /__|_ \/____  >___|  (____  /__| (____  /	  \_/ \_______ \ /\_____  /	**
 **	        \/      \/     \/     \/     \/     \/          \/ 	              \/ \/     \/ 	**
 **                                                                                          **
 **		Source code and assets are property of Taiki, distribution is stricly forbidden		**
 **                                                                                          **
 *********************************************************************************************/

#include <archive.h>
#include <archive_entry.h>

typedef struct libArchiveWrapper
{
	struct archive * archive;
	struct archive_entry * cachedEntry;

	FILE * fileHandle;

	char ** fileList;
	uint nbFiles;

} ARCHIVE;

//Utils
ARCHIVE * openArchiveFromFile(const char * path);
void rarJumpBackAtBegining(ARCHIVE * archive);

bool rarExtractOnefile(ARCHIVE * archive, const char* filename, const char* outputPath);
bool rarExtractToMem(ARCHIVE * archive, const char* filename, byte ** data, uint64_t * length);

bool fileExistInArchive(ARCHIVE * archive, const char * filename);
bool rarLocateFile(ARCHIVE * archive, void ** entryBackup, const char * filename);

void closeArchive(ARCHIVE * archive);