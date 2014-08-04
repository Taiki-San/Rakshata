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
#include "crypto.h"

void checkAjoutRepoParFichier(char *argv)
{
    if(argv == NULL || *argv == '-')
        return;

    int version = 0;
    size_t size;
    char verification[50] = {0}, *bufferRead = NULL;
    FILE *input = fopen(argv, "r");
    if(input == NULL)
        return;

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
        return;

    else if(version > CURRENTVERSION)
    {
        if(langue == 1) //Francais
            UI_Alert("Ajout automatise de depot: echec!", "Le depot que vous tentez d'ajouter n'est pas supporte par cette version de Rakshata, veuillez effectuer une mise a jour en telechargant une version plus recente sur http://www.rakshata.com/");
        else
            UI_Alert("Automated addition of repository: failure!", "The repository you're trying to install isn't supported by this version of Rakshata: please perform an update by getting a newer build from our website: http://www.rakshata.com/");
        free(bufferRead);
        return;
    }
    else if(strcmp(verification, "Repository_for_Rakshata"))
    {
        if(langue == 1) //Francais
            UI_Alert("Ajout automatise de depot: echec!", "Fichier invalide: veuillez contacter l'administrateur du site depuis lequel vous l'avez telecharge");
        else
            UI_Alert("Automated addition of repository: failure!", "Invalid file: please contact the administrator of the website from which you downloaded the file.");
        free(bufferRead);
        return;
    }

    size_t position, nombreRetourLigne, ligneCourante;
    for(position = nombreRetourLigne = 1; position < size && bufferRead[position]; position++)
    {
        if(bufferRead[position] == '\n')
            nombreRetourLigne++;
    }

	bool anySuccess = false;
	char type[LONGUEUR_TYPE_TEAM], URL[LONGUEUR_URL];
	position = ligneCourante = 0;
	while(position < size && ligneCourante < nombreRetourLigne && bufferRead[position])
	{
		position += sscanfs(&bufferRead[position], "%s %s", type, LONGUEUR_TYPE_TEAM, URL, LONGUEUR_URL);
		for(; position < size && bufferRead[position++] != '\n';);
		for(; position < size && (bufferRead[position] == '\n' || bufferRead[position] == '\r'); position++);
		if(URL[0])
			anySuccess |= addRepo(URL, type);
		
		crashTemp(type, LONGUEUR_TYPE_TEAM);
		crashTemp(URL, LONGUEUR_URL);
	}
	
	if(anySuccess)
		updateDatabase(true);
}

bool getRepoData(uint type, char * repoURL, char * output, uint sizeOutput)
{
	if(type == 0 || type > MAX_TYPE_DEPOT || repoURL == NULL || output == NULL || !sizeOutput)
		return false;
	
	output[0] = 0;
	if(type == 1 || type == 2)
	{
		short versionRepo = VERSION_REPO;
		char fullURL[512], *baseURL = type == 1 ? "https://dl.dropboxusercontent.com/u/%s/rakshata-repo-%d" : "http://%s/rakshata-repo-%d";
		do
		{
			snprintf(fullURL, sizeof(fullURL), baseURL, repoURL, versionRepo);
			download_mem(fullURL, NULL, output, sizeOutput, type == 1 ? SSL_ON : SSL_OFF);
			versionRepo--;

		} while(versionRepo > 0 && !isDownloadValid(output));
	}
	else
	{
		char fullURL[64];
		snprintf(fullURL, sizeof(fullURL), "http://goo.gl/%s", repoURL);
		download_mem(fullURL, NULL, output, sizeOutput, SSL_OFF);
	}
	
	return isDownloadValid(output);
}

TEAMS_DATA parseRemoteRepoData(char * remoteData, uint length)
{
	TEAMS_DATA outputData;
	memset(&outputData, 0, sizeof(TEAMS_DATA));
	
	if(remoteData != NULL && length > 0)
	{
		length--;
		
		if(remoteData[length] >= '0' && remoteData[length] <= '9') //Ca fini par un chiffe, c'est la v2
		{
			sscanfs(remoteData, "%s %s %s %s %s %d", outputData.teamLong, LENGTH_PROJECT_NAME, outputData.teamCourt, LONGUEUR_COURT, outputData.type, LONGUEUR_TYPE_TEAM, outputData.URLRepo, LONGUEUR_URL, outputData.site, LONGUEUR_SITE, &outputData.openSite);
		}
		else
		{
			char ID[10];
			sscanfs(remoteData, "%s %s %s %s %s %s", ID, sizeof(ID), outputData.teamLong, LENGTH_PROJECT_NAME, outputData.teamCourt, LONGUEUR_COURT, outputData.type, LONGUEUR_TYPE_TEAM, outputData.URLRepo, LONGUEUR_URL, outputData.site, LONGUEUR_SITE);
			outputData.openSite = 1;
		}
	}
	
	return outputData;
}

#warning "To test"
bool addRepo(char * URL, char *type)
{
    char bufferDL[1000];
    TEAMS_DATA tmpData;

	if(!checkNetworkState(CONNEXION_DOWN))
    {
        /*Lecture du fichier*/
		if(type == NULL)
		{
			getRepoData(defineTypeRepo(URL), URL, bufferDL, sizeof(bufferDL));
		}
		else
		{
			usstrcpy(tmpData.URLRepo, LONGUEUR_URL, URL);
			usstrcpy(tmpData.type, LONGUEUR_TYPE_TEAM, type);
			getUpdatedRepo(bufferDL, sizeof(bufferDL), tmpData);
		}
		
		if(isDownloadValid(bufferDL))
		{
			tmpData = parseRemoteRepoData(bufferDL, sizeof(bufferDL));
			addRepoToDB(tmpData);
			return true;
		}
    }

	return false;
}

char * getPathForTeam(char *URLRepo)
{
	if(URLRepo == NULL)
		return NULL;
	
	size_t length;
	unsigned char URLRepoHash[SHA256_DIGEST_LENGTH];
	sha256((unsigned char*)URLRepo, URLRepoHash);
	return base64_encode(URLRepoHash, SHA256_DIGEST_LENGTH, &length);
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

int confirmationRepo(char team[LENGTH_PROJECT_NAME])
{
    int confirme = 0;
	
	//On demande ~

    if(confirme == 1 || confirme == PALIER_QUIT) //Confirmé
        return confirme;
    return 0;
}

