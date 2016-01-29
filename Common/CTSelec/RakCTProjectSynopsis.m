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
 **			file, You can obtain one at https://mozilla.org/MPL/2.0/.						**
 **                                                                                         **
 **                     			© Taiki 2011-2016                                       **
 **                                                                                         **
 ********************************************************************************************/

@implementation RakCTProjectSynopsis

- (instancetype) initWithProject : (PROJECT_DATA) project : (NSRect) frame : (NSSize) headerSize
{
	self = [self initWithFrame : [self frameFromParent : frame : headerSize]];
	
	if(self != nil)
	{
		self.wantsLayer = YES;
		self.layer.cornerRadius = 4;
		[Prefs registerForChange:self forType:KVO_THEME];
		
		_title = [[RakMenuText alloc] initWithText : _bounds : NSLocalizedString(@"CT-SYNOPSIS", nil)];
		if(_title != nil)
		{
			[_title sizeToFit];
			_title.ignoreInternalFrameMagic = YES;
			_title.drawGradient = YES;
			_title.barWidth = 1;
			_title.widthGradient = 0.4f;
			
			[_title setFrame : [self frameForTitle:_bounds : _title.bounds.size.height]];
			[_title setAlignment : NSTextAlignmentRight];
			[self addSubview:_title];
		}
		
		if([self setStringToSynopsis : getStringForWchar(project.description)])
		{
			[self updateFrame : frame : headerSize : NO];
		}
	}
	
	return self;
}

- (void) updateProject : (PROJECT_DATA) project
{
	if(project.isInitialized)
		[self updateSynopsis:project.description];
	else
	{
		wchar_t empty = 0;
		[self updateSynopsis:&empty];
	}
}

- (BOOL) postProcessScrollView
{
	return [self generatedScrollView : [self frameForContent : self.bounds : _title != nil ? _title.bounds.size.height : 0]];
}

- (void) dealloc
{
	[Prefs deRegisterForChange:self forType:KVO_THEME];
}

#pragma mark - Resize

- (void) setFrame : (NSRect) frameRect : (NSSize) headerSize
{
	[self updateFrame : frameRect : headerSize : NO];
}

- (void) resizeAnimation : (NSRect) frameRect : (NSSize) headerSize
{
	[self updateFrame : frameRect : headerSize : YES];
}

- (void) updateFrame : (NSRect) frame : (NSSize) headerSize : (BOOL) animated
{
	NSRect mainFrame = [self frameFromParent:frame : headerSize];
	const CGFloat titleHeight = _title.bounds.size.height;
	
	[self _updateFrame:mainFrame :animated];
	
	const NSRect titleFrame = [self frameForTitle : mainFrame : titleHeight];
	
	//Update content frames
	NSRect scrollviewRect = [self frameForContent : mainFrame : titleHeight];
	
	if(animated)
	{
		[_title setFrameAnimated:titleFrame];
		
		[_scrollview setFrameAnimated: scrollviewRect];
		if(placeholderString)
			[_placeholder setFrameOriginAnimated : [self placeholderOrigin : scrollviewRect]];
	}
	else
	{
		[_title setFrame:titleFrame];
		
		[_scrollview setFrame: scrollviewRect];
		if(placeholderString)
		{
			[_placeholder setFrameOrigin : [self placeholderOrigin : scrollviewRect]];
		}
	}
	
	[self updateScrollViewState];
}

#pragma mark - Position routines

- (NSRect) frameForTitle : (NSRect) mainBounds : (CGFloat) height
{
	mainBounds.origin.x = 0;
	mainBounds.size.height = height;
	
	return mainBounds;
}

- (NSRect) frameForContent : (NSRect) mainBounds : (CGFloat) titleHeight
{
	mainBounds = [self frameForContent : mainBounds];
	mainBounds.origin.y = titleHeight + SYNOPSIS_SPACING;
	mainBounds.size.height -= titleHeight + SYNOPSIS_SPACING;
	
	return mainBounds;
}

- (NSRect) frameFromParent : (NSRect) parentFrame : (NSSize) image
{
	parentFrame = [self frameFromParent:parentFrame];
	
	parentFrame.origin.y = 0;
	parentFrame.origin.x = image.width;
	parentFrame.size.width -= image.width + SYNOPSIS_BORDER;
	
	return parentFrame;
}

#pragma mark - Property

- (CGFloat) titleHeight
{
	return _title.bounds.size.height + SYNOPSIS_TOP_BORDER_WIDTH;
}

@end
