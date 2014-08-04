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

@interface RakAppDelegate : NSObject <NSApplicationDelegate>
{
	Series * tabSerie;
	CTSelec * tabCT;
	Reader * tabReader;
	MDL * tabMDL;
	
	BOOL loginPromptOpen;
	pthread_cond_t loginLock;
	MUTEX_VAR loginMutex;
}

@property (assign) IBOutlet NSWindow *window;

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