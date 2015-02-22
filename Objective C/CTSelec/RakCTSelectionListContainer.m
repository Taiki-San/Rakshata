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
		
		_title = [[RakCTDragableTitle alloc] initWithText : self.bounds : [self titleString]];
		if(_title != nil)
		{
			_title.ignoreInternalFrameMagic = YES;
			_title.barWidth = 1;
			_title.drawGradient = YES;
			_title.isTome = _content.isTome;
			_title.currentID = _content.cacheID;
			_title.isEmpty = _content.isEmpty;
			
			[_title sizeToFit];
			[_title setFrame : [self frameForTitle : self.bounds]];
			
			[self addSubview:_title];
		}

		_placeholder = [[RakText alloc] initWithText: NSLocalizedString(_content.isTome ? @"CT-NO-VOLUME" : @"CT-NO-CHAPTER", nil) :[Prefs getSystemColor:GET_COLOR_ACTIVE :nil]];
		if(_placeholder != nil)
		{
#ifdef LARGE_FONT_FOR_PLACEHOLDERS
			[_placeholder setFont: [NSFont systemFontOfSize:15]];
#endif
			[_placeholder setAlignment:NSCenterTextAlignment];
			[_placeholder sizeToFit];
			[self addSubview:_placeholder];
		}
		
		_placeholderActive = content.nbElem == 0;
		
		[content getContent].alphaValue = !_placeholderActive;	[content setHidden : _placeholderActive];
		_placeholder.alphaValue = _placeholderActive;			[_placeholder setHidden : !_placeholderActive];
		
		[content setFrame:[self frameForContent : _bounds]];
		[_placeholder setFrameOrigin : NSCenteredRect(content.frame, _placeholder.bounds)];
		
		if(self.compactMode)
			[_title setHidden:YES];
	}
	
	return self;
}

- (NSString *) titleString
{
	return [NSString stringWithFormat:NSLocalizedString(_content.isTome ? @"VOLUME%c" : @"CHAPTER%c", nil), (_content.nbElem > 1 ? 's' : '\0')];
}

#pragma mark - Properties

- (void) setCompactMode : (BOOL) compactMode
{
	if(compactMode == _isCompact)
		return;
	
	_content.compactMode = compactMode;
	
	if(compactMode)
	{
		_isCompact = compactMode;
		[self setHidden:_wasHidden];
		
		self.layer.cornerRadius = 0;
		
		if(_title != nil)
			[_title setHidden:YES];
	}
	else
	{
		_wasHidden = self.isHidden;
		[self setHidden:NO];
		_isCompact = compactMode;
		
		self.layer.cornerRadius = 4.0f;

		if(_title != nil)
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
	
	else
		[super setHidden : hidden];
}

#pragma mark - Sizing

- (BOOL) isFlipped
{
	return YES;
}

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
		parentFrame.size.height -= 10 + 2;
		
		parentFrame.size.width /= 2;
		parentFrame.size.width -= 20;
		
		if(_content.isTome)
		{
			parentFrame.origin.x = parentFrame.size.width + 20;

			//If focus chapter, and MDL around, it's below the tome view
			MDL * tabMDL = [[(RakAppDelegate*) [NSApp delegate] MDL] getMDL : YES];	//Will validate if we can have it
			if(tabMDL != nil)
			{
				parentFrame.origin.y += [tabMDL lastFrame].size.height;
				parentFrame.size.height -= [tabMDL lastFrame].size.height;
			}
		}
		else
			parentFrame.origin.x = 10;
	}
	
	return parentFrame;
}

- (NSRect) frameForTitle : (NSRect) parentFrame
{
	parentFrame.origin = NSZeroPoint;
	
	if(_title != nil)
		parentFrame.size.height = _title.bounds.size.height;
	
	return parentFrame;
}

