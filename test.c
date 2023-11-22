#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <pthread.h>
#include <windows.h>
#include <stdlib.h>
#include <time.h>
#include <conio.h>
#include <dirent.h>

#define MATSIZE 150
#define DISPSIZE (MATSIZE*(MATSIZE+1))
#define CORE 8
#define TTASKNUM (MATSIZE/(CORE-2))
#define TTASKNUM_ (MATSIZE-(CORE-2)*TTASKNUM)

struct matsumpara
{
    int *destMat;
    int *srcMat;
    int flag;
    int len;
};
struct liveordiepara
{
    int *mat;
    int *summat;
    int l;
    int len;
};
struct pos
{
    int y;
    int x;
    int c;
    struct pos *pos_;
};
struct ttask
{
    pthread_t tid;
    void *(*task)(void *);
    void *paras;
    void *reparas;
    int flag;
    int id;
};

int Mat[MATSIZE][MATSIZE];
struct ttask ttasklist[CORE-1];
int tflag;

void disppos(int y, int x, int c);
void dispinit(void);
void* reflashDisp(void* p);
void* linesum(int *destMat, int *srcMat, int flag);
void* matsum(void *paras);
void* liveordie(void *paras);
void* oneday(int (*mat)[MATSIZE], int (*summat)[MATSIZE]);
int setMat(int (*mat)[MATSIZE]);

void* reflashDisp(void* p)
{
    struct pos *pos_l, *pos_f;
    // static clock_t t[10]={0}, tl=0;
    // static int n=0;

    pos_l = (struct pos *)p;
    while(pos_l!=NULL)
    {
        disppos(pos_l->y, pos_l->x, pos_l->c);
        pos_f = pos_l;
        pos_l = pos_l->pos_;
        free(pos_f);
    }
    // n++;
    // t[n%10] = clock() - tl;
    // tl = clock();
    // printf("\033[50;0H%3d", (t[0]+t[1]+t[2]+t[3]+t[4]+t[5]+t[6]+t[7]+t[8]+t[9])/10);
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
    if(c==1) puts("🟧");
    else puts("  ");
}

void* linesum(int *destMat, int *srcMat, int flag)
{
    int i;
    int ml;

    ml = 1;
    if(flag!=0) ml=MATSIZE;

    *(destMat) = *(srcMat) + *(srcMat+ml) + *(srcMat+(MATSIZE-1)*ml) ;
    *(destMat+(MATSIZE-1)*ml) = *(srcMat+(MATSIZE-2)*ml) + *(srcMat+(MATSIZE-1)*ml) + *(srcMat);
    for (i=1; i<(MATSIZE-1); i++)
    {
        *(destMat+i*ml) = *(srcMat+(i-1)*ml) + *(srcMat+i*ml) + *((srcMat+(i+1)*ml));
    }
    return (void *)0;
}

void* matsum(void *paras)
{
    int i;
    int ml;
    struct matsumpara *paras_ = ((struct matsumpara *)paras);

    ml = MATSIZE;
    if((paras_->flag)!=0) ml=1;

    for(i=0; i<paras_->len; i++)
    {
        linesum((paras_->destMat)+ml*i, (paras_->srcMat)+ml*i, paras_->flag);
    }

    return 0;
}

void* liveordie(void *paras)
{
    int j, i;
    int pn, *mat, *summat;
    struct liveordiepara *paras_ = ((struct liveordiepara *)paras);
    int l;
    struct pos pos_h, *pos_l;

    mat = paras_->mat;
    summat = paras_->summat;
    l = paras_->l;

    pos_h.pos_ = NULL;
    pos_l = &pos_h;

    for (j=0; j<paras_->len; j++)
    {
        for (i=0; i<MATSIZE; i++)
        {
            pn = *(summat+j*MATSIZE+i) - *(mat+j*MATSIZE+i);
            if(*(mat+j*MATSIZE+i)==0)
            {
                if(pn==3)
                {
                    *(mat+j*MATSIZE+i) = 1;
                    pos_l->pos_ = (struct pos *)malloc(sizeof(struct pos));
                    pos_l = pos_l->pos_;

                    pos_l->pos_ = NULL;
                    pos_l->x = i;
                    pos_l->y = j+l;
                    pos_l->c = 1;
                }
            }
            else
            {
                if(pn<2 || pn>3)
                {
                    *(mat+j*MATSIZE+i) = 0;
                    pos_l->pos_ = (struct pos *)malloc(sizeof(struct pos));
                    pos_l = pos_l->pos_;

                    pos_l->pos_ = NULL;
                    pos_l->x = i;
                    pos_l->y = j+l;
                    pos_l->c = 0;
                }
            }
        }
    }
    return (void *)pos_h.pos_;
}

int setMat(int (*mat)[MATSIZE])
{
    DIR *d;
    struct dirent *fl;
    FILE *f;
    char buffer[MATSIZE+1], fn[50];
    int ja, i, ia, n;

    ja = 2*MATSIZE/5;
    ia = 2*MATSIZE/5;
    i = 0;
    memset(buffer,'\0',sizeof(char)*(MATSIZE+1));
    memset(fn,'\0',sizeof(char)*(50));

    srand((unsigned)time(NULL));
    n = 3008*rand()/32767;
    d = opendir("all\\");
    while(n--)
    {
        fl = readdir(d);
    }
    sprintf(fn, "all\\\\%s", fl->d_name);
    closedir(d);

    printf("%s", fn);
    f = fopen(fn, "r");
    while(!feof(f))
    {
        fgets(buffer, MATSIZE, f);
        if(buffer[0]=='.' || buffer[0]=='O')
        {
            i = 0;
            while(*(buffer+i)!='\n' && *(buffer+i)!='\0')
            {
                if(*(buffer+i)=='O') mat[ja+2][i+ia+2] = 1;
                i++;
            }
            ja++;
        }
    }
    fclose(f);
    return 0;
}

