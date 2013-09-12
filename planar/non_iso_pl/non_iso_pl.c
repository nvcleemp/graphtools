/* version without orbit computations so that the array numbering
  isn't needed and one can go further */

/* compile with "-lmd" and "-DMD5" for md5 encryption. Option: md5 */
/* since gcc version whatsoever libmd and the header files have vanished. 
  Forget it... */

#include<unistd.h>
#include<stdlib.h>
#include<stdio.h>
#include<ctype.h>
#include<limits.h>
#include<string.h>
#include <time.h>
#include <sys/times.h>
#ifdef MD5
//#include <gcrypt.h>
//#include "md5.h" /* compile with "-lmd" */
//#include "md5c.c"
#endif
#include "hashfunction.c"

#define N 200     /* Maximal moegliche Anzahl der Knoten KN_MAX-2*/
#define MAXVAL 10 //MAXE  /* maximale valenz */
#define MAXFACE 20 /* maximale Flaechengroesse 255 */
#define MAXN (N+1) /* maximal USHRT_MAX -- anders probleme mit overlap by colour */
#define MAXE (6*MAXN)
#define MAXCODELENGTH (MAXN+MAXE+3)

#define aussen   (N+2) /* so werden Kanten nach aussen markiert */

#define infty    LONG_MAX
#define FL_MAX   UCHAR_MAX
#define KN_MAX   USHRT_MAX
#define unbelegt FL_MAX
#define leer     KN_MAX-1
#define f_leer    FL_MAX-1
#define False    0
#define True     1
#define nil      0
#define reg      3

#define NUMBERHASHES 1 // it uses 2*numberhashes uints as hash value
#define HASHFIELDEXPONENT 24 //the initial hashfield has size 1<<HASHFIELDEXPONENT


#define SUBDIVISION (1<<17)
int nvbeforesubdiv=INT_MAX, numbersubdivided=0;
#define ISSUBDIVVERTEX(i) ((i)>nvbeforesubdiv) 
/* this depends on the order in which edges are subdivided! */

/* Typ-Deklarationen: */

typedef  char BOOL; /* von 0 verschieden entspricht True */


typedef unsigned short GRAPH[N+1][MAXVAL]; /* fuer schreibegraph und Isomorphietest */

/* Element der Adjazenztabelle: */

typedef struct K {
                  unsigned short start; /* bei welchem knoten startet die kante */
                  unsigned short end;  /* Identifikation des Knotens, mit
                                      dem Verbindung besteht */
		   long dummy;   /* fuer alle moeglichen zwecke */
		   int kantennummer;
		   int mark;
		   int faceleft, faceright; /* die Flaechengroessen rechts und links der
					       Kante */
                  struct K *prev;  /* vorige Kante im Uhrzeigersinn */
                  struct K *next;  /* naechste Kante im Uhrzeigersinn */
		   struct K *invers; /* die inverse Kante (in der Liste von "end") */
                 } EDGE;

/* "Ueberschrift" der Adjazenztabelle (Array of Pointers): */
typedef EDGE PLANMAP[N+1][MAXVAL];
                 /* Jeweils N EDGEs */
                 /* ACHTUNG: 1. Zeile der Adjazenztabelle hat Index 0 -
                    wird fast nicht benutzt, N Zeilen auch ausreichend
		     In [0][0].end wird aber die knotenzahl gespeichert 
		     und in [0][1].end die Zahl der gerichteten Kanten */


typedef struct sp { short *graph;
                   int length;
                   int nummer;
 struct sp *left, *right, *parent; } SPLAYNODE;


typedef struct hashfield { uint32_t graphhash1[2*NUMBERHASHES];
                          uint32_t graphhash2[2*NUMBERHASHES];
                          struct hashfield *next;} HASHFIELD;

SPLAYNODE *worklist=NULL;

HASHFIELD *hashlist;
int write_new=0, write_old=0, output=0, writemap_old=0, writemap_new=0;
int without_mirror=0, do_rooted=0, do_rooted_v=0, do_rooted_e=0;
int do_rooted_f=0, do_rooted_fl=0, write_roots=0, only_roots=0;
int rest=0, mod=0;
int use_hash=0;
int print_ascii=0, print_ascii_short=0, md5out=0;
unsigned long long int numrootedf=0, numrootedv=0, numrootede=0, numrootedfl=0;
int nv, ne; /* Knoten und Kantenzahl des Graphen -- noch aus historischen Gruenden aus
	       plantri.c global */
char codetype=0;

/* extra arguments to pass to scan procedure */
#define SCAN_ARGS

/* what scan procedure should do for each node p */
#define ACTION(p) outputnode(p)

/* extra arguments for the insertion procedure */
#define INSERT_ARGS , short *canong, int laenge, int nummer , int *test

/* how to compare the key of INSERT_ARGS to the key of node p */
#define COMPARE(p) comparenodes(canong, laenge, p)

/* what to do for a new node p */
#define NOT_PRESENT(p) new_splaynode(p, canong, laenge, nummer, test)

/* what to do for an old code p */
#define PRESENT(p) old_splaynode(p, nummer, test)



static int canon_form();
void new_splaynode();
void old_splaynode();

PLANMAP map;

EDGE *firstedge[MAXN+1];
int degree[MAXN+1];

//static EDGE *numbering[2*MAXE][MAXE]; 
 /* holds numberings produced by canon() or canon_edge() */

static int markvalue = 300000;
#define RESETMARKS {int mki,mkj; if ((markvalue += 2) > 300000) \
      { markvalue = 2; \
	 for (mki=1;mki<=N;++mki) for (mkj=0;mkj<MAXVAL;++mkj) \
	      map[mki][mkj].mark=0;}}
#define MARKLO(e) (e)->mark = markvalue
#define MARKHI(e) (e)->mark = markvalue+1
#define UNMARK(e) (e)->mark = markvalue-1
#define ISMARKED(e) ((e)->mark >= markvalue)
#define ISMARKEDLO(e) ((e)->mark == markvalue)
#define ISMARKEDHI(e) ((e)->mark > markvalue)

/* and the same for vertices */

static int markvalue_v = 300000;
static int marks__v[MAXN+1];
#define RESETMARKS_V {int mki; if ((markvalue_v += 2) > 300000) \
      { markvalue_v = 1; for (mki=0;mki<MAXN;++mki) marks__v[mki]=0;}}
#define UNMARK_V(x) (marks__v[x] = 0)
#define ISMARKED_V(x) (marks__v[x] >= markvalue_v)
#define MARK_V(x) (marks__v[x] = markvalue_v)
#define ISMARKEDLO_V(x) (marks__v[x] == markvalue_v)
#define MARKLO_V(x) (marks__v[x] = markvalue_v)
#define ISMARKEDHI_V(x) (marks__v[x] == markvalue_v+1)
#define MARKHI_V(x) (marks__v[x] = markvalue_v+1)

int elmarkvalue=INT_MAX;
int elmarks[MAXE+1];
#define RESETMARKS_EL {if (elmarkvalue==INT_MAX) \
                       {int i; for (i=0;i<=MAXE;i++) elmarks[i]=0; elmarkvalue=1; }\
                  else elmarkvalue++; }
#define MARK_EL(x) (elmarks[x]=elmarkvalue)
#define ISMARKED_EL(x) (elmarks[x]==elmarkvalue)
#define UNMARKED_EL(x) (elmarks[x]!=elmarkvalue)


#include "splay.c"

/**************************************************************************/

#define ALLOCSIZE 100000

short *canformmalloc(int howmany, int free)
{
 /* allocates howmany short ints and returns the pointer. It uses that
    IF something is freed it is always the last place allocated -- previously
    allocated ones are never freed. */
 static int last_size= ALLOCSIZE;
 static int memstart=ALLOCSIZE;
 static int lastwasfree=1;
 static short *mymemory=NULL;

 if (free) 
   { if (lastwasfree) { fprintf(stderr,"Error -- twice freeing.\n");
                        exit(1); }
     memstart -= last_size; 
     lastwasfree=1;
     return NULL;
   }

 lastwasfree=0;
 last_size=howmany;
 if (memstart+howmany>ALLOCSIZE) /* forget the small rest */
   { mymemory=malloc(ALLOCSIZE*sizeof(short));
   if (mymemory==NULL)
     { fprintf(stderr,"Do not get more memory -- exiting!\n"); exit(1); }
   memstart=0;
   }

 memstart += howmany;
 return mymemory+memstart-howmany;
}

HASHFIELD *hashcalloc()
{
 static int memstart=ALLOCSIZE;
 static HASHFIELD *mymemory=NULL;

 if (memstart>=ALLOCSIZE)
   { mymemory=calloc(ALLOCSIZE,sizeof(HASHFIELD));
   if (mymemory==NULL)
     { fprintf(stderr,"Do not get more memory -- exiting!\n"); exit(1); }
   memstart=0;
   }

 memstart++;

 return mymemory+memstart-1;
}

