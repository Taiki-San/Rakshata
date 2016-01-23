/*********************************************************************************************
 **	__________         __           .__            __                 ________   _______   	**
 **	\______   \_____  |  | __  _____|  |__ _____ _/  |______   	___  _\_____  \  \   _  \  	**
 **	 |       _/\__  \ |  |/ / /  ___/  |  \\__  \\   __\__  \  	\  \/ //  ____/  /  /_\  \ 	**
 **	 |    |   \ / __ \|    <  \___ \|   Y  \/ __ \|  |  / __ \__ \   //       \  \  \_/   \	**
 **	 |____|_  /(____  /__|_ \/____  >___|  (____  /__| (____  /	  \_/ \_______ \ /\_____  /	**
 **	        \/      \/     \/     \/     \/     \/          \/ 	              \/ \/     \/ 	**
 **                                                                                         **
 **			This Source Code Form is subject to the terms of the Mozilla Public				**
 **			License, v. 2.0. If a copy of the MPL was not distributed with this				**
 **			file, You can obtain one at http://mozilla.org/MPL/2.0/.						**
 **                                                                                         **
 **                     			© Taiki 2011-2016                                       **
 **                                                                                         **
 ********************************************************************************************/

@implementation RakCTAnimationController

- (instancetype) init : (NSInteger) initialPos : (CGFloat) diff : (RakSegmentedButtonCell*) cell
{
	self = [super init];
	
	if(self != nil)
	{
		_initialState = initialPos;
		_animationDiff = diff;
		_cell = cell;
		self.stage = self.animationFrame;
	}
	
	return self;
}

#pragma mark - Context update

- (void) addCTContent : (RakCTSelectionListContainer*) chapter : (RakCTSelectionListContainer*) volume
{
	_chapter = chapter;
	_volume = volume;
}

#pragma mark - Animation Control

- (void) initiateCustomAnimation : (CGFloat) stepsRemaining
{
	if(_chapter != nil && _volume != nil)
	{
		_chapter.alphaValue = 1;		_volume.alphaValue = 1;
		
		CGFloat width = _chapter.superview.frame.size.width;
		
		if(!self.stage)
		{
			distanceToCoverPerMark = (width - _chapter.frame.origin.x) / self.animationFrame;
			
			chapOrigin = _chapter.frame.origin;
			volOrigin = _volume.frame.origin;
			
			if(_initialState == 0)
			{
				volOrigin.x = width;
				[_volume setFrameOrigin:volOrigin];
				distanceToCoverPerMark *= -1;
			}
			else
			{
				chapOrigin.x = - _chapter.frame.size.width;
				[_chapter setFrameOrigin : chapOrigin];
			}
			
			_chapter.hidden = _volume.hidden = NO;
		}
		else	//Aborted
		{
			CGFloat delta = CT_READERMODE_BORDER_TABLEVIEW;
			
			if(_initialState == 0)		delta -= volOrigin.x;
			else						delta -= chapOrigin.x;
			
			distanceToCoverPerMark = delta / stepsRemaining;
		}
	}
}

#pragma mark - Animation Work

- (void) animation:(NSAnimation *)animation didReachProgressMark:(NSAnimationProgress)progress
{
	[_cell updateAnimationStatus: YES : _initialState + _animationDiff * progress];
	[_cell.controlView setNeedsDisplay:YES];
	
	chapOrigin.x += distanceToCoverPerMark;		[_chapter setFrameOrigin:chapOrigin];
	volOrigin.x += distanceToCoverPerMark;		[_volume setFrameOrigin:volOrigin];
	
	[super animation:animation didReachProgressMark:progress];
}

- (void) animationDidEnd:(NSAnimation *)animation
{
	[super animationDidEnd:animation];
	
	[_cell.controlView setNeedsDisplay:YES];
}

- (void) postProcessingBeforeAction
{
	[_cell updateAnimationStatus:NO :1];
	
	NSRect superviewFrame = _chapter.superview.frame;
	_chapter.frame = superviewFrame;
	_volume.frame = superviewFrame;
}

@end

@implementation RakCTProjectImageView

- (instancetype) initWithImageName : (PROJECT_DATA) project : (NSRect) superviewFrame
{
	RakImage * projectImageBase = loadCTThumb(project);
	
	if(projectImageBase != nil)
	{
		self = [super initWithFrame:[self getProjectImageSize: superviewFrame : getThumbSize(projectImageBase)]];
		
		if(self != nil)
		{
			registerThumbnailUpdate(self, @selector(thumbnailUpdate:), THUMBID_CT);
			[self registerProject:project];

			[self setWantsLayer:NO];
			[self setImageAlignment:NSImageAlignCenter];
			[self setImageFrameStyle:NSImageFrameNone];
			[self setImage:projectImageBase];
		}
	}
	else
		self = nil;
	
	return self;
}

- (void) dealloc
{
	[[NSNotificationCenter defaultCenter] removeObserver:self];
}

- (void) updateProject : (PROJECT_DATA) project
{
	RakImage * projectImageBase = loadCTThumb(project);
	if(projectImageBase != nil)
	{
		[self setImage:projectImageBase];
		[self setFrame:self.superview.bounds];

		[self registerProject:project];
	}
}

- (void) registerProject : (PROJECT_DATA) project
{
	if(project.isInitialized)
	{
		_cachedProject = project;
		nullifyCTPointers(&_cachedProject);
	}
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
		[self updateProject:_cachedProject];
		[self setNeedsDisplay:YES];
	}
}

#pragma mark - Sizing

- (NSRect) getProjectImageSize : (NSRect) superviewFrame : (NSSize) imageSize
{
	NSRect frame;
	
	frame.size = imageSize;
	
	frame.origin.x = superviewFrame.size.width / 2 - frame.size.width / 2;
	frame.origin.y = superviewFrame.size.height - CT_READERMODE_WIDTH_PROJECT_NAME - CT_READERMODE_WIDTH_SPACE_NEXT_PROJECTIMAGE - imageSize.height;
	
	return frame;
}

- (void) setFrame:(NSRect)frameRect
{
	[super setFrame:[self getProjectImageSize:frameRect :getThumbSize(self.image)]];
}

- (void) resizeAnimation : (NSRect) frameRect
{
	[self setFrameAnimated : [self getProjectImageSize: frameRect :getThumbSize(self.image)]];
}

@end

@implementation RakCTCoreViewButtons

- (NSRect) getButtonFrame : (NSRect) superviewFrame
{
	NSRect frame = self.frame;
	
	frame.size.height = CT_READERMODE_HEIGHT_CT_BUTTON;
	if(frame.size.width > superviewFrame.size.width)
		frame.size.width = superviewFrame.size.width;
	
	frame.origin.y = superviewFrame.size.height - frame.size.height;
	frame.origin.x = superviewFrame.size.width / 2 - frame.size.width / 2;
	
	return frame;
}

@end
