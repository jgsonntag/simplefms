// $Id: soxgui.cpp,v 1.9 2004/08/08 02:44:23 sonntag Exp $
// Fsgui class implementation
//

#include <qapplication.h>
#include <qmessagebox.h>
#include <qfiledialog.h>
#include <qinputdialog.h>

#include "fsgui.h"
#include "setupgui.h"

#include <stdlib.h>

#include <iostream>
using namespace std;

Fsgui::Fsgui()
{
  int i,manrows,nrow;

  // Get the saved user settings
  gpshost = usettings.readEntry("/flightstatus/network/gpshost","localhost");
  gpsport = (Q_UINT16)(usettings.readNumEntry("/flightstatus/network/gpsport",
            4040));
  gpscmd = usettings.readEntry("/flightstatus/network/gpscmd","gga1hzstream");
  wpfilename = usettings.readEntry("/flightstatus/files/wpfilename","");
  indxwp = usettings.readNumEntry("/flightstatus/plan/indxwp",0);
  mode = usettings.readNumEntry("/flightstatus/plan/mode",0);
  hdgtol = usettings.readDoubleEntry("/flightstatus/settings/hdgtol",20.0);
  xtdtol = usettings.readDoubleEntry("/flightstatus/settings/xtdtol",10.0);

  // Set flags
  wpflag = false;
  autostat = 2;

  // Get network settings from user
  Setupgui *setup = new Setupgui(&gpshost,&gpsport,&gpscmd,this);
  while (setup->exec() == QDialog::Rejected)
  {
    QMessageBox::warning(this,"Invalid\n","Port number is invalid\n");
  }
  if (!gpscmd.contains('\n'))
    gpscmd.append('\n');

  // Construct the current position class;
  //current = new Position();

  // Build the main menubar and make connections
  mainmenu = new QMenuBar(this);
  filemenu = new QPopupMenu(mainmenu);
  setmenu  = new QPopupMenu(mainmenu);
  QPopupMenu *helpmenu = new QPopupMenu(mainmenu);
  idwpfile = filemenu->insertItem("Open waypoint file",this,SLOT(slotWPFile()));
  filemenu->insertItem("Quit",qApp,SLOT(quit()));
  setmenu->insertItem("XTD Tol",this,SLOT(slotXTDTol()));
  setmenu->insertItem("HDG Tol",this,SLOT(slotHDGTol()));
  helpmenu->insertItem("About",this,SLOT(about()));
  mainmenu->insertItem("File",filemenu);
  mainmenu->insertItem("Settings",setmenu);
  mainmenu->insertItem("Help",helpmenu);

  // Build the main layout for the gui
  QHBox *mainbox = new QHBox(this);
  QVBox *leftside  = new QVBox(mainbox);

  // Build the current position display
  /*QVGroupBox *current = new QVGroupBox("Current conditions",leftside);
  new QLabel("Time:",current);
  timelab = new QLabel(current);
  timelab->setFont(QFont("Helvetica",14,QFont::Bold));
  new QLabel("Lat (deg):",current);
  latlab = new QLabel(current);
  latlab->setFont(QFont("Helvetica",14,QFont::Bold));
  new QLabel("Lon (deg):",current);
  lonlab = new QLabel(current);
  lonlab->setFont(QFont("Helvetica",14,QFont::Bold));
  new QLabel("Gndspd (knots):",current);
  spdlab = new QLabel(current);
  spdlab->setFont(QFont("Helvetica",14,QFont::Bold)); */

  //  Build the mode indicator
  QVGroupBox *modebox = new QVGroupBox("Waypoint mode",leftside);
  modelab = new QLabel(modebox);
  modelab->setFont(QFont("Helvetica",14,QFont::Bold));
  autolab = new QLabel(modebox);
  autolab->setFont(QFont("Helvetica",8));
  if (mode==0)
  {
    modelab->setText("AUTOMATIC");
    autolab->setText("");
  }
  else
    modelab->setText("MANUAL");

  // Build the next waypoint display
  QVGroupBox *next = new QVGroupBox("Next waypoint",leftside);
  new QLabel("Name:",next);
  namelab = new QLabel(next);
  namelab->setFont(QFont("Helvetica",14,QFont::Bold));
  new QLabel("Dist to (nm):",next);
  distlab = new QLabel(next);
  distlab->setFont(QFont("Helvetica",14,QFont::Bold));
  new QLabel("Time to (hh:mm:ss):",next);
  ttolab = new QLabel(next);
  ttolab->setFont(QFont("Helvetica",14,QFont::Bold));
  new QLabel("ETA (hh:mm:ss):",next);
  etalab = new QLabel(next);
  etalab->setFont(QFont("Helvetica",14,QFont::Bold));

  // Build the waypoint select buttons
  QHBox *pnbuttons = new QHBox(leftside);
  prevwp = new QPushButton(tr("Prev WP"),pnbuttons);
  connect(prevwp,SIGNAL(clicked()),SLOT(slotPrevWP()));
  nextwp = new QPushButton(tr("Next WP"),pnbuttons);
  connect(nextwp,SIGNAL(clicked()),SLOT(slotNextWP()));
  autowp = new QPushButton(tr("Auto WP"),leftside);
  connect(autowp,SIGNAL(clicked()),SLOT(slotAutoWP()));

  // Build the flightplan tabular display
  QVGroupBox *fpbox = new QVGroupBox("Flight Plan",mainbox);
  fptable = new QTable(fpbox);
  fptable->setReadOnly(true);
  fptable->setNumCols(4);
  QHeader *fpheader = fptable->horizontalHeader();
  fpheader->setLabel(0,"Name");
  fpheader->setLabel(1,"Dist to");
  fpheader->setLabel(2,"Time to");
  fpheader->setLabel(3,"ETA");
  QHeader *vheader = fptable->verticalHeader();
  vheader->hide();
  fptable->setLeftMargin(0);
  fptable->setColumnWidth(0,60);
  fptable->setColumnWidth(1,60);
  fptable->setColumnWidth(2,80);
  fptable->setColumnWidth(3,80);

  // Build the next waypoint selector
  /*QVGroupBox *wpbox = new QVGroupBox("Select next",mainbox);
  wplistbox = new QListBox(wpbox);
  connect(wplistbox,SIGNAL(highlighted(int)),this,SLOT(slotNewWP(int))); */

  // Build the network client
  client = new GenericClient(gpshost,gpsport,gpscmd,this);
  connect(client,SIGNAL(gotaline(QString)),this,SLOT(slotNewGPS(QString)));

  // Load a pre-existing waypoint file
  wpfile = new QFile(wpfilename);
  if (wpfile->exists())
  {
    ingest_wp(wpfile);
    wpflag = true;
    stemp = QString("Close waypoint file (");
    stemp.append(wpfilename);
    stemp.append(")");
    filemenu->changeItem(idwpfile,stemp);
    manrows=0;
    for (i=0;i<nwp;i++)
    {
      if (wpmanmin[i]>0.0)
        ++manrows;
    }
    fptable->setNumRows(nwp+manrows);
    nrow = 0;
    for (i=0;i<nwp;i++)
    {
      fptable->setText(nrow,0,wpname[i]);
      ++nrow;
      if (wpmanmin[i]>0.0)
      {
        fptable->setText(nrow,0,"*manvr*");
        ++nrow;
      }
    }
  }

}


