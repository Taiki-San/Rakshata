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

@class RakAuthController;

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

@interface RakAuthController : NSViewController
{
	IBOutlet NSView * container;
	IBOutlet NSView * _containerMail;
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
	
		//Login
	RakButton * forgottenPass;
	RakButton * _login;
	
		//Signup
	RakTextClickable * privacy;
	RakTextClickable * terms;
	NSButton * accept;
	RakButton * confirm;
	
	//Data
	byte currentMode;
	BOOL initialAnimation;
	
	CGFloat baseHeight;
	CGFloat baseContainerHeight;
}

@property BOOL postProcessing;

- (void) launch;

- (void) validEmail : (BOOL) newAccount : (uint) session;

@end
