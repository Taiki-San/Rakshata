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

@implementation RakTextCell

- (id)initWithText : (NSString *) text : (NSColor *) color
{
	self = [self initTextCell:text];
    if (self)
	{
		self.editable = NO;
		self.bordered = NO;
		self.drawsBackground = NO;
		self.backgroundColor = [NSColor clearColor];
		
		self.font = [NSFont systemFontOfSize:13];
		self.alignment = NSCenterTextAlignment;
		
		if(color != nil)
			self.textColor = color;
		
		self.selectable = NO;
    }
    return self;
}

- (void) highlight:(BOOL)flag withFrame:(NSRect)cellFrame inView:(NSView*)controlView
{
	if (flag)
	{
		self.backgroundColor = [NSColor clearColor];
	}
	else
	{

	}
	
	[super highlight:flag withFrame:cellFrame inView:controlView];
}

@end
