#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define TAM 2000

int main (int argc, char **argv) {
    if (argc != 4) {
        printf("Usage: %s (qnt_less_than_six) (qnt_between_six_ten) (qnt_more_ten)\n", argv[0]);
        return 1;
    }

    int less_than_six = atoi(argv[1]),
        between_six_ten = atoi(argv[2]), 
        more_ten = atoi(argv[3]);

    int max = TAM;

    FILE * f;
    f = fopen("test.txt", "at");

    printf("TOTAL: %d\n", (less_than_six+between_six_ten+more_ten));
    printf("less six: %d\n", less_than_six);
    printf("between six ten: %d\n", between_six_ten);
    printf("more ten: %d\n", more_ten);
    char *str = malloc(sizeof(char)*TAM);
    while(less_than_six > 0 || between_six_ten > 0 || more_ten > 0) {
        str[0] = '\0';
        for( int i = 0; i < 10; i++) {
            if(less_than_six > 0) {
                strcat(str, "xxx ");
                less_than_six--;
            }
            if(between_six_ten > 0) {
                strcat(str, "xxxxxxx ");
                between_six_ten--;
            }

            if(more_ten > 0) {
                strcat(str, "xxxxxxxxxxxx ");
                more_ten--;
            }
        }
        str[strlen(str)-1] = '\0';
        printf("%s*\n", str);
        fprintf(f,"%s\n",str);
    }
    fclose(f);
    free(str);
    
return 0;
}