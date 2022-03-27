#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <openssl/md5.h>
#include <math.h>
#include <mpi.h>
#include <time.h>
#include <unistd.h>

#define AMOUNT 12

char possible_characters[AMOUNT] = {'0','1','2','3','4','5','6','7','8','9','v','b'};


void encode_to_MD5(const char *string, char *mdString){
    unsigned char digest[16];

    MD5_CTX ctx;
    MD5_Init(&ctx);
    MD5_Update(&ctx, string, strlen(string));
    MD5_Final(digest, &ctx);
    for (int i = 0; i < 16; i++)
        sprintf(&mdString[i*2], "%02x", (unsigned int)digest[i]);
}

void generate_password(int pass, int len, char conv_pass[]){
	int i = len - 1;
	int j;
	int point;
	for (j =0 ; j < len ; j++){
		conv_pass[j] = '0';
	}
	conv_pass[len] = '\0';
	while(1){
		if (pass >0){
			point = pass % AMOUNT;
			conv_pass[i] = possible_characters[point];
			i--;
			pass = pass / AMOUNT;
		}else{
			break;
		}
	}
}

void find_password(int beginning, int ending, char *mdString, int len, int rank, int *password_found){
	char password[len];
	char temp_mdString[33];
	while(1){
		generate_password(beginning, len, password);
		encode_to_MD5(password, temp_mdString);
        
		if(!strcmp(temp_mdString, mdString)){
			printf("password is %s, rank %d\n", password, rank);
            *password_found = 1;
			break;
		}else if(beginning == ending){
			break;
		}else{
			beginning ++;
		}
	}
	return;
}

int main(int argc, char* argv[])
{
	int size, rank;
	char hostname[50];
	MPI_Init(&argc, &argv);
    char mdString[33];
    int password_length;
    
    if(argc != 3) return -1;
    sscanf(argv[1], "%s", mdString);
    sscanf(argv[2], "%d", &password_length);
    
	MPI_Comm_size(MPI_COMM_WORLD, &size);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	gethostname(hostname, 50);
	
	printf("Rank id = %d , %s \n", rank, hostname);
	
	clock_t t1,t2;
    t1 = clock();

	int beginning, ending;
    int max_password_amount = pow(AMOUNT, password_length);
    int part = max_password_amount / size;
    beginning = part * (rank);

    if (rank != size - 1){
    	ending = beginning + part -1;
	}else{
		ending = max_password_amount -1;
	}

    int password_found = 0;

	find_password(beginning, ending, mdString, password_length, rank, &password_found);
	
    t2=clock();
    float time_difference;
    time_difference = ((float)t2-(float)t1);
    float seconds = time_difference / CLOCKS_PER_SEC;

    printf("Working time = %f seconds, ranks %d \n", seconds, rank);

    if (password_found == 1){
        MPI_Abort(MPI_COMM_WORLD, EXIT_FAILURE);
    }

	MPI_Finalize();
	return 0;
}



