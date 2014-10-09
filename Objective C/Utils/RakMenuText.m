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

@implementation RakMenuText

#pragma mark - Drawing

- (void) additionalDrawing
{
	NSRect frame = self.bounds;
	
	frame.origin.y = frame.size.height - self.barWidth;
	frame.size.height = self.barWidth;
	
	[[self getBarColor] setFill];
	NSRectFill(frame);
}

#pragma mark - Color

- (NSColor *) getTextColor
{
	return [Prefs getSystemColor:GET_COLOR_INACTIVE : nil];
}

- (NSColor *) getBarColor
{
	return [Prefs getSystemColor:GET_COLOR_BORDERS_COREVIEWS : nil];
}

- (NSColor *) getBackgroundColor
{
	return [Prefs getSystemColor:GET_COLOR_BACKGROUND_TABS : nil];
}

- (CGFloat) getFontSize
{
	return 16;
}

- (NSFont *) getFont
{
	return [NSFont fontWithName:[Prefs getFontName:GET_FONT_TITLE] size:[self getFontSize]];
}

#pragma mark - barWidth

- (void) setBarWidth : (CGFloat) barWidth
{
	_barWidthInitialized = YES;
	_barWidth = barWidth;
}

- (CGFloat) barWidth
{
	if(!_barWidthInitialized)
	{
		_barWidthInitialized = YES;
		_barWidth = 2;
	}
	
	return _barWidth;
}

#pragma mark - Controller

- (CGFloat) getTextHeight
{
	return CT_READERMODE_WIDTH_PROJECT_NAME;
}

- (NSRect) getMenuFrame : (NSRect) parentFrame
{
	if(self.ignoreInternalFrameMagic)
		return parentFrame;
	
	NSRect frame = parentFrame;
	frame.size.height = [self getTextHeight];
	frame.origin.y = parentFrame.size.height - frame.size.height;
	
	return frame;
}

- (id) initWithText:(NSRect)frame :(NSString *)text
{
	self = [super initWithText:[self getMenuFrame:frame] :text : [self getTextColor]];
	
	if(self != nil)
	{
		[Prefs getCurrentTheme:self];	//Register for changes
		[self setFont:[self getFont]];
		[self defineBackgroundColor];
	}
	
	return self;
}

- (BOOL) isFlipped
{
	return YES;
}

- (void) defineBackgroundColor
{
	if([self getBackgroundColor] != nil)
	{
		[self setBackgroundColor:[self getBackgroundColor]];
		[self setDrawsBackground:YES];
	}
	else
	{
		[self setBackgroundColor:[NSColor clearColor]];
		[self setDrawsBackground:NO];
	}
}

- (void)observeValueForKeyPath:(NSString *)keyPath ofObject:(id)object change:(NSDictionary *)change context:(void *)context
{
	if ([object class] != [Prefs class])
		return;

	[self setTextColor:[self getTextColor]];
	[self defineBackgroundColor];
	[self setNeedsDisplay:YES];
}

- (void) setFrame:(NSRect)frameRect
{
	[super setFrame:[self getMenuFrame:frameRect]];
}

- (void) resizeAnimation : (NSRect) frameRect
{
	[self.animator setFrame: [self getMenuFrame:frameRect]];
}

- (void) dealloc
{
	[self removeFromSuperview];
}

@end
