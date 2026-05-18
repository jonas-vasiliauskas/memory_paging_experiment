#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdio.h>
#include <stdbool.h>

#define BLOCK_COUNT 64
#define WORD_COUNT_IN_BLOCK 16
#define WORD_SIZE 4
#define PAGES_PER_PROCESS 4

#define PTR 5
#define PTR_SIZE 1


const int USED_PAGES_MAP_SIZE = BLOCK_COUNT /(WORD_COUNT_IN_BLOCK*WORD_SIZE);
const int SYMBOL_COUNT_IN_BLOCK=WORD_COUNT_IN_BLOCK*WORD_SIZE;

int jv_strlen(unsigned char *str) {
    int len = 0;
    while (str[len] != '\0') 
        len++;
    
    return len;
}

void jv_strcpy(unsigned char *dest, const unsigned char *src) {
    while ((*dest++ = *src++) != '\0');
}

/*char* get_memory_block(char* memory){
    int i;
    static bool is_rand_init=false;
    if (!is_rand_init){
        srand(time(NULL));
        is_rand_init=true;
    }
    for (i=0;i<USED_PAGES_MAP_SIZE;++i)
        memory[i]=1;
    int block_id=USED_PAGES_MAP_SIZE;
    while (memory[block_id]==1)
        block_id = (rand() % (BLOCK_COUNT-1))+1;
    memory[block_id]=1;
    return (&memory[WORD_COUNT_IN_BLOCK*WORD_SIZE*block_id]);
}*/

int set_memory_block(unsigned char* memory,unsigned char proc_id,const int block_number,unsigned char* value_to_set){
    if (memory==NULL)
        return 1;
        
    if (((int)jv_strlen(value_to_set))>SYMBOL_COUNT_IN_BLOCK)
        return 2;
    if (memory[block_number]!=0)
        return 3;    
    jv_strcpy(&memory[block_number*SYMBOL_COUNT_IN_BLOCK],value_to_set);
    memory[block_number]=proc_id;
    //const int OFFSET = SYMBOL_COUNT_IN_BLOCK*PTR+block_number;
    //int i;
    //for (i=0;i<SYMBOL_COUNT_IN_BLOCK;++i)
    //memory[OFFSET+i]=proc_id;
    return 0;        
}

int set_memory(unsigned char* memory,unsigned char proc_id,unsigned char **values_to_set,const int values_to_set_count){
    if (memory==NULL)
        return 4;
    if (proc_id==0)
        return 5;
        
    static bool is_rand_init=false;
    
    if (!is_rand_init){
        srand(time(NULL));
        is_rand_init=true;
    }
    
    int i,failed_memory_alloc_count=0;
    
    for (i=0;i<values_to_set_count;++i){
        unsigned char random_block_id;
        do{
            do{ 
                random_block_id=abs(rand())%(BLOCK_COUNT-USED_PAGES_MAP_SIZE)+USED_PAGES_MAP_SIZE;
            }
            while ((random_block_id>=PTR)&&(random_block_id<(PTR+PTR_SIZE)));
            if (failed_memory_alloc_count++ > 100)
                return 6;    
            
        }while (set_memory_block(memory,proc_id,random_block_id,values_to_set[i]));
        failed_memory_alloc_count=0;
    }
        
    return 0;
}

char* get_message(const int code){
    switch (code){
        case 1:
        case 4:
            return "kintamasis 'memory' yra NULL";
        case 2:
            return "kintamojo 'value_to_set' reiksme per ilga eilute";
        case 3:
            return "atminties puslapis jau uzimtas";     
        case 5:
            return "neteisingas proceso indentifikatorius: '0'";
        case 6:
            return "neuztenka atminties";
        case 7:
            return "netinkamas programos parametru skaicius";  
        default:
            return "nera tokio klaidos kodo";
    }
    
}

char read_char() {
    int c = getchar();
    int discard;

    while ((discard = getchar()) != '\n' && discard != EOF);

    return c;
}

