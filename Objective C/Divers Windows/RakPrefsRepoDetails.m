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
	BUTTON_BORDER = 70
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
			flushButton = [RakDeleteButton allocWithText:@"Supprimer\nle contenu" :NSZeroRect];
			if(flushButton != nil)
			{
				[flushButton.cell setBorderWidth:2];
				[flushButton.cell setCustomBackgroundColor:[Prefs getSystemColor:GET_COLOR_BACKGROUND_REPO_LIST :nil]];
				[flushButton sizeToFit];
				[self addSubview:flushButton];
			}
		}
		
		if(deleteButton == nil)
		{
			deleteButton = [RakDeleteButton allocWithText:@"Désactiver\nla source" :NSZeroRect];
			if(deleteButton != nil)
			{
				[deleteButton.cell setBorderWidth:2];
				[deleteButton.cell setCustomBackgroundColor:[Prefs getSystemColor:GET_COLOR_BACKGROUND_REPO_LIST :nil]];
				[deleteButton sizeToFit];
				[self addSubview:deleteButton];
			}
		}
		
		//Resizing
		[data setFrameOrigin:NSMakePoint(_bounds.size.width / 2 - data.bounds.size.width / 2, (baseY -= TEXT_BORDER + data.bounds.size.height))];
		
		[flushButton setFrameOrigin:NSMakePoint(_bounds.size.width / 4 - flushButton.bounds.size.width / 2, BUTTON_BORDER - flushButton.bounds.size.height / 2)];
		[deleteButton setFrameOrigin:NSMakePoint(_bounds.size.width - _bounds.size.width / 4 - deleteButton.bounds.size.width / 2, BUTTON_BORDER - deleteButton.bounds.size.height / 2)];
	}
	else
	{
		[URL removeFromSuperview];
		[data removeFromSuperview];
		
		URL = nil;
		data = nil;
	}
}

- (void) respondTo : (RakClickableText *) sender
{
	NSString * string = sender.URL;
	
	if(string != nil)
		ouvrirSite([string UTF8String]);
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