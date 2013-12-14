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

typedef void(GUI_FUNC) (void * data, bool* cancel);
typedef struct REQ REQ;

struct REQ
{
	GUI_FUNC * functionName;		//Fonction à appelé
	void * dataToFunction;			//Argument
	uint32_t flags;					//Permet de contrôler la provenance de la requête
	uint32_t repeat;				//Si un thread indépendant doit répéter cette requête et si oui, intervale en ms
	bool * cancel;					//Pointeur permettant d'interrompre la requête
	
	MUTEX_VAR * lock;				//Mutex si l'appelant souhaite être verrouillé pendant le traitement
	REQ * next;						//Pointeur vers l'élément suivant
};

/**		Flags	   **/

//THREAD_ID	bytes 1&2

#define GUI_THREAD_READER			0x00010000
#define GUI_THREAD_CT				0x00020000
#define GUI_THREAD_MDL				0x00040000
#define GUI_THREAD_PROJECT			0x00080000

#define GUI_THREAD_MASK				0xFFFF0000

//Windows	byte 3

#define GUI_WINDOW_MAIN				0x00000100
#define GUI_WINDOW_PREFS			0x00000200

#define GUI_WINDOW_MASK				0x0000FF00

//Mode		byte 4

#define GUI_MODE_BIG				0x00000001	//onglet principal
#define GUI_MODE_SMALL				0x00000002	//onglet réduit

#define GUI_MODE_MASK				0x000000FF

#define GUI_MAINTHREAD_QUIT			(GUI_THREAD_MASK|GUI_WINDOW_MASK|GUI_MODE_MASK)	//Code bloqué, réservé au message _quit_

/** Public prototypes	**/
bool GUI_addPlan(GUI_FUNC * functionToCall, void* data, uint32_t flag, uint32_t repeat, bool *cancel, bool waitToBeDone);
bool isMainGUIThreadReady();
void GUI_startupMainGUIThread();

/**	Private prototypes	**/
void GUI_mainThread();				//Semi private, called during initialization
void GUI_initializaMainThread();
void GUI_processRequest(REQ * request);
void GUI_threadRepeatCall(REQ * request);
void GUI_freeChain(REQ *chain);

bool GUI_isWindowAvailable(uint32_t flag);