int newhash(uint32_t *value)
/* inserts the value into the hashlist if not yet present and returns 1
  if the value is new and 0 otherwise. */
{ 

 return hashit(hashlist+
		(value[2*NUMBERHASHES-2]&((uint32_t)((1<<HASHFIELDEXPONENT)-1)))
		,value);
}


int hashit(HASHFIELD *where, uint32_t *value)
{
 int adres;
 static uint32_t empty[2*NUMBERHASHES]={0};

 if (memcmp(where->graphhash1,value,2*NUMBERHASHES*sizeof(uint32_t))==0)
   return 0;
 if (memcmp(where->graphhash1,empty,2*NUMBERHASHES*sizeof(uint32_t))==0)
   { memcpy(where->graphhash1,value,2*NUMBERHASHES*sizeof(uint32_t)); return 1; }


 if (memcmp(where->graphhash2,value,2*NUMBERHASHES*sizeof(uint32_t))==0)
   return 0;
 if (memcmp(where->graphhash2,empty,2*NUMBERHASHES*sizeof(uint32_t))==0)
   { memcpy(where->graphhash2,value,2*NUMBERHASHES*sizeof(uint32_t)); return 1; }

 if (where->next==NULL)
   { where->next=hashcalloc();

     memcpy(where->next->graphhash1,value,2*NUMBERHASHES*sizeof(uint32_t));
     return 1;
   }

 return hashit(where->next,value);
}


static void
check_itp1(int code, int triang)

/* Checks these properties:
  1. Degrees are correct.
  2. Faces are triangles (if triang)
  3. start and end fields are correct.
  4. min fields are ok.
  5. vertex numbers are in range.
*/

{
   int i,j;
   EDGE *e;

   for (i = 1; i <= nv; ++i)
   {
	/*
       if (degree[i] < 3)
       {
           fprintf(stderr,">E degree error, code=%d\n",code);
           exit(1);
       }
	*/

       e = firstedge[i];
       for (j = 0; j < degree[i]; ++j) e = e->next;
       if (e != firstedge[i])
       {
           fprintf(stderr,">E next error, code=%d\n",code);
           exit(1);
       }

       e = firstedge[i];
       for (j = 0; j < degree[i]; ++j) e = e->prev;
       if (e != firstedge[i])
       {
           fprintf(stderr,">E prev error, code=%d\n",code);
           exit(1);
       }

       e = firstedge[i];
       for (j = 0; j < degree[i]; ++j)
       {
           e = e->next;
           if (e->start != i)
           {
               fprintf(stderr,">E start error, code=%d\n",code);
               exit(1);
           }
	    if (/*e->end < 0 ||*/ e->end >= nv)
           {
               fprintf(stderr,">E end label error, code=%d\n",code);
               exit(1);
           }
	    if (e->end != e->invers->start)
           {
               fprintf(stderr,">E invers label error, code=%d\n",code);
               exit(1);
	    }
           if (e->invers->end != i)
           {
               fprintf(stderr,">E end error, code=%d\n",code);
               exit(1);
           }
	    if (triang)
               if (e->invers->next == e
                   || e->invers->next->invers->next == e
                   || e->invers->next->invers->next->invers->next != e)
           {
               fprintf(stderr,">E face error, code=%d\n",code);
               exit(1);
           }

	    /*            if (e->min != e && e->min != e->invers)
           {
               fprintf(stderr,">E min error 1, code=%d\n",code);
               exit(1);
           }

	    if (e->invers->min != e->min)
	    {
               fprintf(stderr,">E min error 2, code=%d\n",code);
               exit(1);
		}*/
       }
   }
}


/************************************************/

void markface_right(EDGE *start)

/* marks the face on the right hand side of start */
{
EDGE *run;

run=start;
do {MARKLO(run); run=run->invers->prev;} while(run != start);
}


/************************************************/

void markvertex(EDGE *start)

/* marks the face on the right hand side of start */
{
EDGE *run;

run=start;
do {MARKLO(run); run=run->next;} while(run != start);
}


/*****************************************************************/

void writemap()
{
int i,j;
EDGE *run;

fprintf(stderr,"\n\n");

for (i=1; i<=nv; i++)
 { fprintf(stderr,"%d:",i);
   run=firstedge[i];
   for (j=0; j<degree[i]; j++) { fprintf(stderr," %d ",run->end); run=run->next; }
   fprintf(stderr,"\n");
 }
fprintf(stderr,"\n\n");
}



/*************************DECODIEREPLANAR******************************/

void decodiereplanar(unsigned short* code, PLANMAP graph, int adj[N])
{
int i,j,k,puffer,zaehler, kantenzaehler, knotenzahl, counter;
EDGE *edge, *run;
unsigned char mark[N];
int loops=0; /* are loops detected ? */
int tree_remember;

numbersubdivided=0;
knotenzahl=graph[0][0].end = code[0];
graph[0][1].end=0;

for (i=2; i<=knotenzahl; i++) mark[i]=0;
mark[1]=1;

kantenzaehler=0;
zaehler=1;

for(i=1;i<=knotenzahl;i++)
   { adj[i]=0;
     for(j=0; code[zaehler]; j++, zaehler++) 
	{ if (j==MAXVAL) { fprintf(stderr,"MAXVAL too small: %d\n",MAXVAL);
			   exit(0); }
	if (code[zaehler]==i) loops++;
	  graph[i][j].end=code[zaehler];
	  graph[i][j].start=i;
	  graph[i][j].invers=NULL;
	  if (code[zaehler]>i) { kantenzaehler++; graph[i][j].kantennummer=kantenzaehler; }
	  else graph[i][j].kantennummer=0;
	}
     adj[i]=j; graph[0][1].end += j;
     for(j=1, k=0; j<adj[i]; j++, k++)     
	{ graph[i][j].prev=graph[i]+k; graph[i][k].next=graph[i]+j; }
     graph[i][0].prev=graph[i]+adj[i]-1; graph[i][adj[i]-1].next=graph[i];
     zaehler++; /* 0 weglesen */
   }

kantenzaehler += (loops/2);

/* Die folgende Methode ist prinzipiell zwar leider quadratisch, hat aber den Vorteil,
  keinen zusaetzlichen Speicher zu verbrauchen und in der Praxis trotzdem in der
  Praxis schnell zu sein: */

/* First initialize the spanning tree given by the first occurences of the vertices 
  in the lists: */

for(i=1;i<=knotenzahl;i++)
   { 
     for(j=0; j<adj[i]; j++) 
	{ puffer=graph[i][j].end;
	if (mark[puffer] == 0)
	  { mark[puffer]=1;
	    for (k=0; graph[puffer][k].end != i; k++);
	    graph[i][j].invers=graph[puffer]+k;
	    graph[puffer][k].invers=graph[i]+j;
	    graph[puffer][k].kantennummer = -(graph[i][j].kantennummer);
	  }
	}
   }

/* Now the spanning tree of well defined edge identifications is built 
  and the other edges can be inserted */

for(i=1;i<=knotenzahl;i++)
   { 
     for (j=0; j<adj[i]; j++) 
	/* loops will be treated separately */
	if ((graph[i][j].invers == NULL) && (graph[i][j].start != graph[i][j].end))
	/* graph[i][j] is left of an already embedded edge. Now we will run along the
	   already embedded part in clockwise direction to find the position to
	   glue it to */
	{ run=(graph[i][j].next);
	  puffer=graph[i][j].end;
	  counter=1; /* How often have I passed another edge with the same start and
			endpoint that still must be assigned before reaching the 
			destination ? One must leave docking points free for these. 
			(Multiple edges.) */
	  while (run->start!=puffer)
	    { if ((run->start==i) && (run->end==puffer) && (run->invers==NULL)) counter++;
	      if (run->invers==NULL) run=run->next; else run=run->invers->next;
	    }
	  /* OK -- Now I have reached the destination and must just run to find
	     the docking point and leave enough docking points free */
	  while (counter)
	    { if (run->end==i) counter--;
	      if (counter) run=run->next;
	    }
	  /* At this point we could now assign ALL the other edges i<->puffer, but this
	     would be an advantage of course only in the case of edges with high
	     multiplicity */
	  graph[i][j].invers=run; run->invers=graph[i]+j;
	  if (run->kantennummer) graph[i][j].kantennummer= -run->kantennummer;
	  else run->kantennummer= -graph[i][j].kantennummer;

	}
   }

if (loops)
 { if (3*knotenzahl-6 != kantenzaehler)
   { fprintf(stderr,"This is not a triangulation.\n");
     fprintf(stderr,"The planarcode is only well defined for loops in triangulations \n");
     exit(0); }
   fprintf(stderr,"Not yet prepared for loops in planarcode -- used edgecode...\n");
   exit(0);
 }



for (i=1; i<=knotenzahl; i++) firstedge[i]=graph[i];
nv=nvbeforesubdiv=knotenzahl;
ne=2*kantenzaehler;
}

