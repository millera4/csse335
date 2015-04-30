#ifndef __NBODYUTILS__
#define __NBODYUTILS__

#include<stdlib.h>
#include<stdio.h>
#include<math.h>
#include<string.h>



typedef struct{
  int numsteps;     //number of steps to take
  double endtime;   //ending time of simulation
  double stepsize;  //stepsize
}sim_opts;

typedef struct{
  double x;
  double y;
}vector;

typedef struct{
  int N;    //how many bodies there are
  double G; //Gravitational constant for units
  vector* X0; //vector array of initial positions
  vector* V0; //vector array of initial velocities
  vector* X;  //vector array [n*times] of all positions at all times
  double* M;  //array of masses
  double* times; //array of times in simulation
  int numsteps; //number of steps taken
}nbody_dataset;

void print_usage(){
  printf("nbody input_file output_file <nsteps=x> <endtime=x> <stepsize=x>\n\n");
  printf("two of nsteps, endtimes, and stepsize must be supplied.\n");
  printf("endtime may be specified in earth days, years, or hours, assuming that the input velocity is in something/second\n");
  printf("For example endtime=1y means 1 earth year, endtime=1d means 1 earth day, endtime=1h means 1 earth hour"); 
    
}


void print_options(sim_opts o){
  
  printf("Number of timesteps: %d\n",o.numsteps);
  printf("Endtime: %lf\n",o.endtime);
  printf("Stepsize: %lf\n",o.stepsize);
}



void read_sim_opts(int argc,char** argv, sim_opts* s){
  int end_time_specified=0;
  double T=0;
  int n_steps_specified=0;
  int stepsize_specified=0;

  float dummy;

  int i;

  for (i=3;i<argc;i++){
    if (sscanf(argv[i],"endtime=%lf",&T)==1){
      switch (argv[i][strlen(argv[i])-1]){
      case 'y':
	T*=3.15569e7;
	break;
      case 'd':
	T*=86400;
	break;
      case 'h':
	T*=3600;
	break;
      }
      s->endtime=T;
      end_time_specified=1;
    }
    else if (sscanf(argv[i],"nsteps=%f",&(dummy))==1){
      s->numsteps=(int)dummy;
      n_steps_specified=1;
    }
    else if (sscanf(argv[i],"stepsize=%lf",&(s->stepsize))==1){
      stepsize_specified=1;
    }
    else{
      print_usage();
      fprintf(stderr,"Error parsing option %s\n",argv[i]);
      exit(1);
    }
  }

  if (end_time_specified && n_steps_specified && stepsize_specified){
    fprintf(stderr,"You can't specify stepsize, number of steps, and end time.\n");
    print_usage();
    exit(1);
  }
  else if (end_time_specified && n_steps_specified){
    s->stepsize=T/s->numsteps;
  }
  else if (end_time_specified && stepsize_specified){
    s->numsteps=T/s->stepsize;
  }
  else if (stepsize_specified && n_steps_specified){

  }
  else{
    fprintf(stderr,"You must specify two of end time, number of steps, and step size.\n");
    print_usage();
    exit(1);
  }


}

double dist(vector v, vector w){
  return hypot(v.x-w.x,v.y-w.y);

}

void free_nbody_dataset(nbody_dataset d){
  free(d.X0);
  free(d.V0);
  free(d.X);
  free(d.M);
  free(d.times);
}



//Load data from file into a dataset. 
void load_data(char* filename, nbody_dataset* d){
  FILE* fp=fopen(filename,"r");
  if (fp==NULL){
    fprintf(stderr,"Error opening %s\n",filename);
    exit(1);
  }
  if (fscanf(fp,"%d ",&(d->N))!=1){
    fprintf(stderr,"Error reading first line of %s to get number of bodies. Sorry!\n",filename);
    exit(1);
  }
  d->X0=malloc(sizeof(vector)*(d->N));
  d->V0=malloc(sizeof(vector)*(d->N));
  d->M=malloc(sizeof(double)*(d->N));
  d->G=6.674e-11;
  vector* x0=d->X0;
  vector* v0=d->V0;
  double* M=d->M;
  int numread;
  int i=0;
  char line[10000];
  while (fgets(line,10000,fp)!=NULL){
    if (line[0]!='#'){
      numread=sscanf(line,"%lf,%lf,%lf,%lf,%lf\n",&(x0[i].x),&(x0[i].y),&(v0[i].x),&(v0[i].y),&(M[i]));
      if (numread!=5){
	fprintf(stderr,"Error reading line %d,%s,of %s.\n",i,line,filename);
	exit(1);
      }
      i++;
    }
  }

  if (i!=d->N){
    fprintf(stderr,"Error.  %s said there were %d bodies, but read %d.\n",filename,d->N,i);
    exit(1);
  }
}


//Write data from dataset to file. 
void write_data(char* filename,nbody_dataset* d ){

  FILE* fp=fopen(filename,"w+");
  if (fp==NULL){
    fprintf(stderr,"Error opening file %s\n",filename);
    exit(1);
  }
  fprintf(fp,"%d,%d\n",d->N,d->numsteps);
  vector (*x)[d->N] = (vector (*)[d->N])d->X;
 
  int ti; int i;
  for (ti=0;ti<=d->numsteps;ti++){
    for (i=0;i<d->N;i++){
      fprintf(fp,"%lf,%lf,%lf\n",x[ti][i].x,x[ti][i].y,d->times[ti]);
    }
  }

  fclose(fp);
}



#endif
