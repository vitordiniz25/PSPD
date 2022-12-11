#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/*
    n_l_six:  quantidade de palavras com menos de 6 bytes;
    n_m_six:  quantidade de palavras entre 6 e 10 bytes;
    n_lm_six: quantidade de palavras com mais de 10 bytes;
*/
int main(){

    int n_l_six, n_m_six, n_lm_six;
    
    scanf("%d %d %d", &n_l_six, &n_m_six, &n_lm_six);

    if(n_l_six < 0 || n_m_six < 0 || n_lm_six < 0){
        printf("Somente valores positivos!!\n");
        return 0;
    }
    
    char s1[5], s2[8], s3[12];

    memset(s1, 'a', 4);
    memset(s2, 'a', 7);
    memset(s3, 'a', 11);

    s1[4] = '\0';
    s2[7] = '\0';
    s3[11] = '\0';

    if(n_l_six) printf("%s", s1);
    for(int i=0; i<n_l_six-1; i++){
        printf(" %s", s1);
    }

    if(!n_l_six && n_m_six) {
        printf("%s", s2);
        n_m_six--;
    }
    for(int i=0; i<n_m_six; i++){
        printf(" %s", s2);
    }

    if(!n_l_six && !n_m_six && n_lm_six) {
        printf("%s", s3);
        n_lm_six--;
    }
    for(int i=0; i<n_lm_six; i++){
        printf(" %s", s3);
    }

return 0;
}