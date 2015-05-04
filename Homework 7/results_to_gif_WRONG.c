#include <stdio.h>
#include <math.h>
#include"gifsave89.c"
#include "vector.h"
#include<limits.h>
#include<string.h>

typedef struct options{
  int numframes;
  int resolution[2];
  double viewport[4];
  char outputfile[1000];
  char inputfile[1000];
  int radius_style;
}options;

typedef struct world{
  double minx;
  double maxx;
  double miny;
  double maxy;
}world;

typedef struct canvas{
  unsigned char* pixels;
  int nr;
  int nc;
}canvas;

typedef struct wcoord{
  double x;
  double y;
}wcoord;

typedef struct pcoord{
  int r;
  int c;
}pcoord;
  

pcoord world_to_px(wcoord wc, world w, canvas c){
  pcoord p;
  p.r=round((wc.y-w.miny)/(w.maxy-w.miny)*c.nr);
  p.c=round((wc.x-w.minx)/(w.maxx-w.minx)*c.nc);
  return p;
}

wcoord px_to_world(pcoord pc, world w, canvas c){
  wcoord wc;
  wc.x=w.minx+ ((float)pc.c/c.nc)*(w.maxx-w.minx);
  wc.y=w.miny+ ((float)pc.r/c.nr)*(w.maxy-w.miny);
}

int wlen_to_clen(double wlen,world w, canvas c){
  return round(wlen/(w.maxy-w.miny)*(c.nr));
}

double clen_to_wlen(int clen,world w, canvas c){
  return (float)clen/(c.nr)*(w.maxy-w.miny);
}


int min(int i, int j){
  return i<j ? i:j;
}

int max(int i, int j){
  return i>j?i:j;
}

double dmin(double i, double j){
  return i<j ? i:j;
}

double dmax(double i, double j){
  return i>j?i:j;
}


int savegif (char* filename, unsigned char *gifimage, int nbytes ) {
  FILE *fileptr = fopen(filename,"wb");
  if (fileptr==NULL){
    fprintf(stderr,"Error opening %s for writing. \n",filename);
    exit(1);
  }
  int  nwritten = 0;
  if ( fileptr != NULL ) {
    nwritten = fwrite(gifimage,sizeof(unsigned char),nbytes,fileptr);
    fclose(fileptr); }
  return ( nwritten );
}


int create_gif(char* fname, int nr, int nc, double minx, double maxx, double miny, double maxy,vector* v, double* r,int numr, int nv, int nt,int numframes){

  /* --- gifsave89's three functions: newgif(),putgif(),endgif() --- */
  void *gsdata=NULL;
  int  nbytes=0;
  
  /* --- additional variables --- */
  int  colortable[]={255,255,255, 0,0,0, -1}, bgindex=0;

  unsigned char* pixels=calloc(nr*nc,sizeof(unsigned char));

  unsigned char *gifimage = NULL;

  
  double cxs[nv];
  double cys[nv];
  double rs[nv];
  int i;
  int ti;


  int frameskip=max(nt/numframes,1);
  
  /* --- render gif from them --- */
  gsdata = newgif((void**) &gifimage,nc,nr,colortable,bgindex);
  if ( gsdata != NULL ) {
    animategif(gsdata,1,0,-1,2);
    for (ti=0;ti<nt;ti+=frameskip){
      //printf("Frame %d\n",ti);
      memset(pixels,0,nr*nc*sizeof(unsigned char));
      
      for (i=0;i<nv;i++){
	cxs[i]=v[ti*nv+i].x;
	cys[i]=v[ti*nv+i].y;
	if (numr==1){
	  rs[i]=r[0];
	}
	else{
	  rs[i]=r[i];
	}
	//	printf("Vector (%lf,%lf)\n",cxs[i],cys[i]);
      }
      draw_circles(pixels,  nr, nc, minx, maxx, miny, maxy, cxs,cys,rs,nv);
      
      //controlgif(gsdata,0,0,0,0);
      putgif(gsdata,pixels);
    }
    nbytes = endgif(gsdata);
  }
  /* --- do what you want with the gif (e.g., write to a file) --- */
  if ( nbytes > 0 ) {
    savegif(fname,gifimage,nbytes);
    free(gifimage); }
  return ( 0 );
}


