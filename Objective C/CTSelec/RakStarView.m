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

enum
{
	WIDTH = 50,
	HEIGHT = 9,
	
	NB_STAR = 5
};

static 	NSImage * starActive, * starIdle;
uint _currentTheme;

@implementation RakStarView

- (instancetype) init : (PROJECT_DATA) project
{
	self = [super init];
	
	if(self != nil)
	{
		uint currentTheme = [Prefs getCurrentTheme:self];
		
		if(starActive == nil)
		{
			starActive = [RakResPath getImageFromTheme :@"ratingSelected" :currentTheme];
			_currentTheme = currentTheme;
		}
		if(starIdle == nil)
		{
			starIdle = [RakResPath getImageFromTheme :@"ratingEmpty" :currentTheme];
			_currentTheme = currentTheme;
		}
		
		if(starActive == nil || starIdle == nil)
			return nil;
		
		_wantNumber = NO;
		_project = project;
		
		rating = (getRandom() % 50) / 10.0f;
		
		[self setFrameSize : NSMakeSize(WIDTH, HEIGHT)];
		basePoint = NSZeroPoint;
	}
	
	return self;
}

- (void) dealloc
{
	[Prefs deRegisterForChange:self forType:KVO_THEME];
}

#pragma mark - Context update

- (void) updateProject : (PROJECT_DATA) project
{
	if(!project.isInitialized)
		return;
	
	if(_wantNumber)
	{
		number.stringValue = [self numberOfRatings];
		
		[number sizeToFit];
		[self refreshFrame];
	}
	
	_project = project;
	rating = (getRandom() % 50) / 10.0f;
	
	[self setNeedsDisplay:YES];
}

- (void) setWantNumber:(BOOL)wantNumber
{
	if(wantNumber == _wantNumber)
		return;
	else
		_wantNumber = wantNumber;
	
	if(!wantNumber)
	{
		number.hidden = YES;
		
		[self setFrameSize:NSMakeSize(WIDTH, HEIGHT)];
		basePoint = NSZeroPoint;
	}
	else
	{
		if(number == nil)
		{
			number = [[RakText alloc] initWithText:[self numberOfRatings] :[self textColor]];
			if(number == nil)
				return;
			
			number.font = [NSFont fontWithName:[Prefs getFontName:GET_FONT_STANDARD] size:9];
			[number sizeToFit];
			
			[self addSubview:number];
		}
		else
			number.hidden = NO;
		
		[self refreshFrame];
	}
}

- (NSString *) numberOfRatings
{
	NSNumberFormatter *formatter = [[NSNumberFormatter alloc] init];
	[formatter setNumberStyle:NSNumberFormatterDecimalStyle];
	
	return [formatter stringFromNumber: @(getRandom() % 10000)];
}

- (void) refreshFrame
{
	if(_wantNumber)
	{
		[self setFrameSize:NSMakeSize(number.bounds.size.width + WIDTH, MAX(HEIGHT, number.bounds.size.height))];
		
		[number setFrameOrigin:NSMakePoint(0, self.bounds.size.height / 2 - number.bounds.size.height / 2)];
		basePoint = NSMakePoint(number.bounds.size.width, self.bounds.size.height / 2 - HEIGHT / 2);
	}
	else
	{
		[self setFrameSize:NSMakeSize(WIDTH, HEIGHT)];
		basePoint = NSZeroPoint;
	}
}

#pragma mark - Drawing

- (NSColor *) textColor
{
	return [Prefs getSystemColor:COLOR_ACTIVE :nil];
}

- (void)observeValueForKeyPath:(NSString *)keyPath ofObject:(id)object change:(NSDictionary *)change context:(void *)context
{
	if([object class] != [Prefs class])
		return [super observeValueForKeyPath:keyPath ofObject:object change:change context:context];
	
	uint currentTheme = [Prefs getCurrentTheme:nil];
	
	if(currentTheme != _currentTheme)
	{
		starActive = [RakResPath getImageFromTheme :@"ratingSelected" :currentTheme];
		starIdle = [RakResPath getImageFromTheme :@"ratingEmpty" :currentTheme];
		
		_currentTheme = currentTheme;
	}
	
	[self setNeedsDisplay:YES];
}

- (void) drawRect : (NSRect) dirtyRect
{
	NSPoint workingPoint = basePoint;
	CGFloat intermediaryPoint = 0;
	uint i;
	
	for(i = 0; i < rating; i++)
	{
		if(i != floor(rating))
		{
			[starActive drawAtPoint:workingPoint fromRect:NSZeroRect operation:NSCompositeCopy fraction:1];
			workingPoint.x += starActive.size.width;
		}
		else
		{
			intermediaryPoint = starActive.size.width * (rating - floor(rating));
			[starActive drawAtPoint:workingPoint fromRect:NSMakeRect(0, 0, intermediaryPoint, starActive.size.height) operation:NSCompositeCopy fraction:1];
			workingPoint.x += intermediaryPoint;
			break;
		}
	}
	
	while(i < NB_STAR)
	{
		if(intermediaryPoint != 0)
		{
			[starIdle drawAtPoint:workingPoint fromRect:NSMakeRect(intermediaryPoint, 0, starActive.size.width - intermediaryPoint, starActive.size.height) operation:NSCompositeCopy fraction:1];
			
			workingPoint.x += starActive.size.width - intermediaryPoint;
			intermediaryPoint = 0;
		}
		else
		{
			[starIdle drawAtPoint:workingPoint fromRect:NSZeroRect operation:NSCompositeCopy fraction:1];
			workingPoint.x += starIdle.size.width;
		}
		i++;
	}
}

@end
