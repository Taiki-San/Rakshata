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

@implementation RakCTHeader

- (id) initWithData : (NSRect) frame : (PROJECT_DATA) project
{
	self = [self initWithFrame:[self frameByParent:frame]];
	
	if(self != nil)
	{
		[self initGradient];
		self.angle = 0;
		[self updateHeaderProjectInternal : project : NO];
	}
	
	return self;
}

#pragma mark - Interface

- (BOOL) updateHeaderProject : (PROJECT_DATA) project
{
	return [self updateHeaderProjectInternal : project : YES];
}

- (BOOL) updateHeaderProjectInternal : (PROJECT_DATA) project : (BOOL) checkReloadNeeded
{
	if(checkReloadNeeded && project.cacheDBID == projectCacheID)
		return NO;
	
	projectCacheID = project.cacheDBID;
	
	//Update background image
	if(_background == nil)
	{
		_background = [[RakCTHImage alloc] initWithProject : self.bounds : project];
		[self addSubview:_background];
	}
	else
		[_background loadProject : project];
	
	if(_container == nil)
	{
		_container = [[RakCTHContainer alloc] initWithProject: self.bounds : project];
		[self addSubview:_container];
	}
	else
		[_container loadProject : project];
	
	return YES;
}

#pragma mark - UI utilities

- (NSColor *) startColor
{
	return [[[[NSApp delegate] CT] getMainColor] retain];
}

- (NSColor *) endColor : (NSColor *) startColor
{
	return [[startColor colorWithAlphaComponent:0] retain];
}

- (NSRect) frameByParent : (NSRect) parentFrame
{
	//We take half of the width, and the top 40% of the view
	parentFrame.size.width /= 2;
	parentFrame.origin.y = parentFrame.size.height * 2 / 5;
	parentFrame.size.height -= parentFrame.origin.y;
	
	return parentFrame;
}

@end
