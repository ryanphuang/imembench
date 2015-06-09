 #include <stdio.h>
 #include <stdlib.h>
 #include <string.h>
 
  float Init_Data[1000][1000];
  int get_value(char* line,int place,char* buffer)
  {
	  int   i;
	  char  return_v[100];
      int   start_place;
      char  *ptr1,*ptr2;
      start_place=place-1;
      //printf("start_place=%d\n",start_place);
      ptr1=line;
        if(start_place!=0)
        {
          for(i=0;i<start_place;i++)
            ptr1=strchr(ptr1,' ')+1;
	    }
        ptr2=strchr(ptr1,' ');
		strncpy(buffer,ptr1,ptr2-ptr1);
        buffer[ptr2-ptr1]='\0';
		  return 1;
  }
  int Trim_table(char* line)
  {
	  int i=0;
      while (line[i]==' ')
	     i++;
      return i;
  }

  int main(int argc, char* argv[])
  {
   if (argc!=3)
   {
	 printf("%s Baselin_File EnlargeNum\n",argv[0]);
       return -1;
   }
   int enlarger=atoi(argv[2]);
   FILE *fp;
   fp=fopen(argv[1],"r+");
   if (fp==NULL)
    return 0;
   char *line=NULL;
   size_t  n;
   int count=0;
   char rr[100];
   while (getline(&line, &n, fp) != -1) 
   {  
	  int c=0; 
      int p=Trim_table(line);
      //get_value(line,1,(char*)rr);               
      //Init_Data[count][c]=atoi(rr); 
	  for (int i=0;i<9;i++)
      {
	      get_value(line+p,i+2,(char*)rr);
		  Init_Data[count][i]=atof(rr);
		  //printf("%f ",Init_Data[count][i]);
	  }
	 // printf("\n");
	  count++;
	//  printf("ccc=%d\n",count);
   }
   int nn=1;
   float ww[10];
   for(int i=0;i<count;i++)
   {
      for (int j=0;j<enlarger;j++)
	  {
	     for (int k=0;k<9;k++)
		   ww[k]=(float)random()/RAND_MAX;  
	     printf("%d %f %f %f %f %f %f %f %f %f\n",nn,Init_Data[i][0]+ww[0],Init_Data[i][1]+ww[1],Init_Data[i][2]+ww[2],Init_Data[i][3]+ww[3],Init_Data[i][4]+ww[4],Init_Data[i][5]+ww[5],Init_Data[i][6]+ww[6],Init_Data[i][7]+ww[7],Init_Data[i][8]+ww[8]);
		 nn++;
	  } 	  
   
   }	   
 }

