/*****************************************************************************
*    GCONV - CONVERTION OF GRAPH CODES                                       *
*    original version   31.8.1995                                            *
*    this version 2.12  22.6.1998                                            *
*    AUTHOR: Thomas Harmuth                                                  *
*    Email:  harmuth@mathematik.uni-bielefeld.de                             *
*    See gconv.man for description of the program                            *
*    This program uses nauty.h                                               *
*    AUTHOR: Brendan D. McKay                                                *
*****************************************************************************/

/****************************************************************************/
/*   CHANGES                                                                */
/*   22.6.1998:  - new code "embed_code"                                    */
/*   30.3.1998:  - added "sun" workstations into big-endian list            */
/*   22.11.1997: - removed error in "write_planar_code_s"  (graphs with     */
/*                 more than 255 vertices were affected)                    */
/*   10.6.1997:  - removed errors in "write_writegraph2d" and "writegraph"  */
/*   16.10.1996: - set default output code to input code                    */ 
/*               - unique exit codes                                        */
/*   7.10.1996:  - machine dependent definition of ENDIAN_OUT and ENDIAN_IN */
/*   29.8.1996:  - additional CONECT-entries for code "BrookhavenPDB"       */
/*               - error:  sometimes the "inputgraph" array has a leading   */
/*                 zero if the vertex number is big, sometimes it has not.  */
/*                 The program now takes care of both possibilities.        */
/*   30.7.1996:  - new code "BrookhavenPDB" (Protein Data Bank)             */
/*   29.7.1996:  - new parameter "planar" in "writegraph2d" and             */
/*                 "writegraph3d" codes                                     */
/*               - additional conditions "C" and "D" for convertion         */
/*               - new in "writegraph" code:  graph number                  */
/*   27.6.1996:  - support of "<",">" and ">>" directives dropped           */
/*               - internal types KNOTENTYP and FLAECHENTYP substituted     */
/*                 by unsigned short and unsigned char                      */ 
/*               - additional conditions for "R" and "T" for convertion     */
/*               - options -x and +x dropped                                */
/****************************************************************************/

#include<stdlib.h>
#include<string.h>
#include<stdio.h>
#include<ctype.h>
#include<limits.h>       /* USHRT_MAX */
#include "nauty.h"

/* internal definitions: */
#ifndef BIG_ENDIAN
#define BIG_ENDIAN 0
#endif
#ifndef LITTLE_ENDIAN
#define LITTLE_ENDIAN 1
#endif

#ifdef __sgi
#define ENDIAN_OUT BIG_ENDIAN
#define ENDIAN_IN  BIG_ENDIAN
#else
#ifdef sun
#define ENDIAN_OUT BIG_ENDIAN
#define ENDIAN_IN BIG_ENDIAN
#else
 /* add ifdef-calls for other BIG_ENDIAN processors */ 
#define ENDIAN_OUT  LITTLE_ENDIAN     /* Output for 2-Byte-Numbers */
#define ENDIAN_IN   LITTLE_ENDIAN     /* default input for 2-byte-numbers */
#endif
#endif

#define INFINITY INT_MAX

#ifdef MAXN
#undef MAXN
#define MAXN     (INFINITY-3)
#endif

#define False    FALSE
#define True     TRUE
#define nil      0
#define MAXR    20      /* length of range-memory */
#define NL      "\n"    /* newline-symbol */
#define LINELEN 2000    /* maximum linelength for combinatorica-codes */

/* Type-Declarations: */

typedef char BOOL;   /* !=0 => True (use only values 0 and 1) */

struct addinfo {int n;              /* number of vertices (-1 = dunno) */
               int reg;            /* regularity: -1 = don't know,    */
				    /* -2 = definitely no regularity   */
		char simple;};      /* simple without loops            */   
                                   /* -1 = dunno, 0 = no, 1 = yes     */

struct edge {unsigned short v1;
            unsigned short v2;};  /* the two adjacent vertices of an edge */ 

/* Global Variables and defines: */

char *codename[] = {"writegraph","planar_code_old","reg_code_old",
		    "multi_code_old","planar_code","reg_code","multi_code",
		    "planar_code_s","reg_code_s","multi_code_s",
		    "planar_code_s_old","reg_code_s_old","multi_code_s_old",
		    "graph6","graph6_old","digraph6","digraph6_old",
		    "writegraph2d_old","writegraph3d_old",
		    "writegraph2d","writegraph3d","multi_code2_s_old",
                   "BrookhavenPDB","embed_code",""};

BOOL codebin[] = {FALSE,TRUE,TRUE,TRUE,TRUE,TRUE,TRUE,TRUE,TRUE,TRUE,TRUE,
		  TRUE,TRUE,FALSE,FALSE,FALSE,FALSE,FALSE,FALSE,FALSE,FALSE,
                 TRUE,FALSE,TRUE};
                 /* FALSE = ASCII-Code, TRUE = binary code */

/* this defines the order of the entries of "codename": */
#define WRITEGRAPH         1
#define PLANAR_CODE_OLD    2
#define REG_CODE_OLD       3
#define MULTI_CODE_OLD     4
#define PLANAR_CODE        5
#define REG_CODE           6
#define MULTI_CODE         7
#define PLANAR_CODE_S      8
#define REG_CODE_S         9
#define MULTI_CODE_S      10
#define PLANAR_CODE_S_OLD 11
#define REG_CODE_S_OLD    12
#define MULTI_CODE_S_OLD  13
#define GRAPH6            14
#define GRAPH6_OLD        15
#define DIGRAPH6          16
#define DIGRAPH6_OLD      17
#define WRITEGRAPH2D_OLD  18
#define WRITEGRAPH3D_OLD  19
#define WRITEGRAPH2D      20
#define WRITEGRAPH3D      21
#define MULTI_CODE2_S_OLD 22
#define BROOKHAVEN_PDB    23
#define EMBED_CODE        24

/* can_convert[i-1][j-1] determines, if a convertion from j to i is possible
  (Yes/No/under several Conditions) */
/* Conditions:
  P = graph must be planar and the code must be allowed to be interpreted as
      a planar embedding of the graph (like in "planar_code")
  R = graph must be regular (with the regularity of the first converted graph)
      and all graphs must have the same vertex number
  S = graph must be simple without loops (where most codes handle loops
      correctly even if they are officially not allowed)
  T = graph must be regular (like above) and simple without loops, and all
      graphs must have the same vertex number
  C = The edges (1,2),...,(1,reg+1) must be in the graph (where "reg" is the
      desired regularity of the graph)
  D = R and C must hold
  E = T and C must hold */
/* If it is clear that a certain condition holds, then the relative entry
  can be "Y" instead of "P", "R", "S" or "T" */

char *can_convert[] =
 {"NYYYYYYYYYYYYYYYYYYYYYNY",
  "NYNNYNNYNNYNNNNNNPPPPNNS",
  "NDYDDCDDCDDYDDDEEEEEEDNE",
  "NYYYYYYYYYYYYYYSSSSSSYNS",
  "NYNNYNNYNNYNNNNNNPPPPNNS",
  "NRYRRYRRYRRYRRRTTTTTTRNE",
  "NYYYYYYYYYYYYYYSSSSSSYNS",
  "NYNNYNNYNNYNNNNNNPPPPNNN",
  "NRYRRYRRYRRYRRRTTTTTTRNE",
  "NYYYYYYYYYYYYYYSSSSSSYNS",
  "NYNNYNNYNNYNNNNNNPPPPNNN",
  "NDYDDCDDCDDYDDDEEEEEEDNE",
  "NYYYYYYYYYYYYYYSSSSSSYNS",
  "NYYYYYYYYYYYYYYSSSSSSYNS",
  "NYYYYYYYYYYYYYYSSSSSSYNS",
  "NYYYYYYYYYYYYYYYYYYYYYNS",
  "NYYYYYYYYYYYYYYYYYYYYYNS",
  "NYYYYYYYYYYYYYYYYYYYYYNY",
  "NYYYYYYYYYYYYYYYYYYYYYNY",
  "NYYYYYYYYYYYYYYYYYYYYYNY",
  "NYYYYYYYYYYYYYYYYYYYYYNY",
  "NYYYYYYYYYYYYYYSSSSSSYNS",
  "NNNNNNNNNNNNNNNNNYYYYNNN",
  "NYNNYNNYNNYNNNNNNPPPPNNY"}; 

/**********************GET_WORKSPACE***************************************/

graph *get_workspace(int maxn,int maxm) {
 graph *erg;
 if (!(erg=(graph *)ALLOCS((size_t)(maxn+1)*(size_t)maxm,sizeof(setword))))
    {fprintf(stderr,"Fatal error:  No workspace!\n");   exit(1);}
 return(erg);
}

/**********************GET_WORKSPACE2**************************************/

unsigned short *get_workspace2(size_t entries) {
 unsigned short *erg;
 if (!(erg=(unsigned short *)ALLOCS((size_t)entries,sizeof(unsigned short))))
    {fprintf(stderr,"Fatal error:  No workspace!\n");   exit(2);}
 return(erg);
}

/**********************GET_WORKSPACE3**************************************/

double *get_workspace3(long int entries) {
 double *erg;
 if (!(erg=(double *)ALLOCS((size_t)entries,sizeof(double))))
    {fprintf(stderr,"Fatal error:  No workspace!\n");   exit(3);}
 return(erg);
}

/**********************GET_WORKSPACE4**************************************/

struct edge *get_workspace4(size_t entries) {
 struct edge *erg;
 if (!(erg=(struct edge *)ALLOCS((size_t)entries,sizeof(struct edge))))
    {fprintf(stderr,"Fatal error:  No workspace!\n");   exit(4);}
 return(erg);
}

/**********************IS_IN***********************************************/
/*  Finds out whether the character "c" is in the string "s".             */

BOOL is_in(char c,char *s) {
 while (*s) {
   if (c == *s) {return(TRUE);}
   s++;
 }
 return(FALSE);
}

/**********************FIND_FORMAT_ID**************************************/

int find_format_ID(char *formatname) {
 int i=0;
 while (strcmp(codename[i],(char *)"")!=0 && 
        strcmp(codename[i],formatname)!=0) {i++;}
 return (strcmp(codename[i],(char *)"")==0 ? 0 : (i+1));
}

