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

@interface RakReaderControllerUIQuery : NSView <INPopoverControllerDelegate>
{
	MANGAS_DATA _project;
	BOOL _isTome;
	int* _arraySelection;
	uint _sizeArray;
	
	Reader* _tabReader;
	MDL* _tabMDL;
	
	bool _remind;
	
	RakPopoverWrapper * popover;
}

- (id) initWithData : (MDL*) tabMDL : (MANGAS_DATA) project : (BOOL) isTome : (int*) arraySelection : (uint) sizeArray;
- (void) locationUpdated : (NSRect) MDLFrame : (BOOL) animated;

@end

@interface RakQuerySegmentedControl : NSSegmentedControl

@end