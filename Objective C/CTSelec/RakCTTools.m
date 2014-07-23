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
	}
	
	return self;
}

#pragma mark - Context update

- (void) addCTContent : (RakCTCoreContentView*) chapter : (RakCTCoreContentView*) volume
{
	_chapter = chapter;
	_volume = volume;
}

- (void) updateState : (NSInteger) initialPos : (CGFloat) diff
{
	_initialState = initialPos;
	_animationDiff = diff;
}

#pragma mark - Animation Control

- (void) startAnimation
{
	if(_animation != nil)
		[self abortAnimation];
	
	_animation = [[[NSAnimation alloc] initWithDuration:0.15 animationCurve:NSAnimationEaseInOut] autorelease];
	[_animation setFrameRate:60];
	[_animation setAnimationBlockingMode:NSAnimationNonblocking];
	[_animation setDelegate:self];
	
	for (NSAnimationProgress i = 0; i < 1; i += 1 / 9.0f)
	{
		[_animation addProgressMark : i];
	}
	
	if(_chapter != nil && _volume != nil)
	{
		[_chapter setHidden: NO];	[_volume setHidden: NO];
		
		CGFloat width = _chapter.superview.frame.size.width;
		distanceToCoverPerMark = (width - _chapter.frame.origin.x) / 9.0f;
		
		chapOrigin = _chapter.frame.origin;
		volOrigin = _volume.frame.origin;

		if(_initialState == 0)
		{
			distanceToCoverPerMark *= -1;
			volOrigin.x = width;
			[_volume setFrameOrigin:volOrigin];
		}
		else
		{
			chapOrigin.x = - _chapter.frame.size.width;
			[_chapter setFrameOrigin : chapOrigin];
		}
	}
	
	[_animation startAnimation];
}

- (void) abortAnimation
{
	[_animation stopAnimation];
	_animation = nil;
}

#pragma mark - Animation Work

- (void) animation:(NSAnimation *)animation didReachProgressMark:(NSAnimationProgress)progress
{
	[_cell updateAnimationStatus: YES : _initialState + _animationDiff * progress];
	[_cell.controlView setNeedsDisplay:YES];
	
	chapOrigin.x += distanceToCoverPerMark;		[_chapter setFrameOrigin:chapOrigin];
	volOrigin.x += distanceToCoverPerMark;		[_volume setFrameOrigin:volOrigin];
}

- (void)animationDidEnd:(NSAnimation *)animation
{
	if(animation == _animation)
	{
		[_cell updateAnimationStatus:NO :1];
		
		NSRect superviewFrame = _chapter.superview.frame;
		_chapter.frame = superviewFrame;
		_volume.frame = superviewFrame;
		
		if(_initialState == 0)
			_chapter.hidden = YES;
		else
			_volume.hidden = YES;
		
		_animation = nil;
	}
	
	[_cell.controlView setNeedsDisplay:YES];
}

@end

@implementation RakTextProjectName

- (CGFloat) getFontSize
{
	return 16;
}

@end

@implementation RakCTProjectImageView

- (id) initWithImageName : (char*) URLRepo : (NSString *) imageName : (NSRect) superViewFrame
{
	NSImage * projectImageBase = nil;
	
	char * encodedHash = getPathForTeam(URLRepo);
	
	if(encodedHash == NULL)
		return nil;
	
	NSString * path = [NSString stringWithFormat:@"imageCache/%s/", encodedHash];
	free(encodedHash);
	
	if(path != nil && imageName != nil)
	{
		NSBundle * bundle = [NSBundle bundleWithPath: path];
		if(bundle != nil)
		{
			projectImageBase = [bundle imageForResource:[NSString stringWithFormat:@"%@_large", imageName]];
		}
	}
	
	if(projectImageBase == nil)
	{
		projectImageBase = [RakResPath craftResNameFromContext:@"defaultCTImage" :NO :YES : 1];
	}
	
	if(projectImageBase != nil)
	{
		if(projectImageBase.size.height != CT_READERMODE_HEIGHT_PROJECT_IMAGE)
		{
			NSSize imageSize = projectImageBase.size;
			CGFloat ratio = imageSize.height / CT_READERMODE_HEIGHT_PROJECT_IMAGE;
			
			imageSize.height *= ratio;
			imageSize.width *= ratio;
			
			[projectImageBase setSize:imageSize];
		}
		
		self = [super initWithFrame:[self getProjectImageSize: superViewFrame : [projectImageBase size] ] ];
		
		if(self != nil)
		{
			[self setWantsLayer:NO];
			[self setImageAlignment:NSImageAlignCenter];
			[self setImageFrameStyle:NSImageFrameNone];
			[self setImage:projectImageBase];
		}
	}
	else
	{
		[self release];
		self = nil;
	}
	
	return self;
}

- (void) updateProject : (NSString *) imageName
{
	NSImage * projectImageBase = [RakResPath craftResNameFromContext:imageName :NO :YES : 1];
	if(projectImageBase != nil)
	{
		if(projectImageBase.size.height != CT_READERMODE_HEIGHT_PROJECT_IMAGE)
		{
			NSSize imageSize = projectImageBase.size;
			CGFloat ratio = imageSize.height / CT_READERMODE_HEIGHT_PROJECT_IMAGE;
			
			imageSize.height *= ratio;
			imageSize.width *= ratio;
			
			[projectImageBase setSize:imageSize];
		}
		
		[self setImage:projectImageBase];
	}
}

- (NSRect) getProjectImageSize : (NSRect) superViewFrame : (NSSize) imageSize
{
	NSRect frame;
	
	frame.size.height = imageSize.height;
	frame.size.width = imageSize.width;
	
	frame.origin.x = superViewFrame.size.width / 2 - frame.size.width / 2;
	frame.origin.y = superViewFrame.size.height - CT_READERMODE_WIDTH_PROJECT_NAME - CT_READERMODE_WIDTH_SPACE_NEXT_PROJECTIMAGE - imageSize.height;
	
	return frame;
}

- (void) setFrame:(NSRect)frameRect
{
	[super setFrame:[self getProjectImageSize:frameRect :[self image].size]];
}

- (void) resizeAnimation : (NSRect) frameRect
{
	[self.animator setFrame : [self getProjectImageSize: frameRect :[self image].size]];
}

@end

