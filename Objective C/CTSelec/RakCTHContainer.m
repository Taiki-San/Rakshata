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
 *********************************************************************************************/

@implementation RakCTHContainer

- (id) initWithProject : (NSRect) frame : (PROJECT_DATA) project
{
	self = [self initWithFrame:frame];
	
	if(self != nil)
	{
		[Prefs getCurrentTheme:self];
		[self updateGradient];
		[self loadProject : project];
	}
	
	return self;
}

- (void) dealloc
{
	[gradient release];
	
	[super dealloc];
}

#pragma mark - Interface

- (void) loadProject : (PROJECT_DATA) project
{
	_data = project;
	
}

#pragma mark - UI routines

- (void) drawRect:(NSRect)dirtyRect
{
	[gradient drawInRect : self.bounds angle : 270];

	[super drawRect:dirtyRect];
}

#pragma mark - UI utilities

- (void) updateGradient
{
	[gradient release];
	gradient = [[NSGradient alloc] initWithStartingColor : [Prefs getSystemColor : GET_COLOR_CTHEADER_GRADIENT_START : NO] endingColor : [Prefs getSystemColor : GET_COLOR_CTHEADER_GRADIENT_END : NO]];
}


@end
