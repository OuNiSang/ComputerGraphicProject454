// priority.cpp
//
// Priority queue, implemented as a heap

#include <cstdio>
#include "priority.h"


template<class T>
void priority_queue<T>::heapify( int i )

{
  int min = i;
  int left, right;

  while (true) {

    left = i << 1;
    right = left + 1;

    if (left <= n && A[left].priority > A[min].priority)
      min = left;
    if (right <= n && A[right].priority > A[min].priority)
      min = right;

    if (min == i)
      break;

    PQnode<T> temp = A[i];
    A[i] = A[min];
    A[min] = temp;

    i = min;
  }
}


template<class T>
void priority_queue<T>::add( T data, float priority )

{
  int i, parent;

  if (n >= size-1) {
    PQnode<T> *B = new PQnode<T>[ size << 1 ];
    for (i=1; i<size; i++)
      B[i] = A[i];
#ifndef WIN32
    delete [] A;
#endif
    A = B;
    size = size << 1;
  }

  n++;
  A[n].priority = priority;
  A[n].data = data;

  i = n;
  parent = i>>1;

  while (i > 1 && A[i].priority > A[parent].priority) {
    PQnode<T> temp = A[i];
    A[i] = A[parent];
    A[parent] = temp;
    i = parent;
    parent = i>>1;
  }
}


template<class T>
void priority_queue<T>::append( T data, float priority )

{
  int i;

  if (n >= size-1) {
    PQnode<T> *B = new PQnode<T>[ size << 1 ];
    for (i=1; i<size; i++)
      B[i] = A[i];
#ifndef WIN32
    delete [] A;
#endif
    A = B;
    size = size << 1;
  }

  n++;
  A[n].priority = priority;
  A[n].data = data;
}


#if 0

main()

{ priority_queue<int> pq;

  pq.add( 1, 9 );
  pq.add( 5, 5 );
  pq.add( 3, 7 );
  pq.add( 4, 6 );
  pq.add( 2, 8 );

  printf( "Queue: " );  // should be 1 2 3 4 5
  while (!pq.empty())
    printf( "%d ", pq.remove_max() );
  putchar('\n');

  pq.add( 1, 1 );
  pq.add( 2, 2 );
  pq.add( 5, 5 );
  pq.add( 3, 3 );
  pq.add( 4, 4 );

  printf( "Queue: " );  // should be 5 4 3 2 1
  while (!pq.empty())
    printf( "%d ", pq.remove_max() );
  putchar('\n');
}

#endif
