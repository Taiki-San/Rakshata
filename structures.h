/*********************************************
**	        	 Rakshata v1.1 		        **
**     Licence propriétaire, code source    **
**        confidentiel, distribution        **
**          formellement interdite          **
**********************************************/

typedef struct argmt
{
    char URL[400];
    char* repertoireEcriture;
} ARGUMENT;

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
    int status;
    int genre;
    int pageInfos;

    /*Team*/
    TEAMS_DATA *team;

} MANGAS_DATA;

typedef struct output_download_with_size
{
    char *buf;
    size_t length;
} OUT_DL;

typedef struct data_pour_installation
{
    MANGAS_DATA mangaDB;
    int chapitre;
    OUT_DL *buf;

} DATA_INSTALL;

typedef unsigned char uint8_t;

#ifndef _WIN32

    /**UNIX only**/
    #define handle_error_en(en, msg) \
       do { errno = en; perror(msg); exit(EXIT_FAILURE); } while (0)

    #define handle_error(msg) \
       do { perror(msg); exit(EXIT_FAILURE); } while (0)

    struct thread_info {        /* Used as argument to thread_start() */
    pthread_t thread_id;        /* ID returned by pthread_create() */
    int       thread_num;       /* Application-defined thread # */
    char     *argv_string;      /* From command-line argument */
    };
#endif
