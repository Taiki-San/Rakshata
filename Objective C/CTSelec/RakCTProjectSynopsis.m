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

#define TOP_BORDER_WIDTH 5
#define SYNOPSIS_BORDER 20

@implementation RakCTProjectSynopsis

- (instancetype) initWithProject : (PROJECT_DATA) project : (NSRect) frame : (NSSize) headerSize
{
	self = [self initWithFrame : [self frameFromParent : frame : headerSize]];
	
	if(self != nil)
	{
		_title = [[RakMenuText alloc] initWithText : self.bounds : @"Résumé"];
		if(_title != nil)
		{
			[_title sizeToFit];
			_title.ignoreInternalFrameMagic = YES;
			_title.barWidth = 1;

			NSRect titleFrame = [self frameForTitle:self.bounds : _title.bounds.size.height];
			
			[_title setFrame : titleFrame];

			[_title setAlignment : NSRightTextAlignment];
			[self addSubview:_title];
			
			_titleGradient = [[RakCTHImageGradient alloc] init];
			if(_titleGradient)
			{
				_titleGradient.gradientWidth = 1.0f;
				[_titleGradient setFrame:NSMakeRect(0, titleFrame.size.height - _title.barWidth, titleFrame.size.width / 4, _title.barWidth)];

				[Prefs getCurrentTheme:_titleGradient];
				[_titleGradient updateGradient];
				
				[_title addSubview:_titleGradient];
			}
		}
		
		_synopsis = [[RakText alloc] init];
		if(_synopsis != nil)
		{
			_synopsis.fixedWidth = self.bounds.size.width - 2 * SYNOPSIS_BORDER;
			[_synopsis.cell setWraps:YES];
			[_synopsis setAlignment:NSJustifiedTextAlignment];

			[_synopsis setTextColor : [Prefs getSystemColor:GET_COLOR_ACTIVE : self]];
			
			if(project.description[0] != 0)
				[_synopsis setStringValue : [[NSString alloc] initWithData:[NSData dataWithBytes:project.description length:sizeof(project.description)] encoding:NSUTF32LittleEndianStringEncoding]];
			else
				[_synopsis setStringValue:@"	But I must explain to you how all this mistaken idea of denouncing pleasure and praising pain was born and I will give you a complete account of the system, and expound the actual teachings of the great explorer of the truth, the master-builder of human happiness. No one rejects, dislikes, or avoids pleasure itself, because it is pleasure, but because those who do not know how to pursue pleasure rationally encounter consequences that are extremely painful. Nor again is there anyone who loves or pursues or desires to obtain pain of itself, because it is pain, but because occasionally circumstances occur in which toil and pain can procure him some great pleasure. To take a trivial example, which of us ever undertakes laborious physical exercise, except to obtain some advantage from it? But who has any right to find fault with a man who chooses to enjoy a pleasure that has no annoying consequences, or one who avoids a pain that produces no resultant pleasure?"];//@"Aucun résumé disponible."];
			
			[_synopsis setFrame : [self frameForContent : self.bounds : _title != nil ? _title.bounds.size.height : 0]];
			[self addSubview:_synopsis];
		}
	}
	
	return self;
}

- (BOOL) isFlipped	{	return YES;	}

#pragma mark - Resize

- (void) setFrame : (NSRect) frameRect : (NSSize) headerSize
{
	[self updateFrame : frameRect : headerSize : NO];
}

- (void) resizeAnimation : (NSRect) frameRect : (NSSize) headerSize
{
	[self updateFrame : frameRect : headerSize : YES];
}

- (void) updateFrame : (NSRect) frame : (NSSize) headerSize : (BOOL) animated
{
	NSRect mainFrame = [self frameFromParent:frame : headerSize];
	_synopsis.fixedWidth = mainFrame.size.width - 2 * SYNOPSIS_BORDER;

	//Update main frame
	if(animated)
		[self.animator setFrame : mainFrame];
	else
		[self setFrame : mainFrame];

	//Set up new frames
	mainFrame.origin = NSZeroPoint;
	
	const CGFloat titleHeight = _title.bounds.size.height;
	const NSRect titleFrame =	[self frameForTitle : mainFrame : titleHeight];
	const NSRect gradientFrame = NSMakeRect(0, titleFrame.size.height - _title.barWidth, titleFrame.size.width / 5, _title.barWidth);

	//Update content frames
	if(animated)
	{
		[_title.animator setFrame:titleFrame];
		[_titleGradient.animator setFrame:gradientFrame];

		[_synopsis.animator setFrame: [self frameForContent : mainFrame : titleHeight]];
	}
	else
	{
		[_title setFrame:titleFrame];
		[_titleGradient setFrame:gradientFrame];

		[_synopsis setFrame: [self frameForContent : mainFrame : titleHeight]];
	}
}

#pragma mark - Position routines

- (NSRect) frameFromParent : (NSRect) parentFrame : (NSSize) headerSize
{
	parentFrame.origin.x = headerSize.width;
	parentFrame.origin.y = headerSize.height;
	parentFrame.size.height -= parentFrame.origin.y;
	parentFrame.size.width -= parentFrame.origin.x;
	
	return parentFrame;
}

- (NSRect) frameForTitle : (NSRect) mainBounds : (CGFloat) height
{
	mainBounds.origin.y = TOP_BORDER_WIDTH;
	mainBounds.size.height = height;
	
	mainBounds.origin.x = 15;
	mainBounds.size.width -= 2 * mainBounds.origin.x;
	return mainBounds;
}

- (NSRect) frameForContent : (NSRect) mainBounds : (CGFloat) titleHeight
{
	mainBounds.origin.x = SYNOPSIS_BORDER;
	
	if(_synopsis == nil)
	{
		mainBounds.size.width -= SYNOPSIS_BORDER + 5;
		mainBounds.size.height -= mainBounds.origin.y;
	}
	else
		mainBounds.size = [_synopsis intrinsicContentSize];
	
	mainBounds.origin.y = TOP_BORDER_WIDTH + titleHeight + 5;

	return mainBounds;
}

@end
