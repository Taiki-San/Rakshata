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

#define REPO_DATABASE "data/repo"
#define MANGA_DATABASE "data/mangas"
#define INSTALL_DATABASE "data/import.dat"

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
#define SECURE_DATABASE "data/secure.enc"
#define SETTINGS_FILE "data/settings"
#define SETTINGS_PASSWORD "Za8zR7e|3rz@[4zEbtr!s36~8713ue68"
#define SETTINGS_EMAIL_FLAG "E"
#define SETTINGS_FAVORITE_FLAG "F"
#define SETTINGS_LANGUE_FLAG "L"
#define SETTINGS_MESSAGE_SECTION_FLAG "S"
#define SETTINGS_MANGADB_FLAG "M"
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
#define LONGUEUR_ID_TEAM 10																	////UNUSED
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

/*Trad*/
#define TRAD_LENGTH 200
#define NOMBRE_TRAD_ID_MAX 31
#define SIZE_TRAD_ID_1 6    //Erreur page illisible
#define SIZE_TRAD_ID_2 5    //Initialisation du module de sélection de téléchargement
#define SIZE_TRAD_ID_3 7    //Préférences
#define SIZE_TRAD_ID_4 2    //Confirmation suppression de dépôt (fonction indé)
#define SIZE_TRAD_ID_5 1    //Raffraichissement en cours
#define SIZE_TRAD_ID_6 2    //Lancement du module de DL
#define SIZE_TRAD_ID_7 2    //Dépôt inconnu
#define SIZE_TRAD_ID_8 1    //Chargement
#define SIZE_TRAD_ID_9 5    //Welcome
#define SIZE_TRAD_ID_10 2   //Header infos.png
#define SIZE_TRAD_ID_11 16  //Engine
#define SIZE_TRAD_ID_12 4   //Module de mise à jour
#define SIZE_TRAD_ID_13 6   //Changement langue
#define SIZE_TRAD_ID_14 4   //Ajout de dépôt
#define SIZE_TRAD_ID_15 1   //Suppression de dépôt
#define SIZE_TRAD_ID_16 35  //Pop-up dans le module de DL
#define SIZE_TRAD_ID_17 5   //Menu principal
#define SIZE_TRAD_ID_18 2   //Selection de manga
#define SIZE_TRAD_ID_19 17  //Selection chapitre/tome
#define SIZE_TRAD_ID_20 7   //Partie DL du module de DL
#define SIZE_TRAD_ID_21 10  //Lecteur
#define SIZE_TRAD_ID_22 18  //Module de DL
#define SIZE_TRAD_ID_23 5   //Aucun manga disponible
#define SIZE_TRAD_ID_24 2   //Erreur connexion internet
#define SIZE_TRAD_ID_25 3   //Nom fenêtre
#define SIZE_TRAD_ID_26 20  //Authentification
#define SIZE_TRAD_ID_27 3   //Impossible de récupérer la MK
#define SIZE_TRAD_ID_28 2   //Obtention du mot de passe
#define SIZE_TRAD_ID_29 4   //Patch du registre
#define SIZE_TRAD_ID_30 11  //Nouveaux éléments détéctés à DL dans le lecteur
#define SIZE_TRAD_ID_31 6   //Interactions pour le DL de chapitre payants
