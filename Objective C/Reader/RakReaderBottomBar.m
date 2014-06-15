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

@implementation RakReaderBottomBar

#define RADIUS_BORDERS 13.0

- (id)init: (BOOL) displayed : (Reader*) parent
{
	self = [super initWithFrame:[self createFrameWithSuperView:parent.frame]];
	
	if(self != nil)
	{
		[self setAutoresizesSubviews:NO];
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

- (void) dealloc
{
	RakButton* icons[] = {favorite, fullscreen, prevChapter, prevPage, nextPage, nextChapter, trash};
	
	for(int i = 0; i < 7; i++)
	{
		if(icons[i] != nil)
		{
			[icons[i] removeFromSuperview];
			[icons[i] release];
		}
	}

	if(pageCount != nil)
	{
		[pageCount removeFromSuperview];
		[pageCount release];
	}
	
	[super dealloc];
}

#pragma mark - Update page counter

- (void) updatePage : (uint) newCurrentPage : (uint) newPageMax
{
	if(pageCount == nil)
	{
		pageCount = [[RakPageCounter alloc] init: self : [self getPosXElement : 8 : self.frame.size.width] :newCurrentPage :newPageMax : (Reader*) self.superview];
		[self addSubview:pageCount];
	}
	else
	{
		[pageCount updatePage:newCurrentPage :newPageMax];
	}
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
	
	favorite = [RakButton allocForReader:self :@"fav" : RB_STATE_STANDARD :[self getPosXElement : 1 : self.frame.size.width] :YES :self :@selector(buttonHitten)];
	fullscreen = [RakButton allocForReader:self :@"fullscreen" : RB_STATE_STANDARD :[self getPosXElement : 2 : self.frame.size.width] :YES :superview :@selector(triggerFullscreen)];
	
	prevChapter = [RakButton allocForReader:self :@"first" : RB_STATE_STANDARD :[self getPosXElement : 3 : self.frame.size.width] :NO :superview :@selector(prevChapter)];
	prevPage = [RakButton allocForReader:self :@"before" : RB_STATE_STANDARD :[self getPosXElement : 4 : self.frame.size.width] :NO :superview :@selector(prevPage)];
	nextPage = [RakButton allocForReader:self :@"next" : RB_STATE_STANDARD :[self getPosXElement : 5 : self.frame.size.width] :YES :superview :@selector(nextPage)];
	nextChapter = [RakButton allocForReader:self :@"last" : RB_STATE_STANDARD :[self getPosXElement : 6 : self.frame.size.width] :YES :superview :@selector(nextChapter)];

	trash = [RakButton allocForReader:self :@"trash": RB_STATE_STANDARD :[self getPosXElement : 7 : self.frame.size.width] :NO :superView :@selector(deleteElement)];
}

- (CGFloat) getPosXElement : (uint) IDButton : (CGFloat) width
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
			output = width / 2 - 40;
			
			if(prevChapter != nil)
				output -= prevChapter.frame.size.width;
			
			break;
		}
			
		case 4:			//previous page
		{
			output = width / 2 - 10;
			
			if(prevPage != nil)
				output -= prevPage.frame.size.width;
			
			break;
		}
			
		case 5:			//next page
		{
			output = width / 2 + 10;
			break;
		}
			
		case 6:			//next chapter
		{
			output = width / 2 + 40;
			break;
		}
			
		case 7:			//trash
		{
			output = width - 25;
			
			if(trash != nil)
				output -= trash.frame.size.width;
			
			break;
		}
			
		case 8:		//Page counter, we set the middle of the place we want to put it
		{
			//Centered between the next chapter and the trashcan
			//output = ((self.frame.size.width / 2 + 40 + 20) + (self.frame.size.width - 25)) / 2;
			output = width * 3 / 4 + 17.5f;		//Optimized version
			break;
		}
	}
	
	return output;
}

- (void) recalculateElementsPosition : (BOOL) isAnimated : (CGFloat) newWidth
{
	RakButton* icons[] = {favorite, fullscreen, prevChapter, prevPage, nextPage, nextChapter, trash};
	short nbElem = [self numberIconsInBar];
	CGFloat midleHeightBar = self.frame.size.height / 2, lastElemHeight = 0;
	NSPoint origin = {0, 0};
	
	for(char pos = 0; pos < nbElem; pos++)
	{
		if (icons[pos] == nil)
			continue;
		
		origin.x = [self getPosXElement : pos+1 : newWidth];
		
		if(icons[pos].frame.size.height != lastElemHeight)
		{
			origin.y = midleHeightBar - icons[pos].frame.size.height / 2;
			lastElemHeight = icons[pos].frame.size.height;
		}
		
		if(isAnimated)
			[icons[pos].animator setFrameOrigin:origin];
		else
			[icons[pos] setFrameOrigin:origin];

	}
	
	//Repositionate pageCounter
	[pageCount updateSize:self.frame.size.height : [self getPosXElement : 8 : newWidth]];
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

- (void) resizeAnimation : (NSRect) frameRect
{
	[self setFrameInternal:frameRect :YES];
}

- (void) setFrame:(NSRect)frameRect
{
	[self setFrameInternal:frameRect :NO];
}

- (void) setFrameInternal : (NSRect) frameRect : (BOOL) isAnimated
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
	
	if(isAnimated)
		[self.animator setFrame:frameRect];
	else
		[super setFrame:frameRect];
	
	if(readerMode)
		[self recalculateElementsPosition : isAnimated : frameRect.size.width];
}

- (void) setFrameOrigin:(NSPoint)newOrigin
{
	if(readerMode)
		[super setFrameOrigin:newOrigin];
}

- (void) mouseUp:(NSEvent *)theEvent
{
	//Prevent a clic on the bar to end up on the page
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
	return heightWindow - RD_CONTROLBAR_POSY - RD_CONTROLBAR_HEIGHT;
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
