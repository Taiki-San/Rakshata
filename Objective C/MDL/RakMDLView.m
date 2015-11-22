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

enum
{
	HEADER_TEXT_HEIGHT = 28
};

@implementation RakMDLView

- (instancetype) initContent: (NSRect) frame : (NSString *) state : (RakMDLController*) controller
{
	self = [super initWithFrame:frame];
	
	if(self != nil)
	{
		_controller = controller;
		[self setupInternal];
		
		headerText = [[RakMenuText alloc] initWithText:_bounds : NSLocalizedString(@"MDL-TAB-TITLE", nil)];
		if(headerText != nil)
		{
			headerText.barWidth = 1;
			
			[self addSubview:headerText];
		}
		
		MDLList = [[RakMDLList alloc] init : [self getMainListFrame:_bounds] : controller];
		if(MDLList != nil)			MDLList.superview = self;
		
		dropPlaceHolder = [[RakText alloc] initWithText:_bounds :NSLocalizedString(@"MDL-DROP-PLACEHOLDER", nil) : [Prefs getSystemColor:COLOR_SURVOL]];	//setupInternal already register
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
		const CGFloat height = [MDLList contentHeight];
		
		if(height)
			contentHeight += height + 5;
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
		return [super observeValueForKeyPath:keyPath ofObject:object change:change context:context];
	
	[dropPlaceHolder setTextColor:[Prefs getSystemColor:COLOR_SURVOL]];
	[self setNeedsDisplay:YES];
}

#pragma mark - Proxy work

- (void) setFrameInternalViews:(NSRect)newBound
{
	[headerText setFrame:_bounds];
	[MDLList setFrame:[self getMainListFrame:newBound]];
	
	[dropPlaceHolder setFrameOrigin: [self getPosDropPlaceHolder:newBound.size]];
}

- (void) resizeAnimationInternalViews : (NSRect) newBound
{
	[headerText resizeAnimation:newBound];
	[MDLList resizeAnimation:[self getMainListFrame:newBound]];
	[dropPlaceHolder.animator setFrameOrigin: [self getPosDropPlaceHolder:newBound.size]];
	
	[MDLList checkIfShouldReload];
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
	NSRect frame = _bounds;
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

#pragma mark - Color

- (NSColor*) getBackgroundColor
{
	return [Prefs getSystemColor: COLOR_COREVIEW_BACKGROUND];
}

- (BOOL) proxyReceiveDrop : (PROJECT_DATA) data : (BOOL) isTome : (uint) element : (uint) sender
{
	return (MDLList != nil && [MDLList receiveDrop:data :isTome :element :sender :-1 :NSTableViewDropOn]);
}

@end
