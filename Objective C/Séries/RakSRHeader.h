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

@class Series;

@interface RakSRHeader : NSView
{
	RakButton *preferenceButton;
	RakButtonMorphic * displayType;
	RakButton * storeSwitch;
	RakSRSearchBar * search;
	
	RakBackButton * backButton;
	
	PrefsUI * winController;
	
	BOOL _haveFocus;
	
	CGFloat _separatorX;
}

@property (weak) Series * responder;
@property CGFloat height;
@property BOOL prefUIOpen;

- (instancetype) initWithFrame : (NSRect) frameRect : (BOOL) haveFocus;

- (void) updateFocus : (uint) mainThread;

@end


enum
{
	SR_CELLTYPE_GRID = 0,
	SR_CELLTYPE_REPO = 1,
	SR_CELLTYPE_LIST = 2,
	SR_CELLTYPE_MAX = SR_CELLTYPE_LIST
};