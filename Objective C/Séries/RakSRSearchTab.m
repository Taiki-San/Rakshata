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

#define SHADOW_HEIGHT 8

@implementation RakSRSearchTab

- (instancetype) initWithFrame:(NSRect)frameRect
{
	self = [super initWithFrame:frameRect];
	
	if(self != nil)
	{
		_isVisible = NO;
		_height = SRSEARCHTAB_DEFAULT_HEIGHT;
		
		self.wantsLayer = YES;
		self.layer.borderWidth = 1;
		self.layer.borderColor = [NSColor blackColor].CGColor;
		self.layer.backgroundColor = [NSColor grayColor].CGColor;
		
		menuText = [[RakMenuText alloc] initWithText:frameRect :@"Liste de filtres & co"];
		if(menuText != nil)
		{
			[menuText sizeToFit];
			
			[menuText setFrameOrigin:NSMakePoint(frameRect.size.width / 2 - menuText.bounds.size.width / 2, frameRect.size.height / 2 - menuText.bounds.size.height / 2)];
			[self addSubview:menuText];
		}
		
		NSMutableArray * _gradients = [NSMutableArray arrayWithCapacity:4];
		for(byte position = 0; position < 4; position++)
		{
			RakGradientView * gradient = [[RakGradientView alloc] initWithFrame:[self getShadowFrame : frameRect : position]];
			if(gradient != nil)
			{
				[gradient initGradient];
				gradient.gradientWidth = 1;
				gradient.gradientMaxWidth = 0;
				gradient.angle = position * 90;
				
				[self addSubview:gradient];
			}
			
			[_gradients addObject:gradient];
		}
		gradients = [NSArray arrayWithArray:_gradients];
		
		[[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(searchWasTriggered:) name:SR_NOTIF_NAME_SEARCH_TRIGGERED object:nil];
	}
	
	return self;
}

#pragma mark - Drawing and view manipulation

- (void) setFrame : (NSRect) frameRect
{
	[super setFrame:frameRect];
	
	[menuText setFrameOrigin:NSMakePoint(frameRect.size.width / 2 - menuText.bounds.size.width / 2, frameRect.size.height / 2 - menuText.bounds.size.height / 2)];
	
	byte position = 0;
	for(RakGradientView * gradient in gradients)
	{
		gradient.frame = [self getShadowFrame:frameRect :position++];
	}
}

- (void) resizeAnimation : (NSRect) frameRect
{
	[self.animator setFrame:frameRect];
	
	[menuText.animator setFrameOrigin:NSMakePoint(frameRect.size.width / 2 - menuText.bounds.size.width / 2, frameRect.size.height / 2 - menuText.bounds.size.height / 2)];
	
	byte position = 0;
	for(RakGradientView * gradient in gradients)
	{
		gradient.animator.frame = [self getShadowFrame:frameRect :position++];
	}
}

//Position: 0 = left, 1 = bottom, 2 = right, 3 = top
- (NSRect) getShadowFrame : (NSRect) frame : (byte) position
{
	frame.origin.x = position == 2 ? frame.size.width - SHADOW_HEIGHT : 0;
	frame.origin.y = position == 3 ? frame.size.height - SHADOW_HEIGHT : 0;
	
	if(frame.origin.x < 0)		frame.origin.x = 0;
	if(frame.origin.y < 0)		frame.origin.y = 0;
	
	if(position & 1)
		frame.size.height = SHADOW_HEIGHT;
	else
		frame.size.width = SHADOW_HEIGHT;
	
	return frame;
}

#pragma mark - Interface with header

- (void) searchWasTriggered : (NSNotification *) notification
{
	NSDictionary * dict = notification.userInfo;
	NSNumber * number;
	
	if(dict != nil 	&& (number = [dict objectForKey:SR_NOTIF_NEW_STATE]) != nil && [number isKindOfClass:[NSNumber class]])
	{
		_isVisible = number.boolValue;
		
		if(_isVisible)
			_height = SR_SEARCH_TAB_INITIAL_HEIGHT;
		else
			_height = SRSEARCHTAB_DEFAULT_HEIGHT;
		
		if([self.superview class] == [Series class])
			[(Series *) self.superview resetFrameSize:YES];
	}
}

@end
