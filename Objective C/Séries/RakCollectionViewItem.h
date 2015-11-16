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

@interface RakEditDetails : RakImportQuery
{
	RakSheetWindow * queryWindow;
	RakSheetView * coreview;
}

- (instancetype) initWithProject : (PROJECT_DATA) project;
- (void) queryUser;

@end

@interface RakCollectionViewItem : RakSRProjectView
{
	uint * _sharedActive;
	uint sessionCode;
	
	//superCriticalFlag activate a small dumb easter egg :P
	BOOL superCriticalFlag;
	
	//Content
	RakText * mainTag;
}

@property BOOL selected;
@property CGFloat requestedHeight;

- (instancetype) initWithProject : (PROJECT_DATA) project : (uint *) sharedActive;
- (void) updateFocus;

@end
