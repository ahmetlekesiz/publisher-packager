
// A C program to demonstrate linked list based implementation of queue
#include <stdio.h>
#include <stdlib.h>

// A linked list (LL) node to store a queue entry
typedef struct Books {
    int type;
    int order;
    struct Books* next;
}book;

// The queue, front stores the front node of LL and rear stores the
// last node of LL
struct Buffer {
    struct Books *front, *rear;
    int bufferType;
    int sizeOfBuffer;
    int emptySpaceInBuffer;
    int typeCounter;
    struct Buffer *next;
};

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
    return q;
}

// The function to add a key k to q
void enQueue(struct Buffer* q, int k)
{
    // Create a new LL node
    struct Books* temp = newNode(k);

    // Order'ını bul
    int order = q->typeCounter + 1;
    temp->order = order;

    // If queue is empty, then new node is front and rear both
    if (q->rear == NULL) {
        q->front = q->rear = temp;
        return;
    }

    // Add the new node at the end of queue and change rear
    q->rear->next = temp;
    q->rear = temp;

    // Buffer counter arragnments
    q->typeCounter++;
    q->emptySpaceInBuffer--;

    // TODO Boş yer kalmadıysa, size 'i  2 ile çarp, gerekli işlemleri yap.
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

// Driver Program to test anove functions
int main()
{
    struct Buffer* q = createBuffer();
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
    return 0;
}