/**************************NEW_SPLAYNODE********************************/

void new_splaynode(SPLAYNODE *el, short *canong, int laenge, int nummer , int *test)

{

if (write_new) fprintf(stderr,"Number %d is new\n",nummer);
if (writemap_new) writemap();
el->nummer=nummer;
el->graph= canong;
el->length=laenge;
*test=1;
}

/**************************OLD_SPLAYNODE********************************/

void old_splaynode(SPLAYNODE *el, int nummer , int *test)

{
	                 
*test=0;
if (write_old) fprintf(stderr,"Number %d and number %d are isomorphic\n",nummer, el->nummer); 
if (writemap_old) writemap();

}

/****************************comparenodes***************************/

int comparenodes(short *canong, int laenge, SPLAYNODE *list)

{
int compare;
int n;

compare = laenge - list->length;
if (compare==0)
 compare=memcmp(canong,list->graph,laenge*sizeof(short));
return compare;
}





#ifdef MD5
md5tos(char *md5, char *s)
{
   static int byteval[256];
   static int init = 0;
   int i,j,x;

   if (!init)
   {
	for (i = 0; i < 10; ++i) byteval['0'+i] = i;
       for (i = 0; i < 6; ++i) byteval['a'+i] = 10+i;
       for (i = 0; i < 6; ++i) byteval['A'+i] = 10+i;
	init = 1;
   }

   j = 0;
   for (i = 0; i < 32; i += 2)
   {
	x = (byteval[md5[i]] << 4) + byteval[md5[i+1]];
	if (x == 0 || x == '\n') ++x;
	s[j++] = x;
   }
   s[j] = '\0';
}
#endif


/**************************NEU*************************************/

int neu( int laenge, int nummer, int colour[])
/* Entscheidet, ob ein graph schon in der Liste behandelter Graphen ist 
  und fuegt ihn ein, wenn nicht

  Im Falle von print_ascii==1 wird nur die kanonische Form berechnet und 
  geschrieben.

*/
{
short *can_form;
uint32_t hashvalues[2*NUMBERHASHES];
int m,n,test,test2, auts, auts_or_pres;
unsigned char charbuffer[33],printbuffer[17];


can_form=(short *)canformmalloc(laenge,0);
//if (can_form==NULL) { fprintf(stderr,"Don't get more space...\n"); exit(1); }
//if (do_rooted) canon_form(colour, numbering, &auts, &auts_or_pres, can_form);
//else 

canon_form(colour, NULL, &auts, &auts_or_pres, can_form);



if (print_ascii) { 
#ifdef MD5
  if (md5out)
{ MD5Data((unsigned char *)can_form, laenge*sizeof(unsigned short), charbuffer); 
md5tos(charbuffer, printbuffer);
  fprintf(stdout,"%s\n",printbuffer);  

canformmalloc(0,1); return 0; }
#else
  if (md5out) { fprintf(stderr,"Not compiled with md5option -DMD5\n");
                exit(0); }
#endif

if (!print_ascii_short)
 { for (m=0; m<laenge; m++) fprintf(stdout,"%d ",can_form[m]); 
 fprintf(stdout,"\n");
 }
else
  { for (m=0; m<laenge; m++)
    {  n=can_form[m];
    if (n>62) { fprintf(stderr,"short ascii only for up to 61 vertices\n"); 
                exit(0); }
    if (n<10) fprintf(stdout,"%c",n+48);
    else if (n<36) fprintf(stdout,"%c",n+55);
    else fprintf(stdout,"%c",n+61);
    }
  fprintf(stdout,"\n");
  }
canformmalloc(0,1);
return 0;
}/* end print ascii */

if (use_hash)
  { int i;
  for (i=0;i<2*NUMBERHASHES;i++)
    { if (i<laenge) hashvalues[i]=can_form[laenge-i-1];
      else hashvalues[i]=(char)(can_form[i%laenge]); }

  for (i=0;i<NUMBERHASHES;i++)
    {
      hashlittle2((char *)(can_form+(i*laenge/NUMBERHASHES)),
		   (sizeof(short))*laenge/NUMBERHASHES,
		   hashvalues+2*i, hashvalues+(2*i+1));
    }
  test=newhash(hashvalues);
  }
else
  {
    if (!only_roots) /*in_liste(liste,can_form,laenge, nummer,&test);*/
      splay_insert(&worklist,can_form,laenge, nummer,&test);
    else test=1;
  }


if ((test==0) || only_roots || use_hash) canformmalloc(0,1);
/*
if (test && do_rooted)
 {
   if (do_rooted_f) 
{ test2 = numfaceorbits(auts,auts_or_pres);
numrootedf+=test2;
if (write_roots) fprintf(stderr,"Number %d face roots %d \n",nummer,test2);
}
if (do_rooted_e) 
{ test2 = numedgeorbits(auts,auts_or_pres);
numrootede+=test2;
if (write_roots) fprintf(stderr,"Number %d edge roots %d \n",nummer,test2);
}
if (do_rooted_fl) 
{ test2 = numflagorbits(auts,auts_or_pres);
numrootedfl+=test2;
if (write_roots) fprintf(stderr,"Number %d flag roots %d \n",nummer,test2);
}
if (do_rooted_v) 
{ test2 = numvertexorbits(auts,auts_or_pres);
numrootedv+=test2;
if (write_roots) fprintf(stderr,"Number %d vertex roots %d \n",nummer,test2);
}
}
*/


return(test);
}

/*********************OUTPUTNODE***********************************/

outputnode(SPLAYNODE *liste)

{
static char first=1;
int i;

if (first) { first=0; fprintf(stdout,">>planar_code<<"); }

if ((liste->graph)[0]>250)
 { putc(0,stdout);
   fwrite(liste->graph,sizeof(unsigned short),liste->length,stdout);
 }
else
 for (i=0;i<liste->length; i++) putc((liste->graph)[i],stdout);
}


/**************************************************************************/

static int 
testcanon(EDGE *givenedge, int representation[], int colour[])

/* Tests whether starting from a given edge and constructing the code in
  "->next" direction, an automorphism or even a better representation 
  can be found. Returns 0 for failure, 1 for an automorphism and 2 for 
  a better representation.  This function exits as soon as a better 
  representation is found. A function that computes and returns the 
  complete better representation can work pretty similar.*/
{
	EDGE *temp, *run;  
	EDGE *startedge[MAXN+1]; /* startedge[i] is the starting edge for 
				exploring the vertex with the number i+1 */
	int number[MAXN], i;   /* The new numbers of the vertices, starting 
				at 1 in order to have "0" as a possibility to
				mark ends of lists and not yet given numbers */
	int last_number, actual_number, vertex, col;

	for (i = 1; i <= nv; i++) number[i] = 0;

	number[givenedge->start] = 1; 
	if (givenedge->start != givenedge->end)
	  {
	    number[givenedge->end] = 2;
	    actual_number = 1;
	    last_number = 2;
	    startedge[1] = givenedge->invers;
	  }
	else
	  { last_number = actual_number = 1; }

	temp = givenedge;

/* A representation is a clockwise ordering of all neighbours ended with a 0.
  The neighbours are numbered in the order that they are reached by the BFS 
  procedure. Every number is preceded by the colour of the vertex.
  Since every representation starts with "2" and the same colour, we do not 
  have to note that. 
  In fact there is one exception from this rule: In case the construction 
  starts with a loop, the first entry of the representation would be "1"
  (The colour would still be the same). Nevertheless this cannot produce
  the same represenation around vertex 1 once for a loop and once for a 
  non-loop as start, since in case the start is a loop, there is an odd
  number of "1's" in the remaining representation and in case of a non-loop
  as start, there is an even number.
  But in fact around vertex 1 we are not really taking the lexicographic
  order starting at startedge, but startedge->next.... In cases without a
  loop this doesn't make any difference, but once starting at a loop and 
  once not, it does !

  Every first entry in a new clockwise ordering (and its 
  colour) is determined by the entries before (the first time it occurs in 
  the list to be exact), so this is not given either. 
  The K4 could be denoted  c0 3 c1 4 0 c1 4 c0 3 0 c3 2 c1 4 0 c0 3 c3 2 0 
  if c0 is the colour of vertex 3, c1 that of vertex 4 and c3 that of 
  vertex 2. Note that the colour of vertex 1 is -- by definition -- always 
  the smallest one */

	while (last_number < nv)
	{  
  	    for (run = temp->next; run != temp; run = run->next)
	/* this loop marks all edges around temp->origin. */
	      { vertex = run->end;
	  	col = colour[vertex];
	  	if (col > (*representation)) return(0);
	  	if (col < (*representation)) return(2);
	  	representation++;
	  	if (!number[vertex]) { startedge[last_number] = run->invers;
				last_number++; number[vertex] = last_number; }
	  	vertex = number[vertex];
	  	if (vertex > (*representation)) return(0);
	  	if (vertex < (*representation)) return(2);
	  	representation++;
	      }
  /* check whether representation[] is also at the end of a cyclic list */
  	    if ((*representation) != 0) return(2); 
  	    representation++;
  /* Next vertex to explore: */
  	    temp = startedge[actual_number];  actual_number++; 
	}

	while (actual_number <= nv) 
			/* Now we know that all numbers have been given */
	{  
  	    for (run = temp->next; run != temp; run = run->next)
	/* this loop marks all edges around temp->origin. */
	      { vertex = run->end;
	    	col = colour[vertex];
	    	if (col > (*representation)) return(0);
	    	if (col < (*representation)) return(2);
	    	representation++;
	    	vertex = number[vertex];
	    	if (vertex > (*representation)) return(0);
	    	if (vertex < (*representation)) return(2);
	    	representation++;
	      }
  /* check whether representation[] is also at the end of a cyclic list */
    	    if ((*representation) != 0) return(2); 
  	    representation++;
  /* Next vertex to explore: */
  	    temp = startedge[actual_number];  actual_number++; 
	}

	return(1);
}

