/*********************************************************************************************
**  __________         __           .__            __                  ____     ________    **
**  \______   \_____  |  | __  _____|  |__ _____ _/  |______    ___  _/_   |    \_____  \   **
**   |       _/\__  \ |  |/ / /  ___/  |  \\__  \\   __\__  \   \  \/ /|   |     /  ____/   **
**   |    |   \ / __ \|    <  \___ \|   Y  \/ __ \|  |  / __ \_  \   / |   |    /       \   **
**   |____|_  /(____  /__|_ \/____  >___|  (____  /__| (____  /   \_/  |___| /\ \_______ \  **
**          \/      \/     \/     \/     \/     \/          \/               \/         \/  **
**                                                                                          **
**    Licence propriétaire, code source confidentiel, distribution formellement interdite   **
**                                                                                          **
*********************************************************************************************/

typedef struct infos_Team
{
    char teamLong[LONGUEUR_NOM_MANGA_MAX];
    char teamCourt[LONGUEUR_COURT];
    char type[LONGUEUR_TYPE_TEAM];
    char URL_depot[LONGUEUR_URL];
    char site[LONGUEUR_SITE];
    int openSite;
} TEAMS_DATA;

typedef struct tome_metadata
{
    int ID;
    unsigned char name[MAX_TOME_NAME_LENGTH];
    unsigned char description1[TOME_DESCRIPTION_LENGTH];
    unsigned char description2[TOME_DESCRIPTION_LENGTH];
}META_TOME;

typedef struct dataMangas
{
    //Infos générales
    char mangaName[LONGUEUR_NOM_MANGA_MAX];
    char mangaNameShort[LONGUEUR_COURT];
    TEAMS_DATA *team;

    int status;
    int genre;
    int pageInfos;

    //Chapitres
    int firstChapter;
    int lastChapter;
    int nombrechapitreSpeciaux;
    int nombreChapitre;
    int *chapitres;

    //Tomes
    int firstTome;
    int lastTome;
    int nombreTomes;
    META_TOME *tomes;

    int favoris;
} MANGAS_DATA;

typedef struct data_lecture_tome
{
    int nombrePageTotale;
    int pageCourante;
    char **nomPages;

    int *pathNumber; //Correspondance entre nomPage et path
    char **path;
    int chapitreOuID;
} DATA_LECTURE;

typedef struct download_data_struct
{
    char *URL;
    char *buf;
    size_t length;
    size_t current_pos;
} TMP_DL;

typedef struct data_loaded_from_download_list
{
	int chapitre;
    int partOfTome; //Si VALEUR_FIN_STRUCTURE, alors chapitre indé, sinon, tome dont c'est l'ID
    MANGAS_DATA* datas;
} DATA_LOADED;

typedef struct data_pour_installation
{
    int chapitre;
    size_t length;
    void *downloadedData;
    MANGAS_DATA *mangaDB;
} DATA_INSTALL;

typedef struct
{
    int r;
    int g;
    int b;
} PALETTE_RGB;

typedef struct
{
    PALETTE_RGB fond;               //Background
    PALETTE_RGB police;             //Police normale
    PALETTE_RGB police_new;         //Police nouveau manga (rouge)
    PALETTE_RGB police_unread;      //Police non-lu (vert)
    PALETTE_RGB police_actif;       //Police pour menus actifs (rouge)
    PALETTE_RGB police_indispo;     //Police pour menus indisponibles (gris)
} PALETTE_GLOBALE;

typedef unsigned char uint8_t;

#ifdef _WIN32
    #define MUTEX_VAR HANDLE
    #define MUTEX_LOCK for(; WaitForSingleObject(mutex, 50) == WAIT_TIMEOUT; SDL_Delay(50))
    #define MUTEX_UNLOCK ReleaseSemaphore (mutex, 1, NULL)
    #define MUTEX_LOCK_DECRYPT for(; WaitForSingleObject(mutex_decrypt, 50) == WAIT_TIMEOUT; SDL_Delay(50))
    #define MUTEX_UNLOCK_DECRYPT ReleaseSemaphore (mutex_decrypt, 1, NULL)
#else
    #define handle_error_en(en, msg) \
       do { errno = en; perror(msg); exit(EXIT_FAILURE); } while (0)

    #define handle_error(msg) \
       do { perror(msg); exit(EXIT_FAILURE); } while (0)

    struct thread_info {        /* Used as argument to thread_start() */
        pthread_t thread_id;        /* ID returned by pthread_create() */
        int       thread_num;       /* Application-defined thread # */
        char     *argv_string;      /* From command-line argument */
    };

    #define MUTEX_VAR pthread_mutex_t
    #define MUTEX_LOCK pthread_mutex_lock(&mutex)
    #define MUTEX_UNLOCK pthread_mutex_unlock(&mutex)
    #define MUTEX_LOCK_DECRYPT pthread_mutex_lock(&mutex_decrypt)
    #define MUTEX_UNLOCK_DECRYPT pthread_mutex_unlock(&mutex_decrypt)

#endif
