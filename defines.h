/*********************************************************************************************
**	__________         __           .__            __                 ________   _______   	**
**	\______   \_____  |  | __  _____|  |__ _____ _/  |______   	___  _\_____  \  \   _  \  	**
**	 |       _/\__  \ |  |/ / /  ___/  |  \\__  \\   __\__  \  	\  \/ //  ____/  /  /_\  \ 	**
**	 |    |   \ / __ \|    <  \___ \|   Y  \/ __ \|  |  / __ \__ \   //       \  \  \_/   \	**
**	 |____|_  /(____  /__|_ \/____  >___|  (____  /__| (____  /	  \_/ \_______ \ /\_____  /	**
**	        \/      \/     \/     \/     \/     \/          \/ 	              \/ \/     \/ 	**
**                                                                                          **
**    Licence propriétaire, code source confidentiel, distribution formellement interdite   **
**                                                                                          **
*********************************************************************************************/

/*DEFINES*/

/*Général*/
#define LARGEUR 800 //Largeur par défaut
#define HAUTEUR 730
#define BARRE_DES_TACHES_WINDOWS 75
#define PERMISSIONS 0755
#define NOMBRE_TEAM_MAX 100
#define TAILLE_BUFFER 500
#define MAX_INT 2147483646

#define CREATE_WINDOW_FLAG SDL_WINDOW_OPENGL|SDL_WINDOW_ALLOW_HIGHDPI

/*Thread*/
#ifdef _WIN32
    #define quit_thread(a) do{ \
                            MUTEX_LOCK(mutex);\
                            THREAD_COUNT--;\
                            MUTEX_UNLOCK(mutex);\
                            ExitThread(a);\
                            }while(0)

#else
    #define quit_thread(a) do{ \
                            MUTEX_LOCK(mutex);\
                            THREAD_COUNT--;\
                            MUTEX_UNLOCK(mutex);\
                            pthread_exit(a);\
                            }while(0)
#endif

/*Environnement*/
#define STORE_URL	"store.rakshata.com"
#define SERVEUR_URL "rsp.rakshata.com"
#define BACKUP_INTERNET_CHECK "http://www.apple.com/library/test/success.html"

#define REPO_DATABASE "data/repo"
#define MANGA_DATABASE "data/mangas"
#define INSTALL_DATABASE "data/import.dat"
#define FONT_USED_BY_DEFAULT "data/font.ttf"
#define TOME_INDEX "tomeDB"
#define CHAPITRE_INDEX "chapDB"

#define ICONE_DELETE "data/icon/d.png"
#define ICONE_FAVORITED "data/icon/f.png"
#define ICONE_FAVORIS_MENU "data/icon/fb.png"
#define ICONE_NOT_FAVORITED "data/icon/nf.png"
#define ICONE_FULLSCREEN "data/icon/fs.png"
#define ICONE_MAIN_MENU "data/icon/mm.png"
#define ICONE_MAIN_MENU_BIG "data/icon/mb.png"
#define ICONE_PREVIOUS_CHAPTER "data/icon/pc.png"
#define ICONE_PREVIOUS_PAGE "data/icon/pp.png"
#define ICONE_NEXT_PAGE "data/icon/np.png"
#define ICONE_NEXT_CHAPTER "data/icon/nc.png"
#define ICONE_LOCK "data/icon/l.png"
#define ICONE_UNLOCK "data/icon/u.png"
#define ICONE_SWITCH_CHAPITRE "data/icon/c.png"
#define ICONE_SWITCH_TOME "data/icon/t.png"

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
#define VERSION_MANGA 2

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
#define POS_BUTTON_STATUS_EN_COURS 0
#define POS_BUTTON_STATUS_SUSPENDU 1
#define POS_BUTTON_STATUS_TERMINE 2
#define POS_BUTTON_STATUS_FAVORIS 3
#define POS_BUTTON_STATUS_SHONEN 4
#define POS_BUTTON_STATUS_SHOJO 5
#define POS_BUTTON_STATUS_SEINEN 6
#define POS_BUTTON_STATUS_HENTAI 7

/*Structure principale -> paliers*/
#define PALIER_DEFAULT -1
#define PALIER_CHAPTER -2
#define PALIER_MENU -3
#define PALIER_QUIT -4

