/******************************************************************************************************
**      __________         __           .__            __                ____     ____     ____      **
**      \______   \_____  |  | __  _____|  |__ _____ _/  |______    /\  /_   |   /_   |   /_   |     **
**       |       _/\__  \ |  |/ / /  ___/  |  \\__  \\   __\__  \   \/   |   |    |   |    |   |     **
**       |    |   \ / __ \|    <  \___ \|   Y  \/ __ \|  |  / __ \_ /\   |   |    |   |    |   |     **
**       |____|_  /(____  /__|_ \/____  >___|  (____  /__| (____  / \/   |___| /\ |___| /\ |___|     **
**              \/      \/     \/     \/     \/     \/          \/             \/       \/           **
**                                                                                                   **
**         Licence propriétaire, code source confidentiel, distribution formellement interdite       **
**                                                                                                   **
******************************************************************************************************/

typedef struct infos_Team
{
    char IDTeam[LONGUEUR_ID_TEAM];
    char teamLong[LONGUEUR_NOM_MANGA_MAX];
    char teamCourt[LONGUEUR_COURT];
    char type[LONGUEUR_TYPE_TEAM];
    char URL_depot[LONGUEUR_URL];
    char site[LONGUEUR_SITE];
} TEAMS_DATA;

typedef struct dataMangas
{
    char mangaName[LONGUEUR_NOM_MANGA_MAX];
    char mangaNameShort[LONGUEUR_COURT];
    int firstChapter;
    int lastChapter;
    int *chapitres;
    int firstTome;
    int lastTome;
    int status;
    int genre;
    int pageInfos;
    int favoris;
    int chapitreSpeciauxDisponibles;

    /*Team*/
    TEAMS_DATA *team;

} MANGAS_DATA;

typedef struct output_download_with_size
{
    char *buf;
    size_t length;
} OUT_DL;

typedef struct download_data_struct
{
    char *buf;
    size_t length;
    size_t current_pos;
} TMP_DL;

typedef struct data_loaded_from_download_list
{
    int chapitre;
    MANGAS_DATA* datas;
} DATA_LOADED;

typedef struct data_pour_installation
{
    MANGAS_DATA mangaDB;
    int chapitre;
    OUT_DL *buf;

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

#endif
