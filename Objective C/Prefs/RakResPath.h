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

#define NAME_ICON_FAV @"RD_favorite",
#define NAME_ICON_FULLSCREEN	@"RD_fullscreen"
#define NAME_ICON_PREVCHAP		@"RD_prev_chapter"
#define NAME_ICON_PREVPAGE		@"RD_prev_page"
#define NAME_ICON_NXTPAGE		@"RD_next_page"
#define NAME_ICON_NXTCHAP		@"RD_next_chapter"
#define NAME_ICON_TRASH			@"RD_trash"

enum RAKBUTTON_STATES
{
	RB_STATE_STANDARD		= NSOffState,
	RB_STATE_HIGHLIGHTED	= NSOnState,
	RB_STATE_UNAVAILABLE	= NSMixedState
};

@interface RakResPath : NSObject

+ (NSImage *) craftResNameFromContext: (NSString*) baseName : (BOOL) highlighted : (BOOL) available : (uint) themeID;

@end
