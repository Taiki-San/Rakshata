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
 ********************************************************************************************/

NSRect prefsPercToFrame(NSRect percentage, NSSize superview);
CGFloat percToSize(CGFloat percentage, CGFloat superview, CGFloat max);

NSArray * loadCustomColor(const char * file);

enum RAKBUTTON_STATES
{
	RB_STATE_STANDARD		= NSOffState,
	RB_STATE_HIGHLIGHTED	= NSOnState,
	RB_STATE_UNAVAILABLE	= NSMixedState
};

@interface RakResPath : NSObject

+ (NSImage *) getImage: (NSString*) baseName;

@end
