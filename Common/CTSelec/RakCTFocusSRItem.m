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
 *********************************************************************************************/

enum
{
	BORDER_STARS = 3
};

@implementation RakCTFocusSRItem

- (void) initContent
{
	stars = [[RakStarView alloc] init:_project];
	if(stars != nil)
		[self addSubview:stars];

	[super initContent];
}

#pragma mark - Sizing

- (NSPoint) resizeContent : (NSSize) newSize : (BOOL) animated
{
	NSPoint previousOrigin = [super resizeContent:newSize :animated];
	
	if(animated)
		[stars setFrameOriginAnimated:	(previousOrigin = [self originOfStars : _workingArea : previousOrigin])];
	else
		[stars setFrameOrigin:			(previousOrigin = [self originOfStars : _workingArea : previousOrigin])];
	
	return previousOrigin;
}

- (NSPoint) reloadOrigin
{
	NSPoint previousOrigin = [super reloadOrigin];
	
	[stars setFrameOrigin:				(previousOrigin = [self originOfStars : _workingArea : previousOrigin])];
	
	return previousOrigin;
}


- (NSPoint) originOfStars : (NSRect) frameRect : (NSPoint) tagOrigin
{
	NSPoint center = NSCenteredRect(frameRect, stars.bounds);
	
	center.y = tagOrigin.y - BORDER_STARS - stars.bounds.size.height;
	
	return center;
}

- (CGFloat) getMinimumHeight
{
	return [super getMinimumHeight] + stars.bounds.size.height + BORDER_STARS;
}

- (void) drawRect:(NSRect)dirtyRect
{
	[super drawRect:dirtyRect];
	
	if(!_last)
	{
		dirtyRect = _workingArea;
		dirtyRect.size.height = 1;
		
		[[self borderColor] setFill];
		NSRectFill(dirtyRect);
	}
}

#pragma mark - Clic management

- (void) mouseDown:(NSEvent *)theEvent
{
	if(_table != nil)
		_table.clickedID = _project.cacheDBID;
}

- (void) mouseUp:(NSEvent *)theEvent
{
	if(_table == nil || _table.clickedID != _project.cacheDBID)
		return;
	
	[super mouseUp:theEvent];
}

@end
