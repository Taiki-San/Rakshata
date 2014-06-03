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

- (id)initContent:(NSRect)frame : (MANGAS_DATA) project : (bool) isTome : (long [4]) context
{
    self = [super initWithFrame:frame];
    if (self)
	{
		[self setupInternal];
		
		projectName = [[RakTextProjectName alloc] initWithText:[self bounds] : [NSString stringWithUTF8String:project.mangaName] : [Prefs getSystemColor:GET_COLOR_BACKGROUND_TABS]];
		if(projectName != nil)	[self addSubview:projectName];
		
		projectImage = [[RakCTProjectImageView alloc] initWithImageName: [NSString stringWithUTF8String:project.mangaName] : [self bounds]];
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
	
	return [Prefs getSystemColor:code];
}

#pragma mark - Proxy

- (void) updateContext : (MANGAS_DATA) data
{
	if(projectName != nil)
		[projectName setStringValue : [[NSString stringWithUTF8String: data.mangaName] stringByReplacingOccurrencesOfString:@"_" withString:@" "]];
	else
	{
		projectName = [[RakTextProjectName alloc] initWithText:[self bounds] : [NSString stringWithUTF8String:data.mangaName] : [Prefs getSystemColor:GET_COLOR_BACKGROUND_TABS]];
		if(projectName != nil)		[self addSubview:projectName];
	}

	if(projectImage != nil)
		[projectImage updateProject:[NSString stringWithUTF8String: data.mangaName]];
	else
	{
		projectImage = [[RakCTProjectImageView alloc] initWithImageName: [NSString stringWithUTF8String:data.mangaName] : [self bounds]];
		if(projectImage != nil)		[self addSubview:projectImage];
	}
	
	if(coreView != nil)
		[coreView updateContext:data];
	else
	{
		coreView = [[RakCTContentTabView alloc] initWithProject : data : false : [self bounds] : (long [4]) {-1, -1, -1, -1}];
		if(coreView != nil)			[self addSubview:coreView];
	}
}

- (void) refreshCT : (BOOL) checkIfRequired : (uint) ID;
{
	if (coreView != nil)
		[coreView refreshCTData : checkIfRequired : ID];
}

@end

@implementation RakTextProjectName

- (CGFloat) getFontSize
{
	return 16;
}

@end

@implementation RakCTProjectImageView

- (id) initWithImageName : (NSString *) imageName : (NSRect) superViewFrame
{
	NSImage * projectImageBase = [RakResPath craftResNameFromContext:imageName :NO :YES : 1];
	
	if(projectImageBase == nil)
	{
		projectImageBase = [RakResPath craftResNameFromContext:@"defaultCTBackground" :NO :YES : 1];
	}
	
	if(projectImageBase != nil)
	{
		if(projectImageBase.size.height != CT_READERMODE_HEIGHT_PROJECT_IMAGE)
		{
			NSSize imageSize = projectImageBase.size;
			CGFloat ratio = imageSize.height / CT_READERMODE_HEIGHT_PROJECT_IMAGE;
			
			imageSize.height *= ratio;
			imageSize.width *= ratio;
			
			[projectImageBase setSize:imageSize];
		}
		
		self = [super initWithFrame:[self getProjectImageSize: superViewFrame : [projectImageBase size] ] ];
		
		if(self != nil)
		{
			[self setWantsLayer:NO];
			[self setImageAlignment:NSImageAlignCenter];
			[self setImageFrameStyle:NSImageFrameNone];
			[self setImage:projectImageBase];
		}
	}
	else
	{
		[self release];
		self = nil;
	}
	
	return self;
}

- (void) updateProject : (NSString *) imageName
{
	NSImage * projectImageBase = [RakResPath craftResNameFromContext:imageName :NO :YES : 1];
	if(projectImageBase != nil)
	{
		if(projectImageBase.size.height != CT_READERMODE_HEIGHT_PROJECT_IMAGE)
		{
			NSSize imageSize = projectImageBase.size;
			CGFloat ratio = imageSize.height / CT_READERMODE_HEIGHT_PROJECT_IMAGE;
			
			imageSize.height *= ratio;
			imageSize.width *= ratio;
			
			[projectImageBase setSize:imageSize];
		}
		
		[self setImage:projectImageBase];
	}
}

- (NSRect) getProjectImageSize : (NSRect) superViewFrame : (NSSize) imageSize
{
	NSRect frame;
	
	frame.size.height = imageSize.height;
	frame.size.width = imageSize.width;
	
	frame.origin.x = superViewFrame.size.width / 2 - frame.size.width / 2;
	frame.origin.y = superViewFrame.size.height - CT_READERMODE_WIDTH_PROJECT_NAME - CT_READERMODE_WIDTH_SPACE_NEXT_PROJECTIMAGE - imageSize.height;
	
	return frame;
}

- (void) setFrame:(NSRect)frameRect
{
	[super setFrame:[self getProjectImageSize:frameRect :[self image].size]];
}

- (void) resizeAnimation : (NSRect) frameRect
{
	[self.animator setFrame : [self getProjectImageSize: frameRect :[self image].size]];
}

@end

@implementation RakCTContentTabView

- (id) initWithProject : (MANGAS_DATA) project : (bool) isTome : (NSRect) frame : (long [4]) context
{
	if(project.nombreChapitre == 0 && project.nombreTomes == 0)
	{
		[self release];
		return nil;
	}
	
	self = [super initWithFrame:[self getSizeOfCoreView:frame]];
	
	if (self != nil)
	{
		buttons = [[RakCTCoreViewButtons alloc] initWithFrame:[self bounds]];
		[buttons setTarget:self];
		[buttons setAction:@selector(switchIsTome:)];
		
		data = getCopyOfProjectData(project);
		
		if(data.nombreChapitre > 0)
		{
			[buttons setEnabled:true forSegment:0];
			
			if(!isTome)
				[buttons setSelected:true forSegment:0];
			
			if(data.nombreChapitre == 1)
			{
				NSString * name = [buttons labelForSegment:0];
				[buttons setLabel:[name substringToIndex:[name length] - 0] forSegment:0];
			}
		}
		else if(!isTome)	//Si on recoit une demande incohérante
			isTome = true;
		
		if(data.nombreTomes > 0)
		{
			[buttons setEnabled:true forSegment:1];
			
			if(isTome)
				[buttons setSelected:true forSegment:1];
			
			if(data.nombreTomes == 1)
			{
				NSString * name = [buttons labelForSegment:1];
				[buttons setLabel:[name substringToIndex:[name length] - 1] forSegment:1];
			}
		}
		else if(isTome)
		{
			if(data.nombreChapitre > 0)
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
		
		if(data.nombreChapitre > 0)
		{
			tableViewControllerChapter = [[[RakCTCoreContentView alloc] init:[self frame] : data :false : context[0] : context[1]] retain];
			if(tableViewControllerChapter != nil)
			{
				[tableViewControllerChapter setHidden:isTome];
				[tableViewControllerChapter setSuperView:self];
			}
		}

		if(data.nombreTomes > 0)
		{
			tableViewControllerVolume =  [[[RakCTCoreContentView alloc] init:[self frame] : data : true : context[2] : context[3]] retain];
			if(tableViewControllerVolume != nil)
			{
				[tableViewControllerVolume setHidden:!isTome];
				[tableViewControllerVolume setSuperView:self];
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
	
	[self refreshCTData];
	
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
	
	[buttons resizeAnimation:frameRect];
	
	[self refreshCTData];
	
	[tableViewControllerChapter resizeAnimation:coreView];
	[tableViewControllerVolume resizeAnimation:coreView];
}

- (void) failure
{
	NSLog(@"Got crappy data D:");
	[buttons removeFromSuperview];
	[self release];
}

- (NSString *) getContextToGTFO
{
	if(data.team == NULL)
		return nil;
	
	return [NSString stringWithFormat:@"%s\n%s\n%d\n%ld\n%.0f\n%ld\n%.0f", data.team->URL_depot, data.mangaNameShort, [buttons selectedSegment] == 1 ? 1 : 0, (long)[tableViewControllerChapter getSelectedElement], [tableViewControllerChapter getSliderPos], (long)[tableViewControllerVolume getSelectedElement], [tableViewControllerVolume getSliderPos]];
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
	[buttons removeFromSuperview];
	[[tableViewControllerChapter getContent] removeFromSuperviewWithoutNeedingDisplay];
	[tableViewControllerChapter release];
	[[tableViewControllerVolume getContent] removeFromSuperviewWithoutNeedingDisplay];
	[tableViewControllerVolume release];
	
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
	int ID;
	
	if(isTome && index < data.nombreTomes)
		ID = data.tomes[index].ID;
	else if(!isTome && index < data.nombreChapitre)
		ID = data.chapitres[index];
	else
		return;
	
	[RakTabView broadcastUpdateContext:self :data :isTome :ID];
}

- (void) switchIsTome : (RakCTCoreViewButtons*) sender
{
	bool isTome;
	if ([sender selectedSegment] == 0)
		isTome = false;
	else
		isTome = true;
	
	if(tableViewControllerChapter != nil)
		[tableViewControllerChapter setHidden:isTome];
	if(tableViewControllerVolume != nil)
		[tableViewControllerVolume setHidden:!isTome];
}

- (void) refreshCTData : (BOOL) checkIfRequired : (uint) ID;
{
	if(checkIfRequired && data.cacheDBID != ID)
		return;
		
	if(updateIfRequired(&data, RDB_CTXCT))
	{
		checkChapitreValable(&data, NULL);
		[tableViewControllerChapter reloadData : data.nombreChapitre : data.chapitres : NO];
		
		checkTomeValable(&data, NULL);
		[tableViewControllerVolume reloadData : data.nombreTomes : data.tomes : NO];
	}
}

- (void) updateContext : (MANGAS_DATA) newData
{
	//Some danger of TOCTOU around here, mutexes would be great
	
	if(!memcmp(&newData, &data, sizeof(data)))
		return;
	else
		memcpy(&data, &newData, sizeof(data));
	
	updateIfRequired(&data, RDB_CTXCT);
	
	//No data available
	if(data.nombreChapitre == 0 && data.nombreTomes == 0)
	{
		[tableViewControllerChapter setHidden:YES];
		[tableViewControllerVolume setHidden:YES];

		[self failure];
	}
	
	//Update views, create them if required
	if(data.chapitres != NULL)
	{
		checkChapitreValable(&data, NULL);
		
		if(tableViewControllerChapter == nil)
		{
			tableViewControllerChapter =  [[[[RakCTCoreContentView alloc] init:[self frame] : data : false : -1 : -1] retain] retain];	//Two retains because we, as a subview, will get released at the end of the refresh
			[tableViewControllerChapter setSuperView:self];
		}
		else
			[tableViewControllerChapter reloadData : data.nombreChapitre : data.chapitres : YES];
		
		[buttons setEnabled:YES forSegment:0];
	}
	else
		[buttons setEnabled:NO forSegment:0];

	if(data.tomes != NULL)
	{
		checkTomeValable(&data, NULL);
		
		if(tableViewControllerVolume == nil)
		{
			tableViewControllerVolume =  [[[[RakCTCoreContentView alloc] init:[self frame] : data : true : -1 : -1] retain] retain];
			[tableViewControllerVolume setSuperView:self];
		}
		else
			[tableViewControllerVolume reloadData : data.nombreTomes : data.tomes : YES];
		
		[buttons setEnabled:YES forSegment:1];
	}
	else
		[buttons setEnabled:NO forSegment:1];
	
	BOOL isTome = [buttons selectedSegment] == 1;
	
	[tableViewControllerChapter setHidden:isTome];
	[tableViewControllerVolume setHidden:!isTome];
	
	//Update focus
	if(isTome && data.tomes == NULL)
	{
		[tableViewControllerChapter setHidden:NO];
		[tableViewControllerVolume setHidden:YES];
		[buttons setSelectedSegment:0];
	}
	else if(!isTome && data.chapitres == NULL)
	{
		[tableViewControllerVolume setHidden:NO];
		if(![tableViewControllerChapter isHidden])
			[tableViewControllerChapter setHidden:YES];
		[buttons setSelectedSegment:1];
	}
}

@end
