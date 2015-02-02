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

#include "db.h"

#define SHADOW_HEIGHT 8

@implementation RakSRSearchTab

- (instancetype) initWithFrame:(NSRect)frameRect
{
	self = [super initWithFrame:frameRect];
	
	if(self != nil)
	{
		_isVisible = NO;
		_collapsed = YES;
		_height = SRSEARCHTAB_DEFAULT_HEIGHT;
		
		[Prefs getCurrentTheme:self];

		self.wantsLayer = YES;
		self.layer.borderWidth = 1;
		self.layer.borderColor = [self getBorderColor].CGColor;
		self.layer.backgroundColor = [self getBackgroudColor].CGColor;
		
		placeholder = [[RakText alloc] initWithText:frameRect :@"Afficher les outils de recherche avancée" : [self placeholderTextColor]];
		if(placeholder != nil)
		{
			placeholder.font = [self placeholderFont];

			[placeholder sizeToFit];
			[placeholder setFrameOrigin:NSCenterPoint(frameRect, placeholder.bounds)];
			
			[self addSubview:placeholder];
		}
		
		[self initContent];
		
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
				[_gradients addObject:gradient];
			}
			else
				[_gradients addObject:@(0)];
		}
		gradients = [NSArray arrayWithArray:_gradients];
		
		[[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(searchWasTriggered:) name:SR_NOTIF_NAME_SEARCH_TRIGGERED object:nil];
	}
	
	return self;
}

- (void) initContent
{
	author = [[RakSRSearchTabGroup alloc] initWithFrame:[self getBlockFrame : _bounds : 1] :SEARCH_BAR_ID_AUTHOR];
	if(author != nil)
	{
		author.hidden = YES;
		[self addSubview:author];
	}
}

- (void) dealloc
{
	[[NSNotificationCenter defaultCenter] removeObserver:self];
}

#pragma mark - Drawing and view manipulation

- (void) setFrame : (NSRect) frameRect
{
	CGFloat oldWidth = self.bounds.size.width;
	
	[super setFrame:frameRect];
	
	[placeholder setFrameOrigin:NSCenterPoint(frameRect, placeholder.bounds)];
	
	if(oldWidth != frameRect.size.width)
	{
		[author setFrame:[self getBlockFrame:frameRect :1]];
	}
	
	byte position = 0;
	for(RakGradientView * gradient in gradients)
	{
		if([gradient class] == [RakGradientView class])
			gradient.frame = [self getShadowFrame:frameRect :position];
		position++;
	}
}

- (void) resizeAnimation : (NSRect) frameRect
{
	CGFloat oldWidth = self.bounds.size.width;

	[self.animator setFrame:frameRect];
	[placeholder.animator setFrameOrigin:NSCenterPoint(frameRect, placeholder.bounds)];
	
	if(oldWidth != frameRect.size.width)
	{
		[author resizeAnimation:[self getBlockFrame:frameRect :1]];
	}
	
	byte position = 0;
	for(RakGradientView * gradient in gradients)
	{
		gradient.animator.frame = [self getShadowFrame:frameRect :position++];
	}
}

- (void) drawRect:(NSRect)dirtyRect
{
	if(_isVisible && _collapsed)
	{
		[[[NSColor blackColor] colorWithAlphaComponent:0.3] setFill];
		NSRectFill(dirtyRect);
	}
}

#pragma mark - Frame

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

#define BORDER_HORIZON 	10
#define BORDER_VERT 	25

- (NSRect) getBlockFrame : (NSRect) frame : (byte) position
{
	frame.size.height = SR_SEARCH_TAB_EXPANDED_HEIGHT - 2 * BORDER_HORIZON;
	
	frame.size.width -= BORDER_VERT;
	frame.size.width /= 4;
	frame.size.width *= position;
	frame.size.width -= BORDER_VERT;
	
	frame.origin.x = BORDER_VERT;
	frame.origin.y = BORDER_HORIZON;
	
	return frame;
}

#pragma mark - Colors

