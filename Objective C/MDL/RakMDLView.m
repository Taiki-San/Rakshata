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
		_controller = controller;
		[self setupInternal];
		
		headerText = [[RakMDLHeaderText alloc] initWithText:[self bounds] : @"Téléchargement"];
		if(headerText != nil)	{	[self addSubview:headerText];		}
		
		MDLList = [[RakMDLList alloc] init : [self getMainListFrame:[self bounds]] : controller];
		if(MDLList != nil)			MDLList.superview = self;
		
		dropPlaceHolder = [[RakText alloc] initWithText:[self bounds] :@"Lâchez ici pour télécharger" : [Prefs getSystemColor:GET_COLOR_SURVOL : nil]];	//setupInternal already register
		if(dropPlaceHolder != nil)
		{
			[dropPlaceHolder setFont:[NSFont fontWithName:[Prefs getFontName:GET_FONT_RD_BUTTONS] size:15]];
			[dropPlaceHolder sizeToFit];
			[dropPlaceHolder setHidden:YES];
			[self addSubview:dropPlaceHolder];
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

- (void) observeValueForKeyPath:(NSString *)keyPath ofObject:(id)object change:(NSDictionary *)change context:(void *)context
{
	if([object class] != [Prefs class])
		return;
	
	[dropPlaceHolder setTextColor:[Prefs getSystemColor:GET_COLOR_SURVOL : nil]];
	[super observeValueForKeyPath:keyPath ofObject:object change:change context:context];
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
	[dropPlaceHolder.animator setFrameOrigin: [self getPosDropPlaceHolder:newBound.size]];
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
	{
		RakTabForegroundView * foregroundView;
		
		[dropPlaceHolder setAlphaValue:!isFocusDrop];
		
		if(dropPlaceHolder.isHidden)
			[dropPlaceHolder setHidden : NO];
		
		[NSAnimationContext beginGrouping];
		
		[[NSAnimationContext currentContext] setDuration:0.1f];
		[dropPlaceHolder.animator setAlphaValue:isFocusDrop];
		
		foregroundView = [_controller getForegroundView];
		if(foregroundView != nil)
		{
			if(foregroundView.animationInProgress)
			{
				do	{	usleep(500);	}	while(foregroundView.animationInProgress);
				foregroundView = [_controller getForegroundView];
			}
			
			if(foregroundView != nil)
			{
				[foregroundView setAlphaValue:isFocusDrop];

				if(foregroundView.isHidden)
					[foregroundView setHidden:NO];

				[foregroundView.animator setAlphaValue:!isFocusDrop];
			}
		}
		
		[[NSAnimationContext currentContext] setCompletionHandler:^{

			if(!isFocusDrop)
				[dropPlaceHolder setHidden : YES];
			else if(foregroundView != nil)
				[foregroundView setHidden : YES];
		}];
		
		[NSAnimationContext endGrouping];
	}
}

/** Color **/

- (NSColor*) getBackgroundColor
{
	byte code;
	switch (mainThread & TAB_MASK)
	{
		case TAB_READER:
		{
			code = GET_COLOR_BACKGROUD_MDL_READERMODE;
			break;
		}
			
		case TAB_CT:
		{
			code = GET_COLOR_BACKGROUD_MDL_READERMODE;
			break;
		}
			
		case TAB_SERIES:
		{
			code = GET_COLOR_BACKGROUD_MDL_READERMODE;
			break;
		}
			
		default:
			return [NSColor clearColor];
	}
	
	return [Prefs getSystemColor: code: nil];
}

- (BOOL) proxyReceiveDrop : (PROJECT_DATA) data : (bool) isTome : (int) element : (uint) sender
{
	return (MDLList != nil && [MDLList receiveDrop:data :isTome :element :sender :-1 :NSTableViewDropOn]);
}

@end
