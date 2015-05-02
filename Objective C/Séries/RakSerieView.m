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

- (id)initContent:(NSRect)frame : (NSString *) state
{
    self = [super initWithFrame:frame];
    if(self != nil)
	{
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
	[headerText setFrame:newBound];
	[compactList setFrame:[self getCompactListFrame : newBound]];
	[contentManager setFrame : newBound];
}

- (void) resizeAnimationInternalViews:(NSRect)newBound
{
	[headerText resizeAnimation: newBound];
	[compactList resizeAnimation:[self getCompactListFrame : newBound]];
	[contentManager resizeAnimation : newBound];
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

#pragma mark - Color

- (NSColor*) getBackgroundColor
{
	return [Prefs getSystemColor : GET_COLOR_BACKGROUND_COREVIEW : nil];
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
				scrollview.animator.alphaValue = 1;
				headerText.hidden = NO;
				headerText.animator.alphaValue = 1;
			}
			else
			{
				scrollview.animator.alphaValue = 0;
				headerText.animator.alphaValue = 0;
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
		NSView * view = [contentManager getActiveView];
		
		if(!serieViewHidden)
			view.hidden = NO;
		
		if(_animatedContextChange)
			view.animator.alphaValue = !serieViewHidden;
		else
			view.alphaValue = !serieViewHidden;
	}
}

- (void) setCTViewHidden : (BOOL) CTViewHidden
{
	if(!CTViewHidden && compactList != nil)
		compactList.installOnly = YES;
	
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
	for(NSView * view in @[[compactList getContent], headerText, [contentManager getActiveView]])
	{
		if(view.alphaValue == 0)
			view.hidden = YES;
	}
}

@end
