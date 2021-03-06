#include <cstdio>
#include <cstdlib>
#include <mpi.h>
#include <set>
#include <vector>

using namespace std;
typedef struct {
		long bound;
		long actualCost;
		set<long> yDone;
		set<long> xDone;
		vector <long> assignment;
} Node;

void sendDataSet(long **inputArray, long size, int dest, int tag, MPI_Comm comm) {
	int totalCount = (size * size);
	long *buffer = (long *) malloc(sizeof(long) * (totalCount));
	//buffer[0] = size;
	int count = 0;
	for(int i = 0 ; i < size; i++) {
		for (int j = 0 ; j < size; j++) {
			buffer[count++] = inputArray[i][j];
		}
	}
	long *sizeBuff = &size;
	MPI_Send(sizeBuff, 1 , MPI_INT, dest, tag, comm);
	MPI_Send(buffer,totalCount, MPI_LONG, dest, tag, comm);
	free(buffer);
}

void recvDataSet(long ***inputArray, long *size, int src, int tag, MPI_Comm comm) {
	MPI_Status status;

	MPI_Recv(size, 1, MPI_LONG, src, tag, comm, &status);
	
	int totalSize = (*size) * (*size);

	printf("Total size : %d\n", totalSize );

	long *buffer = (long *)malloc(sizeof(long) * (totalSize));

	int count = 0;

	MPI_Recv(buffer, totalSize, MPI_LONG, src, tag, comm, &status);

	*inputArray = (long **) malloc(sizeof(long *) * (*size));
	printf("Ok\n");
	for(int i = 0; i < (*size) ; i++) {
			(*inputArray)[i] = (long *)malloc(sizeof(long) * (*size));
			for(int j = 0 ; j < (*size) ;j++) {
				(*inputArray)[i][j] = buffer[count++];
				printf("%ld\n", (*inputArray)[i][j] );
			}
	}
	free(buffer);
}


void sendNodeMPI(Node *node, int dest, int tag, MPI_Comm comm) {
	long *buffer =  (long *) malloc(sizeof(long) * 5);
	buffer[0] = node->bound;
	buffer[1] = node->actualCost;
	buffer[2] = node->yDone.size();
	buffer[3] = node->xDone.size();
	buffer[4] = node->assignment.size();
	printf("Sending values : %ld, %ld, %ld ,%ld , %ld\n", buffer[0], buffer[1], buffer[2], buffer[3], buffer[4] );
	MPI_Send(buffer,5, MPI_LONG, dest, tag, comm);
	int totalDataSize = buffer[2] + buffer[3] + buffer[4];
	long *buffer2 =  (long *) malloc(sizeof(long) * totalDataSize);
	int count = 0;
	for(auto data : node->yDone) {
		buffer2[count++] = data;
	}
	for(auto data : node->xDone) {
		buffer2[count++] = data;
	}

	for (auto data : node->assignment) {
		buffer2[count++] = data;
 	}

 	MPI_Send(buffer2, totalDataSize, MPI_LONG, dest, tag, comm);

	free(buffer);
	free(buffer2);
}

void recvNodeMPI(Node *node, int src, int tag, MPI_Comm comm) {
	long *buffer = (long *)malloc(sizeof(long) * 5);
	MPI_Status status;
	MPI_Recv(buffer, 5 , MPI_LONG, src , tag, comm, &status);
	printf("Bound value received : %ld\n", buffer[0]);
	printf("ActualCost value received : %ld\n", buffer[1]);
	printf("yDone size value received : %ld\n", buffer[2]);
	printf("xDone size value received : %ld\n", buffer[3]);
	printf("Assignment value received : %ld\n", buffer[4]);

	node->bound = buffer[0];
	node->actualCost = buffer[1];
	int receiveCount = buffer[2] + buffer[3] + buffer[4];
	long *buffer2 =  (long *) malloc(sizeof(long) * receiveCount);
	MPI_Recv(buffer2, receiveCount , MPI_LONG, src , tag, comm, &status);
	int count = 0;
	for(int i = 0; i < buffer[2]; i++) {
		node->yDone.insert(buffer2[count++]);
	}

	for(int i = 0; i < buffer[3]; i++) {
		node->xDone.insert(buffer2[count++]);
	}

	for(int i = 0; i < buffer[4]; i++) {
		node->assignment.push_back(buffer2[count++]);
	}

	free(buffer);
	free(buffer2);
}

