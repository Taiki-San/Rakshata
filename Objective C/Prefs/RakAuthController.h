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
	AUTHEMAIL_STATE_GOOD,
	AUTHEMAIL_STATE_LOADING,
	AUTHEMAIL_STATE_NONE,
	AUTHEMAIL_STATE_INVALID
};

@class RakAuthController;

@interface RakEmailField : RakText <NSTextFieldDelegate>
{
	byte currentStatus;
	
	RakAuthController * authController;
}

@property uint currentEditingSession;

- (void) addController : (RakAuthController *) controller;

@end

@interface RakPassField : RakText

@end

@interface RakAuthController : NSViewController
{
	IBOutlet NSView * _newAccount;
	IBOutlet NSView * _login;
	
	IBOutlet NSView * container;
	IBOutlet NSView * _containerEmail;
	IBOutlet NSView * _containerPass;
	
	RakForegroundView * foreground;
	
	//Main view elements
	IBOutlet RakText * header;
	
	IBOutlet RakText * labelMail;
	IBOutlet RakText * labelPass;
	
	RakEmailField * mailInput;
	RakPassField * passInput;
	
	//Container
	RakText * footerPlaceholder;
	
	RakButton * forgottenPass;
	RakButton * login;
	
	//Data
	NSSize originalSize;
}

- (void) launch;

- (void) validEmail : (BOOL) newAccount : (uint) session;

@end
