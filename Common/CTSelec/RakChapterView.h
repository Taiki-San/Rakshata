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
 **			file, You can obtain one at http://mozilla.org/MPL/2.0/.						**
 **                                                                                         **
 **                     			© Taiki 2011-2016                                       **
 **                                                                                         **
 ********************************************************************************************/

#include "RakCTSelection.h"

@interface RakChapterView : RakTabContentTemplate
{
	//SR mode only
	RakSRSuggestions * suggestions;
	RakSRDetails * mainDetailView, * tmpDetailView;
	
	//CT mode only
	RakCTHeader * header;
	
	//Reader mode only
	RakMenuText * projectName;
	RakCTProjectImageView * projectImage;

	RakCTSelection * coreview;
}

- (instancetype) initContent:(NSRect)frame : (PROJECT_DATA) project : (BOOL) isTome : (long [4]) context;

- (void) updateContext : (PROJECT_DATA) data;
- (void) projectDataUpdate : (PROJECT_DATA) oldData : (PROJECT_DATA) newData;
- (void) selectElem : (uint) projectID : (BOOL) isTome : (uint) element;
- (PROJECT_DATA) activeProject;

@end

enum
{
	CT_READERMODE_LATERAL_BORDER = 5, // %
	CT_READERMODE_BOTTOMBAR_WIDTH = 14,
	CT_READERMODE_WIDTH_PROJECT_NAME = 25,
	CT_READERMODE_WIDTH_SPACE_NEXT_PROJECTIMAGE = 20,
	CT_READERMODE_HEIGHT_PROJECT_IMAGE = 150,
	CT_READERMODE_HEIGHT_HEADER_TAB = (CT_READERMODE_WIDTH_PROJECT_NAME + 2 * CT_READERMODE_WIDTH_SPACE_NEXT_PROJECTIMAGE + CT_READERMODE_HEIGHT_PROJECT_IMAGE),
	CT_READERMODE_HEIGHT_CT_BUTTON = 25,
	CT_READERMODE_HEIGHT_BORDER_TABLEVIEW = 10,
	CT_READERMODE_BORDER_TABLEVIEW = 10
};