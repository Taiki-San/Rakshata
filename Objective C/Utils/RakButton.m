/*********************************************************************************************
 **	__________         __           .__            __                 ________   _______   	**
 **	\______   \_____  |  | __  _____|  |__ _____ _/  |______   	___  _\_____  \  \   _  \  	**
 **	 |       _/\__  \ |  |/ / /  ___/  |  \\__  \\   __\__  \  	\  \/ //  ____/  /  /_\  \ 	**
 **	 |    |   \ / __ \|    <  \___ \|   Y  \/ __ \|  |  / __ \__ \   //       \  \  \_/   \	**
 **	 |____|_  /(____  /__|_ \/____  >___|  (____  /__| (____  /	  \_/ \_______ \ /\_____  /	**
 **	        \/      \/     \/     \/     \/     \/          \/ 	              \/ \/     \/ 	**
 **                                                                                         **
 **    Licence propriétaire, code source confidentiel, distribution formellement interdite  **
 **                                                                                         **
 *********************************************************************************************/

@implementation RakButton

+ (id) initForSeries : (NSView*) superView : (NSString*) imageName : (NSPoint) origin : (id) target : (SEL) selectorToCall
{
	RakButton *output = [[RakButton new] autorelease];
	
	if(output != nil)
	{
		RakButtonCell * cell = [[RakButtonCell alloc] initWithPage: imageName : RB_STATE_STANDARD];
		
		if(cell != nil)
		{
			//Set image
			[output setCell:cell];
			
			//Update a couple of prefs
			[output sizeToFit];
			output.wantsLayer = YES;
			output.layer.backgroundColor = [Prefs getSystemColor:GET_COLOR_BACKGROUD_BACK_BUTTONS].CGColor;
			output.layer.cornerRadius = 4;
			[output setBordered:NO];
			
			//Set action
			if(target != nil)
			{
				[output setTarget:target];
				[output setAction:selectorToCall];
			}
			
			//Set origin
			origin.x -= cell.cellSize.width / 2;
			[output setFrameOrigin: origin];
			
			//Add to the superview
			[superView addSubview:output];
		}
		else
		{
			[output release];
			output = nil;
		}
	}
	
	return output;
}

+ (id) initForReader : (NSView*) superView : (NSString*) imageName : (short) stateAtStartup : (CGFloat) posX : (BOOL) posXFromLeftSide : (id) target : (SEL) selectorToCall
{
	RakButton *output = [[RakButton new] autorelease];
	
	if(output != nil)
	{
		RakButtonCell * cell = [[RakButtonCell alloc] initWithPage: imageName : stateAtStartup];
		
		if(cell != nil)
		{
			//Set image
			[output setCell:cell];
			
			//Update a couple of prefs
			[output sizeToFit];
			[output setBordered:NO];

			//Set action
			[output setTarget:target];
			[output setAction:selectorToCall];
			
			//Set origin
			NSPoint point;
			
			if(posXFromLeftSide)
				point = NSMakePoint(posX, superView.frame.size.height / 2 - output.frame.size.height / 2);
			else
				point = NSMakePoint(posX - output.frame.size.width, superView.frame.size.height / 2 - output.frame.size.height / 2);
			
			[output setFrameOrigin: point];
			
			//Add to the superview
			[superView addSubview:output];
		}
		else
		{
			[output release];
			output = nil;
		}
	}
	
	return output;
}

+ (Class) cellClass
{
	return [RakButtonCell class];
}

- (void) refreshViewSize
{
	NSRect frame = { {25, self.superview.frame.size.height - 45 } , {65, 28} };
	[self setFrame:frame];
}

@end

@implementation RakButtonCell

- (id) initWithPage : (NSString*) imageName : (short) state
{
	self = [super init];
	
	if(self != nil)
	{
		clicked		= [[RakResPath craftResNameFromContext:imageName : YES : YES : 1] retain];
		nonClicked	= [[RakResPath craftResNameFromContext:imageName : NO : YES : 1] retain];
		unAvailable = [[RakResPath craftResNameFromContext:imageName : NO : NO : 1] retain];
		notAvailable = false;
		
		if(state == RB_STATE_STANDARD && nonClicked != nil)
			[self setImage:nonClicked];
		else if(state == RB_STATE_HIGHLIGHTED && clicked != nil)
			[self setImage:clicked];
		else if(unAvailable != nil)
		{
			[self setImage:unAvailable];
			notAvailable = true;
		}
		else
		{
			NSLog(@"Failed at create button for icon: %@", imageName);
			[self release];
			return nil;
		}
	}
	
	return self;
}

- (void) highlight:(BOOL)flag withFrame:(NSRect)cellFrame inView:(NSView*)controlView
{
	if(notAvailable)
		return;
	
	if (flag)
	{
		[self setImage:clicked];
		self.backgroundColor = [NSColor clearColor];
	}
	else
	{
		[self setImage:nonClicked];
	}
	
	[super highlight:flag withFrame:cellFrame inView:controlView];
}

- (void) dealloc
{
	[clicked release];
	[nonClicked release];
	[unAvailable release];
	[self setImage:nil];
	[super dealloc];
}

@end