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

@implementation RakAppDelegate

- (void) awakeFromNib
{
	loginPromptOpen = NO;
	
	RakView * contentView = [self earlyInit];
	
	pthread_cond_init(&loginLock, NULL);
	pthread_mutex_init(&loginMutex, NULL);
	
	[self mainInit : contentView];
	
	deleteCrashFile();
	
	_initialized = YES;
	RakRealApp.savedContext = nil;
}

- (RakView *) earlyInit
{
	return nil;
}

- (void) mainInit : (RakView *) contentView
{
	
}

#pragma mark - Login

- (pthread_cond_t*) sharedLoginLock
{
	return &loginLock;
}

- (MUTEX_VAR *) sharedLoginMutex : (BOOL) locked
{
	if(locked)
		pthread_mutex_trylock(&loginMutex);
	
	return &loginMutex;
}

- (BOOL) loginPromptOpen
{
	return loginPromptOpen;
}

- (void) openLoginPrompt
{
	if(loginPromptOpen)
		return;
	else
		loginPromptOpen = YES;
	
	NSArray * array;
	
	[[NSBundle mainBundle] loadNibNamed:@"auth" owner:nil topLevelObjects:&array];
	
	for(RakAuthController * object in array)
	{
		if([object class] == [RakAuthController class])
		{
			[object launch];
			return;
		}
	}
}

- (void) loginPromptClosed
{
	loginPromptOpen = NO;
	
	if(COMPTE_PRINCIPAL_MAIL != NULL)
	{
		pthread_cond_broadcast(&loginLock);
	}
}

#pragma mark - Access to tabs

- (Series *)	serie	{	if(![NSThread isMainThread]) 	{	while(!self.initialized);	}	 return tabSerie;	}
- (CTSelec *)	CT		{	if(![NSThread isMainThread]) 	{	while(!self.initialized);	}	 return tabCT;		}
- (MDL *)		MDL		{	if(![NSThread isMainThread]) 	{	while(!self.initialized);	}	 return tabMDL;		}
- (Reader *)	reader	{	if(![NSThread isMainThread]) 	{	while(!self.initialized);	}	 return tabReader;	}

@end
