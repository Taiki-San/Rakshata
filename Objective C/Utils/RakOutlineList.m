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

@implementation RakTreeView

- (instancetype) makeViewWithIdentifier:(NSString *)identifier owner:(id)owner
{
	id view = [super makeViewWithIdentifier:identifier owner:owner];
	
	if ([identifier isEqualToString:NSOutlineViewDisclosureButtonKey])
	{
		uint themeID = [Prefs getCurrentTheme:nil];
		[(NSButton *)view setImage:[RakResPath getImageFromTheme:@"TD->" :themeID]];
		[(NSButton *)view setAlternateImage:[RakResPath getImageFromTheme:@"TD-v" :themeID]];
	}
	
	return view;
}

- (NSRect) frame
{
	if(_defaultFrame.size.height != 0)
		return _defaultFrame;
	else
		return [super frame];
}

- (void) setFrameSize:(NSSize)newSize
{
	[super setFrameSize:newSize];
	
	if(_wantUpdateScrollview)
	{
		RakListScrollView * view = (id) self.superview;
		
		if(view == nil)
			return;
		
		view = (id) view.superview;
		if(view == nil || ![view isKindOfClass:[RakListScrollView class]])
			return;
		
		if(!view.hasVerticalScroller)
			[view updateScrollerState:view.frame];
	}
}

- (void) setFrame:(NSRect)frameRect
{
	_defaultFrame = frameRect;
	[super setFrame:_defaultFrame];
}

- (void) setDefaultFrame : (NSRect) frame
{
	_defaultFrame = frame;
}

@end

@implementation RakOutlineList

- (instancetype) init
{
	self = [super init];
	
	if (self != nil)
	{
		nbColumn = 1;
	}
	
	return self;
}

- (void) initializeMain : (NSRect) frame
{
	[Prefs getCurrentTheme:self];		//register
	
	content = [[RakTreeView alloc] initWithFrame:frame];
	
	if(content == nil)
		return;
	
	for (uint i = 0; i < nbColumn; i++)
	{
		NSTableColumn * column = [[NSTableColumn alloc] initWithIdentifier:@"The Solar Empire shall fall!"];
		if(column != nil)
		{
			[self setColumnWidth:column :i :frame.size.width];
			
			//Customisation
			[content setIndentationPerLevel:[content indentationPerLevel] / 2];
			[content setBackgroundColor:[NSColor clearColor]];
			[content setFocusRingType:NSFocusRingTypeNone];
			[content setAutoresizesOutlineColumn:NO];
			[content addTableColumn:column];
			
			if(i == 0)
			{
				firstColumn = column;
				[content setOutlineTableColumn:column];
			}
		}
	}
	
	//End of setup
	[content setDelegate:self];
	[content setDataSource:self];
}

- (void) setColumnWidth : (NSTableColumn *) _column : (uint) index : (CGFloat) fullWidth
{
	_column.width = _column.maxWidth = _column.minWidth = fullWidth;
}

- (void) reloadColumnsWidth : (CGFloat) fullWidth
{
	uint i = 0;
	
	for(NSTableColumn * current in content.tableColumns)
	{
		[self setColumnWidth:current :i++ :fullWidth];
	}
}

- (RakTreeView *) getContent
{
	return content;
}

- (void) dealloc
{
	[RakDBUpdate unRegister : self];
	[[NSNotificationCenter defaultCenter] removeObserver:self];
	[self moreFlushing];
}

- (void) moreFlushing
{
	
}

- (void) setFrame: (NSRect) frame
{
	[content setFrame:frame];
	[self reloadColumnsWidth:frame.size.width];
	
	[self additionalResizing : frame : YES];
	
	[content reloadData];
}

- (void) resizeAnimation : (NSRect) frame
{
	[content.animator setFrame:frame];
	
	[content setDefaultFrame:frame];
	[self reloadColumnsWidth:frame.size.width];
	
	frame.origin = NSZeroPoint;
	
	[self additionalResizing : frame : YES];
	
	[content reloadData];
}

- (void) setFrameOrigin : (NSPoint) newOrigin
{
	[content setFrameOrigin:newOrigin];
}

- (void) additionalResizing : (NSRect) frame : (BOOL) animated
{
	
}

#pragma mark - Color

- (NSColor *) getFontTopColor
{
	return [Prefs getSystemColor:COLOR_INACTIVE : nil];
}

- (NSColor *) getFontClickableColor
{
	return [Prefs getSystemColor:COLOR_CLICKABLE_TEXT : nil];
}

- (void) observeValueForKeyPath:(NSString *)keyPath ofObject:(id)object change:(NSDictionary *)change context:(void *)context
{
	if([object class] != [Prefs class])
		return [super observeValueForKeyPath:keyPath ofObject:object change:change context:context];
	
	[content reloadData];
}

#pragma mark - Context change

- (BOOL) isHidden
{
	return content.isHidden;
}

- (void) setHidden:(BOOL)hidden
{
	content.hidden = hidden;
}

#pragma mark - Data source to the view

- (uint) getNbRoot
{
	return _nbRoot;
}

- (NSInteger)outlineView:(NSOutlineView *)outlineView numberOfChildrenOfItem:(id)item
{
	if(item == nil)
		return [self getNbRoot];
	
	else if ([item isRootItem])
		return [item getNbChildren];
	
	return 0;
}

- (BOOL) outlineView:(NSOutlineView *)outlineView isItemExpandable:(id)item
{
	return item == nil ? YES : [item isRootItem];
}

- (id) outlineView:(NSOutlineView *)outlineView child:(NSInteger)index ofItem:(id)item
{
	return nil;
}

- (id) outlineView:(NSOutlineView *)outlineView objectValueForTableColumn:(NSTableColumn *)tableColumn byItem:(id)item
{
	id output = [item getData];
	
	return output != nil ? output : @"Invalid data :(";
}

#pragma mark - Delegate of the view

- (BOOL) outlineView:(NSOutlineView *)outlineView shouldExpandItem:(id)item
{
	((RakSerieListItem*) item).expanded = YES;
	return YES;
}

- (BOOL) outlineView:(NSOutlineView *)outlineView shouldCollapseItem:(id)item
{
	((RakSerieListItem*) item).expanded = NO;
	return YES;
}

- (BOOL)outlineView:(NSOutlineView *)outlineView isGroupItem:(id)item
{
	return item != nil && ![item isRootItem];
}

- (BOOL)outlineView:(NSOutlineView *)outlineView shouldEditTableColumn:(NSTableColumn *)tableColumn item:(id)item
{
	return NO;
}

- (BOOL)outlineView:(NSOutlineView *)outlineView shouldShowOutlineCellForItem:(id)item
{
	return [item isRootItem];
}

///			Manipulation we view added/removed

- (void)outlineView:(NSOutlineView *)outlineView didAddRowView:(NSTableRowView *)rowView forRow:(NSInteger)row
{
	
}

- (void)outlineView:(NSOutlineView *)outlineView didRemoveRowView:(NSTableRowView *)rowView forRow:(NSInteger)row
{
	
}

@end