/***********************READ_HEADERFORMAT*********************************/
/*    Determines the format of file f by reading the headername          */

int read_headerformat(FILE *f) {
 unsigned char c[128];
 int i=-1;
 if (fread(&c[0],sizeof(unsigned char),2,f)<2) {return(-2);}

 if (strncmp((char *)&c[0],">>",2)!=0) {return(-4);}
 do {
   i++; if (i>127) {return(-3);}
   if (fread(&c[i],sizeof(unsigned char),1,f)==0) {return(-2);}
 } while (c[i]!=' ' && c[i]!='<');
 if (c[i]=='<') {
   if (ungetc(c[i],f)==EOF) {return(-2);}
 }
 c[i]=0;
 return(find_format_ID((char *)&c[0]));
}

/****************************READ_TO_END_OF_HEADER************************/

char read_to_end_of_header(FILE *f,unsigned char *c) {
 while (strncmp((char *)c,"<<",2)!=0) {
   c[0]=c[1];
   if (fread(&c[1],sizeof(unsigned char),1,f)==0) {return(2);}
 }
 return(1);
}

/***************************READ_HEADER_ENTRY*****************************/
/*  This function finds out whether the string "s" is the next entry in
   the header. If it is, it returns 1, if it is not, it returns 0, if
   If an error occurrs, it returns 2. 
   Whitespaces in the header are skipped.                               */

char read_header_entry(FILE *f,char *s) {
 unsigned int pos = 0;
 char c;
 while (s[pos]) {         /* not end of string */
   if (fread(&c,sizeof(char),1,f)==0) {return(2);}
   if (c!=' ') {          /* skip whitespace */
     if (s[pos++] != c) {         
       if (ungetc(c,f)==EOF) {return(2);} else {return(0);}
       /* last character must be read again because it might be a part
          of the header end "<<" */
     }
   }
 }
 return(1);
}

/*****************************EXTRACT_THE_HEADER**************************/
/*  writes header of file f into file h                                  */

char extract_the_header(FILE *f, FILE *h) {
 unsigned char c[2];
 if (fread(&c[0],sizeof(unsigned char),2,f)<2) {return(2);}
 if (strncmp((char *)&c[0],">>",2)!=0) {return(4);}
 fprintf(h,">>");
 while (strncmp((char *)&c[0],"<<",2)!=0) {
   c[0]=c[1];
   if (fread(&c[1],sizeof(unsigned char),1,f)==0) {return(2);}
   fprintf(h,"%c",c[1]);  if (ferror(f)) {return(2);}
 }
 return(1);
}

/****************************ADD_THE_HEADER****************************/
/*  adds header from file h to file f, eventually deleting the header */
/*	 of f.  Resulting file is written into d.                     */
/*  Condition:  length(f without header)>=2                           */

char add_the_header(FILE *f,FILE *d,FILE *h) {
 unsigned char c[2],c2[2];
 unsigned long i;
 /* check new header: */
 if (fread(&c2[0],sizeof(unsigned char),2,h)<2) {return(2);}
 if (strncmp((char *)&c2[0],">>",2)!=0) {return(4);}
 /* drop old header of f: */
 if (fread(&c[0],sizeof(unsigned char),2,f)<2) {return(2);}
 if (strncmp((char *)&c[0],">>",2)==0) {
   while (strncmp((char *)&c[0],"<<",2)!=0) {
     c[0]=c[1];
     if (fread(&c[1],sizeof(unsigned char),1,f)==0) {return(2);}
   }
   if (fread(&c[0],sizeof(unsigned char),2,f)<2) {return(2);}
 }
 /* copy new header h in d: */
 fprintf(d,">>");
 while (strncmp((char *)&c2[0],"<<",2) !=0) {
   c2[0]=c2[1];
   if (fread(&c2[1],sizeof(unsigned char),1,h)==0) {return(2);}
   fprintf(d,"%c",c2[1]);
 }
 if (ferror(d)) {return(2);}
 /* copy rest of f in d: */
 i = 2;
 fprintf(d,"%c%c",c[0],c[1]);
 while (fread(&c[0],sizeof(unsigned char),1,f)!=0) {
   i++;
   fprintf(d,"%c",c[0]);
 }
 fprintf(stderr,"Transferred %ld Bytes from original file.\n",i);
 return(ferror(d) ? 2 : 1);
}

/**********************WRITE_2BYTE_NUMBER***************************/
/*  This procedure takes care of the endian                        */

char write_2byte_number(FILE *f,unsigned short n,int endian) {
 if (endian==BIG_ENDIAN) {fprintf(f,"%c%c",n/256,n%256);}
 else                    {fprintf(f,"%c%c",n%256,n/256);}
 return(ferror(f) ? 2 : 1);
}

/***********************READ_2BYTE_NUMBER***************************/
/*  This procedure takes care of the endian                        */

char read_2byte_number(FILE *f,unsigned short *n,int endian) {
 unsigned char c[2];
 if (fread(&c[0],sizeof(unsigned char),2,f)<2) {return(2);}
 if (endian==BIG_ENDIAN) {*n = c[0]*256+c[1];}
 else                    {*n = c[1]*256+c[0];}
 return(1);
}

/************************READ_ENDIAN**************************************/
/*  Reads in the endian type (le/be), if existing, and skips to the end  */
/*  of the header                                                 	 */

char read_endian(FILE *f,int *endian) {
 unsigned char c[2];
 do {
   if (fread(&c[0],sizeof(unsigned char),1,f)==0) {return(2);}
 } while (isspace(c[0]));
 if (fread(&c[1],sizeof(unsigned char),1,f)==0) {return(2);}
 if (strncmp((char *)&c[0],"le",2)==0) {*endian = LITTLE_ENDIAN;}
 else if (strncmp((char *)&c[0],"be",2)==0) {*endian = BIG_ENDIAN;}
 else {*endian = ENDIAN_IN;}
 if (read_to_end_of_header(f,&c[0])==2) {return(2);}
 return(1);
}

/**********************READ_OLD_OR_NEW***********************************/
/*  This function decides for a short-code  if a number is to be read   */
/*  from the last graph or from the file                                */

char read_old_or_new(FILE *f,unsigned short *lastinput,unsigned short s,
 unsigned short *z,size_t maxentries,BOOL bignum,int endian,
 unsigned short *num) {
 unsigned char k;
 if (*z>=s) {    /* new number to be read from the file */
   if (bignum)
     {if (read_2byte_number(f,num,endian)==2) {return(2);} }
   else
     {if (fread(&k,sizeof(unsigned char),1,f)==0) {return(2);}
      *num = (unsigned short)k; }
   if (lastinput && *z<maxentries) {lastinput[*z]=*num; (*z)++;}
 }  /* if */
 else {*num=lastinput[*z]; (*z)++;}
 return(1);
}

/*************EMBED_2_PLANAR**************************************************/
/*  This conversion is useful if the code must be tested for simplicity
   (planar_code is easier to handle).                                       */

void embed_2_planar(struct edge *edge,unsigned short maxedgenum,unsigned short
                   *planarcode) {
 unsigned short i,j,used_entries = 0;    /* converted entries */   
 int codelen = 0;
 for (i=1; i<=maxedgenum; i++) {
   if (edge[i].v1==0) {used_entries++;}    /* no convertion necessary */
   if (edge[i].v2==0) {used_entries++;}    /* no convertion necessary */
 }
 i = 0;     /* vertex number */
 do {
   i++;
   for (j=1; j<=maxedgenum; j++) {     /* edge number */
     if (edge[j].v1==i) {used_entries++;  
                         planarcode[++codelen] = edge[j].v2;}
     if (edge[j].v2==i) {used_entries++;  
                         planarcode[++codelen] = edge[j].v1;}
   }
   planarcode[++codelen] = 0;
 } while ((int)used_entries < 2*(int)maxedgenum);  
   /* not all vertex information used so far */
 planarcode[0] = i;    /* vertex number of graph */
}

/*************EMBED_2_GRAPH***************************************************/

void embed_2_graph(struct edge *edge,unsigned short maxedgenum,
                  graph *g,int maxm,int n) {
 int i;
 for (i=1; i<=n; i++) {EMPTYSET(GRAPHROW(g,i,maxm),maxm);}
 for (i=1; i<=(int)maxedgenum; i++) {
   if (edge[i].v1 && edge[i].v2) {
     ADDELEMENT(GRAPHROW(g,edge[i].v1,maxm),edge[i].v2);
     ADDELEMENT(GRAPHROW(g,edge[i].v2,maxm),edge[i].v1);
   }
 }
}

/*******************************IS_SIMPLE_NO_LOOPS*************************/
/*     Checks if the graph g is simple and without loops                  */

char is_simple_no_loops(graph *g,int n,int maxm,struct edge *edge,
                       unsigned short maxedgenum) {
 char erg=1;
 int i=1, j=1;
 while (erg==1 && i<=n) {
   if (i==j)
     {if (ISELEMENT(GRAPHROW(g,i,maxm),i)) {return(0);} }
   else {
     if (ISELEMENT(GRAPHROW(g,i,maxm),j) != ISELEMENT(GRAPHROW(g,j,maxm),i))
	 {return(0);}
   }  /* else */
   j++; if (j>n) {i++; j=i;}
 }   /* while */

 if (edge && maxedgenum) {    
    /* edge-embedding  =>  maybe double edges or loops */
   for (i=1; i<(int)maxedgenum; i++) {
     for (j=i+1; j<=(int)maxedgenum; j++) { 
       if ((edge[i].v1==edge[j].v1 && edge[i].v2==edge[j].v2) ||
           (edge[i].v1==edge[j].v2 && edge[i].v2==edge[j].v1)) {return(0);}
     }
   }
   for (i=1; i<=(int)maxedgenum; i++) {
     if (edge[i].v1==edge[i].v2) {return(0);}
   }
 }
 return(1);
}

/*********************CANONICAL_ADJACENCIES**********************************/
/*  Checks for a simple regular graph if it contains the edges 
   (1,2),...,(1,reg+1)                                                     */

BOOL canonical_adjacencies(graph *g,int maxm,int reg) {
 int i;
 for (i=2; i<=reg+1; i++) {
   if (!ISELEMENT(GRAPHROW(g,1,maxm),i) || !ISELEMENT(GRAPHROW(g,i,maxm),1))
     {return(FALSE);}
 }
 return(TRUE);
}