/*Langue*/
#define NOMBRE_LANGUE 4
#define LANGUE_PAR_DEFAUT 2
#define LARGEUR_LANGUE 800
#define HAUTEUR_LANGUE 325
#define HAUTEUR_MENU_LANGUE 15
#define HAUTEUR_TEXTE_LANGUE 50
#define INTERLIGNE_LANGUE 10

/*Police*/
#define POLICE_TOUT_PETIT 16
#define POLICE_PETIT 18
#define POLICE_MOYEN 22
#define POLICE_GROS 26

/*Couleurs*/
#define OLD_STYLE
#ifdef OLD_STYLE
    #define POLICE_R 0
    #define POLICE_G 0
    #define POLICE_B 0
    #define POLICE_NEW_R 203
    #define POLICE_NEW_G 47
    #define POLICE_NEW_B 47
    #define POLICE_UNREAD_R 48
    #define POLICE_UNREAD_G 170
    #define POLICE_UNREAD_B 0
    #define POLICE_ENABLE_R 203
    #define POLICE_ENABLE_G 47
    #define POLICE_ENABLE_B 47
    #define POLICE_UNAVAILABLE_R 77
    #define POLICE_UNAVAILABLE_G 77
    #define POLICE_UNAVAILABLE_B 77
    #define FOND_R 214
    #define FOND_G 214
    #define FOND_B 214
    #define BANDEAU_INFOS_LECTEUR_STYLES TTF_STYLE_UNDERLINE
#else
    #define POLICE_R 85
    #define POLICE_G 200
    #define POLICE_B 85
    #define POLICE_NEW_R 203
    #define POLICE_NEW_G 35
    #define POLICE_NEW_B 39
    #define POLICE_UNREAD_R 72
    #define POLICE_UNREAD_G 76
    #define POLICE_UNREAD_B 246
    #define POLICE_ENABLE_R 203
    #define POLICE_ENABLE_G 35
    #define POLICE_ENABLE_B 39
    #define POLICE_UNAVAILABLE_R 77
    #define POLICE_UNAVAILABLE_G 77
    #define POLICE_UNAVAILABLE_B 77
    #define FOND_R 54
    #define FOND_G 54
    #define FOND_B 54
    #define BANDEAU_INFOS_LECTEUR_STYLES TTF_STYLE_UNDERLINE|TTF_STYLE_BOLD
#endif

/*Settings*/
#define SECURE_DATABASE "data/secure.enc"
#define SETTINGS_FILE "data/settings"
#define SETTINGS_PASSWORD "Za8zR7e|3rz@[4zEbtr!s36~8713ue68"
#define SETTINGS_EMAIL_FLAG 'E'
#define SETTINGS_FAVORITE_FLAG 'F'
#define SETTINGS_LANGUE_FLAG 'L'
#define SETTINGS_MESSAGE_SECTION_FLAG 'S'
#define SETTINGS_MANGADB_FLAG 'M'
#define SETTINGS_REPODB_FLAG 'R'
#define SETTINGS_PASSWORD_FLAG 'P' //Pass de déverrouillage

/*Section*/
#define POSITION_ICONE_MENUS 10
#define TAILLE_ICONE_MENUS 60
#define NOMBRESECTION 4
#define BORDURE_SUP_MENU 35
#define INTERLIGNE_MENU 15
#define BORDURE_SUP_SECTION 75
#define INTERLIGNE_SECTION 10
#define BORDURE_VERTICALE_SECTION 75
#define HAUTEUR_FENETRE_SECTION 450

/*Selection manga*/
#define BORDURE_SUP_TITRE_MANGA 25
#define LARGEUR_MOYENNE_MANGA_PETIT 30
#define LARGEUR_MOYENNE_MANGA_MOYEN 36
#define LARGEUR_MOYENNE_MANGA_GROS 43
#define INTERLIGNE 5
#define BORDURE_SUP_SELEC_MANGA 140
#define BORDURE_SUP_SELEC_MANGA_LECTURE 160
#define MANGAPARPAGE_DUO 14
#define NBRCOLONNES_DUO 2
#define LONGUEURTEXTE 100
#define LONGUEURMANGA 200
#define BORDURELATSELECTION 45
#define LONGUEURMANGA_XL 247
#define BORDURELATSELECTION_XL 15
#define HAUTEURNUMEROTATION_OLD 50
#define HAUTEURNUMEROTATION_NEW 50
#define BANDEAU_INF_NEW 20
#define BORDURE_INF_NUMEROTATION_TRI 70
#define HAUTEUR_BOUTONS_DL 575
#define NOMBRE_LETTRE_MAX 20

