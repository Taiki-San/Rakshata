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

@interface RakSRSearchList : RakList
{
	byte _type;
	BOOL _manualSelection;
	
	uint64_t * indexes;
	
	BOOL lastWasSelected;
	NSMutableArray * selection;
}

- (instancetype) init : (NSRect) frame ofType : (byte) type withData: (charType **) dataList ofSize : (uint) nbDataList andIndexes : (uint64_t *) indexes;

- (void) updateData : (charType **) names ofSize : (uint) nbData andIndexes : (uint64_t *) listIndexes;

- (void) fullCleanup;

@end
