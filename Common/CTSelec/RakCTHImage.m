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

#define DEFAULT_TITLE_HEIGHT (CT_READERMODE_WIDTH_PROJECT_NAME + 6)

@implementation RakCTHImage

- (instancetype) initWithProject : (NSRect) parentFrame : (PROJECT_DATA) data
{
	self = [self initWithFrame : parentFrame];
	
	if(self != nil)
	{
		if(![self loadProject : data])
			return nil;
		
		registerThumbnailUpdate(self, @selector(thumbnailUpdate:), THUMBID_HEAD);
		
		[self setImageScaling:NSImageScaleProportionallyUpOrDown];
	}
	
	return self;
}

- (BOOL) mouseDownCanMoveWindow
{
	return YES;
}

- (BOOL) loadProject : (PROJECT_DATA) data
{
	if(data.isInitialized)
	{
		_cachedProject = data;
		nullifyCTPointers(&_cachedProject);
	}
	
	return (self.image = loadCTHeader(data)) != nil;
}

- (void) thumbnailUpdate : (NSNotification *) notification
{
	NSDictionary * dict = notification.userInfo;
	if(dict == nil || !_cachedProject.isInitialized)
		return;
	
	NSNumber * project = [dict objectForKey:@"project"], * repo = [dict objectForKey:@"source"];
	
	if(project == nil || repo == nil)
		return;
	
	if([project unsignedIntValue] == _cachedProject.projectID && [repo unsignedLongLongValue] == getRepoID(_cachedProject.repo))
	{
		[self loadProject:_cachedProject];
		[self setNeedsDisplay:YES];
	}
}

- (RakCTHImageGradient *) gradientView
{
	if(gradient == nil)
	{
		gradient = [[RakCTHImageGradient alloc] initWithFrame:self.frame];
		if(gradient != nil)
		{
			gradient.gradientMaxWidth = 200;
			gradient.gradientWidth = 0.2;
			gradient.angle = 180;
			[gradient initGradient];
		}
	}
	return gradient;
}

- (void) dealloc
{
	[[NSNotificationCenter defaultCenter] removeObserver:self];

	self.image = nil;
	[gradient removeFromSuperview];
}

- (void) setFrame:(NSRect)frameRect
{
	[super setFrame : frameRect];
	[gradient setFrame : frameRect];
}

- (void) resizeAnimation : (NSRect) frameRect
{
	[self setFrameAnimated : frameRect];
	[gradient setFrameAnimated : frameRect];
}

@end

@implementation RakCTHImageGradient

- (RakColor *) startColor
{
	return [Prefs getSystemColor : COLOR_COREVIEW_BACKGROUND];
}

- (RakColor *) endColor : (RakColor *) startColor
{
	return [startColor colorWithAlphaComponent:0];
}

- (void) drawRect:(NSRect)dirtyRect
{
	[super drawRect:dirtyRect];
	
	NSRect frame = [super grandientBounds];
	frame.origin.y = frame.size.height - DEFAULT_TITLE_HEIGHT;
	frame.size.height = DEFAULT_TITLE_HEIGHT;
	
	[_titleGradient drawInRect : frame angle : self.angle];
}

#pragma mark - UI utilities

- (NSRect) grandientBounds
{
	NSRect frame = [super grandientBounds];
	
	frame.size.height -= DEFAULT_TITLE_HEIGHT;
	
	return frame;
}

- (void) updateGradient
{
	RakColor * startColor = [Prefs getSystemColor:COLOR_TABS_BACKGROUND];
	_titleGradient = [[NSGradient alloc] initWithStartingColor : startColor endingColor : [self endColor: startColor]];
	
	[super updateGradient];
}

@end