int draw_circles(unsigned char* pixels, int nr, int nc, double minx, double maxx, double miny, double maxy, double* cx, double* cy, double* r, int ncircs){


  world w;
  w.minx=minx;
  w.maxx=maxx;
  w.miny=miny;
  w.maxy=maxy;

  canvas c;
  c.nr=nr;
  c.nc=nc;
  c.pixels=pixels;
  
  int i;
  for (i=0;i<ncircs;i++){
  int dr, dc;
  //printf("r=%f",r[i]);
  int rp=wlen_to_clen(r[i],w,c);
  wcoord wcenter; wcenter.x=cx[i]; wcenter.y=cy[i];
  pcoord pcenter;
  pcenter=world_to_px(wcenter,w,c);
  int row;
  int coffset, ocoffset;
  int o;
  int cc;
  ocoffset=0;
  //  printf("C=(%d,%d), R=%d\n",pcenter.r,pcenter.c,rp);
  for (row=pcenter.r+rp; row>=pcenter.r-rp;row--){
    //printf("row=%d\n",row);
    if (row>=0 && row<c.nr){
      coffset=round(sqrt(rp*rp-(row-pcenter.r)*(row-pcenter.r)));
      //printf("coffset=%d, ocoffset=%d\n",coffset,ocoffset);
      for (o=min(coffset,ocoffset); o<=max(coffset,ocoffset);o++){
	cc=pcenter.c+o;
	if (0<=cc && cc< c.nc){
	  c.pixels[(c.nr-1-row)*c.nc+cc]=1;
	}
	cc=pcenter.c-o;
	if (0<=cc && cc< c.nc){
	  c.pixels[(c.nr-1-row)*c.nc+cc]=1;
	}
      }
     ocoffset=coffset;

    }
  }
  }



}


void load_results(char* filename,vector** v,double** ms,double* maxmass,int* N, int* T,double* minxcoord,double* maxxcoord,double* minycoord,double* maxycoord){
  FILE* fp=fopen(filename,"r");
  if (fp==NULL){
    fprintf(stderr,"Error opening file %s for reading. \n",filename);
    exit(1);
  }
  if (fscanf(fp,"%d,%d",N,T)!=2){
    fprintf(stderr,"Error reading file %s on line 1.\n",filename);
    exit(1);
  }

  *v=malloc((*N)*(*T)*sizeof(vector));
  *ms=malloc((*N)*sizeof(double));
  int n,t;
  double x,y,m;
  double minx,miny,maxx,maxy;
  minx=INFINITY;
  miny=INFINITY;
  maxx=-INFINITY;
  maxy=-INFINITY;
  *maxmass=-INFINITY;
  int line=2;

  for (n=0;n<*N;n++){
    if(fscanf(fp,"%lf",(*ms)+n)!=1){
      fprintf(stderr,"Error reading file %s on line %d.\n",filename,line);
      exit(1);
    }
    *maxmass=dmax(*maxmass,(*ms)[n]);
    line++;
  }
  
  for (t=0;t<*T;t++){
    for (n=0;n<*N;n++){
      if (fscanf(fp,"%lf,%lf,%lf",&x,&y,&m)!=3){
	fprintf(stderr,"Error reading file %s on line %d.\n",filename,line);
	exit(1);
      }
      (*v)[t*(*N)+n].x=x;
      (*v)[t*(*N)+n].y=y;
      minx=dmin(x,minx); maxx=dmax(x,maxx); miny=dmin(y,miny); maxy=dmax(y,maxy);
      line++;
    }
  }
  *maxxcoord=maxx;
  *maxycoord=maxy;
  *minxcoord=minx;
  *minycoord=miny;
}
void print_usage(){
  fprintf(stderr,"usage:\n");
  fprintf(stderr,"results_to_gif input_file  <options>\n");
  fprintf(stderr,"\noptions:\n");
  fprintf(stderr,"-h this help message.\n");
  fprintf(stderr,"--outputfile=filename\tCreate outputfile filename.  The gif extension is not automatically appended.  Default is test.gif.\n");
  fprintf(stderr,"--viewport=(mx,Mx,my,My)\tOnly render the picture for world coordinates mx < x <Mx and my < y < My.  Default is to keep all bodies on screen at all times.\n");
  fprintf(stderr,"--resolution=MxN\tMake the gif by M x N pixels.  Default is 256x256.");
  fprintf(stderr,"--numframes=%d\tDisplay this many frames from the simulation.  It is not possible to show every single time step, as this would take forever and would make the gif huge.  Instead, use this many equally spaced (in time) frames. Default is 200\n");
  fprintf(stderr,"radius=<constant|proportional>\tIf radius is constant, each body will have the same radius.  If proportional, radius is proportional to mass. Default is proportional.\n");
   
}

