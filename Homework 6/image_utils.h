
#ifndef IMAGE_UTILS
#define IMAGE_UTILS

#include <stdint.h>
#include <math.h>
#define round(x) ((x)>=0?(long)((x)+0.5):(long)((x)-0.5))

typedef struct bitmapheader
{
  uint8_t  type[2];                 /* "BM" */
  uint32_t filesize;                /* Size of file in bytes */
  uint16_t reserved;                /* set to 0 */
  uint32_t offset_to_bitmaps;       /* Byte offset to actual bitmap data (= 54) */
  uint32_t bisize;                  /* Size of BITMAPINFOHEADER, in bytes (= 40) */
  uint32_t biwidth;                 /* Width of image, in pixels */
  uint32_t biheight;                /* Height of images, in pixels */
  uint16_t bicplanes;               /* Number of planes in target device (set to 1) */
  uint16_t bitdepth;                /* Bits per pixel (24 in this case) */
  uint32_t compression  ;           /* Type of compression (0 if no compression) */
  uint32_t compressedsize;          /* Image size, in bytes (0 if no compression) */
  uint32_t xpixelspermeter;         /* Resolution in pixels/meter of display device */
  uint32_t ypixelspermeter;         /* Resolution in pixels/meter of display device */
  uint32_t numcolors;               /* Number of colors in the color table (if 0, use 
				        maximum allowed by biBitCount) */
  uint32_t numimptcolors;           /* Number of important colors.  If 0, all colors 
				       are important */
}bitmapheader;

void byteswap(void* data, void* swapped_data,int numbytes){


  
  if (numbytes==1){
    
    uint8_t* sd=(uint8_t*) swapped_data;
    uint8_t* d=(uint8_t*) data;
    *sd=*d;
  }
  else if(numbytes==2){
    uint16_t bytemask=0xff;
    uint16_t* sd=(uint16_t*) swapped_data;
    uint16_t* d=(uint16_t*) data;
    *sd=(*d&bytemask)<<8 | (*d&(bytemask<<8))>>8;
  }
  else if(numbytes==4){ 
     uint32_t bytemask=0xff;
     uint32_t* d=(uint32_t*)data;
     uint32_t* sd=(uint32_t*) swapped_data;
     /* uint32_t part1=(bytemask&*d)<<24; */
     /*  printf("part1: 0x%08x\n",part1); */
     /* uint32_t part2=((bytemask<<8)&*d)<<8; */
     /*  printf("part2: 0x%08x\n",part2); */
     /* uint32_t part3=((bytemask<<16)&*d)>>8; */
     /*  printf("part3: 0x%08x\n",part3); */
     /* uint32_t part4= ((bytemask<<24)&*d)>>24; */
     /*  printf("part4: 0x%08x\n",part4); */
     *sd=(bytemask&*d)<<24 | ((bytemask<<8)&*d)<<8 | ((bytemask<<16)&*d)>>8 | ((bytemask<<24)&*d)>>24; 
   } 
  else{
   fprintf(stderr,"Unsupported byteswap requested: %d\n",numbytes);
    exit(1);
  }

  }
  


void fwrite_swapped(void* data, int datanum, int datasize_bytes, FILE* fp){
  void* tmp=malloc(datasize_bytes);
  int i;
  for (i=0;i<datanum;i++){
    byteswap(data+i,tmp,datasize_bytes);
    fwrite(tmp,1,datasize_bytes,fp);
  }

  free(tmp);

}

void fwrite_lsb(void* data, int datanum, int datasize_bytes, FILE* fp, int force_swapped){

  
  uint32_t one=1;
  int is_little_endian=0;
  int do_swapping=0;
  if (((char*)&one)[0]==1){
    is_little_endian=1;
  }

  do_swapping= is_little_endian==1 ? 0:1;
  
  if (force_swapped==1){
    do_swapping= do_swapping==1 ? 0:1;
  }
  
  
  if (!do_swapping){
    fwrite(data,datanum,datasize_bytes,fp);
  }
  else{
    fwrite_swapped(data,datanum,datasize_bytes,fp);
  }

}



/*Writes a 24 bit full-color bmp file.
fname - file name to be written.  String.
r,g,b - each m*n where the image will be m x n pixels. Stored in ROW-MAJOR order. 
that is, r[0] is pixel (0,0).  r[1] is for pixel (0,1).  r[2] is pixel (0,2), ... , r[n-1]
is pixel (0,n-1).  r[n] is pixel (1,0), and so on. r stores the red color component, g the green color component,
b the blue color component.  

m,n - the dimensions of the image in pixels. 

min - a value smaller than any value in r,g,b.  This value will mean "no color". 
max - a value larger than any value in r,g,b.  This value will mean "max color". 

forcebyteswap -- if 1, this forces output to be in byte-swapped order. Shouldn't be needed, as endianness should be taken care of automatically. */ 
 