/******************************REGULARITY**********************************/
/*     Gives the regularity of graph g  (-2 if there is none)             */

int regularity(graph *g,int n,int maxm) {
 int i,j,reg=0,reg2;
 for (j=1; j<=n; j++) {if (ISELEMENT(GRAPHROW(g,1,maxm),j)) {reg++;} }
 i=2;
 while (i<=n && reg>=0) {
   reg2=0;
   for (j=1; j<=n; j++) {if (ISELEMENT(GRAPHROW(g,i,maxm),j)) {reg2++;} }
   if (reg2!=reg) {reg=-2;}
   i++;
 }
 return(reg);
}

/*****************************WRITEGRAPH**********************************/
/*  If "planar==TRUE", then "graphdata" must contain the planar embedding
   in "planar_code" or "embed_code" style.                              */

char writegraph(graph *g,int n,int maxm,FILE *f,BOOL header,BOOL planar,
    unsigned short *graphdata,unsigned long count) {
 int x,y,i,pos;
 if (planar) {pos = graphdata[0] ? 0 : 1;}    /* skip leading zero */
 if (header) {fprintf(f,">>writegraph 1%s<<%s",planar ? (char *)" planar" :
              "",NL);}   /* 1 = standard with graph number and 
                                   planar embedding (since 30.7.1996) */
 fprintf(f,"%sGraph number %ld%s",NL,count,NL);
 fprintf(f,"%sNumber of vertices: %5d%s",NL,n,NL);
 fprintf(f,"=========================%s",NL);
 for(x=1; x<=n; x++) {
   i=1;
   fprintf(f,"%5d | ",x);
   if (planar) {
     while (graphdata[++pos]!=0) {  
       /* first entry is vertex number; it is skipped */
	if (i*6+2>CONSOLWIDTH) {i=1; fprintf(f,"%s        ",NL);}
       fprintf(f,"%5d ",graphdata[pos]); i++;
     }
   }
   else {
     for (y=1; y<=n; y++) {
       if (ISELEMENT(GRAPHROW(g,x,maxm),y)) {
	  if (i*6+2>CONSOLWIDTH) {i=1; fprintf(f,"%s        ",NL);}
         fprintf(f,"%5d ",y); i++;
       }
     }
   }
   fprintf(f,NL);
 }
 fprintf(f,NL);
 return(ferror(f) ? 2 : 1);
}

/**************************WRITE_REG_CODE_S**********************************/
/*   includes code REG_CODE (then g2==NULL) and _OLD codes (then old==TRUE) */

char write_reg_code_s(FILE *f,graph *g,graph *g2,int maxm,int n,int reg,
		      unsigned long count,BOOL header,BOOL old,size_t
                     maxentries) {
 unsigned short i,j,s=0,z=0;   /* z = number of dropped elements */
 if (header) {fprintf(f,">>reg_code%s %d %d %s<<",(g2==NULL) ? "" : "_s",n,
              reg,(ENDIAN_OUT==LITTLE_ENDIAN ? "le" : "be"));}
 if (g2!=NULL) {
   if (count>0) {	  /* determine s (as big as possible) */
     if (n>UCHAR_MAX) {s++;}
     i=1+(old==TRUE); j=i+1;
     while (i<n && ISELEMENT(GRAPHROW(g,i,maxm),j)==
                   ISELEMENT(GRAPHROW(g2,i,maxm),j)) {
       if (ISELEMENT(GRAPHROW(g,i,maxm),j) && (size_t)s<maxentries) {s++;}
	j++; if (j>n) {i++; j=i+1;}
     }
   }
   if (write_2byte_number(f,s,ENDIAN_OUT)==2) {return(2);}
 }
 if (n>UCHAR_MAX) {if (z>=s) {fprintf(f,"%c",0);} else {z++;}} 
    /* big graph */
 for (i=1+(old==TRUE); i<n; i++) {
   for (j=i+1; j<=n; j++) {
     if (ISELEMENT(GRAPHROW(g,i,maxm),j)) {
	if (z>=s) {
	  if (n<=UCHAR_MAX) {fprintf(f,"%c",(unsigned char)j);}
	  else {if (write_2byte_number(f,j,ENDIAN_OUT)==2) {return(2);} } }
	else {z++;}
     }
   }
 }
 return(ferror(f) ? 2 : 1);
}

/**************************READ_REG_CODE_S_OLD*****************************/
/*    This function covers the code REG_CODE_OLD (then lastinput==NULL)   */

char read_reg_code_s_old(FILE *f,graph *g,int maxn,int maxm,struct addinfo 
    *info,int endian,BOOL old,unsigned short *lastinput,size_t maxentries) {
 unsigned short s=0, z=0, num, signum, i, *adj;
 int n, reg;
 void einfuegen(graph *g,unsigned short *adj,unsigned short i,
		 unsigned short j,int maxm);

 reg = info->reg;   n = info->n;
 if (!(adj=(unsigned short *)ALLOCS((size_t)(maxn+1),sizeof(unsigned short))))
    {fprintf(stderr,"Fatal error: No workspace!\n");  return(2);}
 for (i=1; i<=n; i++) {EMPTYSET(GRAPHROW(g,i,maxm),maxm);  adj[i]=0;}
 if (old) {for (i=2; i<=reg+1; i++) {einfuegen(g,adj,1,i,maxm);} }
 if (lastinput!=NULL && read_2byte_number(f,&s,endian)==2) 
    {FREES(adj);  return(feof(f) ? 0 : 2);}
 if ((size_t)s>maxentries) {FREES(adj);  return(3);}
 if (read_old_or_new(f,lastinput,s,&z,maxentries,FALSE,endian,&signum)==2)
    {FREES(adj);  return(feof(f) ? 0 : 2);}
 if (signum!=0) {num = signum;}   /* if old, then signum!=0 */
 else {
   if (read_old_or_new(f,lastinput,s,&z,maxentries,TRUE,endian,&num)==2)
      {FREES(adj);  return(2);}
 }
 einfuegen(g,adj,2-(old==FALSE),num,maxm);   /* first vertex-entry in file */
 for (i=2-(old==FALSE); i<n; i++) {
   while (adj[i]<reg) {
     if (read_old_or_new(f,lastinput,s,&z,maxentries,signum==0,endian,&num)
         ==2)  {FREES(adj);  return(2);}
     einfuegen(g,adj,i,num,maxm);
   }
 }
 FREES(adj);
 info->simple = 1;
 return(1);
}

 /* following function valid only for read_reg_code_old_s */
 void einfuegen(graph *g,unsigned short *adj,unsigned short i,
					  unsigned short j,int maxm) {
   ADDELEMENT(GRAPHROW(g,i,maxm),j);    adj[i]++;
   ADDELEMENT(GRAPHROW(g,j,maxm),i);    adj[j]++;
 }

/**************************READ_REG_CODE_S*******************************/
/* This function covers the code  REG_CODE  (then lastinput==NULL)      */

char read_reg_code_s(FILE *f,graph *g,int maxn,int maxm,struct addinfo *info,
 int *endian,unsigned long num,unsigned short *lastinput,size_t maxentries) {
 if (num==0) {
   if (fscanf(f,"%d",&(info->n))==EOF)  {return(2);}
   if (info->n > maxn) {return(3);}
   if (fscanf(f,"%d",&(info->reg))==EOF)  {return(2);}
   if (read_endian(f,endian)==2) {return(2);}
 }
 return(read_reg_code_s_old(f,g,maxn,maxm,info,*endian,FALSE,
                            lastinput,maxentries));
}

/**************************WRITE_MULTI_CODE_S********************************/
/*  includes code MULTI_CODE (then g2==NULL) and _OLD codes 
   (then header==FALSE) and MULTI_CODE2_S_OLD (then code2==True)           */

char write_multi_code_s(FILE *f,graph *g,graph *g2,int maxm,int n,int n2,
			unsigned long count,BOOL header,BOOL code2,
                       size_t maxentries) {
 unsigned short i,j,s=0,z=0;     /* number of dropped elements */
 if (header) {fprintf(f,">>multi_code%s %s<<",(g2==NULL) ? "" : "_s",
	       (ENDIAN_OUT==LITTLE_ENDIAN ? "le" : "be"));}
 if (g2!=NULL) {
   if (count>0 && n==n2) {
     /* determine s (as big as possible) */
     if (n>UCHAR_MAX) {s+=2;} else {s++;}
     i=1; j=2;
     while (i<n && ISELEMENT(GRAPHROW(g,i,maxm),j)==
                   ISELEMENT(GRAPHROW(g2,i,maxm),j)) {
       if (ISELEMENT(GRAPHROW(g,i,maxm),j) && (size_t)s<maxentries) {s++;}
       j++; if (j>n) {i++; j=i+1; if ((size_t)s<maxentries) {s++;}}
     }
   }
   if (code2) {
     if (s>UCHAR_MAX) {s=UCHAR_MAX;}
     fprintf(f,"%c",(unsigned char)s);
   }
   else {
     if (write_2byte_number(f,s,ENDIAN_OUT)==2) {return(2);}
   }
 }
 if (n<=UCHAR_MAX) {if (z>=s) {fprintf(f,"%c",(unsigned char)n);} else {z++;}}
 else {
   if (z>=s) {fprintf(f,"%c",0);} else {z++;}     /* big graph */
   if (z>=s)
      {if (write_2byte_number(f,(unsigned short)n,ENDIAN_OUT)==2) 
          {return(2);} }
   else {z++;}
 }
 for (i=1; i<n; i++) {
   for (j=i+1; j<=n; j++) {
     if (ISELEMENT(GRAPHROW(g,i,maxm),j)) {
	if (z>=s) {     /* write element */
	  if (n<=UCHAR_MAX) {fprintf(f,"%c",(unsigned char)j);}
	  else {if (write_2byte_number(f,j,ENDIAN_OUT)==2) {return(2);} } }
       else {z++;}
     }
   }
   if (z>=s) {
     if (n<=UCHAR_MAX) {fprintf(f,"%c",0);}
     else {if (write_2byte_number(f,0,ENDIAN_OUT)==2) {return(2);} } }
   else {z++;}
 }
 return(ferror(f) ? 2 : 1);
}

