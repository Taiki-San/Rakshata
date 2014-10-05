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
	self = [self initWithFrame : [self frameByParent:frame]];
	
	if(self != nil)
	{
		[self updateHeaderProjectInternal : project : NO];
		
		if(_background != nil)	//We may need to update our frame
			[self setFrame: frame];
	}
	
	return self;
}

- (void) setFrame:(NSRect)frameRect
{
	frameRect = [self frameByParent:frameRect];
	[super setFrame :  frameRect];
	
	frameRect.origin = NSZeroPoint;
	
	[_background setFrame : frameRect];
	[_container setFrame : frameRect];
}

- (void) resizeAnimation : (NSRect) frameRect
{
	frameRect = [self frameByParent:frameRect];
	[self.animator setFrame :  frameRect];
	
	frameRect.origin = NSZeroPoint;
	
	[_background resizeAnimation : frameRect];
	[_container.animator setFrame : frameRect];
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
	
	BOOL needAddBackgroundGradient = NO;
	projectCacheID = project.cacheDBID;
	
	//Update background image
	if(_background == nil)
	{
		_background = [[RakCTHImage alloc] initWithProject : self.bounds : project];
		[self addSubview:_background];
		
		needAddBackgroundGradient = YES;
	}
	else
		[_background loadProject : project];
	
	if(_container == nil)
	{
		_container = [[RakCTHContainer alloc] initWithProject: self.bounds : project];
		[self addSubview:_container];
		
		needAddBackgroundGradient = YES;
	}
	else
		[_container loadProject : project];
	
	if(needAddBackgroundGradient)
	{
		NSView * backgroundGradient = [_background gradientView];
		if(backgroundGradient != nil)
			[self addSubview:backgroundGradient];	//Add subview may just reorder is needed
	}
	
	return YES;
}

#pragma mark - UI utilities

- (NSRect) frameByParent : (NSRect) parentFrame
{
	//We take half of the width, and the top 40% of the view
	parentFrame.size.width /= 2;
	
	//We'll adapt our size to image's
	if(_background != nil && _background.image != nil)
	{
		NSSize size = _background.image.size;
		CGFloat ratio = parentFrame.size.width / size.width;
		
		//Make the thing bigger is too wide, and no high enough
		if(size.height * ratio > parentFrame.size.height / 2)
			ratio = (parentFrame.size.height / 2) / size.height;
		
		//We scale everything
		size.height = round(size.height * ratio);
		size.width = round(size.width * ratio);
		
		parentFrame.origin.y = parentFrame.size.height - size.height;
		parentFrame.size = size;
	}
	else
	{
		parentFrame.origin.y = parentFrame.size.height * 3 / 5;
		parentFrame.size.height -= parentFrame.origin.y;
	}
	
	return parentFrame;
}

@end
