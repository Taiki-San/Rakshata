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

enum
{
	IMAGE_BORDER = 50,
	TEXT_BORDER = 3,
	BUTTON_BORDER = 50
};

@interface RakPrefsRepoDetails()
{
	NSImage * repoImage;
	RakClickableText * URL, * group;
	RakText * data;
	
	RakDeleteButton * flushButton, * deleteButton;
	
	NSRect imageFrame;
	
	REPO_DATA * _repo;
	
	RakPrefsRepoView * __weak _responder;
}

@end

@implementation RakPrefsRepoDetails

- (instancetype) initWithRepo : (NSRect) frame : (BOOL) isRoot : (void *) repo : (RakPrefsRepoView *) responder
{
	self = [self initWithFrame:[self getFrame:frame]];
	
	if(self != nil)
	{
		_responder = responder;
		[self updateContent:isRoot :repo : NO];
	}
	
	return self;
}

- (NSRect) getFrame : (NSRect) parentFrame
{
	return NSMakeRect(PREFS_REPO_LIST_WIDTH, PREFS_REPO_BORDER_BELOW_LIST, parentFrame.size.width - PREFS_REPO_LIST_WIDTH, parentFrame.size.height - PREFS_REPO_BORDER_BELOW_LIST);
}

#pragma mark - Interface

- (void) updateContent : (BOOL) isRoot : (void *) repo : (BOOL) animated
{
	if(animated)
	{
		[NSAnimationContext runAnimationGroup:^(NSAnimationContext *context) {
			
			context.duration = 0.2f;
			self.animator.alphaValue = 0;
			
		} completionHandler:^{
			[self _updateContent:isRoot :repo];
			
			[NSAnimationContext runAnimationGroup:^(NSAnimationContext *context) {

				context.duration = 0.2f;
				self.animator.alphaValue = 1;
			
			} completionHandler:^{}];
		}];
	}
	else
		[self _updateContent:isRoot :repo];
}

- (void) _updateContent:(BOOL)isRoot :(void *)repo
{
	if(repo == NULL)
		return;
	
	_repo = repo;
	
	CGFloat baseY = _bounds.size.height;
	
	repoImage = loadImageForRepo(isRoot, repo);
	if(repoImage != nil)
	{
		imageFrame.size = repoImage.size;
		imageFrame.origin.x = _bounds.size.width / 2 - imageFrame.size.width / 2;
		imageFrame.origin.y = (baseY -= IMAGE_BORDER + imageFrame.size.height);
	}
	else
		imageFrame = NSZeroRect;
	
	if(!isRoot)
	{
		//URL field
		if(((REPO_DATA *) repo)->website)
		{
			if(URL == nil)
			{
				URL = [[RakClickableText alloc] initWithText:NSLocalizedString(@"WEBSITE", nil) :[self textColor] responder:self];
				if(URL != nil)
				{
					[URL sizeToFit];
					[self addSubview:URL];
				}
			}
			
			[URL setFrameOrigin:NSMakePoint(_bounds.size.width / 2 - URL.bounds.size.width / 2, (baseY -= TEXT_BORDER + URL.bounds.size.height))];
			URL.URL = [NSString stringWithUTF8String : ((REPO_DATA *) repo)->website];
		}
		else
		{
			[URL removeFromSuperview];
			URL = nil;
		}
		
		//Metadata on repo
		NSString * string = [NSString stringWithFormat:@"[%s]%s", ((REPO_DATA *) repo)->language, ((REPO_DATA *) repo)->isMature ? " [-18]" : ""];
		if(data == nil)
		{
			data = [[RakText alloc] initWithText:string :[self textColor]];
			[self addSubview:data];
		}
		else
			data.stringValue = string;
		
		if(flushButton == nil)
		{
			flushButton = [RakDeleteButton allocWithText:NSLocalizedString(@"PREFS-DELETE-CONTENT", nil) :NSZeroRect];
			if(flushButton != nil)
			{
				[flushButton.cell setBorderWidth:2];
				[flushButton.cell setCustomBackgroundColor:[Prefs getSystemColor:GET_COLOR_BACKGROUND_REPO_LIST :nil]];
				[flushButton sizeToFit];
				
				flushButton.target = self;
				flushButton.action = @selector(nukeTheDB);
				
				[self addSubview:flushButton];
			}
		}
		
		if(deleteButton == nil)
		{
			deleteButton = [RakDeleteButton allocWithText:NSLocalizedString(@"PREFS-DELETE-SOURCE", nil) :NSZeroRect];
			if(deleteButton != nil)
			{
				[deleteButton.cell setBorderWidth:2];
				[deleteButton.cell setCustomBackgroundColor:[Prefs getSystemColor:GET_COLOR_BACKGROUND_REPO_LIST :nil]];
				[deleteButton sizeToFit];

				deleteButton.target = self;
				deleteButton.action = @selector(nukeEverything);
				
				[self addSubview:deleteButton];
			}
		}
		
		//Resizing
		[data setFrameOrigin:NSMakePoint(_bounds.size.width / 2 - data.bounds.size.width / 2, (baseY -= TEXT_BORDER + data.bounds.size.height))];
		
		[flushButton setFrameOrigin:NSMakePoint(_bounds.size.width / 4 - flushButton.bounds.size.width / 2, BUTTON_BORDER)];
		[deleteButton setFrameOrigin:NSMakePoint(_bounds.size.width - _bounds.size.width / 4 - deleteButton.bounds.size.width / 2, BUTTON_BORDER)];
	}
	else
	{
		[URL removeFromSuperview];
		[data removeFromSuperview];
		
		URL = nil;
		data = nil;
	}
}

