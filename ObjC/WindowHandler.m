/*********************************************************************************************
 **	__________         __           .__            __                 ________   _______   	**
 **	\______   \_____  |  | __  _____|  |__ _____ _/  |______   	___  _\_____  \  \   _  \  	**
 **	 |       _/\__  \ |  |/ / /  ___/  |  \\__  \\   __\__  \  	\  \/ //  ____/  /  /_\  \ 	**
 **	 |    |   \ / __ \|    <  \___ \|   Y  \/ __ \|  |  / __ \__ \   //       \  \  \_/   \	**
 **	 |____|_  /(____  /__|_ \/____  >___|  (____  /__| (____  /	  \_/ \_______ \ /\_____  /	**
 **	        \/      \/     \/     \/     \/     \/          \/ 	              \/ \/     \/ 	**
 **                                                                                          **
 **    Licence propriétaire, code source confidentiel, distribution formellement interdite   **
 **                                                                                          **
 *********************************************************************************************/

#import <Cocoa/Cocoa.h>
#include "WindowHandler.h"

@implementation Windows


- (NSWindow*)createWindow : (int) w : (int) h : (int) x : (int) y : (NSString*) title
{
	windowDimension = NSMakeRect(x, y, w, h);
	window = [[NSWindow alloc] initWithContentRect:windowDimension
											   styleMask:NSTitledWindowMask|NSTitledWindowMask|NSMiniaturizableWindowMask|NSResizableWindowMask
												 backing:NSBackingStoreBuffered
												   defer:NO];
	[window setTitle:title];
	[window makeKeyAndOrderFront:NSApp];
		
	return window;
}

- (void) setFlag : (uint32_t) flagArg	{	flag = flagArg;	}
- (uint32_t) getFlag	{	return flag;	}

- (int) getWidth	{	return windowDimension.size.width;	}
- (int) getHeight	{	return windowDimension.size.height;	}

- (id) init : (int) w : (int) h : (int) x : (int) y : (NSString*) title : (int) flagArg
{
	NSWindow * newWindow = [self createWindow: 800 : 150 : 6 : 100 : @"SuperTitleName"];
	flag = flagArg;
	return self;
}

@end
