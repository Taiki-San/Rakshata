//
//  RakKit.h
//  RakKit
//
//  Created by Taiki on 19/11/2016.
//  Copyright © 2016 Taiki. All rights reserved.
//

#import <Cocoa/Cocoa.h>

//! Project version number for RakKit.
FOUNDATION_EXPORT double RakKitVersionNumber;

//! Project version string for RakKit.
FOUNDATION_EXPORT const unsigned char RakKitVersionString[];

// In this header, you should import all the public headers of your framework using statements like #import <RakKit/PublicHeader.h>

#import <RakKit/Prefs.h>
#import <RakKit/RakView.h>


#import <RakKit/RakTextCell.h>
#import <RakKit/RakText.h>
#import <RakKit/RakMenuText.h>

#define CONFIGURE_APPEARANCE_DARK(__view) if(__view != nil && floor(NSAppKitVersionNumber) >= NSAppKitVersionNumber10_10) __view.appearance = [NSAppearance appearanceNamed:NSAppearanceNameVibrantDark]