void print_memory(unsigned char *memory,bool is_hex){
    int i,j,k;
    
    if (is_hex){
        for (i=0;i<16;++i)
            printf("%9X",i);
        puts("");
    }
    else{
        printf("%c",' ');
        for (i=0;i<16;++i)
            printf("%5X",i);
        puts("");
    }     
        
    for (i=0;i<BLOCK_COUNT;++i){
        printf("%3X ",i);
        for (j=0;j<WORD_COUNT_IN_BLOCK;++j){
            putc(' ',stdout); 
            for (k=0;k<WORD_SIZE;++k){
                const int offset=i*(WORD_COUNT_IN_BLOCK*WORD_SIZE)+j*WORD_SIZE+k;
                if (offset<USED_PAGES_MAP_SIZE){
                    if (is_hex)
                        fputs("**",stdout);
                    else putc('*',stdout);
                    continue;
                }   
                if (is_hex)
                    printf("%02X",memory[offset]);
                else printf("%c",memory[offset]);
            }
        }        
        puts("");
    }    
}

int main(int arg_c,char **arg_v){
    if (arg_c == 1){
        fprintf(stderr,"%s\n","Netinkamas programos parametru skaicius");
        return 1;
    }
    if (arg_c == 2 && (strcmp(arg_v[1],"\?")==0)){
       printf("%s%s%s\n","programos naudojimo pavyzdys ",arg_v[0]," <kartojimu skaicius>");
       return 0;
    }
    int i,error;
    char proccesses[]={'A','B','C','D','E','F','G','H'};
    unsigned char *memory = (unsigned char*) calloc(sizeof(unsigned char),WORD_SIZE*WORD_COUNT_IN_BLOCK*BLOCK_COUNT);
   
    unsigned char  *values[]={(unsigned char*)"XXXXXXXXXXXX"};
    if ((error=set_memory(memory,'X',values,sizeof(values)/sizeof(unsigned char*)))!=0){
        fprintf(stderr,"%s%i%s%s%s\n","klaida vykdant 'set_memory' klaidos kodas: ",error," \"",get_message(error),"\"");
        free(memory);
        return 1;
    }
    
    bool is_hex=false;
    if (arg_c >= 3)
        if (strcmp(arg_v[2],"--hex")==0)
            is_hex=true;
    
    print_memory(memory,is_hex);
    puts("---------------------------------------------------------------------------------");
    
    const int iteration_count = atoi(arg_v[1]);
    if (iteration_count==0 && strcmp(arg_v[1],"0")!=0){
        fprintf(stderr,"%s\n","Blogi parametrai");
        free(memory);
        return 1;
    }
    for (i=0;i<iteration_count;++i){
        unsigned char  *values[]={(unsigned char*)"Labas rytas",(unsigned char*)"Kaip jums sekasi",(unsigned char*)"Ar gerai issimiegojote -------------------\
------------",
        (unsigned char*)"Sudie"};
        int proccess_id=rand()%(sizeof(proccesses)/sizeof(char));
        if ((error=set_memory(memory,proccesses[proccess_id],values,sizeof(values)/sizeof(unsigned char*)))!=0){
            fprintf(stderr,"%s%i%s%s%s\n","klaida vykdant 'set_memory' klaidos kodas: ",error," \"",get_message(error),"\"");
            free(memory);
            return 1;
        }
    
    }
    if (iteration_count != 0)
        print_memory(memory,is_hex);
    bool is_time_to_exit=false;
    
    do{ 
        unsigned char proc_id;

        puts("Ar ieskosite konkretaus proceso duomenu bloku?");
        char choice=read_char();
        if ((choice=='t')||(choice=='T')){
            puts("Iveskite proceso indentifikatoriu");
            proc_id=read_char();
            for (i=0;i<BLOCK_COUNT;++i)
                if (memory[i]==proc_id){
                    puts("Radau");
                    break;
                }
        }
        else if ((choice=='n')||(choice=='N')){
                 is_time_to_exit=true;
                 break;
             }
             else puts("Blogas pasirinkimas"); 
        choice=0;         
        
    }while (!is_time_to_exit);
    free(memory);
    return 0; 
}