/*Infos.png*/
#define HAUTEUR_INFOSPNG 40

/*Engine*/
#define ENGINE_ELEMENT_PAR_PAGE 30
#define ENGINE_NOMBRE_LIGNE 10
#define ENGINE_NOMBRE_COLONNE 3
#define MAX_LENGTH_TO_DISPLAY 50
#define ENGINE_OUTPUT_DEFAULT 0
#define ENGINE_OUTPUT_CLIC 1
#define ENGINE_OUTPUT_RESTRICTION 2
#define ENGINE_OUTPUT_PREV_PAGE 3
#define ENGINE_OUTPUT_NEXT_PAGE 4
#define ENGINE_OUTPUT_WEBSITE 5
#define ENGINE_OUTPUT_SWITCH 6
#define ENGINE_OUTPUT_QUIT 7
#define ENGINE_OUTPUT_DL_CANCEL 8
#define ENGINE_OUTPUT_DL_START 9
#define ENGINE_OUTPUT_BANDEAU_CTRL 10
#define ENGINE_OUTPUT_BOUTON_CHAPITRE 11
#define ENGINE_OUTPUT_TYPED_CHAPITRE 12
#define ENGINE_OUTPUT_MOUSE_ABOVE_TOME 13
#define ENGINE_OUTPUT_MOUSE_DL_RIGHT_CLICK 14
#define ENGINE_OUTPUT_BOUTON_CHAPITRE_1 1
#define ENGINE_OUTPUT_BOUTON_CHAPITRE_2 2
#define ENGINE_OUTPUT_BOUTON_CHAPITRE_3 3
#define ENGINE_RETVALUE_SWITCH -5
#define ENGINE_RETVALUE_DL_START -6
#define ENGINE_RETVALUE_DL_CANCEL -7

/*Multi Page*/
#define HAUTEUR_BOUTONS_CHANGEMENT_PAGE 105
#define MARGE_LATERALE_BOUTONS_MULTI_PAGE 20
#define LONGUEUR_PRECENDENT 169
#define LONGUEUR_SUIVANT 141

/*Bandeau de contrôle*/
#define LARGEUR_BANDEAU_CONTROLE_SELECTION_MANGA 140
#define HAUTEUR_PREMIERE_LIGNE_BANDEAU_CONTROLE 50
#define LARGEUR_INTERLIGNE_BANDEAU_CONTROLE 30
#define PREMIERE_COLONNE_BANDEAU_RESTRICTION 50
#define LARGEUR_MOYENNE_BOUTON_RESTRICTION 175
#define LARGEUR_COLONNE_BOUTON_RESTRICTION 180

/*Chapitre*/
#define NOMBRE_CHAPITRE_MAX 300
#define BORDURE_SUP_SELEC_CHAPITRE_FULL 150
#define BORDURE_SUP_SELEC_CHAPITRE_PARTIAL 110
#define BORDURE_SUP_TITRE_CHAPITRE 10
#define BORDURE_SUP_INFOS_MANGA 150
#define BORDURE_SUP_CHAPITRE 225
#define BORDURE_SUP_NUMEROTATION_CHAPITRE 450
#define BORDURE_SUP_INFOS_TEAM_CHAPITRE 55
#define HAUTEUR_BOUTONS_CHAPITRE 40
#define BORDURE_BOUTON_LECTEUR 20
#define SEPARATION_COLONNE_1_CHAPITRE 220
#define SEPARATION_COLONNE_2_CHAPITRE 570
#define VALEUR_FIN_STRUCTURE_CHAPITRE -1

/*Tome*/
#define MAX_TOME_NAME_LENGTH 31
#define TOME_DESCRIPTION_LENGTH 60
#define BORDURE_SUP_SELEC_TOME_FULL 150
#define BORDURE_SUP_SELEC_TOME_PARTIAL 110
#define HAUTEUR_INFOS_TOMES 75
#define HAUTEUR_PREMIERE_LIGNE_INFOS_TOME 70