- (NSRect) frameForContent : (NSRect) parentFrame
{
	if(!self.compactMode)
	{
		parentFrame.origin.x = BORDERS_CONTENT;
		parentFrame.origin.y = _title.bounds.size.height + BORDERS_CONTENT;
		
		parentFrame.size.height -= parentFrame.origin.y + BORDERS_CONTENT;
		parentFrame.size.width -= 2 * BORDERS_CONTENT;
	}

	return parentFrame;
}

- (void) setFrame : (NSRect) parentFrame
{
	[super setFrame : [self frameFromParent:parentFrame]];

	[_title setFrame : [self frameForTitle : _bounds]];

	NSRect contentFrame = [self frameForContent : _bounds];
	[_content setFrame : contentFrame];
	[_placeholder setFrameOrigin : NSCenteredRect(contentFrame, _placeholder.bounds)];
}

- (void) resizeAnimation : (NSRect) parentFrame
{
	NSRect frame = [self frameFromParent:parentFrame];
	[self.animator setFrame : frame];
	frame.origin = NSZeroPoint;
	
	[_title resizeAnimation : [self frameForTitle : frame]];
	
	frame = [self frameForContent : frame];
	[_content resizeAnimation : frame];
	
	if(_placeholderActive)
		[_placeholder.animator setFrameOrigin : NSCenteredRect(frame, _placeholder.bounds)];
	else
		[_placeholder setFrameOrigin : NSCenteredRect(frame, _placeholder.bounds)];
}

#pragma mark - Color

- (CGColorRef) getBackgroundColor
{
	if(self.compactMode)
		return [NSColor clearColor].CGColor;
	
	return [Prefs getSystemColor : GET_COLOR_BACKGROUD_COREVIEW : nil].CGColor;
}

- (void) observeValueForKeyPath:(NSString *)keyPath ofObject:(id)object change:(NSDictionary *)change context:(void *)context
{
	if([object class] == [Prefs class])
		return;
	
	self.layer.backgroundColor = [self getBackgroundColor];
}

#pragma mark - Proxy

- (NSScrollView*) getContent	{	return [_content getContent];	}

- (uint) getSelectedElement	{	return [_content getSelectedElement];	}
- (float) getSliderPos	{	return [_content getSliderPos];		}
- (uint) nbElem	{	return _content.nbElem;	}

- (BOOL) reloadData : (PROJECT_DATA) project : (BOOL) resetScroller;
{
	BOOL retValue = [_content reloadData : project : resetScroller];
	
	if(_placeholderActive == retValue)
	{
		_placeholderActive = !_placeholderActive;
		
		[NSAnimationContext runAnimationGroup:^(NSAnimationContext *context) {
			
			[context setDuration:CT_TRANSITION_ANIMATION];
			
			if(retValue)
			{
				[_content setHidden:NO];
				[_content setAlphaValue:1 :YES];

				_placeholder.animator.alphaValue = 0;
			}
			else
			{
				[_content setAlphaValue:0 :YES];

				[_placeholder setHidden:NO];
				_placeholder.animator.alphaValue = 1;

				[_content flushContext:YES];
			}
			
		} completionHandler:^{
			
			if(retValue)
				[_placeholder setHidden : YES];
			else
				[_content setHidden : YES];

		}];
	}
	
	_title.currentID = project.cacheDBID;
	_title.isEmpty = !retValue;
	_title.stringValue = [self titleString];
	
	return retValue;
}

- (NSInteger) getIndexOfElement : (NSInteger) element	{	return [_content getIndexOfElement:element];	}
- (void) selectIndex : (uint) index	{	[_content selectIndex:index];	}
- (void) jumpScrollerToIndex : (uint) index
{
	[_content jumpScrollerToIndex:index];
}

- (void) resetSelection	{	[_content resetSelection:nil];	}

- (void) setWantIsTome : (BOOL) isTome
{
	BOOL hidden = _content.isTome != isTome;

	if(_isCompact)
	{
		self.hidden = hidden;
		self.alphaValue = !hidden;
	}
	else
		_wasHidden = hidden;
}

@end