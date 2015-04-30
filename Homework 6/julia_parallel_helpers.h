void printusage(){
  fprintf(stderr,"julia <options>\n");
  fprintf(stderr,"options are:\n");
  fprintf(stderr,"--c=<a+bi|#1|#2|#3|#4|#5>    Sets the constant for iteration.  Defaults to -0.4+0.6i. #1,#2,#3,#4,#5 give nice outputs.\n");
  fprintf(stderr,"--minr=a    Sets minimum real coordinate to a. Defaults to -2.\n");
  fprintf(stderr,"--maxr=b    Sets maximum real coordinate to b.  Defaults to 2.\n");
  fprintf(stderr,"--mini=c    Sets minimum imaginary coordinate to c.  Defaults to -2.\n");
  fprintf(stderr,"--maxi=d    Sets maximum imaginary coordinate to d.  Defaults to 2.\n");
  fprintf(stderr,"--numsamples=n  Sets the number of samples in each direction to n.  Results in n x n pixel image.  Defaults to 512.\n");
  fprintf(stderr,"--maxiterations=n  Sets number of iterations to be considered \"surely in Mandelbrot set\" to n.  Defaults to 500.\n");
  fprintf(stderr,"--maxnorm=M    Sets cutoff on norm of iterations to M.  Defaults to 2.\n");
  fprintf(stderr,"--outputfile=filename  Sets the output file name.  Defaults to mandelbrot. Proper extension is appended.\n");
  fprintf(stderr,"--forcebyteswap=<0|1>  Forces output file to be written in with endian-ness reversed.  Defaults to 0.  Debugging purposes only.");

  fprintf(stderr,"--staticallocation=<0|1>  Forces static processor allocation.\n");

  fprintf(stderr,"--linechunksize=l  Forces dynamic allocator to dole out l lines at a time.  Defaults to 10. Ignored if staticallocation==1\n");
  


  
  fprintf(stderr,"-h print this message.\n");

  
}

void set_opts(int argc, char** argv,double* minr, double* maxr, double* mini, double* maxi, int* numsamples, long int* maxiterations, double* maxnorm, char** outputfile, complex* c,
	      int* forcebyteswap,int* staticallocation, int* linechunksize){
  int i;
  double dummyd;
  double dummyd1;
  long int dummyli;
  int dummyi;
  char dummystr[1000];
  *minr=-2;
  *maxr=2;
  *mini=-2;
  *maxi=2;
  *numsamples=512;
  *maxiterations=500;
  *maxnorm=2;
  *outputfile=malloc(1000*sizeof(char));
  strcpy(*outputfile,"julia.bmp");
  c->real=-0.4;
  c->imag=0.6;
  *forcebyteswap=0;
  *staticallocation=0;
  *linechunksize=10;
  
  int maxiteroverride=0;
  
  for (i=1;i<argc;i++){
    if (sscanf(argv[i],"--staticallocation=%d",&dummyi)==1){
      *staticallocation=dummyi;
    }
    else if (sscanf(argv[i],"--linechunksize=%d",&dummyi)==1){
      *linechunksize=dummyi;
    }
    else if (sscanf(argv[i],"--c=%lf+%lfi",&dummyd,&dummyd1)==2){
      c->real=dummyd;
      c->imag=dummyd1;
    }
    else if(sscanf(argv[i],"--c=%lf-%lfi",&dummyd,&dummyd1)==2){
      c->real=dummyd;
      c->imag=-dummyd1;
    }
    else if(sscanf(argv[i],"--c=%lf",&dummyd)==1 && argv[i][strlen(argv[i])-1]=='i'){
      c->real=0;
      c->imag=dummyd;
    }
    else if(sscanf(argv[i],"--c=%lf",&dummyd)==1){
      c->real=dummyd;
      c->imag=0;
    }
    else if(strcmp(argv[i],"--c=#1")==0){
      c->real=-0.4;
      c->imag=0.6;
    }
    else if(strcmp(argv[i],"--c=#2")==0){
      c->real=-.618034;
      c->imag=0;
    }
    else if(strcmp(argv[i],"--c=#3")==0){
      c->real=-0.123;
      c->imag=0.745;
    }
    else if(strcmp(argv[i],"--c=#4")==0){
      c->real=-1;
      c->imag=0;
    }
    else if(strcmp(argv[i],"--c=#5")==0){
      c->real=-0.8;
      c->imag=0.156;
    }
    
    else if (sscanf(argv[i],"--minr=%lf",&dummyd)==1){
      *minr=dummyd;
    }
    else if (sscanf(argv[i],"--maxr=%lf",&dummyd)==1){
      *maxr=dummyd;
    }
    else if (sscanf(argv[i],"--mini=%lf",&dummyd)==1){
      *mini=dummyd;
    }
    else if (sscanf(argv[i],"--maxi=%lf",&dummyd)==1){
      *maxi=dummyd;
    }
    else if (sscanf(argv[i],"--numsamples=%d",&dummyi)==1){
      *numsamples=dummyi;
    }
    else if (sscanf(argv[i],"--maxiterations=%ld",&dummyli)==1){
      *maxiterations=dummyli;
      maxiteroverride=1;
    }
    else if (sscanf(argv[i],"--maxnorm=%lf",&dummyd)==1){
      *maxnorm=dummyd;
    }
    else if (sscanf(argv[i],"--outputfile=%s",dummystr)==1){
      strcpy(*outputfile,dummystr);
    }
    else if (strcmp(argv[i],"-h")==0){
      printusage();
      exit(0);
    }
    else if(sscanf(argv[i],"--forcebyteswap=%d",&dummyi)==1){
      *forcebyteswap=dummyi;
    }
    else{
      fprintf(stderr,"Unrecognized option %s\n",argv[i]);
      printusage();
      exit(1);
    }
      

  }

  

}

