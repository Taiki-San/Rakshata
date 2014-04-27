/*********************************************************************************************
 **	__________         __           .__            __                 ________   _______   	**
 **	\______   \_____  |  | __  _____|  |__ _____ _/  |______   	___  _\_____  \  \   _  \  	**
 **	 |       _/\__  \ |  |/ / /  ___/  |  \\__  \\   __\__  \  	\  \/ //  ____/  /  /_\  \ 	**
 **	 |    |   \ / __ \|    <  \___ \|   Y  \/ __ \|  |  / __ \__ \   //       \  \  \_/   \	**
 **	 |____|_  /(____  /__|_ \/____  >___|  (____  /__| (____  /	  \_/ \_______ \ /\_____  /	**
 **	        \/      \/     \/     \/     \/     \/          \/ 	              \/ \/     \/ 	**
 **                                                                                          **
 **    Licence propriétaire, code source confidentiel, distribution formellement interdite   **
 **                                                                                          **
 *********************************************************************************************/

@implementation MDL

- (id)init : (NSView*)contentView : (NSString *) state
{
    self = [super init];
    if (self)
	{
		flag = GUI_THREAD_MDL;
		self = [self initView: contentView : state];
		
		self.layer.borderColor = [Prefs getSystemColor:GET_COLOR_BORDER_TABS].CGColor;
		self.layer.borderWidth = 2;
		
		[self initContent:state];
	}
    return self;
}

- (void) initContent : (NSString *) state
{
	coreView = [[RakMDLView alloc]  initContent:[self getCoreviewFrame] : state];
	if(coreView != nil)
		[self addSubview:coreView];
}

/*Coreview manipulation*/

- (NSRect) getCoreviewFrame
{
	NSRect output = [self bounds];
	
	output.origin.x = MDL_READERMODE_LATERAL_BORDER;
	output.origin.y = MDL_READERMODE_BOTTOMBAR_WIDTH;
	output.size.height -= MDL_READERMODE_BOTTOMBAR_WIDTH;
	output.size.width -= 2 * MDL_READERMODE_LATERAL_BORDER;
	
	return output;
}

- (void) setFrame:(NSRect)frameRect
{
	[super setFrame:frameRect];
	if(coreView != nil)
		[coreView setFrame:frameRect];
}

/*Internal stuffs*/

- (BOOL) isStillCollapsedReaderTab
{
	int state;
	[Prefs getPref:PREFS_GET_READER_TABS_STATE :&state];
	return (state & STATE_READER_TAB_MDL_FOCUS) == 0;
}

- (NSRect) generateNSTrackingAreaSize : (NSRect) viewFrame
{
	CGFloat posReader;
	NSRect frame = viewFrame;
	[Prefs getPref:PREFS_GET_TAB_READER_POSX :&posReader];
	frame.size.width = posReader * self.superview.frame.size.width / 100;
	frame.origin.x = frame.origin.y = 0;
	
	return frame;
}

- (void) refreshViewSize
{
	NSRect frame = [self createFrame];
	
	[self setFrameSize:frame.size];
	[self setFrameOrigin:frame.origin];
	
	[self refreshDataAfterAnimation];
}

- (BOOL) acceptsFirstMouse:(NSEvent *)theEvent { return NO; }
- (BOOL) acceptsFirstResponder { return NO; }

/**	 Get View Size	**/

- (CGFloat) getRequestedViewPosX:(CGFloat) widthWindow
{
	CGFloat output;
	[Prefs getPref:PREFS_GET_MDL_POSX:&output];
	return output * widthWindow / 100;
}

- (CGFloat) getRequestedViewPosY:(CGFloat) heightWindow
{
	CGFloat output;
	[Prefs getPref:PREFS_GET_MDL_POSY:&output];
	return output * heightWindow / 100;
}

- (CGFloat) getRequestedViewHeight:(CGFloat) heightWindow
{
	CGFloat output;
	[Prefs getPref:PREFS_GET_MDL_HEIGHT: &output];
	return output * heightWindow / 100;
}

- (CGFloat) getRequestedViewWidth: (CGFloat) widthWindow
{
	CGFloat prefData;
	[Prefs getPref:PREFS_GET_MDL_WIDTH:&prefData];
	return widthWindow * prefData / 100;
}

- (int) getCodePref : (int) request
{
	int output;
	
	switch (request)
	{
		case CONVERT_CODE_POSX:
		{
			output = PREFS_GET_MDL_POSX;
			break;
		}
			
		case CONVERT_CODE_POSY:
		{
			output = PREFS_GET_MDL_POSY;
			break;
		}
			
		case CONVERT_CODE_HEIGHT:
		{
			output = PREFS_GET_MDL_HEIGHT;
			break;
		}
			
		case CONVERT_CODE_WIDTH:
		{
			output = PREFS_GET_MDL_WIDTH;
			break;
		}
			
		case CONVERT_CODE_FRAME:
		{
			output = PREFS_GET_MDL_FRAME;
			break;
		}
			
		default:
			output = 0;
	}
	
	return output;
}

@end
