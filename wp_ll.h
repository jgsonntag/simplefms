// $Id: coords_ll.h,v 1.2 2004/05/21 22:21:17 wright Exp $
#ifndef WP_LL_H
#define WP_LL_H


//
//  Definition of wp_ll_element class
//

#define PI (4.0*atan((double)(1.0)))
#define AE 6378137.0
#define FLAT (1.0/298.257223563)

#include <qstring.h>

#include <iostream>
using namespace std;

class wp_ll_element
{

  friend class wp_ll;

  public:
    wp_ll_element(void);
    ~wp_ll_element(void);

  private:
    QString name;
    double lat;
    double lon;
    wp_ll_element *next_ptr;

};


//
//  Definition of wp_ll class
//

class wp_ll
{

  public:
    wp_ll(void);
    ~wp_ll(void);
    void empty(); // empty the list and reset pointers to 0
    void add_element(QString, double, double);  // add a new element
    void go_to_top(void);  //  Sets current pointer to top of list
    char get_next_element(QString, double, double);  // gets next element
                                                // returns 0 if at end-of-list
                                                // returns 1 otherwise

  private:
    wp_ll_element *first_ptr;
    wp_ll_element *current_ptr;
    wp_ll_element latlon;
    bool firstcoords;

};

#endif // WP_LL_H
