// This is the header file for declaring the queue struct
// and its functions

struct queue {
   int array;
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
void print_stuff(struct queue *queue);

// Define the functions

int queue_init(struct queue *queue) {
   // Do nothing for array, pri and ptr_pri as
   // they are and should be empty

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
      // Create temp_array of double size and copy the
      // values from the original array to this array.
      int temp_array[2 * queue[0].array_size];

      for (int i = 0; i < queue[0].array_size; i++){
	     temp_array[i] = queue[i].array;
       }

      // Expand the queue array and load the values
      // saved above into the array.
      queue[2 * queue[0].array_size].array = 0;
      for (int i = 0; i < queue[0].array_size; i++){
	     queue[i].array = temp_array[i];
      }

      // Update the new properties of the queue array
      // and insert the new element.
      queue[0].array_size = 2 * queue[0].array_size;
      queue[queue[0].num_elem].array = pri;
   }
   else{
      // The queue array is artificially filled with zeroes
      // after expansion, and num_elem != array_size is handled here
      queue[queue[0].num_elem].array = pri;
   }
   // A push is always performed, so num_elem++;
   queue[0].num_elem = queue[0].num_elem + 1;

   return 0;
}


int queue_pop(struct queue *queue, int *pri_ptr){
   // Pop the item with the maximum priority

   // Checking for underflow
   if (queue[0].num_elem == 0){
      return -1;
   }

   // Find the index with the largest value
   int index = 0;
   for (int i = 1; i < queue[0].num_elem; i++){
      if (queue[i].array > queue[index].array){
         index = i;
      }
   }

   // Before popping, save the popped value
   *pri_ptr = queue[index].array;

   // Do the popping
   for (int i = index; i < queue[0].array_size; i++){
      queue[i].array = queue[i + 1].array;
   }

   // Update the properties of the queue
   queue[0].num_elem = queue[0].num_elem - 1;
   queue[0].array_size = queue[0].array_size - 1;

   return 0;
}




int queue_destroy(struct queue *queue){
   // Destroy the queue
  
   // Is this necessary for a dynamic array, where
   // we haven't used malloc?

   return 0;
}


void print_stuff(struct queue *queue){
  // Print information about the queue

  for (int i = 0; i < queue[0].num_elem; i++){
      printf("The queue consists of values: %x\n",queue[i].array);
  }
  printf("The queue has array_size: %x\n",queue[0].array_size);
}