int write_rgb_bmp(char* fname, double* r,double* g, double* b,  int m, int n, double min, double max,int forcebyteswap){

  bitmapheader H;
  FILE* fp;
  int pad_bytes=(4-((3*n)%4))%4;
  int linewidth_bytes=3*n+pad_bytes;
  fp = fopen(fname,"wb");
  if (fp==NULL){
    fprintf(stderr,"Error opening %s for writing.\n",fname);
    return 1;
  }
  H.type[0]='B'; H.type[1]='M';
  H.filesize=14+40+linewidth_bytes*m;
  H.reserved=0;
  H.offset_to_bitmaps=14+40;
  H.bisize=40;
  H.biwidth=n;
  H.biheight=m;
  H.bicplanes=1;
  H.bitdepth=24;
  H.compression=0;
  H.compressedsize=0;
  H.xpixelspermeter=0;
  H.ypixelspermeter=0;
  H.numcolors=0;
  H.numimptcolors=0;

  //Now write the header to file.
  fwrite_lsb(&H.type,2,1,fp,forcebyteswap);
  fwrite_lsb(&H.filesize,1,4,fp,forcebyteswap);
  //Not a typo -- two reserved fields that I duplicate. 
  fwrite_lsb(&H.reserved,1,2,fp,forcebyteswap);
  fwrite_lsb(&H.reserved,1,2,fp,forcebyteswap);
  fwrite_lsb(&H.offset_to_bitmaps,1,4,fp,forcebyteswap);

  fwrite_lsb(&H.bisize,1,4,fp,forcebyteswap);
  fwrite_lsb(&H.biwidth,1,4,fp,forcebyteswap);
  fwrite_lsb(&H.biheight,1,4,fp,forcebyteswap);
  fwrite_lsb(&H.bicplanes,1,2,fp,forcebyteswap);
  fwrite_lsb(&H.bitdepth,1,2,fp,forcebyteswap);
  fwrite_lsb(&H.compression,1,4,fp,forcebyteswap);
  fwrite_lsb(&H.compressedsize,1,4,fp,forcebyteswap);
  fwrite_lsb(&H.xpixelspermeter,1,4,fp,forcebyteswap);
  fwrite_lsb(&H.ypixelspermeter,1,4,fp,forcebyteswap);
  fwrite_lsb(&H.numcolors,1,4,fp,forcebyteswap);
  fwrite_lsb(&H.numimptcolors,1,4,fp,forcebyteswap);


  
  //Ok.  Now write the data. 
  int row, col;
  uint8_t rt,gt,bt;
  uint8_t zero=0;
  for (row=0;row<n;row++){
    for (col=0;col<m;col++){
      rt=(uint8_t)((r[row*n+col]-min)/(max-min)*255);
      gt=(uint8_t)((g[row*n+col]-min)/(max-min)*255);
      bt=(uint8_t)((b[row*n+col]-min)/(max-min)*255);
      fwrite_lsb(&rt,1,1,fp,forcebyteswap);
      fwrite_lsb(&gt,1,1,fp,forcebyteswap);
      fwrite_lsb(&bt,1,1,fp,forcebyteswap);
    }
    for (col=0;col<pad_bytes;col++){
      fwrite_lsb(&zero,1,1,fp,forcebyteswap);
    }
  }

  return 0;
}


/*Writes an 8 bit greyscale bmp file.
fname - file name to be written.  String.
g - each m*n where the image will be m x n pixels. Stored in ROW-MAJOR order. 
that is, g[0] is pixel (0,0).  g[1] is for pixel (0,1).  g[2] is pixel (0,2), ... , g[n-1]
is pixel (0,n-1).  g[n] is pixel (1,0), and so on. 

m,n - the dimensions of the image in pixels. 

min - a value smaller than any value in g.  This value will mean black. 
max - a value larger than any value in g.  This value will mean white. 

forcebyteswap -- if 1, this forces output to be in byte-swapped order. Shouldn't be needed, as endianness should be taken care of automatically. */ 

