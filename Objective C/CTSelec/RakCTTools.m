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
		state = animationFrame;
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
		
		if(!state)
		{
			distanceToCoverPerMark = (width - _chapter.frame.origin.x) / animationFrame;
			
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
		}
		else	//Aborted
		{
			CGFloat delta = CT_READERMODE_BORDER_TABLEVIEW;
			
			if (_initialState == 0)		delta -= volOrigin.x;
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

- (id) initWithImageName : (REPO_DATA*) repo : (NSString *) imageName : (NSRect) superviewFrame
{
	NSImage * projectImageBase = nil;
	
	if(repo != NULL && imageName != nil)
	{
		char * encodedHash = getPathForRepo(repo);
		if(encodedHash != NULL)
		{
			NSString * path = [NSString stringWithFormat:@"imageCache/%s/", encodedHash];
			free(encodedHash);
			
			if(path != nil)
				projectImageBase = [[NSBundle bundleWithPath: path] imageForResource:[NSString stringWithFormat:@"%@_"PROJ_IMG_SUFFIX_CT, imageName]];
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
		
		self = [super initWithFrame:[self getProjectImageSize: superviewFrame : [projectImageBase size] ] ];
		
		if(self != nil)
		{
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

- (NSRect) getProjectImageSize : (NSRect) superviewFrame : (NSSize) imageSize
{
	NSRect frame;
	
	frame.size.height = imageSize.height;
	frame.size.width = imageSize.width;
	
	frame.origin.x = superviewFrame.size.width / 2 - frame.size.width / 2;
	frame.origin.y = superviewFrame.size.height - CT_READERMODE_WIDTH_PROJECT_NAME - CT_READERMODE_WIDTH_SPACE_NEXT_PROJECTIMAGE - imageSize.height;
	
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

NSString * priceString(uint price)
{
	if(price != 0)
	{
		NSNumberFormatter * formater = [[NSNumberFormatter alloc] init];
		
		formater.numberStyle = NSNumberFormatterCurrencyStyle;
		formater.usesSignificantDigits = YES;
		formater.minimumSignificantDigits = 2;
		
		return [formater stringFromNumber:@(price / 100.0f)];
	}
	
	return NSLocalizedString(@"FREE", nil);
}

