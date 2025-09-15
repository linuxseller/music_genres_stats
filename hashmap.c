struct genre_count_HM {
    char *genre;
    int count;
};

unsigned long hash(unsigned char *str)
{
    unsigned long hash = 5381;
    int c;
    while ((c = *str++))
        hash = ((hash << 5) + hash) + c; /* hash * 33 + c */

    return hash;
}

struct genre_count_HM hm[HM_SIZE];

void addGenre(char *genre)
{
    unsigned long hsh = hash((unsigned char*)genre)%HM_SIZE;
    if(hm[hsh].count==0){
        hm[hsh].genre = calloc(strlen(genre), 1);
        strcpy(hm[hsh].genre, genre);
    }
    hm[hsh].count++;
}

int compare_genre_count_HM(const void *a, const void *b)
{
    return ((struct genre_count_HM*) b)->count - ((struct genre_count_HM*)a)->count;
}

void sortHM(void)
{
    qsort(hm, HM_SIZE, sizeof(struct genre_count_HM), compare_genre_count_HM);
}
