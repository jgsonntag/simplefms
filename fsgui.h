// $Id: soxgui.h,v 1.5 2004/08/08 02:44:23 sonntag Exp $
// Fsgui class definition
//

#ifndef FSGUI_H
#define FsGUI_H

#include <qmenubar.h>
#include <qpopupmenu.h>
#include <qvbox.h>
#include <qfile.h>
#include <qtextstream.h>
#include <qsettings.h>
#include <qlabel.h>
#include <qvgroupbox.h>
#include <qlistbox.h>
#include <qtable.h>
#include <qpushbutton.h>

#include "genericclient.h"
#include "utility.h"
//#include "wp_ll.h"

#define FT2M (12.0*2.54/100.0)
//#define HDGTOL 20.0 // Deg of delta-heading tolerance
//#define XTDTOL 10.0 // Smallest xtd to output with closely matched heading


class Fsgui : public QVBox
{

  Q_OBJECT

  public:
    Fsgui();  // Constructor
    ~Fsgui();  // Destructor

  private:
    QMenuBar *mainmenu;
    QPopupMenu *filemenu;
    QPopupMenu *setmenu;
    GenericClient *client;
    QString stemp;
    QString gpshost;
    Q_UINT16 gpsport;
    QString gpscmd;
   //  Position *current;
    double t,lat,lon,spd,hdg;
    double hdgtol,xtdtol;
    int idwpfile;
    bool wpflag;
    bool firstoff;
    QString wpfilename;
    QFile *wpfile;
    QSettings usettings;
    double wplat[200];
    double wplon[200];
    double wpspd[200];
    double wpmanmin[200];
    QString wpname[200];
    int nwp;
    int indxwp;
    int mode; // 0=auto, 1=manual
    int autostat; // 0=locked xtd&hdg, 1=locked xtd, 2=reverting to prev
    void ingest_wp(QFile *);
    QListBox *wplistbox;
    QLabel *modelab;
    QLabel *autolab;
    QLabel *timelab;
    QLabel *latlab;
    QLabel *lonlab;
    QLabel *spdlab;
    QLabel *namelab;
    QLabel *distlab;
    QLabel *ttolab;
    QLabel *etalab;
    QTable *fptable;
    double sec2next,eta2next,dist2next;
    void update_autolab();
    void update_current();
    void update_next();
    void update_table();
    int find_segment();
    QPushButton *prevwp;
    QPushButton *nextwp;
    QPushButton *autowp;

  private slots:
    void about();
    void slotNewGPS(QString);
    void slotGPSClosed();
    void slotGPSError(int);
    void slotWPFile();
    void slotXTDTol();
    void slotHDGTol();
    /*void slotNewWP(int);*/
    void slotPrevWP();
    void slotNextWP();
    void slotAutoWP();

};

#endif // FSGUI_H
