/*********************************************************************************************
 **	__________         __           .__            __                 ________   _______   	**
 **	\______   \_____  |  | __  _____|  |__ _____ _/  |______   	___  _\_____  \  \   _  \  	**
 **	 |       _/\__  \ |  |/ / /  ___/  |  \\__  \\   __\__  \  	\  \/ //  ____/  /  /_\  \ 	**
 **	 |    |   \ / __ \|    <  \___ \|   Y  \/ __ \|  |  / __ \_  \   //       \  \  \_/   \	**
 **	 |____|_  /(____  /__|_ \/____  >___|  (____  /__| (____  /	  \_/ \_______ \ /\_____  /	**
 **	        \/      \/     \/     \/     \/     \/          \/ 	              \/ \/     \/ 	**
 **                                                                                         **
 **			This Source Code Form is subject to the terms of the Mozilla Public				**
 **			License, v. 2.0. If a copy of the MPL was not distributed with this				**
 **			file, You can obtain one at https://mozilla.org/MPL/2.0/.						**
 **                                                                                         **
 **                     			© Taiki 2011-2016                                       **
 **                                                                                         **
 *********************************************************************************************/

@implementation RakCustomWindow

- (void) createWindow
{
	if(window == nil)
	{
		NSSize size = [[self class] defaultWindowSize];
		
		window = [[RakWindow alloc] initWithContentRect:NSMakeRect(200, 200, size.width, size.height) styleMask:NSTitledWindowMask|NSClosableWindowMask backing:NSBackingStoreBuffered defer:YES];
		
		if(window != nil)
		{
			window.styleMask |= NSTexturedBackgroundWindowMask;
			window.delegate = RakApp;
			window.contentView = [[RakContentViewBack alloc] initWithFrame:NSMakeRect(0, 0, size.width, size.height)];;
			
			[window configure];
			[self fillWindow];
		}
	}
	else
	{
		[self resetWindow];
	}
	
	[window makeKeyAndOrderFront:self];
}

- (BOOL) windowShouldClose:(id)sender
{
	[window orderOut:self];
	return NO;
}

- (void) dealloc
{
	[window close];
}

#pragma mark - Content management

+ (NSSize) defaultWindowSize
{
	return NSZeroSize;
}

- (NSRect) contentFrame : (RakView *) content
{
	return NSMakeRect(WIDTH_BORDER_ALL, WIDTH_BORDER_ALL, content.bounds.size.width - 2 * WIDTH_BORDER_ALL, content.bounds.size.height - 2 * WIDTH_BORDER_ALL);
}

- (void) fillWindow
{
	window.title = @"";
	
	RakView * _contentView = (id) window.contentView;
	
	if(_contentView == nil)
		return;
	else
	{
		contentView = [[[self contentClass] alloc] initWithFrame:[self contentFrame : _contentView]];
		if(contentView == nil)
			return;
		
		[_contentView addSubview:contentView];
	}
}

- (Class) contentClass
{
	return [RakView class];
}

- (void) resetWindow
{
	
}

@end
