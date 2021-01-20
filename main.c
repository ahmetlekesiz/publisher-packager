#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <math.h>
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
	sem_t numberOfEmptySpace;
	pthread_mutex_t bufferManipulation;
}buffer;

/*struct publisherArgs{
	buffer* buff;	
};*/

// A utility function to create a new linked list node.
struct Books* newNode(int type)
{
    struct Books* temp = (struct Books*)malloc(sizeof(struct Books));
    temp->order = -1;
    temp->type = type;
    temp->next = NULL;
    return temp;
}

// A utility function to create an empty queue
struct Buffer* createBuffer(int type, int sizeOfBuffer)
{
    struct Buffer* q = (struct Buffer*)malloc(sizeof(struct Buffer));
    q->front = q->rear = NULL;
    q->bufferType = type;
    q->sizeOfBuffer = sizeOfBuffer;
    q->emptySpaceInBuffer = sizeOfBuffer;
    q->typeCounter = 0;
    sem_init (&(q->numberOfEmptySpace),0,sizeOfBuffer);
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

void* packager(void* arg)
{	
	// TODO Rastgele buffer se�ilecek. E�er empty ve thread varsa beklenecek. Yoksa yeni rastgele buffer se�ilecek.
	buffer* buff = (buffer*)arg;
    // If queue is empty, return NULL.
    if (buff->front == NULL){
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

void* publisher(void* arg)
{	
	buffer* buff = (buffer*)arg;
	if(buff->emptySpaceInBuffer==0){
		buff->emptySpaceInBuffer = buff->sizeOfBuffer;
		buff->sizeOfBuffer = buff->sizeOfBuffer*2;
	}
    // Create a new LL node
    book* temp = newNode(buff->bufferType);

    // Order'�n� bul
    int order = buff->typeCounter + 1;
    temp->order = order;
	sem_wait(&buff->numberOfEmptySpace);
	pthread_mutex_lock (&(buff->bufferManipulation));
    // If queue is empty, then new node is front and rear both
    if (buff->rear == NULL) {
        buff->front = buff->rear = temp;
    }
	else{
    // Add the new node at the end of queue and change rear
	    buff->rear->next = temp;
	    buff->rear = temp;
	}

    // Buffer counter arragnments
    buff->typeCounter++;
    buff->emptySpaceInBuffer--;
    pthread_mutex_unlock (&(buff->bufferManipulation));
	sem_post(&buff->numberOfBooks);
    // TODO Bo� yer kalmad�ysa, size 'i  2 ile �arp, gerekli i�lemleri yap.
}

// Driver Program to test anove functions
int main()
{	int numberOfType=2;
    int numberOfPublisherForEachType=3;
    int numberOfPublisher = numberOfType*numberOfPublisherForEachType;
    int numberOfPackager = 10;
    int numberOfBooksForEachPublisher = 3;
    int maxNumOfBooksForPackager = 5;
    int numberOfTotalBooks = numberOfBooksForEachPublisher*numberOfPublisher;
    int initialBufferSize = 7;
    int rc;
    pthread_t publisherThreads[numberOfPublisher];
    pthread_t packagerThreads[numberOfPackager];
    buffer *bufferTypes[numberOfType];
    long i;
    for(i=0;i<numberOfType;i++){
        buffer * newBuffer = createBuffer(i,initialBufferSize);
        bufferTypes[i] = (struct Buffer*)newBuffer;
    }
    for(i=0;i<numberOfPublisher;i++){
        rc = pthread_create(&publisherThreads[i], NULL, publisher, (void*) bufferTypes);
        if(rc){
            printf("ERROR; return code from pthread_create() is %d\n", rc);
            exit(-1);
        }
    }

    for(i=0;i<numberOfPackager;i++){
        rc=pthread_create(&packagerThreads[i], NULL, packager, (void*) i);
        if(rc){
            printf("ERROR; return code from pthread_create() is %d\n", rc);
            exit(-1);
        }
    }

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




