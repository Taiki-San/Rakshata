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

@implementation RakSerieView

- (instancetype) initContent:(NSRect)frame : (NSString *) state
{
	self = [super initWithFrame:frame];
	if(self != nil)
	{
		_currentValidHeight = frame.size.height;
		[self setupInternal];
		
		headerText = [[RakSRHeaderText alloc] initWithText : _bounds : NSLocalizedString(@"YOUR-PROJECTS", nil)];
		if(headerText != nil)
			[self addSubview:headerText];
		
		compactList = [[RakSerieList alloc] init : [self getCompactListFrame : _bounds] : mainThread == TAB_READER : state];
		if(compactList != nil)
			[self addSubview:[compactList getContent]];
		
		contentManager = [[RakSRContentManager alloc] init];
		if(contentManager != nil)
		{
			contentManager.controlView = self;
			contentManager.activeView = SR_CELLTYPE_GRID;
			[contentManager initViews];
		}
		
		if(mainThread == TAB_SERIES)
		{
			compactListHidden = YES;
			[compactList getContent].alphaValue = 0;
			[compactList getContent].hidden = YES;
			
			headerText.alphaValue = 0;
			headerText.hidden = YES;
		}
		else
		{
			compactListHidden = NO;
			[contentManager getActiveView].alphaValue = 0;
			[contentManager getActiveView].hidden = YES;
		}
	}
	
	return self;
}

- (void) setFrameInternalViews:(NSRect)newBound
{
	_currentValidHeight = newBound.size.height;
	
	[headerText setFrame:newBound];
	[compactList setFrame:[self getCompactListFrame : newBound]];
	[contentManager setFrame : newBound];
}

- (void) resizeAnimationInternalViews:(NSRect)newBound
{
	_currentValidHeight = newBound.size.height;
	
	if(headerText.isHidden)		[headerText setFrame:newBound];
	else						[headerText resizeAnimation: newBound];
	
	if(headerText.isHidden)		[compactList setFrame:[self getCompactListFrame : newBound]];
	else						[compactList resizeAnimation:[self getCompactListFrame : newBound]];
	
	if([contentManager getActiveView].isHidden)		[contentManager setFrame : newBound];
	else											[contentManager resizeAnimation : newBound];
}

- (NSString *) getContextToGTFO
{
	return [compactList getContextToGTFO];
}

- (void) dealloc
{
	headerText = nil;	compactList = nil;
}

- (BOOL) isFlipped
{
	return YES;	//Required to fix issues with QuickAccesses
}

- (void) mouseDown:(NSEvent *)theEvent
{
	
}

#pragma mark - Color

- (RakColor*) getBackgroundColor
{
	return [Prefs getSystemColor : COLOR_COREVIEW_BACKGROUND];
}

#pragma mark - Frame calcul

- (NSRect) getCompactListFrame : (NSRect) frame
{
	if(headerText != nil)
		frame.origin.y = headerText.frame.size.height + SR_READERMODE_LBWIDTH_OUTLINE;
	
	frame.size.height -= frame.origin.y;
	frame.origin.x = SR_READERMODE_MARGIN_OUTLINE / 2;
	frame.size.width -= 2 * SR_READERMODE_MARGIN_OUTLINE;
	
	return frame;
}

#pragma mark - Context change

- (void) setSerieViewHidden : (BOOL) serieViewHidden
{
	if(compactList != nil && compactListHidden == serieViewHidden)
	{
		compactListHidden = !serieViewHidden;
		
		RakTreeView * scrollview = [compactList getContent];
		
		if(_animatedContextChange)
		{
			if(serieViewHidden)
			{
				scrollview.hidden = NO;
				scrollview. alphaAnimated = 1;
				headerText.hidden = NO;
				headerText. alphaAnimated = 1;
			}
			else
			{
				scrollview. alphaAnimated = 0;
				headerText. alphaAnimated = 0;
			}
		}
		else
		{
			scrollview.hidden = headerText.hidden = serieViewHidden;
			scrollview.alphaValue = headerText.alphaValue = !serieViewHidden;
		}
	}
	
	if(contentManager != nil)
	{
		RakView * view = [contentManager getActiveView];
		
		if(!serieViewHidden)
			view.hidden = NO;
		
		if(_animatedContextChange)
			view. alphaAnimated = !serieViewHidden;
		else
			view.alphaValue = !serieViewHidden;
	}
}

- (void) setCTViewHidden : (BOOL) CTViewHidden
{
	if(!CTViewHidden && compactList != nil)
		compactList.installOnly = NO;
	
	[super setCTViewHidden:CTViewHidden];
}

- (void) setReaderViewHidden : (BOOL) readerViewHidden
{
	if(!readerViewHidden && compactList != nil)
		compactList.installOnly = YES;
	
	[super setReaderViewHidden:readerViewHidden];
}

- (void) cleanupFocusViewChange
{
	for(RakView * view in @[[compactList getContent], headerText, [contentManager getActiveView]])
	{
		if(view.alphaValue == 0)
			view.hidden = YES;
	}
}

@end