Fsgui::~Fsgui()
{

  // Close the network connection
  client->closeConnection();

  // Save user settings
  usettings.writeEntry("/flightstatus/network/gpshost",gpshost);
  usettings.writeEntry("/flightstatus/network/gpsport",gpsport);
  usettings.writeEntry("/flightstatus/network/gpscmd",gpscmd);
  usettings.writeEntry("/flightstatus/files/wpfilename",wpfilename);
  usettings.writeEntry("/flightstatus/plan/indxwp",indxwp);
  usettings.writeEntry("/flightstatus/plan/mode",mode);
  usettings.writeEntry("/flightstatus/settings/hdgtol",hdgtol);  
  usettings.writeEntry("/flightstatus/settings/xtdtol",xtdtol);  

  // Delete the objects still out there
  delete mainmenu;

}


void Fsgui::slotXTDTol()
{
  bool ok;
  double dtemp = QInputDialog::getDouble("XTD Tolerance",
                 "Enter cross-track distance tolerance (km)",
                 xtdtol,0.0,1000.0,2,&ok,this);
  if (ok)
    xtdtol = dtemp;
}


void Fsgui::slotHDGTol()
{
  bool ok;
  double dtemp = QInputDialog::getDouble("HDG Tolerance",
                 "Enter heading tolerance (deg)",
                 hdgtol,0.0,360.0,2,&ok,this);
  if (ok)
    hdgtol = dtemp;
}


void Fsgui::slotPrevWP()
{
  if (indxwp>0)  indxwp--;
  namelab->setText(wpname[indxwp]);
  mode = 1;
  modelab->setText("MANUAL");
  autolab->setText("");
}


