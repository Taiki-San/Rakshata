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

@interface RakAuthTermsButton : NSButton

@property (assign) RakAuthController* controller;

@end

