/*********************************************************************************************
**	__________         __           .__            __                 ________   _______   	**
**	\______   \_____  |  | __  _____|  |__ _____ _/  |______   	___  _\_____  \  \   _  \  	**
**	 |       _/\__  \ |  |/ / /  ___/  |  \\__  \\   __\__  \  	\  \/ //  ____/  /  /_\  \ 	**
**	 |    |   \ / __ \|    <  \___ \|   Y  \/ __ \|  |  / __ \__ \   //       \  \  \_/   \	**
**	 |____|_  /(____  /__|_ \/____  >___|  (____  /__| (____  /	  \_/ \_______ \ /\_____  /	**
**	        \/      \/     \/     \/     \/     \/          \/ 	              \/ \/     \/ 	**
**                                                                                          **
**    Licence propritaire, code source confidentiel, distribution formellement interdite   **
**                                                                                          **
*********************************************************************************************/

#ifndef uint32_t
	#include <stdint.h>
#endif

#define CRYPTO_BUFFER_SIZE 16
typedef uint32_t DWORD;
typedef unsigned char BYTE;

/*****************************************************
**                                                  **
**                      Rijndael                    **
**                                                  **
*****************************************************/

typedef DWORD RK_KEY;

int rijndaelSetupEncrypt(RK_KEY *rk, const unsigned char *key, int keybits);
int rijndaelSetupDecrypt(RK_KEY *rk, const unsigned char *key, int keybits);
void rijndaelEncrypt(const RK_KEY *rk, int nrounds, const rawData plaintext[16], rawData ciphertext[16]);
void rijndaelDecrypt(const RK_KEY *rk, int nrounds, const rawData ciphertext[16], rawData plaintext[16]);

#define KEYLENGTH(keybits) ((keybits)/8)
#define RKLENGTH(keybits)  ((keybits)/8+28)
#define NROUNDS(keybits)   ((keybits)/32+6)

#define KEYBITS 256


/*****************************************************
**                                                  **
**                      Serpent                     **
**                                                  **
*****************************************************/

typedef struct {
	DWORD	key[140];
} SERPENT_STATIC_DATA;

void Serpent_set_key(SERPENT_STATIC_DATA *l_key,const DWORD *in_key, const DWORD key_len);
void Serpent_encrypt(SERPENT_STATIC_DATA *l_key,const DWORD *in_blk, DWORD *out_blk);
void Serpent_decrypt(SERPENT_STATIC_DATA *l_key,const DWORD *in_blk, DWORD *out_blk);


/*****************************************************
**                                                  **
**                      Twofish                     **
**                                                  **
*****************************************************/

#define Q_TABLES
#define M_TABLE
#define MK_TABLE
#define ONE_STEP

typedef struct {
	DWORD	k_len;
	DWORD	l_key[40];
	DWORD	s_key[4];
	
#ifdef  Q_TABLES
	DWORD	qt_gen;
	BYTE	q_tab[2][256];
#endif
	
#ifdef  M_TABLE
	DWORD	mt_gen;
	DWORD	m_tab[4][256];
#endif
	
#ifdef  MK_TABLE
#ifdef  ONE_STEP
	DWORD	mk_tab[4][256];
#else
	BYTE	sb[4][256];
#endif
#endif
} TWOFISH_DATA;

void Twofish_set_key(TWOFISH_DATA *pTfd,const DWORD *in_key, const DWORD key_len);
void Twofish_encrypt(const TWOFISH_DATA *pTfd,const DWORD *in_blk, DWORD *out_blk);
void Twofish_decrypt(const TWOFISH_DATA *pTfd,const DWORD *in_blk, DWORD *out_blk);
