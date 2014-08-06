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

/*DEFINES*/

/*Général*/
#define TAILLE_BUFFER 500

/*Thread*/
#ifdef _WIN32
    #define quit_thread(a) { \
                            MUTEX_LOCK(mutex);\
                            THREAD_COUNT--;\
                            MUTEX_UNLOCK(mutex);\
                            ExitThread(a);\
                            }

#else
    #define quit_thread(a) { \
                            MUTEX_LOCK(mutex);\
                            THREAD_COUNT--;\
                            MUTEX_UNLOCK(mutex);\
                            pthread_exit(a);\
                            }
#endif

/*Environnement*/
#define STORE_URL	"store.rakshata.com"
#define SERVEUR_URL "rsp.rakshata.com"
#define BACKUP_INTERNET_CHECK "http://www.apple.com/library/test/success.html"

#define REPO_REC_NAME "repo"
#define PROJECT_REC_NAME "projects"
#define PROJECT_ROOT "project/"

#ifdef DEV_VERSION
    #define BUILD "dev"
#else
    #ifdef _WIN32
        #define BUILD "win32"
    #else
        #ifdef __APPLE__
            #define BUILD "OSX"
        #else
            #define BUILD "linux"
        #endif
    #endif
#endif

/*Contextes*/
#define CONTEXTE_LECTURE 1
#define CONTEXTE_DL 2
#define CONTEXTE_SUPPRESSION 3
#define CONTEXTE_CHAPITRE 4
#define CONTEXTE_TOME 5

/*Standard des dépôts*/
#define CONFIGFILE "config.dat"
#define CONFIGFILETOME "config.tome.dat"
#define VERSION_REPO 2
#define VERSION_PROJECT 1

/**		Flags	   **/

#define GUI_THREAD_READER			0x0001
#define GUI_THREAD_CT				0x0002
#define GUI_THREAD_MDL				0x0004
#define GUI_THREAD_SERIES			0x0008
#define GUI_DEFAULT_THREAD			0x1000
#define GUI_THREAD_MASK				0xFFFF

/*Réseau*/
#define CONNEXION_TEST_IN_PROGRESS -1
#define CONNEXION_OK 0
#define CONNEXION_SERVEUR_DOWN 1
#define CONNEXION_DOWN 2

/*Genre*/
#define GENRE_SHONEN 1
#define GENRE_SHOJO 2
#define GENRE_SEINEN 3
#define GENRE_HENTAI 4

/*Structure principale -> paliers*/
#define PALIER_MENU -3
#define PALIER_QUIT -4

/*Langue*/
#define NOMBRE_LANGUE 4
#define LANGUE_PAR_DEFAUT 2

/*Settings*/
#define SECURE_DATABASE "secure.enc"
#define SETTINGS_FILE "settings"
#define SETTINGS_PASSWORD "Za8zR7e|3rz@[4zEbtr!s36~8713ue68"
#define SETTINGS_EMAIL_FLAG "E"
#define SETTINGS_FAVORITE_FLAG "F"
#define SETTINGS_LANGUE_FLAG "L"
#define SETTINGS_PROJECTDB_FLAG "M"
#define SETTINGS_REPODB_FLAG "R"
#define SETTINGS_PASSWORD_FLAG "P" //Pass de déverrouillage

/*Chapitre*/
#define VALEUR_FIN_STRUCT INT_MIN

/*Tome*/
#define MAX_TOME_NAME_LENGTH 31
#define TOME_DESCRIPTION_LENGTH 256

/*Check environnement*/
#define NOMBRE_DE_FICHIER_A_CHECKER 16
#define LONGUEUR_NOMS_DATA 100

/*Ajout repo*/
#define TYPE_DEPOT_1 "DB" //Dépôt dropbox
#define TYPE_DEPOT_2 "O" //Dépôt classique
#define TYPE_DEPOT_3 "P" //Dépôts payants
#define TYPE_DEPOT_4 "GOO" //Raccourcis pour acceder au dépôt
#define TYPE_DEPOT_5 "MG" //Dépôts MEGA
#define MAX_TYPE_DEPOT 4

/*String macro*/
#define STRINGIZE(s) "`"#s"`"

/*Download.c*/
#define CODE_RETOUR_OK 0
#define CODE_RETOUR_DL_CLOSE 1
#define CODE_FAILED_AT_RESOLVE 2
#define CODE_RETOUR_INTERNAL_FAIL 3
#define CODE_RETOUR_PARTIAL 4

#define SSL_OFF 0
#define SSL_ON 1

/*Database*/
#define LENGTH_DESCRIPTION 1024
#define LENGTH_PROJECT_NAME 51
#define LENGTH_AUTHORS		51
#define LENGTH_URL			256
#define	LENGTH_HASH			9
#define STATUS_MAX			3
#define TYPE_MAX			3

#define LONGUEUR_COURT 10
#define LONGUEUR_SITE 100
#define LONGUEUR_TYPE_TEAM 5
#define LONGUEUR_URL 300
#define SIZE_BUFFER_UPDATE_DATABASE (20*1024*1024)
#define NUMBER_MAX_TEAM_KILLSWITCHE 200

/*Sécurité*/
#define EVERYTHING_IN_HDD 1
#define INPUT_IN_MEMORY 2
#define OUTPUT_IN_MEMORY 3
#define EVERYTHING_IN_MEMORY 4
#define NOMBRE_CLE_MAX_ACCEPTE 10
#define FULL_ENCRYPTION 1

/*Hash algorithms*/
#define SHA256_BLOCK_LENGTH	64
#define SHA256_DIGEST_LENGTH 32
#define WP_DIGEST_SIZE	64
#define PBKDF2_OUTPUT_LENGTH 32

enum
{
	GMK_RETVAL_OK,
	GMK_RETVAL_NEED_CREDENTIALS_MAIL,
	GMK_RETVAL_NEED_CREDENTIALS_PASS,
	GMK_RETVAL_INTERNALERROR
};

#define IMGLOAD_NODATA NULL
#define IMGLOAD_INCORRECT_DECRYPTION ((void*) 0x1)
#define IMGLOAD_NEED_CREDENTIALS_MAIL ((void*) 0x2)
#define IMGLOAD_NEED_CREDENTIALS_PASS ((void*) 0x3)
