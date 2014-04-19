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
 ********************************************************************************************/

@implementation RakMenuText

#pragma mark - Drawing

- (void) additionalDrawing
{
	NSRect frame = self.bounds;
	
	frame.origin.y = frame.size.height - 2;
	frame.size.height = 2;
	
	[[self getBarColor] setFill];
	NSRectFill(frame);
}

#pragma mark - Color

- (NSColor *) getTextColor
{
	return [Prefs getSystemColor:GET_COLOR_INACTIVE];
}

- (NSColor *) getBarColor
{
	return [Prefs getSystemColor:GET_COLOR_INACTIVE];
}

- (CGFloat) getFontSize
{
	return [NSFont systemFontSize];
}

- (NSFont *) getFont
{
	return [NSFont fontWithName:@"Helvetica-Bold" size:[self getFontSize]];
}

#pragma mark - Controller

- (CGFloat) getTextHeight
{
	return CT_READERMODE_WIDTH_PROJECT_NAME;
}

- (NSRect) getProjectNameSize : (NSRect) superViewSize
{
	NSRect frame = superViewSize;
	frame.size.height = [self getTextHeight];
	frame.origin.y = superViewSize.size.height - frame.size.height;
	
	return frame;
}

- (id) initWithText:(NSRect)frame :(NSString *)text :(NSColor *)color
{
	text = [text stringByReplacingOccurrencesOfString:@"_" withString:@" "];
	self = [super initWithText:[self getProjectNameSize:frame] :text : [self getTextColor]];
	
	if(self != nil)
	{
		[self setFont:[self getFont]];
		
		if(color != nil)
		{
			[self setBackgroundColor:color];
			[self setDrawsBackground:YES];
		}
		else
		{
			[self setBackgroundColor:[NSColor clearColor]];
			[self setDrawsBackground:NO];
		}
		
	}
	
	return self;
}

- (void) setFrame:(NSRect)frameRect
{
	[super setFrame:[self getProjectNameSize:frameRect]];
}


@end
