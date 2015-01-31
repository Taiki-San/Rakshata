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

static NSSize workingSize = {RCVC_MINIMUM_WIDTH, RCVC_MINIMUM_HEIGHT};

@implementation RakCollectionViewItem

- (instancetype) initWithProject : (PROJECT_DATA) project
{
	self = [self initWithFrame:NSMakeRect(0, 0, RCVC_MINIMUM_WIDTH, RCVC_MINIMUM_HEIGHT)];
	
	if(self != nil)
	{
		//We really don't care about those data, so we don't want the burden of having to update them
		project.chapitresFull = project.chapitresInstalled = NULL;
		project.tomesFull = project.tomesInstalled = NULL;
		project.chapitresPrix = NULL;
		
		_project = project;
		[self initContent];
	}
	
	return self;
}

- (void) initContent
{
	name = [[RakText alloc] initWithText :getStringForWchar(_project.projectName) : [Prefs getSystemColor:GET_COLOR_INACTIVE :nil]];
	if(name != nil)
	{
		name.alignment = NSCenterTextAlignment;
		
		[name.cell setWraps : YES];
		name.fixedWidth = RCVC_MINIMUM_WIDTH * 0.8;
		
		[self addSubview:name];
	}
}

#pragma mark - Resizing code

- (void) setFrame:(NSRect)frameRect
{
	[super setFrame:frameRect];
	
	//We update the frame
	frameRect.origin = NSCenterSize(frameRect.size, workingSize);
	frameRect.size = workingSize;
	
	//We resize our content
	NSPoint newPoint = NSCenteredRect(frameRect, name.bounds);
	[name setFrameOrigin: newPoint];
}

- (void) drawRect:(NSRect)dirtyRect
{
	[[NSColor blackColor] setFill];
	
	NSRect printArea = {NSCenterSize(dirtyRect.size, workingSize), workingSize};
	
	NSRectFill(printArea);
}

@end
