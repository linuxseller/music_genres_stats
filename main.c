#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdint.h>
#include <assert.h>
#include <stdlib.h>
#include <dirent.h>
#include <errno.h>

#define HM_SIZE 2048
#include "hashmap.c"

#define EXPERIMENTAL      (1<<5)
#define EXTENDED_HEADER   (1<<6)
#define UNSYNCHRONISATION (1<<7)

int my_strncmp(char *a, char *b, int n)
{
    for(int i=0; i<n; i++){
        if(*a!=*b){return 1;}
        a++;
        b++;
    }
    return 0;
}

int swapEndian(int src)
{
    int dst;
    uint8_t *buf = (uint8_t*)&src;
    dst = (int)buf[0]<<24 | (int)buf[1]<<16 | (int)buf[2]<<8 | (int)buf[3]<<0;
    return dst;
}

char *tryReadGenreFromFilename(char *filename, int *offset)
{
    int fd = open(filename, O_RDONLY);
    char mime[3], frame_name[4];
    uint8_t version, revision, header_flags;
    int next_tag_size;
    int frame_size;
    read(fd, mime, 3);
    if(my_strncmp(mime, "ID3", 3)!=0){ // not mp3
        return NULL;
    }
    read(fd, &version, 1);
    read(fd, &revision, 1);
    read(fd, &header_flags, 1);
    read(fd, &next_tag_size, 4);
    if(header_flags & EXTENDED_HEADER){
        assert(0 && "extended headers are unsupported");
    }
    read(fd, frame_name, 4);
    while(my_strncmp(frame_name, "TCON", 4)!=0 && lseek(fd, 0, SEEK_CUR)<next_tag_size){
        read(fd, &frame_size, 4);
        frame_size = swapEndian(frame_size);
        lseek(fd, frame_size+2, SEEK_CUR);
        read(fd, frame_name, 4);
    }
    if(lseek(fd, 0, SEEK_CUR)>next_tag_size){
        return NULL;
    }
    read(fd, &frame_size, 4);
    frame_size = swapEndian(frame_size);
    read(fd, &header_flags, 1);
    char *content_type = malloc(frame_size*sizeof(char));
    if(content_type==NULL){
        printf("BUY MORE RAM BITCH!!!\n");
    }
    read(fd, content_type, frame_size);
    int start = 0;
    while(*(content_type+start++)==0);
    start--;
    close(fd);
    *offset = start;
    return content_type;
}

void updateDirectory(char *dirName)
{
    struct dirent *entry;
    DIR *dp = opendir(dirName);
    if (dp == NULL) {
        perror("opendir");
    }
    char full_filename[256];
    while ((entry = readdir(dp)) != NULL) {
        // Skip the current and parent directory entries
        if(entry->d_type == DT_DIR && entry->d_name[0] != '.'){
            sprintf(full_filename, "%s/%s", dirName, entry->d_name);
            printf("recursing into dir %s\n", full_filename);
            updateDirectory(full_filename);
            continue;
        }
        if (entry->d_name[0] != '.') {
            sprintf(full_filename, "%s/%s", dirName, entry->d_name);
            int off;
            char *genre = tryReadGenreFromFilename(full_filename, &off);
            if(genre!=NULL) addGenre(&genre[off]);
            free(genre);
        }

    }
    closedir(dp);
}

int main(int argc, char **argv){
    const char *directoryPath = "/home/user/Music"; // Change this to your desired directory
    if(argc==2){
        directoryPath = argv[1];
    }
    updateDirectory(directoryPath);
    sortHM();
    for(int i=0, hits=0; i<HM_SIZE && hits<10; i++){
        if(hm[i].count!=0){
            hits++;
            printf("%s has %d tracks\n", hm[i].genre, hm[i].count);
        }
    }

}
