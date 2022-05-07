/* priority.h
 *
 * new PQnode()                  create a priority queue node
 * new PQnode( data, priority )  create a node with data and priority
 *
 * new priority_queue()       create a priority queue
 * pq.add( data, priority )   add data with priority to queue
 * pq.remove_max()            remove & return maximum-priority data
 * pq.max()                   return maximum-priority data
 * pq.empty()                 1 if queue is empty, 0 otherwise
 */


#ifndef PRIORITYH
#define PRIORITYH


template<class T> class priority_queue;


template<class T>
class PQnode {
  friend class priority_queue<T>;
  T data;
  float priority;
};


template<class T>
class priority_queue {

  PQnode<T> *A;

  void heapify( int i );

 public:

  int n;
  int size;

  priority_queue()
    { size = 100; n = 0; A = new PQnode<T>[size]; }
  
  ~priority_queue()
    { delete [] A; }

  int empty()
    { return (n == 0); }

  void makeEmpty()
    { n = 0; }

  float max_priority()
    { return A[1].priority; }
  
  T remove_max() {

    T temp = A[1].data;
    n--;
    if (n > 0) {
      A[1] = A[n+1];
      heapify(1);
    }
    return temp;
  }

  void buildHeap() {
    for (int i=(n>>1); i>0; i--)
      heapify( i );
  }

  void add( T data, float priority );
  void append( T data, float priority );
};

#endif
