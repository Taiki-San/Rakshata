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

@interface RakImportController : NSObject

+ (void) importFile : (NSString *) file : (BOOL) generatedArchive;

@end

@interface RakImportItem : NSObject

@property NSString * path;
@property PROJECT_DATA_EXTRA projectData;
@property int contentID;
@property BOOL isTome;

- (BOOL) isReadable;
- (BOOL) install : (unzFile *) archive;
- (void) processThumbs : (unzFile *) archive;

@end


#define ARCHIVE_VERSION 1

#define METADATA_FILE "catalog.json"

#define RAK_STRING_VERSION				@"version"
#define RAK_STRING_METADATA				@"metadata"
#define RAK_STRING_METADATA_PROJECT		@"projects"
#define RAK_STRING_METADATA_PROJECTID	@"projectID"
#define RAK_STRING_METADATA_PROJECTNAME	@"projectName"
#define RAK_STRING_METADATA_REPOTYPE	@"repoType"
#define RAK_STRING_METADATA_REPOURL		@"repoURL"
#define RAK_STRING_METADATA_REPO_PROJID	@"repoProjectID"
#define RAK_STRING_METADATA_DESCRIPTION	@"description"
#define RAK_STRING_METADATA_AUTHOR		@"author"
#define RAK_STRING_METADATA_STATUS		@"status"
#define RAK_STRING_METADATA_TAGMASK		@"tagMask"
#define RAK_STRING_METADATA_ASIANORDER	@"rightToLeft"
#define RAK_STRING_METADATA_IMG_GRID	@"imgGrid"
#define RAK_STRING_METADATA_IMG_GRID_2X	@"imgGrid@2x"
#define RAK_STRING_METADATA_IMG_CT		@"imgCT"
#define RAK_STRING_METADATA_IMG_CT_2X	@"imgCT@2x"
#define RAK_STRING_METADATA_IMG_DD		@"imgDD"
#define RAK_STRING_METADATA_IMG_DD_2X	@"imgDD@2x"

#define RAK_STRING_CONTENT				@"content"
#define RAK_STRING_CONTENT_DIRECTORY	@"dirName"
#define RAK_STRING_CONTENT_PROJECT		@"projectID"
#define RAK_STRING_CONTENT_ISTOME		@"isTome"
#define RAK_STRING_CONTENT_VOL_DETAILS	@"detail"
#define RAK_STRING_CONTENT_ID			@"ID"