/*********************************************************************************************
 **	__________         __           .__            __                 ________   _______   	**
 **	\______   \_____  |  | __  _____|  |__ _____ _/  |______   	___  _\_____  \  \   _  \  	**
 **	 |       _/\__  \ |  |/ / /  ___/  |  \\__  \\   __\__  \  	\  \/ //  ____/  /  /_\  \ 	**
 **	 |    |   \ / __ \|    <  \___ \|   Y  \/ __ \|  |  / __ \_  \   //       \  \  \_/   \	**
 **	 |____|_  /(____  /__|_ \/____  >___|  (____  /__| (____  /	  \_/ \_______ \ /\_____  /	**
 **	        \/      \/     \/     \/     \/     \/          \/ 	              \/ \/     \/ 	**
 **                                                                                         **
 **			This Source Code Form is subject to the terms of the Mozilla Public				**
 **			License, v. 2.0. If a copy of the MPL was not distributed with this				**
 **			file, You can obtain one at https://mozilla.org/MPL/2.0/.						**
 **                                                                                         **
 **                     			© Taiki 2011-2016                                       **
 **                                                                                         **
 *********************************************************************************************/

@interface RakImportStatusListItem : RakOutlineListItem

@property PROJECT_DATA projectData;
@property RakImportItem * itemForChild;
@property byte status;
@property (readonly) BOOL metadataProblem;

@property uint indexOfLastRemoved;

- (instancetype) initWithProject : (PROJECT_DATA) project;
- (void) addItemAsChild : (RakImportItem *) item;
- (void) insertItemAsChild : (RakImportItem *) item atIndex : (uint) index;
- (BOOL) removeChild : (RakImportItem *) item;
- (void) commitFinalList;
- (byte) checkStatusFromChildren;
- (void) checkRefreshStatusRoot;

- (BOOL) canMoveToIndependentNode;
- (BOOL) canConvertToVol;
- (void) convertToVol;
- (BOOL) canConvertToChap;
- (void) convertToChap;

- (void) moveToIndependentNode;
- (void) removeItem;

@end

@interface RakImportStatusList : RakOutlineList
{
	NSArray * _dataSet;
	NSArray * rootItems;
	
	PROJECT_DATA _draggedProject;
}

@property (readonly) BOOL haveDuplicate;
@property (nonatomic) RakImportQuery * query;
@property __weak RakImportStatusController * controller;

- (instancetype) initWithImportList : (NSArray *) dataset;
+ (void) refreshAfterPass;

@end

@interface RakImportStatusListRowView : RakView
{
	RakImportStatusListItem * listItem;

	RakImportItem * _item;
	BOOL isRoot;

#if !TARGET_OS_IPHONE
	RakText * projectName;
	RakStatusButton * button;
#endif

	__weak RakImportQuery * alert;
}

@property RakImportStatusList * list;

- (void) updateWithItem : (RakImportStatusListItem *) item;
- (byte) status;

@end
