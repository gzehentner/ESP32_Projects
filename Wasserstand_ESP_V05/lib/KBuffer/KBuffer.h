/*
 * KBuffer.h
 *
 * Generic KBuffer
 * Version 1: simple float values, no template
 * Version 2: all data types for elements of the buffer are supported
 *
 *  Created on: 07.06.2020
 *      Author: Kai
 */

#ifndef KBuffer_H_
#define KBuffer_H_

// Debug printing
#define PRINT_BUF_S(s,v) Serial.print(s); Serial.println(v);

#include "Arduino.h"

template <typename TEl>
class KBuffer
{
public:
	KBuffer(int KBufferSize);
	bool add(TEl Value, bool OverWrite);		// adds an element at the end / top of the queue
	bool get(TEl *Element, long int Iterator);	// gets an arbitrary element
	bool pop(TEl *Element);						// gets and removes last element (at the end of the queue)
	bool popFront(TEl *Element);				// gets and removes first element (at the front of the queue)
	bool isEmpty();
	bool getFirst(TEl *Element);				// gets first element (at the front of the queue)
	bool getLast(TEl *Element);					// gets last element (at the end of the queue)
	bool getNext(TEl *Element);					// gets  element after the last one that has been 
	long int GetNoOfElements();                 // get the current number of elements in the buffer
	// Iterator functions
	long int begin();							// return an iterator (long int) to the first element of the buffer
	long int end();								// return an iterator (long int) to the last element of the buffer
	long int next(long int Iterator);			// return an iterator (long int) to the next element of the buffer
	bool ResetIteratorRead();					// reset the  iterator to IteratorInvalid
	bool ReadValid();
	bool ReadLast();


	bool deleteMe();							// resets the buffer / the queue
	bool PrintStatus();							// debug printing
	virtual ~KBuffer();

	const long int IteratorInvalid = -1;
private:
	TEl *Values;
	long int IteratorFirst, IteratorRead;
	long int Count, MaxSize;

};

#endif /* KBuffer_H_ */
