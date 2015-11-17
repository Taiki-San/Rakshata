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

@class RakAboutWindow;

@interface RakAppDelegate : NSObject <NSApplicationDelegate, NSWindowDelegate, NSUserNotificationCenterDelegate>
{
	Series * tabSerie;
	CTSelec * tabCT;
	Reader * tabReader;
	MDL * tabMDL;
	
	RakAboutWindow * aboutWindow;
	RakPrefsWindow * prefWindow;
	
	BOOL loginPromptOpen;
	pthread_cond_t loginLock;
	MUTEX_VAR loginMutex;
}

@property BOOL initialized;
@property (weak) IBOutlet RakWindow *window;
@property BOOL haveDistractionFree;
@property BOOL hasFocus;

- (RakContentView*) getContentView;

- (pthread_cond_t*) sharedLoginLock;
- (MUTEX_VAR *) sharedLoginMutex : (BOOL) locked;

- (void) openLoginPrompt;
- (void) loginPromptClosed;

- (Series *)	serie;
- (CTSelec *)	CT;
- (MDL *)		MDL;
- (Reader *)	reader;

@end

//Extensions
#define SOURCE_FILE_EXT 		@"rakSource"
#define ARCHIVE_FILE_EXT		@"rak"
#define EXTERNAL_FILE_EXT_ZIP	@[@"zip", @"cbz"]
#define EXTERNAL_FILE_EXT_RAR	@[@"rar", @"cbr"]
#define EXTERNAL_FILE_EXT_PLAIN @[@"pdf"]
#define ARCHIVE_SUPPORT			@[@"rak", @"zip", @"cbz", @"rar", @"cbr", @"pdf"]
#define DEFAULT_ARCHIVE_SUPPORT	@[@"rak", @"cbz", @"cbr"]
