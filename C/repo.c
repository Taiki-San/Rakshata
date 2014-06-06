/*********************************************************************************************
**	__________         __           .__            __                 ________   _______   	**
**	\______   \_____  |  | __  _____|  |__ _____ _/  |______   	___  _\_____  \  \   _  \  	**
**	 |       _/\__  \ |  |/ / /  ___/  |  \\__  \\   __\__  \  	\  \/ //  ____/  /  /_\  \ 	**
**	 |    |   \ / __ \|    <  \___ \|   Y  \/ __ \|  |  / __ \__ \   //       \  \  \_/   \	**
**	 |____|_  /(____  /__|_ \/____  >___|  (____  /__| (____  /	  \_/ \_______ \ /\_____  /	**
**	        \/      \/     \/     \/     \/     \/          \/ 	              \/ \/     \/ 	**
**                                                                                          **
**		Source code and assets are property of Taiki, distribution is stricly forbidden		**
**                                                                                          **
*********************************************************************************************/

#include "db.h"

bool addRepoByFileInProgress;

typedef struct AUTO_ADD_REPO {
    char type[LONGUEUR_TYPE_TEAM];
    char URL[LONGUEUR_URL];
} AUTO_ADD_REPO;

AUTO_ADD_REPO *addRepoData = NULL;

int checkAjoutRepoParFichier(char *argv)
{
    addRepoByFileInProgress = false;
    if(argv == NULL || *argv == '-')
        return 1;

    int version = 0;
    size_t size;
    char verification[50] = {0}, *bufferRead = NULL;
    FILE *input = fopen(argv, "r");
    if(input == NULL)
        return 0;

    size = getFileSize(argv);
    bufferRead = calloc(size+1, sizeof(char));
    if(bufferRead != NULL)
    {
        fscanfs(input, "%s %d\n", verification, 50, &version);
        size -= ftell(input);
        fread(bufferRead, size, sizeof(char), input);
    }
    fclose(input);

    if(bufferRead == NULL)
        return 0;

    else if(version > CURRENTVERSION)
    {
        if(langue == 1) //Francais
            UI_Alert("Ajout automatise de depot: echec!", "Le depot que vous tentez d'ajouter n'est pas supporte par cette version de Rakshata, veuillez effectuer une mise a jour en telechargant une version plus recente sur http://www.rakshata.com/");
        else
            UI_Alert("Automated addition of repository: failure!", "The repository you're trying to install isn't supported by this version of Rakshata: please perform an update by getting a newer build from our website: http://www.rakshata.com/");
        free(bufferRead);
        return 0;
    }
    else if(strcmp(verification, "Repository_for_Rakshata"))
    {
        if(langue == 1) //Francais
            UI_Alert("Ajout automatise de depot: echec!", "Fichier invalide: veuillez contacter l'administrateur du site depuis lequel vous l'avez telecharge");
        else
            UI_Alert("Automated addition of repository: failure!", "Invalid file: please contact the administrator of the website from which you downloaded the file.");
        free(bufferRead);
        return 0;
    }

    size_t position, nombreRetourLigne, ligneCourante;
    for(position = nombreRetourLigne = 1; position < size && bufferRead[position]; position++)
    {
        if(bufferRead[position] == '\n')
            nombreRetourLigne++;
    }

    addRepoData = calloc(nombreRetourLigne+1, sizeof(AUTO_ADD_REPO));
    if(addRepoData != NULL)
    {
        position = ligneCourante = 0;
        while(position < size && ligneCourante < nombreRetourLigne && bufferRead[position])
        {
            position += sscanfs(&bufferRead[position], "%s %s", addRepoData[ligneCourante].type, LONGUEUR_TYPE_TEAM, addRepoData[ligneCourante].URL, LONGUEUR_URL);
            for(; position < size && bufferRead[position++] != '\n';);
            for(; position < size && (bufferRead[position] == '\n' || bufferRead[position] == '\r'); position++);
            if(addRepoData[ligneCourante].URL[0])
            {
                int typeExpected = defineTypeRepo(addRepoData[ligneCourante].URL);
                if((typeExpected == 1 && !strcmp(addRepoData[ligneCourante].type, TYPE_DEPOT_1)) //Dropbox
                   || (typeExpected == 2 && !strcmp(addRepoData[ligneCourante].type, TYPE_DEPOT_2)) //Other
                   || (typeExpected == 3 && !strcmp(addRepoData[ligneCourante].type, TYPE_DEPOT_4))) //Goo.gl
                {
                    ligneCourante++;
                    addRepoByFileInProgress = true;
                }
                else
                {
                    crashTemp(addRepoData[ligneCourante].type, LONGUEUR_TYPE_TEAM);
                    crashTemp(addRepoData[ligneCourante].URL, LONGUEUR_URL);
                }
            }
            else
                crashTemp(addRepoData[ligneCourante].type, LONGUEUR_TYPE_TEAM);
        }
    }
    free(bufferRead);
    return addRepoByFileInProgress;
}

