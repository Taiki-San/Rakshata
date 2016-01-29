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

enum
{
	AUTH_STATE_GOOD,
	AUTH_STATE_LOADING,
	AUTH_STATE_NONE,
	AUTH_STATE_INVALID
};

enum
{
	AUTH_MODE_DEFAULT,
	AUTH_MODE_NEW_ACCOUNT,
	AUTH_MODE_LOGIN,
};

@interface RakEmailField : RakText <NSTextFieldDelegate>
{
	uint currentEditingSession;
	
	RakAuthController * authController;
}

@property byte currentStatus;

- (void) addController : (RakAuthController *) controller;

@end

@interface RakPassField : RakText <NSTextFieldDelegate>
{
	RakAuthController * authController;
}

- (void) addController : (RakAuthController *) controller;

@property byte currentStatus;

@end

@interface RakAuthText : RakView
{
	RakColor * backgroundColor;
}

@property (strong) NSString * URL;

- (instancetype) initWithText:(NSRect)frame : (NSString *) text : (RakColor *) color;

@end

@interface RakAuthTermsButton : RakSwitchButton

@end

