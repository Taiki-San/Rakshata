/*********************************************************************************************
 **	__________         __           .__            __                 ________   _______   	**
 **	\______   \_____  |  | __  _____|  |__ _____ _/  |______   	___  _\_____  \  \   _  \  	**
 **	 |       _/\__  \ |  |/ / /  ___/  |  \\__  \\   __\__  \  	\  \/ //  ____/  /  /_\  \ 	**
 **	 |    |   \ / __ \|    <  \___ \|   Y  \/ __ \|  |  / __ \__ \   //       \  \  \_/   \	**
 **	 |____|_  /(____  /__|_ \/____  >___|  (____  /__| (____  /	  \_/ \_______ \ /\_____  /	**
 **	        \/      \/     \/     \/     \/     \/          \/ 	              \/ \/     \/ 	**
 **                                                                                         **
 **    Licence propriétaire, code source confidentiel, distribution formellement interdite  **
 **                                                                                         **
 *********************************************************************************************/

@interface RakPrefsMDLDeepData : RakPrefsDeepData
{
	CGFloat heightMDLSerie;
	CGFloat heightMDLCT;
	CGFloat heightMDLReader;
	CGFloat heightMDLFull;
	
	CGFloat widthMDLSerie;
	CGFloat widthMDLCT;
	CGFloat widthMDLReader;
	CGFloat widthMDLFull;

	CGFloat posXMDLSerie;
	CGFloat posXMDLCT;
	CGFloat posXMDLReader;
	CGFloat posXMDLFull;
	
	CGFloat posYMDL;
	CGFloat posYMDLFull;
}

- (CGFloat) getData: (int) mainThread : (uint8_t) request;
- (uint8_t) getIndexFromInput: (int) mainThread : (int) request;
- (void) setAtIndex: (uint8_t) index : (CGFloat) data;

@end
