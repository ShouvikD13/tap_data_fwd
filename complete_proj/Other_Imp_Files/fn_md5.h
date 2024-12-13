/******************************************************/
/** MD5 structure and functions declarations    		 **/
/**	Ver 1.0  Shailesh Hinge 								 	  	   **/
/******************************************************/

typedef unsigned char *POINTER;

typedef unsigned short int UINT2;

typedef unsigned long int UINT4;

typedef struct {
  UINT4 state[4];                                   
  UINT4 count[2];        
  unsigned char buffer[64];   
} MD5_CTX;

void MD5Init (MD5_CTX *);
void MD5Update (MD5_CTX *, unsigned char *, unsigned int);
void MD5Final (unsigned char [16], MD5_CTX *);
void fn_cal_md5digest(void *, unsigned int , unsigned char []);
int Authenticate_msg( void *, unsigned int , unsigned char *);
