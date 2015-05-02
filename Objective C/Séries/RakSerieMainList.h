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

@interface RakSerieMainList : RakList
{
	uint _nbElemFull;
	bool* _installed;
	uint _nbElemInstalled;
	uint* _jumpToInstalled;
	
	BOOL installedRequested;
}

@property (setter=setInstallOnly:, getter=installOnlyMode) BOOL installOnlyMode;

- (id) init : (NSRect) frame : (NSInteger) selectedDBID : (NSInteger) scrollPosition : (BOOL) installOnly;

- (void) updateJumpTable;
- (PROJECT_DATA) getElementAtIndex : (NSInteger) index;

@end