/*Check environnement*/
#define NOMBRE_DE_FICHIER_A_CHECKER 41
#define LONGUEUR_NOMS_DATA 100

/*Show numéro*/
#define HAUTEUR_BORDURE_AFFICHAGE_NUMERO 30

/*Module Gestion*/
#define NOMBRE_MENU_GESTION 6
#define HAUTEUR_TEXTE 30
#define HAUTEUR_CHOIX 50
#define HAUTEUR_SELECTION_REPO 325

/*Ajout repo*/
#define BUFFER_MAX 100
#define HAUTEUR_MENU_AJOUT_REPO 50
#define HAUTEUR_FENETRE_AJOUT_REPO_INIT 200
#define HAUTEUR_FENETRE_AJOUT_REPO 250
#define HAUTEUR_ID_AJOUT_REPO 140
#define HAUTEUR_TEAM_AJOUT_REPO 175
#define LONGUEUR_ID_TEAM 10
#define TYPE_DEPOT_1 "DB" //Dépôt dropbox
#define TYPE_DEPOT_2 "O" //Dépôt classique
#define TYPE_DEPOT_3 "P" //Dépôts payants
#define TYPE_DEPOT_4 "GOO" //Raccourcis pour acceder au dépôt
#define TYPE_DEPOT_5 "MG" //Dépôts MEGA

/*Menu d'aide*/
#define HAUTEUR_FENETRE_AIDE 600
#define LARGEUR_FENETRE_AIDE 800

/*Suppression repo*/
#define HAUTEUR_DEL_REPO 275
#define HAUTEUR_MENU_CONFIRMATION_SUPPRESSION 50
#define HAUTEUR_TEAM_CONFIRMATION_SUPPRESSION 125
#define HAUTEUR_CONSIGNES_CONFIRMATION_SUPPRESSION 200
#define HAUTEUR_LISTE 100

/*Initialisation module de selection de DL*/
#define HAUTEUR_AFFICHAGE_INITIALISATION 75

/*Mise à jour*/
#define HAUTEUR_MAJ 250

/*Download.c*/
#define MODE_DOWNLOAD_VERBOSE_ENABLE 1
#define CODE_RETOUR_OK 0
#define CODE_RETOUR_DL_CLOSE 1
#define CODE_FAILED_AT_RESOLVE 2
#define CODE_RETOUR_INTERNAL_FAIL 3
#define CODE_RETOUR_MAX 3

#define STATUS_IT_IS_OVER -10
#define STATUS_FORCE_CLOSE -1
#define STATUS_END 0
#define STATUS_DOWNLOADING 1

#define SSL_OFF 0
#define SSL_ON 1

/*Database*/
#define NOMBRE_MANGA_MAX 1000
#define NOMBRE_MANGA_MAX_PAR_DEPOT 1000
#define LONGUEUR_NOM_MANGA_MAX 50
#define LONGUEUR_COURT 10
#define LONGUEUR_SITE 100
#define LONGUEUR_TYPE_TEAM 5
#define LONGUEUR_URL 300
#define SIZE_BUFFER_UPDATE_DATABASE (20*1024*1024)
#define NUMBER_MAX_TEAM_KILLSWITCHE 200
#define LOAD_DATABASE_INSTALLED 1
#define LOAD_DATABASE_ALL 2

/*Sécurité*/
#define EVERYTHING_IN_HDD 1
#define INPUT_IN_MEMORY 2
#define OUTPUT_IN_MEMORY 3
#define EVERYTHING_IN_MEMORY 4
#define NOMBRE_CLE_MAX_ACCEPTE 10
#define FULL_ENCRYPTION 1
#define MESSAGE_PASSWORD "0x4b4d1d341N33d4Password4b4d1d34"

/*Hash algorithms*/
#define SHA256_BLOCK_LENGTH	64
#define SHA256_DIGEST_LENGTH 32
#define PBKDF2_OUTPUT_LENGTH 32

/*Login*/
#define SIZE_WINDOWS_AUTHENTIFICATION 250

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
