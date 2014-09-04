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
		
		projectName = [[RakTextProjectName alloc] initWithText:[self bounds] : [[[NSString alloc] initWithData:[NSData dataWithBytes:project.projectName length:sizeof(project.projectName)] encoding:NSUTF32LittleEndianStringEncoding] autorelease]];
		if(projectName != nil)	[self addSubview:projectName];
		
		projectImage = [[RakCTProjectImageView alloc] initWithImageName: project.team->URLRepo : [NSString stringWithFormat:@"%d_CT", project.projectID] : [self bounds]];
		if(projectImage != nil)	[self addSubview:projectImage];
		
		coreView = [[RakCTContentTabView alloc] initWithProject : project : isTome : [self bounds] : context];
		if(coreView != nil)		[self addSubview:coreView];
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

#pragma mark - Color

- (NSColor*) getBackgroundColor
{
	byte code;
	switch (mainThread & GUI_THREAD_MASK)
	{
		case GUI_THREAD_READER:
		{
			code = GET_COLOR_BACKGROUD_CT_READERMODE;
			break;
		}
			
		case GUI_THREAD_CT:
		{
			code = GET_COLOR_BACKGROUD_CT_READERMODE;
			break;
		}
			
		case GUI_THREAD_SERIES:
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
		projectImage = [[RakCTProjectImageView alloc] initWithImageName: data.team->URLRepo : [NSString stringWithFormat:@"%d_CT", data.projectID] : [self bounds]];
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

@implementation RakCTContentTabView

- (id) initWithProject : (PROJECT_DATA) project : (bool) isTome : (NSRect) frame : (long [4]) context
{
	if(project.nombreChapitreInstalled == 0 && project.nombreTomesInstalled == 0)
	{
		[self release];
		return nil;
	}
	
	self = [super initWithFrame:[self getSizeOfCoreView:frame]];
	
	if (self != nil)
	{
		[self setAutoresizesSubviews:NO];
		buttons = [[[RakCTCoreViewButtons alloc] initWithFrame:[self bounds]] retain];
		
		data = getCopyOfProjectData(project);
		
		if(data.nombreChapitreInstalled > 0)
		{
			[buttons setEnabled:true forSegment:0];
			
			if(!isTome)
				[buttons setSelected:true forSegment:0];
			
			if(data.nombreChapitreInstalled == 1)
			{
				NSString * name = [buttons labelForSegment:0];
				[buttons setLabel:[name substringToIndex:[name length] - 0] forSegment:0];
			}
		}
		else if(!isTome)	//Si on recoit une demande incohérante
			isTome = true;
		
		if(data.nombreTomesInstalled > 0)
		{
			[buttons setEnabled:true forSegment:1];
			
			if(isTome)
				[buttons setSelected:true forSegment:1];
			
			if(data.nombreTomesInstalled == 1)
			{
				NSString * name = [buttons labelForSegment:1];
				[buttons setLabel:[name substringToIndex:[name length] - 1] forSegment:1];
			}
		}
		else if(isTome)
		{
			if(data.nombreChapitreInstalled > 0)
			{
				[buttons setSelected:true forSegment:0];
				isTome = false;
			}
			else	//Projet illisible
			{
				[self failure];
				return nil;
			}
		}

		[self addSubview:buttons];
		
		if(data.nombreChapitreInstalled > 0)
		{
			tableViewControllerChapter = [[[RakCTCoreContentView alloc] init:[self frame] : data :false : context[0] : context[1]] retain];
			if(tableViewControllerChapter != nil)
			{
				tableViewControllerChapter.hidden = isTome;
				tableViewControllerChapter.superview = self;
			}
		}

		if(data.nombreTomesInstalled > 0)
		{
			tableViewControllerVolume =  [[[RakCTCoreContentView alloc] init:[self frame] : data : true : context[2] : context[3]] retain];
			if(tableViewControllerVolume != nil)
			{
				tableViewControllerVolume.hidden = !isTome;
				tableViewControllerVolume.superview = self;
			}
		}
	}
	
	return self;
}

- (void) setFrame:(NSRect)frameRect
{
	NSRect coreView = [self getSizeOfCoreView:frameRect], bounds = self.bounds;
	
	if(NSEqualRects(self.frame, coreView))
		return;

	[super setFrame:coreView];
	coreView.origin.x = coreView.origin.y = 0;
	
	if(NSEqualRects(coreView, bounds))
	   return;
	
	[buttons setFrame:[self bounds]];
	
	[self refreshCTData : NO : 0];
	
	[tableViewControllerChapter setFrame:[self bounds]];
	[tableViewControllerVolume setFrame:[self bounds]];
}

- (void) resizeAnimation : (NSRect) frameRect
{
	NSRect coreView = [self getSizeOfCoreView:frameRect], bounds = self.bounds;
	
	if(NSEqualRects(self.frame, coreView))
		return;
	
	[self.animator setFrame:coreView];
	
	if(NSEqualRects(self.bounds, bounds))
		return;
	
	[buttons resizeAnimation:coreView];
	
	[self refreshCTData : NO : 0];
	
	[tableViewControllerChapter resizeAnimation:coreView];
	[tableViewControllerVolume resizeAnimation:coreView];
}

- (void) failure
{
	NSLog(@"Got crappy data D:");
	[buttons removeFromSuperview];
	
	if(self.superview != nil)
		[self removeFromSuperview];
}

- (NSString *) getContextToGTFO
{
	if(data.team == NULL)
		return nil;
	
	return [NSString stringWithFormat:@"%s\n%d\n%d\n%ld\n%.0f\n%ld\n%.0f", data.team->URLRepo, data.projectID, [buttons selectedSegment] == 1 ? 1 : 0, (long)[tableViewControllerChapter getSelectedElement], [tableViewControllerChapter getSliderPos], (long)[tableViewControllerVolume getSelectedElement], [tableViewControllerVolume getSliderPos]];
}

- (id) retain
{
	[buttons retain];
	[tableViewControllerChapter retain];
	[tableViewControllerVolume retain];
	return [super retain];
}

- (oneway void) release
{
	[buttons release];
	[tableViewControllerChapter release];
	[tableViewControllerVolume release];
	[super release];
}

- (void) dealloc
{
	[buttons removeFromSuperview];	[buttons release];
	
	[[tableViewControllerChapter getContent] removeFromSuperviewWithoutNeedingDisplay];
	[[tableViewControllerChapter getContent] release];	[tableViewControllerChapter release];
	
	[[tableViewControllerVolume getContent] removeFromSuperviewWithoutNeedingDisplay];
	[[tableViewControllerVolume getContent] release];	[tableViewControllerVolume release];
	
	releaseCTData(data);
	
	[super dealloc];
}

- (NSRect) getSizeOfCoreView : (NSRect) superViewFrame
{
	NSRect frame = superViewFrame;
	
	frame.size.height -= CT_READERMODE_HEIGHT_HEADER_TAB;
	
	return frame;
}

#pragma mark - Proxy

- (void) gotClickedTransmitData : (bool) isTome : (uint) index
{
	if(self.dontNotify)
		return;
		
	int ID;
	
	if(isTome && index < data.nombreTomesInstalled)
		ID = data.tomesInstalled[index].ID;
	else if(!isTome && index < data.nombreChapitreInstalled)
		ID = data.chapitresInstalled[index];
	else
		return;
	
	self.dontNotify = YES;

	[RakTabView broadcastUpdateContext:self :data :isTome :ID];
	
	self.dontNotify = NO;
}

- (void) feedAnimationController : (RakCTAnimationController *) animationController
{
	[animationController addCTContent: tableViewControllerChapter : tableViewControllerVolume];
	[animationController addAction:self :@selector(switchIsTome:)];
}

- (void) switchIsTome : (RakCTCoreViewButtons*) sender
{
	bool isTome;
	if ([sender selectedSegment] == 0)
		isTome = false;
	else
		isTome = true;
	
	if(tableViewControllerChapter != nil)
		tableViewControllerChapter.hidden = isTome;
	if(tableViewControllerVolume != nil)
		tableViewControllerVolume.hidden = !isTome;
}

- (void) refreshCTData : (BOOL) checkIfRequired : (uint) ID;
{
	if((checkIfRequired || data.cacheDBID != ID) && (!checkIfRequired || !updateIfRequired(&data, RDB_CTXCT)))
		return;

	getUpdatedChapterList(&data, true);
	[tableViewControllerChapter reloadData : data : data.nombreChapitreInstalled : data.chapitresInstalled : NO];
		
	getUpdatedTomeList(&data, true);
	[tableViewControllerVolume reloadData : data : data.nombreTomesInstalled : data.tomesInstalled : NO];
}

- (void) selectElem : (uint) projectID : (BOOL) isTome : (int) element
{
	if(data.cacheDBID != projectID)
		return;
	
	RakCTCoreContentView * tab = nil;
	
	if(isTome)
		tab = tableViewControllerVolume;
	else if(!isTome)
		tab = tableViewControllerChapter;

	if(tab != nil)
	{
		int row = [tab getIndexOfElement:element];
		[tab selectRow:row];
		[tab jumpScrollerToRow:row];
	}
}

- (BOOL) updateContext : (PROJECT_DATA) newData
{
	//Some danger of TOCTOU around here, mutexes would be great
	
	if(data.cacheDBID == newData.cacheDBID)
	{
		getUpdatedCTList(&data, true);
		getUpdatedCTList(&data, false);
	}
	else
	{
		if(tableViewControllerChapter != NULL)	[tableViewControllerChapter resetSelection:nil];
		if(tableViewControllerVolume != NULL)	[tableViewControllerVolume resetSelection:nil];

		releaseCTData(data);
		data = getCopyOfProjectData(newData);
	}
	
	if(!updateIfRequired(&data, RDB_CTXCT))
	{
		getUpdatedChapterList(&data, true);
		getUpdatedTomeList(&data, true);
	}
	
	//No data available
	if(data.nombreChapitreInstalled == 0 && data.nombreTomesInstalled == 0)
	{
		tableViewControllerChapter.hidden = YES;
		tableViewControllerVolume.hidden = YES;

		[self failure];
		return NO;
	}
	
	//Update views, create them if required
	if(data.nombreChapitreInstalled)
	{
		if(tableViewControllerChapter == nil)
		{
			tableViewControllerChapter =  [[[[RakCTCoreContentView alloc] init:[self frame] : data : false : -1 : -1] retain] retain];	//Two retains because we, as a subview, will get released at the end of the refresh
			tableViewControllerChapter.superview = self;
		}
		else
			[tableViewControllerChapter reloadData : data : data.nombreChapitreInstalled : data.chapitresInstalled : YES];
		
		[buttons setEnabled:YES forSegment:0];
	}
	else
	{
		[buttons setEnabled:NO forSegment:0];
	}

	if(data.nombreTomesInstalled)
	{
		if(tableViewControllerVolume == nil)
		{
			tableViewControllerVolume =  [[[[RakCTCoreContentView alloc] init:[self frame] : data : true : -1 : -1] retain] retain];
			tableViewControllerVolume.superview = self;
		}
		else
			[tableViewControllerVolume reloadData : data : data.nombreTomesInstalled : data.tomesInstalled : YES];
		
		[buttons setEnabled:YES forSegment:1];
	}
	else
	{
		[buttons setEnabled:NO forSegment:1];
	}
	
	BOOL isTome = [buttons selectedSegment] == 1;
	
	[tableViewControllerChapter setHidden:isTome];
	[tableViewControllerVolume setHidden:!isTome];
	
	//Update focus
	if(isTome && data.nombreTomesInstalled == 0)
	{
		[tableViewControllerChapter setHidden:NO];
		[tableViewControllerVolume setHidden:YES];
		[buttons setSelectedSegment:0];
	}
	else if(!isTome && data.nombreChapitreInstalled == 0)
	{
		[tableViewControllerVolume setHidden:NO];
		if(![tableViewControllerChapter isHidden])
			[tableViewControllerChapter setHidden:YES];
		[buttons setSelectedSegment:1];
	}
	
	return YES;
}

@end
