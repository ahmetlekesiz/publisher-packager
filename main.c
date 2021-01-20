#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <math.h>
#include <string.h>
#include <unistd.h>

int bufferArraySize = 3;
int totalBooksToProcess;
sem_t bookCounter;

unsigned int
randomNumber(unsigned int min, unsigned int max)
{
    double x= (double)rand()/RAND_MAX;

    return (max - min +1)*x+ min;
}

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
    int totalBooksUntilNow;
	sem_t numberOfBooks;
	pthread_mutex_t bufferManipulation;
	struct Buffer* nextBuffer;
}buffer;

typedef struct PublisherArguments {
    buffer * buff;
    int publisherID;
    int numberOfBooksForEachPublisher;
}publisherArgs;

typedef struct PackagerArguments {
    buffer * bufferHead;
    int packagerID;
    int numberOfTotalBuffer;
    int maxNumOfBooksForPackager;
}packagerArgs;

publisherArgs* newPublisherArgs(buffer *buff, int publisherID, int numberOfBooksForEachPublisher, int maxNumOfBooksForPackager){
    publisherArgs *temp =  (publisherArgs *)malloc(sizeof(publisherArgs));
    temp->publisherID = publisherID;
    temp->buff = buff;
    temp->numberOfBooksForEachPublisher = numberOfBooksForEachPublisher;
    temp->numberOfBooksForEachPublisher = maxNumOfBooksForPackager;
    return temp;
}

publisherArgs* newPackagerArgs(buffer *bufferHead, int numberOfTotalBuffer, int packagerID){
    packagerArgs *temp =  (packagerArgs *)malloc(sizeof(packagerArgs));
    temp->bufferHead = bufferHead;
    temp->numberOfTotalBuffer = numberOfTotalBuffer;
    temp->packagerID = packagerID;
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
    q->totalBooksUntilNow = 0;
    q->nextBuffer = NULL;
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

buffer* selectRandomBuffer(int numberOfTotalBuffer, buffer *buff){
    int notEmptyBuffers[numberOfTotalBuffer];
    int l;
    for (l = 0; l < numberOfTotalBuffer; ++l) {
        notEmptyBuffers[l] = -1;
    }
    // Fill notEmptyBuffer Array
    int index = 0;
    buffer *temp = buff;
    int totalBookUntilNow = 0;
    for (int i = 0; i < numberOfTotalBuffer ; ++i) {
        // Find Total Number of Book in All Buffers.
        totalBookUntilNow = totalBookUntilNow + temp->totalBooksUntilNow;
        // Find non empty buffer.
        if (temp->emptySpaceInBuffer != temp->sizeOfBuffer){
            notEmptyBuffers[index] = i;
            index++;
        }
        temp = temp->nextBuffer;
    }

    if(notEmptyBuffers[0] == -1 && totalBookUntilNow == totalBooksToProcess){
        printf("All buffers are empty and All books are already packaged!");
        pthread_exit(0);
    }else if(notEmptyBuffers[0] == -1){
        sem_wait(&bookCounter);
    }

    int randomBufferIndex = notEmptyBuffers[randomNumber(0, index-1)];
    int counter = 0;
    buffer *bufferToTake = buff;
    while (counter != randomBufferIndex){
        bufferToTake = bufferToTake->nextBuffer;
        counter++;
    }
    return bufferToTake;
}

void* packager(void* arg)
{	
	packagerArgs * currentPackagerArgs = (packagerArgs*)arg;
	buffer *buff = currentPackagerArgs->bufferHead->nextBuffer;
    int numberOfTotalBuffer = currentPackagerArgs->numberOfTotalBuffer;
    int maxNumOfBooksForPackager = currentPackagerArgs->maxNumOfBooksForPackager;
    int packageCounter = 0;
    int packagerID = currentPackagerArgs->packagerID;
    book *packageHead = newBookNode(-1);
    while(1){
        // Select random buffer
        buffer *bufferToTake = selectRandomBuffer(numberOfTotalBuffer, buff);
        // Lock selected buffer
        pthread_mutex_lock(&(bufferToTake->bufferManipulation));
        // Remove Book
        book* bookToTake = deQueue(bufferToTake);
        // Check if package is full
        if(packageCounter == maxNumOfBooksForPackager){
            printf("Packager %d     Finished preparing one package. The package contains: ", packagerID);
            // Print package context
            book *packageIter = packageHead->next;
            while(packageIter != NULL){
                int typeOfBook = packageIter->type;
                int orderOfBook = packageIter->order;
                printf("Book%d_%d, ", typeOfBook, orderOfBook);
                book *packageToFree = packageIter;
                packageIter = packageIter->next;
                free(packageToFree);
            }
            printf("\n");
            // Make the package empty
            packageHead->next = NULL;
        }
        // Find empty place in the package
        book *temp = packageHead;
        while(temp->next != NULL){
            temp = temp->next;
        }
        // Insert new book to package
        temp->next = bookToTake;
        // Unlock buffer
        pthread_mutex_unlock (&(buff->bufferManipulation));
    }
}


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
        buff->totalBooksUntilNow++;
        buff->typeCounter++;
        buff->emptySpaceInBuffer--;
        printf("Publisher %d of type %d       Book%d_%d is published and put into the buffer %d.\n", publisherID, typeOfPublisher, typeOfPublisher, order, typeOfPublisher);
        pthread_mutex_unlock (&(buff->bufferManipulation));
        sem_post(&buff->numberOfBooks);
        sem_post(&bookCounter);
    }
    printf("------------------------------------------------------------------------------------\n");
    printf("Publisher %d of type %d     Finished publishing %d books. Exiting system.\n", publisherID, typeOfPublisher, numberOfBooks);
    printf("------------------------------------------------------------------------------------\n");
    pthread_exit(0);

}

