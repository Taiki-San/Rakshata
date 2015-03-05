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
	LIST_ROW_HEIGHT = 45,
	LIST_ROW_IMAGE_DIAMETER = 40,
	
	OFFSET_IMAGE_ROW = 10,
	OFFSET_TITLE_X = 10,
	OFFSET_TITLE_Y = 22,
	OFFSET_DETAIL_Y = 6
};

@interface RakPrefsRepoListItem : NSView
{
	NSImage * image;
	RakText * title, * detail;
	
	NSRect imageFrame;
}

- (instancetype) initWithRepo : (BOOL) isRoot : (void *) repo : (NSString *) detailString;
- (void) updateContent : (BOOL) isRoot : (void *) repo : (NSString *) detailString;

@end

@implementation RakPrefsRepoList

- (instancetype) initWithFrame : (NSRect) frame
{
	self = [self init];
	
	if(self != nil)
	{
		_nbData = [_responder sizeForMode:_rootMode];
		
		[self applyContext:frame : selectedRowIndex : -1];
	}
	
	return self;
}

- (BOOL) didInitWentWell
{
	return YES;
}

#pragma mark - Element generation

- (NSInteger) numberOfRowsInTableView : (RakTableView *) tableView
{
	return _nbData;
}

- (CGFloat)tableView:(NSTableView *)tableView heightOfRow:(NSInteger)row
{
	return LIST_ROW_HEIGHT;
}

- (NSView*) tableView : (RakTableView *) tableView viewForTableColumn : (NSTableColumn*) tableColumn row : (NSInteger) row
{
	void ** list = [_responder listForMode:_rootMode];
	if(list == NULL || row >= [_responder sizeForMode:_rootMode])
		return nil;
	
	NSString * detail = nil;
	if(!_rootMode && list[row] != NULL)
		detail = [_responder nameOfParent: ((REPO_DATA *) list[row])->parentRepoID];

	RakPrefsRepoListItem *result = [tableView makeViewWithIdentifier : _identifier owner:self];
	if (result == nil)
		result = [[RakPrefsRepoListItem alloc] initWithRepo :_rootMode : list[row] : detail];
	else
		[result updateContent:_rootMode :list[row] :detail];
	
	return result;
}

@end

@implementation RakPrefsRepoListItem

- (instancetype) initWithRepo : (BOOL) isRoot : (void *) repo : (NSString *) detailString
{
	self = [self initWithFrame:NSMakeRect(0, 0, 300, LIST_ROW_HEIGHT)];
	
	if(self != nil)
		[self updateContent:isRoot :repo :detailString];
	
	return self;
}

- (void) updateContent : (BOOL) isRoot : (void *) repo : (NSString *) detailString
{
	//Image
	NSBundle * bundle = [NSBundle bundleWithPath: @"imageCache/"];
	if(bundle != nil)
		image = [bundle imageForResource:[NSString stringWithUTF8String:repo]];
	
	if(image == nil)
		image = [NSImage imageNamed: isRoot ? @"defaultRepoRoot" : @"defaultRepo"];
	
	//Title
	NSString * string = getStringForWchar(isRoot ? ((ROOT_REPO_DATA *) repo)->name : ((REPO_DATA *) repo)->name);
	
	if(title == nil)
	{
		title = [[RakText alloc] initWithText:string :[self textColor]];
		if(title != nil)
		{
			title.font = [NSFont fontWithName:[Prefs getFontName:GET_FONT_PREFS_TITLE] size:15];
			[title sizeToFit];
			
			[self addSubview:title];
		}
	}
	else
	{
		title.stringValue = string;
		[title sizeToFit];
	}
	
	//Detail
	if(isRoot)
	{
		if(((ROOT_REPO_DATA *) repo)->nombreSubrepo == 0)
			string = NSLocalizedString(@"PREFS-ROOT-NO-ACTIVE-REPO", nil);
		else if(((ROOT_REPO_DATA *) repo)->nombreSubrepo == 1)
			string = NSLocalizedString(@"PREFS-ROOT-ONE-ACTIVE-REPO", nil);
		else
			string = [NSString localizedStringWithFormat:NSLocalizedString(@"PREFS-ROOT-%zu-ACTIVE-REPO", nil), ((ROOT_REPO_DATA *) repo)->nombreSubrepo];
	}
	else
	{
		if(detailString == nil)
			string = NSLocalizedString(@"PREFS-ROOT-NO-GROUP", nil);
		else
			string = [NSString localizedStringWithFormat:NSLocalizedString(@"PREFS-ROOT-GROUPS-%@", nil), detailString];
	}
	
	if(detail == nil)
	{
		detail = [[RakText alloc] initWithText:string :[self detailTextColor]];
		
		if(detail != nil)
		{
			detail.font = [NSFont fontWithName:[Prefs getFontName:GET_FONT_STANDARD] size:12];
			[detail sizeToFit];
			
			[self addSubview:detail];
		}
	}
	else
		detail.stringValue = string;
}

- (void) setFrame:(NSRect)frameRect
{
	[super setFrame:frameRect];
	
	imageFrame = NSMakeRect(OFFSET_IMAGE_ROW, frameRect.size.height / 2 - LIST_ROW_IMAGE_DIAMETER / 2, LIST_ROW_IMAGE_DIAMETER, LIST_ROW_IMAGE_DIAMETER);
	
	[title setFrameOrigin:NSMakePoint(NSMaxX(imageFrame) + OFFSET_TITLE_X, OFFSET_TITLE_Y)];
	[detail setFrameOrigin:NSMakePoint(NSMaxX(imageFrame) + OFFSET_TITLE_X, OFFSET_DETAIL_Y)];
}

- (void) drawRect:(NSRect)dirtyRect
{
	[[self backgroundColor] setFill];
	NSRectFill(dirtyRect);

	[NSGraphicsContext saveGraphicsState];
	
	NSBezierPath *path = [NSBezierPath bezierPathWithRoundedRect:imageFrame
														 xRadius:LIST_ROW_IMAGE_DIAMETER / 2
														 yRadius:LIST_ROW_IMAGE_DIAMETER / 2];
	[path addClip];
	
	[image drawInRect:imageFrame
			 fromRect:NSZeroRect
			operation:NSCompositeSourceOver
			 fraction:1.0];
	
	[NSGraphicsContext restoreGraphicsState];
}

- (NSColor *) textColor
{
	return [Prefs getSystemColor:GET_COLOR_ACTIVE :nil];
}

- (NSColor *) detailTextColor
{
	return [Prefs getSystemColor:GET_COLOR_SURVOL :nil];
}

- (NSColor *) backgroundColor
{
	return [Prefs getSystemColor:GET_COLOR_BACKGROUND_TABS :nil];
}

@end