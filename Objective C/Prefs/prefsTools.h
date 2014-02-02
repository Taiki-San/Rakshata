/*********************************************************************************************
 **	__________         __           .__            __                 ________   _______   	**
 **	\______   \_____  |  | __  _____|  |__ _____ _/  |______   	___  _\_____  \  \   _  \  	**
 **	 |       _/\__  \ |  |/ / /  ___/  |  \\__  \\   __\__  \  	\  \/ //  ____/  /  /_\  \ 	**
 **	 |    |   \ / __ \|    <  \___ \|   Y  \/ __ \|  |  / __ \__ \   //       \  \  \_/   \	**
 **	 |____|_  /(____  /__|_ \/____  >___|  (____  /__| (____  /	  \_/ \_______ \ /\_____  /	**
 **	        \/      \/     \/     \/     \/     \/          \/ 	              \/ \/     \/ 	**
 **                                                                                          **
 **    Licence propriétaire, code source confidentiel, distribution formellement interdite   **
 **                                                                                          **
 *********************************************************************************************/

/***********	Get Width	**************/
int getWidthSerie(int mainThread, int stateTabsReader, bool toGetPosX);
int getWidthCT(int mainThread, int stateTabsReader, bool toGetPosX);
int getWidthReader(int mainThread, int stateTabsReader);

void getMDLPosX(int * output, int mainThread, int stateTabsReader);
void getMDLPosY(int * output, int mainThread, int stateTabsReader);
void getMDLHeight(int * output, int mainThread, int stateTabsReader);
void getMDLWidth(int * output, int mainThread, int stateTabsReader);