void Fsgui::slotNextWP()
{
  if (indxwp<(nwp-1))  indxwp++;
  namelab->setText(wpname[indxwp]);
  mode = 1;
  modelab->setText("MANUAL");
  autolab->setText("");
}


void Fsgui::slotAutoWP()
{
  mode = 0;
  modelab->setText("AUTOMATIC");
}


void Fsgui::slotNewGPS(QString gps)
{
  int k;

  // Determine the message type and parse as appropriate
  stemp = gps.section(',',0,0);
  if (stemp=="10")
  {
    stemp = gps.section(',',1,1);
    t = stemp.toDouble();
    stemp = gps.section(',',2,2);
    lat = stemp.toDouble();
    stemp = gps.section(',',3,3);
    lon = stemp.toDouble();
    stemp = gps.section(',',5,5);
    hdg = stemp.toDouble();
    stemp = gps.section(',',6,6);
    spd = stemp.toDouble();
    if (mode==0 && wpflag)  // Auto waypoint select
    {
      k = find_segment();
      if (k==-999)
      {
        if (firstoff&&indxwp<(nwp-1))
        {
          ++indxwp;
          namelab->setText(wpname[indxwp]);
          firstoff = 0;
        }
      }
      else
      {
        firstoff = 1;
        indxwp = k;
        namelab->setText(wpname[indxwp]);
      }
    }
    if (wpflag)
    {
      //update_current();
      update_next();
      update_table();
      if (mode==0) update_autolab();
    }
  }
  else
  {
    cout << "Unrecognized network message type - exiting\n";
    exit(-1);
  }

}


int Fsgui::find_segment()
{
  int i,indx_sm,indx_crs_sm;
  double distAD,distBD,distAB,crsAD,crsAB;
  double xtd_sm,xtd_crs_sm,xtd,dhdg;
   
  // Loop over waypoint list
  xtd_sm = 999.9;
  xtd_crs_sm = 999.9;
  for (i=0; i<(nwp-1); i++)
  {
    //cout << wpname[i] << ' ' << wpname[i+1] << ' ';
    distAD = gcdist(wplat[i]*PI/180.0,wplon[i]*PI/180.0,lat*PI/180.0,
                    lon*PI/180.0);
    distBD = gcdist(wplat[i+1]*PI/180.0,wplon[i+1]*PI/180.0,lat*PI/180.0,
                    lon*PI/180.0);
    distAB = gcdist(wplat[i]*PI/180.0,wplon[i]*PI/180.0,wplat[i+1]*PI/180.0,
                    wplon[i+1]*PI/180.0);
    crsAD  = gccourse1(wplat[i]*PI/180.0,wplon[i]*PI/180.0,lat*PI/180.0,
                       lon*PI/180.0);
    crsAB  = gccourse1(wplat[i]*PI/180.0,wplon[i]*PI/180.0,
                       wplat[i+1]*PI/180.0,wplon[i+1]*PI/180.0);
    if (distAD > distAB || 180.0/PI*fabs(crsAB-crsAD)>hdgtol)
      xtd = 999.9;
    else
      xtd = RAD2KM*fabs(asin(sin(distAD)*sin(crsAD-crsAB)));
    crsAB *= 180.0/PI;
    dhdg = fabs(hdg - crsAB);
    //printf("%lf %lf %lf %lf ",crsAB,hdg,dhdg,xtd);
    //if ( (dhdg-180.0) < hdgtol )
    //  dhdg -= 180.0;
    if ( xtd <= xtd_sm )
    {
      indx_sm = i;
      xtd_sm = xtd;
    }
    if ( xtd <= xtd_crs_sm && dhdg <= hdgtol )
    {
      indx_crs_sm = i;
      xtd_crs_sm = xtd;
    }
  }

  //  Select xtd to return
  if ( xtd_crs_sm < xtdtol )
  {
    autostat = 0;
    return(indx_crs_sm+1);
  }
  else if (xtd_sm < xtdtol)
  {
    autostat = 1;
    return(indx_sm+1);
  }
  else
  {
    autostat = 2;
    return(-999);
  }

}