int main(int argc, char **argv) {

	int rank, size, i;
   	int buffer[10];

   	// sendBuff is used to send bound, actualCost , size of yDone, size of xDone, size of assignment
   	//long sendBuff[5]; 

    MPI_Status status;
 
    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    if (size < 2)
    {
        printf("Please run with two processes.\n");fflush(stdout);
        MPI_Finalize();
        return 0;
    }
    if (rank == 0)
    {
        // for (i=0; i<10; i++)
        //     buffer[i] = i;
        // MPI_Send(buffer, 10, MPI_INT, 1, 123, MPI_COMM_WORLD);

    	Node * node = new Node();
    	node->bound = 10;
    	node->actualCost = 120;
    	
    	//node->yDone.insert(12);
    	//node->yDone.insert(13);
    	//node->yDone.insert(14);

    	//node->xDone.insert(15);
    	//node->xDone.insert(16);

    	//node->assignment.push_back(17);
    	//->assignment.push_back(18);
    	printf("Sending data \n");
    	sendNodeMPI(node, 1 , 0, MPI_COMM_WORLD);


    	FILE *inputFILE = fopen(argv[1],"r");
		char *line = NULL;
		size_t len ;
		ssize_t read;
		int noOfJob = 0;
		
		if( inputFILE ==  NULL)
			exit(-2);

		fscanf(inputFILE, "%d", &noOfJob);
		long **inputArray;
		long limit;
		limit = noOfJob;
		inputArray = (long **) malloc(noOfJob * sizeof(long *));
		
		int temp = 0;
		int i = 0;
		int j = 0;

		for(i = 0; i < noOfJob ; i++) {
			inputArray[i] = (long *)malloc(noOfJob * sizeof(long));
			for(j = 0 ; j < noOfJob ;j++) {
				fscanf(inputFILE, "%ld ", &inputArray[i][j]);
				printf("%ld ", inputArray[i][j] );
			}
			printf("\n"); 
		}
 		
 		printf("Sending data \n");
 		sendDataSet(inputArray, noOfJob, 1, 0, MPI_COMM_WORLD);

    }
    if (rank == 1)
    {	Node *  node = new Node();
    	recvNodeMPI(node, 0,0,MPI_COMM_WORLD);
    	for(auto data : node->yDone) {
    		printf("yDone value : %ld\n", data);
    	}

    	for(auto data : node->xDone) {
    		printf("xDone value : %ld\n", data);
    	}

    	for(auto data : node->assignment) {
    		printf("assignment value : %ld\n", data);
    	}

    	long **inputArray;
		long limit;

		recvDataSet(&inputArray, &limit, 0, 0, MPI_COMM_WORLD);

		for(int i = 0; i < limit ; i++) {
			for(int j = 0 ; j < limit ;j++) {
				printf("%ld ", inputArray[i][j] );
			}
			printf("\n"); 
		}

        // for (i=0; i<10; i++)
        //     buffer[i] = -1;
        // MPI_Recv(buffer, 10, MPI_INT, 0, 123, MPI_COMM_WORLD, &status);
        // for (i=0; i<10; i++)
        // {
        //     if (buffer[i] != i)
        //         printf("Error: buffer[%d] = %d but is expected to be %d\n", i, buffer[i], i);
        //     else
        //     	printf("%d received data \n", buffer[i]);
        // }
        // fflush(stdout);
    }
    MPI_Finalize();
    return 0;
 }