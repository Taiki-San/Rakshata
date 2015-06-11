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

#include "RakAuthTools.h"

@interface RakAuthController : NSViewController
{
	IBOutlet NSView * container;
	IBOutlet NSView * _containerMail;
	IBOutlet NSView * _containerPass;
	
	RakAuthForegroundView * foreground;
	
	//Main view elements
	IBOutlet RakText * header;
	
	IBOutlet RakText * labelMail;
	IBOutlet RakText * labelPass;
	
	RakEmailField * mailInput;
	RakPassField * passInput;
	
	//Container
	RakText * footerPlaceholder;
	
	//Login
	RakButton * forgottenPass, * _login;
	
	//Signup
	RakClickableText * privacy, * terms;
	RakAuthTermsButton * accept;
	RakButton * confirm;
	
	//Data
	byte currentMode;
	BOOL initialAnimation;
	
	CGFloat baseHeight;
	CGFloat baseContainerHeight;
}

@property BOOL postProcessing;
@property BOOL offseted;

- (void) launch;

- (void) wakePassUp;
- (void) validEmail : (BOOL) newAccount : (uint) session;

- (void) switchOver : (NSNumber*) isDisplayed;
- (void) focusLeft : (id) caller : (NSUInteger) flag;

@end
