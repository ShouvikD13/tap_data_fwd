/** Ver 1.1 | Password Encryption and Decryption method changes| Sachin Birje **/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>  /* VER TOL : TUX on LINUX -- Changed header from time.h to sys/time.h */
#include <fn_log.h>
#include <fn_rndm_passwd_genrn.h>

/**   Description of the funtion `fn_get_passwd`
 * Generates a new password that differs from the old password and meets specific criteria.
 *
 * The new password includes:
 * - Uppercase letters (A-Z)
 * - A lowercase letter (a-z)
 * - A special character (@/#$%&*)
 * - Two numeric digits (1-9)
 *
 * The length of the new password is adjusted by subtracting 3 to account for the mandatory components.
 * 
 * Steps:
 * 1. Initializes a password buffer.
 * 2. Randomly selects characters and assembles the password.
 * 3. Shuffles characters to randomize order.
 * 4. Recursively regenerates the password if it matches the old one.
 * 5. Stores the new password in `c_new_passwd`.
 *
 * Parameters:
 * - `c_old_passwd`: The old password.
 * - `c_new_passwd`: Buffer for the new password.
 * - `l_passwd_len`: Desired length of the new password.
 *
 * Returns:
 * - 0 on success.
 */

int fn_get_passwd(char *c_old_passwd,char *c_new_passwd,int l_passwd_len)
{
  long l_rand_num;
  char *c_char_C="ABCDEFGHIJKLMNOPQRSTUVWXYZ";
  char *c_char_S="abcdefghijklmnopqrstvuwxyz";
  char c_num[9] = "123456789";
  char *c_spl_char= "@/#$%&*";
  char c_passwd[MAX_PSSWD_LEN];
  int i;
  int i_ret_val;

  memset(c_passwd,'\0',sizeof(c_passwd));
  
  l_passwd_len = l_passwd_len-3;  
 
  for(i=0;i<l_passwd_len-1;i++)
  {
  c_passwd[i]=c_char_C[fn_rand_val(1,25)];
  }
  
  c_passwd[i]=c_char_S[fn_rand_val(1,25)];

  i = i + 1;
  c_passwd[i]=c_spl_char[fn_rand_val(1,6)];
 
  i=i+1;
  c_passwd[i]= c_num[fn_rand_val(1,8)];
  i=i+1;
  c_passwd[i]= c_num[fn_rand_val(1,8)];
 
  rtrim(c_passwd);

  fn_str_shuffle(c_passwd);

  if( strcmp(c_old_passwd,c_passwd) == 0 )
  {
   memset(c_new_passwd,'\0',sizeof(c_passwd));
   i_ret_val = fn_get_passwd(c_old_passwd,c_new_passwd,l_passwd_len);
  }
 
  strcpy(c_new_passwd,c_passwd);
  
  return 0;
}


int fn_rand_val(int start,int end)
{
 long l_randum_num;
 struct timeval tv;
 time_t curtime;
 gettimeofday(&tv, NULL);
 curtime=tv.tv_sec;

 srand(tv.tv_usec);

 l_randum_num = rand() % (end - start ) + start;
 
 return l_randum_num;
}

/**  Description of the funtion `fn_str_shuffle`
 * Randomly shuffles the characters in the given string using the Fisher-Yates algorithm.
 *
 * Process:
 * 1. Iterates from the end of the string to the beginning.
 * 2. For each position, selects a random index.
 * 3. Swaps the character at the current position with the character at the random index.
 *
 * Parameters:
 * - `c_shuffle`: The string to be shuffled.
 *
 * Modifies the input string in-place to produce a randomly shuffled version.
 */

void fn_str_shuffle(char *c_shuffle)
{
    int i, random, length = strlen(c_shuffle);
    char temp;

    for (i = length-1; i > 0; i--)
    {
        random = rand()%(i+1);
        temp = c_shuffle[random];
        c_shuffle[random] = c_shuffle[i];
        c_shuffle[i] = temp;
    }
}


void fn_encrypt(char *str , char *en_str)
{
int i , j;
long l_len; 

l_len = strlen(str);

for (i=0, j=0;i<l_len;i++)
{
  if(i%2 == 0)
   /** en_str[j++] = str[i] + 5 + i; ** Ver 1.1 **/
    en_str[j++] = str[i] + 2;        /** Ver 1.1 **/
  else
    /** en_str[j++] = str[i] + 3 + i; *** Ver 1.1 **/
    en_str[j++] = str[i] + 1;         /** Ver 1.1 **/
}

en_str[j] = '\0';

}

void fn_decrypt(char *en_str, char *de_str)
{
int i , j;
long l_len; 

l_len = strlen(en_str);

for (i=0;i<l_len;i++)
{
  if(i%2 == 0)
    de_str[i] = en_str[i] - 2;         
  else
    de_str[i] = en_str[i] - 1;         
}

de_str[i] = '\0';

}
