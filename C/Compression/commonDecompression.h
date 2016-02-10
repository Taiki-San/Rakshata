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

typedef bool (*open_archive_ptr) (ARCHIVE * structure, const char * path);
typedef void (*jump_back_begining_ptr) (ARCHIVE * structure);
typedef bool (*move_next_file_ptr) (ARCHIVE * structure);
typedef size_t (*get_size_ptr) (ARCHIVE * structure);
typedef const char * (*get_name_ptr) (ARCHIVE * structure);
typedef bool (*is_EOF_ptr) (ARCHIVE * structure);
typedef int (*read_data_ptr) (ARCHIVE * structure, rawData * buffer, uint64_t length);
typedef void (*cleanup_entry_ptr) (ARCHIVE * structure);
typedef void (*close_archive_ptr) (ARCHIVE * structure);
typedef const char * (*get_error_string_ptr) (ARCHIVE * structure);

struct libArchiveWrapper
{
	void * archive;
	void * workingEntry;
	
	struct
	{
		open_archive_ptr open_archive;
		jump_back_begining_ptr jump_back_begining;
		move_next_file_ptr move_next_file;
		get_size_ptr get_size;
		get_name_ptr get_name;
		is_EOF_ptr is_EOF;
		read_data_ptr read_data;
		cleanup_entry_ptr cleanup_entry;
		close_archive_ptr close_archive;
		get_error_string_ptr get_error_string;
	} utils;
	
	void * fileHandle;
	
	char ** fileList;
	uint64_t currentEntryLength;
	uint nbFiles;
	bool isEOF;
};


enum
{
	LIB_UNARR,
	LIB_LIBARCHIVE
};

void configureLibarchiveJumptable(ARCHIVE * archive);
void configureUnarrJumptable(ARCHIVE * archive);

ARCHIVE * openArchiveFromFile(const char * path, byte format);
bool fileExistInArchive(ARCHIVE * archive, const char * filename);

bool archiveLocateFile(ARCHIVE * archive, const char * filename);
bool archiveExtractOnefile(ARCHIVE * archive, const char* filename, const char* outputPath);
bool archiveExtractToMem(ARCHIVE * archive, const char* filename, byte ** data, uint64_t * length);

void closeArchive(ARCHIVE * archive);