/*****************************************************************************/

static int 
testcanon_mirror(EDGE *givenedge, int representation[], int colour[])

/* Tests whether starting from a given edge and constructing the code in
  "->prev" direction, an automorphism or even a better representation can 
  be found. Comments see testcanon -- it is exactly the same except for 
  the orientation */
{
	EDGE *temp, *run;  
	EDGE *startedge[MAXN+1]; /* startedge[i] is the starting edge for 
			       exploring the vertex with the number i+1 */
	int number[MAXN], i; /* The new numbers of the vertices, starting 
				at 1 in order to have "0" as a possibility to
		                mark ends of lists and not yet given numbers */
	int last_number, actual_number, vertex, col;

	for (i = 1; i <= nv; i++) number[i] = 0;

	number[givenedge->start] = 1; 
	if (givenedge->start != givenedge->end)
	  {
	    number[givenedge->end] = 2;
	    actual_number = 1;
	    last_number = 2;
	    startedge[1] = givenedge->invers; 
	  }
	else
	  { last_number = actual_number = 1; }

	temp = givenedge;


/* A representation is a clockwise ordering of all neighbours ended with a 0.
  The neighbours are numbered in the order that they are reached by the BFS 
  procedure. Every number is preceded by the colour of the vertex.
  Since every representation starts with "2" and the same colour, we do not 
  have to note that. Every first entry in a new clockwise ordering (and its 
  colour) is determined by the entries before (the first time it occurs in 
  the list to be exact), so this is not given either. 
  The K4 could be denoted  c0 3 c1 4 0 c1 4 c0 3 0 c3 2 c1 4 0 c0 3 c3 2 if
  c0 is the colour of vertex 3, c1 that of vertex 4 and c3 that of vertex 2. 
  Note that the colour of vertex 1 is -- by definition -- always the 
  smallest one */

	while (last_number < nv)
	{  
  	    for (run = temp->prev; run != temp; run = run->prev)
	/* this loop marks all edges around temp->origin. */
	      { vertex = run->end;
	  	col = colour[vertex];
	  	if (col > (*representation)) return(0);
	  	if (col < (*representation)) return(2);
	  	representation++;
	  	if (!number[vertex]) { startedge[last_number] = run->invers;
				 last_number++; number[vertex] = last_number; }
	  	vertex = number[vertex];
	  	if (vertex > (*representation)) return(0);
	  	if (vertex < (*representation)) return(2);
	  	representation++;
	      }
	      
  /* check whether representation[] is also at the end of a cyclic list */
  	    if ((*representation) != 0) return(2); 
  	    representation++;
  /* Next vertex to explore: */
  	    temp = startedge[actual_number];  actual_number++; 
	}

	while (actual_number <= nv) 
			  /* Now we know that all numbers have been given */
	{  
	    for (run = temp->prev; run != temp; run = run->prev)
	/* this loop marks all edges around temp->origin. */
	      { vertex = run->end;
	  	col = colour[vertex];
	  	if (col > (*representation)) return(0);
	  	if (col < (*representation)) return(2);
	  	representation++;
	  	vertex = number[vertex];
	  	if (vertex > (*representation)) return(0);
	  	if (vertex < (*representation)) return(2);
	  	representation++;
	      }
  /* check whether representation[] is also at the end of a cyclic list */
  	    if ((*representation) != 0) return(2); 
  	    representation++;
  /* Next vertex to explore: */
  	    temp = startedge[actual_number];  actual_number++; 
	}

	return(1);
}

/****************************************************************************/

static void
testcanon_first_init(EDGE *givenedge, int representation[], int colour[])

/* Tests whether starting from a given edge and constructing the code in
  "->next" direction, an automorphism or even a better representation can 
  be found. A better representation will be completely constructed and 
  returned in "representation".  It works pretty similar to testcanon except 
  for obviously necessary changes, so for extensive comments see testcanon */
{
	register EDGE *run;
	register int vertex;
	EDGE *temp;  
	EDGE *startedge[MAXN+1]; 
	int number[MAXN], i; 
	int last_number, actual_number;

	for (i = 1; i <= nv; i++) number[i] = 0;

	number[givenedge->start] = 1; 
	if (givenedge->start != givenedge->end)
	  {
	    number[givenedge->end] = 2;
	    actual_number = 1;
	    last_number = 2;
	    startedge[1] = givenedge->invers;
	  }
	else
	  { last_number = actual_number = 1; }

	temp = givenedge;

	while (last_number < nv)
	{  
	    for (run = temp->next; run != temp; run = run->next)
	      { vertex = run->end;
		if (!number[vertex]) { startedge[last_number] = run->invers;
		                 last_number++; number[vertex] = last_number; }
		*representation = colour[vertex]; representation++;
		*representation = number[vertex]; representation++;
	      }
	    *representation = 0;
	    representation++;
	    temp = startedge[actual_number];  actual_number++;
	}

	while (actual_number <= nv) 
	{  
	    for (run = temp->next; run != temp; run = run->next)
	      { vertex = run->end; 
		*representation = colour[vertex]; representation++;
		*representation = number[vertex]; representation++;
	      }
	    *representation = 0;
	    representation++;
	    temp = startedge[actual_number];  actual_number++;
	}

	return;
}

/****************************************************************************/

static int 
testcanon_init(EDGE *givenedge, int representation[], int colour[])

/* Tests whether starting from a given edge and constructing the code in
  "->next" direction, an automorphism or even a better representation can 
  be found. A better representation will be completely constructed and 
  returned in "representation".  It works pretty similar to testcanon except 
  for obviously necessary changes, so for extensive comments see testcanon */
{
	register EDGE *run;
	register int col, vertex;
	EDGE *temp;  
	EDGE *startedge[MAXN+1]; 
	int number[MAXN], i; 
	int better = 0; /* is the representation already better ? */
	int last_number, actual_number;


	for (i = 1; i <= nv; i++) number[i] = 0;

	number[givenedge->start] = 1; 
	if (givenedge->start != givenedge->end)
	  {
	    number[givenedge->end] = 2;
	    actual_number = 1;
	    last_number = 2;
	    startedge[1] = givenedge->invers;
	  }
	else
	  { last_number = actual_number = 1; }

	temp = givenedge;

	while (last_number < nv)
	{  
  	    for (run = temp->next; run != temp; run = run->next)
	      { vertex = run->end;
	  	col = colour[vertex];
	  	if (!number[vertex]) { startedge[last_number] = run->invers;
				 last_number++; number[vertex] = last_number; }
	  	if (better)
	    	{ *representation = col; representation++;
	      	  *representation = number[vertex]; representation++; }
	  	else
	    	{
	      	    if (col > (*representation)) return(0);
	            if (col < (*representation)) 
		    { better = 1; *representation = col;
			     	   representation++; 
			          *representation = number[vertex]; 
				   representation++; }
	            else
		    {
		        representation++;
		        vertex = number[vertex];
		        if (vertex > (*representation)) return(0);
		        if (vertex < (*representation)) { better = 1;  
						   *representation = vertex; }
		        representation++;
		    }
	        }
	      }
  	    if ((*representation) != 0) { better = 1; *representation = 0; }
  	    representation++;
  	    temp = startedge[actual_number];  actual_number++;
	}

	while (actual_number <= nv) 
	{  
	    for (run = temp->next; run != temp; run = run->next)
	      { vertex = run->end; 
	  	col = colour[vertex];
	  	if (better)
	    	{ *representation = col; representation++;
	    	  *representation = number[vertex]; representation++; }
	        else
	        {
	            if (col > (*representation)) return(0);
	            if (col < (*representation)) 
		    { better = 1; *representation = col;
				   representation++; 
			          *representation = number[vertex]; 
				   representation++; }
	      	    else
		    {
		  	representation++;
		  	vertex = number[vertex];
		  	if (vertex > (*representation)) return(0);
		  	if (vertex < (*representation)) { better = 1;  
						   *representation = vertex; }
		  	representation++;
		    }
	        }
	      }
  	    if ((*representation) != 0) { better = 1; *representation = 0; }
  	    representation++;
  	    temp = startedge[actual_number];  actual_number++;
	}

	if (better) return(2);
	return(1);
}

