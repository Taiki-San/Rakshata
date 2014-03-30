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

- (id)initWithFrame:(NSRect)frame : (MANGAS_DATA) project : (bool) isTome
{
    self = [super initWithFrame:frame];
    if (self)
	{
		[Prefs getPref:PREFS_GET_MAIN_THREAD :&mainThread];
		[self setWantsLayer:true];
		self.layer.backgroundColor = [self getBackgroundColor].CGColor;
		self.layer.cornerRadius = 12;
		
		projectName = [[RakTextProjectName alloc] initWithText:[self bounds] : [NSString stringWithUTF8String:project.mangaName] : [Prefs getSystemColor:GET_COLOR_BACKGROUND_TABS]];
		[self addSubview:projectName];
		
		projectImage = [[RakCTProjectImageView alloc] initWithImageName:@"defaultCTBackground" :[self bounds]];
		[self addSubview:projectImage];
		
		coreView = [[RakCTContentTabView alloc] initWithProject : project : isTome : [self bounds]];
		[self addSubview:coreView];
    }
    return self;
}

- (void) setFrame:(NSRect)frameRect
{
	[super setFrame:frameRect];
	
	frameRect.origin.x = frameRect.origin.y = 0;
	
	[projectName setFrame:frameRect];
	[projectImage setFrame:frameRect];
	[coreView setFrame:frameRect];
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
	}
	
	return [Prefs getSystemColor:code];
}

@end

@implementation RakTextProjectName

- (void) additionalDrawing
{
	NSRect frame = self.bounds;

	frame.origin.y = frame.size.height - 2;
	frame.size.height = 2;
	
	[[Prefs getSystemColor:GET_COLOR_INACTIVE] setFill];
	NSRectFill(frame);
}

- (NSRect) getProjectNameSize : (NSRect) superViewSize
{
	NSRect frame = superViewSize;
	frame.size.height = CT_READERMODE_WIDTH_PROJECT_NAME;
	frame.origin.y = superViewSize.size.height - frame.size.height;

	return frame;
}

- (id) initWithText:(NSRect)frame :(NSString *)text :(NSColor *)color
{
	text = [text stringByReplacingOccurrencesOfString:@"_" withString:@" "];
	self = [super initWithText:[self getProjectNameSize:frame] :text :color];
	
	if(self != nil)
	{
		[self setFont:[NSFont fontWithName:@"Helvetica-Bold" size:16]];
		[self setTextColor:[Prefs getSystemColor:GET_COLOR_INACTIVE]];
	}
	
	return self;
}

- (void) setFrame:(NSRect)frameRect
{
	[super setFrame:[self getProjectNameSize:frameRect]];
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

- (id) initWithProject : (MANGAS_DATA) project : (bool) isTome : (NSRect) frame
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
		else if(!isTome)
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
		else
		{
			if(data.nombreChapitre > 0)
				isTome = false;
			else	//Projet illisible
			{
				[self faillure];
				return nil;
			}
		}

		[self addSubview:buttons];
		tableViewControllerChapter = [[RakCTCoreContentView alloc] init:[self frame] : data :false];
		if(tableViewControllerChapter != nil)
		{
			[tableViewControllerChapter setHidden:isTome];
			[tableViewControllerChapter setSuperView:self];
		}
	
		tableViewControllerVolume =  [[RakCTCoreContentView alloc] init:[self frame] : data : true];
		if(tableViewControllerVolume != nil)
		{
			[tableViewControllerVolume setHidden:!isTome];
			[tableViewControllerVolume setSuperView:self];
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
}

- (void) faillure
{
	NSLog(@"Got crappy data D:");
	[buttons removeFromSuperview];
	[self release];
}

- (NSRect) getSizeOfCoreView : (NSRect) superViewFrame
{
	NSRect frame = superViewFrame;
	
	frame.size.height -= CT_READERMODE_HEIGHT_HEADER_TAB;
	
	return frame;
}

@end
