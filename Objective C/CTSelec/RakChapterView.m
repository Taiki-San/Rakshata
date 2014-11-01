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
		
		BOOL readerMode = [(RakAppDelegate*) [NSApp delegate]CT].readerMode;

		[self initCTView : project : readerMode];
		[self initReaderView : project : readerMode];

		coreview = [[RakCTSelection alloc] initWithProject : project : isTome : self.bounds : (header != nil ? header.bounds.size.height : 0) : context];
		if(coreview != nil)
			[self addSubview:coreview];
    }
    return self;
}

- (NSString *) getContextToGTFO
{
	if (coreview == nil)
		return nil;
	return [coreview getContextToGTFO];
}

- (void) setFrameInternalViews : (NSRect) newBound
{
	if(!self.CTViewHidden)
	{
		[header setFrame:newBound];
	}

	if(!self.readerViewHidden)
	{
		[projectName setFrame:newBound];
		[projectImage setFrame:newBound];
	}

	[coreview setFrame : newBound : (header != nil ? header.bounds.size.height : 0)];
}

- (void) resizeAnimationInternalViews : (NSRect) newBound
{
	CGFloat headerHeight = 0;
	
	if(!self.CTViewHidden)
	{
		[header resizeAnimation:newBound];
		headerHeight = [header frameByParent : newBound].size.height;
	}
	
	if(!self.readerViewHidden)
	{
		[projectName resizeAnimation:newBound];
		[projectImage resizeAnimation:newBound];
	}

	[coreview resizeAnimation : newBound : headerHeight];
}

- (void) dealloc
{
	[header removeFromSuperview];		header = nil;
	
	[projectName removeFromSuperview];	projectName = nil;
	[projectImage removeFromSuperview];	projectImage = nil;
	
	[coreview removeFromSuperview];		coreview = nil;
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
	projectName = [[RakMenuText alloc] initWithText:[self bounds] : [[NSString alloc] initWithData:[NSData dataWithBytes:project.projectName length:sizeof(project.projectName)] encoding:NSUTF32LittleEndianStringEncoding]];
	if(projectName != nil)	[self addSubview:projectName];
	
	projectImage = [[RakCTProjectImageView alloc] initWithImageName: project.team->URLRepo : [NSString stringWithFormat:@"%d_"PROJ_IMG_SUFFIX_CT, project.projectID] : [self bounds]];
	if(projectImage != nil)	[self addSubview:projectImage];
	
	self.readerViewHidden = !readerMode;
}

- (void) setCTViewHidden : (BOOL) CTViewHidden
{
	if(_CTViewHidden == CTViewHidden)
		return;

	if(CTViewHidden)
		[header setHidden:YES];
	else
	{
		[header setHidden:NO];
		coreview.currentContext = TAB_CT;
	}
	
	[super setCTViewHidden:CTViewHidden];
}

- (void) setReaderViewHidden : (BOOL) readerViewHidden
{
	if(_readerViewHidden == readerViewHidden)
		return;
	
	if(readerViewHidden)
	{
		[projectName setHidden:YES];
		[projectImage setHidden:YES];
	}
	else
	{
		[projectName setHidden:NO];
		[projectImage setHidden:NO];
		
		coreview.currentContext = TAB_READER;
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
		projectName = [[RakMenuText alloc] initWithText:[self bounds] : projectNameString];
		if(projectName != nil)		[self addSubview:projectName];
	}

	if(projectImage != nil)
		[projectImage updateProject:projectNameString];
	else
	{
		projectImage = [[RakCTProjectImageView alloc] initWithImageName: data.team->URLRepo : [NSString stringWithFormat:@"%d_"PROJ_IMG_SUFFIX_CT, data.projectID] : [self bounds]];
		if(projectImage != nil)		[self addSubview:projectImage];
	}
	
	if(header != nil)
		[header updateProject:data];
	else
	{
		header = [[RakCTHeader alloc] initWithData : self.bounds :data];
		if(header != nil)
			[self addSubview:header];
	}
	
	if(coreview != nil)
	{
		if(![coreview updateContext : data])
			coreview = nil;
	}
	else
	{
		coreview = [[RakCTSelection alloc] initWithProject : data : false : self.bounds : (header != nil ? header.bounds.size.height : 0) : (long [4]) {-1, -1, -1, -1}];
		if(coreview != nil)
			[self addSubview:coreview];
	}
}

#warning "Granular update"
- (void) projectDataUpdate : (PROJECT_DATA) oldData : (PROJECT_DATA) newData
{
//	uint length;
//	if((length = wstrlen(oldData.projectName)) != wstrlen(newData.projectName) || memcmp(oldData.projectName, newData.projectName, length * sizeof(wchar_t)))
//	{
//		
//	}
	[self updateContext:newData];
}

- (void) selectElem : (uint) projectID : (BOOL) isTome : (int) element
{
	if(coreview != nil)
		[coreview selectElem : projectID : isTome : element];
}

@end