/****************************************************************************/

static int 
testcanon_mirror_init(EDGE *givenedge, int representation[], int colour[])

/* Tests whether starting from a given edge and constructing the code in
  "->prev" direction, an automorphism or even a better representation can 
  be found. A better representation will be completely constructed and 
  returned in "representation".  It works pretty similar to testcanon except 
  for obviously necessary changes, so for extensive comments see testcanon */
{
	EDGE *temp, *run;  
	EDGE *startedge[MAXN+1]; 
	int number[MAXN], i; 
	int better = 0; /* is the representation already better ? */
	int last_number, actual_number, vertex, col;


	for (i = 1; i <= nv; i++) number[i] = 0;

	number[givenedge->start] = 1;
	if (givenedge->start != givenedge->end)
	  {
	    number[givenedge->end] = 2;
	    actual_number = 1;
	    last_number = 2;
	    startedge[1] = givenedge->invers;
	  }
	else
	  { last_number = actual_number = 1; }

	temp = givenedge;

	while (last_number < nv)
	{  
	    for (run = temp->prev; run != temp; run = run->prev)
	      { vertex = run->end;
	  	col = colour[vertex];
	  	if (!number[vertex]) { startedge[last_number] = run->invers;
				 last_number++; number[vertex] = last_number; }
	  	if (better)
	        { *representation = col; representation++;
	          *representation = number[vertex]; representation++; }
	  	else
	        {
	      	    if (col > (*representation)) return(0);
	      	    if (col < (*representation)) 
		    { better = 1; *representation = col;
				   representation++; 
				  *representation = number[vertex]; 
				   representation++; }
	            else
		    {
		        representation++;
		  	vertex = number[vertex];
		  	if (vertex > (*representation)) return(0);
		  	if (vertex < (*representation)) { better = 1;  
						   *representation = vertex; }
		  	representation++;
		    }
	        }
	      }
  	    if ((*representation) != 0) { better = 1; *representation = 0; }
  	    representation++;
  	    temp = startedge[actual_number];  actual_number++;
	}

	while (actual_number <= nv) 
	{  
  	    for (run = temp->prev; run != temp; run = run->prev)
	      { vertex = run->end; 
	  	col = colour[vertex];
	  	if (better)
	    	{ *representation = col; representation++;
	          *representation = number[vertex]; representation++; }
	        else
	        {
	            if (col > (*representation)) return(0);
	            if (col < (*representation)) 
		    { better = 1; *representation = col;
				   representation++; 
				  *representation = number[vertex]; 
				   representation++; }
	      	    else
		    {
		        representation++;
		        vertex = number[vertex];
		        if (vertex > (*representation)) return(0);
		        if (vertex < (*representation)) { better = 1;  
						   *representation = vertex; }
		        representation++;
		    }
	        }
	      }
	   if ((*representation) != 0) { better = 1; *representation = 0; }
	   representation++;
	   temp = startedge[actual_number];  actual_number++;
	}

	if (better) return(2);
	return(1);
}

/****************************************************************************/

static void
construct_canform(EDGE *givenedge, short canform[])

/* Starts at givenedge and writes the canonical form
  into the list.  Works like testcanon. Look there for comments. */
{



	EDGE *temp, *run;  
	short *limit;
	EDGE *startedge[MAXN+1]; /* startedge[i] is the starting edge for 
 				 exploring the vertex with the number i+1 */
	int number[MAXN], i; /* The new numbers of the vertices, starting 
				at 1 in order to have "0" as a possibility to
   				mark ends of lists and not yet given numbers */
	int last_number, actual_number, vertex;


	for (i = 1; i <= nv; i++) number[i] = 0;

	limit = canform+nv+ne;  

	number[givenedge->start] = 1;
	if (givenedge->start != givenedge->end)
	  { number[givenedge->end] = 2;
	    actual_number = 1;
	    last_number = 2;
	    startedge[1] = givenedge->invers;
	  }
	else
	  { last_number = actual_number = 1; }

	temp = givenedge;

	*canform=number[givenedge->end]; canform++;

	while (last_number < nv)
	{  
  	    for (run = temp->next; run != temp; run = run->next)
	/* this loop marks all edges around temp->origin. */
	      { vertex = run->end;
	  	if (!number[vertex]) { startedge[last_number] = run->invers;
				 last_number++; number[vertex] = last_number; }
		*canform=number[vertex]; canform++;
	      }
	    *canform=0; canform++;
  	    if (canform != limit)
    	    { temp = startedge[actual_number];  actual_number++; 
	    *canform=number[temp->end]; canform++;
	    }
	}

	while (canform != limit) 
			/* Now we know that all numbers have been given */
	{  
  	    for (run = temp->next; run != temp; run = run->next)
	/* this loop marks all edges around temp->origin. */
	      { *canform=number[run->end]; canform++; }
	    *canform=0; canform++;
  	    if ( canform != limit)
    	    { 
      /* Next vertex to explore: */
	       temp = startedge[actual_number];  actual_number++; 
	       *canform=number[temp->end]; canform++; }
	}

	return;
}

/****************************************************************************/

static void 
construct_canform_mirror(EDGE *givenedge, short canform[])

/* Starts at givenedge and writes the canonical form
  into the list.  Works like testcanon. Look there for comments. */
{



	EDGE *temp, *run;  
	short *limit;
	EDGE *startedge[MAXN+1]; /* startedge[i] is the starting edge for 
 				 exploring the vertex with the number i+1 */
	int number[MAXN], i; /* The new numbers of the vertices, starting 
				at 1 in order to have "0" as a possibility to
   				mark ends of lists and not yet given numbers */
	int last_number, actual_number, vertex;

	for (i = 1; i <= nv; i++) number[i] = 0;


	limit = canform+nv+ne;  

	number[givenedge->start] = 1;
	if (givenedge->start != givenedge->end)
	  { number[givenedge->end] = 2;
	    actual_number = 1;
	    last_number = 2;
	    startedge[1] = givenedge->invers;
	  }
	else
	  { last_number = actual_number = 1; }

	temp = givenedge;

	*canform=number[givenedge->end]; canform++;

	while (last_number < nv)
	{  
  	    for (run = temp->prev; run != temp; run = run->prev)
	/* this loop marks all edges around temp->origin. */
	      { vertex = run->end;
	  	if (!number[vertex]) { startedge[last_number] = run->invers;
				 last_number++; number[vertex] = last_number; }
		*canform=number[vertex]; canform++;
	      }
	    *canform=0; canform++;
  	    if (canform != limit)
    	    { temp = startedge[actual_number];  actual_number++; 
	    *canform=number[temp->end]; canform++;
	    }
	}

	while (canform != limit) 
			/* Now we know that all numbers have been given */
	{  
  	    for (run = temp->prev; run != temp; run = run->prev)
	/* this loop marks all edges around temp->origin. */
	      { *canform=number[run->end]; canform++; }
	    *canform=0; canform++;
  	    if ( canform != limit)
    	    { 
      /* Next vertex to explore: */
	       temp = startedge[actual_number];  actual_number++; 
	       *canform=number[temp->end]; canform++; }
	}

	return;
}


/****************************************************************************/

static int 
canon_form(int colour[], EDGE *can_numberings[][MAXE], 
	   int *num_can_numberings, int *num_can_numberings_or_pres,
	   short can_form[])

/* Computes the canonical form of the global firstedge[] map 
  and writes it into can_form.
  One of the criterions a canonical starting vertex must fulfill, 
  is that its colour is minimal. 
  A possible starting edge for the construction of a representation is 
  one with lexicographically minimal colour pair (start,INT_MAX-end).

  In can_numberings[][] the canonical numberings are stored as sequences 
  of oriented edges.  For every 0 <= i,j < *num_can_numberings and every 
  0 <= k < ne the edges can_numberings[i][k] and can_numberings[j][k] can 
  be mapped onto each other by an automorphism. The first 
  *num_can_numberings_or_pres numberings are orientation preserving while 
  the rest is orientation reversing.

  In case can_numberings==NULL, this part is omitted.

  Works OK if at least one vertex has valence >= 3. Otherwise some numberings 
  are computed twice, since changing the orientation (the cyclic order around 
  each vertex) doesn't change anything 

  Does __NOT__ work for maps with loops, except in some special
  cases (e.g.triangulations), but works for maps with double edges.

*/

