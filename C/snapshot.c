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

//****		Les routines pour sauvegarder et restaurer l'état de Rakshata	****//

bool checkRestore()
{
    if(checkRestoreAvailable())
        return true;
    remove("data/laststate.dat");
    return false;
}

//		Vieeeelllllllleeeeeee rouine u_u
void teamOfProject(char nomProjet[LONGUEUR_NOM_MANGA_MAX], char nomTeam[LONGUEUR_NOM_MANGA_MAX])
{
	char temp[LONGUEUR_NOM_MANGA_MAX] = {0}, buffer[LONGUEUR_COURT] = {0};
    char* manga = loadLargePrefs(SETTINGS_MANGADB_FLAG), *mangaBak;
    mangaBak = manga;
	
    if(manga != NULL)
    {
        manga += sscanfs(manga, "%s", nomTeam, LONGUEUR_NOM_MANGA_MAX);
        for(; *manga && *manga != '\n'; manga++);
        for(; *manga == '\n'; manga++);
        while(strcmp(temp, nomProjet) && strcmp(buffer, nomProjet) && *manga)
        {
            if(*manga == '#')
            {
                manga++;
                for(;*manga == '\n'; manga++);
                manga += sscanfs(manga, "%s", nomTeam, LONGUEUR_NOM_MANGA_MAX);
                for(; *manga && *manga != '\n'; manga++);
                for(; *manga == '\n'; manga++);
            }
            else if(*manga)
            {
                manga += sscanfs(manga, "%s %s\n", temp, LONGUEUR_NOM_MANGA_MAX, buffer, LONGUEUR_COURT);
                for(; *manga && *manga != '\n'; manga++);
                for(; *manga == '\n'; manga++);
            }
            else
                break;
        }
        free(mangaBak);
    }
}

int checkRestoreAvailable()
{
    FILE* restore = fopen("data/laststate.dat", "r");
    if(restore != NULL)
    {
        int chapitre = 0;
        char manga[LONGUEUR_NOM_MANGA_MAX], temp[LONGUEUR_NOM_MANGA_MAX*5+50], team[LONGUEUR_NOM_MANGA_MAX], type[2] = {0, 0};
        fscanfs(restore, "%s %s %d", manga, LONGUEUR_NOM_MANGA_MAX, type, 2, &chapitre);
        fclose(restore);
		
        teamOfProject(manga, team);
        if(type[0] == 'C')
        {
            if(chapitre%10)
                snprintf(temp, LONGUEUR_NOM_MANGA_MAX*5+50, "manga/%s/%s/Chapitre_%d.%d/%s", team, manga, chapitre/10, chapitre%10, CONFIGFILE);
            else
                snprintf(temp, LONGUEUR_NOM_MANGA_MAX*5+50, "manga/%s/%s/Chapitre_%d/%s", team, manga, chapitre/10, CONFIGFILE);
        }
        else
            snprintf(temp, LONGUEUR_NOM_MANGA_MAX*5+50, "manga/%s/%s/Tome_%d/%s", team, manga, chapitre, CONFIGFILETOME);
		
        return checkFileExist(temp);
    }
    return 0;
}

//Reader

void reader_saveStateForRestore(char * mangaName, int currentSelection, bool isTome, int currentPage)
{
	FILE * file = fopen("data/laststate.dat", "w+");
	fprintf(file, "%s %c %d %d", mangaName, isTome?'T':'C', currentSelection, currentPage);
	fclose(file);
}

void reader_loadStateForRestore(char * mangaName, int * currentSelection, bool * isTome, int * page, bool removeWhenDone)
{
	char mangaNameInternal[LONGUEUR_NOM_MANGA_MAX], isTomeChar[2];
	int currentSelectionInternal, pageInternal;
	
	if(mangaName == NULL)
		mangaName = (char*) &mangaNameInternal;
	
	if(currentSelection == NULL)
		currentSelection = &currentSelectionInternal;
	
	if(page == NULL)
		page = &pageInternal;
	
	FILE * file = fopen("data/laststate.dat", "r");
	fscanfs(file, "%s %s %d %d", mangaName, LONGUEUR_NOM_MANGA_MAX, isTomeChar, 2, currentSelection, page);
	fclose(file);
	
	if(isTome != NULL)
		*isTome = (isTomeChar[0] == 'T');
	
	if(removeWhenDone)
		remove("data/laststate.dat");
}

