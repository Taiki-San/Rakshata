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
    if (self)
	{
		[self setupInternal];
		
		headerText = [[RakSRHeaderText alloc] initWithText : self.bounds : @"Vos séries"];
		if(headerText != nil)
			[self addSubview:headerText];
		
		compactList = [[RakSerieList alloc] init : [self getCompactListFrame : self.bounds] : mainThread == TAB_READER : state];
		if(compactList != nil)
			[self addSubview:[compactList getContent]];
		
		gridView = [[RakGridView alloc] initWithFrame:self.bounds];
		if(gridView != nil)
			[self addSubview : gridView.contentView];
		
		if(mainThread == TAB_SERIES)
		{
			compactListHidden = compactList.hidden = headerText.hidden = YES;
			self.layer.backgroundColor = nil;
		}
		else
		{
			compactListHidden = NO;
			gridView.hidden = YES;
		}
	}
	
	return self;
}

- (void) setFrameInternalViews:(NSRect)newBound
{
	[headerText setFrame:newBound];
	[compactList setFrame:[self getCompactListFrame : newBound]];
	[gridView setFrame : newBound];
}

- (void) resizeAnimationInternalViews:(NSRect)newBound
{
	[headerText resizeAnimation: newBound];
	[compactList resizeAnimation:[self getCompactListFrame : newBound]];
	[gridView resizeAnimation : newBound];
}

- (NSString *) getContextToGTFO
{
	return [compactList getContextToGTFO];
}

- (void) dealloc
{
	headerText = nil;	compactList = nil;
}

- (BOOL)isFlipped
{
	return YES;	//Required to fix issues with QuickAccesses
}

#pragma mark - Color

- (NSColor*) getBackgroundColor
{
	byte code;
	switch (mainThread & TAB_MASK)
	{
		case TAB_READER:
		{
			code = GET_COLOR_BACKGROUD_SR_READERMODE;
			break;
		}
			
		case TAB_CT:
		{
			code = GET_COLOR_BACKGROUD_SR_READERMODE;
			break;
		}
			
		case TAB_SERIES:
		{
			code = GET_COLOR_BACKGROUD_SR_READERMODE;
			break;
		}
			
		default:
			return [NSColor clearColor];
	}
	
	return [Prefs getSystemColor : code : nil];
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
	if(!serieViewHidden)
	{
		if(compactList != nil && !compactListHidden)
			compactListHidden = compactList.hidden = headerText.hidden = YES;
		
		if(gridView != nil && gridView.hidden)
			gridView.hidden = NO;
		
		if(self.layer.backgroundColor != nil)
			self.layer.backgroundColor = nil;
	}
	else
	{
		if(self.layer.backgroundColor == nil)
			self.layer.backgroundColor = [self getBackgroundColor].CGColor;
		
		if(gridView != nil && !gridView.hidden)
			gridView.hidden = YES;
	}
}

- (void) setCTViewHidden : (BOOL) CTViewHidden
{
	if(!CTViewHidden && compactList != nil)
	{
		if(compactListHidden)
			compactListHidden = compactList.hidden = headerText.hidden = NO;
		
		compactList.installOnly = NO;
	}
	
	[super setCTViewHidden:CTViewHidden];
}

- (void) setReaderViewHidden : (BOOL) readerViewHidden
{
	if(!readerViewHidden && compactList != nil)
	{
		if(compactListHidden)
			compactListHidden = compactList.hidden = headerText.hidden = NO;
		
		compactList.installOnly = YES;
	}
	
	[super setReaderViewHidden:readerViewHidden];
}


@end