int write_greyscale_bmp(char* fname, double* g, int m, int n, double min, double max,int forcebyteswap){

  bitmapheader H;
  FILE* fp;
  int pad_bytes=(4-((1*n)%4))%4;
  int linewidth_bytes=1*n+pad_bytes;
  fp = fopen(fname,"wb");
  if (fp==NULL){
    fprintf(stderr,"Error opening %s for writing.\n",fname);
    return 1;
  }
  H.type[0]='B'; H.type[1]='M';
  H.filesize=14+40+256*4+linewidth_bytes*m;
  H.reserved=0;
  H.offset_to_bitmaps=14+40+4*256;
  H.bisize=40;
  H.biwidth=n;
  H.biheight=m;
  H.bicplanes=1;
  H.bitdepth=8;
  H.compression=0;
  H.compressedsize=0;
  H.xpixelspermeter=0;
  H.ypixelspermeter=0;
  H.numcolors=256;
  H.numimptcolors=0;

  
  
  //Now write the header to file.
  fwrite_lsb(&H.type,2,1,fp,forcebyteswap);
  fwrite_lsb(&H.filesize,1,4,fp,forcebyteswap);
  //Not a typo -- two reserved fields that I duplicate. 
  fwrite_lsb(&H.reserved,1,2,fp,forcebyteswap);
  fwrite_lsb(&H.reserved,1,2,fp,forcebyteswap);
  fwrite_lsb(&H.offset_to_bitmaps,1,4,fp,forcebyteswap);

  fwrite_lsb(&H.bisize,1,4,fp,forcebyteswap);
  fwrite_lsb(&H.biwidth,1,4,fp,forcebyteswap);
  fwrite_lsb(&H.biheight,1,4,fp,forcebyteswap);
  fwrite_lsb(&H.bicplanes,1,2,fp,forcebyteswap);
  fwrite_lsb(&H.bitdepth,1,2,fp,forcebyteswap);
  fwrite_lsb(&H.compression,1,4,fp,forcebyteswap);
  fwrite_lsb(&H.compressedsize,1,4,fp,forcebyteswap);
  fwrite_lsb(&H.xpixelspermeter,1,4,fp,forcebyteswap);
  fwrite_lsb(&H.ypixelspermeter,1,4,fp,forcebyteswap);
  fwrite_lsb(&H.numcolors,1,4,fp,forcebyteswap);
  fwrite_lsb(&H.numimptcolors,1,4,fp,forcebyteswap);


  //Now write the color table.
  uint8_t color=0;
  uint8_t zero=0;
  int i;
  for (i=0;i<256;i++){
    color=i;
    fwrite_lsb(&color,1,1,fp,forcebyteswap);
    fwrite_lsb(&color,1,1,fp,forcebyteswap);
    fwrite_lsb(&color,1,1,fp,forcebyteswap);
    fwrite_lsb(&zero,1,1,fp,forcebyteswap);
  }

  
  
  //Ok.  Now write the data. 
  int row, col;
  for (row=0;row<n;row++){
    for (col=0;col<m;col++){
      color=(uint8_t)((g[row*n+col]-min)/(max-min)*255);
      //color=row;
      fwrite_lsb(&color,1,1,fp,forcebyteswap);
    }
    for (col=0;col<pad_bytes;col++){
      fwrite_lsb(&zero,1,1,fp,forcebyteswap);
    }
  }

  return 0;
}


/*Writes a 2 bit monochrome bmp file.
fname - file name to be written.  String.
g -  m*n where the image will be m x n pixels. Stored in ROW-MAJOR order. 
that is, g[0] is pixel (0,0).  g[1] is for pixel (0,1).  g[2] is pixel (0,2), ... , g[n-1]
is pixel (0,n-1).  g[n] is pixel (1,0), and so on. g[i]==1 the corresponding pixel is white, otherwise it is black.   

m,n - the dimensions of the image in pixels. 

forcebyteswap -- if 1, this forces output to be in byte-swapped order. Shouldn't be needed, as endianness should be taken care of automatically. */ 