/**********************READ_MULTI_CODE_S_OLD*****************************/
/*  This function covers the code MULTI_CODE_OLD (then lastinput==NULL) */
/*  and the function MULTI_CODE2_S_OLD (then code2==True)               */

char read_multi_code_s_old(FILE *f,graph *g,int maxn,int maxm,struct addinfo
    *info,int endian, unsigned short *lastinput,size_t maxentries,
    BOOL code2) {
 int i,n;
 unsigned char c;
 unsigned short s=0, z=0, signum, num;    /* z = read numbers */
 if (lastinput!=NULL) {
   if (code2) 
     {if (fread(&c,sizeof(unsigned char),1,f)==0) {return(feof(f) ? 0 : 2);}
      s = (unsigned short)c;}
   else
     {if (read_2byte_number(f,&s,endian)==2) {return(feof(f) ? 0 : 2);} }
 }
 if ((size_t)s>maxentries) {return(3);}
 if (read_old_or_new(f,lastinput,s,&z,maxentries,FALSE,endian,&signum)==2)
	 {return(feof(f) ? 0 : 2);}
 if (signum==0) {
   if (read_old_or_new(f,lastinput,s,&z,maxentries,TRUE,endian,&num)==2)
      {return(2);}
 }
 else {num = signum;}
 if ((n=(int)num) > maxn) {return(3);}
 for (i=1; i<=n; i++) {EMPTYSET(GRAPHROW(g,i,maxm),maxm);}
 i=1;
 while (i < n) {
   if (read_old_or_new(f,lastinput,s,&z,maxentries,signum==0,endian,&num)==2)
      {return(2);}
   if (num!=0) {ADDELEMENT(GRAPHROW(g,i,maxm),num);
		 ADDELEMENT(GRAPHROW(g,num,maxm),i);}
   else {i++;}
 }  /* while */
 info->reg = -1;
 info->simple = 1;
 info->n = n;
 return(1);
}

/*****************************READ_MULTI_CODE_S*******************************/
/*  This function covers the code MULTI_CODE (then lastinput==NULL)          */

char read_multi_code_s(FILE *f,graph *g,int maxn,int maxm,struct addinfo *info,
    int *endian,unsigned long num,unsigned short *lastinput,
    size_t maxentries) {
 if (num==0) {if (read_endian(f,endian)==2) {return(2);} }
 return(read_multi_code_s_old(f,g,maxn,maxm,info,*endian,lastinput,maxentries,
        False));
}

/**************************WRITE_PLANAR_CODE_S *******************************/
/*  This function covers the code PLANAR_CODE      (then old_g==NULL)        */
/*  and  _OLD codes  (then header==FALSE) and EMBED_CODE (embedcode==True).  */
/*  If embedcode==True AND edgenumbers or vertex number are bigger than 
   UCHAR_MAX, then bigedge==True.                                           */

char write_planar_code_s(FILE *f,unsigned short *output_g,unsigned short
	  *old_g,unsigned long count,BOOL header,size_t maxentries,BOOL
         embedcode,BOOL bigedge) {
 unsigned long s=0,z=0;
 int n,i=1;
 if (header) {fprintf(f,">>%s%s %s<<",
              embedcode ? (char *)"embed_code" : (char *)"planar_code",
              (old_g==NULL) ? "" : "_s",
              (ENDIAN_OUT==LITTLE_ENDIAN ? "le" : "be"));}
 if (embedcode) {fprintf(f,"%c%c",1,2);}   /* significant beginning */
 n = (int)(output_g[0]==0 ? output_g[1] : output_g[0]);
 if (output_g[0] && (n>UCHAR_MAX || bigedge)) {output_g--;}
    /* a leading zero will be added to the current graph 
       (and has been added to the old graph, if also n>UCHAR_MAX holded) */
 else if (output_g[0]==0 && n<=UCHAR_MAX && !bigedge) {output_g++;}
    /* the leading zero will be skipped in the current graph
       (and has been skipped in the old graph, if also n<=UCHAR_MAX holded) */
 /* now  n==output_g[0], if n<=UCHAR_MAX, and
         n==output_g[1], if n>UCHAR_MAX 
         (in the latter case output_g[0] not necessarily ==0) */
 if (old_g != NULL) {   /* write number of common entries */
   if (count>0) {
     if (n>UCHAR_MAX && old_g[0]==0 && (size_t)s<maxentries) {s++;}
        /* both codes begin with 0 (for output_g, this is implicit) */
     while (output_g[s]==old_g[s] && (size_t)s<maxentries) {s++;}
   }
   if (write_2byte_number(f,(unsigned short)s,ENDIAN_OUT)==2) {return(2);}
 }
 /* n>UCHAR_MAX <=> use unsigned short for output */
 if (n<=UCHAR_MAX)                        /* small graph */ 
   {if (z>=s) {fprintf(f,"%c",(unsigned char)n);}
    z++;}   
 else {                                   /* big graph */
   if (z>=s) {fprintf(f,"%c",0);}
   z++;
   if (z>=s)
     {if (write_2byte_number(f,(unsigned short)n,ENDIAN_OUT)==2) {return(2);}}
   z++;
 }
 while (i <= n) {
   if (z>(unsigned long)maxentries) {return(3);} 
      /* graph too big for output_g */
   if (output_g[z]==0) {i++;}      /* next vertex */
   if (z>=s) {
     if (n<=UCHAR_MAX)
        {fprintf(f,"%c",(unsigned char)output_g[z++]);}
     else {if (write_2byte_number(f,output_g[z++],ENDIAN_OUT)==2) 
              {return(2);} }
   }
 }
 return(ferror(f) ? 2 : 1);
}

/**********************READ_PLANAR_CODE_S_OLD*****************************/
/*  This function covers the code PLANAR_CODE_OLD (then sh==FALSE) and
   EMBED_CODE ("edge" has space for the edges).                         */

char read_planar_code_s_old(FILE *f,graph *g,int maxn,int maxm,struct addinfo
    *info,int endian,unsigned short *lastinput,size_t maxentries,BOOL sh,
    struct edge *edge,unsigned short *maxedgenum) {
 int i, n;
 unsigned short s=0, z=0, j, signum, num;    /* z = read numbers */
 BOOL embedcode = False, firstentry = False;
 *maxedgenum = 0;       /* biggest read number of edge so far */
 if (sh && read_2byte_number(f,&s,endian)==2) {return(feof(f) ? 0 : 2);}
 if ((size_t)s>maxentries) {return(3);}
 if (read_old_or_new(f,lastinput,s,&z,maxentries,FALSE,endian,&signum)==2)
    {return(feof(f) ? 0 : 2);}
 if (signum==0) {
   if (read_old_or_new(f,lastinput,s,&z,maxentries,TRUE,endian,&num)==2)
      {return(2);}
   n = (int)num;
 }
 else if (signum==1 && !sh) {     /* embed_code? */
   if (read_old_or_new(f,lastinput,s,&z,maxentries,FALSE,endian,&signum)==2)
     {return(2);}
   if (signum==2) {               /* embed_code */
     embedcode = True;  z-=2;     /* Index 1,2 nicht speichern */
     if (read_old_or_new(f,lastinput,s,&z,maxentries,FALSE,endian,&signum)==2)
        {return(feof(f) ? 0 : 2);}
     if (signum==0) {
       if (read_old_or_new(f,lastinput,s,&z,maxentries,TRUE,endian,&num)==2)
          {return(2);}
       n = (int)num;
     }
     else {n = (int)signum;}    /* n = number of vertices */
   }
   else {num = signum;  n = 1;  signum = 0;  firstentry = True;}
        /* 1-vertex graph, "num" = first entry */
 }
 else {n = (int)signum;}
 if (n>maxn) {return(3);}
 i=1;
 while (i <= n) {
   EMPTYSET(GRAPHROW(g,i,maxm),maxm);
   do {
     if (firstentry)             /* first entry already read -> "num" */
        {firstentry = False;}    /* noticed */
     else if (read_old_or_new(f,lastinput,s,&z,maxentries,
              signum==0,endian,&num)==2)  {return(2);}   /* read new entry */
     if (num!=0) { 
       if (embedcode) {
         if (num>maxentries) {return(3);}
         if (num > *maxedgenum) {       /* initialize new part */
           for (j = *maxedgenum+1; j<=num; j++) {edge[j].v1 = edge[j].v2 = 0;}
           *maxedgenum = num;
         }
         if (edge[num].v1) {edge[num].v2 = i;} else {edge[num].v1 = i;}
       }
       else {ADDELEMENT(GRAPHROW(g,i,maxm),num);}
     }
   } while (num!=0);
   i++;
 }  /* while */
 info->n = n;
 info->reg = -1;
 info->simple = -1;
 return(1);
}

/*****************************READ_PLANAR_CODE_S******************************/
/*  This function covers the code PLANAR_CODE (then sh==FALSE)               */
/*  and EMBED_CODE ("edge" has space for edge information)                   */

char read_planar_code_s(FILE *f,graph *g,int maxn,int maxm,struct addinfo 
    *info,int *endian,unsigned long num,unsigned short *lastinput,
    size_t maxentries,BOOL sh,struct edge *edge,unsigned short *maxedgenum) {
 if (num==0) {if (read_endian(f,endian)==2) {return(2);} }
 return(read_planar_code_s_old(f,g,maxn,maxm,info,*endian,lastinput,
        maxentries,sh,edge,maxedgenum));
}

/**************************WRITE_GRAPH6***************************************/
/*  This function covers the code GRAPH6_OLD  (then header==FALSE)           */

char write_graph6(FILE *f,graph *g,int maxm,int n,BOOL header) {
 int i,j,k=32;
 char byte=0;
 if (header) {fprintf(f,">>graph6<<");}
 if (n<=62) {fprintf(f,"%c",n+63);}    /* N(n) */
 else {fprintf(f,"%c%c%c%c",126,n/4096+63,(n%4096)/64+63,n%64+63);} /* N(n) */
 for (j=2; j<=n; j++) {
   for (i=1; i<j; i++) {
     if (ISELEMENT(GRAPHROW(g,i,maxm),j)) {byte |= k;}
     k=k>>1;
     if (k==0) {fprintf(f,"%c",byte+63); k=32; byte=0;}
   }
 }
 if (k!=32) {fprintf(f,"%c",byte+63);}
 fprintf(f,NL);
 return(ferror(f) ? 2 : 1);
}

