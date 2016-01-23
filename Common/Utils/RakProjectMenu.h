/*********************************************************************************************
 **	__________         __           .__            __                 ________   _______   	**
 **	\______   \_____  |  | __  _____|  |__ _____ _/  |______   	___  _\_____  \  \   _  \  	**
 **	 |       _/\__  \ |  |/ / /  ___/  |  \\__  \\   __\__  \  	\  \/ //  ____/  /  /_\  \ 	**
 **	 |    |   \ / __ \|    <  \___ \|   Y  \/ __ \|  |  / __ \__ \   //       \  \  \_/   \	**
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

@interface RakProjectMenu : NSObject
{
	PROJECT_DATA _project;
	
	RakView * _view;
}

- (instancetype) initWithProject : (PROJECT_DATA) project;

- (void) configureMenu : (NSMenu *) menu;
- (void) configureCleanMenu : (BOOL) optionPressed toView : (RakView *) view;
- (void) configureMenu : (NSMenu *) menu withOption : (BOOL) optionPressed;

- (void) deleteSerie;

@end

@interface RakEditDetails : RakImportQuery
{
	RakSheetWindow * queryWindow;
	RakSheetView * coreview;
}

- (instancetype) initWithProject : (PROJECT_DATA) project;
- (void) queryUser;

@end

