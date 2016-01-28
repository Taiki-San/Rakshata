/*********************************************************************************************
 **	__________         __           .__            __                 ________   _______   	**
 **	\______   \_____  |  | __  _____|  |__ _____ _/  |______   	___  _\_____  \  \   _  \  	**
 **	 |       _/\__  \ |  |/ / /  ___/  |  \\__  \\   __\__  \  	\  \/ //  ____/  /  /_\  \ 	**
 **	 |    |   \ / __ \|    <  \___ \|   Y  \/ __ \|  |  / __ \_  \   //       \  \  \_/   \	**
 **	 |____|_  /(____  /__|_ \/____  >___|  (____  /__| (____  /	  \_/ \_______ \ /\_____  /	**
 **	        \/      \/     \/     \/     \/     \/          \/ 	              \/ \/     \/ 	**
 **                                                                                         **
 **			This Source Code Form is subject to the terms of the Mozilla Public				**
 **			License, v. 2.0. If a copy of the MPL was not distributed with this				**
 **			file, You can obtain one at http://mozilla.org/MPL/2.0/.						**
 **                                                                                         **
 **                     			© Taiki 2011-2016                                       **
 **                                                                                         **
 *********************************************************************************************/

@implementation RakCTHeaderImage

- (instancetype) initWithData : (NSRect) frame : (PROJECT_DATA) project
{
	self = [self initWithFrame : [self frameByParent:frame]];
	
	if(self != nil)
	{
		[self updateHeaderProjectInternal : getCopyOfProjectData(project)];
		
		if(_background != nil)	//We may need to update our frame
			[self setFrame: frame];
		
		[RakDBUpdate registerForUpdate:self :@selector(DBUpdated:)];
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
	[self setFrameAnimated :  frameRect];
	
	frameRect.origin = NSZeroPoint;
	
	[_background resizeAnimation : frameRect];
	[_container resizeAnimation : frameRect];
}

- (void) DBUpdated : (NSNotification*) notification
{
	if(!_data.isInitialized)
		return;
	
	if([RakDBUpdate analyseNeedUpdateProject:notification.userInfo :_data])
		[self updateHeaderProjectInternal:getProjectByID(_data.cacheDBID)];
}

#pragma mark - Interface

- (void) updateHeaderProject : (PROJECT_DATA) project
{
	PROJECT_DATA copy = getCopyOfProjectData(project);
	
	[NSAnimationContext runAnimationGroup:^(NSAnimationContext *context) {
		
		[context setDuration:CT_HALF_TRANSITION_ANIMATION];
		[self setAlphaAnimated:0.0];
		
	} completionHandler:^{
		
		[self updateHeaderProjectInternal : copy];
		
		[NSAnimationContext runAnimationGroup:^(NSAnimationContext *context) {
			
			[context setDuration:CT_HALF_TRANSITION_ANIMATION];
			[self setAlphaAnimated:1.0];
			
		} completionHandler:^{
			
		}];
	}];
}

- (BOOL) updateHeaderProjectInternal : (PROJECT_DATA) project
{
	BOOL needAddBackgroundGradient = NO;
	
	releaseCTData(_data);
	_data = project;
	
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
		RakView * backgroundGradient = [_background gradientView];
		if(backgroundGradient != nil)
			[self addSubview:backgroundGradient];	//Add subview may just reorder is needed
	}
	
	return YES;
}

#pragma mark - UI utilities

- (NSRect) frameByParent:(NSRect)parentFrame
{
	if(_background != nil && _background.image != nil && _background.image.size.height != 0)		//round(ratio * fullWidth)
	{
		parentFrame.size.width = round((parentFrame.size.height / _background.image.size.height) * _background.image.size.width);
//
//		if(parentFrame.size.width < CT_HEADER_LEFT_MIN_WIDTH)
//		{
//			parentFrame.size.width = CT_HEADER_LEFT_MIN_WIDTH;
//			parentFrame.size.height = (CT_HEADER_LEFT_MIN_WIDTH / _background.image.size.width) * _background.image.size.height;
//		}
	}
//	else
//		parentFrame.size.width = MAX(CT_HEADER_LEFT_MIN_WIDTH, parentFrame.size.width / 2);
	
	parentFrame.origin.x = 1;
	parentFrame.origin.y = 0;
	
	return parentFrame;
}

- (NSSize) sizeByParent : (NSRect) parentFrame
{
	NSSize output = parentFrame.size;
	
	//We'll adapt our size to image's
	if(_background != nil && _background.image != nil)
	{
		NSSize size = _background.image.size;
		if(NSEqualSizes(size, NSZeroSize))
			size = output;
		
		CGFloat ratio = (size.width != 0 ? output.width / size.width : 1 ) / 2;
		
		if(size.width * ratio < CT_HEADER_LEFT_MIN_WIDTH)
		{
			ratio = CT_HEADER_LEFT_MIN_WIDTH / size.width;
		}
		
		//Make the thing bigger is too wide, and no high enough
		if(size.height * ratio > output.height / 2)
			ratio = (output.height / 2) / size.height;
		
		//We scale everything
		output.height = round(size.height * ratio);
		output.width = round(size.width * ratio);
	}
	else
	{
		//If we can't, we take half of the width, and the top 40% of the view
		output.width = MAX(CT_HEADER_LEFT_MIN_WIDTH, output.width / 2);
		output.height = output.height * 2 / 5;
	}
	
	return output;
}

@end
