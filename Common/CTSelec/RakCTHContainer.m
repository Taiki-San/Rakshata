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

#define BORDER_BETWEEN_NAME_AND_TABLE 10

@implementation RakCTHContainer

- (instancetype) initWithProject : (NSRect) frame : (PROJECT_DATA) project
{
	self = [self initWithFrame : [self frameFromParent : frame]];
	
	if(self != nil)
	{
		[self initGradient];
		self.gradientMaxWidth = frame.size.height;
		self.gradientWidth = 100;
		self.angle = 270;
		self.autoresizesSubviews = NO;
		
		[self loadProject : project];
	}
	
	return self;
}

- (BOOL) isFlipped
{
	return YES;
}

- (void) setFrame : (NSRect) frameRect
{
	frameRect = [self frameFromParent : frameRect];
	lastKnownHeight = frameRect.size.height;
	
	[super setFrame : frameRect];
	
	self.gradientMaxWidth = frameRect.size.height;
	
	[_tableController setFrame : self.bounds];
	CGFloat tableControllerBaseX = _tableController.baseX, oldNameHeight = projectName.bounds.size.height, oldAuthorHeight = authorName.bounds.size.height;
	
	[projectName setFrameOrigin:[self projectNamePos : self.bounds.size]];
	projectName.fixedWidth = tableControllerBaseX - projectName.frame.origin.x - BORDER_BETWEEN_NAME_AND_TABLE;
	
	if(projectName.bounds.size.height != oldNameHeight)	//fixedWidth may warp the lines
		[projectName setFrameOrigin:[self projectNamePos : self.bounds.size]];
	
	[authorName setFrameOrigin:[self authorNamePos : self.bounds.size]];
	authorName.fixedWidth = tableControllerBaseX - authorName.frame.origin.x - BORDER_BETWEEN_NAME_AND_TABLE;
	
	if(authorName.bounds.size.height != oldAuthorHeight)	//fixedWidth may warp the lines
		[authorName setFrameOrigin:[self authorNamePos : self.bounds.size]];
}

- (void) resizeAnimation : (NSRect) frameRect
{
	frameRect = [self frameFromParent : frameRect];
	lastKnownHeight = frameRect.size.height;
	
	[self setFrameAnimated : frameRect];
	frameRect.origin = NSZeroPoint;
	
	self.gradientMaxWidth = frameRect.size.height;
	
	NSPoint nameOrigin = [self projectNamePos : frameRect.size], authorOrigin = [self authorNamePos : frameRect.size];
	CGFloat tableControllerBaseX = [_tableController rawBaseX:frameRect];
	
	projectName.fixedWidth = tableControllerBaseX - nameOrigin.x - BORDER_BETWEEN_NAME_AND_TABLE;
	authorName.fixedWidth = tableControllerBaseX - authorOrigin.x - BORDER_BETWEEN_NAME_AND_TABLE;
	
	[projectName setFrameOriginAnimated:[self projectNamePos : frameRect.size]];
	[authorName setFrameOriginAnimated:[self authorNamePos : frameRect.size]];
	
	[_tableController resizeAnimation : frameRect];
}

#pragma mark - Interface