void Fsgui::update_table()
{
  int i,hr,min,nrow;
  double sec,dist,eta;

//    nrow = 0;
//    for (i=0;i<nwp;i++)
//    {
//      fptable->setText(nrow,0,wpname[i]);
//      ++nrow;
//      if (wpmanmin[i]>0.0)
//      {
//        fptable->setText(nrow,0,"*manvr*");
//        ++nrow;
//      }
//   }


  // Waypoints before the selected next one
  nrow = 0;
  for (i=0; i<indxwp; i++)
  {
    fptable->setText(nrow,1,"-");  // Dist
    fptable->setText(nrow,2,"-");  // Time to
    fptable->setText(nrow,3,"-");  // ETA
    ++nrow;
    if (wpmanmin[i]>0.0)
    {
      fptable->setText(nrow,1,"-");
      fptable->setText(nrow,2,"-");
      fptable->setText(nrow,3,"-");
      ++nrow;
    }
  }

  // Next waypoint
  stemp.sprintf("%.1lf",dist2next);
  fptable->setText(nrow,1,stemp);
  sec2hms(sec2next,&hr,&min,&sec);
  stemp.sprintf("%02d:%02d:%02d",hr,min,(int)(sec));
  fptable->setText(nrow,2,stemp);
  sec2hms(eta2next,&hr,&min,&sec);
  stemp.sprintf("%02d:%02d:%02d",hr,min,(int)(sec));
  fptable->setText(nrow,3,stemp);
  ++nrow;

  // Maneuver immediately following next waypoint
  double alldist = dist2next;
  double allsec = sec2next;
  if (wpmanmin[i]>0.0)
  {
    allsec += wpmanmin[i]*60.0;
    alldist += wpmanmin[i]/60.0*wpspd[i];
    stemp.sprintf("%.1lf",alldist);
    fptable->setText(nrow,1,stemp);
    sec2hms(allsec,&hr,&min,&sec);
    stemp.sprintf("%02d:%02d:%02d",hr,min,(int)(sec));
    fptable->setText(nrow,2,stemp);
    eta = t + allsec;
    sec2hms(eta,&hr,&min,&sec);
    stemp.sprintf("%02d:%02d:%02d",hr,min,(int)(sec));
    fptable->setText(nrow,3,stemp);
    ++nrow;
  }

  // Waypoints after next
  for (i=indxwp+1;i<nwp;i++)
  {
    dist = RAD2NM*gcdist(wplat[i-1]*DEG2RAD,wplon[i-1]*DEG2RAD,
           wplat[i]*DEG2RAD,wplon[i]*DEG2RAD);
    alldist += dist;
    stemp.sprintf("%.1lf",alldist);
    fptable->setText(nrow,1,stemp);
    allsec += 3600.0*dist/wpspd[i-1];
    sec2hms(allsec,&hr,&min,&sec);
    stemp.sprintf("%02d:%02d:%02d",hr,min,(int)(sec));
    fptable->setText(nrow,2,stemp);
    eta = t + allsec;
    sec2hms(eta,&hr,&min,&sec);
    stemp.sprintf("%02d:%02d:%02d",hr,min,(int)(sec));
    fptable->setText(nrow,3,stemp);
    ++nrow;
    if (wpmanmin[i]>0.0)
    {
      allsec += wpmanmin[i]*60.0;
      alldist += wpmanmin[i]/60.0*wpspd[i];
      stemp.sprintf("%.1lf",alldist);
      fptable->setText(nrow,1,stemp);
      sec2hms(allsec,&hr,&min,&sec);
      stemp.sprintf("%02d:%02d:%02d",hr,min,(int)(sec));
      fptable->setText(nrow,2,stemp);
      eta = t + allsec;
      sec2hms(eta,&hr,&min,&sec);
      stemp.sprintf("%02d:%02d:%02d",hr,min,(int)(sec));
      fptable->setText(nrow,3,stemp);
      ++nrow;
    }
  }

}


void Fsgui::update_next()
{
  int hr,min;
  double sec;

  // Update waypoint name
  namelab->setText(wpname[indxwp]);

  // Update distance to next
  dist2next = RAD2NM*gcdist(wplat[indxwp]*DEG2RAD,wplon[indxwp]*DEG2RAD,
                lat*DEG2RAD,lon*DEG2RAD);
  stemp.sprintf("%.1lf",dist2next);
  distlab->setText(stemp);

  // Update time to next
  if (spd>0.0)
  {
    sec2next = 3600*dist2next/spd;
    eta2next  = t + sec2next;
    sec2hms(sec2next,&hr,&min,&sec);
    stemp.sprintf("%02d:%02d:%02d",hr,min,(int)(sec));
    ttolab->setText(stemp);
  }
  else
  {
    stemp.sprintf("--:--:--");
    ttolab->setText(stemp);
  }

  // Update ETA at next
  if (spd>0.0)
  {
    sec2hms(eta2next,&hr,&min,&sec);
    stemp.sprintf("%02d:%02d:%02d",hr,min,(int)(sec));
    etalab->setText(stemp);
  }
  else
  {
    stemp.sprintf("--:--:--");
    etalab->setText(stemp);
  }

}