{ 
	int i, j, last_vertex, test;
	int minstart, maxend; /* (minstart,maxend) will be the chosen colour 
				 pair of an edge */
	EDGE *startlist[5*MAXN], *run, *end;
	int list_length;
	int representation[2*MAXE+MAXN];
	EDGE *numblist[MAXE], *numblist_mirror[MAXE]; /* lists of edges where 
						starting gives a canonical representation */
	int numbs = 1, numbs_mirror = 0;



	last_vertex = nv;
	minstart = colour[1];
	for (i=2; i<= nv; i++) if (colour[i]<minstart) minstart=colour[i];

	list_length=0;
	maxend=0;
	for (i=1; i<= nv; i++) 
	  { 
	  if (colour[i]==minstart) 
	    { run=firstedge[i];
	      for (j=0; j<degree[i]; j++, run=run->next) 
		if (colour[run->end]>maxend)
		  { list_length=1; startlist[0]=run; maxend=colour[run->end]; }
	        else if (colour[run->end]==maxend)
		  { startlist[list_length]=run; list_length++; }
	    }
	  }


/* OK -- now we have all the pairs and have to determine the smallest representation */

	testcanon_first_init(startlist[0], representation, colour);
	numblist[0] = startlist[0];
	if (!without_mirror) 
	  { test = testcanon_mirror_init(startlist[0], representation, colour);
	    if (test == 1) 
	      { numbs_mirror = 1; numblist_mirror[0] = startlist[0]; }
	    else if (test == 2)  
	      { numbs_mirror = 1; numbs = 0; 
		numblist_mirror[0] = startlist[0]; };
	  }

	for (i = 1; i < list_length; i++)
	  { 
	    test = testcanon_init(startlist[i], representation, colour);
   	    if (test == 1) { numblist[numbs] = startlist[i]; numbs++; }
   	    else if (test == 2) 
	    { numbs_mirror = 0; numbs = 1; numblist[0] = startlist[i]; };

	    if (!without_mirror) 
	      {
		test = testcanon_mirror_init(startlist[i], 
					     representation, colour);
		if (test == 1)  
		  { numblist_mirror[numbs_mirror] = startlist[i]; 
		    numbs_mirror++; }
		else if (test == 2) 
		  { numbs_mirror = 1; numbs = 0; 
		    numblist_mirror[0] = startlist[i]; };
	      }
 	  }

	*num_can_numberings_or_pres = numbs;
	*num_can_numberings = numbs+numbs_mirror;

	/*
	if (can_numberings != NULL)
 	{ for (i = 0; i < numbs; i++) 
	      construct_numb(numblist[i], can_numberings[i]); 
	for (i = 0 ; i < numbs_mirror; i++) 
	   construct_numb_mirror(numblist_mirror[i], 
				     can_numberings[numbs+i]);
				     }*/


	can_form[0]=nv;
	if (numbs) construct_canform(numblist[0], can_form+1);
	else construct_canform_mirror(numblist_mirror[0], can_form+1);


	return(1);
}



/**********************CHECKSIZE_LEFT**************************************/

/* bestimmt die groesse der flaeche links von edge -- ist da keine gibt's Probleme */ 

int checksize_left(EDGE* edge)

{
EDGE *run; 
int zaehler=1;

for (run=edge->invers->next; run != edge; run=run->invers->next) zaehler++;

return(zaehler);
}



/**********************CHECKSIZE_RIGHT**************************************/

/* bestimmt die groesse der flaeche rechts von edge -- sonst wie oben */

int checksize_right(EDGE* edge)

{
EDGE *run; 
int zaehler=1;

for (run=edge->invers->prev; run != edge; run=run->invers->prev) zaehler++;

return(zaehler);
}



/*********************BELEGE_FLAECHENGROESSEN****************************/

void belege_flaechengroessen(EDGE *map[],int adj[],int *max)
{
int i,j,k;
EDGE *run;
int facesize;

*max=0;

for (i=1; i<=nv; i++)
 { run=map[i]; 
   for (j=0; j<adj[i]; j++, run=run->next) 
     { run->faceleft=run->faceright=run->dummy=0; }
 }

for (i=1; i<=nv; i++)
 { run=map[i];
   for (k=0; k<adj[i]; k++, run=run->next)
     { 
	if (run->faceleft==0)
	  { facesize=checksize_left(run); 
	    if (facesize > *max) *max=facesize;
	    for (j=0; j<facesize; j++, run=run->invers->next)
	      { run->faceleft=run->invers->faceright=facesize; }
	  }
	if (run->faceright==0)
	  { facesize=checksize_right(run);
	    if (facesize > *max) *max=facesize;
	    for (j=0; j<facesize; j++, run=run->invers->prev)
	      { run->faceright=run->invers->faceleft=facesize; }
	  }
     }
 }
}

int compare_degs(const void *a, const void *b)
{ 
 const unsigned short *aa, *bb;

 aa=a; bb=b;

return (int)(*aa)-(int)(*bb); 
}

/*******************INVARIANT_CODE***************************/

int invariant_code(unsigned short code[], int codelength)

/* computes an invariant of the map that only depends on the code
  -- just something stupid depending on the degrees */

{
 int nbs[MAXE+1][2],a,b,pos;
 EDGE *dummy, *inverse;
 int i,j, buffer, nse,nv, inv;
 unsigned short degs[MAXE];

 RESETMARKS_EL

   //code_edge=NULL;

  for (i=nse=degree[1]=0, nv=1;i<codelength;i++)
    { buffer=code[i];
    if (buffer==255)
      {nv++; degree[nv]=0;}
    else /* edge */
      { 
	 
	 if (ISMARKED_EL(buffer))
	   { nbs[buffer][1]=nv; }
	 else
	   {  MARK_EL(buffer); nbs[buffer][0]=nv; nse++;}
	 (degree[nv])++;
      }
    }

 /*
for (i=pos=0; i<nse; i++)
 {
   a=degree[nbs[i][0]]; b=degree[nbs[i][1]];

   if (a<b)
     { degs[pos]=a<<8 +b; pos++; }
   else 
     { degs[pos]=b<<8 +a; pos++; }
 }

 qsort((void *)degs, pos,sizeof(unsigned short), compare_degs);

 inv= hashlittle( degs, 2*pos, 1) & (uint32_t)INT_MAX; */

for (i=inv=0; i<nse; i++)
 {
   a=degree[nbs[i][0]]; b=degree[nbs[i][1]];

   if (a<b)
     { inv+= ((a & 71) + ((b<<1)&111)); }
   else 
     { inv+= ((b & 71) + ((a<<1)&111)); }
 }


return(inv);
}



/*******************INVARIANT***************************/

int invariant()

/* computes an invariant of the map -- just something stupid depending
  on the degrees and face sizes */

{

int maxface,i,j, test, pos,a,b;
int inv=0;
EDGE *run, *end;
unsigned short degs[MAXE];


RESETMARKS;


belege_flaechengroessen(firstedge,degree,&maxface);

for (i=1; i<=nv; i++)
 {
   run=firstedge[i];
   for (j=0; j<degree[i]; j++, run=run->next)
     if (!ISMARKED(run))
     { test=((degree[run->start]*degree[run->end])+(run->faceright*run->faceleft))%71;
	if ((INT_MAX-test) < inv) { fprintf(stderr,"inv gets too large\n"); exit(0); }
	inv += test; 
	MARKLO(run); MARKLO(run->invers);
     }
 }

/*

pos=0;
for (i=1; i<=nv; i++)
 {
   run=end=firstedge[i];
   do
     {       
	if (!ISMARKED(run))
	  { MARKLO(run->invers);
	    a=degree[run->start]; b=degree[run->end];
	    if (a<b)
	      { degs[pos]=a<<8 +b; pos++; }
	    else 
	      { degs[pos]=b<<8 +a; pos++; }
	  }
	run=run->next;
     } while (run!=end);
 }


qsort((void *)degs, pos,sizeof(unsigned short), compare_degs);

inv= hashlittle( degs, ne, 1);
*/
return(inv);
}

/******************CHECK_CODE***************************/

int check_code(unsigned short code[],unsigned short neuer_code[])
    /* gibt 1 zurueck, wenn der Code erfuellt, dass jeder Knoten ausser
	1 einen kleineren Nachbarn hat, (dann ist neuer_code undefiniert.
	Sonst wird 0 zurueckgegeben und neuer_code enthaelt einen in Breitensuche
       konstruierten code */
{ int i,j,run,knotenzahl,OK,naechste_nummer;
int bild[N+1], urbild[N+1], start[N+1]; /* wo startet die Liste von i */

knotenzahl=code[0];


for (i=2, OK=1, run=1; (i<=knotenzahl) && OK; i++)
  { OK=0;
    for ( ; code[run]!=0; run++); /* sucht naechste 0 -- d.h. Anfang naechste Liste */
    run++;
    for ( ; !OK && (code[run]!=0); run++) if (code[run]<i) OK=1;
  }
if (OK) return 1;


start[1]=1;
for (i=2, run=1; i<=knotenzahl ; i++)
  { bild[i]=0;
    for ( ; code[run]!=0; run++); /* sucht naechste 0 -- d.h. Anfang naechste Liste */
    run++;
    start[i]=run; 
  }

bild[1]=urbild[1]=1; naechste_nummer=2;

for (i=1; naechste_nummer<=knotenzahl; i++)
  { 
  run=start[urbild[i]]; 
    for (; code[run]!=0; run++) 
      { if (bild[code[run]]==0)
	 { bild[code[run]]=naechste_nummer; urbild[naechste_nummer]=code[run];
	 naechste_nummer++; }
      }
  }

neuer_code[0]=code[0];
for (i=1,run=1; i<=knotenzahl; i++)
  { j=start[urbild[i]];
  for ( ; code[j]!=0; j++, run++) neuer_code[run]=bild[code[j]];
  neuer_code[run]=0; run++;
  }

return 0;

}


