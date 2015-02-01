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
 *********************************************************************************************/

#include "db.h"

wchar_t * getTagForCode(uint32_t tagID)
{
	if(tagID < 10)			return L"Shonen";
	else if(tagID < 20)		return L"Shojo";
	else if(tagID < 30)		return L"Seinen";
	else if(tagID < 40)		return L"Comics";
	else if(tagID == 42)	return L"Pony";
	else if(tagID < 50)		return L"Manwa";
	else if(tagID < 60)		return L"Webcomic";
	else if(tagID < 69)		return L"Ecchi";
	
	return L"Hentai";
}