/**********************READ_GRAPH6_OLD******************************/

char read_graph6_old(FILE *f,graph *g,int maxn,int maxm,struct addinfo *info) {
 int n,i,j,k=0;
 char c[3];
 int byte;
 do {                        /* skip newline(s) */
   if ((byte = fgetc(f))==EOF) {return(feof(f) ? 0 : 2);}
 } while (byte<63 || byte>126);
 if (byte==126) {
   if (fread(&c[0],sizeof(char),3,f)<3) {return(2);}
   n = ((int)c[0]-63)*4096+((int)c[1]-63)*64+((int)c[2]-63); }
 else {n = byte-63;}
 if (n>maxn) {return(3);}
 for (i=1; i<=n; i++) {EMPTYSET(GRAPHROW(g,i,maxm),maxm);}
 byte = 0;
 for (j=2; j<=n; j++) {
   for (i=1; i<j; i++) {
     if (k==0) {if ((byte = fgetc(f))==EOF) {return(2);}
		 byte-=63; k=32;}
     if (byte&k) {ADDELEMENT(GRAPHROW(g,i,maxm),j);
		   ADDELEMENT(GRAPHROW(g,j,maxm),i);}
     k=k>>1;
   }
 }
 info->n = n;
 info->reg = -1;
 info->simple = 1;
 return(1);
}

/************************READ_GRAPH6***************************************/

char read_graph6(FILE *f,graph *g,int maxn,int maxm,struct addinfo *info,
                unsigned long num) {
 unsigned char c[2]={' ',' '};
 if (num==0) {if (read_to_end_of_header(f,&c[0])==2) {return(2);} }
 return(read_graph6_old(f,g,maxn,maxm,info));
}

/**************************WRITE_DIGRAPH6*************************************/
/*  This function covers the code DIGRAPH6_OLD  (then header==FALSE)         */

char write_digraph6(FILE *f,graph *g,int maxm,int n,BOOL header) {
 int i,j,k=32;
 char byte=0;
 if (header) {fprintf(f,">>digraph6<<");}
 if (n<=62) {fprintf(f,"%c",n+63);}    /* N(n) */
 else {fprintf(f,"%c%c%c%c",126,n/4096+63,(n%4096)/64+63,n%64+63);}  /* N(n) */
 for (i=1; i<=n; i++) {
   for (j=1; j<=n; j++) {
     if (ISELEMENT(GRAPHROW(g,i,maxm),j)) {byte |= k;}
     k=k>>1;
     if (k==0) {fprintf(f,"%c",byte+63); k=32; byte=0;}
   }
 }
 if (k!=32) {fprintf(f,"%c",byte+63);}
 fprintf(f,NL);
 return(ferror(f) ? 2 : 1);
}

/**********************READ_DIGRAPH6_OLD******************************/

char read_digraph6_old(FILE *f,graph *g,int maxn,int maxm,struct addinfo 
    *info) {
 int n,i,j,k=0;
 char c[3];
 int byte;
 do {                        /* skip newline(s) */
   if ((byte = fgetc(f))==EOF) {return(feof(f) ? 0 : 2);}
 } while (byte<63 || byte>126);
 if (byte==126) {
   if (fread(&c[0],sizeof(char),3,f)<3) {return(2);}
   n = ((int)c[0]-63)*4096+((int)c[1]-63)*64+((int)c[2]-63); }
 else {n = byte-63;}
 if (n>maxn) {return(3);}
 byte = 0;
 for (i=1; i<=n; i++) {
   EMPTYSET(GRAPHROW(g,i,maxm),maxm);
   for (j=1; j<=n; j++) {
     if (k==0) {if ((byte = fgetc(f))==EOF) {return(2);}
	         k=32; byte-=63;}
     if (byte&k) {ADDELEMENT(GRAPHROW(g,i,maxm),j);}
     k=k>>1;
   }
 }
 info->n = n;
 info->reg = -1;
 info->simple = -1;
 return(1);
}

/************************READ_DIGRAPH6***************************************/

char read_digraph6(FILE *f,graph *g,int maxn,int maxm,struct addinfo *info,
    unsigned long num) {
 unsigned char c[2]={' ',' '};
 if (num==0) {if (read_to_end_of_header(f,&c[0])==2) {return(2);} }
 return(read_digraph6_old(f,g,maxn,maxm,info));
}

/************************WRITE_WRITEGRAPH2D**********************************/
/*  covers code WRITEGRAPH3D (then dim=3, else dim=2) and _OLD
   (then old==TRUE)                                                        */
/*  If "planar==TRUE", then "inputgraph" must contain the planar embedding
   in "planar_code" style.                                                 */

char write_writegraph2d(FILE *f,graph *g,double *coord,int maxm,int n,int dim,
 BOOL old,BOOL header,BOOL planar,unsigned short *inputgraph) {
 int i,j,pos;
 if (planar) {pos = inputgraph[0] ? 0 : 1;}    /* skip leading zero */
 if (header) {fprintf(f,">>writegraph%dd%s<<%s%s",dim,planar ?
    (char *)" planar" : "",NL,NL);}
 for (i=1; i<=n; i++) {
   fprintf(f,"%d",i);
   for (j=1; j<=dim; j++) {fprintf(f," %g",coord ? coord[(i-1)*3+(j-1)] : 0);}
   if (planar) {
     while (inputgraph[++pos]!=0) {fprintf(f," %d",inputgraph[pos]);}
     /* the first entry is the vertex number, so it must be skipped */
   }
   else {    
     for (j=1; j<=n; j++)
       {if (ISELEMENT(GRAPHROW(g,i,maxm),j)) {fprintf(f," %d",j);} }
   }
   fprintf(f,NL);
 }
 if (old==FALSE) {fprintf(f,"0%s%s",NL,NL);}
 return(ferror(f) ? 2 : 1);
}

/************************READ_WRITEGRAPH2D_OLD********************************/
/*  covers code WRITEGRAPH3D_OLD (then dim=3, else dim=2) and new formats
	 (then graph ends with 0 instead of a vertex-id)                     */
/*  If "planar_out==TRUE", then "inputgraph" must provide memory to receive 
   the planar embedding in "planar_code" style. "planar_out==TRUE" does not
   mean that the input is really planar, but the user chooses an output code
   which contains a planar embedding. If "*inputgraph==nil", then the memory
   is provided automatically.                                               */

char read_writegraph2d_old(FILE *f,graph *g,double *coord,int maxn,int maxm,
    struct addinfo *info,int dim,BOOL planar_out,unsigned short **inputgraph,
    size_t maxentries) {
 char c[LINELEN];
 char *pos;
 long int i,j;
 unsigned short *sortgraph=nil; /* if "planar==TRUE", then the input is first
             stored into this array because the entries must be sorted after
             vertex IDs */
 int n,sortpos = 0;    /* next position to fill in array "sortgraph" */  
 int sortlen;          /* length of array "sortgraph" */
 int inputpos = 0;     /* next position to fill in array "inputgraph" */
 double d;
 BOOL ende=FALSE;
 BOOL found=FALSE;     

 n=0;
 for (i=1; (int)i<=maxn; i++) {EMPTYSET(GRAPHROW(g,i,maxm),maxm);}
 if (planar_out) {    /* provide memory to store adjacencies */
   sortgraph = get_workspace2(maxentries);
   if (*inputgraph == NULL)   /* provide memory for embedding */
      {*inputgraph = get_workspace2(maxentries);}
 }

 do {
   if (fgets(&c[0],LINELEN,f)==NULL) {
     ende = TRUE;
     if (!feof(f))  {if (sortgraph) {FREES(sortgraph);} 
                     return(2);}
     else if (n==0) {if (sortgraph) {FREES(sortgraph);} 
                     return(0);}
     /* else:  read last graph in file:  continue after loop */
   }
   else {
     pos = &c[0];
     if ((i = strtol(pos,&pos,10))!=0L) {    /* regular line */
       if ((int)i>n) {n=(int)i;}
       if (n>maxn) {if (sortgraph) {FREES(sortgraph);} 
                    return(3);}
       if (planar_out) {sortgraph[sortpos++] = i;}    /* store vertex ID */
       if (coord) {coord[(i-1)*3+2] = 0.0;}  /* 3rd dimension=0, if 2d->3d */
         for (j=1; j<=dim; j++) {
  	  d = strtod(pos,&pos);
	  if (coord) {coord[(i-1)*3+(j-1)] = d;}
       }
       while ((j = strtol(pos,&pos,10))!=0L) {
         ADDELEMENT(GRAPHROW(g,i,maxm),j);
         if (planar_out) {sortgraph[sortpos++] = j;}
       }
       if (planar_out) {sortgraph[sortpos++] = 0;}
     }  /* if */
     else {if (*(pos-1)=='0') {ende=TRUE;} }
   }
 } while (!feof(f) && !ende);
 info->n = n;
 info->reg = -1;
 info->simple = -1;

 if (planar_out) {      /* sort entries after vertex IDs */
   sortlen = sortpos;
   (*inputgraph)[inputpos++] = n;    /* number of vertices */
   for (i=1; i<=n; i++) {         /* find adjacent vertices of vertex "i" */
     sortpos = 0;  found = FALSE;
     while (sortpos<sortlen) {
       if (sortpos==0 || sortgraph[sortpos-1]==0) {
         /* sortgraph[sortpos] indicates a vertex number */
         if (sortgraph[sortpos]==i) {  /* adjacencies of vertex "i" follow */
           found = TRUE;               /* found adjacencies of vertex "i" */
           while (sortgraph[++sortpos]!=0)  
             {(*inputgraph)[inputpos++] = sortgraph[sortpos];}
           (*inputgraph)[inputpos++] = 0;
           sortpos = sortlen;
         }
       }
       sortpos++;
     }
     if (!found) {     /* no adjacencies for vertex "i" found */
       (*inputgraph)[inputpos++] = 0;
     }
   }
 }         /* if planar */            
 if (sortgraph) {FREES(sortgraph);}
 return(1);
}

