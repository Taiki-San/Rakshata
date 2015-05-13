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
	BUTTON_SIZE = 25,
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
		collapseButton = [[NSButton alloc] init];
		if(collapseButton != nil)
		{
			[(NSButtonCell *) collapseButton.cell setFocusRingType:NSFocusRingTypeNone];
			[collapseButton.cell setImage:[self getImage]];
			[collapseButton.cell setBackgroundColor:[NSColor clearColor]];
			collapseButton.bordered = NO;
			[collapseButton sizeToFit];
			
			[self addSubview:collapseButton];
		}
	}
	
	return self;
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
	}
	else
	{
		[self setFrame:newFrame];
		[collapseButton setFrame:[self collapseFrame:newFrame]];
	}
}

- (NSRect) collapseFrame : (NSRect) bounds
{
	NSSize size = collapseButton.bounds.size;
	
	bounds.origin.x = bounds.size.width - 50;
	bounds.origin.y = bounds.size.height / 2 - size.height / 2;
	bounds.size = size;
	
	return bounds;
}

#pragma mark - Color management

- (NSImage *) getImage
{
	NSImage * image = [NSImage imageNamed:NSImageNameSlideshowTemplate], * output = [image copy];
	
	if(output != nil)
	{
		[output setTemplate:NO];
		[output setSize:NSMakeSize(BUTTON_SIZE, BUTTON_SIZE)];
		
		[output lockFocus];
		
		[[Prefs getSystemColor:GET_COLOR_SURVOL :nil] set];
		NSRectFillUsingOperation(NSMakeRect(0, 0, BUTTON_SIZE, BUTTON_SIZE), NSCompositeSourceAtop);
		
		[output unlockFocus];
	}
	
	return output;
}

@end