void addRepoByFile()
{
    if(addRepoByFileInProgress)
    {
        if(ajoutRepo(true) > 0)
            raffraichissmenent(true);
    }
	return;
}

int ajoutRepo(bool ajoutParFichier)
{
    int continuer = 0, somethingAdded = 0, ajoutFichierDecalageRefuse = 0;
    char temp[TAILLE_BUFFER], texteTrad[SIZE_TRAD_ID_14][TRAD_LENGTH];
    TEAMS_DATA teams;

	loadTrad(texteTrad, 14);

#ifdef IDENTIFY_MISSING_UI
	#warning "ajoutRepo"
#endif
	
    if(!ajoutParFichier)
    {

    }
    else
    {
        if(addRepoData == NULL)
            return 0;
    }
    if(!checkNetworkState(CONNEXION_DOWN))
    {
        /*Lecture du fichier*/
        while(!continuer && (!ajoutParFichier || addRepoData[somethingAdded + ajoutFichierDecalageRefuse].type[0]))
        {
            if(!ajoutParFichier)
            {
                /*On attend l'URL*/
                crashTemp(teams.URL_depot, LONGUEUR_URL);
                
                if(continuer == PALIER_MENU || continuer == PALIER_CHAPTER || strlen(teams.URL_depot) == 0)
                    continue;
                else if(continuer == PALIER_QUIT)
                    return PALIER_QUIT;

                /*Si que des chiffres, DB, sinon, O*/
                switch(defineTypeRepo(teams.URL_depot))
                {
                    case 1:
                        ustrcpy(teams.type, TYPE_DEPOT_1); //Dropbox
                        break;

                    case 2:
                        ustrcpy(teams.type, TYPE_DEPOT_2); //Other
                        break;

                    case 3: //Goo.gl
                        ustrcpy(teams.type, TYPE_DEPOT_4);
                        break;
                }
            }
            else
            {
                if(continuer < PALIER_MENU)
                    break;
                usstrcpy(teams.URL_depot, LONGUEUR_URL, addRepoData[somethingAdded + ajoutFichierDecalageRefuse].URL);
                usstrcpy(teams.type, LONGUEUR_TYPE_TEAM, addRepoData[somethingAdded + ajoutFichierDecalageRefuse].type);
            }

            if(!continuer)
            {
                int versionRepo = VERSION_REPO;
                char bufferDL[1000];

                if(strcmp(teams.type, TYPE_DEPOT_4))
                {
                    do
                    {
                        if(!strcmp(teams.type, TYPE_DEPOT_1))
                            snprintf(temp, TAILLE_BUFFER, "https://dl.dropboxusercontent.com/u/%s/rakshata-repo-%d", teams.URL_depot, versionRepo);
                        else if(!strcmp(teams.type, TYPE_DEPOT_2))
                            snprintf(temp, TAILLE_BUFFER, "http://%s/rakshata-repo-%d", teams.URL_depot, versionRepo);

                        download_mem(temp, NULL, bufferDL, 1000, !strcmp(teams.type, TYPE_DEPOT_1)?SSL_ON:SSL_OFF);
                        versionRepo--;
                    } while(!isDownloadValid(bufferDL) && versionRepo > 0);
                    
                    if(isDownloadValid(bufferDL))
                    {
                        if(versionRepo == 1)
                            sscanfs(bufferDL, "%s %s %s %s %s %d", teams.teamLong, LONGUEUR_NOM_MANGA_MAX, teams.teamCourt, LONGUEUR_COURT, teams.type, LONGUEUR_TYPE_TEAM, teams.URL_depot, LONGUEUR_URL, teams.site, LONGUEUR_SITE, &teams.openSite);
                        else
                        {
                            char ID[LONGUEUR_ID_TEAM];
                            sscanfs(bufferDL, "%s %s %s %s %s %s", ID, LONGUEUR_ID_TEAM, teams.teamLong, LONGUEUR_NOM_MANGA_MAX, teams.teamCourt, LONGUEUR_COURT, teams.type, LONGUEUR_TYPE_TEAM, teams.URL_depot, LONGUEUR_URL, teams.site, LONGUEUR_SITE);
                            teams.openSite = 1;
                        }
                    }
                }

                else
                {
                    snprintf(temp, TAILLE_BUFFER, "http://goo.gl/%s", teams.URL_depot);
                    download_mem(temp, NULL, bufferDL, 1000, !strcmp(teams.type, TYPE_DEPOT_1)?SSL_ON:SSL_OFF);
                    if(isDownloadValid(bufferDL))
                    {
                        int posBuf;
                        for(posBuf = strlen(bufferDL); bufferDL[posBuf] == '#' || bufferDL[posBuf] == '\n' || bufferDL[posBuf] == '\r'; bufferDL[posBuf--] = 0);
                        if(bufferDL[posBuf] >= '0' && bufferDL[posBuf] <= '9') //Ca fini par un chiffe, c'est la v2
                            sscanfs(bufferDL, "%s %s %s %s %s %d", teams.teamLong, LONGUEUR_NOM_MANGA_MAX, teams.teamCourt, LONGUEUR_COURT, teams.type, LONGUEUR_TYPE_TEAM, teams.URL_depot, LONGUEUR_URL, teams.site, LONGUEUR_SITE, &teams.openSite);
                        else
                        {
                            char ID[LONGUEUR_ID_TEAM];
                            sscanfs(bufferDL, "%s %s %s %s %s %s", ID, LONGUEUR_ID_TEAM, teams.teamLong, LONGUEUR_NOM_MANGA_MAX, teams.teamCourt, LONGUEUR_COURT, teams.type, LONGUEUR_TYPE_TEAM, teams.URL_depot, LONGUEUR_URL, teams.site, LONGUEUR_SITE);
                            teams.openSite = 1;
                        }
                    }
                }

                if(isDownloadValid(bufferDL)) //Si on pointe sur un vrai dépôt
                {
                    /*On affiche les infos*/
                    snprintf(temp, TAILLE_BUFFER, "Team: %s", teams.teamLong);
                    changeTo(temp, '_', ' ');
                    
					snprintf(temp, TAILLE_BUFFER, "Site: %s", teams.site);
					
					bool userAgreeToInjection = getRandom() % 2;
					
#ifdef IDENTIFY_MISSING_UI
	#warning "the user _should_ be prompted"
#endif
                    
					if(userAgreeToInjection)
                    {
                        addRepoToDB(teams);
                        somethingAdded++;
                        if(ajoutParFichier)
                            continuer = 0;
                    }
                    else if(ajoutParFichier && continuer >= PALIER_MENU)
                        continuer = 0;
                }

                else if(!ajoutParFichier)
                {
                    affichageRepoIconnue();
                    continuer = 0;
                }
                else
                    ajoutFichierDecalageRefuse++;
            }
        }
    }


    if(ajoutParFichier)
    {
        free(addRepoData);
    }

    if(continuer >= PALIER_MENU)
        continuer = somethingAdded;
    else if(ajoutParFichier && somethingAdded)
        return somethingAdded;
    return continuer;
}

int defineTypeRepo(char *URL)
{
    int i = 0;
    if(strlen(URL) == 8) //SI DB, seulement 8 chiffres
    {
        while(i < 8 && isNbr(URL[i++]));
        if(i == 8) //Si que des chiffres
            return 1; //DB
    }
    else if(strlen(URL) == 5 || strlen(URL) == 6) //GOO.GL
        return 3;
    return 2; //O
}

int confirmationRepo(char team[LONGUEUR_NOM_MANGA_MAX])
{
    int confirme = 0;
	char trad[SIZE_TRAD_ID_4][TRAD_LENGTH];
	loadTrad(trad, 4);
    
	//On demande ~

    if(confirme == 1 || confirme == PALIER_QUIT) //Confirmé
        return confirme;
    return 0;
}

