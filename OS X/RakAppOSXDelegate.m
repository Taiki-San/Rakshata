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

@implementation RakAppOSXDelegate

- (RakView *) earlyInit
{
	_hasFocus = YES;

	self.window.isMainWindow = YES;
	[self.window setDelegate:self];
	[self.window configure];
	[self.window registerForDrop];
	
	RakContentView * contentView = [self getContentView];
	if(contentView == nil)
		alertExit("Couldn't build view structure, basically, it's a _very_ early failure, we can't recover from that =/");
	
	[[NSUserNotificationCenter defaultUserNotificationCenter] setDelegate:self];
	
	return contentView;
}

- (void) mainInit : (RakContentView *) contentView
{
	Series * _tabSerie =	[Series alloc];
	CTSelec  * _tabCT =		[CTSelec alloc];
	Reader  * _tabReader =	[Reader alloc];
	MDL * _tabMDL =			[MDL alloc];
	
	[contentView setupCtx : tabSerie : tabCT : tabReader : tabMDL];
	self.window.defaultDispatcher = contentView;
	[self.window makeFirstResponder:contentView];
	
	tabSerie = [_tabSerie init:contentView : RakRealApp.savedContext[1]];
	tabCT = [_tabCT init:contentView : RakRealApp.savedContext[2]];
	tabMDL = [_tabMDL init:contentView : RakRealApp.savedContext[3]];
	tabReader = [_tabReader init:contentView : RakRealApp.savedContext[4]];
	
	//Update everything's size now that everything if up to date
	tabMDL.needUpdateMainViews = YES;
	[tabMDL resetFrameSize:NO];
	
	[self.window makeKeyAndOrderFront:self];
}

- (RakContentView*) getContentView
{
	if(_contentView != nil)
		return _contentView;
	
	for(id view in ((RakView *) [(id) self window].contentView).subviews)
	{
		if([view class] == [RakContentView class])
			return (_contentView = view);
	}
	
	return nil;
}

#pragma mark - Application Delegate

- (void) applicationWillTerminate:(NSNotification *)notification
{
	[self flushState];
}

- (BOOL) applicationShouldTerminateAfterLastWindowClosed:(NSApplication *)sender
{
	return YES;
}

- (BOOL)application:(NSApplication *)theApplication openFile:(NSString *)filename
{
	NSString * extension = [filename pathExtension];
	BOOL retValue = NO;
	
	//Source insertions are only supported on the per file basis
	if([extension caseInsensitiveCompare:SOURCE_FILE_EXT] == NSOrderedSame)
	{
		dispatch_async(dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_DEFAULT, 0), ^{
			[[[RakAddRepoController alloc] init] analyseFileContent:[NSData dataWithContentsOfFile:filename]];
		});
		return YES;
	}
	
	//Okay, file Import
	RakImportBaseController <RakImportIO> * IOController = createIOForFilename(filename);
	
	if(IOController != nil)
		[RakImportController importFile:@[IOController]];
	else
		NSLog(@"Couldn't open %@, either a permission issue or an invalid file :/", filename);
	
	return retValue;
}

- (void)application:(NSApplication *)sender openFiles:(NSArray<NSString *> *)filenames;
{
	NSMutableArray * collector = [NSMutableArray array];
	
	for(NSString * filename in filenames)
	{
		RakImportBaseController <RakImportIO> * IOController = createIOForFilename(filename);
		
		if(IOController != nil)
			[collector addObject:IOController];
		else
			NSLog(@"Couldn't open %@, either a permission issue or an invalid file :/", filename);
	}
	
	if([collector count])
		[RakImportController importFile:[NSArray arrayWithArray:collector]];
}

- (void) applicationWillBecomeActive:(nonnull NSNotification *)notification
{
	((RakContentViewBack*) self.window.contentView).isMainWindow = YES;
}

- (void) applicationWillResignActive:(nonnull NSNotification *)notification
{
	((RakContentViewBack*) self.window.contentView).isMainWindow = NO;
}

#pragma mark - Delegate work

- (void) windowWillEnterFullScreen:(nonnull NSNotification *)notification
{
	((RakContentViewBack*) self.window.contentView).heightOffset = -TITLE_BAR_HEIGHT;
}

- (void) windowWillExitFullScreen:(NSNotification *)notification
{
	[tabReader shouldLeaveDistractionFreeMode];
	((RakContentViewBack*) self.window.contentView).heightOffset = 0;
}

- (BOOL)userNotificationCenter:(NSUserNotificationCenter *)center shouldPresentNotification:(NSUserNotification *)notification
{
	return YES;
}

#pragma mark Focus management

- (void) applicationDidBecomeActive:(NSNotification *)notification
{
	_hasFocus = YES;
	[[NSUserNotificationCenter defaultUserNotificationCenter] removeAllDeliveredNotifications];
}

- (void) applicationDidResignActive:(NSNotification *)notification
{
	_hasFocus = NO;
}

#pragma mark - Menu interface

#pragma mark Entry point for about and preference windows

- (void)orderFrontStandardAboutPanel:(id)sender
{
	if(aboutWindow == nil)
		aboutWindow = [[RakAboutWindow alloc] init];
	
	[aboutWindow createWindow];
}

- (void)orderFrontStandardAboutPanelWithOptions:(NSDictionary *)optionsDictionary
{
	[self orderFrontStandardAboutPanel:self];
}

- (IBAction) openPreferenceWindow : (id) sender
{
	prefWindow = [[RakPrefsWindow alloc] init];
	[prefWindow createWindow];
}

#pragma mark Panneau

- (IBAction)jumpToSeries:(id)sender
{
	[tabSerie ownFocus];
}

- (IBAction)jumpToCT:(id)sender
{
	[tabCT ownFocus];
}

- (IBAction)jumpToReader:(id)sender
{
	[tabReader ownFocus];
}

- (IBAction)switchDistractionFree:(id)sender
{
	if(tabReader.mainThread == TAB_READER)
		[tabReader switchDistractionFree];
}

- (IBAction)close : (id)sender
{
	[[RakRealApp keyWindow] close];
}

#pragma mark Debug

- (IBAction) reloadFromRemote : (id)sender
{
	updateDatabase(YES);
}

@end
