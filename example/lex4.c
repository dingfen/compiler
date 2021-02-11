struct abc {
    int a,d;
    char b;
    short c;
};

int main()
{
    char *d = "awesbsdq";
    int i;
    for(i = 0; i < strlen(d); i=i+1) {
        if (d[i] > 's') {
            d[i] = d[i]+1;  /* for d[i]++ */
        } else {
            d[i] = 0;
        }
    }
    return 0;
}