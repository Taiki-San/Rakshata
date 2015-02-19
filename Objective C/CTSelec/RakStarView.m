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
	SEPARATOR = 0
};

@implementation RakStarView

- (instancetype) init : (PROJECT_DATA) project
{
	self = [super init];
	
	if(self != nil)
	{
		stars = [[NSImageView alloc] init];	//NSMakeRect(0, 0, 50, 9)
		if(stars != nil)
		{
			stars.image = [[NSImage alloc] initByReferencingFile:@"stars.png"];
			[stars setFrame: (NSRect) {NSZeroPoint, stars.image.size}];

			[self addSubview:stars];
//			[stars setNumberOfMajorTickMarks:5];
//			stars.cell = [[NSLevelIndicatorCell alloc] initWithLevelIndicatorStyle:NSRatingLevelIndicatorStyle];
		}
		else
			return nil;

		_wantNumber = NO;
		_project = project;

		[self setFrameSize : stars.image.size];
		[Prefs getCurrentTheme:self];
	}
	
	return self;
}

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
}

- (void) dealloc
{
	[Prefs deRegisterForChanges:self];
}

- (void) setWantNumber:(BOOL)wantNumber
{
	if(wantNumber == _wantNumber)
		return;
	
	if(!wantNumber)
	{
		number.hidden = YES;

		[self setFrameSize:stars.bounds.size];
		[stars setFrameOrigin:NSZeroPoint];
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
	
	_wantNumber = wantNumber;
}

- (BOOL) wantNumber
{
	return _wantNumber;
}

- (NSString *) numberOfRatings
{
	NSNumberFormatter *formatter = [[NSNumberFormatter alloc] init];
	[formatter setNumberStyle:NSNumberFormatterDecimalStyle];
	
	return [formatter stringFromNumber: @(getRandom() % 10000)];
}

- (void) refreshFrame
{
	[self setFrameSize:NSMakeSize(number.bounds.size.width + SEPARATOR + stars.bounds.size.width, MAX(stars.bounds.size.height, number.bounds.size.height))];
	
	[number setFrameOrigin:NSMakePoint(0, self.bounds.size.height / 2 - number.bounds.size.height / 2)];
	[stars setFrameOrigin:NSMakePoint(number.bounds.size.width + SEPARATOR, self.bounds.size.height / 2 - stars.bounds.size.height / 2)];
}

#pragma mark - Color

- (NSColor *) textColor
{
	return [Prefs getSystemColor:GET_COLOR_ACTIVE :nil];
}

- (void)observeValueForKeyPath:(NSString *)keyPath ofObject:(id)object change:(NSDictionary *)change context:(void *)context
{
	if([object class] != [Prefs class])
		return;
	
	if(number != nil)
		number.textColor = [self textColor];
}

@end
