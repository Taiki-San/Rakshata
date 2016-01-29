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

@implementation RakTreeView

- (instancetype) init
{
	self = [super init];
	
	if(self != nil)
		self.manualEventDispatching = NO;
	
	return self;
}

- (instancetype) makeViewWithIdentifier:(NSString *)identifier owner:(id)owner
{
	id view = [super makeViewWithIdentifier:identifier owner:owner];
	
	if([identifier isEqualToString:NSOutlineViewDisclosureButtonKey])
	{
		[(NSButton *)view setImage:[RakResPath getImage:@"TD->"]];
		[(NSButton *)view setAlternateImage:[RakResPath getImage:@"TD-v"]];
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

#pragma mark - Buggy event handler management

- (void) setManualEventDispatching:(BOOL)manualEventDispatching
{
	tryingToProxy = !manualEventDispatching;
}

- (BOOL) manualEventDispatching
{
	return !tryingToProxy;
}

- (void) mouseDown:(NSEvent *)theEvent
{
	if(self.window.firstResponder == self && !tryingToProxy)
	{
		tryingToProxy = YES;
		[[self findSubviewAtCoordinate:[self convertPoint:[theEvent locationInWindow] fromView:nil]] mouseDown:theEvent];
		tryingToProxy = NO;
	}
	else
		[super mouseDown:theEvent];
}

- (void) mouseUp:(NSEvent *)theEvent
{
	if(self.window.firstResponder == self && !tryingToProxy)
	{
		tryingToProxy = YES;
		[[self findSubviewAtCoordinate:[self convertPoint:[theEvent locationInWindow] fromView:nil]] mouseUp:theEvent];
		tryingToProxy = NO;
	}
	else
		[super mouseUp:theEvent];
}

@end

@implementation RakOutlineList

- (instancetype) init
{
	self = [super init];
	
	if(self != nil)
	{
		nbColumn = 1;
	}
	
	return self;
}

- (void) initializeMain : (NSRect) frame
{
	[Prefs registerForChange:self forType:KVO_THEME];
	
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
			[content setBackgroundColor:[RakColor clearColor]];
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
	[Prefs deRegisterForChange:self forType:KVO_THEME];
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
	[content setFrameAnimated:frame];
	
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

- (RakColor *) getFontTopColor
{
	return [Prefs getSystemColor:COLOR_INACTIVE];
}

- (RakColor *) getFontClickableColor
{
	return [Prefs getSystemColor:COLOR_CLICKABLE_TEXT];
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
	
	else if([item isRootItem])
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
	
	return output != nil ? output : nil;//@"Invalid data :(";
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

#pragma mark - Menu

- (void) activateMenu
{
	//The menu will reuse our appearance
	CONFIGURE_APPEARANCE_DARK(content);
	
	content.menu = [[NSMenu alloc] init];
	content.menu.delegate = self;
}

- (void) menuNeedsUpdate : (NSMenu *) menu
{
	if (content.clickedRow > -1 && content.clickedColumn > -1)
	{
		[menu removeAllItems];
		[self configureMenu:menu forItem:[content itemAtRow:content.clickedRow] atColumn:content.clickedColumn];
	}
}

- (void) configureMenu : (NSMenu *) menu forItem : (id) item atColumn : (NSInteger) column
{
	
}

///			Manipulation we view added/removed

- (void)outlineView:(NSOutlineView *)outlineView didAddRowView:(NSTableRowView *)rowView forRow:(NSInteger)row
{
	
}

- (void)outlineView:(NSOutlineView *)outlineView didRemoveRowView:(NSTableRowView *)rowView forRow:(NSInteger)row
{
	
}

@end