/************************READ_WRITEGRAPH2D************************************/
/*  covers code WRITEGRAPH3D                                                 */

char read_writegraph2d(FILE *f,graph *g,double *coord,int maxn,int maxm,
    struct addinfo *info,int dim,unsigned long num,BOOL *planar_in,BOOL
    *planar_out,unsigned short **inputgraph,size_t maxentries) {
 unsigned char c[2]={' ',' '};
 if (num==0) {
   switch (read_header_entry(f,(char *)"planar")) {
     case 0: {*planar_in = FALSE;   break;}
     case 1: {*planar_in = *planar_out = TRUE;   break;}
             /* the value of "planar_out" is only important if the output
                code supports planar embeddings AND adjacency matrices
                (like writegraph2d or writegraph3d), so that the program
                tries to write a planar embedding whenever it is able to */
     case 2: {return(2);}
   }
   if (read_to_end_of_header(f,&c[0])==2) {return(2);} 
 }
 return(read_writegraph2d_old(f,g,coord,maxn,maxm,info,dim,*planar_out,
        inputgraph,maxentries));
}

/************************WRITE_BROOKHAVEN_PDB********************************/
/*  "coord" MUST point on a coordinate array                                */

char write_brookhaven_pdb(FILE *f,graph *g,double *coord,int maxm,int n) {
 int i,j,adj,pos;
 for (i=1; i<=n; i++) {
   adj = 0;                 /* adjacency of vertex i */
   for (j=1; j<=n; j++) {
     if (ISELEMENT(GRAPHROW(g,i,maxm),j) || ISELEMENT(GRAPHROW(g,j,maxm),i))
	 {adj++;}
   }
   fprintf(f,"ATOM  %5d  %c                %8.3f%8.3f%8.3f"
             "                          %s",
             i,adj==1 ? 'H' : 'C',coord[(i-1)*3],
             coord[(i-1)*3+1],coord[(i-1)*3+2],NL);
 }
 for (i=1; i<=n; i++) {
   fprintf(f,"CONECT%5d",i);
   for (j=1; j<=n; j++) {
     if (ISELEMENT(GRAPHROW(g,i,maxm),j) || ISELEMENT(GRAPHROW(g,j,maxm),i))
	{fprintf(f,"%5d",j);}
   }
   fprintf(f,"%s",NL);
 }
 return(ferror(f) ? 2 : 1);
}

/***************************IN_RANGE******************************************/

BOOL in_range(unsigned long num,int r,unsigned long *range) {
 int i=0;
 if (r==0) {return(TRUE);}
 while (i<r) {
   if (range[i*2]<=num && range[i*2+1]>=num) {return(TRUE);}
   i++;
 }
 return(FALSE);
}

/***************************CONVERT************************************/
/*    return-values from the functions called:                        */
/*    return = 4  =>  header expected, but not found                  */
/*    return = 3  =>  graph is too big                                */
/*    return = 2  =>  error while reading/writing                     */
/*    return = 1  =>  no error occurred                               */
/*    return = 0  =>  end of file at the beginning of reading         */

