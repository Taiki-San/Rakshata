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
	uint8_t heightMDLSerie;
	uint8_t heightMDLCT;
	uint8_t heightMDLReader;
	uint8_t heightMDLFull;
	
	uint8_t widthMDLSerie;
	uint8_t widthMDLCT;
	uint8_t widthMDLReader;
	uint8_t widthMDLFull;

	uint8_t posXMDLSerie;
	uint8_t posXMDLCT;
	uint8_t posXMDLReader;
	uint8_t posXMDLFull;
	
	uint8_t posYMDL;
	uint8_t posYMDLFull;
}

- (uint8_t) getData: (int) mainThread : (uint8_t) request;
- (uint8_t) getIndexFromInput: (int) mainThread : (int) request;

@end
