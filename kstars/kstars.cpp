/***************************************************************************
                          kstars.cpp  -  K Desktop Planetarium
                             -------------------
    begin                : Mon Feb  5 01:11:45 PST 2001
    copyright            : (C) 2001 by Jason Harris
    email                : jharris@30doradus.org
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
//JH 11.06.2002: replaced infoPanel with infoBoxes
//JH 24.08.2001: reorganized infoPanel
//JH 25.08.2001: added toolbar, converted menu items to KAction objects
//JH 25.08.2001: main window now resizable, window size saved in config file

#include <qfont.h>
#include <qpopupmenu.h>
#include <qtextstream.h>
#include <qlineedit.h>
#include <qsizepolicy.h>
#include <qlayout.h>

#include <kaccel.h>
#include <kconfig.h>
#include <kstdaction.h>
#include <kpopupmenu.h>

#include <stdio.h>
#include <stdlib.h>
#include <stream.h>
#include <kdebug.h>
#include <dcopclient.h>

#include "finddialog.h"
#include "kstars.h"
#include "kstarssplash.h"
#include "simclock.h"
#include "ksutils.h"
#include "infoboxes.h"

KStars::KStars( bool doSplash ) :
	KMainWindow( NULL, NULL ), DCOPObject("KStarsInterface"),
	findDialog( 0 ), DialogIsObsolete( false )
{
	pd = new privatedata(this);

	// we're nowhere near ready to take dcop calls
	kapp->dcopClient()->suspend();

	if ( doSplash ) {
		pd->kstarsData = new KStarsData(this);
		QObject::connect(pd->kstarsData, SIGNAL( initFinished(bool) ),
				this, SLOT( datainitFinished(bool) ) );

		pd->splash = new KStarsSplash(0, "Splash");
		QObject::connect(pd->kstarsData, SIGNAL( progressText(QString) ),
				pd->splash, SLOT( setMessage(QString) ));
		pd->splash->show();
	}
	pd->kstarsData->initialize();
}

KStars::KStars( KStarsData* kd )
	: KMainWindow( NULL, NULL ), DCOPObject("KStarsInterface")
{
	// The assumption is that kstarsData is fully initialized
	
	pd = new privatedata(this);
	pd->kstarsData = kd;
	pd->buildGUI();

	//Instantiate the SimClock object
//JH (22 Jan 2002): we instantiated clock already in buildGUI()...
//	clock = new SimClock(this);
	clock->start();
	show();
}

KStars::~KStars()
{
	saveOptions();

	clearCachedFindDialog();

	delete IBoxes;
	delete skymap;
	delete pd;
	delete clock;
	delete centralWidget;
}

void KStars::changeTime( QDate newDate, QTime newTime ) {

	QDateTime new_time(newDate, newTime);

	GeoLocation *Geo = geo();
	KStarsData *Data = data();

	clock->stop();

	//Make sure Numbers, Moon, planets, and sky objects are updated immediately
	Data->LastNumUpdate = -1000000.0;
	Data->LastMoonUpdate = -1000000.0;
	Data->LastPlanetUpdate = -1000000.0;
	Data->LastSkyUpdate = -1000000.0;

	// reset tzrules data with newlocal time and time direction (forward or backward)
	Geo->tzrule()->reset_with_ltime(new_time, Geo->TZ0(), Data->isTimeRunningForward() );
	// Reset the local time to a valid local time. See TimeZoneRule for explanation.
	new_time = Geo->tzrule()->validLTime();
	
	// reset next dst change time
	Data->setNextDSTChange( KSUtils::UTtoJulian( Geo->tzrule()->nextDSTChange() ) );

	clock->setUTC( new_time.addSecs( int(-3600 * Geo->TZ()) ) );
	
	// reset local sideral time
	setLSTh( clock->UTC() );
}

void KStars::clearCachedFindDialog() {
	if ( findDialog  ) {  // dialog is cached
/**
	*Delete findDialog only if it is not opened
	*/	
		if ( findDialog->isHidden() ) {
			delete findDialog;
			findDialog = 0;
			DialogIsObsolete = false;
		}
		else
			DialogIsObsolete = true;  // dialog was opened so it could not deleted
   }
}

void KStars::updateTime( void ) {
	QTime oldLST = data()->LST;
	// Due to frequently use of this function save data and map pointers for speedup.
	// Save options() and geo() to a pointer would not speedup because most of time options
	// and geo will accessed only one time.
	KStarsData *Data = data();
	SkyMap *Map = map();

	Data->updateTime( clock, geo(), Map );

	infoBoxes()->timeChanged(Data->UTime, Data->LTime, Data->LST, Data->CurrentDate);

	if ( !options()->isTracking && Data->LST > oldLST ) { //kludge advancing the focus
		int nSec = oldLST.secsTo( Data->LST );
		Map->focus()->setRA( Map->focus()->ra().Hours() + double( nSec )/3600. );
		if ( options()->useAltAz ) Map->focus()->EquatorialToHorizontal( Data->LSTh, geo()->lat() );
		showFocusCoords();
	}

	Map->update();

	//If time is accelerated beyond slewTimescale, then the clock's timer is stopped,
	//so that it can be ticked manually after each update, in order to make each time
	//step exactly equal to the timeScale setting.
	//Wrap the call to manualTick() in a singleshot timer so that it doesn't get called until
	//the skymap has been completely updated.
	if ( clock->isManualMode() && clock->isActive() ) {
		QTimer::singleShot( 0, clock, SLOT( manualTick() ) );
	}
}

