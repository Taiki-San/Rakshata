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
    char ID[LONGUEUR_ID_MAX];
    char teamLong[LONGUEUR_NOM_MANGA_MAX];
    char teamCourt[LONGUEUR_COURT];
    char mode[BUFFER_MAX];
    char URL[LONGUEUR_URL];
    char site[LONGUEUR_SITE];
} INFOS_TEAMS;

typedef struct output_download_with_size
{
    char *buf;
    size_t length;
} OUT_DL;

typedef struct data_pour_installation
{
    char teamLong[LONGUEUR_NOM_MANGA_MAX];
    char mangaLong[LONGUEUR_NOM_MANGA_MAX];
    int chapitre;
    OUT_DL *buf;

} DATA_INSTALL;

typedef unsigned char uint8_t;

typedef void (*I2prf) (
            const uint8_t   *key,
            uint32_t        keylen,
            const uint8_t   *txt,
            uint32_t        txtlen,
            uint8_t         *digest_ret);

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