- (void) loadProject : (PROJECT_DATA) project
{
	_data = project;
	
	BOOL needProcessName = NO, needProcessAuthor = NO;
	NSString * currentElem = _data.isInitialized ? getStringForWchar(_data.projectName) : @"";
	
	//Project name
	if(projectName == nil)
	{
		projectName = [[RakText alloc] initWithText : self.bounds : currentElem : [self textColor]];
		if(projectName != nil)
		{
			[projectName setAlignment:NSTextAlignmentLeft];
			[projectName.cell setWraps:YES];
			needProcessName = YES;
			
			[projectName setFont : [Prefs getFont:FONT_TITLE ofSize: 18]];
			projectName.fixedWidth = projectName.fixedWidth;	//Will refresh our width
			[projectName setFrameOrigin : [self projectNamePos : self.bounds.size]];
			
			[self addSubview: projectName];
		}
	}
	else if(![currentElem isEqualToString:projectName.stringValue])
	{
		[projectName setStringValue : currentElem];
		[projectName setFrameOrigin : [self projectNamePos : self.bounds.size]];
	}
	
	currentElem = _data.isInitialized ? getStringForWchar(_data.authorName) : @"";
	
	//Author name
	if(authorName == nil)
	{
		authorName = [[RakClickableText alloc] initWithText : currentElem :[self textColor] responder: self];
		if(authorName != nil)
		{
			authorName.URL = @"";
			
			[authorName setAlignment:NSTextAlignmentLeft];
			[authorName.cell setWraps:YES];
			needProcessAuthor = YES;
			
			authorName.font = [Prefs getFont:FONT_AUTHOR_ITALIC ofSize:13];
			authorName.fixedWidth = authorName.fixedWidth;	//Will refresh our width
			[authorName setFrameOrigin : [self authorNamePos : self.bounds.size]];
			
			[self addSubview: authorName];
		}
	}
	else if(![currentElem isEqualToString:authorName.stringValue])
	{
		[authorName setStringValue : currentElem];
		authorName.fixedWidth = authorName.fixedWidth;	//Will refresh our width
		[authorName setFrameOrigin : [self authorNamePos : self.bounds.size]];
	}
	
	if(_tableController == nil)
	{
		_tableController = [[RakCTHTableController alloc] initWithProject : _data frame : self.bounds];
		
		if(_tableController != nil)
		{
			if(_tableController.scrollView != nil)
				[self addSubview:_tableController.scrollView];
			else
				_tableController = nil;
		}
	}
	else
	{
		[_tableController updateProject:_data];
		
		NSRect bounds = self.bounds;
		if(bounds.size.height != lastKnownHeight)	//May happend during animation
			bounds.size = [self frameFromParent:self.superview.bounds].size;
		
		[_tableController setFrame : bounds];	//We refresh scrollview size
	}
	
	if(_tableController != nil && (needProcessAuthor || needProcessName))
	{
		CGFloat tableControllerBaseX = _tableController.baseX, oldHeight;
		
		if(needProcessName)
		{
			oldHeight = projectName.bounds.size.height;
			projectName.fixedWidth = tableControllerBaseX - projectName.frame.origin.x - BORDER_BETWEEN_NAME_AND_TABLE;
			
			if(projectName.bounds.size.height != oldHeight)
				[projectName setFrameOrigin : [self projectNamePos : self.bounds.size]];
			
		}
		
		if(needProcessAuthor)
		{
			oldHeight = authorName.bounds.size.height;
			authorName.fixedWidth = tableControllerBaseX - authorName.frame.origin.x - BORDER_BETWEEN_NAME_AND_TABLE;
			
			if(authorName.bounds.size.height != oldHeight)
				[authorName setFrameOrigin : [self authorNamePos : self.bounds.size]];
			
		}
	}
}

- (void) respondTo : (RakClickableText *) sender
{
	uint ID = _getFromSearch(NULL, PULL_SEARCH_AUTHORID, &(_data.authorName));
	
	if(ID != UINT_MAX)
	{
		[[NSNotificationCenter defaultCenter] postNotificationName:SR_NOTIFICATION_AUTHOR object:getStringForWchar(_data.authorName) userInfo:@{SR_NOTIF_CACHEID : @(ID), SR_NOTIF_OPTYPE : @(YES)}];
		[RakApp.serie ownFocus];
	}
}

#pragma mark - Elements positions

- (NSPoint) projectNamePos : (NSSize) size
{
	CGFloat height = size.height * 6.5 / 20;
	
	if(projectName != nil)
		height -= projectName.bounds.size.height / 2;
	
	return NSMakePoint(size.width * 7 / 100, height);
}

- (NSPoint) authorNamePos : (NSSize) size
{
	CGFloat height = size.height * 13 / 20;
	
	if(authorName != nil)
		height -= authorName.bounds.size.height / 2;
	
	return NSMakePoint(size.width * 9 / 100, height);
}

#pragma mark - UI utilities

- (NSRect) frameFromParent : (NSRect) parentFrame
{
	parentFrame.size.height *= 0.45f;
	
	return parentFrame;
}

- (NSRect) grandientBounds
{
	CGFloat height  = MIN(self.gradientMaxWidth, self.bounds.size.height * self.gradientWidth);
	return NSMakeRect(0, 0, self.bounds.size.width, height);
}

- (RakColor *) startColor
{
	return [Prefs getSystemColor : COLOR_CTHEADER_GRADIENT_START];
}

- (RakColor *) endColor : (const RakColor *) startColor
{
	return [Prefs getSystemColor : COLOR_CTHEADER_GRADIENT_END];
}

- (void) observeValueForKeyPath:(NSString *)keyPath ofObject:(id)object change:(NSDictionary *)change context:(void *)context
{
	if([object class] != [Prefs class])
		return [super observeValueForKeyPath:keyPath ofObject:object change:change context:context];
	
	if(projectName != nil)
		[projectName setTextColor:[self textColor]];
	
	if(authorName != nil)
		[authorName setTextColor:[self textColor]];
	
	[self setNeedsDisplay:YES];
}

- (RakColor *) textColor
{
	return [Prefs getSystemColor:COLOR_CTHEADER_FONT];
}

@end
