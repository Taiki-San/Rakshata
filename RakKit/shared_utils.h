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

#if TARGET_OS_OSX
	#define RakView		NSView
	#define RakImage	NSImage
	#define RakColor	NSColor

	#define RakApplication	NSApplication
	#define RakRealApp		NSApp

	#define CONFIGURE_APPEARANCE_DARK(__view) if(__view != nil && floor(NSAppKitVersionNumber) >= NSAppKitVersionNumber10_10) __view.appearance = [NSAppearance appearanceNamed:NSAppearanceNameVibrantDark]

#endif

NSRect prefsPercToFrame(NSRect percentage, NSSize superview);
CGFloat percToSize(CGFloat percentage, CGFloat superview, CGFloat max);

RakImage * getResImageWithName(NSString* baseName);
