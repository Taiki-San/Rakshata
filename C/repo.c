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
	char URL[LONGUEUR_URL];
	position = ligneCourante = 0;
	while(position < size && ligneCourante < nombreRetourLigne && bufferRead[position])
	{
		uint typeBuf = MAX_TYPE_DEPOT + 1;
		position += sscanfs(&bufferRead[position], "%d %s", &typeBuf, URL, LONGUEUR_URL);
		
		for(; position < size && bufferRead[position++] != '\n';);
		for(; position < size && (bufferRead[position] == '\n' || bufferRead[position] == '\r'); position++);
		if(typeBuf <= MAX_TYPE_DEPOT && URL[0])
			anySuccess |= addRepo(URL, typeBuf & 0xff);
		
		crashTemp(URL, LONGUEUR_URL);
	}
	
	if(anySuccess)
		updateDatabase(true);
}

bool getRepoData(byte type, char * repoURL, char * output, uint sizeOutput)
{
	if(type == 0 || type > MAX_TYPE_DEPOT || repoURL == NULL || output == NULL || !sizeOutput)
		return false;
	
	output[0] = 0;
	if(type == TYPE_DEPOT_DB || type == TYPE_DEPOT_OTHER)
	{
		short versionRepo = VERSION_REPO;
		char fullURL[512];
		
		snprintf(fullURL, sizeof(fullURL), type == TYPE_DEPOT_DB ? "https://dl.dropboxusercontent.com/u/%s/rakshata-repo-%d" : "http://%s/rakshata-repo-%d", repoURL, versionRepo);
		download_mem(fullURL, NULL, output, sizeOutput, type == 1 ? SSL_ON : SSL_OFF);
	}
	else
	{
		char fullURL[64];
		snprintf(fullURL, sizeof(fullURL), "http://goo.gl/%s", repoURL);
		download_mem(fullURL, NULL, output, sizeOutput, SSL_OFF);
	}
	
	return isDownloadValid(output);
}

#warning "To upgrade"
bool addRepo(char * URL, byte type)
{
    char bufferDL[1000];

	if(!checkNetworkState(CONNEXION_DOWN))
    {
		if(type > MAX_TYPE_DEPOT)
		{
			getRepoData(defineTypeRepo(URL), URL, bufferDL, sizeof(bufferDL));
		}
		else
		{
			ROOT_REPO_DATA tmpData;
			
			usstrcpy(tmpData.URL, LONGUEUR_URL, URL);
			tmpData.type = type;
			getUpdatedRepo(bufferDL, sizeof(bufferDL), tmpData);
		}
		
		if(isDownloadValid(bufferDL))
		{
			ROOT_REPO_DATA * root = parseRemoteRepo(bufferDL);
			root->repoID = getFreeRootRepoID();
			enforceRepoExtra(root, true);
			addRepoToDB(root);
			return true;
		}
    }

	return false;
}

void * enforceRepoExtra(ROOT_REPO_DATA * root, bool getRidOfThemAfterward)
{
	if(!root->subRepoAreExtra)
		return NULL;
	
	ICONS_UPDATE * begin = NULL, * current, * new;
	REPO_DATA_EXTRA * data = (void *) root->subRepo;
	uint nbSubRepo = root->nombreSubrepo;
	char rootPath[64], imagePath[256];
	
	for(uint i = 0; i < nbSubRepo; i++)
	{
		//We create the file path
		char * encodedHash = getPathForRepo(data[i].data);
		if(encodedHash == NULL)
			continue;
		snprintf(rootPath, sizeof(rootPath), "imageCache/%s", encodedHash);
		createPath(rootPath);
		free(encodedHash);

		//Check if there is any data
		if(!data[i].URLImage[0])
			continue;
		
		new = calloc(1, sizeof(ICONS_UPDATE));
		if(new == NULL)
		{
			memoryError(sizeof(ICONS_UPDATE));
			continue;
		}

		//Copy the data
		snprintf(imagePath, sizeof(imagePath), "%s/"REPO_IMG_NAME".png", rootPath);
		new->URL = strdup(data[i].URLImage);
		new->filename = strdup(imagePath);
		
		if(new->URL == NULL || new->filename == NULL)
		{
			free(new->URL);
			free(new->filename);
			free(new);
			continue;
		}
		
		//We check for retina
		if(data[i].haveRetina)
		{
			ICONS_UPDATE * retina = calloc(1, sizeof(ICONS_UPDATE));
			if(retina == NULL)
			{
				memoryError(sizeof(ICONS_UPDATE));
				continue;
			}
			
			//Copy the data
			snprintf(imagePath, sizeof(imagePath), "%s/"REPO_IMG_NAME"@2x.png", rootPath);
			retina->URL = strdup(data[i].URLImage);
			retina->filename = strdup(imagePath);
			
			if(retina->URL == NULL || new->filename == NULL)
			{
				free(retina->URL);
				free(retina->filename);
				free(retina);
			}
			else
			{
				new->next = retina;
			}
		}
		
		if(begin == NULL)
		{
			begin = new;
			if(new->next == NULL)
				current = new;
			else
				current = new->next;
		}
		else
		{
			current->next = new;
			if(new->next == NULL)
				current = new;
			else
				current = new->next;
		}
	}
	
	if(getRidOfThemAfterward)
	{
		REPO_DATA * final = calloc(nbSubRepo, sizeof(REPO_DATA));
		
		for(uint i = 0; i < nbSubRepo; i++)
		{
			final[i] = *(((REPO_DATA_EXTRA *)root->subRepo)[i].data);
		}
		
		free(root->subRepo);
		root->subRepo = final;
		root->subRepoAreExtra = false;
	}
	
	return begin;
}

char * getPathForRepo(REPO_DATA * repo)
{
	if(repo == NULL)
		return NULL;
	
	char * output = calloc(20, sizeof(char));
	if(output != NULL)
	{
		snprintf(output, 20, "%x/%x", repo->parentRepoID, repo->repoID);
	}

	return output;
}

byte defineTypeRepo(char *URL)
{
    int i = 0;
    if(strlen(URL) == 8) //SI DB, seulement 8 chiffres
    {
        while(i < 8 && isNbr(URL[i++]));
        if(i == 8) //Si que des chiffres
            return TYPE_DEPOT_DB;
    }
    else if(strlen(URL) == 5 || strlen(URL) == 6) //GOO.GL
        return TYPE_DEPOT_GOO;

	return TYPE_DEPOT_OTHER;
}

int confirmationRepo(char team[LENGTH_PROJECT_NAME])
{
    int confirme = 0;
	
	//On demande ~

    if(confirme == 1 || confirme == PALIER_QUIT) //Confirmé
        return confirme;
    return 0;
}

