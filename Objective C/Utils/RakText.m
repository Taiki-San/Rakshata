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

@implementation RakText

- (id) init
{
	self = [super init];
	
	if(self != nil)
	{
		self.wantsLayer = NO;
		self.editable = NO;
		self.bordered = NO;
		self.drawsBackground = NO;
		self.backgroundColor = [NSColor clearColor];
		self.selectable = NO;
	}
	
	return self;
}

- (id)initWithText:(NSRect)frame : (NSString *) text : (NSColor *) color
{
    self = [self initWithFrame:frame];
    if (self)
	{
		self.wantsLayer = NO;
		self.editable = NO;
		self.bordered = NO;
		self.drawsBackground = NO;
		self.backgroundColor = [NSColor clearColor];
		
		if(color != nil)
			self.textColor = color;
		
		self.stringValue = text;
		self.selectable = NO;
    }
    return self;
}

+ (Class) cellClass
{
	return [RakCenteredTextFieldCell class];
}

- (void) additionalDrawing
{
	
}

- (void) drawRect:(NSRect)dirtyRect
{
	if(self.drawsBackground)
	{
		[self.backgroundColor setFill];
		NSRectFill(dirtyRect);
	}
	
	[super drawRect:dirtyRect];

	[self additionalDrawing];
}

@end