void* taskpro(void *paras)
{
    int *id = (int *)paras;

    while(tflag!=0)
    {
        if(ttasklist[*id].flag!=0)
        {
            ttasklist[*id].reparas = ttasklist[*id].task(ttasklist[*id].paras);
            ttasklist[*id].flag = 0;
        }
    }
}

void tcontrol(void)
{
    int i;

    for(i=0; i<(CORE-1); i++)
    {
        ttasklist[i].id = i;
        pthread_create(&(ttasklist[i].tid), NULL, taskpro, &(ttasklist[i].id));
    }
}

int main(void)
{
    int i;
    int summat[MATSIZE][MATSIZE], tmpmat[MATSIZE][MATSIZE];
    struct matsumpara mps[CORE-1];
    struct liveordiepara lps[CORE-1];
    struct pos pos_h, *pos_l;

    memset(Mat,0,sizeof(int)*(MATSIZE*MATSIZE));
    memset(summat,0,sizeof(int)*(MATSIZE*MATSIZE));
    for(i=0; i<(CORE-1); i++)
    {
        ttasklist[i].tid = 0;
        ttasklist[i].task = NULL;
        ttasklist[i].paras = NULL;
        ttasklist[i].reparas = NULL;
        ttasklist[i].flag = 0;
    }
    ttasklist[CORE-2].task = reflashDisp;
    tflag = 1;

    setMat(Mat);
    dispinit();

    tcontrol();
    
    while(1)
    {
        for(i=0; i<(CORE-2); i++)
        {
            mps[i].destMat = tmpmat[i*TTASKNUM];
            mps[i].srcMat = Mat[i*TTASKNUM];
            mps[i].flag = 0;
            mps[i].len = TTASKNUM;

            ttasklist[i].task = matsum;
            ttasklist[i].paras = &(mps[i]);
            ttasklist[i].flag = 1;
        }
        if(TTASKNUM_>0)
        {
            mps[CORE-2].destMat = tmpmat[MATSIZE-TTASKNUM_];
            mps[CORE-2].srcMat = Mat[MATSIZE-TTASKNUM_];
            mps[CORE-2].flag = 0;
            mps[CORE-2].len = TTASKNUM_;
            matsum(&(mps[CORE-2]));
        }
        for(i=0; i<(CORE-2); i++)
        {
            while(ttasklist[i].flag!=0);
        }
        ///////////////////////////////////////////////////
        for(i=0; i<(CORE-2); i++)
        {
            mps[i].destMat = &(summat[0][i*TTASKNUM]);
            mps[i].srcMat = &(tmpmat[0][i*TTASKNUM]);
            mps[i].flag = 1;
            mps[i].len = TTASKNUM;

            ttasklist[i].task = matsum;
            ttasklist[i].paras = &(mps[i]);
            ttasklist[i].flag = 1;
        }
        if(TTASKNUM_>0)
        {
            mps[CORE-2].destMat = &(summat[0][MATSIZE-TTASKNUM_]);
            mps[CORE-2].srcMat = &(tmpmat[0][MATSIZE-TTASKNUM_]);
            mps[CORE-2].flag = 1;
            mps[CORE-2].len = TTASKNUM_;
            matsum(&(mps[CORE-2]));
        }
        for(i=0; i<(CORE-2); i++)
        {
            while(ttasklist[i].flag!=0);
        }
        ///////////////////////////////////////////////////
        pos_h.pos_ = NULL;
        pos_l = &pos_h;
        for(i=0; i<(CORE-2); i++)
        {
            lps[i].mat = Mat[i*TTASKNUM];
            lps[i].summat = summat[i*TTASKNUM];
            lps[i].l = i*TTASKNUM;
            lps[i].len = TTASKNUM;

            ttasklist[i].task = liveordie;
            ttasklist[i].paras = &(lps[i]);
            ttasklist[i].flag = 1;
        }
        if(TTASKNUM_>0)
        {
            lps[CORE-2].mat = Mat[MATSIZE-TTASKNUM_];
            lps[CORE-2].summat = summat[MATSIZE-TTASKNUM_];
            lps[CORE-2].l = MATSIZE-TTASKNUM_;
            lps[CORE-2].len = TTASKNUM_;
            pos_l->pos_ = liveordie(&(lps[CORE-2]));
            while(pos_l->pos_!=NULL)
            {
                pos_l = pos_l->pos_;
            }
        }
        for(i=0; i<(CORE-2); i++)
        {
            while(ttasklist[i].flag!=0);
            pos_l->pos_ = ttasklist[i].reparas;
            while(pos_l->pos_!=NULL)
            {
                pos_l = pos_l->pos_;
            }
        }
        ttasklist[CORE-2].paras = pos_h.pos_;
        while(ttasklist[CORE-2].flag!=0);
        ttasklist[CORE-2].flag = 1;
        // Sleep(100);
        if(kbhit()) 
        {
            if(getche()==27) break;
        }
    }
    tflag = 0;
    for(i=0; i<(CORE-1); i++) pthread_join(ttasklist[i].tid, NULL);
    printf("\033[%d;%dHEND", MATSIZE, 0);

    return 0;
}
// all\\p100piheptominohassler.cell