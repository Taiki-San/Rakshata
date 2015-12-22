/*********************************************************************************************
 **	__________         __           .__            __                 ________   _______   	**
 **	\______   \_____  |  | __  _____|  |__ _____ _/  |______   	___  _\_____  \  \   _  \  	**
 **	 |       _/\__  \ |  |/ / /  ___/  |  \\__  \\   __\__  \  	\  \/ //  ____/  /  /_\  \ 	**
 **	 |    |   \ / __ \|    <  \___ \|   Y  \/ __ \|  |  / __ \__ \   //       \  \  \_/   \	**
 **	 |____|_  /(____  /__|_ \/____  >___|  (____  /__| (____  /	  \_/ \_______ \ /\_____  /	**
 **	        \/      \/     \/     \/     \/     \/          \/ 	              \/ \/     \/ 	**
 **                                                                                         **
 **		Source code and assets are property of Taiki, distribution is stricly forbidden		**
 **                                                                                         **
 *********************************************************************************************/

#if !TARGET_OS_IPHONE

	#import "RakExportController.h"
	#import "RakImportController.h"

#else

	#import "RakImportController.h"

#endif

//Extensions
#define SOURCE_FILE_EXT 		@"rakSource"
#define ARCHIVE_FILE_EXT		@"rak"
#define EXTERNAL_FILE_EXT_ZIP	@[@"zip", @"cbz"]
#define EXTERNAL_FILE_EXT_RAR	@[@"rar", @"cbr"]
#define EXTERNAL_FILE_EXT_PLAIN @[@"pdf"]
#define ARCHIVE_SUPPORT			@[@"rak", @"zip", @"cbz", @"rar", @"cbr", @"pdf"]
#define DEFAULT_ARCHIVE_SUPPORT	@[@"rak", @"cbz", @"cbr"]

//Values
#define CHAPTER_FOR_IMPORTED_VOLUMES 42