/*********************************************************************************************
 **	__________         __           .__            __                 ________   _______   	**
 **	\______   \_____  |  | __  _____|  |__ _____ _/  |______   	___  _\_____  \  \   _  \  	**
 **	 |       _/\__  \ |  |/ / /  ___/  |  \\__  \\   __\__  \  	\  \/ //  ____/  /  /_\  \ 	**
 **	 |    |   \ / __ \|    <  \___ \|   Y  \/ __ \|  |  / __ \__ \   //       \  \  \_/   \	**
 **	 |____|_  /(____  /__|_ \/____  >___|  (____  /__| (____  /	  \_/ \_______ \ /\_____  /	**
 **	        \/      \/     \/     \/     \/     \/          \/ 	              \/ \/     \/ 	**
 **                                                                                         **
 **    Licence propriétaire, code source confidentiel, distribution formellement interdite  **
 **                                                                                         **
 ********************************************************************************************/

@implementation RakReaderBottomBar

#define RADIUS_BORDERS 13.0

- (id)init: (BOOL) displayed : (Reader*) parent
{
	self = [super initWithFrame:[self createFrameWithSuperView:parent.frame]];
	
	if(self != nil)
	{
		[self setAutoresizesSubviews:true];
		[parent addSubview:self];
		
		readerMode = parent->readerMode;

		[self setWantsLayer:YES];
		[self.layer setCornerRadius:RADIUS_BORDERS];
		
		[self loadIcons : parent];
		
		if(!displayed)
			[self setHidden:![self isHidden]];
	}

	return self;
}

- (void) setupPath
{
	NSSize selfSize = self.frame.size;
	
	contextBorder = [[NSGraphicsContext currentContext] graphicsPort];
	
	CGContextBeginPath(contextBorder);
	CGContextAddArc(contextBorder, RADIUS_BORDERS, selfSize.height / 2, RADIUS_BORDERS, -5 * M_PI_2 / 3, M_PI_2, 1);
	CGContextAddLineToPoint(contextBorder, selfSize.width - RADIUS_BORDERS, selfSize.height);
	CGContextAddArc(contextBorder, selfSize.width - RADIUS_BORDERS, selfSize.height/2, RADIUS_BORDERS, M_PI_2, M_PI_2 / 3, 1);
}

- (void) leaveReaderMode
{
	readerMode = false;
}

- (void) startReaderMode
{
	readerMode = true;
}

#pragma mark - Buttons

- (void) buttonHitten
{
	NSLog(@"Luna shall be our queen");
}

- (short) numberIconsInBar
{
	return 7;
}

- (void) loadIcons : (Reader*) superView
{
	NSView * superview = self.superview;
	
	favorite = [RakButton initForReader:self :@"fav" : RB_STATE_STANDARD :[self getPosXButton:1] :YES :self :@selector(buttonHitten)];
	fullscreen = [RakButton initForReader:self :@"Fullscreen" : RB_STATE_STANDARD :[self getPosXButton:2] :YES :superview :@selector(triggerFullscreen)];
	
	prevChapter = [RakButton initForReader:self :@"first" : RB_STATE_STANDARD :[self getPosXButton:3] :NO :superview :@selector(prevChapter)];
	prevPage = [RakButton initForReader:self :@"before" : RB_STATE_STANDARD :[self getPosXButton:4] :NO :superview :@selector(prevPage)];
	nextPage = [RakButton initForReader:self :@"next" : RB_STATE_STANDARD :[self getPosXButton:5] :YES :superview :@selector(nextPage)];
	nextChapter = [RakButton initForReader:self :@"last" : RB_STATE_STANDARD :[self getPosXButton:6] :YES :superview :@selector(nextChapter)];

	trash = [RakButton initForReader:self :@"X": RB_STATE_STANDARD :[self getPosXButton:7] :NO :self :@selector(buttonHitten)];
}

