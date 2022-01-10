#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <assert.h>
#include <sys/stat.h>

typedef struct Person {
    char * gender;
    char * name;
    char * surname;
} Person;
Person **persons;
int personCount;

int countlines(char * filename){
    int count = 0; size_t len = 0; 
    FILE * fp; ssize_t read;
    char * line = NULL; 

    fp = fopen(filename, "r");
    while ((read = getline(&line, &len, fp)) != -1)
        count++;
    
    fclose(fp); return count;
}

char* concat(const char *string1, const char *string2)
{
    char * str = malloc(strlen(string1) + strlen(string2) + 1);
    strcpy(str, string1);
    strcat(str, string2);
    return str;
}

const char * filename_ext(const char *filename) {
    const char *dot = strrchr(filename, '.');
    if(!dot || dot == filename) return "";
    return dot + 1;
}

void corrector(char * path){
    DIR * srcdir = opendir(path);
    assert(srcdir != NULL);
    struct dirent * d;
    struct stat st;
    int tmp; char word[100];

    while ((d = readdir(srcdir)) != NULL){
        //printf("%s, %lu %s\n",path, (unsigned long) d->d_ino, d->d_name);
        if(strcmp(d->d_name, ".") == 0 || strcmp(d->d_name, "..") == 0)
            continue;

        if (fstatat(dirfd(srcdir), d->d_name, &st, 0) < 0){
            perror(d->d_name);
            continue;
        }

        if (S_ISDIR(st.st_mode)){
            char * dir = concat(path,d->d_name);
            dir = concat(dir,"/");
            corrector(dir);
        }

        if (strcmp(filename_ext(d->d_name), "txt") == 0 && (strcmp(d->d_name, "database.txt") != 0 || strcmp(path,"./") != 0)){
            char * f_name = concat(path,d->d_name);
            FILE * fd = fopen(f_name, "r+");
            if (fd == NULL){
                printf("Could not open txt file.");
                exit(EXIT_FAILURE);
            }

            while(fscanf(fd, "%s", word) > 0){
                for (int i = 0; i < personCount; i++) {
                    if (strcmp(persons[i]->name, word) == 0){
                        int n_len = strlen(persons[i]->name);
                        int loc = ftell(fd) - n_len;
                        //printf("Loc: %d, Word: %s, Length: %d\n", loc, persons[i]->name, n_len);

                        fseek(fd,loc-4,SEEK_SET);
                        fputs(strcmp(persons[i]->gender, "m") == 0 ? "Mr." : "Ms.",fd);
                        fseek(fd, ftell(fd) + n_len + 2, SEEK_SET);
                        fputs(persons[i]->surname,fd);
                    }
                }
            }
            fclose(fd);
        }
    }
    closedir(srcdir);
}

int main(int argc, char* argv[]){

    FILE* dataFile = fopen("./database.txt", "r");
    if (dataFile == NULL){
        printf("Could not open database.txt.");
        return -1;
    }

    personCount = countlines("./database.txt"); 
    //printf("%d",personCount);

    persons = malloc(personCount * sizeof(Person));

    char * line = NULL; 
    size_t len = 0; 
    ssize_t read;
    int count = 0;

    while ((read = getline(&line, &len, dataFile)) != -1){
        //printf("Retrieved line of length %zu:\n", read);
        //printf("Line: %s", line);

        char * gender = strtok(line, " ");
        char * name = strtok(NULL, " ");
        char * surname = strtok(NULL, " ");

        if (surname[strlen(surname)-1] == '\n')
            surname[strlen(surname)-1] = '\0';

        //printf("%s, %s, %s\n", gender, name, surname);

        persons[count] = (Person*)malloc(sizeof(Person));
        persons[count]->gender = (char*)malloc(strlen(gender) + 1);
        persons[count]->name = (char*)malloc(strlen(name) + 1);
        persons[count]->surname = (char*)malloc(strlen(surname) + 1);

        strcpy(persons[count]->gender, gender);
        strcpy(persons[count]->name, name);
        strcpy(persons[count]->surname, surname);

        count++;
    }

    // for (int i = 0; i < count; i++){
    //     printf("%s, %s, %s\n", persons[i]->gender, persons[i]->name, persons[i]->surname);
    // }
    
    fclose(dataFile);
    corrector("./");
    
    return 0;
}