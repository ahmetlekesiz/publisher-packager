#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <math.h>
#include <string.h>
#include <unistd.h>

// A linked list (LL) node to store a queue entry
typedef struct Books {
    int type;
    int order;
    struct Books* next;
}book;

// The queue, front stores the front node of LL and rear stores the
// last node of LL
typedef struct Buffer {
    struct Books *front, *rear;
    int bufferType;
    int sizeOfBuffer;
    int emptySpaceInBuffer;
    int typeCounter;
	sem_t numberOfBooks;
	pthread_mutex_t bufferManipulation;
}buffer;

typedef struct PublisherArguments {
    buffer * buff;
    int publisherID;
    int numberOfBooksForEachPublisher;
}publisherArgs;

publisherArgs* newPublisherArgs(buffer *buff, int publisherID, int numberOfBooksForEachPublisher){
    publisherArgs *temp =  (publisherArgs *)malloc(sizeof(publisherArgs));
    temp->publisherID = publisherID;
    temp->buff = buff;
    temp->numberOfBooksForEachPublisher = numberOfBooksForEachPublisher;
    return temp;
}

// A utility function to create a new linked list node.
struct Books* newBookNode(int type)
{
    struct Books* temp = (struct Books*)malloc(sizeof(struct Books));
    temp->order = -1;
    temp->type = type;
    temp->next = NULL;
    return temp;
}

// A utility function to create an empty queue
struct Buffer* createBuffer(int type, int sizeOfBuffer, int numberOfBooksForEachPublisher)
{
    struct Buffer* q = (struct Buffer*)malloc(sizeof(struct Buffer));
    q->front = q->rear = NULL;
    q->bufferType = type;
    q->sizeOfBuffer = sizeOfBuffer;
    q->emptySpaceInBuffer = sizeOfBuffer;
    q->typeCounter = 0;
    sem_init (&(q->numberOfBooks),0,0);
    pthread_mutex_init(&(q->bufferManipulation),NULL);
    return q;
}

// Function to remove a key from given queue q
struct Books* deQueue(struct Buffer* q)
{
    // If queue is empty, return NULL.
    if (q->front == NULL){
        printf("Queue is empty!");
    }

    // Store previous front and move front one node ahead
    struct Books* temp = q->front;

    q->front = q->front->next;

    // If front becomes NULL, then change rear also as NULL
    if (q->front == NULL)
        q->rear = NULL;

    // Arrange buffer counter
    q->emptySpaceInBuffer++;

    return temp;
}
/*
void* packager(void* arg)
{	
	// TODO Rastgele buffer seçilecek. Eğer empty ve thread varsa beklenecek. Yoksa yeni rastgele buffer seçilecek.
	buffer* buff = (buffer*)arg;

    // If queue is empty, return NULL.
    if (buff->front == NULL){
        printf("Queue is empty!");
    }

    // Store previous front and move front one node ahead
    struct Books* temp = buff->front;

    buff->front = buff->front->next;

    // If front becomes NULL, then change rear also as NULL
    if (buff->front == NULL)
        buff->rear = NULL;

    // Arrange buffer counter
    buff->emptySpaceInBuffer++;

    return temp;
}
*/

void* publisher(void* arg)
{
	publisherArgs * currentPublisherArgs = (publisherArgs *)arg;
	buffer *buff = currentPublisherArgs->buff;
	int publisherID = currentPublisherArgs->publisherID;
    int numberOfBooks = currentPublisherArgs->numberOfBooksForEachPublisher;

    // Create books for each publisher
	int typeOfPublisher = buff->bufferType;
	book* bookArray[numberOfBooks];
	int i;
    for (i = 0; i < numberOfBooks; ++i) {
        book* newBook = newBookNode(typeOfPublisher);
        bookArray[i] = newBook;
    }

    int order;



    for (int j = 0; j < numberOfBooks; ++j) {
        book* bookToAdd = bookArray[j];
        pthread_mutex_lock (&(buff->bufferManipulation));
        //printf("start inserting buffer type: %d\n", typeOfPublisher);
        //sleep(1);
        // Put item to the related buffer type
        // Find the order of current book
        order = buff->typeCounter + 1;
        bookToAdd->order = order;
        // Check buffer size
        if(buff->emptySpaceInBuffer==0){
            printf("Publisher %d of type %d       Buffer is full. Resizing the buffer.\n", publisherID, typeOfPublisher);
            buff->emptySpaceInBuffer = buff->sizeOfBuffer;
            buff->sizeOfBuffer = buff->sizeOfBuffer*2;
        }
        // Add to buffer
        // If queue is empty, then new node is front and rear both
        if (buff->rear == NULL) {
            buff->front = buff->rear = bookToAdd;
        }
        else{
            // Add the new node at the end of queue and change rear
            buff->rear->next = bookToAdd;
            buff->rear = bookToAdd;
        }
        // Buffer counter arraignments
        buff->typeCounter++;
        buff->emptySpaceInBuffer--;
        printf("Publisher %d of type %d       Book%d_%d is published and put into the buffer %d.\n", publisherID, typeOfPublisher, typeOfPublisher, order, typeOfPublisher);
        pthread_mutex_unlock (&(buff->bufferManipulation));
        sem_post(&buff->numberOfBooks);
    }
    printf("------------------------------------------------------------------------------------\n");
    printf("Publisher %d of type %d     Finished publishing %d books. Exiting system.\n", publisherID, typeOfPublisher, numberOfBooks);
    printf("------------------------------------------------------------------------------------\n");
    pthread_exit(0);

}


