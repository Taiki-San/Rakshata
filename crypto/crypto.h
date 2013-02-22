/*********************************************************************************************
**      __________         __           .__            __                ____     ____      **
**      \______   \_____  |  | __  _____|  |__ _____ _/  |______    /\  /_   |   /_   |     **
**       |       _/\__  \ |  |/ / /  ___/  |  \\__  \\   __\__  \   \/   |   |    |   |     **
**       |    |   \ / __ \|    <  \___ \|   Y  \/ __ \|  |  / __ \_ /\   |   |    |   |     **
**       |____|_  /(____  /__|_ \/____  >___|  (____  /__| (____  / \/   |___| /\ |___|     **
**              \/      \/     \/     \/     \/     \/          \/             \/           **
**                                                                                          **
**   Licence propriétaire, code source confidentiel, distribution formellement interdite    **
**                                                                                          **
*********************************************************************************************/

#define CRYPTO_BUFFER_SIZE 16

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

void Serpent_set_key(unsigned long *l_key,const unsigned long *in_key, const unsigned long key_len);
void Serpent_encrypt(const unsigned long *l_key,const unsigned long *in_blk, unsigned long *out_blk);
void Serpent_decrypt(const unsigned long *l_key,const unsigned long *in_blk, unsigned long *out_blk);


/*****************************************************
**                                                  **
**                      Twofish                     **
**                                                  **
*****************************************************/

typedef struct {
	unsigned long	k_len;
	unsigned long	l_key[40];
	unsigned long	s_key[4];
    unsigned long	qt_gen;
	unsigned char	q_tab[2][256];
	unsigned long	mt_gen;
	unsigned long	m_tab[4][256];
	unsigned long	mk_tab[4][256];
} TWOFISH_DATA;

void Twofish_set_key(TWOFISH_DATA *pTfd,const unsigned long *in_key, const unsigned long key_len);
void Twofish_encrypt(const TWOFISH_DATA *pTfd,const unsigned long *in_blk, unsigned long *out_blk);
void Twofish_decrypt(const TWOFISH_DATA *pTfd,const unsigned long *in_blk, unsigned long *out_blk);

