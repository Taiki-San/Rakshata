/*********************************************************************************************
 **	__________         __           .__            __                 ________   _______   	**
 **	\______   \_____  |  | __  _____|  |__ _____ _/  |______   	___  _\_____  \  \   _  \  	**
 **	 |       _/\__  \ |  |/ / /  ___/  |  \\__  \\   __\__  \  	\  \/ //  ____/  /  /_\  \ 	**
 **	 |    |   \ / __ \|    <  \___ \|   Y  \/ __ \|  |  / __ \__ \   //       \  \  \_/   \	**
 **	 |____|_  /(____  /__|_ \/____  >___|  (____  /__| (____  /	  \_/ \_______ \ /\_____  /	**
 **	        \/      \/     \/     \/     \/     \/          \/ 	              \/ \/     \/ 	**
 **                                                                                         **
 **    Licence propriétaire, code source confidentiel, distribution formellement interdite  **
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
		[self addSubview:projectName];
		
		projectImage = [[RakCTProjectImageView alloc] initWithImageName:@"defaultCTBackground" :[self bounds]];
		[self addSubview:projectImage];
		
		coreView = [[RakCTContentTabView alloc] initWithProject : project : isTome : [self bounds] : context];
		[self addSubview:coreView];
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

- (void) gotClickedTransmitData : (MANGAS_DATA) data : (bool) isTome : (uint) index
{
	[(RakChapterView *) self.superview gotClickedTransmitData: data : isTome : index];
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
			[self setImageAlignment:NSImageAlignCenter];
			[self setImageFrameStyle:NSImageFrameNone];
			[self setImage:projectImageBase];
		}
	}
	else
		self = nil;
	
	return self;
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

@end

@implementation RakCTContentTabView

- (id) initWithProject : (MANGAS_DATA) project : (bool) isTome : (NSRect) frame : (long [4]) context
{
	if(project.nombreChapitre == 0 && project.nombreTomes == 0)
		return nil;
	
	self = [super initWithFrame:[self getSizeOfCoreView:frame]];
	
	if (self != nil)
	{
		buttons = [[RakCTCoreViewButtons alloc] initWithFrame:[self bounds]];
		[buttons setTarget:self];
		[buttons setAction:@selector(switchIsTome:)];
		
		memcpy(&data, &project, sizeof(MANGAS_DATA));
		
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
			tableViewControllerChapter = [[RakCTCoreContentView alloc] init:[self frame] : data :false : context[0] : context[1]];
			if(tableViewControllerChapter != nil)
			{
				[tableViewControllerChapter setHidden:isTome];
				[tableViewControllerChapter setSuperView:self];
			}
		}

		if(data.nombreTomes > 0)
		{
			tableViewControllerVolume =  [[RakCTCoreContentView alloc] init:[self frame] : data : true : context[2] : context[3]];
			if(tableViewControllerVolume != nil)
			{
				[tableViewControllerVolume setHidden:!isTome];
				[tableViewControllerVolume setSuperView:self];
			}
		}
	}
	
	return self;
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

- (void) gotClickedTransmitData : (bool) isTome : (uint) index
{
	[(RakChapterView *) self.superview gotClickedTransmitData: data : isTome : index];
}

- (void) setFrame:(NSRect)frameRect
{
	[super setFrame:[self getSizeOfCoreView:frameRect]];
	[buttons setFrame:[self bounds]];
	
	if(updateIfRequired(&data, RDB_CTXCT))
	{
		checkChapitreValable(&data, NULL);
		[tableViewControllerChapter reloadData : data.nombreChapitre : data.chapitres];
		
		checkTomeValable(&data, NULL);
		[tableViewControllerVolume reloadData : data.nombreTomes : data.tomes];
	}
	
	[tableViewControllerChapter setFrame:[self bounds]];
	[tableViewControllerVolume setFrame:[self bounds]];
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
	[super release];
	[buttons release];
	[tableViewControllerChapter release];
	[tableViewControllerVolume release];
}

- (NSRect) getSizeOfCoreView : (NSRect) superViewFrame
{
	NSRect frame = superViewFrame;
	
	frame.size.height -= CT_READERMODE_HEIGHT_HEADER_TAB;
	
	return frame;
}

@end
