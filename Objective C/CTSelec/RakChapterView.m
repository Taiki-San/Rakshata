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

- (instancetype) initContent : (NSRect)frame : (PROJECT_DATA) project : (bool) isTome : (long [4]) context
{
    self = [super initWithFrame:frame];
    if (self != nil)
	{
		[self setupInternal];
		
		coreview = [[RakCTSelection alloc] initWithProject : project : isTome : _bounds : (header != nil ? header.bounds.size.height : 0) : context : mainThread];
		if(coreview != nil)
			[self addSubview:coreview];

		[self initSerieView : project : mainThread & TAB_SERIES];
		[self initCTView : project : mainThread & TAB_CT];
		[self initReaderView : project : mainThread & TAB_READER];
		
		[[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(respondToSRFocus:) name:SR_NOTIFICATION_FOCUS object:nil];
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
	if(!self.serieViewHidden)
	{
		[suggestions setFrame:newBound];
	}
	
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
	
	if(!self.serieViewHidden)
	{
		[suggestions resizeAnimation:newBound];
	}
	
	if(!self.CTViewHidden)
	{
		[header resizeAnimation:newBound];
		headerHeight = [header frameByParent : newBound : nil].size.height;
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

- (void) initSerieView : (PROJECT_DATA) project : (BOOL) serieMode
{
	suggestions = [[RakSRSuggestions alloc] init: _bounds];
	if(suggestions != nil)
	{
		[suggestions getContent].alphaValue = serieMode;
		[suggestions setHidden:!serieMode];
		[self addSubview:[suggestions getContent]];
	}
	
	mainDetailView = [[RakSRDetails alloc] initWithFrame:_bounds];
	tmpDetailView = [[RakSRDetails alloc] initWithFrame:_bounds];
	
	self.serieViewHidden = !serieMode;
}

- (void) initCTView : (PROJECT_DATA) project : (BOOL) CTMode
{
	header = [[RakCTHeader alloc] initWithData : self.bounds : project];
	if(header != nil)
		[self addSubview:header];
	
	self.CTViewHidden = !CTMode;
}

- (void) initReaderView : (PROJECT_DATA) project : (BOOL) readerMode
{
	projectName = [[RakMenuText alloc] initWithText:_bounds : project.isInitialized ? getStringForWchar(project.projectName) : @""];
	if(projectName != nil)	[self addSubview:projectName];
	
	if(project.isInitialized)
		projectImage = [[RakCTProjectImageView alloc] initWithImageName: project.repo : [NSString stringWithFormat:@"%d_"PROJ_IMG_SUFFIX_CT, project.projectID] : _bounds];
	else
		projectImage = [[RakCTProjectImageView alloc] initWithImageName: NULL : nil : _bounds];
	
	if(projectImage != nil)	[self addSubview:projectImage];
	
	self.readerViewHidden = !readerMode;
}

- (void) setSerieViewHidden : (BOOL) serieViewHidden
{
	if(serieViewHidden)
	{
		[suggestions getContent].animator.alphaValue = 0;
		if(mainDetailView != nil && mainDetailView.superview != nil)
			mainDetailView.animator.alphaValue = 0;
	}
	else
	{
		[suggestions setHidden:NO];
		[suggestions getContent].animator.alphaValue = 1;
		coreview.currentContext = TAB_SERIES;
	}
	
	[super setSerieViewHidden:serieViewHidden];
}

- (void) setCTViewHidden : (BOOL) CTViewHidden
{
	if(CTViewHidden)
	{
		header.animator.alphaValue = 0;
	}
	else
	{
		header.hidden = NO;	header.animator.alphaValue = 1;
		coreview.currentContext = TAB_CT;
	}
	
	[super setCTViewHidden:CTViewHidden];
}

- (void) setReaderViewHidden : (BOOL) readerViewHidden
{
	if(readerViewHidden)
	{
		projectName.animator.alphaValue = 0;
		projectImage.animator.alphaValue = 0;
	}
	else
	{
		projectName.hidden = NO;	projectName.animator.alphaValue = 1;
		projectImage.hidden = NO;	projectImage.animator.alphaValue = 1;
		
		coreview.currentContext = TAB_READER;
	}
	
	[super setReaderViewHidden:readerViewHidden];
}

- (void) cleanupFocusViewChange
{
	for(NSView * view in @[header, projectName, projectImage])
	{
		if(view.alphaValue == 0)
			view.hidden = YES;
	}
	
	if([suggestions getContent].alphaValue == 0)
		suggestions.hidden = YES;
	
	if(self.serieViewHidden)
	{
		[mainDetailView removeFromSuperview];
		mainDetailView.alphaValue = 1;
		[tmpDetailView removeFromSuperview];
	}
	
	[coreview cleanChangeCurrentContext];
}

#pragma mark - Color

- (NSColor*) getBackgroundColor
{
	byte code;
	switch (mainThread & TAB_MASK)
	{
		case TAB_READER:
		case TAB_SERIES:
		{
			code = GET_COLOR_BACKGROUD_COREVIEW;
			break;
		}
			
		case TAB_CT:
		{
			code = GET_COLOR_BACKGROUND_TABS;
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
	NSString *projectNameString = getStringForWchar(data.projectName);
	
	if(projectName != nil)
		[projectName setStringValue : projectNameString];
	else
	{
		projectName = [[RakMenuText alloc] initWithText:_bounds : projectNameString];
		if(projectName != nil)		[self addSubview:projectName];
	}

	if(projectImage != nil)
		[projectImage updateProject:projectNameString];
	else
	{
		projectImage = [[RakCTProjectImageView alloc] initWithImageName: data.repo : [NSString stringWithFormat:@"%d_"PROJ_IMG_SUFFIX_CT, data.projectID] : _bounds];
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
		[coreview updateContext : data];
	}
	else
	{
		coreview = [[RakCTSelection alloc] initWithProject : data : false : self.bounds : (header != nil ? header.bounds.size.height : 0) : (long [4]) {-1, -1, -1, -1} : mainThread];
		if(coreview != nil)
			[self addSubview:coreview];
	}
}

- (void) projectDataUpdate : (PROJECT_DATA) oldData : (PROJECT_DATA) newData
{
	uint length;
	if((length = wstrlen(oldData.projectName)) != wstrlen(newData.projectName) || memcmp(oldData.projectName, newData.projectName, length * sizeof(charType)))
	{
		NSString *projectNameString = getStringForWchar(newData.projectName);

		[projectName setStringValue : projectNameString];
		[projectImage updateProject:projectNameString];
	}

	[header updateProjectDiff:oldData :newData];
}

- (void) selectElem : (uint) projectID : (BOOL) isTome : (int) element
{
	if(coreview != nil)
		[coreview selectElem : projectID : isTome : element];
}

#pragma mark - Suggestion management

enum
{
	DIR_RIGHT	= 0,
	DIR_UP		= 1,
	DIR_LEFT	= 2,
	DIR_DOWN	= 3,
	DIR_OPPOSITE = 2,
	DIR_NB_ELEM = 4,
	DIR_MIDDLE 	= 5
};

- (void) respondToSRFocus : (NSNotification *) notification
{
	if(self.serieViewHidden || mainDetailView == nil)
		return;
	
	NSNumber * _project = notification.object, * _inOrOut = [notification.userInfo objectForKey:SR_FOCUS_IN_OR_OUT];
	
	if(_project == nil || ![_project isKindOfClass:[NSNumber class]] || _inOrOut == nil || ![_inOrOut isKindOfClass:[NSNumber class]])
		return;
	
	PROJECT_DATA project = getElementByID([_project unsignedIntValue]);
	if(!project.isInitialized)
		return;
	
	[CATransaction begin];
	
	if(![_inOrOut boolValue])			//Leaving
	{
		[self changeSRFocus:mainDetailView :[suggestions getContent] : DIR_RIGHT];
	}
	else if([suggestions getContent].alphaValue != 0)		//Suggestion -> Detail view
	{
		mainDetailView.project = project;
		[self changeSRFocus:[suggestions getContent] :mainDetailView : DIR_LEFT];
	}
	else								//Detail view -> New detail view
	{
		tmpDetailView.project = project;
		byte direction = wstrcmp(mainDetailView.project.projectName, project.projectName) > 0 ? DIR_DOWN : DIR_UP;

		[self changeSRFocus:mainDetailView :tmpDetailView : direction];
		
		id tmp = mainDetailView;
		mainDetailView = tmpDetailView;
		tmpDetailView = tmp;
	}
	
	[CATransaction commit];
}

- (void) changeSRFocus : (NSView *) oldView : (NSView *) newView : (byte) direction
{
	if(![NSThread isMainThread])
	{
		dispatch_sync(dispatch_get_main_queue(), ^{
			[self changeSRFocus:oldView :newView :direction];
		});
		return;
	}
	
	[CATransaction begin];
	
	if(newView.superview == nil)
		[self addSubview:newView];
	
	NSPoint base = [suggestions getFrameFromParent : _bounds].origin;
	
	[newView setFrame:(NSRect) {[self newOriginFocus:newView.bounds :(direction + DIR_OPPOSITE) % DIR_NB_ELEM], _bounds.size}];
	newView.alphaValue = 1;
	
	[oldView setFrameOrigin:base];
	
	[CATransaction commit];
	
	[NSAnimationContext beginGrouping];
	
	[oldView.animator setFrameOrigin:[self newOriginFocus:oldView.bounds :direction]];
	[newView.animator setFrameOrigin:base];
	
	if(oldView == [suggestions getContent])		//Completion handler is a bitch :x
		oldView.animator.alphaValue = 0;
	
	[NSAnimationContext endGrouping];
}

- (NSPoint) newOriginFocus : (NSRect) itemBounds : (byte) direction
{
	NSPoint output = NSZeroPoint;
	
	switch (direction)
	{
		case DIR_RIGHT:
			output.x = _bounds.size.width;
			break;
			
		case DIR_UP:
			output.y = _bounds.size.height;
			break;

		case DIR_LEFT:
			output.x = -itemBounds.size.width;
			break;
			
		case DIR_DOWN:
			output.y = -itemBounds.size.height;
			break;
	}
	
	return output;
}

@end


