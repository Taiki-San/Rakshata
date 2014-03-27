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

- (id)init: (BOOL) displayed : (id) parent
{
	self = [self initView : parent : nil];
	
	[self.layer setCornerRadius:RADIUS_BORDERS];
	
	[self loadIcons : parent];
		
	if(!displayed)
		[self setHidden:![self isHidden]];
	
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
	
	favorite = [RakButton initForReader:self :@"icon" : 1 :[self getPosXButton:1] :YES :self :@selector(buttonHitten)];
	fullscreen = [RakButton initForReader:self :@"icon" : 2 :[self getPosXButton:2] :YES :superview :@selector(triggerFullscreen)];
	
	prevChapter = [RakButton initForReader:self :@"icon" : 3 :[self getPosXButton:3] :NO :superview :@selector(prevChapter)];
	prevPage = [RakButton initForReader:self :@"icon" : 1 :[self getPosXButton:4] :NO :superview :@selector(prevPage)];
	nextPage = [RakButton initForReader:self :@"icon" : 2 :[self getPosXButton:5] :YES :superview :@selector(nextPage)];
	nextChapter = [RakButton initForReader:self :@"icon" : 3 :[self getPosXButton:6] :YES :superview :@selector(nextChapter)];

	trash = [RakButton initForReader:self :@"icon": 1 :[self getPosXButton:7] :NO :self :@selector(buttonHitten)];
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
	NSPoint origin;
	
	for(char pos = 0; pos < nbElem; pos++)
	{
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
	[super drawRect:dirtyRect];
	
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
		if(frameRect.size.height == self.frame.size.height)
			return;
		
		frameRect.size.width = self.frame.size.width;
		frameRect.origin.x = self.frame.origin.x;
		frameRect.origin.y = self.frame.origin.y;
	}
	
	[super setFrame:frameRect];
	
	if(readerMode)
		[self recalculateButtonPosition];
}

- (void) setFrameOrigin:(NSPoint)newOrigin
{
	if(readerMode)
		[super setFrameOrigin:newOrigin];
}

/* Gestion des évènements*/
- (void)mouseDown:(NSEvent *)theEvent
{
	
}

- (void) readerIsOpening
{
	
}

- (void) applyRefreshSizeReaderChecks
{
	
}

/*Constraints routines*/
#pragma mark - Data about bar position

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
