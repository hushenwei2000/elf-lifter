//extern int a;
// extern int func2();
/*
int func(int a, int b, int c){
	return a+b;
}*/

// int func2(double a, double b, double c){
// 	return 1;
// }

/*
int main(int argc, int* argv[]){
	int i,j,ret=0;
	int p[4] = {1,2,3,4};
	for(i=0;i<=3;i++) {
		ret += p[i];
	}
	return ret;

int i=		1;
int f=		0;
int func(int x) {
	return x;
}
int main(){
	int k;
	int m = 1;
	//const char str[1000] = "This is a const value"
	//arr[10]='a';
	k = f+i;
  m++;
	printf("hello world\n");
	return 0;
}

int main() {

	dfs(0);
}
int main(){
int i=0;
int a=1,b=1;

int c;
for(i=3;i<8000;i++) {
c=a+b;
a=b;
b=c;
}
return c;
}
*/
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#define SIZE 200
double mysecond() {
	return 1000.0;
}

int main(int argc,char** argv)
{

        int i = 0;
        int size = 0;
        int64_t* a = (int64_t*)malloc (SIZE*sizeof(int64_t));
        int64_t* b = (int64_t*)malloc (SIZE*sizeof(int64_t));
        int64_t* c = (int64_t*)malloc (SIZE*sizeof(int64_t));
        double start = 0.0, end = 0.0;
        /*if(argc != 2){
                printf("incorrect input, please input number of elements\n");
                exit(0);
        }


        size = atoi (argv[1]);
        */
        size = SIZE;
        printf("Element number per array = %dM, Size = %d MB\n", size/1024/1024, size/1024/1024*8);



        // init 
        for (i = 0; i < size; i++){
                a[i]    = 0;
                b[i] = rand();
                c[i] = rand()%size;

        }

        start = mysecond();
        for (i = 0; i < size; i++)
                a[i]    = b[c[i]];

        end = mysecond();

        printf("The time cost is %f\n", end-start);



        return 0;


}