#pragma mark - Responder

- (void) respondTo : (RakClickableText *) sender
{
	NSString * string = sender.URL;
	
	if(string != nil)
		ouvrirSite([string UTF8String]);
}

- (void) nukeTheDB
{
	NSAlert * alert = [[NSAlert alloc] init];
	
	if(alert != nil)
	{
		alert.alertStyle = NSCriticalAlertStyle;
		alert.messageText = NSLocalizedString(@"PREFS-DELETE-CONTENT-TITLE", nil);
		alert.informativeText = NSLocalizedString(@"PREFS-DELETE-CONTENT-MESSAGE", nil);
		[alert addButtonWithTitle:NSLocalizedString(@"PREFS-DELETE-CANCEL", nil)];
		[alert addButtonWithTitle:NSLocalizedString(@"PREFS-DELETE-GO-FOR-IT", nil)];
		
		[alert beginSheetModalForWindow:self.window completionHandler:^(NSModalResponse returnCode) {
			if(returnCode != -NSModalResponseStop)
				[self deleteContent:NO];
		}];
	}
}

- (void) nukeEverything
{
	NSAlert * alert = [[NSAlert alloc] init];
	
	if(alert != nil)
	{
		alert.alertStyle = NSCriticalAlertStyle;
		alert.messageText = NSLocalizedString(@"PREFS-DELETE-SOURCE-TITLE", nil);
		alert.informativeText = NSLocalizedString(@"PREFS-DELETE-SOURCE-MESSAGE", nil);
		[alert addButtonWithTitle:NSLocalizedString(@"PREFS-DELETE-CANCEL", nil)];
		[alert addButtonWithTitle:NSLocalizedString(@"PREFS-DELETE-GO-FOR-IT", nil)];
		
		[alert beginSheetModalForWindow:self.window completionHandler:^(NSModalResponse returnCode) {
			if(returnCode != -NSModalResponseStop)
				[self deleteContent:YES];
		}];
	}
}

- (void) deleteContent : (BOOL) nukeRepo
{
	CTSelec * CT = [[NSApp delegate] CT];
	Reader * reader = [[NSApp delegate] reader];
	
	PROJECT_DATA readerProject = [reader activeProject], CTProject = [CT activeProject];
	uint64_t ID = getRepoID(_repo);
	
	if(getRepoID(CTProject.repo) == ID || getRepoID(readerProject.repo) == ID || 1)
	{
		self.window.title = NSLocalizedString(@"PREFS-DELETE-KILL-USE", nil);

		BOOL readerDeleted = NO, CTDeleted = NO;
		
		//We check which tab are using content we are about to delete
		if(getRepoID(readerProject.repo) == ID)
		{
			[reader resetReader];
			readerDeleted = YES;
		}

		if(getRepoID(CTProject.repo) == ID)
		{
			if(readerDeleted)
			{
				[CT resetTabContent];
				CTDeleted = YES;
			}
			else
				[CT updateProject:readerProject :reader.isTome :reader.currentElem];
		}
		
		//Update focus if required
		if(CTDeleted)
		{
			[RakTabView broadcastUpdateFocus:TAB_SERIES];
		}
		else if(readerDeleted)
		{
			uint mainThread;
			[Prefs getPref:PREFS_GET_MAIN_THREAD :&mainThread];
			
			if(mainThread == TAB_READER)
				[RakTabView broadcastUpdateFocus:TAB_CT];
		}
	}
	
	if(nukeRepo)
	{
		self.window.title = NSLocalizedString(@"PREFS-DELETE-REMOVE", nil);
		removeRepoFromCache(*_repo);
		deleteSubRepo(ID);
		syncCacheToDisk(SYNC_ALL);
	}
	
	self.window.title = NSLocalizedString(@"PREFS-DELETE-PURGE", nil);

	//Delete projects
	char path[256];
	snprintf(path, sizeof(path), PROJECT_ROOT"%s/", getPathForRepo(_repo));
	removeFolder(path);
	
	if(nukeRepo)
		[RakDBUpdate postNotificationFullUpdate];
	else
		setUninstalled(false, ID);	//Update DB, and notify everything
}

#pragma mark - Drawing

- (NSColor *) textColor
{
	return [Prefs getSystemColor:GET_COLOR_CLICKABLE_TEXT :nil];
}

- (void) drawRect:(NSRect)dirtyRect
{
	[repoImage drawInRect:imageFrame
			 fromRect:NSZeroRect
			operation:NSCompositeSourceOver
			 fraction:1.0];
}

@end