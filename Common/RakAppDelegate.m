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

#if !TARGET_OS_IPHONE
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
#endif
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

- (RakContentView*) getContentView
{
	return _contentView;
}

- (Series *)	serie	{	if(![NSThread isMainThread]) 	{	while(!self.initialized);	}	 return [self _serie];	}
- (CTSelec *)	CT		{	if(![NSThread isMainThread]) 	{	while(!self.initialized);	}	 return [self _CT];		}
- (MDL *)		MDL		{	if(![NSThread isMainThread]) 	{	while(!self.initialized);	}	 return [self _MDL];	}
- (Reader *)	reader	{	if(![NSThread isMainThread]) 	{	while(!self.initialized);	}	 return [self _reader];	}

- (Series *)	_serie	{	 return tabSerie;	}
- (CTSelec *)	_CT		{	 return tabCT;		}
- (MDL *)		_MDL	{	 return tabMDL;		}
- (Reader *)	_reader	{	 return tabReader;	}

#pragma mark - Flush State

- (void) flushState
{
#ifdef FLUSH_PREFS_PROPERLY
	@autoreleasepool
	{
#endif
		NSString *saveSerie, *saveCT, *saveReader, *saveMDL;
		
#if TARGET_OS_IPHONE
		saveSerie = [tabSerie byebye];		tabSerie = nil;
		saveCT =	[tabCT byebye];			tabCT = nil;
		saveReader =[tabReader byebye];		tabReader = nil;
		saveMDL =	[tabMDL byebye];		tabMDL = nil;
#else
		saveSerie = [tabSerie byebye];		[tabSerie removeFromSuperview];				tabSerie = nil;
		saveCT =	[tabCT byebye];			[tabCT removeFromSuperview];				tabCT = nil;
		saveReader =[tabReader byebye];		[tabReader removeFromSuperview];			tabReader = nil;
		saveMDL =	[tabMDL byebye];		[tabMDL removeFromSuperview];				tabMDL = nil;
#endif
		
		[RakContextRestoration saveContextPrefs:[Prefs dumpPrefs]
										 series:saveSerie
											 CT:saveCT
										 reader:saveReader
											MDL:saveMDL];
		
		[[self getContentView] cleanCtx];
		
#if !TARGET_OS_IPHONE
		[(RakAppOSXDelegate *) self window].contentView = nil;
		[(RakAppOSXDelegate *) self window].imatureFirstResponder = nil;
		[(RakAppOSXDelegate *) self window].defaultDispatcher = nil;
#endif
#ifdef FLUSH_PREFS_PROPERLY
	}
	
	[Prefs deletePrefs];
#endif
}

@end
