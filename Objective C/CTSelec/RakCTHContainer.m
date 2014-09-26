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
		[self initGradient];
		self.angle = 270;

		[self loadProject : project];
	}
	
	return self;
}

#pragma mark - Interface

- (void) loadProject : (PROJECT_DATA) project
{
	_data = project;
	
}

#pragma mark - UI utilities

- (NSColor *) startColor
{
	return [[Prefs getSystemColor : GET_COLOR_CTHEADER_GRADIENT_START : NO] retain];
}

- (NSColor *) endColor : (const NSColor *) startColor
{
	return [[Prefs getSystemColor : GET_COLOR_CTHEADER_GRADIENT_END : NO] retain];
}

@end
