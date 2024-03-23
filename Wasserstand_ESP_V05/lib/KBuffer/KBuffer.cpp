/*
 * KBuffer.cpp
 * Generic KBuffer
 * Version 1: simple float values, no template#
 * Version 2: all data types for elements of the buffer are supported
 *
 *  KBuffer = Values[0 .... MaxSize] (MaxSize + 1 values)
 *  IteratorFirst:   index of first value of the KBuffer
 *  IteratorCurrent: index of last value that has been written
 *	IteratorInvalid: -1, invalid iterator pointing to no element of the queue
 *					 the next element after IteratorLast is IteratorInvalid
 *					 the next element after IteratorInvalid is IteratorFirst
 *
 *
 *
 *
 *
 *  Created on: 07.06.2020
 *      Author: Kai
 */

#include "KBuffer.h"
//#include <new.h>

template<typename TEl>
KBuffer<TEl>::KBuffer(const int KBufferSize)
{
	long int i;

	Values = new TEl[KBufferSize + 1];
	PRINT_BUF_S("-------------------", 0);
	PRINT_BUF_S("TEl:", sizeof(TEl));
	PRINT_BUF_S("KBufferSize:", KBufferSize);
	PRINT_BUF_S("Values:", (long int )&Values[0]);
	PRINT_BUF_S("&Values:", (long int )&Values);
	for (i = 0; i < KBufferSize; i++)
	{
		Values[i] = (TEl)0;
	}

	MaxSize = KBufferSize; // 0 ... KBufferSize
	IteratorFirst = IteratorInvalid;		// index of first element
	IteratorRead = IteratorInvalid;      // index of last element, that has been
	Count = 0;	// number of elements in the KBuffer


	PRINT_BUF_S("Count:", (long int )Count);
	PRINT_BUF_S("&Count:", (long int )&Count);
	PRINT_BUF_S("MaxSize:", (long int )MaxSize);
	PRINT_BUF_S("&MaxSize:", (long int )&MaxSize);
	PRINT_BUF_S("IteratorFirst:", (long int )IteratorFirst);
	PRINT_BUF_S("&IteratorFirst:", (long int )&IteratorFirst);
	PRINT_BUF_S("IteratorRead:", (long int )IteratorRead);
	PRINT_BUF_S("&IteratorRead:", (long int )&IteratorRead);

}

/*=================================================================================
 * This is the destructor for my buffer class!
 *
 */

template<typename TEl>
KBuffer<TEl>::~KBuffer()
{
	delete[] Values;
}

/*=================================================================================
 *
 * Add a value to the KBuffer
 *
 */
template<typename TEl>
bool KBuffer<TEl>::add(TEl Value, bool OverWrite)
{
	long int IteratorNew;
	if(Count == 0)
	{
		// we add the first element to an empty buffer => at index [0]
		IteratorFirst = 0;
	}

	if ((Count < MaxSize) || OverWrite)
	{
		// determine write index
		IteratorNew = (IteratorFirst + Count) % MaxSize;
		// add element
		Values[IteratorNew] = Value;
		Count++;

		if (Count > MaxSize)
		{
			// one element has been overwritten
			IteratorFirst = (IteratorFirst + 1) % MaxSize;
			Count--;
		}
		return (true);
	}
	else
	{
		// MaxSize reached and no over write allowed => error
		return (false);
	}
}

/*=================================================================================
 *
 * Get a specific value from the KBuffer
 *
 */
template<typename TEl>
bool KBuffer<TEl>::get(TEl *Element, long int Iterator)
{
	if ((0 <= Iterator) && (Iterator < MaxSize))
	{
		IteratorRead = Iterator;
		(*Element) = Values[Iterator];
		return (true);
	}
	else
	{
		return (false);
	}
}

/*=================================================================================
 *
 * Get first value from the KBuffer
 *
 */
template<typename TEl>
bool KBuffer<TEl>::getFirst(TEl *Element)
{
	if (!isEmpty())
	{
		IteratorRead = IteratorFirst;
		(*Element) = Values[IteratorRead];
		return (true);
	}
	else
	{
		return (false);
	}
}

/*=================================================================================
 *
 * Get last value from the KBuffer
 *
 */
template<typename TEl>
bool KBuffer<TEl>::getLast(TEl *Element)
{
	long int IteratorLast;

	if (!isEmpty())
	{
		IteratorLast = (IteratorFirst + Count - 1) % MaxSize; // index of the last element
		(*Element) = Values[IteratorLast];
		return (true);
	}
	else
	{
		return (false);
	}
}

/*=================================================================================
 *
 * Get next value from the KBuffer, until you reach the end
 *
 */
template<typename TEl>
bool KBuffer<TEl>::getNext(TEl *Element)
{
	long int IteratorLast;

	if (!isEmpty())
	{
		IteratorRead = next(IteratorRead);
		if (IteratorRead != IteratorInvalid)
		{
			(*Element) = Values[IteratorRead];
			return (true);
		}
		else
		{
			return(false);
		}
	}
	else
	{
		return (false);
	}
}

/*=================================================================================
 *
 * Get last value from the KBuffer and delete it
 *
 * Return true if pop succeeds and false if the buffer is empty
 *
 */