int read_edgecode(FILE *fil, unsigned short code[], int *codelength)
{
 int length,dummy,i;
 unsigned char dummycode[MAXCODELENGTH];

 length=getc(fil);

 if (length==EOF) return 0;

 if (length==0)
   { length=getc(fil);
   if (length==EOF) { fprintf(stderr,"Error in code!\n"); exit(1); }
   if (length != (2<<4)+1) { fprintf(stderr,"Not prepared for such large codes!\n");
                             fprintf(stderr,"Exiting!\n"); exit(1); }
   length=getc(fil); 
   if (length==EOF) { fprintf(stderr,"Error in code!\n"); exit(1); }
   length=length<<8;
   dummy=getc(fil);
   if (dummy==EOF) { fprintf(stderr,"Error in code!\n"); exit(1); }
   length+=dummy;
   }

 *codelength=length;

 if (length>MAXCODELENGTH)
   { fprintf(stderr,"MAXCODELENGTH must be at least %d -- exiting.\n",length);
   exit(1);
   }

 if (fread(dummycode,1,length,fil)!= length)
   { fprintf(stderr,"Error in code -- code too short!\n"); exit(1); }

 for (i=0;i<length;i++) code[i]=dummycode[i];

 //fprintf(stderr,"Code: "); for (i=0;i<length;i++) fprintf(stderr," %d",code[i]);
 //fprintf(stderr,"\n");

 return 1;
}



/**************************LESECODE*******************************/

int lesecode(unsigned short code[], int *laenge, FILE *file, char *type)
/* gibt 1 zurueck, wenn ein code gelesen wurde und 0 sonst */
/* type is `p' for planarcode or `e' for edgecode */
{
static int first=1;
unsigned char ucharpuffer;
char testheader[20];
int too_large=0, lauf, nullenzaehler;

//if (fread(&ucharpuffer,sizeof(unsigned char),1,file)==0) return(0);


if (first)
  {first=0;
  *type=0;

  if (fread(&testheader,sizeof(unsigned char),13,file)!=13) 
    { fprintf(stderr,"can't read header ((1)file too small)-- exiting\n");
    exit(1); }
  testheader[13]=0;
  if (strcmp(testheader,">>edge_code<<")==0) *type ='e';
  else
    { 
    if (strcmp(testheader,">>planar_code")==0)
      { if (fread(&testheader,sizeof(unsigned char),2,file)!=2) 
	 { fprintf(stderr,"can't read header ((2)file too small)-- exiting\n");
	 exit(1); }
      testheader[2]=0;
      if (strcmp(testheader,"<<")==0) { *type='p'; }
      }

    }

  if (*type==0) 
    { 
      fprintf(stderr,"Neither edgecode nor planarcode header detected -- exiting!\n");
      exit(1);
    }
  }

if (*type=='p')
  { /* possibly removing interior headers -- only done for planarcode */
    if (fread(&ucharpuffer,sizeof(unsigned char),1,file)==0) return(0);
    if (ucharpuffer=='>') /* koennte ein header sein -- oder 'ne 62, also ausreichend fuer
			      unsigned char */
      { code[0]=ucharpuffer;
      lauf=1; nullenzaehler=0;
      code[1]=(unsigned short)getc(file);
      if(code[1]==0) nullenzaehler++; 
      code[2]=(unsigned short)getc(file);
      if(code[2]==0) nullenzaehler++; 
   lauf=3;
   /* jetzt wurden 3 Zeichen gelesen */
   if ((code[1]=='>') && (code[2]=='p')) /*garantiert header*/
     { while ((ucharpuffer=getc(file)) != '<');
     /* noch zweimal: */ ucharpuffer=getc(file); 
     if (ucharpuffer!='<') { fprintf(stderr,"Problems with header -- single '<'\n"); exit(1); }
     if (!fread(&ucharpuffer,sizeof(unsigned char),1,file)) return(0);
     /* kein graph drin */
     lauf=1; nullenzaehler=0; }
   /* else kein header */
      }
    else { lauf=1; nullenzaehler=0; }

    if (ucharpuffer!=0) /* kann noch in unsigned char codiert werden ... */
      { too_large=0;
      code[0]=ucharpuffer;
      if (code[0]>N) { fprintf(stderr,"Constant N too small %d > %d \n",code[0],N); exit(1); }
      while(nullenzaehler<code[0])
	 { code[lauf]=(unsigned short)getc(file);
	 if(code[lauf]==0) nullenzaehler++;
	 lauf++; }
      }
    else  { too_large=1;
    fread(code,sizeof(unsigned short),1,file);
    if (code[0]>N) { fprintf(stderr,"Constant N too small %d > %d \n",code[0],N); exit(1); }
    lauf=1; nullenzaehler=0;
    while(nullenzaehler<code[0])
      { fread(code+lauf,sizeof(unsigned short),1,file);
      if(code[lauf]==0) nullenzaehler++;
      lauf++; }
    }

    *laenge=lauf;
    return(1);
  }

/* OK -- here it arrives only in the case of type=='e' */
//if (type=='e')

return read_edgecode(file, code, laenge);

}




void decode_edgecode(unsigned short code[], int codelength)
{
 EDGE *firstoccurence[MAXE+1];
 EDGE *dummy, *inverse;
 int i,j, buffer;

 RESETMARKS_EL

   //code_edge=NULL;

  for (i=ne=degree[1]=0, nv=1;i<codelength;i++)
    { buffer=code[i];
    if (buffer==255)
      {nv++; degree[nv]=0;}
    else /* edge */
      { //if ((buffer==0) && (nv==1)) code_edge=map[1]+degree[1];
        if (buffer>MAXE) { fprintf(stderr,"MAXE too small %d<%d exiting!\n",MAXE,buffer);
	                    exit(1); }
	 ne++;
	 if (ISMARKED_EL(buffer))
	 { dummy=map[nv]+degree[nv];
	   dummy->start=nv;
	 //dummy->index=buffer;
	   (degree[nv])++;
	   if (degree[nv]>=MAXVAL) 
	     {fprintf(stderr,"not prepared for such large degrees -- exiting.\n");
	      exit(1); }
	   inverse=firstoccurence[buffer];
	   dummy->invers=inverse;
	   dummy->end=inverse->start;
	   inverse->end=nv;
	   inverse->invers=dummy;
	 }
      else
	 { firstoccurence[buffer]=map[nv]+degree[nv];
	 //map[nv][degree[nv]].index=buffer;
	   firstoccurence[buffer]->start=nv;
	   (degree[nv])++;
	   if (degree[nv]>=MAXN) 
	     {fprintf(stderr,"not prepared for such large degrees -- exiting.\n");
	      exit(1); }
	   MARK_EL(buffer);
	 }
      }
    }

  //  nv++;

 for (i=1;i<=nv;i++)
   {
     map[i][0].prev=map[i]+(degree[i]-1);
     map[i][degree[i]-1].next=map[i];
     if (degree[i]>1)
	{
	  map[i][0].next=map[i]+1;
	  map[i][degree[i]-1].prev=map[i]+degree[i]-2;
	  for (j=degree[i]-2;j>=1;j--)
	    { map[i][j].prev=map[i]+(j-1);
	      map[i][j].next=map[i]+(j+1);
	    }
	}

   }

 numbersubdivided=0;

for (i=1; i<=nv; i++) firstedge[i]=map[i];

map[0][0].end = nvbeforesubdiv= nv;
map[0][1].end=ne;
}

void subdivideonce(EDGE *run, int colour[])
{ EDGE *first, *second, *inv;
 if (nv>=MAXN-2) { fprintf(stderr,"Increase constant N\n"); exit(1); }

 //printf(stderr,"subdividing %d->%d\n",run->start,run->end);

 numbersubdivided++;

 if (degree[run->start]>=MAXVAL-1) 
   { fprintf(stderr,"Increase MAXVAL\n"); exit(1); }

 nv++;
 ne+=2;
 first=firstedge[nv]=map[nv];
 second=first+1;
 inv=run->invers;


 colour[nv]=SUBDIVISION;
 degree[nv]=2;
 first->start=second->start=nv;
 first->next=first->prev=second;
 second->next=second->prev=first;
 /* second to inv, first to run */

 second->invers=inv;
 inv->invers=second;
 second->end=inv->start;
 inv->end=nv;

 first->invers=run;
 run->invers=first;
 first->end=run->start;
 run->end=nv;

 map[0][0].end=nv;
 map[0][1].end=ne;
}