void convert(graph *g,graph *g2,int maxn,int maxm,int oldformat,int newformat,
	     FILE *inputfile,FILE *outputfile,int r,unsigned long *range,
	     unsigned long range_max,int *options,unsigned short **inputgraph,
	     unsigned short *inputgraph2,size_t maxentries,BOOL append,
	     double *coords,BOOL planar_in,BOOL planar_out,
            struct edge *edge,unsigned short *planarcode) {
 struct addinfo addinfo;   /* additional information about the read graph 
    (this information is determined by the reading function except
     REG_CODE_OLD where the information has to be given by the user) */
 struct addinfo addinfo2;            /* for old graph */
 char erg=1;
 unsigned long num=0;     /* num = number of read graphs */
 unsigned long count=0;   /* count = number of written graphs */
 unsigned short maxedgenum;    /* for embed_code */
 int endian;              /* determined and used by some format functions */
 graph *h;                /* dummy variable for swapping */
 int reg = -1;            /* desired regularity for every graph */
 int n = -1;              /* desired vertex number for every graph */
 BOOL header = TRUE;      /* TRUE => write header */
 if (append) {header=FALSE;}  /* no header when new graphs to existing file */
 do {
   /* read graph: */
   maxedgenum = 0;
   switch (oldformat) {
     case PLANAR_CODE_S_OLD:
       {erg = read_planar_code_s_old(inputfile,g,maxn,maxm,&addinfo,
              ENDIAN_IN,*inputgraph,maxentries,TRUE,edge,&maxedgenum); break;}
     case PLANAR_CODE_S:
       {erg = read_planar_code_s(inputfile,g,maxn,maxm,&addinfo,&endian,
              num,*inputgraph,maxentries,TRUE,edge,&maxedgenum); break;}
     case PLANAR_CODE_OLD:
       {erg = read_planar_code_s_old(inputfile,g,maxn,maxm,&addinfo,
              ENDIAN_IN,*inputgraph,maxentries,FALSE,edge,&maxedgenum); 
              break;}
     case PLANAR_CODE:
	{erg = read_planar_code_s(inputfile,g,maxn,maxm,&addinfo,&endian,
              num,*inputgraph,maxentries,FALSE,edge,&maxedgenum); break;}
     case EMBED_CODE:
       {erg = read_planar_code_s(inputfile,g,maxn,maxm,&addinfo,&endian,
              num,*inputgraph,maxentries,FALSE,edge,&maxedgenum); break;}
     case MULTI_CODE_S_OLD:
       {erg = read_multi_code_s_old(inputfile,g,maxn,maxm,&addinfo,
              ENDIAN_IN,*inputgraph,maxentries,FALSE); break;}
     case MULTI_CODE_S:
	{erg = read_multi_code_s(inputfile,g,maxn,maxm,&addinfo,&endian,num,
                                *inputgraph,maxentries); break;}
     case MULTI_CODE_OLD:
	{erg = read_multi_code_s_old(inputfile,g,maxn,maxm,&addinfo,
                                    ENDIAN_IN,NULL,0,FALSE); break;}
     case MULTI_CODE:
       {erg = read_multi_code_s(inputfile,g,maxn,maxm,&addinfo,&endian,num,
				 NULL,0); break;}
     case REG_CODE_S_OLD:
	{addinfo.n = options[0];  addinfo.reg = options[1];
	 erg = read_reg_code_s_old(inputfile,g,maxn,maxm,&addinfo,
	       ENDIAN_IN,TRUE,*inputgraph,maxentries); break;}
     case REG_CODE_S:
       {erg = read_reg_code_s(inputfile,g,maxn,maxm,&addinfo,
              &endian,num,*inputgraph,maxentries); break;}
     case REG_CODE_OLD:
       {addinfo.n = options[0];  addinfo.reg = options[1];
	 erg = read_reg_code_s_old(inputfile,g,maxn,maxm,&addinfo,
				   ENDIAN_IN,TRUE,NULL,0); break;}
     case REG_CODE:
	{erg = read_reg_code_s(inputfile,g,maxn,maxm,&addinfo,
              &endian,num,NULL,0); break;}
     case GRAPH6:
       {erg = read_graph6(inputfile,g,maxn,maxm,&addinfo,num); break;}
     case GRAPH6_OLD:
	{erg = read_graph6_old(inputfile,g,maxn,maxm,&addinfo); break;}
     case DIGRAPH6:
	{erg = read_digraph6(inputfile,g,maxn,maxm,&addinfo,num); break;}
     case DIGRAPH6_OLD:
	{erg = read_digraph6_old(inputfile,g,maxn,maxm,&addinfo); break;}
     case WRITEGRAPH2D_OLD:
	{erg = read_writegraph2d_old(inputfile,g,coords,maxn,maxm,&addinfo,2,
              planar_out,inputgraph,maxentries); break;}
     case WRITEGRAPH3D_OLD:
	{erg = read_writegraph2d_old(inputfile,g,coords,maxn,maxm,&addinfo,3,
              planar_out,inputgraph,maxentries); break;}
     case WRITEGRAPH2D:
	{erg = read_writegraph2d(inputfile,g,coords,maxn,maxm,&addinfo,2,num,
              &planar_in,&planar_out,inputgraph,maxentries);   break;}
     case WRITEGRAPH3D:
	{erg = read_writegraph2d(inputfile,g,coords,maxn,maxm,&addinfo,3,num,
              &planar_in,&planar_out,inputgraph,maxentries);   break;}
     case MULTI_CODE2_S_OLD:
       {erg = read_multi_code_s_old(inputfile,g,maxn,maxm,&addinfo,
              ENDIAN_IN,*inputgraph,maxentries,TRUE); break;}
   }
   switch (erg) {
     case 0: {break;}
     case 1: {num++;
       erg=1;
	if (in_range(num,r,range)) {    /* check conditions: */
         if (is_in(can_convert[newformat-1][oldformat-1],(char *)"P")) { 
           /* planar embedding */
           if (!planar_in && count==0) {
             fprintf(stderr,"Warning:  It is not checked whether the read "
                     "Graphs are\nplanar embeddings. If they are not, you "
                     "will get useless results.\n");
           }
         } 
         if (is_in(can_convert[newformat-1][oldformat-1],(char *)"RTDE")) { 
           /* Vertex number */
           if (count==0) {n=addinfo.n;}
           else {
             if (n!=addinfo.n)
               {fprintf(stderr,"Error while checking: Graph %ld has not "
                        "%d vertices!\n",num,n); return;}
           }
         }
	  if (is_in(can_convert[newformat-1][oldformat-1],(char *)"RTDE")) { 
             /* regular? */
	      /* the first graph determines the desired regularity for
                all graphs */
	    if (count==0) {
	      if (addinfo.reg>=0) {reg=addinfo.reg;}
	      else {
	        if ((reg = regularity(g,addinfo.n,maxm))==-2)
		   {fprintf(stderr,"Error while checking: The first graph"
			    " to be written is not regular!\n"); return;}
	      }
	    }
	    else {   /* count>0 */
	      if (addinfo.reg==-2 || (addinfo.reg>=0 && reg!=addinfo.reg) ||
	          (addinfo.reg==-1 && reg!=regularity(g,addinfo.n,maxm)))
	        {fprintf(stderr,"Error while checking: Graph %ld is "
				"not %d-regular!\n",num,reg); return;}
	    }
	  }
	  if (is_in(can_convert[newformat-1][oldformat-1],(char *)"STE")) { 
           /* simple without loops ? */
	    if (addinfo.simple!=1 && 
               is_simple_no_loops(g,addinfo.n,maxm,edge,maxedgenum)!=1)
	      {fprintf(stderr,"Error while checking: Graph %ld has loops"
			      " or is not simple!\n",num); return;}
         }
         if (is_in(can_convert[newformat-1][oldformat-1],(char *)"CDE")) {
            /* Canonical adjacencies (in regular graphs): this check must be
               after the regularity check so that the desired regularity is
               already determined when the canonical check appears. */
           if (!canonical_adjacencies(g,maxm,reg))
              {fprintf(stderr,"Error while checking: Graph %ld does not "
               "contain the canonical edges for the output code!\n",num);
               return;}
         } 
	  /* write graph: */
         if (edge && maxedgenum) {  /* convert to "graph" */
           embed_2_graph(edge,maxedgenum,g,maxm,addinfo.n);
           embed_2_planar(edge,maxedgenum,planarcode);
         }
	  switch (newformat) {
	    case WRITEGRAPH:
	      {erg = writegraph(g,addinfo.n,maxm,outputfile,header,planar_out,
                    *inputgraph,num);  break;}
	    case MULTI_CODE_S:
	      {erg = write_multi_code_s(outputfile,g,g2,maxm,addinfo.n,
		     addinfo2.n,count,header,FALSE,maxentries); break;}
	    case MULTI_CODE:
	      {erg = write_multi_code_s(outputfile,g,NULL,maxm,addinfo.n,
		     0,count,header,FALSE,maxentries); break;}
	    case REG_CODE_S:
	      {erg = write_reg_code_s(outputfile,g,g2,maxm,addinfo.n,
                    addinfo.reg,count,header,FALSE,maxentries); break;}
	    case REG_CODE:
	      {erg = write_reg_code_s(outputfile,g,NULL,maxm,addinfo.n,
                    addinfo.reg,count,header,FALSE,maxentries); break;}
	    case REG_CODE_S_OLD:
	      {erg = write_reg_code_s(outputfile,g,g2,maxm,addinfo.n,
                    addinfo.reg,count,FALSE,TRUE,maxentries); break;}
	    case REG_CODE_OLD:
	      {erg = write_reg_code_s(outputfile,g,NULL,maxm,addinfo.n,
                    addinfo.reg,count,FALSE,TRUE,maxentries); break;}
	    case MULTI_CODE_S_OLD:
	      {erg = write_multi_code_s(outputfile,g,g2,maxm,addinfo.n,
		     addinfo2.n,count,FALSE,FALSE,maxentries); break;}
	    case MULTI_CODE_OLD:
	      {erg = write_multi_code_s(outputfile,g,NULL,maxm,addinfo.n,
		     0,count,FALSE,FALSE,maxentries); break;}
	    case PLANAR_CODE_S:
	      {erg = write_planar_code_s(outputfile,*inputgraph,inputgraph2,
                    count,header,maxentries,False,False);  break;}
	    case PLANAR_CODE:
	      {erg = write_planar_code_s(outputfile,maxedgenum ?
                    planarcode : *inputgraph,NULL,count,header,maxentries,
                    False,False);   break;}
	    case PLANAR_CODE_S_OLD:
	      {erg = write_planar_code_s(outputfile,*inputgraph,inputgraph2,
		     count,FALSE,maxentries,False,False); break;}
	    case PLANAR_CODE_OLD:
	      {erg = write_planar_code_s(outputfile,maxedgenum ?
                    planarcode : *inputgraph,NULL,count,FALSE,maxentries,
                    False,False);   break;}
	    case EMBED_CODE:
	      {erg = write_planar_code_s(outputfile,*inputgraph,NULL,count,
                    FALSE,maxentries,maxedgenum>0,maxedgenum>UCHAR_MAX);  
                    break;}
	    case GRAPH6:
	      {erg = write_graph6(outputfile,g,maxm,addinfo.n,header);
	       break;}
	    case GRAPH6_OLD:
	      {erg = write_graph6(outputfile,g,maxm,addinfo.n,FALSE);
	       break;}
	    case DIGRAPH6:
	      {erg = write_digraph6(outputfile,g,maxm,addinfo.n,header);
	       break;}
	    case DIGRAPH6_OLD:
	      {erg = write_digraph6(outputfile,g,maxm,addinfo.n,FALSE);
	       break;}
	    case WRITEGRAPH2D_OLD:
	      {erg = write_writegraph2d(outputfile,g,coords,maxm,
		     addinfo.n,2,TRUE,FALSE,planar_out,*inputgraph);
	       if (erg==1) {erg=0;}  /* write only one graph */   break;}
	    case WRITEGRAPH3D_OLD:
	      {erg = write_writegraph2d(outputfile,g,coords,maxm,
		     addinfo.n,3,TRUE,FALSE,planar_out,*inputgraph);
	       if (erg==1) {erg=0;}  /* write only one graph */	break;}
	    case WRITEGRAPH2D:
	      {erg = write_writegraph2d(outputfile,g,coords,maxm,
		     addinfo.n,2,FALSE,header,planar_out,*inputgraph);  break;}
	    case WRITEGRAPH3D:
	      {erg = write_writegraph2d(outputfile,g,coords,maxm,
		     addinfo.n,3,FALSE,header,planar_out,*inputgraph);  break;}
	    case MULTI_CODE2_S_OLD:
	      {erg = write_multi_code_s(outputfile,g,g2,maxm,addinfo.n,
		     addinfo2.n,count,FALSE,TRUE,maxentries); break;}
           case BROOKHAVEN_PDB:
             {erg = write_brookhaven_pdb(outputfile,g,coords,maxm,addinfo.n);
              if (erg==1) {erg=0;}  /* write only one graph */   break;} 
	  }
	  switch (erg) {
	    case 2: {fprintf(stderr,"Error while writing graph %d!\n",num); 
                    return;}
	    case 3: {fprintf(stderr,"Error while writing graph %d:"
				    " Graph too big!\n",num); return;}
	  }
	  count++;
	  header = FALSE;    /* after first written graph no more headers */
	}
	break;}
     case 3: {fprintf(stderr,"Error while reading graph %d: Graph too big!\n",
                      num+1);  return;}
     case 4: {fprintf(stderr,"Error while reading: No header found!\n");
	       return;}
     default: /* case 2 */ {fprintf(stderr,"Error while reading graph %d!\n",
                                    num+1); return;}
   }  /* switch */
   /* save written graph: */
   if (g2!=NULL) {
      h=g2;    /* pointer to old graph memory must be saved */
      g2=g;    /* read graph becomes old graph */
      g=h;     /* next graph uses the memory of the dropped graph */
      addinfo2 = addinfo;
   }
   if (inputgraph2!=NULL) {    /* inputgraph2 gets old data */
      if (memcpy(inputgraph2,inputgraph,sizeof(unsigned short)*maxentries)==0)
	  {fprintf(stderr,"Error while copying internal data!\n"); return;}
   }
 } while (erg==1 && (r==0 || num<range_max));
 fprintf(stderr,"Read %d Graphs.\n",num);
}

/************************************MAIN************************************/

int main(int argc,char *argv[])