void Fsgui::update_autolab()
{
  if (autostat==0)
    autolab->setText("Locked on hdg & xtd");
  else if (autostat==1)
    autolab->setText("Locked on xtd only");
  else
    autolab->setText("Off-course; assuming next");
}


void Fsgui::update_current()
{
  int hr,min;
  double sec;

  latlab->setNum(lat);
  lonlab->setNum(lon);
  spdlab->setNum(spd);
  sec2hms(t,&hr,&min,&sec);
  stemp.sprintf("%02d:%02d:%02d",hr,min,(int)(sec));
  timelab->setText(stemp);
}


void Fsgui::slotGPSClosed()
{
  stemp.sprintf("Connection to GPS network server closed by the server");
  QMessageBox::critical(this,"GPS Connection Closed",stemp);
}



void Fsgui::slotGPSError(int err)
{
  if (err==QSocket::ErrConnectionRefused)
    stemp.sprintf("Connection to GPS server refused\n");
  else if (err==QSocket::ErrHostNotFound)
    stemp.sprintf("GPS server host not found\n");
  else if (err==QSocket::ErrSocketRead)
    stemp.sprintf("Socket read from GPS server failed");
  QMessageBox::critical(this,"GPS Network Error",stemp);
}


/*void Fsgui::slotNewWP(int indx)
{
  namelab->setText(wpname[indx]);
  indxwp = indx;
}*/

void Fsgui::slotWPFile()
{
  int i,manrows,nrow;

  // If wp file not already open, allow user to open one
  if (!wpflag)
  {
    wpfilename = QFileDialog::getOpenFileName(".",
                 "sequence (*.sequence)",
		 this,"waypointfile","Select a waypoint sequence file");
    wpfile = new QFile(wpfilename);
    if (wpfile->exists())
    {
      ingest_wp(wpfile);
      wpflag = true;
      stemp = QString("Close waypoint file (");
      stemp.append(wpfilename);
      stemp.append(")");
      filemenu->changeItem(idwpfile,stemp);
      manrows=0;
      for (i=0;i<nwp;i++)
      {
        if (wpmanmin[i]>0.0)
          ++manrows;
      }
      fptable->setNumRows(nwp+manrows);
      nrow = 0;
      for (i=0;i<nwp;i++)
      {
        fptable->setText(nrow,0,wpname[i]);
        ++nrow;
        if (wpmanmin[i]>0.0)
        {
          fptable->setText(nrow,0,"*manvr*");
          ++nrow;
        }
      }
//      ingest_wp(wpfile);
//      wpflag = true;
//      stemp = QString("Close waypoint file (");
//      stemp.append(wpfilename);
//      stemp.append(")");
//      filemenu->changeItem(idwpfile,stemp);
//      fptable->setNumRows(nwp);
//      for (i=0;i<nwp;i++)
//        fptable->setText(i,0,wpname[i]);
    }
  }

  // Otherwise null the wp list
  else
  {
    nwp = 0;
    wpflag = false;
    filemenu->changeItem(idwpfile,"Open waypoint file");
    wpfilename = "";
    //wplistbox->clear();
    namelab->clear();
    distlab->clear();
    ttolab->clear();
    etalab->clear();
    indxwp = 0;
  }

}


void Fsgui::ingest_wp(QFile *infile)
{
  const char *sline;
  char schar[10],mandesc[30];
  double manmin;
  QString line,name;

  infile->open(IO_ReadOnly);
  QTextStream instream(infile);
  nwp = 0;
  while(!instream.atEnd())
  {
    line = instream.readLine();
    sline = line.latin1();
    sscanf(sline,"%s %lf %lf %lf %lf %28c",schar,&lat,&lon,&spd,&manmin,mandesc);
    wpname[nwp] = schar;
    wplat[nwp] = lat;
    wplon[nwp] = lon;
    wpspd[nwp] = spd;
    wpmanmin[nwp] = manmin;
    //wplistbox->insertItem(schar);
    ++nwp;
  }
  //wplistbox->setCurrentItem(indxwp);
  infile->close();
}


void Fsgui::about()
{
  QMessageBox::about(this,"About SIMPLEFMS",
                     "SIMPLEFMS\nAuthor: John G. Sonntag\n"
		     "Prototype FLIGHTSTATUS Released 3 November 2004\n"
		     "SIMPLEFMS Released 8 June 2006\n"
                     "Last Updated 14 April 2009\n"
		    );
}
