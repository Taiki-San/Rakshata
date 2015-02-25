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

#define CURRENTVERSIONSTRING STRINGIZE(CURRENTVERSION)

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

/*Standard des dépôts*/
#define CONFIGFILE "config.dat"
#define CONFIGFILETOME "config.tome.dat"
#define VERSION_REPO 3
#define VERSION_FIRST_REPO_JSON 3
#define VERSION_PROJECT 1

/*Réseau*/
#define CONNEXION_TEST_IN_PROGRESS -1
#define CONNEXION_OK 0
#define CONNEXION_SERVEUR_DOWN 1
#define CONNEXION_DOWN 2

/*Structure principale -> paliers*/
#define PALIER_MENU -3
#define PALIER_QUIT -4

/*Settings*/
#define SECURE_DATABASE "secure.enc"
#define DRM_FILE	".config.enc"
#define SETTINGS_FILE "settings"
#define SETTINGS_PASSWORD "Za8zR7e|3rz@[4zEbtr!s36~8713ue68"
#define SETTINGS_EMAIL_FLAG "E"
#define SETTINGS_FAVORITE_FLAG "F"
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
enum
{
	TYPE_DEPOT_PAID 	= 1,
	TYPE_DEPOT_DB 		= 2,
	TYPE_DEPOT_OTHER	= 3,
	TYPE_DEPOT_GOO		= 4
};
#define MAX_TYPE_DEPOT 4

/*String macro*/
#define STRINGIZE_HELPER(s) #s
#define STRINGIZE(s) STRINGIZE_HELPER(s)

/*Download.c*/
#define CODE_RETOUR_OK 0
#define CODE_RETOUR_DL_CLOSE 1
#define CODE_FAILED_AT_RESOLVE 2
#define CODE_RETOUR_INTERNAL_FAIL 3
#define CODE_RETOUR_PARTIAL 4

#define SSL_OFF 0
#define SSL_ON 1

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
