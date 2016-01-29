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

@class RakAboutWindow;

@interface RakAppDelegate : NSObject
{
	Series * tabSerie;
	CTSelec * tabCT;
	Reader * tabReader;
	MDL * tabMDL;
	
	RakContentView * _contentView;
	
	BOOL loginPromptOpen;
	pthread_cond_t loginLock;
	MUTEX_VAR loginMutex;
}

@property BOOL initialized;
@property BOOL haveDistractionFree;

- (RakView *) earlyInit;
- (void) mainInit : (RakView *) contentView;

- (pthread_cond_t*) sharedLoginLock;
- (MUTEX_VAR *) sharedLoginMutex : (BOOL) locked;

- (void) openLoginPrompt;
- (void) loginPromptClosed;

- (RakContentView*) getContentView;

- (Series *)	serie;
- (CTSelec *)	CT;
- (MDL *)		MDL;
- (Reader *)	reader;

//Shouldn't be accessed directly
- (Series *)	_serie;
- (CTSelec *)	_CT;
- (MDL *)		_MDL;
- (Reader *)	_reader;

- (void) flushState;

@end