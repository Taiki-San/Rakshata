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

#define BUTTON_NAME @"clearTag"

@implementation RakSRTagItem

- (instancetype) initWithTag : (NSString *) tagName
{
	self = [self init];
	
	if(self != nil)
	{
		[Prefs getCurrentTheme:self];
		
		if(![self updateIcon])
			return nil;
		
		label = [[RakText alloc] initWithText:NSMakeRect(0, 0, 25, TAG_BUTTON_HEIGHT) :tagName :[self getTextColor]];
		if(label != nil)
		{
			[label sizeToFit];
			[label setFrameOrigin:NSMakePoint(NSMaxX(close.frame) + TAG_BORDER_LEFT_LABEL, TAG_BUTTON_MIDDLE + 1 -label.bounds.size.height / 2)];
			[self addSubview:label];
		}
		else
			return nil;
		
		[self setFrameSize:NSMakeSize(NSMaxX(label.frame) + TAG_BORDER_RIGHT_LABEL, TAG_BUTTON_HEIGHT)];
	}
	
	return self;
}

- (BOOL) updateIcon
{
	if(close != nil)
		[close removeFromSuperview];
	
	close = [RakButton allocImageWithoutBackground:BUTTON_NAME :RB_STATE_STANDARD :self :@selector(close)];
	if(close != nil)
	{
		[close.cell setHighlightAllowed:NO];
		[close setFrameOrigin:NSMakePoint(5, TAG_BUTTON_MIDDLE - close.bounds.size.height / 2)];
		[self addSubview:close];
		return YES;
	}
	
	return NO;
}

- (void) updateContent : (NSString *) newTagName
{
	label.stringValue = newTagName;
	[label sizeToFit];

	[self setFrameSize:NSMakeSize(NSMaxX(label.frame) + TAG_BORDER_RIGHT_LABEL, TAG_BUTTON_HEIGHT)];
}

- (NSColor *) getTextColor
{
	return [Prefs getSystemColor:GET_COLOR_TAGITEM_FONT :nil];
}

- (NSColor *) backgroundColor
{
	return [Prefs getSystemColor:GET_COLOR_TAGITEM_BACKGROUND :nil];
}

- (NSColor *) borderColor
{
	return [Prefs getSystemColor:GET_COLOR_READER_BAR_FRONT : nil];
}

#pragma mark - UI Drawing

- (void) drawRect : (NSRect) dirtyRect
{
	NSSize size = self.bounds.size;
	
	CGContextRef contextBorder = [[NSGraphicsContext currentContext] graphicsPort];
	
	//Background
	CGContextBeginPath(contextBorder);
	CGContextMoveToPoint(contextBorder, 0, TAG_BUTTON_MIDDLE);
	CGContextAddLineToPoint(contextBorder, TAG_BUTTON_ARROW_DEPTH, size.height);
	CGContextAddLineToPoint(contextBorder, size.width, size.height);
	CGContextAddLineToPoint(contextBorder, size.width, 0);
	CGContextAddLineToPoint(contextBorder, TAG_BUTTON_ARROW_DEPTH, 0);
	CGContextAddLineToPoint(contextBorder, 0, TAG_BUTTON_MIDDLE);

	[[self backgroundColor] setFill];
	CGContextFillPath(contextBorder);

	//Front
	CGContextBeginPath(contextBorder);
	CGContextMoveToPoint(contextBorder, TAG_BUTTON_ARROW_DEPTH - 1, 0);
	CGContextAddLineToPoint(contextBorder, size.width, 0);
	CGContextAddLineToPoint(contextBorder, size.width, size.height);
	
	[[self borderColor] setStroke];
	CGContextSetLineWidth(contextBorder, 2);
	CGContextStrokePath(contextBorder);
}

- (void) close
{
	if(_parent != nil)
	{
		[_parent removeTag : self.index];
		_parent = nil;
	}
}

- (void)observeValueForKeyPath:(NSString *)keyPath ofObject:(id)object change:(NSDictionary *)change context:(void *)context
{
	if ([object class] != [Prefs class])
		return;
	
	[self updateIcon];
	[label setTextColor:[self getTextColor]];
}

@end
