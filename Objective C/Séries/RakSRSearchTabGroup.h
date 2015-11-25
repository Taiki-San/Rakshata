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

@interface RakSRSearchTabGroup : NSView
{
	byte _ID;
	
	RakSRSearchList * list;
	RakSRSearchBar * searchBar;
	
	//Used by extra
	RakButton * close, *flush;
	RakSwitchButton * freeSwitch, * favsSwitch;
	RakText * freeText, * favsText;
	
	charType ** listData, **_cachedListData;
	uint nbDataList, _cachedNbDataList;
	uint64_t * indexesData, * _cachedIndexesData, sessionID;
	
	BOOL manualUpdateNoNotif;
}

- (instancetype) initWithFrame:(NSRect)frameRect : (byte) ID;
- (void) resizeAnimation:(NSRect)frameRect;

- (RakSRSearchBar *) searchBar;

@end
