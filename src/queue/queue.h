// This is the header file for declaring the queue struct

#include <stdio.h>    // Usually nice to include
#include <stdlib.h>

struct queue {
   int array;   // Needs to be to non-zero?
   int pri;
   int num_elem;
   int array_size;
};


// Declare the functions
//struct queue queue;
int queue_init(struct queue *queue);
int queue_push(struct queue *queue, int pri);
int queue_pop(struct queue *queue, int *pri_ptr);
int queue_destroy(struct queue *queue);


// Define the functions

int queue_init(struct queue *queue) {
   // Do nothing for array, pri and ptr_pri as they are empty

   queue[0].num_elem   = 0;
   queue[0].array_size = 0;
   
   return 0;
}

int queue_push(struct queue *queue, int pri){
   // Dynamically expanding table

   if (queue[0].array_size == 0){
      // Allocate array with one slot:
      queue[0].array = pri; 
      queue[0].array_size = 1;
   }
   else if (queue[0].num_elem == queue[0].array_size){
      //printf("The queue has array size: %x before inserting\n",queue[0].array_size);
      //printf("The queue has num_elem: %x before inserting\n",queue[0].num_elem);
      int temp_array[2 * queue[0].array_size];

      for (int i = 0; i < queue[0].array_size; i++){
	 temp_array[i] = queue[i].array;
      }
      queue[2 * queue[0].array_size].array = 0;
      for (int i = 0; i < queue[0].array_size; i++){
	 queue[i].array = temp_array[i];
      }
      queue[0].array_size = 2 * queue[0].array_size;

      //printf("Trying to insert: %x\n",pri);
      //printf("Into index: %x\n",queue[0].num_elem);

      queue[queue[0].num_elem].array = pri;
   }
   else{
      queue[queue[0].num_elem].array = pri;
   }

   queue[0].num_elem = queue[0].num_elem + 1;

   //printf("The queue has array size: %x after inserting\n",queue[0].array_size);
   //printf("The queue has num_elem: %x after inserting\n",queue[0].num_elem);

   //for (int i = 0; i < queue[0].num_elem; i++){
	 //printf("The queue consists of values: %x\n",queue[i].array);
   //}

   return 0;
}


int queue_pop(struct queue *queue, int *pri_ptr){
   // Pop the item with the maximum priority

   int index = 0;
   
   if (queue[0].num_elem == 0){
      return -1;  //Underflow
   }

   for (int i = 1; i < queue[0].num_elem; i++){
      if (queue[i].array > queue[index].array){
         index = i;
      }
   }

   //printf("Located largest element at index %x\n",index);

   // BEFORE the popping, save the popped value
   *pri_ptr = queue[index].array;

   //printf("We are trying to pop value: %x\n", queue[index].array);
   //printf("pri_ptr gets the value: %x\n", *pri_ptr);

   // Do the popping
   for (int i = index; i < queue[0].array_size; i++){
      queue[i].array = queue[i + 1].array;
   }

   queue[0].num_elem = queue[0].num_elem - 1;
   queue[0].array_size = queue[0].array_size - 1;

   //printf("After the popping: \n");
   //for (int i = 0; i < queue[0].num_elem; i++){
       //printf("The queue consists of values: %x\n",queue[i].array);
   //}
   return 0;
}




int queue_destroy(struct queue *queue){
   // Destroy the queue
   // Is this necessary for a dynamic array, where
   // we haven't used malloc?

   return 0;
}



