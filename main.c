#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <pthread.h>
#include <windows.h>
#include <stdlib.h>
#include <time.h>

#define MATSIZE 150
#define DISPSIZE (MATSIZE*(MATSIZE+1))

struct linesumpara
{
    int *destMat;
    int *srcMat;
    int flag;
};
struct liveordiepara
{
    int *mat;
    int *summat;
    int l;
};
struct pos
{
    int y;
    int x;
    int c;
    struct pos *pos_;
};
struct disppara
{
    struct pos *dispbuffer;
    int flag;
};

int Mat[MATSIZE][MATSIZE];
struct disppara dp;

void disppos(int y, int x, int c);
void dispinit(void);
void* reflashDisp(void* p);
void* linesum(void *paras);
int matsum(int (*srcmat)[MATSIZE], int (*destmat)[MATSIZE] );
void* liveordie(void *paras);
void* oneday(int (*mat)[MATSIZE], int (*summat)[MATSIZE]);
int setMat(int (*mat)[MATSIZE]);

void* reflashDisp(void* p)
{
    struct pos *pos_l, *pos_f;

    while(1)
    {
        if(dp.flag!=0)
        {
            pos_l = dp.dispbuffer;
            while(pos_l!=NULL)
            {
                disppos(pos_l->y, pos_l->x, pos_l->c);
                pos_f = pos_l;
                pos_l = pos_l->pos_;
                free(pos_f);
            }
            dp.flag = 0;
        }
    }
}

void dispinit(void)
{
    int j, i;

    printf("\033[2J\033[?25l");
    for(j=0; j<MATSIZE; j++)
    {
        for(i=0; i<MATSIZE; i++)
        {
            disppos(j, i, Mat[j][i]);
        }
    }
}

void disppos(int y, int x, int c)
{   
    COORD pos = {2*x, y};
    SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), pos);
    if(c==1) printf("🟧");
    else printf("  ");
}

void* linesum(void *paras)
{
    int i;
    int ml;
    int *destMat, *srcMat;
    struct linesumpara *paras_ = ((struct linesumpara *)paras);

    destMat = paras_->destMat;
    srcMat = paras_->srcMat;

    ml = 1;
    if(paras_->flag!=0) ml=MATSIZE;

    *(destMat) = *(srcMat) + *(srcMat+ml);
    *(destMat+(MATSIZE-1)*ml) = *(srcMat+(MATSIZE-2)*ml) + *(srcMat+(MATSIZE-1)*ml);
    for (i=1; i<(MATSIZE-1); i++)
    {
        *(destMat+i*ml) = *(srcMat+(i-1)*ml) + *(srcMat+i*ml) + *((srcMat+(i+1)*ml));
    }
    return (void *)0;
}

int matsum(int (*srcmat)[MATSIZE], int (*destmat)[MATSIZE] )
{
    int i;
    pthread_t t_ls[MATSIZE];
    struct linesumpara paras[MATSIZE];
    int tmpmat[MATSIZE][MATSIZE];

    memset(tmpmat,0,sizeof(int)*(MATSIZE*MATSIZE));

    for(i=0; i<MATSIZE; i++)
    {
        paras[i].destMat = tmpmat[i];
        paras[i].srcMat = srcmat[i];
        paras[i].flag = 0;
        // linesum(&paras);
        pthread_create(&t_ls[i], NULL, linesum, &(paras[i]));
    }
    for(i=0; i<MATSIZE; i++)
    {
        pthread_join(t_ls[i], NULL);
    }

    for(i=0; i<MATSIZE; i++)
    {
        paras[i].destMat = &(destmat[0][i]);
        paras[i].srcMat = &(tmpmat[0][i]);
        paras[i].flag = 1;
        // linesum(&paras);
        pthread_create(&t_ls[i], NULL, linesum, &(paras[i]));
    }
    for(i=0; i<MATSIZE; i++)
    {
        pthread_join(t_ls[i], NULL);
    }
    return 0;
}

void* liveordie(void *paras)
{
    int i;
    int pn, *mat, *summat;
    struct liveordiepara *paras_ = ((struct liveordiepara *)paras);
    int l;
    struct pos pos_h, *pos_l;

    mat = paras_->mat;
    summat = paras_->summat;
    l = paras_->l;

    pos_h.pos_ = NULL;
    pos_l = &pos_h;

    for (i=0; i<MATSIZE; i++)
    {
        pn = *(summat+i) - *(mat+i);
        if(*(mat+i)==0)
        {
            if(pn==3)
            {
                *(mat+i) = 1;
                pos_l->pos_ = (struct pos *)malloc(sizeof(struct pos));
                pos_l = pos_l->pos_;

                pos_l->pos_ = NULL;
                pos_l->x = i;
                pos_l->y = l;
                pos_l->c = 1;
            }
        }
        else
        {
            if(pn<2 || pn>3)
            {
                *(mat+i) = 0;
                pos_l->pos_ = (struct pos *)malloc(sizeof(struct pos));
                pos_l = pos_l->pos_;

                pos_l->pos_ = NULL;
                pos_l->x = i;
                pos_l->y = l;
                pos_l->c = 0;
            }
        }
    }
    return (void *)pos_h.pos_;
}

void* oneday(int (*mat)[MATSIZE], int (*summat)[MATSIZE])
{
    int i;
    pthread_t t_lod[MATSIZE];
    struct liveordiepara paras[MATSIZE];
    struct pos pos_h, *pos_l;

    pos_h.pos_ = NULL;
    pos_l = &pos_h;

    for(i=0; i<MATSIZE; i++)
    {
        paras[i].mat = mat[i];
        paras[i].summat = summat[i];
        paras[i].l = i;
        pthread_create(&t_lod[i], NULL, liveordie, &(paras[i]));
    }
    for(i=0; i<MATSIZE; i++)
    {
        pthread_join(t_lod[i], (void **)(&(pos_l->pos_)));
        while(pos_l->pos_!=NULL)
        {
            pos_l = pos_l->pos_;
        }
    }
    while(dp.flag!=0);
    if(pos_h.pos_!=NULL)
    {
        dp.dispbuffer = pos_h.pos_;
        dp.flag = 1;
    }
    return (void *)pos_h.pos_;
}

int setMat(int (*mat)[MATSIZE])
{
    FILE *f;
    char buffer[MATSIZE+1];
    int j, i;

    j = 0;
    i = 0;
    memset(buffer,'\0',sizeof(char)*(MATSIZE+1));

    f = fopen("p.txt", "r");
    while(!feof(f))
    {
        fgets(buffer, MATSIZE, f);
        i = 0;
        while(*(buffer+i)!='\n' && *(buffer+i)!='\0')
        {
            if(*(buffer+i)=='1') mat[j+2][i+2] = 1;
            i++;
        }
        j++;
    }
    fclose(f);
    return 0;
}

int main(void)
{
    int i;
    int summat[MATSIZE][MATSIZE];
    pthread_t t_disp;

    memset(Mat,0,sizeof(int)*(MATSIZE*MATSIZE));
    memset(summat,0,sizeof(int)*(MATSIZE*MATSIZE));
    dp.dispbuffer = NULL;
    dp.flag = 0;

    setMat(Mat);
    dispinit();

    pthread_create(&t_disp, NULL, reflashDisp, NULL);

    i = 0;
    while (1)
    {
        matsum(Mat, summat);
        oneday(Mat, summat);
        // reflashDisp();
        // Sleep(50);
        // printf("%d\n", i++);
    }
    return 0;
}