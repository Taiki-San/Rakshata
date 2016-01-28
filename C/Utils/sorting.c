/*********************************************************************************************
 **	__________         __           .__            __                 ________   _______   	**
 **	\______   \_____  |  | __  _____|  |__ _____ _/  |______   	___  _\_____  \  \   _  \  	**
 **	 |       _/\__  \ |  |/ / /  ___/  |  \\__  \\   __\__  \  	\  \/ //  ____/  /  /_\  \ 	**
 **	 |    |   \ / __ \|    <  \___ \|   Y  \/ __ \|  |  / __ \_  \   //       \  \  \_/   \	**
 **	 |____|_  /(____  /__|_ \/____  >___|  (____  /__| (____  /	  \_/ \_______ \ /\_____  /	**
 **	        \/      \/     \/     \/     \/     \/          \/ 	              \/ \/     \/ 	**
 **                                                                                          **
 **			This Source Code Form is subject to the terms of the Mozilla Public				**
 **			License, v. 2.0. If a copy of the MPL was not distributed with this				**
 **			file, You can obtain one at http://mozilla.org/MPL/2.0/.						**
 **                                                                                         **
 **                     			© Taiki 2011-2016                                       **
 **                                                                                          **
 *********************************************************************************************/

int sortNumbers(const void *a, const void *b)
{
	return ( *(uint*)a > *(uint*)b ) ? 1 : (( *(uint*)a == *(uint*)b ) ? 0 : -1);
}

int sortTomes(const void *a, const void *b)
{
	if(a == NULL)
		return 1;
	
	if(b == NULL)
		return -1;
	
	return sortNumbers(&(((META_TOME *) a)->ID), &(((META_TOME *) b)->ID));
}

int sortProjects(const void *a, const void *b)
{
	const PROJECT_DATA_PARSED *struc1 = a;
	const PROJECT_DATA_PARSED *struc2 = b;
	
	if(!struc1->project.isInitialized || struc1->project.projectName[0] == 0)
		return 1;
	
	else if(!struc2->project.isInitialized || struc2->project.projectName[0] == 0)
		return -1;
	
	if(isLocalRepo(struc1->project.repo) == isLocalRepo(struc2->project.repo)
	   && (isLocalRepo(struc1->project.repo) || !strcmp(struc1->project.repo->URL, struc2->project.repo->URL)))
		return sortNumbers(&(struc1->project.projectID), &(struc2->project.projectID));
	
	return wcscmp(struc1->project.projectName, struc2->project.projectName);
}

int sortRepo(const void *a, const void *b)
{
	return wstrcmp((*((REPO_DATA**) a))->name, (*((REPO_DATA**) b))->name);
}

int sortRootRepo(const void *a, const void *b)
{
	return wstrcmp((*((ROOT_REPO_DATA**) a))->name, (*((ROOT_REPO_DATA**) b))->name);
}

bool areProjectsIdentical(PROJECT_DATA_PARSED a, PROJECT_DATA_PARSED b)
{
	if(a.project.projectID != b.project.projectID)
		return false;
	
	for(byte count = 0; count < 2; count++)
	{
		uint lengthA, lengthB;
		uint * aChap, * bChap;
		
		if(count == 0)
		{
			lengthA = a.nbChapterLocal;
			aChap = a.chaptersLocal;
			lengthB = b.nbChapterLocal;
			bChap = b.chaptersLocal;
		}
		else
		{
			lengthA = a.nbChapterRemote;
			aChap = a.chaptersRemote;
			lengthB = b.nbChapterRemote;
			bChap = b.chaptersRemote;
		}
		
		if(lengthA != lengthB)
			return false;
		
		if(aChap == NULL ^ bChap == NULL)
			return false;
		
		if(aChap != NULL && memcmp(aChap, bChap, lengthA * sizeof(uint)))
			return false;
		
		META_TOME * aTome, * bTome;
		
		if(count == 0)
		{
			lengthA = a.nbVolumesLocal;
			aTome = a.tomeLocal;
			lengthB = b.nbVolumesLocal;
			bTome = b.tomeLocal;
		}
		else
		{
			lengthA = a.nbVolumesRemote;
			aTome = a.tomeRemote;
			lengthB = b.nbVolumesRemote;
			bTome = b.tomeRemote;
		}
		
		if(lengthA != lengthB)
			return false;
		
		if(aTome == NULL ^ bTome == NULL)
			return false;
		
		if(aTome != NULL && bTome != NULL)
		{
			for(uint i = 0; i < a.project.nbVolumes; i++)
			{
				if(aTome[i].price != bTome[i].price)
					return false;
				
				if(aTome[i].ID != bTome[i].ID || aTome[i].readingID != bTome[i].readingID || wcscmp(aTome[i].description, bTome[i].description) || wcscmp(aTome[i].readingName, bTome[i].readingName))
					return false;
				
				if(aTome[i].details == NULL ^ bTome[i].details == NULL)
					return false;
				
				if(aTome[i].details != NULL)
				{
					if(aTome[i].lengthDetails != bTome[i].lengthDetails)
						return false;
					
					for(uint pos = 0, max = aTome[i].lengthDetails; pos < max; pos++)
					{
						if(aTome[i].details[pos].ID != bTome[i].details[pos].ID || aTome[i].details[pos].isPrivate != bTome[i].details[pos].isPrivate)
							return false;
					}
				}
			}
		}
	}
	
	if(a.project.status != b.project.status)
		return false;
	
	if(a.project.nbTags != b.project.nbTags)
		return false;
	
	if((a.project.tags != NULL) ^ (b.project.tags != NULL))
		return false;
	
	if(a.project.tags != NULL && memcmp(a.project.tags, b.project.tags, a.project.nbTags * sizeof(TAG)))
		return false;
	
	if(a.project.rightToLeft != b.project.rightToLeft)
		return false;
	
	if(a.project.isPaid != b.project.isPaid)
		return false;
	
	if(a.project.chaptersPrix == NULL ^ b.project.chaptersPrix == NULL)
		return false;
	
	if(wcscmp(a.project.projectName, b.project.projectName) || wcscmp(a.project.authorName, b.project.authorName) || wcscmp(a.project.description, b.project.description))
		return false;
	
	return true;
}