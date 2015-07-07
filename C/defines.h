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

#define CURRENTVERSIONSTRING STRINGIZE(CURRENTVERSION)

/*Environnement*/
#define STORE_URL	"https://store.rakshata.com"
#define SERVEUR_URL "https://rsp.rakshata.com"
#define BACKUP_INTERNET_CHECK "http://www.apple.com/library/test/success.html"

#define REPO_REC_NAME "repo"
#define PROJECT_REC_NAME "projects"
#define PROJECT_ROOT "project/"
#define CHAPTER_PREFIX "Chapitre_"
#define VOLUME_PREFIX "Tome_"
#define VOLUME_PRESHARED_DIR "native"
#define VOLUME_CHAP_SHARED_TOKEN "shared"
#define CHAPITER_INSTALLING_TOKEN "installing"
#define CUSTOM_COLOR_FILE "theme/customTheme"

#ifdef DEV_VERSION
    #define BUILD "dev"
#elif defined(_WIN32)
	#define BUILD "win32"
#else
	#define BUILD "OSX"
#endif

/*Standard des dépôts*/
#define CONFIGFILE "config.dat"
#define CONFIGFILETOME "config.tome.dat"
#define LOCAL_PATH_NAME "local"
#define VERSION_REPO 3
#define VERSION_FIRST_REPO_JSON 3
#define VERSION_PROJECT 1
#define VERSION_RAK_FILE 1

/*Réseau*/
enum
{
	CONNEXION_TEST_IN_PROGRESS = -1,
	CONNEXION_OK,
	CONNEXION_SERVEUR_DOWN,
	CONNEXION_DOWN,
#ifdef DEV_VERSION
	CONNEXION_IDENTIFIED_DOWN = CONNEXION_SERVEUR_DOWN
#else
	CONNEXION_IDENTIFIED_DOWN = CONNEXION_DOWN
#endif
};

/*Settings*/
#define SECURE_DATABASE "secure.enc"
#define DRM_FILE		".config.enc"
#define CONTEXT_FILE	"context.dat"
#define SETTINGS_FILE 	"settings"
#define SETTINGS_PASSWORD "Za8zR7e|3rz@[4zEbtr!s36~8713ue68"
#define SETTINGS_EMAIL_FLAG "E"
#define SETTINGS_FAVORITE_FLAG "F"
#define SETTINGS_PROJECTDB_FLAG "M"
#define SETTINGS_REPODB_FLAG "R"

/*Constants*/
#define INVALID_VALUE UINT_MAX
#define INVALID_SIGNED_VALUE INT_MIN

/*Tome*/
#define MAX_TOME_NAME_LENGTH 31
#define TOME_DESCRIPTION_LENGTH 256

/*Tags*/
#define TAG_PAR_CAT 32
#define TAG_NO_VALUE 0

/*Check environnement*/
#define NOMBRE_DE_FICHIER_A_CHECKER 16
#define LONGUEUR_NOMS_DATA 100

/*Ajout repo*/
enum
{
	TYPE_DEPOT_PAID 	= 1,
	TYPE_DEPOT_DB 		= 2,
	TYPE_DEPOT_OTHER	= 3,
	TYPE_DEPOT_GOO		= 4,
	MAX_TYPE_DEPOT = TYPE_DEPOT_GOO
};

/*String macro*/
#define STRINGIZE_HELPER(s) #s
#define STRINGIZE(s) STRINGIZE_HELPER(s)

/*Download.c*/
enum
{
	CODE_RETOUR_OK,
	CODE_RETOUR_DL_CLOSE,
	CODE_FAILED_AT_RESOLVE,
	CODE_RETOUR_INTERNAL_FAIL,
	CODE_RETOUR_PARTIAL,
	
	SSL_OFF = 0,
	SSL_ON
};

/*Sécurité*/
enum
{
	EVERYTHING_IN_HDD,
	INPUT_IN_MEMORY,
	OUTPUT_IN_MEMORY,
	EVERYTHING_IN_MEMORY
};
#define NOMBRE_CLE_MAX_ACCEPTE 10

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