{
FILE *inputfile, *outputfile, *headerfile;
BOOL extract_header,add_header,read_header;
int maxn,maxm,i,r=0,outputfilename=0,inputfilename=0;
size_t maxentries;
unsigned short *inputgraph=NULL;   /* pointer on workspace for input graph */
unsigned short *inputgraph2=NULL;  /* pointer for planar_code-conversion */
unsigned short *planarcode=NULL;   /* for converted embed_code */
struct edge *edge = NULL;          /* for embed_code */
unsigned long j;
unsigned long range[MAXR][2];
unsigned long range_max=0;    /* highest right-entry in array range */
char *c;
graph *g,*g2=NULL;     /* g2 for short-codes (old graph) */
double *coords=NULL;   /* for codes using coordinates */
int oldformat;
int newformat;
int options[2];        /* for REG_CODE_OLD and REG_CODE_S_OLD */
BOOL append=FALSE;     /* TRUE => append new graphs to existing file */
BOOL planar_in,planar_out;   /* TRUE => read/written codes are 
                               planar embeddings (these parameters may be
                               changed after reading the header of a file) */

inputfile = stdin;
outputfile = stdout;    /* for converted graphs */
headerfile = stdout;    /* for extracted header */
extract_header = FALSE; /* no header extraction by default */
add_header = FALSE;     /* no header addition by default */
maxn = 511;             /* 511: don't increase this on PC */
maxentries = 16300;     /* maximum common short-int-entries for 2 successive
                          graphs (needed only for short-code input)
			   or for convertion with planar_code
			   (16300: don't increase this on PC) */
oldformat = 0;          /* 0 = format will be recognized automatically */
newformat = 0;          /* 0 = keep old format
                     (so the program can be used to extract graphs) */

i=1;
while (i<argc) {
 switch (argv[i][0]) {
   case '-': {
     if (strcmp(argv[i],"-s")==0) {     /* "s" like "special" */
	i++;
	if (i>=argc || !(oldformat = find_format_ID(argv[i]))) {
	  fprintf(stderr,"Error -s: Could not find format %s!\n",argv[i]);
	  exit(4);
	}
	i++;
	if (oldformat==REG_CODE_OLD || oldformat==REG_CODE_S_OLD) {
	  if (i<argc-1) {
	    options[0]=atoi(argv[i]); i++; options[1]=atoi(argv[i]); i++;
	    if (options[0]>maxn) 
             {fprintf(stderr,"Error -s: Parameter n too big!\n"); exit(44);}
           if (options[0]<=0 || options[1]<=0) 
             {fprintf(stderr,"Error -s: Invalid or missing parameters!\n");
	       exit(5);}
	  }
	  else {fprintf(stderr,"Error -s: Additional parameters missing!\n");
	        exit(6);}
	}
     }
     else if (strcmp(argv[i],"-f")==0) {
       i++;
	if (i>=argc || !(newformat = find_format_ID(argv[i]))) {
	  fprintf(stderr,"Error -f: Could not find format %s!\n",argv[i]);
	  exit(7);
	}
	i++;
     }
     else if (strcmp(argv[i],"-i")==0) {
	i++;
	if (i>=argc)
	  {fprintf(stderr,"Error -i: No input file specified!\n"); exit(8);}
	if ((inputfile=fopen(argv[i],"rb"))==nil)
	  {fprintf(stderr,"Error -i: Could not open input file!\n"); exit(9);}
	inputfilename = i;
	i++;
     }
     else if (strcmp(argv[i],"-o")==0) {
	i++;
	if (i>=argc)
	  {fprintf(stderr,"Error -o: No output file specified!\n"); exit(10);}
	outputfilename = i;
       append=FALSE;
       i++;
     }
     else if (strcmp(argv[i],"-oo")==0) {
	i++;
	if (i>=argc)
	  {fprintf(stderr,"Error -oo: No output file specified!\n"); exit(11);}
       outputfilename = i;
	append=TRUE;
	i++;
     }
     else if (strcmp(argv[i],"-e")==0) {
	if (add_header==TRUE)
	  {fprintf(stderr,"Error -e: Don't use option -a simultaneously!\n");
           exit(12);}
	extract_header=TRUE;
	i++;
	if (!(i>=argc || argv[i][0]=='-' || argv[i][0]=='+')) {
	  if ((headerfile=fopen(argv[i],"w"))==nil)
	    {fprintf(stderr,"Error -e: Could not open header file!\n");
            exit(13);}
	  i++;
	}
     }
     else if (strcmp(argv[i],"-a")==0) {
	if (extract_header==TRUE)
	  {fprintf(stderr,"Error -a: Don't use option -e simulaneously!\n");
          exit(14);}
	add_header = TRUE;
       i++;
	if (i>=argc || (headerfile=fopen(argv[i],"rb"))==nil)
	  {fprintf(stderr,"Error -a: Could not open header file!\n"); 
          exit(15);}
	i++;
     }
     else if (strcmp(argv[i],"-n")==0) {
       i++;
       if (i>=argc || !(maxn = atoi(argv[i]))) {
	  fprintf(stderr,"Error -n: Could not read number!\n"); exit(16);
	}
	i++;
     }
     else if (strcmp(argv[i],"-m")==0) {
	i++;
	if (i>=argc || !(maxentries = (size_t)atol(argv[i])))
	  {fprintf(stderr,"Error -m: Could not read number!\n"); exit(17);}
	i++;
     }
     else
	{fprintf(stderr,"Error: Nonidentified option %s!\n",argv[i]);
        exit(18);}
     break;
   }   /* case */

   case '+': {
     if (strcmp(argv[i],"+e")==0) {
       i++;
	if (add_header==TRUE)
	   {fprintf(stderr,"Error +e: Don't use option -a simultaneously!\n");
           exit(19);}
       extract_header=TRUE;
     }
     else
	{fprintf(stderr,"Error: Nonidentified option %s!\n",argv[i]); 
        exit(20);}
     break;
   }

   default: {     /* range numbers */
     if ((j = strtoul(argv[i],&c,10))==0)
	{fprintf(stderr,"Error: Nonidentified option %s!\n",argv[i]); 
        exit(21);}
     else {
	if (r>=MAXR) {fprintf(stderr,"Error: Too many options!\n"); exit(22);}
	range[r][0]=j;
	if (*c==0) {range[r][1]=j;}
	else if (*c=='-') {
	  if ((j = strtoul(c+1,nil,10))==0)
	    {fprintf(stderr,"Error: Nonidentified option %s!\n",c+1); 
            exit(23);}
	  else {range[r][1]=j;}
	}
	else {fprintf(stderr,"Error: Nonidentified option %s!\n",c); exit(24);}
	if (range[r][1]>range_max) {range_max=range[r][1];}
	r++;  i++;
     }
   }
 }   /* switch */
}     /* while */

/*  for extraction of header  */
if (extract_header) {
 char erg;
 erg = extract_the_header(inputfile,headerfile);
 switch (erg) {
   case 2: {fprintf(stderr,"Error while reading!\n"); exit(25);}
   case 4: {fprintf(stderr,"Error while reading: No header found!\n"); 
            exit(26);}
 }
 return(0);
}

/*  for addition or substitution of header:  */
if (add_header) {
 char erg;
 if (outputfilename!=0) {
   if ((outputfile=fopen(argv[outputfilename],append ? "ab" : "wb"))==nil)
      {fprintf(stderr,"Error: Could not open output file!\n"); exit(27);}
 }
 erg = add_the_header(inputfile,outputfile,headerfile);
 switch (erg) {
   case 2: {fprintf(stderr,"Error while reading!\n"); exit(28);}
   case 4: {fprintf(stderr,"Error while reading: No header found!\n"); 
            exit(29);}
 }
 return(0);
}

/*  for normal convertion:  look for input code */
if (maxn>MAXN)
 {fprintf(stderr,"Error -n: Number too big (maximum %d)!\n",MAXN); exit(30);}
if (maxn<1)
 {fprintf(stderr,"Error -n: Number too small (minimum 1)!\n"); exit(31);}
read_header = (oldformat==0);
if (read_header) {oldformat = read_headerformat(inputfile);}
switch (oldformat) {
 case -2: {fprintf(stderr,"Error while reading header!\n"); exit(32);}
 case -3: {fprintf(stderr,"Error while reading header: Code name too "
           "long!\n");  exit(33);}
 case -4: {fprintf(stderr,"Error while reading header: No header found!\n");
	    exit(34);}
 case  0: {fprintf(stderr,"Error while reading header: Unknown code!\n");
	    exit(35);}
}
if (newformat==0)  {newformat = oldformat;}
if (can_convert[newformat-1][oldformat-1]=='N')
 {fprintf(stderr,"Error: This convertion is not possible/not supported!\n");
	exit(36);}

/* open input file again:  this time ASCII or binary */
if (inputfile != stdin) {
 fclose(inputfile);
 if ((inputfile=fopen(argv[inputfilename],codebin[oldformat-1] ? "rb" : "r"))
     ==nil)
   {fprintf(stderr,"Error: Could not open input file!\n"); exit(37);}
 if (read_header) {oldformat = read_headerformat(inputfile);}
   /* no new information, just to reach the old position in the inputfile */
 switch (oldformat) {
   case -2: {fprintf(stderr,"Error while reading header!\n"); exit(38);}
   case -3: {fprintf(stderr,"Error while reading header: Code name too "
             "long!\n");  exit(39);}
   case -4: {fprintf(stderr,"Error while reading header: No header found!\n");
	      exit(40);}
   case  0: {fprintf(stderr,"Error while reading header: Unknown code!\n");
	      exit(41);}
 }
}

/* open output file:  ASCII or binary */
if (outputfilename!=0) {
 if (codebin[newformat-1]==FALSE) {
   if ((outputfile=fopen(argv[outputfilename],append ? "a" : "w"))==nil)
     {fprintf(stderr,"Error: Could not open output file!\n"); exit(42);} }
 else {
   if ((outputfile=fopen(argv[outputfilename],append ? "ab" : "wb"))==nil)
     {fprintf(stderr,"Error: Could not open output file!\n"); exit(43);}
 }
}

/* preparation: */
maxm = (maxn+WORDSIZE-1)/WORDSIZE;
g = get_workspace(maxn,maxm);
planar_in = oldformat==PLANAR_CODE_S_OLD || oldformat==PLANAR_CODE_S ||
           oldformat==PLANAR_CODE_OLD   || oldformat==PLANAR_CODE ||
           oldformat==EMBED_CODE;
           /* more possibilities after reading the header (then the
              preparation will be done inside the appropriate read/write
              function) */
planar_out = newformat==PLANAR_CODE_S_OLD || newformat==PLANAR_CODE_S ||
            newformat==PLANAR_CODE_OLD   || newformat==PLANAR_CODE   ||
            newformat==EMBED_CODE ||  planar_in; 
            /* more possibilities after reading the header */
            /* if "planar_in==TRUE", then it always tried to store the
               planar_embedding, so "planar_out==TRUE" */
if (newformat==REG_CODE_S    || newformat==REG_CODE_S_OLD    ||
   newformat==MULTI_CODE_S  || newformat==MULTI_CODE_S_OLD  ||
   newformat==MULTI_CODE2_S_OLD)
    {g2 = get_workspace(maxn,maxm);}
if (planar_out || planar_in ||
   oldformat==REG_CODE_S   || oldformat==REG_CODE_S_OLD   ||
   oldformat==MULTI_CODE_S || oldformat==MULTI_CODE_S_OLD ||
   oldformat==MULTI_CODE2_S_OLD) {
   inputgraph = get_workspace2(maxentries);
   planarcode = get_workspace2(maxentries);
   edge = get_workspace4(maxentries);
   if (newformat==PLANAR_CODE_S || newformat==PLANAR_CODE_S_OLD)
      {inputgraph2 = get_workspace2(maxentries);}
}   /* If oldformat is some kind of PLANAR_CODE, then the input graph is
      stored in case the newformat is writable. If a PLANAR_CODE is written,
      then the adjacent vertices of a vertex are not sorted. */ 
if (newformat==WRITEGRAPH2D_OLD || newformat==WRITEGRAPH3D_OLD ||
   newformat==WRITEGRAPH2D     || newformat==WRITEGRAPH3D     ||
   newformat==BROOKHAVEN_PDB) {coords = get_workspace3(3*(long int)maxn);}

/* convertion: */
convert(g,g2,maxn,maxm,oldformat,newformat,inputfile,outputfile,
	r,&range[0][0],range_max,options,&inputgraph,inputgraph2,maxentries,
	append,coords,planar_in,planar_out,edge,planarcode);

/* after the end: */
FREES(g);
if (g2) {FREES(g2);}
if (inputgraph) {FREES(inputgraph);}
if (inputgraph2) {FREES(inputgraph2);}
if (planarcode) {FREES(planarcode);}
if (edge) {FREES(edge);}
if (coords) {FREES(coords);}
if (inputfile != stdin)   {fclose(inputfile);}
if (outputfile != stdout) {fclose(outputfile);}
if (headerfile != stdout) {fclose(headerfile);}
return(0);
}