SkyObject* KStars::getObjectNamed( QString name ) {
	if ( (name== "star") || (name== "nothing") || name.isEmpty() ) return NULL;
	if ( name== data()->Moon->name() ) return data()->Moon;

	SkyObject *so = data()->PC.findByName(name);

	if (so != 0)
		return so;

	//Stars
	for ( unsigned int i=0; i<data()->starList.count(); ++i ) {
		if ( name==data()->starList.at(i)->name() ) return data()->starList.at(i);
	}

	//Deep-sky catalogs
	for ( unsigned int i=0; i<data()->deepSkyList.count(); ++i ) {
		if ( name==data()->deepSkyList.at(i)->name() ) return data()->deepSkyList.at(i);
	}

	//Constellations
	for ( unsigned int i=0; i<data()->cnameList.count(); ++i ) {
		if ( name==data()->cnameList.at(i)->name() ) return data()->cnameList.at(i);
	}

	//reach here only if argument is not matched
	return NULL;
}

void KStars::showFocusCoords( void ) {
	//display object info in infoBoxes
	QString oname;

	oname = i18n( "nothing" );
	if ( map()->foundObject() != NULL && options()->isTracking ) {
		oname = map()->foundObject()->translatedName();
		//add genetive name for stars
	  if ( map()->foundObject()->type()==0 && map()->foundObject()->name2().length() )
			oname += " (" + map()->foundObject()->name2() + ")";
	}

	//
	//This is ugly, got to find a way to change this. But for now.
	//
	infoBoxes()->focusObjChanged(oname);
	infoBoxes()->focusCoordChanged(map()->focus());

}

QString KStars::getDateString( QDate date ) {
  QString dummy;
  QString dateString = dummy.sprintf( "%02d / %02d / %04d",
			  date.month(), date.day(), date.year() );
  return dateString;
}

void KStars::setAltAz(double alt, double az) {
 	map()->setFocusAltAz(alt,az);
}

void KStars::lookTowards (QString direction) {
  QString dir = direction.lower();
	if (dir == "zenith" || dir=="z") map()->invokeKey( KAccel::stringToKey( "Z" ) );
	else if (dir == "north" || dir=="n") map()->invokeKey( KAccel::stringToKey( "N" ) );
	else if (dir == "east"  || dir=="e") map()->invokeKey( KAccel::stringToKey( "E" ) );
	else if (dir == "south" || dir=="s") map()->invokeKey( KAccel::stringToKey( "S" ) );
	else if (dir == "west"  || dir=="w") map()->invokeKey( KAccel::stringToKey( "W" ) );
	else if (dir == "northeast" || dir=="ne") {
		map()->setClickedObject( NULL );
		map()->clickedPoint()->setAlt( 15.0 ); map()->clickedPoint()->setAz( 45.0 );
		map()->clickedPoint()->HorizontalToEquatorial( data()->LSTh, geo()->lat() );
		map()->slotCenter();
	} else if (dir == "southeast" || dir=="se") {
		map()->setClickedObject( NULL );
		map()->clickedPoint()->setAlt( 15.0 ); map()->clickedPoint()->setAz( 135.0 );
		map()->clickedPoint()->HorizontalToEquatorial( data()->LSTh, geo()->lat() );
		map()->slotCenter();
	} else if (dir == "southwest" || dir=="sw") {
		map()->setClickedObject( NULL );
		map()->clickedPoint()->setAlt( 15.0 ); map()->clickedPoint()->setAz( 225.0 );
		map()->clickedPoint()->HorizontalToEquatorial( data()->LSTh, geo()->lat() );
		map()->slotCenter();
	} else if (dir == "northwest" || dir=="nw") {
		map()->setClickedObject( NULL );
		map()->clickedPoint()->setAlt( 15.0 ); map()->clickedPoint()->setAz( 315.0 );
		map()->clickedPoint()->HorizontalToEquatorial( data()->LSTh, geo()->lat() );
		map()->slotCenter();
	} else {
		SkyObject *target = getObjectNamed( direction );
		if ( target != NULL ) {
			map()->setClickedObject( target );
			map()->setClickedPoint( target );
			map()->slotCenter();
		}
	}
}

void KStars::setLocalTime(int yr, int mth, int day, int hr, int min, int sec) {
	changeTime( QDate(yr, mth, day), QTime(hr,min,sec));
}


//
//This is bogus, but until all the data members of KStarsData migrate to the
//class where they _really_ belong, we'll do the forwarding.
//
void KStars::setHourAngle() {
	data()->HourAngle.setH( data()->LSTh.Hours() - map()->focus()->ra().Hours() );
}

void KStars::setLSTh( QDateTime UTC ) {
	data()->LST = KSUtils::UTtoLST( UTC, geo()->lng() );
	data()->LSTh.setH( data()->LST.hour(), data()->LST.minute(), data()->LST.second() );
}

dms KStars::LSTh() { return data()->LSTh; }

KStarsData* KStars::data() { return pd->kstarsData; }

void KStars::mapGetsFocus() { map()->QWidget::setFocus(); }

#include "kstars.moc"
