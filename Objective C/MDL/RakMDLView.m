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

@implementation RakMDLView

- (id)initContent: (NSRect) frame : (NSString *) state : (RakMDLController*) controller
{
    self = [super initWithFrame:frame];
	
    if (self)
	{
		[self setupInternal];
		
		headerText = [[RakMDLHeaderText alloc] initWithText:[self bounds] : @"Téléchargement" : [Prefs getSystemColor:GET_COLOR_BACKGROUND_TABS]];
		if(headerText != nil)	{	[self addSubview:headerText];	[headerText release];	}
		
		
		MDLList = [[RakMDLList alloc] init : [self getMainListFrame:[self bounds]] : controller];
		if(MDLList != nil)			[MDLList setSuperView:self];
		
		dropPlaceHolder = [[RakText alloc] initWithText:[self bounds] :@"Lâchez ici pour télécharger" : [Prefs getSystemColor:GET_COLOR_SURVOL]];
		if(dropPlaceHolder != nil)
		{
			[dropPlaceHolder setFont:[NSFont fontWithName:[Prefs getFontName:GET_FONT_RD_BUTTONS] size:15]];
			[dropPlaceHolder sizeToFit];
			[dropPlaceHolder setHidden:YES];
			[self addSubview:dropPlaceHolder];
			[dropPlaceHolder release];
		}
	}	
	return self;
}

- (CGFloat) getContentHeight
{
	CGFloat contentHeight = 0;
	
	if(headerText != nil)
		contentHeight += headerText.frame.size.height;
	
	if(MDLList != nil)
	{
		CGFloat height = [MDLList contentHeight];
		
		if(height)	height += 5;

		contentHeight+= height;
	}
	
	return contentHeight;
}

- (NSRect) getMainListFrame : (NSRect) output
{
	output.origin.x = output.origin.y = 0;
	
	if(headerText != nil)
		output.size.height -= 5 + headerText.frame.size.height;
	
	return output;
}

- (NSPoint) getPosDropPlaceHolder : (NSSize) frameSize
{
	CGFloat posY;
	
	if(headerText != nil)
		posY = (frameSize.height - headerText.frame.size.height) / 2 - dropPlaceHolder.frame.size.height / 2;
	else
		posY = frameSize.height / 2 - dropPlaceHolder.frame.size.height / 2;

	return NSMakePoint(frameSize.width / 2 - dropPlaceHolder.frame.size.width / 2, posY);
}

/** Proxy work **/

- (void) setFrameInternalViews:(NSRect)newBound
{
	[headerText setFrame:newBound];
	[MDLList setFrame:[self getMainListFrame:newBound]];

	[dropPlaceHolder setFrameOrigin: [self getPosDropPlaceHolder:newBound.size]];
}

- (void) resizeAnimationInternalViews : (NSRect) newBound
{
	[headerText.animator setFrame:[headerText getMenuFrame:newBound]];
	[MDLList resizeAnimation:[self getMainListFrame:newBound]];
	[dropPlaceHolder setFrameOrigin: [self getPosDropPlaceHolder:newBound.size]];
}

- (void) retainInternalViews
{
	[headerText retain];
	[MDLList retain];
	[dropPlaceHolder retain];
}

- (void) releaseInternalViews
{
	[headerText release];
	[MDLList release];
	[dropPlaceHolder release];
}

- (void) updateScroller : (BOOL) hidden
{
	if(MDLList != nil)
		[MDLList setScrollerHidden:hidden];
}

- (void) wakeUp
{
	if([MDLList isHidden])
		[MDLList setHidden:NO];
	[MDLList wakeUp];
}

- (void) hideList : (BOOL) hide
{
	[MDLList setHidden:hide];
}

- (void) drawFocusRing
{
	if(headerText == nil)
		return;
	
	NSColor * color = [self getBorderColor];
	NSRect frame = [self bounds];
	CGFloat radius = self.layer.cornerRadius;
	
	CGContextRef contextBorder = [[NSGraphicsContext currentContext] graphicsPort];
	CGContextSetLineWidth(contextBorder, 2.0);
	
	CGContextBeginPath(contextBorder);
	CGContextMoveToPoint(contextBorder, 0, frame.size.height - headerText.frame.size.height);
	CGContextAddLineToPoint(contextBorder, 0, radius);
	CGContextAddArc(contextBorder, radius, radius, radius, -M_PI, -M_PI_2, 0);
	CGContextAddLineToPoint(contextBorder, frame.size.width - radius, 0);
	CGContextAddArc(contextBorder, frame.size.width - radius, radius, radius, -M_PI_2, 0, 0);
	CGContextAddLineToPoint(contextBorder, frame.size.width, frame.size.height - headerText.frame.size.height);
	
	[color setStroke];
	CGContextStrokePath(contextBorder);
}

- (void) updateViewForFocusDrop
{
	if(dropPlaceHolder == nil)
		return;
	
	if([dropPlaceHolder isHidden] == isFocusDrop)
		[dropPlaceHolder setHidden:!isFocusDrop];
}

/** Color **/

- (NSColor*) getBackgroundColor
{
	byte code;
	switch (mainThread & GUI_THREAD_MASK)
	{
		case GUI_THREAD_READER:
		{
			code = GET_COLOR_BACKGROUD_MDL_READERMODE;
			break;
		}
			
		case GUI_THREAD_CT:
		{
			code = GET_COLOR_BACKGROUD_MDL_READERMODE;
			break;
		}
			
		case GUI_THREAD_SERIES:
		{
			code = GET_COLOR_BACKGROUD_MDL_READERMODE;
			break;
		}
			
		default:
			return [NSColor clearColor];
	}
	
	return [Prefs getSystemColor:code];
}

- (BOOL) proxyReceiveDrop : (MANGAS_DATA) data : (bool) isTome : (int) element : (uint) sender
{
	return (MDLList != nil && [MDLList receiveDrop:data :isTome :element :sender :-1 :NSTableViewDropOn]);
}

@end
