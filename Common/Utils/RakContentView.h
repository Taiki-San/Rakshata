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

#define WIDTH_BORDER_ALL		4
#define WIDTH_BORDER_FAREST		2
#define WIDTH_BORDER_MIDDLE		1
#define WIDTH_BORDER_INTERNAL	1

@interface RakContentView : RakView
{
	uint _mainThread;
	uint _stateTabsReader;
	
	Series* _tabSerie;
	CTSelec* _tabCT;
	Reader* _tabReader;
	MDL* _tabMDL;
}

- (void) setupCtx : (Series*) tabSerie : (CTSelec*) tabCT : (Reader*) tabReader : (MDL*) tabMDL;
- (void) updateContext : (uint) mainThread : (uint) stateTabsReader;
- (void) cleanCtx;

@end

@interface RakContentViewBack : RakView
{
	RakColor * backgroundColor;
	
	RakContentView * firstResponder;

#if !TARGET_OS_IPHONE
	RakBorder* internalRows1, * internalRows2;
	RakText * titleView;
#endif
	
	BOOL didRegisterKVO;
}

//Used when leaving fullscreen mode
@property (nonatomic) CGFloat heightOffset;
@property (nonatomic) NSString * title;
@property (nonatomic) BOOL isMainWindow;

- (void) setupBorders;
- (RakContentView *) getFirstResponder;

- (NSRect) internalFrame;

@end

