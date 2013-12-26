//
//  RakAppDelegate.h
//  Interface
//
//  Created by Taiki on 23/12/2013.
//  Copyright (c) 2013 Taiki. All rights reserved.
//

#import <Cocoa/Cocoa.h>

@interface NSViewCustom : NSView
{
	int flag;
}

@end

@interface RakAppDelegate : NSObject <NSApplicationDelegate>

@property (assign) IBOutlet NSWindow *window;

- (void) validateWindowData : (NSRect) size;
-(IBAction)showMessage: (id) sender;

@end