- (NSColor *) getBorderColor
{
	return [NSColor blackColor];
}

- (NSColor *) getBackgroudColor
{
	return [Prefs getSystemColor:GET_COLOR_SEARCHTAB_BACKGROUND :nil];
}

- (NSColor *) placeholderTextColor
{
	return [[Prefs getSystemColor:GET_COLOR_ACTIVE :nil] colorWithAlphaComponent:0.6];
}

- (NSFont *) placeholderFont
{
	return [NSFont fontWithName:[Prefs getFontName:GET_FONT_PLACEHOLDER] size:14];
}

- (void) observeValueForKeyPath:(NSString *)keyPath ofObject:(id)object change:(NSDictionary *)change context:(void *)context
{
	if([object class] != [Prefs class])
		return;

	placeholder.font = [self placeholderFont];
	[placeholder sizeToFit];
	[placeholder setFrameOrigin:NSCenterPoint(self.bounds, placeholder.bounds)];

	placeholder.textColor = [self placeholderTextColor];
	self.layer.borderColor = [self getBorderColor].CGColor;
	self.layer.backgroundColor = [self getBackgroudColor].CGColor;
}

#pragma mark Interactions

- (void) mouseDown:(NSEvent *)theEvent
{
	if (!_isVisible)
		return;

	[self updateCollapseState:!_collapsed : NO];
	[self updateGeneralFrame];
}

- (void) updateCollapseState : (BOOL) newIsCollapsed : (BOOL) silentUpdate
{
	if(_collapsed != newIsCollapsed)
		_collapsed = newIsCollapsed;
	else
		return;
	
	if(silentUpdate)
	{
		[placeholder setHidden:!_collapsed];
		[author setHidden:_collapsed];
		return;
	}
	
	[NSAnimationContext beginGrouping];
	
	author.hidden = _collapsed;
	placeholder.animator.alphaValue = _collapsed;
	
	__block BOOL getCollapsed = _collapsed;
	[[NSAnimationContext currentContext] setCompletionHandler:^{
		[placeholder setHidden:!getCollapsed];
		placeholder.alphaValue = 1;
	}];
	
	[NSAnimationContext endGrouping];
	
	if(_collapsed)
		_height = SR_SEARCH_TAB_INITIAL_HEIGHT;
	else
		_height = SR_SEARCH_TAB_EXPANDED_HEIGHT;
}

- (void) updateGeneralFrame
{
	if([self.superview class] == [Series class])
		[(Series *) self.superview resetFrameSize:YES];
}

#pragma mark - Interface with header

- (void) searchWasTriggered : (NSNotification *) notification
{
	NSNumber * ID = notification.object;
	if(ID == nil || ![ID isKindOfClass:[NSNumber class]])
		return;
	
	NSDictionary * dict = notification.userInfo;
	NSNumber * number;
	
	if(dict != nil 	&& (number = [dict objectForKey:SR_NOTIF_NEW_STATE]) != nil && [number isKindOfClass:[NSNumber class]] && _isVisible != number.boolValue)
	{
		if([ID unsignedCharValue] == SEARCH_BAR_ID_MAIN_TRIGGERED || [ID unsignedCharValue] ==  SEARCH_BAR_ID_AUTHOR_TRIGGERED)
			[self mainSearchWasTriggered:number.boolValue];
	}
}

- (void) mainSearchWasTriggered : (BOOL) isVisible
{
	//We need to check if the focus is not just changing to an other search bar
	if(!isVisible && [((RakWindow *)self.window).imatureFirstResponder class] == [RakSRSearchBar class])
	{
		//Basic check, if the previous firstResponder was a search bar, we block the request
		if([self.window.firstResponder class] == [RakWindow class])
			return;
	}
	
	_isVisible = isVisible;
	
	if(_isVisible)
	{
		[self updateCollapseState : YES : YES];
		_height = SR_SEARCH_TAB_INITIAL_HEIGHT;
	}
	else
		_height = SRSEARCHTAB_DEFAULT_HEIGHT;
	
	[self updateGeneralFrame];
}

@end
