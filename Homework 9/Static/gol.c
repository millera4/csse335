#include "gol_helpers.h"
#include <stdio.h>
#include <stdlib.h>

void transition(gol_board* g, int t){
  int i; int j;

  enum STATUS mystatus;
  enum STATUS newstatus;
  int oc_nbrs;
  for (i=0; i<g->m; i++){
    for (j=0;j<g->n;j++){
      mystatus=get_status(g,i,j,t);
      oc_nbrs=num_occupied_nbrs(g,i,j,t);
      if (mystatus==OCCUPIED){
	if (oc_nbrs==2 || oc_nbrs==3) newstatus=OCCUPIED;
	if (oc_nbrs>=4) newstatus=EMPTY;
	if (oc_nbrs<2) newstatus=EMPTY;
      }
      if (mystatus==EMPTY){
	if (oc_nbrs==3) newstatus=OCCUPIED;
	else newstatus=EMPTY;
      }
      set_status(g,i,j,t+1,newstatus);
    }

  }

}


int main(int argc,char** argv){

  gol_board g;
  options o;
  o=parse_commandline(argc,argv);
  print_options(o);
  g.generations=o.generations;
  initialize_board(o.inputfile,&g);
  int i;
  for (i=0;i<g.generations-1;i++){
    transition(&g,i);
  }
  //print_gol_board(&g);
  gol_board_to_gif(&g,o);
  return (0);
}
