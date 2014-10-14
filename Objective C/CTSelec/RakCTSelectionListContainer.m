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
 ********************************************************************************************/

#define BORDERS_CONTENT 5

@implementation RakCTSelectionListContainer

- (instancetype) initWithFrame : (NSRect) parentFrame : (BOOL) isCompact : (RakCTSelectionList*) content
{
	if(content == nil)
		return nil;
	
	_isCompact = isCompact;
	_content = content;
	
	self = [self initWithFrame : [self frameFromParent:parentFrame]];
	
	if(self != nil)
	{
		self.wantsLayer = YES;
		self.layer.backgroundColor = [self getBackgroundColor];
		self.layer.cornerRadius = isCompact ? 0 : 4.0;
		
		[Prefs getCurrentTheme:self];
		[content setSuperview:self];
		
		NSString * string = [NSString stringWithFormat : @"%s%c", (content.isTome ? "Tome" : "Chapitre"), (content.nbElem > 1 ? 's' : '\0')];
		_title = [[RakMenuText alloc] initWithText : self.bounds : string];
		if(_title != nil)
		{
			_title.ignoreInternalFrameMagic = YES;
			_title.barWidth = 1;
			[_title sizeToFit];
			
			[_title setFrame : [self frameForTitle : self.bounds]];
			
			_title.drawGradient = YES;
			
			[self addSubview:_title];
		}

		[content setFrame:[self frameForContent : self.bounds]];
	
		if(self.compactMode)
			[_title setHidden:YES];
	}
	
	return self;
}

#pragma mark - Properties

- (void) setCompactMode : (BOOL) compactMode
{
	if(compactMode)
	{
		_isCompact = compactMode;
		[self setHidden:_wasHidden];
		
		self.layer.cornerRadius = 0;
		
		if(_title != nil && !_title.isHidden)
			[_title setHidden:YES];
	}
	else
	{
		_wasHidden = self.isHidden;
		[self setHidden:NO];
		_isCompact = compactMode;
		
		self.layer.cornerRadius = 4.0f;

		if(_title != nil && _title.isHidden)
			[_title setHidden:NO];
	}
	
	self.layer.backgroundColor = [self getBackgroundColor];
}

- (BOOL) compactMode
{
	return _isCompact;
}

- (void) setHidden : (BOOL) hidden
{
	if(!_isCompact)
		_wasHidden = hidden;
	
	else if(self.isHidden != hidden)
		[super setHidden : hidden];
}

#pragma mark - Sizing

- (NSRect) frameFromParent : (NSRect) parentFrame
{
	if(self.compactMode)
	{
		parentFrame.origin.x = CT_READERMODE_BORDER_TABLEVIEW;
		parentFrame.origin.y = 0;
		parentFrame.size.width -= 2 * CT_READERMODE_BORDER_TABLEVIEW;
		parentFrame.size.height -= CT_READERMODE_HEIGHT_CT_BUTTON + CT_READERMODE_HEIGHT_BORDER_TABLEVIEW;
	}
	else
	{
		parentFrame.origin.y = 10;
		parentFrame.size.height -= 20 + 8;
		
		parentFrame.size.width /= 2;
		parentFrame.size.width -= 20;
		
		if(_content.isTome)
			parentFrame.origin.x = parentFrame.size.width + 20;
		else
			parentFrame.origin.x = 10;
	}
	
	return parentFrame;
}

- (NSRect) frameForTitle : (NSRect) parentFrame
{
	parentFrame.origin.y = parentFrame.size.height;
	
	if(_title != nil)
		parentFrame.size.height = _title.bounds.size.height;

	parentFrame.origin.y -= parentFrame.size.height;
	
	return parentFrame;
}

- (NSRect) frameForContent : (NSRect) parentFrame
{
	if(!self.compactMode)
	{
		parentFrame.origin.x = BORDERS_CONTENT;
		parentFrame.origin.y = BORDERS_CONTENT;
		
		parentFrame.size.height -= _title.bounds.size.height + 2 * BORDERS_CONTENT;
		parentFrame.size.width -= 2 * BORDERS_CONTENT;
	}

	return parentFrame;
}

- (void) setFrame : (NSRect) parentFrame
{
	[super setFrame : [self frameFromParent:parentFrame]];
	
	[_title setFrame : [self frameForTitle : self.bounds]];
	
	[_content setFrame : [self frameForContent : self.bounds]];
}

- (void) resizeAnimation : (NSRect) parentFrame
{
	NSRect frame = [self frameFromParent:parentFrame];
	[self.animator setFrame : frame];
	
	frame.origin = NSZeroPoint;
	
	[_title resizeAnimation : [self frameForTitle : frame]];
	[_content resizeAnimation : [self frameForContent : frame]];
}

#pragma mark - Color

- (CGColorRef) getBackgroundColor
{
	if(self.compactMode)
		return [NSColor clearColor].CGColor;
	
	return [Prefs getSystemColor : GET_COLOR_BACKGROUD_CT_READERMODE : nil].CGColor;
}

- (void) observeValueForKeyPath:(NSString *)keyPath ofObject:(id)object change:(NSDictionary *)change context:(void *)context
{
	if([object class] == [Prefs class])
		return;
	
	self.layer.backgroundColor = [self getBackgroundColor];
}

#pragma mark - Proxy

- (NSScrollView*) getContent	{	return [_content getContent];	}

- (NSInteger) getSelectedElement	{	return [_content getSelectedElement];	}
- (float) getSliderPos	{	return [_content getSliderPos];		}

- (BOOL) reloadData : (PROJECT_DATA) project : (int) nbElem : (void *) newData : (BOOL) resetScroller;
{
	return [_content reloadData : project : nbElem : newData : resetScroller];
}

- (NSInteger) getIndexOfElement : (NSInteger) element	{	return [_content getIndexOfElement:element];	}
- (void) selectRow : (int) row	{	[_content selectRow:row];	}
- (void) jumpScrollerToRow : (int) row	{	[_content jumpScrollerToRow:row];	}

- (void) resetSelection : (NSTableView *) tableView	{	[_content resetSelection:tableView];	}

@end