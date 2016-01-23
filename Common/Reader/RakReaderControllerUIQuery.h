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

@interface RakReaderControllerUIQuery : RakPopoverView
{
	PROJECT_DATA _project;
	BOOL _isTome;
	uint* _arraySelection;
	uint _sizeArray;
	
	Reader* __weak _tabReader;
	
	BOOL _remind;
}

- (instancetype) initWithData : (MDL*) tabMDL : (PROJECT_DATA) project : (BOOL) isTome : (uint *) arraySelection : (uint) sizeArray;

- (void) locationUpdated : (NSRect) MDLFrame : (BOOL) animated;

@end

@interface RakQuerySegmentedControl : RakMinimalSegmentedControl

- (instancetype) initWithData : (NSRect) frame : (NSString *) agree : (NSString *) disagree;

@end
