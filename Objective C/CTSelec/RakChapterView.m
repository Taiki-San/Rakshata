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

@implementation RakChapterView

- (id)initContent:(NSRect)frame : (PROJECT_DATA) project : (bool) isTome : (long [4]) context
{
    self = [super initWithFrame:frame];
    if (self)
	{
		[self setupInternal];
		
		BOOL readerMode = [[NSApp delegate] CT].readerMode;

		coreView = [[RakCTContentTabView alloc] initWithProject : project : isTome : [self bounds] : context];
		if(coreView != nil)		[self addSubview:coreView];
		
		[self initCTView : project : readerMode];
		[self initReaderView : project : readerMode];
    }
    return self;
}

- (NSString *) getContextToGTFO
{
	if (coreView == nil)
		return nil;
	return [coreView getContextToGTFO];
}

- (void) setFrameInternalViews : (NSRect) newBound
{
	[projectName setFrame:newBound];
	[projectImage setFrame:newBound];
	[coreView setFrame:newBound];
}

- (void) resizeAnimationInternalViews:(NSRect)newBound
{
	[projectName resizeAnimation:newBound];
	[projectImage resizeAnimation:newBound];
	[coreView resizeAnimation:newBound];
}

- (void) retainInternalViews
{
	[projectName retain];
	[projectImage retain];
	[coreView retain];
}

- (void) releaseInternalViews
{
	[projectName release];
	[projectImage release];
	[coreView release];
}

- (void) dealloc
{
	[projectName removeFromSuperview];
	[projectName release];
	
	[projectImage removeFromSuperview];
	[projectImage release];
	
	[coreView removeFromSuperview];
	[coreView release];
	
	[super dealloc];
}

#pragma mark - UI Initializers

- (void) initCTView : (PROJECT_DATA) project : (BOOL) readerMode
{
	header = [[RakCTHeader alloc] initWithData : self.bounds : project];
	if(header != nil)
		[self addSubview:header];
	
	self.CTViewHidden = readerMode;
}

- (void) initReaderView : (PROJECT_DATA) project : (BOOL) readerMode
{
	projectName = [[RakTextProjectName alloc] initWithText:[self bounds] : [[[NSString alloc] initWithData:[NSData dataWithBytes:project.projectName length:sizeof(project.projectName)] encoding:NSUTF32LittleEndianStringEncoding] autorelease]];
	if(projectName != nil)	[self addSubview:projectName];
	
	projectImage = [[RakCTProjectImageView alloc] initWithImageName: project.team->URLRepo : [NSString stringWithFormat:@"%d_"PROJ_IMG_SUFFIX_CT, project.projectID] : [self bounds]];
	if(projectImage != nil)	[self addSubview:projectImage];
	
	self.readerViewHidden = !readerMode;
}

- (void) setCTViewHidden : (BOOL) CTViewHidden
{
	if(CTViewHidden)
	{
		if(!header.isHidden)
			[header setHidden:YES];
	}
	else
	{
		if(header.isHidden)
			[header setHidden:NO];
	}
	
	[super setCTViewHidden:CTViewHidden];
}

- (void) setReaderViewHidden : (BOOL) readerViewHidden
{
	if(readerViewHidden)
	{
		if(!projectImage.isHidden)
			[projectName setHidden:YES];
		if(!projectImage.isHidden)
			[projectImage setHidden:YES];
		if(!coreView.isHidden)
			[coreView setHidden:YES];
	}
	else
	{
		if(projectImage.isHidden)
			[projectName setHidden:NO];
		if(projectImage.isHidden)
			[projectImage setHidden:NO];
		if(coreView.isHidden)
			[coreView setHidden:NO];
	}
	
	[super setReaderViewHidden:readerViewHidden];
}

#pragma mark - Color

- (NSColor*) getBackgroundColor
{
	byte code;
	switch (mainThread & TAB_MASK)
	{
		case TAB_READER:
		{
			code = GET_COLOR_BACKGROUD_CT_READERMODE;
			break;
		}
			
		case TAB_CT:
		{
			code = GET_COLOR_BACKGROUND_TABS;
			break;
		}
			
		case TAB_SERIES:
		{
			code = GET_COLOR_BACKGROUD_CT_READERMODE;
			break;
		}
			
		default:
			return [NSColor clearColor];
	}
	
	return [Prefs getSystemColor:code : nil];
}

#pragma mark - Proxy

- (void) updateContext : (PROJECT_DATA) data
{
	NSString *projectNameString = [[NSString alloc] initWithData:[NSData dataWithBytes:data.projectName length:sizeof(data.projectName)] encoding:NSUTF32LittleEndianStringEncoding];
	
	if(projectName != nil)
		[projectName setStringValue : projectNameString];
	else
	{
		projectName = [[RakTextProjectName alloc] initWithText:[self bounds] : projectNameString];
		if(projectName != nil)		[self addSubview:projectName];
	}

	if(projectImage != nil)
		[projectImage updateProject:projectNameString];
	else
	{
		projectImage = [[RakCTProjectImageView alloc] initWithImageName: data.team->URLRepo : [NSString stringWithFormat:@"%d_"PROJ_IMG_SUFFIX_CT, data.projectID] : [self bounds]];
		if(projectImage != nil)		[self addSubview:projectImage];
	}
	
	if(coreView != nil)
	{
		if(![coreView updateContext:data])
			coreView = nil;
	}
	else
	{
		coreView = [[RakCTContentTabView alloc] initWithProject : data : false : [self bounds] : (long [4]) {-1, -1, -1, -1}];
		if(coreView != nil)			[self addSubview:coreView];
	}
}

- (BOOL) refreshCT : (BOOL) checkIfRequired : (uint) ID;
{
	if (coreView != nil)
	{
		[coreView refreshCTData : checkIfRequired : ID];
		return YES;
	}
	
	return NO;
}

- (void) selectElem : (uint) projectID : (BOOL) isTome : (int) element
{
	if(coreView != nil)
		[coreView selectElem : projectID : isTome : element];
}

@end


