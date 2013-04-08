/******************************************************************************************************
**  __________         __           .__            __                  ____     ____     ________    **
**  \______   \_____  |  | __  _____|  |__ _____ _/  |______    ___  _/_   |   /_   |    \_____  \   **
**   |       _/\__  \ |  |/ / /  ___/  |  \\__  \\   __\__  \   \  \/ /|   |    |   |     /  ____/   **
**   |    |   \ / __ \|    <  \___ \|   Y  \/ __ \|  |  / __ \_  \   / |   |    |   |    /       \   **
**   |____|_  /(____  /__|_ \/____  >___|  (____  /__| (____  /   \_/  |___| /\ |___| /\ \_______ \  **
**          \/      \/     \/     \/     \/     \/          \/               \/       \/         \/  **
**                                                                                                   **
**         Licence propriétaire, code source confidentiel, distribution formellement interdite       **
**                                                                                                   **
******************************************************************************************************/

#define CRYPTO_BUFFER_SIZE 16
typedef unsigned long DWORD;

/*****************************************************
**                                                  **
**                      Rijndael                    **
**                                                  **
*****************************************************/

int rijndaelSetupEncrypt(unsigned long *rk, const unsigned char *key, int keybits);
int rijndaelSetupDecrypt(unsigned long *rk, const unsigned char *key, int keybits);
void rijndaelEncrypt(const unsigned long *rk, int nrounds, const unsigned char plaintext[16], unsigned char ciphertext[16]);
void rijndaelDecrypt(const unsigned long *rk, int nrounds, const unsigned char ciphertext[16], unsigned char plaintext[16]);

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

void Serpent_set_key(SERPENT_STATIC_DATA *l_key,const unsigned long *in_key, const unsigned long key_len);
void Serpent_encrypt(SERPENT_STATIC_DATA *l_key,const unsigned long *in_blk, unsigned long *out_blk);
void Serpent_decrypt(SERPENT_STATIC_DATA *l_key,const unsigned long *in_blk, unsigned long *out_blk);


/*****************************************************
**                                                  **
**                      Twofish                     **
**                                                  **
*****************************************************/

typedef unsigned long   u4byte;

unsigned long Twofish_set_key(const unsigned long *in_key, const unsigned long key_len);
void Twofish_encrypt(const unsigned long *in_blk, unsigned long *out_blk);
void Twofish_decrypt(const unsigned long *in_blk, unsigned long *out_blk);

