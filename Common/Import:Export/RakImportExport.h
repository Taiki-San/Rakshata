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