/***************************************************************************
                          objectnamelist.h  -  description
                             -------------------
    begin                : Mon Feb 18 2002
    copyright          : (C) 2002 by Thomas Kabelmann
    email                : tk78@gmx.de
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef OBJECTNAMELIST_H
#define OBJECTNAMELIST_H


/**
	*This class provides an interface like a QPtrList, but sorts objects internally
	*in 27 lists. The objects will sorted alphabetically. List 0 contains all objects
	*beginning not with a letter. List 1 - 26 contains objects beginning with a letter.
	*The number of the list is similar to positon of letter in alphabet. (A = 1 .. Z = 26 )
	*@author Thomas Kabelmann
	*@version 0.9
	*/


#include <qstring.h>
#include "skyobjectname.h"

#include <qglobal.h>
#include <qptrlist.h>

class ObjectNameList {

	public:
	/** Constructor */
		ObjectNameList();

	/** Destructor */
		~ObjectNameList();

	/**
		*Appends skyobjects to list.
		*@param useLongName - using of longname might be forced by true
		*/
		void append(SkyObject *object, bool useLongName=false);

	/**
		*Returns first object in list if available.
		*This function is caseinsensitive.
		*/
		SkyObjectName* first(const QString &name = QString::null);

	/**
		*Returns next object in list if available.
		*/
		SkyObjectName* next();

	/**
		*Returns object if it is in list else NULL.
		*This function is casesensitive.
		*/
		SkyObjectName* find(const QString &name = QString::null);

	/**
		*Define the languages which should be used.
		*/
		enum Language { local = 0, latin = 1 };

	/**
		*Change language option.
		*/
		void setLanguage( Language lang );
		
	/**
		*Change language option (latin, localized).
		*/
		void setLanguage( bool lang );

		uint count() { return amount; }

	private:

	/**
		*Sorts the lists with objects for faster access.
		*It's needed for find(). first() and find() call this function.
		*/
		void sort();

	/**
		*To modes are available:
		*allLists = loop through the whole list if next() is called
		*oneList = loop through one list if next is called
		*If oneList is set, just check all objects which start with same letter to save cpu time.
		*/
		enum Mode { allLists, oneList } mode;

	/**
		*Set mode
		*@see first()
		*/		
		void setMode( Mode m );

		/**Reimplemented from QPtrList for sorting objects in the list. */
		template <class T> class SortedList : public QPtrList <T> {
			protected:
				int compareItems(QPtrCollection::Item item1, QPtrCollection::Item item2) {
					if ( *((T*)item1) == *((T*)item2) ) return 0;
					return ( *((T*)item1) < *((T*)item2) ) ? -1 : 1;
				}
		};

	/**
		*Hold 2 pointer lists in memory. Number of lists can easily increased for adding more languages.
		*First index is the language and second index is reserved for alphabetically sorted list.
		* 0 = all objects beginning with a number
		* 1 = A ... 26 = Z
		*/
		SortedList <SkyObjectName> list[2][27];

		/**Hold status if list is unsorted or not.*/
		bool unsorted[27];

	/**
		*Constellations has latin names and alloc extra SkyObjectNames which will stored in 2 list.
		*But second list will not delete objects while clearing it, because most of objects in this list
		*are in first list too. We just have to delete objects which are not in first list. These objects
		*will stored in this list.
		*/
		QPtrList <SkyObjectName> constellations;

	/**
		*Which list was accessed last time by first() or next()
		*/
		int currentIndex;

	/**@returns the position in the list of the object with a name matching the argument */
		int getIndex( const QString &name = QString::null );

		Language language;

		unsigned int amount;
};

#endif
