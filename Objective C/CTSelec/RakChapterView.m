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

- (id)initWithFrame:(NSRect)frame
{
    self = [super initWithFrame:frame];
    if (self)
	{
		[Prefs getPref:PREFS_GET_MAIN_THREAD :&mainThread];
		[self setWantsLayer:true];
		self.layer.backgroundColor = [self getBackgroundColor].CGColor;
		self.layer.cornerRadius = 12;
		
		projectName = [[RakTextProjectName alloc] initWithText:[self frame] :@"TestTestTestTestTestTestTestTestTest" : [Prefs getSystemColor:GET_COLOR_BACKGROUND_TABS]];
		[self addSubview:projectName];
		
		projectImage = [[RakCTProjectImageView alloc] initWithImageName:@"defaultCTBackground" :[self frame]];
		[self addSubview:projectImage];
    }
    return self;
}

- (void)drawRect:(NSRect)dirtyRect
{
    [super drawRect:dirtyRect];
    
    // Drawing code here.
}

- (void) setFrame:(NSRect)frameRect
{
	[super setFrame:frameRect];
	[projectName setFrame:frameRect];
	[projectImage setFrame:frameRect];
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
	NSRect frame = self.frame;

	frame.origin.x = 0;
	frame.origin.y = frame.size.height - 1;
	frame.size.height = 2;
	
	[[Prefs getSystemColor:GET_COLOR_INACTIVE] setFill];
	NSRectFill(frame);
}

- (NSRect) getProjectNameSize : (NSRect) superViewSize
{
	NSRect frame = superViewSize;
	frame.size.height = CT_VIEW_READERMORE_WIDTH_PROJECT_NAME;
	frame.origin.x = 0;
	frame.origin.y = superViewSize.size.height - frame.size.height;

	return frame;
}

- (id) initWithText:(NSRect)frame :(NSString *)text :(NSColor *)color
{
	self = [super initWithText:[self getProjectNameSize:frame] :text :color];
	
	if(self != nil)
	{
		[self setFont:[NSFont fontWithName:@"Helvetica" size:16]];
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
	frame.origin.y = superViewFrame.size.height - CT_VIEW_READERMORE_WIDTH_PROJECT_NAME - 25 - imageSize.height;
	
	return frame;
}

- (void) setFrame:(NSRect)frameRect
{
	[super setFrame:[self getProjectImageSize:frameRect :[self image].size]];
}


@end