- (CGFloat) getPosXButton : (uint) IDButton
{
	CGFloat output = 0;
	
	switch (IDButton)
	{
		case 1:			//favorite
		{
			output = 20;
			break;
		}
		case 2:			//fullscreen
		{
			output = 60;
			break;
		}
			
		case 3:			//previous chapter
		{
			output = self.frame.size.width / 2 - 40;
			
			if(prevChapter != nil)
				output -= prevChapter.frame.size.width;
			
			break;
		}
			
		case 4:			//previous page
		{
			output = self.frame.size.width / 2 - 5;
			
			if(prevPage != nil)
				output -= prevPage.frame.size.width;
			
			break;
		}
			
		case 5:			//next page
		{
			output = self.frame.size.width / 2 + 5;
			break;
		}
			
		case 6:			//next page
		{
			output = self.frame.size.width / 2 + 40;
			break;
		}
			
		case 7:			//trash
		{
			output = self.frame.size.width - 25;
			
			if(trash != nil)
				output -= trash.frame.size.width;
			
			break;
		}
	}
	
	return output;
}

- (void) recalculateButtonPosition
{
	RakButton* icons[] = {favorite, fullscreen, prevChapter, prevPage, nextPage, nextChapter, trash};
	short nbElem = [self numberIconsInBar];
	CGFloat midleHeightBar = self.frame.size.height / 2, lastElemHeight = 0;
	NSPoint origin = {0, 0};
	
	for(char pos = 0; pos < nbElem; pos++)
	{
		if (icons[pos] == nil)
			continue;
		
		origin.x = [self getPosXButton:pos+1];
		
		if(icons[pos].frame.size.height != lastElemHeight)
		{
			origin.y = midleHeightBar - icons[pos].frame.size.height / 2;
			lastElemHeight = icons[pos].frame.size.height;
		}
		
		[icons[pos] setFrameOrigin:origin];
	}
}

#pragma mark - Color stuffs

- (NSColor*) getMainColor
{
	return [Prefs getSystemColor:GET_COLOR_READER_BAR];
}

- (NSColor*) getColorFront
{
	return [Prefs getSystemColor:GET_COLOR_READER_BAR_FRONT];
}

- (void)drawRect:(NSRect)dirtyRect
{
	[[self getMainColor] setFill];
	NSRectFill(dirtyRect);
	
	[self setupPath];
	[[self getColorFront] setStroke];
	CGContextStrokePath(contextBorder);
}

/*	Routines à overwrite	*/
#pragma mark - Routine to overwrite

- (void) setFrame:(NSRect)frameRect
{
	if(!readerMode)
	{
		frameRect.size.height = [self getRequestedViewHeight:frameRect.size.height];
		
		if(frameRect.size.height == self.frame.size.height)
			return;
		
		frameRect.size.width = self.frame.size.width;
		frameRect.origin.x = self.frame.origin.x;
		frameRect.origin.y = self.frame.origin.y;
	}
	else
		frameRect = [self createFrameWithSuperView:frameRect];
	
	[super setFrame:frameRect];
	
	if(readerMode)
		[self recalculateButtonPosition];
}

- (void) setFrameOrigin:(NSPoint)newOrigin
{
	if(readerMode)
		[super setFrameOrigin:newOrigin];
}


/*Constraints routines*/
#pragma mark - Data about bar position

- (NSRect) createFrameWithSuperView : (NSRect) superviewRect
{
	NSSize size = superviewRect.size;
	return NSMakeRect([self getRequestedViewPosX:size.width], [self getRequestedViewPosY:size.height], [self getRequestedViewWidth:size.width], [self getRequestedViewHeight:size.height]);
}

- (CGFloat) getRequestedViewPosX:(CGFloat) widthWindow
{
	return widthWindow / 2 - [self getRequestedViewWidth : widthWindow] / 2;
}

- (CGFloat) getRequestedViewPosY:(CGFloat) heightWindow
{
	return RD_CONTROLBAR_POSY;
}

- (CGFloat) getRequestedViewWidth:(CGFloat) widthWindow
{
	CGFloat output = widthWindow * RD_CONTROLBAR_WIDHT_PERC / 100;
	
	output = MIN(output, RD_CONTROLBAR_WIDHT_MAX);
	output = MAX(output, RD_CONTROLBAR_WIDHT_MIN);
	
	return output;
}

- (CGFloat) getRequestedViewHeight:(CGFloat) heightWindow
{
	return RD_CONTROLBAR_HEIGHT;
}

@end