int write_monochrome_bmp(char* fname, short int* g, int m, int n,int forcebyteswap){

  uint8_t twopow[8]={1,2,4,8,16,32,64,128};
  bitmapheader H;
  FILE* fp;

  int pixel_data_bytes=n%8==0?n/8:n/8+1;
  int pad_bytes=(4-((pixel_data_bytes)%4))%4;
  printf("There are %d pixels/row so we need %d bytes/row and a pad of %d bytes.\n",n,pixel_data_bytes,pad_bytes);
  int linewidth_bytes=pixel_data_bytes+pad_bytes;
  fp = fopen(fname,"wb");
  if (fp==NULL){
    fprintf(stderr,"Error opening %s for writing.\n",fname);
    return 1;
  }
  H.type[0]='B'; H.type[1]='M';
  H.filesize=14+40+2*4+linewidth_bytes*m;
  H.reserved=0;
  H.offset_to_bitmaps=14+40+4*2;
  H.bisize=40;
  H.biwidth=n;
  H.biheight=m;
  H.bicplanes=1;
  H.bitdepth=1;
  H.compression=0;
  H.compressedsize=0;
  H.xpixelspermeter=0;
  H.ypixelspermeter=0;
  H.numcolors=2;
  H.numimptcolors=0;

  
  
  //Now write the header to file.
  fwrite_lsb(&H.type,2,1,fp,forcebyteswap);
  fwrite_lsb(&H.filesize,1,4,fp,forcebyteswap);
  //Not a typo -- two reserved fields that I duplicate. 
  fwrite_lsb(&H.reserved,1,2,fp,forcebyteswap);
  fwrite_lsb(&H.reserved,1,2,fp,forcebyteswap);
  fwrite_lsb(&H.offset_to_bitmaps,1,4,fp,forcebyteswap);

  fwrite_lsb(&H.bisize,1,4,fp,forcebyteswap);
  fwrite_lsb(&H.biwidth,1,4,fp,forcebyteswap);
  fwrite_lsb(&H.biheight,1,4,fp,forcebyteswap);
  fwrite_lsb(&H.bicplanes,1,2,fp,forcebyteswap);
  fwrite_lsb(&H.bitdepth,1,2,fp,forcebyteswap);
  fwrite_lsb(&H.compression,1,4,fp,forcebyteswap);
  fwrite_lsb(&H.compressedsize,1,4,fp,forcebyteswap);
  fwrite_lsb(&H.xpixelspermeter,1,4,fp,forcebyteswap);
  fwrite_lsb(&H.ypixelspermeter,1,4,fp,forcebyteswap);
  fwrite_lsb(&H.numcolors,1,4,fp,forcebyteswap);
  fwrite_lsb(&H.numimptcolors,1,4,fp,forcebyteswap);


  //Now write the color table.
  uint8_t zero=0;
  uint8_t full=255;
  fwrite_lsb(&zero,1,1,fp,forcebyteswap);
  fwrite_lsb(&zero,1,1,fp,forcebyteswap);
  fwrite_lsb(&zero,1,1,fp,forcebyteswap);
  fwrite_lsb(&zero,1,1,fp,forcebyteswap);

  fwrite_lsb(&full,1,1,fp,forcebyteswap);
  fwrite_lsb(&full,1,1,fp,forcebyteswap);
  fwrite_lsb(&full,1,1,fp,forcebyteswap);
  fwrite_lsb(&full,1,1,fp,forcebyteswap);
  
  
  //Ok.  Now write the data. 
  int row, col;
  uint8_t color=0;
  int localpos=0;
  for (row=0;row<n;row++){

    for (col=0;col<m;col++){
      localpos=col%8;
      if (g[row*n+col]==1){
	color=color|twopow[7-localpos];
      }
      if (localpos==7||col==m-1){
	fwrite_lsb(&color,1,1,fp,forcebyteswap);
	color=0;
      }
    }
    for (col=0;col<pad_bytes;col++){
      fwrite_lsb(&zero,1,1,fp,forcebyteswap);
    }
  }

  return 0;
}

/*Writes a greyscale ppm file.

fname - file name to be written.  String.
arr -  m*n where the image will be m x n pixels. Stored in ROW-MAJOR order. 

m,n - the dimensions of the image in pixels. 

min - a value smaller than any value in arr.  This value will mean "black". 
max - a value larger than any value in arr.  This value will mean "white". 

forcebyteswap -- if 1, this forces output to be in byte-swapped order. Shouldn't be needed, as endianness should be taken care of automatically. */ 


int write_ppm(char* fname,double* arr, int m, int n, double min, double max){


  FILE* fp=fopen(fname,"wb");
  if (fp==NULL){

    fprintf(stderr,"Can't open file %s\n",fname);
    exit(1);
  }
  fprintf(fp,"P6\n%d\n%d\n%d\n",m,n,255);

  int i,j;
  for (i=0;i<m;i++){
    for (j=0;j<n;j++){
      uint8_t c=round((arr[i*m+j]-min)/(max-min)*255);
	fwrite(&c,sizeof(c),1,fp);
	fwrite(&c,sizeof(c),1,fp);
	fwrite(&c,sizeof(c),1,fp);
    }
  }

  fclose(fp);
  return 0;
}

#endif