void print_options(options o){
  printf("Input file: %s\n",o.inputfile);
  printf("Output file: %s\n",o.outputfile);
  printf("Resolution: %d x %d\n",o.resolution[0],o.resolution[1]);
  if (o.viewport[0]==-1){
    printf("Viewport: Default\n");
  }
  else{
    printf("Viewport: %lf < x < %lf, %lf < y < %lf\n",o.viewport[0],o.viewport[1],o.viewport[2],o.viewport[3]);
  }
  printf("Number of output frames: %d\n",o.numframes);
  printf("Radius Style: %s\n",o.radius_style==1?"Proportional":"Constant");

}

options parseoptions(int argc, char** argv){
  int i;
  options o;
  if (argc<2){
    print_usage();
    exit(1);
  }
  o.viewport[0]=o.viewport[1]=o.viewport[2]=o.viewport[3]=-1;
  o.resolution[0]=o.resolution[1]=256;
  strcpy(o.inputfile,argv[1]);
  strcpy(o.outputfile,"test.gif");
  o.numframes=200;
  o.radius_style=2;
  char dummys[1000];
  int dummyi1,dummyi2;
  double dummyd1,dummyd2,dummyd3,dummyd4;
  strcpy(argv[1],o.inputfile);
  for (i=2;i<argc;i++){
    if (sscanf(argv[i],"--resolution=%dx%d",&dummyi1,&dummyi2)==2){
      o.resolution[0]=dummyi1;
      o.resolution[1]=dummyi2;
    }
    else if (sscanf(argv[i],"--outputfile=%s",dummys)==1){
      strcpy(o.outputfile,dummys);
    }
    else if (sscanf(argv[i],"--numframes=%d",dummyi1)==1){
      o.numframes=dummyi1;
    }
    else if (sscanf(argv[i],"--viewport=(%lf,%lf,%lf,%lf)",&dummyd1,&dummyd2,&dummyd3,&dummyd4)==4){
      o.viewport[0]=dummyd1; o.viewport[1]=dummyd2; o.viewport[2]=dummyd3; o.viewport[3]=dummyd4;
    }
    else if (strcmp(argv[i],"--radius=constant")==0){
      o.radius_style=2;
    }
    else if (strcmp(argv[i],"--radius=proportional")==0){
      o.radius_style=1;
    }
    else{
      print_usage();
      fprintf(stderr,"\n\nUnknown option %s\n",argv[i]);
      exit(1);
    }
  }
  return o;
}


int main ( int argc, char** argv ) {


  int N;
  int T;
  double minx,miny,maxx,maxy;
  vector* vs;
  double* ms;
  double maxmass;
  options o;
  o=parseoptions(argc,argv);
  print_options(o);
  load_results(o.inputfile,&vs,&ms,&maxmass,&N,&T,&minx,&maxx,&miny,&maxy);
  printf("Loaded %d positions at %d times in range x=(%lf, %lf), y=(%lf,%lf)\n",N,T,minx,maxx,miny,maxy);
  if (o.viewport[0]==o.viewport[1]&&o.viewport[1]==-1){
    o.viewport[0]=minx;
    o.viewport[1]=maxx;
    o.viewport[2]=miny;
    o.viewport[3]=maxy;
  }

  //Most massive object takes up this percent of area on the screen.
  double C=.01;
  double maxr=sqrt(C*(maxy-miny)*(maxx-minx)/M_PI);
  double* rs;
  if (o.radius_style==1){
    rs=malloc(N*sizeof(double));
    int i;
    for (i=0;i<N;i++){
      rs[i]=maxr*ms[i]/maxmass;
      printf("Radius=%lf\n",rs[i]);
    }

  }
  else{
    rs=malloc(sizeof(double));
    rs[0]=maxr;
  }
  
  create_gif(o.outputfile, o.resolution[0],o.resolution[1], o.viewport[0], o.viewport[1], o.viewport[2], o.viewport[3],vs, rs,o.radius_style==1?N:1 , N,T,o.numframes);


} 



