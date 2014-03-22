/*********************************************************************************************
 **	__________         __           .__            __                 ________   _______   	**
 **	\______   \_____  |  | __  _____|  |__ _____ _/  |______   	___  _\_____  \  \   _  \  	**
 **	 |       _/\__  \ |  |/ / /  ___/  |  \\__  \\   __\__  \  	\  \/ //  ____/  /  /_\  \ 	**
 **	 |    |   \ / __ \|    <  \___ \|   Y  \/ __ \|  |  / __ \__ \   //       \  \  \_/   \	**
 **	 |____|_  /(____  /__|_ \/____  >___|  (____  /__| (____  /	  \_/ \_______ \ /\_____  /	**
 **	        \/      \/     \/     \/     \/     \/          \/ 	              \/ \/     \/ 	**
 **                                                                                          **
 **    Licence propriétaire, code source confidentiel, distribution formellement interdite   **
 **                                                                                          **
 *********************************************************************************************/

@implementation RakButton

+ (id) initForReader : (NSView*) superView : (NSString*) pathToImage : (CGFloat) posX : (BOOL) posXFromLeftSide : (id) target : (SEL) selectorToCall
{
	RakButton *output = [[RakButton new] autorelease];
	
	if(output != nil)
	{
		NSImage * tmpImage = [[NSImage alloc] initByReferencingFile:pathToImage];
		
		if(tmpImage != nil)
		{
			//Set image
			[output setImage:tmpImage];
			[tmpImage release];
			
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

- (void) highlight:(BOOL)flag withFrame:(NSRect)cellFrame inView:(NSView*)controlView
{
	if (flag)
		self.backgroundColor = [NSColor clearColor];
	
	[super highlight:flag withFrame:cellFrame inView:controlView];
}

@end