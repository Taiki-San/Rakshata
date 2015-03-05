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

@implementation RakPrefsWindow

+ (NSSize) defaultWindowSize
{
	return NSMakeSize(PREF_WINDOW_WIDTH, PREF_WINDOW_HEIGHT);
}

- (NSRect) contentFrame : (NSView *) content
{
	NSRect frame = [super contentFrame:content];
	
	CGFloat offset = frame.origin.x;
	
	frame.origin.x -= offset - 2;
	frame.origin.y -= offset;
	frame.size.height += 2 * offset - 2;
	frame.size.width += 2 * offset - 4;
	
	return frame;
}

- (void) fillWindow
{
	[super fillWindow];
	
	activeView = PREFS_BUTTON_CODE_UNUSED;

	header = [[RakPrefsButtons alloc] initWithFrame : NSMakeRect(0, 0, PREF_WINDOW_WIDTH, PREF_BUTTON_BAR_HEIGHT) :self];
	if(header != nil)
	{
		[header selectElem:PREFS_BUTTON_CODE_REPO];
		[contentView addSubview:header];
	}
	
	[self focusChanged:PREFS_BUTTON_CODE_REPO];
}

- (Class) contentClass
{
	return [RakFlippedView class];
}

#pragma mark - Drawing

- (NSRect) mainFrame
{
	NSRect frame = contentView.bounds;
	
	frame.origin.y = PREF_BUTTON_BAR_HEIGHT;
	frame.size.height -= PREF_BUTTON_BAR_HEIGHT + 4;
	
	return frame;
}

- (NSColor *) textColor
{
	return [Prefs getSystemColor:GET_COLOR_CLICKABLE_TEXT :nil];
}

#pragma mark - Buttons responder

- (void) focusChanged : (byte) newTab
{
	NSView * old = [self viewForCode : activeView : NO], * new = [self viewForCode : newTab : YES];
	
	if(old == nil)
	{
		NSRect newWindowFrame = window.frame;
		const CGFloat diff = contentView.bounds.size.height - PREF_BUTTON_BAR_HEIGHT - new.bounds.size.height;
		newWindowFrame.size.height -= diff;
		newWindowFrame.origin.y = MAX(0, newWindowFrame.origin.y + diff);

		[window setFrame:newWindowFrame display:YES animate:NO];
	}
	else
	{
		new.alphaValue = 0;
		new.hidden = NO;
		
		[NSAnimationContext runAnimationGroup:^(NSAnimationContext *context) {
			old.animator.alphaValue = 0;
		} completionHandler:^{
			
			NSRect newWindowFrame = window.frame;

			CGFloat diff = old.bounds.size.height - new.bounds.size.height;
			newWindowFrame.size.height -= diff;
			newWindowFrame.origin.y = MAX(0, newWindowFrame.origin.y + diff);
			
			diff = old.bounds.size.width - new.bounds.size.width;
			newWindowFrame.size.width -= diff;
			newWindowFrame.origin.x = MAX(0, newWindowFrame.origin.x + diff);
			
			[NSAnimationContext runAnimationGroup:^(NSAnimationContext *context) {

				[window.animator setFrame:newWindowFrame display:YES animate:YES];
				[header.animator setFrameSize:NSMakeSize(newWindowFrame.size.width - 4, PREF_BUTTON_BAR_HEIGHT)];
		 	
			} completionHandler:^{
				
				[new setFrameOrigin:NSMakePoint(0, PREF_BUTTON_BAR_HEIGHT)];
				
				[NSAnimationContext runAnimationGroup:^(NSAnimationContext *context) {
					new.animator.alphaValue = 1;
				} completionHandler:^{
					old.hidden = YES;
				}];
			}];
		}];
	}
	
	activeView = newTab;
}

- (NSView *) viewForCode : (byte) code : (BOOL) createIfNeeded
{
	switch (code)
	{
		case PREFS_BUTTON_CODE_GENERAL:
		{
			if(generalView != nil)
				return generalView;

			else if(createIfNeeded)
			{
				generalView = [[NSView alloc] initWithFrame:[self mainFrame]];
				
				if(generalView != nil)
					[contentView addSubview:generalView];
				
				return generalView;
			}
			
			break;
		}

		case PREFS_BUTTON_CODE_REPO:
		{
			if(generalView != nil)
				return repoView;
			
			else if(createIfNeeded)
			{
				repoView = [[RakPrefsRepoView alloc] init];
				
				if(repoView != nil)
					[contentView addSubview:repoView];
				
				return repoView;
			}
			
			break;
		}
			
		case PREFS_BUTTON_CODE_FAV:
		{
			if(favoriteView != nil)
				return favoriteView;
			
			else if(createIfNeeded)
			{
				favoriteView = [[NSView alloc] initWithFrame:[self mainFrame]];
				
				if(favoriteView != nil)
					[contentView addSubview:favoriteView];
				
				return favoriteView;
			}
			
			break;
		}
			
		case PREFS_BUTTON_CODE_CUSTOM:
		{
			if(customView != nil)
				return customView;
			
			else if(createIfNeeded)
			{
				customView = [[NSView alloc] initWithFrame:[self mainFrame]];
				
				if(customView != nil)
					[contentView addSubview:customView];
				
				return customView;
			}
			
			break;
		}
  	}
	
	return nil;
}

@end
