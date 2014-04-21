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

- (void) dealloc
{
	[favorite removeFromSuperview];		[favorite release];
	[fullscreen removeFromSuperview];	[fullscreen release];
	[prevChapter removeFromSuperview];	[prevChapter release];
	[prevPage removeFromSuperview];		[prevPage release];
	[nextPage removeFromSuperview];		[nextPage release];
	[nextChapter removeFromSuperview];	[nextChapter release];
	[trash removeFromSuperview];		[trash release];
	
	[pageCount removeFromSuperview];	[pageCount release];
	
	[super dealloc];
}

#pragma mark - Update page counter

- (void) updatePage : (uint) newCurrentPage : (uint) newPageMax
{
	if(pageCount == nil)
	{
		pageCount = [[RakPageCounter alloc] init: self : [self getPosXElement:8] :newCurrentPage :newPageMax];
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
	
	favorite = [RakButton allocForReader:self :@"fav" : RB_STATE_STANDARD :[self getPosXElement:1] :YES :self :@selector(buttonHitten)];
	fullscreen = [RakButton allocForReader:self :@"Fullscreen" : RB_STATE_STANDARD :[self getPosXElement:2] :YES :superview :@selector(triggerFullscreen)];
	
	prevChapter = [RakButton allocForReader:self :@"first" : RB_STATE_STANDARD :[self getPosXElement:3] :NO :superview :@selector(prevChapter)];
	prevPage = [RakButton allocForReader:self :@"before" : RB_STATE_STANDARD :[self getPosXElement:4] :NO :superview :@selector(prevPage)];
	nextPage = [RakButton allocForReader:self :@"next" : RB_STATE_STANDARD :[self getPosXElement:5] :YES :superview :@selector(nextPage)];
	nextChapter = [RakButton allocForReader:self :@"last" : RB_STATE_STANDARD :[self getPosXElement:6] :YES :superview :@selector(nextChapter)];

	trash = [RakButton allocForReader:self :@"X": RB_STATE_STANDARD :[self getPosXElement:7] :NO :superView :@selector(deleteElement)];
}

- (CGFloat) getPosXElement : (uint) IDButton
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
			
		case 6:			//next chapter
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
			
		case 8:		//Page counter, we set the middle of the place we want to put it
		{
			//Centered between the next chapter and the trashcan
			//output = ((self.frame.size.width / 2 + 40 + 20) + (self.frame.size.width - 25)) / 2;
			output = self.frame.size.width * 3 / 4 + 17.5f;		//Optimized version
			break;
		}
	}
	
	return output;
}

- (void) recalculateElementsPosition
{
	RakButton* icons[] = {favorite, fullscreen, prevChapter, prevPage, nextPage, nextChapter, trash};
	short nbElem = [self numberIconsInBar];
	CGFloat midleHeightBar = self.frame.size.height / 2, lastElemHeight = 0;
	NSPoint origin = {0, 0};
	
	for(char pos = 0; pos < nbElem; pos++)
	{
		if (icons[pos] == nil)
			continue;
		
		origin.x = [self getPosXElement:pos+1];
		
		if(icons[pos].frame.size.height != lastElemHeight)
		{
			origin.y = midleHeightBar - icons[pos].frame.size.height / 2;
			lastElemHeight = icons[pos].frame.size.height;
		}
		
		[icons[pos] setFrameOrigin:origin];
	}
	
	//Repositionate pageCounter
	[pageCount updateSize:self.frame.size.height : [self getPosXElement:8]];
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
		[self recalculateElementsPosition];
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