template<typename TEl>
bool KBuffer<TEl>::pop(TEl *Element)
{
	long int IteratorLast;
	if (!isEmpty())
	{
		IteratorLast = (IteratorFirst + Count - 1) % MaxSize; // index of the last element
		IteratorRead = IteratorLast;
		(*Element) = Values[IteratorLast];
		Count--;
		if(isEmpty())
		{
			IteratorRead = IteratorInvalid;
			IteratorFirst = IteratorInvalid;
		}
		return (true);
	}
	else
	{
		return (false);
	}

}

/*=================================================================================
 *
 * Get first value from the KBuffer and delete it
 *
 */
template<typename TEl>
bool KBuffer<TEl>::popFront(TEl *Element)
{
	if (!isEmpty())
	{
		(*Element) = Values[IteratorFirst];
		Count--;
		if (!isEmpty())
		{
			IteratorFirst = (IteratorFirst + 1) % MaxSize; // index of the next element after the head;
		}
		else
		{
			IteratorRead = IteratorInvalid;
			IteratorFirst = IteratorInvalid;
		}
		return (true);
	}
	else
	{
		return (false);
	}

}

/*=================================================================================
 *
 *  Reset the  iterator to IteratorInvalid
 *  the next element  with getNext is then element IteratorStart
 */

template<typename TEl>
bool KBuffer<TEl>::ResetIteratorRead()
{
	IteratorRead = IteratorInvalid;
	return(true);
}

/*=================================================================================
 *
 *  Is the current element that Iterator is pointing to a valid one?
 */

template<typename TEl>
bool KBuffer<TEl>::ReadValid()
{
	return(IteratorRead != IteratorInvalid);
}

/*=================================================================================
 *
 *  Is the current element that Iterator is pointing to the last one?
 */

template<typename TEl>
bool KBuffer<TEl>::ReadLast()
{
	long int IteratorLast;

	IteratorLast = (IteratorFirst + Count - 1) % MaxSize; // index of the last element
	return(IteratorRead == IteratorLast);
}



/*=================================================================================
 *
 * Delete all values and reset the KBuffer
 * Values do not have to be overwritten
 *
 */
template<typename TEl>
bool KBuffer<TEl>::deleteMe()
{
	IteratorFirst = IteratorInvalid;
	IteratorRead = IteratorInvalid;
	Count = 0;
	return (true);
}
/*=================================================================================
 *
 * Is the KBuffer empty?
 *
 */
template<typename TEl>
bool KBuffer<TEl>::isEmpty()
{
	return (Count == 0);
}

/*=================================================================================
 *
 * get the current number of elements in the buffer
 *
 */
template<typename TEl>
long int KBuffer<TEl>::GetNoOfElements()
{
	return (Count);
}

/*=================================================================================
 *
 * Get iterator (long int) to the first element of the buffer
 *
 */
template<typename TEl>
long int KBuffer<TEl>::begin()
{
	return (IteratorFirst);
}

/*=================================================================================
 *
 * Get iterator (long int) to the last element of the buffer
 *
 */
template<typename TEl>
long int KBuffer<TEl>::end()
{
	long int IteratorLast;

	IteratorLast = (IteratorFirst + Count - 1) % MaxSize; // index of the last element
	return (IteratorLast);
}

/*=================================================================================
 *
 * Get iterator (long int) to the next element of the buffer
 *
 */
template<typename TEl>
long int KBuffer<TEl>::next(long int Iterator)
{
	long int IteratorNext, IteratorLast;

	if(Iterator == IteratorInvalid)
	{
		return (IteratorFirst); // by definition, next element after invalid is the start
	}

	IteratorLast = (IteratorFirst + Count - 1) % MaxSize; // index of the last element

	if(Iterator == IteratorLast)
	{
		// we are at the last element, thus the next one is invalid
		IteratorNext = IteratorInvalid;
	}
	else
	{
		// Calculate iterator of next element
		IteratorNext = (Iterator + 1) % MaxSize; // index of the next element
	}
	return (IteratorNext);
}


/*=================================================================================
 *
 * Print status
 *
 */
template<typename TEl>
bool KBuffer<TEl>::PrintStatus()
{
	int i;

	Serial.println("--------------------");
	Serial.print("MaxSize:");
	Serial.println(MaxSize);

	Serial.print("Count:");
	Serial.println(Count);

	Serial.print("IteratorFirst:");
	Serial.println(IteratorFirst);

	Serial.print("IteratorRead:");
	Serial.println(IteratorRead);

	Serial.print("V = [");
	for (i = 0; i < MaxSize; i++)
	{
		Serial.print(Values[i]);
		Serial.print(" ,");
	}
	Serial.println("]");
	return (true);
}

/*=================================================================================
 force the compiler to generate the required instantiations
 https://stackoverflow.com/questions/8752837/undefined-reference-to-template-class-constructor

 */

template class KBuffer<float> ;
template class KBuffer<long> ;
template class KBuffer<unsigned long> ;
template class KBuffer<int> ;
template class KBuffer<unsigned int> ;
template class KBuffer<byte> ;

