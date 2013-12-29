//
//  RakAppDelegate.m
//  Interface
//
//  Created by Taiki on 23/12/2013.
//  Copyright (c) 2013 Taiki. All rights reserved.
//

#define SIZE_MIN_HEIGHT 300
#define SIZE_MIN_WIDTH 500

#import "Series.h"
#import "RakAppDelegate.h"

@implementation RakAppDelegate

- (void)applicationDidFinishLaunching:(NSNotification *)aNotification
{
	[self validateWindowData:[[self window] frame]];
	Series* tabSerie = [[Series alloc] init:self.window];
	[tabSerie drawRect:NSMakeRect(0, 0, tabSerie.frame.size.width, tabSerie.frame.size.height)];
	
	//[[self window] close];
	//[tabSerie release];
}

- (void) validateWindowData : (NSRect) size
{
	bool needUpdate = false;

	if(size.size.height < SIZE_MIN_HEIGHT)
	{
		size.size.height = SIZE_MIN_HEIGHT;
		needUpdate = true;
	}

	if(size.size.width < SIZE_MIN_WIDTH)
	{
		size.size.width = SIZE_MIN_WIDTH;
		needUpdate = true;
	}
	
	if(needUpdate)
		[[self window] setFrame:size display:NO];
}

@end

@implementation NSViewCustom

@end