// Driver Program to test anove functions
int main(int argc, char *argv[]){
    int numberOfType=2;
    int numberOfPublisherForEachType=11;
    int numberOfPublisher = numberOfType*numberOfPublisherForEachType;
    int numberOfPackager = 0;
    int numberOfBooksForEachPublisher = 17;
    int maxNumOfBooksForPackager = 5;
    int numberOfTotalBooks = numberOfBooksForEachPublisher*numberOfPublisher;
    int initialBufferSize = 7;
    int rc;
    pthread_t publisherThreads[numberOfPublisher];
    pthread_t packagerThreads[numberOfPackager];
    buffer *bufferTypes[numberOfType];

    printf("<Thread-type and ID>        <Output>\n");

    /*
    // Read Arguments
    if (argc != 10)
    {
        printf("enter 10 arguments only.");
        return 0;
    }

    if(strcmp(argv[1], "-n") != 0 || strcmp(argv[5], "-b") != 0 || strcmp(argv[7], "-s") != 0) 	{
        printf("Please enter -n for argv[1], -b for argv[5] and -s for argv[7]\n");
        return 0;
    }

    numberOfType = atoi(argv[2]);
    numberOfPublisherForEachType = atoi(argv[3]);
    numberOfPackager = atoi(argv[4]);
    numberOfBooksForEachPublisher = atoi(argv[6]);
    maxNumOfBooksForPackager = atoi(argv[8]);
    initialBufferSize = atoi(argv[9]);

    if(numberOfType == 0 || numberOfPublisherForEachType == 0 || numberOfPackager == 0 || numberOfBooksForEachPublisher == 0 || maxNumOfBooksForPackager == 0 || initialBufferSize == 0) {
        printf("Please enter numeric values for arguments 2,3,4,6,8,9\n");
        return 0;
    }
    printf("%d %d %d %d %d %d\n", numberOfType, numberOfPublisherForEachType, numberOfPackager, numberOfBooksForEachPublisher, maxNumOfBooksForPackager, initialBufferSize);
     */

    int i;
    for(i=0;i<numberOfType;i++){
        buffer * newBuffer = createBuffer(i,initialBufferSize, numberOfBooksForEachPublisher);
        bufferTypes[i] = (struct Buffer*)newBuffer;
    }

    int k;
    for (k = 0; k < numberOfType; k++) {
        for(i=0;i<numberOfPublisherForEachType;i++){
            publisherArgs *currentPublisherArgs = newPublisherArgs(bufferTypes[k], i, numberOfBooksForEachPublisher);
            rc = pthread_create(&publisherThreads[i], NULL, publisher, (void*) currentPublisherArgs);
            if(rc){
                printf("ERROR; return code from pthread_create() is %d\n", rc);
                exit(-1);
            }
        }
    }


    /*
    for(i=0;i<numberOfPackager;i++){
        rc=pthread_create(&packagerThreads[i], NULL, packager, (void*) i);
        if(rc){
            printf("ERROR; return code from pthread_create() is %d\n", rc);
            exit(-1);
        }
    }*/

    pthread_exit(0);

/*
    enQueue(q, 10);
    enQueue(q, 20);
    deQueue(q);
    deQueue(q);
    enQueue(q, 30);
    enQueue(q, 40);
    enQueue(q, 50);
    deQueue(q);
    printf("Buffer Front : %d \n", q->front->key);
    printf("Buffer Rear : %d", q->rear->key);
    return 0;*/
}




