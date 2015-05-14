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
	BUTTON_WIDTH = 17,
	BUTTON_HEIGHT = 14,
	BORDER_COLLAPSE = 40
};

@interface RakMDLFooter()
{
	NSButton *collapseButton;
	RakButton * actionButton;
}

@end

@implementation RakMDLFooter

- (instancetype) initWithFrame : (NSRect) frameRect
{
	self = [super initWithFrame:frameRect];
	
	if(self != nil)
	{
		collapseButton = [[NSButton alloc] initWithFrame:[self collapseFrame:_bounds]];
		if(collapseButton != nil)
		{
			[(NSButtonCell *) collapseButton.cell setFocusRingType:NSFocusRingTypeNone];
			[collapseButton.cell setImage:[self getImage]];
			[collapseButton.cell setBackgroundColor:[NSColor clearColor]];
			collapseButton.bordered = NO;
			
			collapseButton.target = self;
			collapseButton.action = @selector(collapseClicked);
			
			[self addSubview:collapseButton];
		}
		
		actionButton = [RakButton allocWithText:NSLocalizedString(@"MDL-FLUSH-INSTALLED", nil)];
		if(actionButton != nil)
		{
			[actionButton setFrame:[self actionFrame:_bounds]];

			actionButton.target = self;
			actionButton.action = @selector(actionClicked);
			
			[self addSubview:actionButton];
		}
	}
	
	return self;
}

#pragma mark - Clic callback

- (void) actionClicked
{
	[_controller discardInstalled];
}

- (void) collapseClicked
{
	NSLog(@"Collapse clicked");
}

#pragma mark - Sizing

- (void) setFrame:(NSRect)frameRect
{
	[self resize:frameRect :NO];
}

- (void) resizeAnimation : (NSRect) frameRect
{
	[self resize:frameRect :YES];
}

- (void) resize : (NSRect) newFrame : (BOOL) animated
{
	if(animated)
	{
		[self.animator setFrame:newFrame];
		[collapseButton.animator setFrame:[self collapseFrame:newFrame]];
		[actionButton.animator setFrame:[self actionFrame:_bounds]];
	}
	else
	{
		[super setFrame:newFrame];
		[collapseButton setFrame:[self collapseFrame:newFrame]];
		[actionButton setFrame:[self actionFrame:_bounds]];
	}
}

- (NSRect) actionFrame : (NSRect) bounds
{
	NSSize size = actionButton.bounds.size;
	
	bounds.origin.x = bounds.size.width / 20;
	bounds.origin.y = bounds.size.height / 2 - size.height / 2;
	bounds.size = size;
	
	return bounds;
}

- (NSRect) collapseFrame : (NSRect) bounds
{
	bounds.origin.x = bounds.size.width * 19 / 20 - BUTTON_WIDTH;
	bounds.origin.y = bounds.size.height / 2 - BUTTON_HEIGHT / 2;
	bounds.size = NSMakeSize(BUTTON_WIDTH, BUTTON_HEIGHT);
	
	return bounds;
}

#pragma mark - Color management

- (NSImage *) getImage
{
	NSImage * image = [NSImage imageNamed:NSImageNameSlideshowTemplate], * output = [image copy];
	
	if(output != nil)
	{
		[output setTemplate:NO];
		[output setSize:NSMakeSize(BUTTON_WIDTH, BUTTON_HEIGHT)];
		
		[output lockFocus];
		
		[[Prefs getSystemColor:GET_COLOR_INACTIVE :nil] set];
		NSRectFillUsingOperation(NSMakeRect(0, 0, BUTTON_WIDTH, BUTTON_HEIGHT), NSCompositeSourceAtop);
		
		[output unlockFocus];
	}
	
	return output;
}

@end