void subdividetwice(EDGE *run, int colour[])
{
 subdivideonce(run, colour);
 subdivideonce(run, colour);
}

/**********************************************************/

void  subdividedubbelandloop(int colour[])
{
 EDGE *firstoccurence[MAXN];
 int endtop, oldnv, i;
 EDGE *run, *end;

 oldnv=nv;

 for (i=1;i<=oldnv;i++)
   {
     RESETMARKS_V;
     run=end=map[i];
     do{ endtop=run->end;
     if (endtop==i) { subdividetwice(run,colour); }
	else
	  { if (ISMARKED_V(endtop))
	  {
	    if (ISMARKEDLO_V(endtop)) /* already seen but this is the first double */
	      { 
		MARKHI_V(endtop);
		subdivideonce(run,colour);
		subdivideonce(firstoccurence[endtop],colour);
	      }
	    else // ISMARKED_HI(endtop) the previous ones already marked
	      {
		subdivideonce(run,colour);
	      }
	  }
	    else /* not yet seen */
	  {
	    MARKLO_V(endtop);
	    firstoccurence[endtop]=run;
	  }
	  }
	run=run->next;
     } while (run!=end);
   }
 return;

}

/*******************MAIN********************************/

main(argc,argv)

int argc;
char *argv[];


{
GRAPH graph;
int colour[MAXN];
int zaehlen, zaehlen2, non_iso;
unsigned short code[MAXCODELENGTH];
unsigned short neuer_code[MAXCODELENGTH];
int lauf, nullenzaehler;
unsigned char ucharpuffer;
int too_large, i, test, globaltest=0;

char outputstring[300];
int first_call=1;
int auts, auts_or_pres;
short *can_form;
int return_total = 0;


fprintf(stderr,"Use for double edges and loops only with edgecode.\n");
fprintf(stderr,"Double edges and loops are represented by inserting coloured vertices.\n");
fprintf(stderr,"The colours do not occur when the graphs are outputted!\n");
fprintf(stderr,"Output always in planarcode.\n");


if (N>USHRT_MAX-2) { fprintf(stderr,"Constant N (%d) may be at most %d.\n",N,USHRT_MAX-2);
		     exit(0); }


non_iso=zaehlen=zaehlen2=0;

for (i=1; i<argc; i++)
 { if (strcmp(argv[i],"o")==0) {write_old=1; } else
   if (strcmp(argv[i],"p")==0) {print_ascii=1; } else
   if (strcmp(argv[i],"ps")==0) {print_ascii=print_ascii_short=1; } else
if (strcmp(argv[i],"md5")==0) {print_ascii=md5out=1; } else
     if (strcmp(argv[i],"n")==0) { write_new=1; } else
	if (strcmp(argv[i],"f")==0) { output=1; } else
	  if (strcmp(argv[i],"wo")==0) { writemap_old=1; } else
	    if (strcmp(argv[i],"wn")==0) { writemap_new=1; } else
	      //if (strcmp(argv[i],"wr")==0) { write_roots=1; } else
	      if (strcmp(argv[i],"or")==0) { without_mirror=1; } else
		//if (strcmp(argv[i],"rv")==0) { do_rooted=1; do_rooted_v=1;} else
		//if (strcmp(argv[i],"re")==0) { do_rooted=1; do_rooted_e=1;} else
		//if (strcmp(argv[i],"rfl")==0) { do_rooted=1; do_rooted_fl=1;} else
		//if (strcmp(argv[i],"rf")==0) { do_rooted=1; do_rooted_f=1;} else
		//if (strcmp(argv[i],"ro")==0) { only_roots=1; } else
		  if (strcmp(argv[i],"hash")==0) { use_hash=1; } else
		  if (strcmp(argv[i],"t")==0) { return_total=1; } else
		    if (strcmp(argv[i],"mod")==0) { i++; rest=atoi(argv[i]);
                                                   i++; mod=atoi(argv[i]); } else

	      if (strcmp(argv[i],"u")==0) 
		{ fprintf(stderr,"At least one option of \n o (write number of old)\n");
               fprintf(stderr,"n (write number of new)\n");
		fprintf(stderr,"f (filter non-isomorphic ones to stdout)\n");
		fprintf(stderr,"p (do not look for isomorphs -- just compute canonical form and write it to stdout (\"ps\": a short ascii presentation for up to 61 vertices)\n");
fprintf(stderr,"in ascii format)\n");
fprintf(stderr,"md5 (print an ascii md5 representation to stdout)\n");
fprintf(stderr,"hash (use a hash value instead of the real canonical form)\n");
fprintf(stderr,"In case hash is used no output of the graph is possible.\n");
		fprintf(stderr,"wo wn (write the map if old/new)\n");
		fprintf(stderr,"or (don't regard orientation reversing automorphisms)\n");
		//fprintf(stderr,"rv, re, rfl, rf (count the rooted nonisomorphic structures)\n");
		//fprintf(stderr,"(rooted at vertices, edges, flags, faces)\n");
		//fprintf(stderr,"wr (write the number of roots for every new map)\n");
		//fprintf(stderr,"ro (roots only -- do not compute isomorphisms)\n");
		  fprintf(stderr,"mod x y (test only part x (0<=x<y) of y parts in all)\n");
		  fprintf(stderr,"The part is selected by using a hash function of the canonical form,\n");
		  fprintf(stderr,"so all isomorphic graphs are in the same part. The canonical\n");
		  fprintf(stderr,"form is computed for all graphs -- just the storage consumption is reduced.\n");
		fprintf(stderr,"must be given\n");
		exit(0); } else
		  { fprintf(stderr,"unidentified option %s  -- try option u.\n",argv[i]); exit(0); }
 }

if (use_hash) 
  { hashlist=calloc((1<<HASHFIELDEXPONENT),sizeof(HASHFIELD));
    if (hashlist==NULL) 
      { fprintf(stderr,"don't get memory for hashfield\n"); exit(1); }

  }

if (mod) { fprintf(stderr,"Note: modulo saves memory -- but time is a bad.\n");
fprintf(stderr,"The sum of the runs takes MUCH more times than a single one! \n"); }

/*
if (do_rooted_v || do_rooted_f || do_rooted_e)
  { fprintf(stderr,"Rooted counts for edges vertices and faces not yet tested!\n");
    fprintf(stderr,"__DO__ test it against known numbers before using the results!\n"); }
*/

while (lesecode(code, &lauf, stdin, &codetype))
  { zaehlen++; 
  if ((mod==0) || (codetype!='e') || ((invariant_code(code, lauf)%mod)==rest))
 { 


   //fprintf(stderr,"number %d\n",zaehlen);

   if (codetype=='p')
     {
	if (test=check_code(code,neuer_code)) decodiereplanar(code,map,degree);
	else { decodiereplanar(neuer_code,map,degree); globaltest++; }
	for (i=1;i<=nv;i++) colour[i]=degree[i];
     }
   else // type=='e'
     {
	decode_edgecode(code, lauf);
	for (i=1;i<=nv;i++) colour[i]=degree[i];
	//writemap();
	subdividedubbelandloop(colour);
	/*	if (zaehlen==2 || zaehlen==5) 
	  	  { int i; 
fprintf(stderr,"colours: ");for (i=1;i<=nv;i++) fprintf(stderr," %d",colour[i]); fprintf(stderr,"\n");
writemap(); }*/
     }




   if ((mod==0) || (codetype=='e') || ((invariant()%mod)==rest))
     { zaehlen2++;
     if (neu(nv+ne+1,zaehlen,colour)) non_iso++;
     }

 }
  }



if (output && !use_hash) splay_scan(worklist);

if (globaltest!=0)
fprintf(stderr,"WARNING !! %d graphs had to be renumbered, since there was a vertex\n different from 1 not adjacent to a smaller one ! \n",globaltest);



if (only_roots || print_ascii) fprintf(stderr,"Read %d graphs.\n", zaehlen);
else fprintf(stderr,"Read %d graphs, %d non-isomorphic.\n", zaehlen,non_iso);
if (mod) fprintf(stderr,"Selected %d graphs due to an invariant.\n",zaehlen2);


if (do_rooted_f) fprintf(stderr,"%llu face rooted graphs.\n",numrootedf);
if (do_rooted_e) fprintf(stderr,"%llu edge rooted graphs.\n",numrootede);
if (do_rooted_fl) fprintf(stderr,"%llu flag rooted graphs.\n",numrootedfl);
if (do_rooted_v) fprintf(stderr,"%llu vertex rooted graphs.\n",numrootedv);

if(return_total)
	return non_iso;
else
	return(0);

}


