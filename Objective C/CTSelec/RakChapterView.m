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
		
		
		projectName = [[RakTextProjectName alloc] initWithText:[self getProjectNameSize] :@"\nTestTestTestTestTestTestTestTestTest" : [Prefs getSystemColor:GET_COLOR_BACKGROUND_TABS]];
		[projectName setTextColor:[Prefs getSystemColor:GET_COLOR_INACTIVE]];
		[self addSubview:projectName];
    }
    return self;
}

- (NSRect) getProjectNameSize
{
	NSRect frame = [self frame];
	frame.size.height = 40;
	frame.origin.x = 0;
	frame.origin.y = self.frame.size.height - frame.size.height;
	
	return frame;
}

- (void)drawRect:(NSRect)dirtyRect
{
    [super drawRect:dirtyRect];
    
    // Drawing code here.
}

- (void) setFrame:(NSRect)frameRect
{
	[super setFrame:frameRect];
	[projectName setFrame:[self getProjectNameSize]];
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

@end