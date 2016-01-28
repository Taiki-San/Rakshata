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
 *********************************************************************************************/
#include "lecteur.h"

/**	Set up the evnt	**/

bool reader_getNextReadableElement(PROJECT_DATA project, bool isTome, uint *currentPosIntoStructure)
{
	uint maxValue = isTome ? project.nbVolumesInstalled : project.nbChapterInstalled;
	
	//No item, nothing to look for, nothing to find
	if(maxValue == 0)
	{
		*currentPosIntoStructure = INVALID_VALUE;
		return false;
	}
	
	//Can go forward, and doesn't want to stall
	else if(*currentPosIntoStructure + 1 < maxValue)
		++(*currentPosIntoStructure);

	//Too far, probably deletions
	else if(*currentPosIntoStructure >= maxValue)
		*currentPosIntoStructure = maxValue - 1;
	
	while(*currentPosIntoStructure < maxValue && !checkReadable(project, isTome, ACCESS_ID(isTome, project.chaptersInstalled[*currentPosIntoStructure], project.volumesInstalled[*currentPosIntoStructure].ID)))
		(*currentPosIntoStructure)++;
	
	return *currentPosIntoStructure < maxValue;
}

/**	Load the reader data	**/

char ** loadChapterConfigDat(char* input, uint *nbPage, uint ** nameID)
{
	bool haveNameID = nameID != NULL;
	char ** output, current;
	FILE* fileInput = fopen(input, "r");
	
	if(fileInput == NULL)
		return NULL;
	
	fscanf(fileInput, "%d", nbPage);
	
	//We go to the next line
	while((current = fgetc(fileInput)) != EOF && current != '\n' && current != '\r');
	while((current = fgetc(fileInput)) != EOF && (current == '\n' || current == '\r'));
	
	//We extract the data from the file
	if(current != EOF)
	{
		if(current != 'N')
		{
			fclose(fileInput);
			logR("Obsolete "CONFIGFILE" detected, please notify the maintainer of the repository");
			
			return NULL;
		}
		
		output = calloc(*nbPage, sizeof(char*));
		if(haveNameID)
		{
			*nameID = calloc(*nbPage, sizeof(uint));
			haveNameID = *nameID != NULL;
		}
		
		for(uint i = 0, j; i < *nbPage; ++i)
		{
			output[i] = malloc(LONGUEUR_NOM_PAGE+1);
			if(output[i] == NULL)
			{
				memoryError(LONGUEUR_NOM_PAGE+1);
				while(i-- != 0)
					free(output[i]);
				free(output);
				*nbPage = 0;
				return NULL;
			}
			
			if(fscanf(fileInput, "%u %"STRINGIZE(LONGUEUR_NOM_PAGE)"s", &j, output[i]) != 2)
			{
				//Handle the case where we got less pages than expected
				
				free(output[i]);
				
				if(i == 0)
				{
					free(output);
					fclose(fileInput);
					return NULL;
				}
				
				*nbPage = i;
				
				void * tmp = realloc(output, i * sizeof(char*));
				if(tmp != NULL)
					output = tmp;
				
				break;
			}
			
			if(haveNameID)
				(*nameID)[i] = j;
			
			//We go to the next line
			while((current = fgetc(fileInput)) != EOF && current != '\n' && current != '\r');
			while((current = fgetc(fileInput)) != EOF && (current == '\n' || current == '\r'));
			if(current != EOF)
				fseek(fileInput, -1, SEEK_CUR);
		}
	}
	else
	{
		output = malloc(++(*nbPage) * sizeof(char*));
		
		for(uint i = 0; i < *nbPage; i++)
		{
			output[i] = malloc(20);
			if(output[i] == NULL)
			{
				memoryError(20);
				while(i-- != 0)
					free(output[i]);
				free(output);
				*nbPage = 0;
				return NULL;
			}
			else
				snprintf(output[i], 20, "%d.jpg", i);	//Sadly, legacy, use png as a default would have been more clever
		}
		
		if(haveNameID)
			*nameID = NULL;
	}
	
	fclose(fileInput);
	
	//We remove config.dat from the path
	for(uint i = strlen(input); i > 0 && input[i] != '/'; input[i--] = 0);
	
	//We then check that all those files exists
	bool needCompact = false;
	uint pathLength = strlen(input) + LONGUEUR_NOM_PAGE + 1;
	char temp[pathLength];
	
	for(uint i = 0; i < *nbPage; i++)
	{
		if(output[i] != NULL && output[i][0])
		{
			snprintf(temp, pathLength, "%s%s", input, output[i]);
			if(!checkFileExist(temp))
			{
				free(output[i]);
				output[i] = NULL;
				needCompact = true;
			}
		}
		else
			needCompact = true;
	}
	
	//We compact the list if invalid items were detected
	if(needCompact)
	{
		uint validEntries = 0;
		for(uint carry = 0; validEntries < *nbPage; validEntries++)
		{
			if(output[validEntries] == NULL)
			{
				if(carry <= validEntries)
					carry = validEntries + 1;
				
				for(; carry < *nbPage && output[carry] == NULL; carry++);
				
				if(carry < *nbPage)
				{
					output[validEntries] = output[carry];
					output[carry] = NULL;
					
					if(haveNameID)
						nameID[validEntries] = nameID[carry];
				}
			}
		}
		
		void * tmp = realloc(output, validEntries * sizeof(char *));
		if(tmp != NULL)
			output = tmp;
		
		if(haveNameID)
		{
			tmp = realloc(*nameID, validEntries * sizeof(uint));
			if(tmp != NULL)
				*nameID = tmp;
		}
		
		*nbPage = validEntries;
	}
	
	return output;
}

bool changeChapter(PROJECT_DATA* project, bool isTome, uint *ptrToSelectedID, uint *posIntoStruc, bool goToNextChap)
{
	if(goToNextChap)
		(*posIntoStruc)++;
	else
		(*posIntoStruc)--;
	
	if(!changeChapterAllowed(project, isTome, *posIntoStruc))
	{
		getUpdatedCTList(project, isTome);
		
		if(!changeChapterAllowed(project, isTome, *posIntoStruc))
			return false;
	}
	if(isTome)
		*ptrToSelectedID = project->volumesInstalled[*posIntoStruc].ID;
	else
		*ptrToSelectedID = (uint) project->chaptersInstalled[*posIntoStruc];
	return true;
}

bool changeChapterAllowed(PROJECT_DATA* project, bool isTome, uint posIntoStruc)
{
	return (isTome && posIntoStruc < project->nbVolumesInstalled) || (!isTome && posIntoStruc < project->nbChapterInstalled);
}

