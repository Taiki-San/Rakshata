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

//Don't forget to reflect any addition to +haveAnyRemindedValue
#define PREFS_REMIND_DELETE			@"DefaultPopoverDelete"
#define PREFS_REMIND_AUTODL			@"DefaultPopoverAutoDL"
#define PREFS_REMIND_FAVS			@"PopoverFavoriteDisplayed"
#define PREFS_REMIND_SUGGESTION		@"GetThoseDamnSuggestionsOutOfTheWay"

@interface RakPrefsRemindPopover : NSObject

+ (void) setValueReminded : (NSString *) element : (BOOL) value;
+ (BOOL) getValueReminded : (NSString *) element : (BOOL *) value;
+ (void) removeValueReminded : (NSString *) element;

+ (BOOL) haveAnyRemindedValue;
+ (void) flushRemindedValues;

@end
