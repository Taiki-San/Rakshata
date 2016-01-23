/*********************************************************************************************
 **	__________         __           .__            __                 ________   _______   	**
 **	\______   \_____  |  | __  _____|  |__ _____ _/  |______   	___  _\_____  \  \   _  \  	**
 **	 |       _/\__  \ |  |/ / /  ___/  |  \\__  \\   __\__  \  	\  \/ //  ____/  /  /_\  \ 	**
 **	 |    |   \ / __ \|    <  \___ \|   Y  \/ __ \|  |  / __ \__ \   //       \  \  \_/   \	**
 **	 |____|_  /(____  /__|_ \/____  >___|  (____  /__| (____  /	  \_/ \_______ \ /\_____  /	**
 **	        \/      \/     \/     \/     \/     \/          \/ 	              \/ \/     \/ 	**
 **                                                                                         **
 **			This Source Code Form is subject to the terms of the Mozilla Public				**
 **			License, v. 2.0. If a copy of the MPL was not distributed with this				**
 **			file, You can obtain one at http://mozilla.org/MPL/2.0/.						**
 **                                                                                         **
 **                     			© Taiki 2011-2016                                       **
 **                                                                                         **
 *********************************************************************************************/

#define TITLE_BAR_HEIGHT 22

@interface RakWindow : NSWindow <NSDraggingDestination>
{
	BOOL _sheetManipulation;
}

@property (getter=isMainWindow) BOOL isMainWindow;

//Event data
@property BOOL shiftPressed;
@property BOOL optionPressed;
@property BOOL controlPressed;
@property BOOL functionPressed;
@property BOOL commandPressed;

@property (readonly, getter=isFullscreen) BOOL fullscreen;
@property (weak) RakView* defaultDispatcher;
@property (strong) NSResponder* imatureFirstResponder;

- (void) configure;
- (void) registerForDrop;

- (void) resetTitle;
- (void) setProjectTitle : (PROJECT_DATA) project;
- (void) setCTTitle : (PROJECT_DATA) project : (NSString *) element;

@end
