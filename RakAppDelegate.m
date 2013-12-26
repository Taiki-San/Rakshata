//
//  RakAppDelegate.m
//  Interface
//
//  Created by Taiki on 23/12/2013.
//  Copyright (c) 2013 Taiki. All rights reserved.
//

#define SIZE_MIN_HEIGHT 300
#define SIZE_MAX_HEIGHT 800

#define SIZE_MIN_WIDTH 500
#define SIZE_MAX_WIDTH 1200

#import "RakAppDelegate.h"

@implementation RakAppDelegate

- (void)applicationDidFinishLaunching:(NSNotification *)aNotification
{
	[self validateWindowData:[[self window] frame]];
	
	//[[self window] close];
}

- (void) validateWindowData : (NSRect) size
{
	bool needUpdate = false;

	if(size.size.height < SIZE_MIN_HEIGHT)
	{
		size.size.height = SIZE_MIN_HEIGHT;
		needUpdate = true;
	}
	
	if(size.size.height > SIZE_MAX_HEIGHT)
	{
		size.size.height = SIZE_MAX_HEIGHT;
		needUpdate = true;
	}
	
	if(size.size.width < SIZE_MIN_WIDTH)
	{
		size.size.width = SIZE_MIN_WIDTH;
		needUpdate = true;
	}

	if(size.size.width > SIZE_MAX_WIDTH)
	{
		size.size.width = SIZE_MAX_WIDTH;
		needUpdate = true;
	}
	
	if(needUpdate)
		[[self window] setFrame:size display:NO];
}

-(IBAction)showMessage: (id) sender
{
	NSLog(@"showMessage called :)");
}

@end
