// This is brl/bbas/vidl2/vidl2_v4l2_istream.cxx
#include "vidl2_v4l2_istream.h"
//:
// \file
//
// \author Antonio Garrido
// \verbatim
//  Modifications
//   30 Apr 2008 Created (A. Garrido)
//\endverbatim

#include "vcl_iostream.h"
#include "vidl2_frame.h"
#include "v4l2_pixel_format.h"


void vidl2_v4l2_istream::update_frame()
{
  frame_number_=0;
#if 0
  if (!dev) return;
  vcl_cout << "Nbuf: " << dev->get_number_of_buffers() << vcl_endl;
  if (dev->get_number_of_buffers()!=1 && dev->is_capturing()){
    dev->stop_capturing();
    if (!dev->set_number_of_buffers(1)) // right now, only 1
      vcl_cout << "No puedo!" << vcl_endl;
    dev->start_capturing();
  }
  else if (!dev->set_number_of_buffers(1)) // right now, only 1
    vcl_cout << "No!" << vcl_endl;
#endif // 0
  dev.set_number_of_buffers(1);
  if (dev.ibuffer(0)) {
    cur_frame_ = new vidl2_shared_frame(
        dev.ibuffer(0), dev.get_width(), dev.get_height(), v4l2_to_vidl2(dev.get_v4l2_format()));
  }
  else {
    vcl_cout << "No Buf!" << vcl_endl;
    cur_frame_=0;
  }
}

vidl2_v4l2_istream::vidl2_v4l2_istream(vidl2_v4l2_device& device): dev(device)
{
  update_frame();
}

bool vidl2_v4l2_istream::advance()
{
   if (!dev.is_capturing())
    if (!dev.start_capturing())
       return false;
   if (cur_frame_!=0 && dev.read_frame()) {
     frame_number_++;
     return true;
   }
   else return false;
}

vidl2_frame_sptr vidl2_v4l2_istream::read_frame()
{
  advance();
  return cur_frame_;
}

