#ifndef brct_menus_h_
#define brct_menus_h_
//-----------------------------------------------------------------------------
//:
// \file
// \brief This file is adopted from Joe's work.
//  This is essentially a namespace for static methods;
//  there is no public constructor
// \author
//  Kongbin Kang 
//
//-----------------------------------------------------------------------------

class brct_menus
{
 public:
	static void kalman_go_callback();
  static void quit_callback();
  static vgui_menu get_menu();
 private:
  brct_menus() {}
};

#endif // brct_menus_h_

