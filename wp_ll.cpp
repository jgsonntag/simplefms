//
// $Id: coords_ll.cpp,v 1.2 2004/05/21 22:21:17 wright Exp $
//  Implementation of wp_ll_element class
//

#include "wp_ll.h"

wp_ll_element::wp_ll_element(void)
{
  next_ptr = 0;
}

wp_ll_element::~wp_ll_element(void)
{
  cout << "in wp_ll_element destructor\n";
  if (next_ptr != 0)
    delete next_ptr;
}


//
//  Implementation of wp_ll class
//

wp_ll::wp_ll(void)
{
  first_ptr = 0;
  current_ptr = 0;
  firstcoords = true;
}

wp_ll::~wp_ll(void)
{
  if (first_ptr != 0)
    delete first_ptr;
}

void wp_ll::empty()
{
  cout << "hi1\n";
  if (first_ptr!=0)
    delete first_ptr;
  cout << "hi2\n";
  first_ptr = 0;
  cout << "hi3\n";
  current_ptr = 0;
  cout << "hi4\n";
}

void wp_ll::add_element(QString name, double lat, double lon)
{
  wp_ll_element *new_ptr;

  // Store the new element
  new_ptr = new wp_ll_element;
  new_ptr->name = name;
  new_ptr->lat = lat;
  new_ptr->lon = lon;
  new_ptr->next_ptr = first_ptr;
  first_ptr = new_ptr;

}

char wp_ll::get_next_element(QString name, double lat, double lon)
{
  if (current_ptr == 0)
    return(0);
  else
  {
    name = current_ptr->name;
    lat = current_ptr->lat;
    lon = current_ptr->lon;
    current_ptr = current_ptr->next_ptr;
    return(1);
  }
}


void wp_ll::go_to_top(void)
{
  current_ptr = first_ptr;
}