// Driver Program to test anove functions
int main(int argc, char *argv[]){
    int numberOfType=3;
    int numberOfPublisherForEachType=2;
    int numberOfPublisher = numberOfType*numberOfPublisherForEachType;
    int numberOfPackager = 2;
    int numberOfBooksForEachPublisher = 4;
    int maxNumOfBooksForPackager = 3;
    int numberOfTotalBooks = numberOfBooksForEachPublisher*numberOfPublisher;
    int initialBufferSize = 7;
    int rc;
    pthread_t publisherThreads[numberOfPublisher];
    pthread_t packagerThreads[numberOfPackager];
    buffer *bufferTypes[numberOfType];
    bufferArraySize = numberOfType;
    // Set to Global Variable
    totalBooksToProcess = numberOfTotalBooks;
    printf("<Thread-type and ID>        <Output>\n");
    sem_init(&bookCounter, 0,0);
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

    buffer *headBuffer = createBuffer(-1, initialBufferSize, numberOfBooksForEachPublisher);

    int i;
    for(i=0;i<numberOfType;i++){
        buffer * newBuffer = createBuffer(i,initialBufferSize, numberOfBooksForEachPublisher);
        // Add to Linked List
        // boş yeri bul
        buffer *temp = headBuffer;
        while (temp->nextBuffer != NULL){
            temp = temp->nextBuffer;
        }
        temp->nextBuffer = newBuffer;
        // Add to Array
        bufferTypes[i] = (struct Buffer*)newBuffer;
    }

    int k;
    for (k = 0; k < numberOfType; k++) {
        for(i=0;i<numberOfPublisherForEachType;i++){
            publisherArgs *currentPublisherArgs = newPublisherArgs(bufferTypes[k], i, numberOfBooksForEachPublisher, maxNumOfBooksForPackager);
            rc = pthread_create(&publisherThreads[i], NULL, publisher, (void*) currentPublisherArgs);
            if(rc){
                printf("ERROR; return code from pthread_create() is %d\n", rc);
                exit(-1);
            }
        }
    }

    int packagerID;
    for(i=0;i<numberOfPackager;i++){
        packagerID = i + 1;
        publisherArgs *currentPackagerArgs = newPackagerArgs(headBuffer, numberOfType, packagerID);
        rc=pthread_create(&packagerThreads[i], NULL, packager, (void*) currentPackagerArgs);
        if(rc){
            printf("ERROR; return code from pthread_create() is %d\n", rc);
            exit(-1);
        }
    }

    pthread_exit(0);

   // return 0;
}




