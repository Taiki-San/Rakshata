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

- (instancetype) initContent : (NSRect)frame : (PROJECT_DATA) project : (BOOL) isTome : (long [4]) context
{
	self = [super initWithFrame:frame];
	if(self != nil)
	{
		[self setupInternal];
		
		[self initCTView : project : mainThread == TAB_CT];
		
		coreview = [[RakCTSelection alloc] initWithProject : project : isTome : _bounds : (header != nil ? header.bounds.size.height : 0) : context : mainThread];
		if(coreview != nil)
		{
			[self addSubview:coreview];
			
			if(mainThread == TAB_SERIES)
			{
				coreview.alphaValue = 0;
				coreview.hidden = YES;
			}
		}
		
		[self initSerieView : project : mainThread == TAB_SERIES];
		[self initReaderView : project : mainThread == TAB_READER];
		
		[[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(respondToSRFocus:) name:SR_NOTIFICATION_FOCUS object:nil];
	}
	return self;
}

- (NSString *) getContextToGTFO
{
	if(coreview == nil)
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
	
	[coreview setFrame : newBound : headerHeight];
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
	if(mainDetailView != nil)
		mainDetailView.offsetX = [suggestions getFrameFromParent:_bounds].origin.x;
	
	tmpDetailView = [[RakSRDetails alloc] initWithFrame:_bounds];
	if(tmpDetailView != nil)
		tmpDetailView.offsetX = [suggestions getFrameFromParent:_bounds].origin.x;
	
	[super setSerieViewHidden:!serieMode];
}

- (void) initCTView : (PROJECT_DATA) project : (BOOL) CTMode
{
	header = [[RakCTHeader alloc] initWithData : self.bounds : project];
	if(header != nil)
	{
		[self addSubview:header];
		
		if(!CTMode)
		{
			header.alphaValue = 0;
			header.hidden = YES;
		}
	}
	
	[super setCTViewHidden:!CTMode];
}

- (void) initReaderView : (PROJECT_DATA) project : (BOOL) readerMode
{
	projectName = [[RakMenuText alloc] initWithText:_bounds : project.isInitialized ? getStringForWchar(project.projectName) : @""];
	if(projectName != nil)	[self addSubview:projectName];
	
	if(project.isInitialized)
		projectImage = [[RakCTProjectImageView alloc] initWithImageName: project : _bounds];
	else
		projectImage = [[RakCTProjectImageView alloc] initWithImageName: getEmptyProject() : _bounds];
	
	if(projectImage != nil)
		[self addSubview:projectImage];
	
	if(!readerMode)
	{
		projectName.alphaValue = 0;		projectName.hidden = YES;
		projectImage.alphaValue = 0;	projectImage.hidden = YES;
	}
	
	[self setReaderViewHidden:!readerMode];
}

- (void) setSerieViewHidden : (BOOL) serieViewHidden
{
	if(serieViewHidden)
	{
		[suggestions getContent].animator.alphaValue = 0;
		if(mainDetailView != nil && mainDetailView.superview != nil)
			mainDetailView.animator.alphaValue = 0;
		
		coreview.hidden = NO;
		coreview.animator.alphaValue = 1;
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
	for(NSView * view in @[header, projectName, projectImage, coreview])
	{
		if(view.alphaValue == 0)
			view.hidden = YES;
	}
	
	if(self.serieViewHidden && [suggestions getContent].alphaValue == 0)
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
			code = COLOR_BACKGROUND_COREVIEW;
			break;
		}
			
		case TAB_CT:
		{
			code = COLOR_BACKGROUND_TABS;
			break;
		}
			
		default:
			return [NSColor clearColor];
	}
	
	return [Prefs getSystemColor:code];
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
		[projectImage updateProject:data];
	else
	{
		projectImage = [[RakCTProjectImageView alloc] initWithImageName: data : _bounds];
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
		coreview = [[RakCTSelection alloc] initWithProject : data : NO : self.bounds : (header != nil ? header.bounds.size.height : 0) : (long [4]) {-1, -1, -1, -1} : mainThread];
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
		[projectImage updateProject:newData];
	}
	
	[header updateProjectDiff:oldData :newData];
}

- (void) selectElem : (uint) projectID : (BOOL) isTome : (uint) element
{
	if(coreview != nil)
		[coreview selectElem : projectID : isTome : element];
}

- (PROJECT_DATA) activeProject
{
	return coreview.currentProject;
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
	
	PROJECT_DATA project = getProjectByID([_project unsignedIntValue]);
	if(!project.isInitialized)
		return;
	
	[CATransaction begin];
	
	if(![_inOrOut boolValue])			//Leaving
	{
		//At one point, because of a bug, suggestion would be hidden, as the test is cheap, we keep it in place if the issue happen again
		if(suggestions.isHidden)
		{
#ifdef EXTENSIVE_LOGGING
			NSLog(@"Suggestions was hidden, probably a bug");
#endif
			[suggestions getContent].alphaValue = 0;
			suggestions.hidden = NO;
		}
		
		[self changeSRFocus:mainDetailView :suggestions : DIR_RIGHT];
	}
	else if([suggestions getContent].alphaValue != 0)		//Suggestion -> Detail view
	{
		mainDetailView.project = project;
		[self changeSRFocus:suggestions :mainDetailView : DIR_LEFT];
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

//RakList is more restrictive than NSView, so if it support the call, NSView will
- (void) changeSRFocus : (id) oldView : (id) newView : (byte) direction
{
	if(![NSThread isMainThread])
	{
		dispatch_sync(dispatch_get_main_queue(), ^{
			[self changeSRFocus:oldView :newView :direction];
		});
		return;
	}
	
	//Set the positions of the items
	[CATransaction begin];
	
	if(newView != suggestions && ((NSView *)newView).superview == nil)
		[self addSubview:newView];
	
	[newView setFrame:(NSRect) {[self newOriginFocus:newView :(direction + DIR_OPPOSITE) % DIR_NB_ELEM], _bounds.size}];
	if(newView == suggestions)
		[newView getContent].alphaValue = 1;
	
	[oldView setFrame: _bounds];
	
	[CATransaction commit];
	
	//Actual animation
	[NSAnimationContext beginGrouping];
	
	[(RakList *) oldView resizeAnimation: (NSRect) {[self newOriginFocus:oldView :direction], ((RakList *)oldView).frame.size}];
	[(RakList *) newView resizeAnimation: _bounds];
	
	if(oldView == suggestions)		//Completion handler is a bitch :x
		[oldView getContent].animator.alphaValue = 0;
	
	[NSAnimationContext endGrouping];
}

- (NSPoint) newOriginFocus : (NSView *) item : (byte) direction
{
	NSRect itemBounds = item.frame;
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


