//:
// \file 
// \brief  First attempt at multi gpu update
// \author Andy Miller
// \date 13-Nov-2011
#include <boxm2_multi_cache.h>
#include <algo/boxm2_multi_render.h>
#include <algo/boxm2_multi_update.h>

#include <vcl_where_root_dir.h>
#include <vcl_algorithm.h>

//executable args
#include <vul/vul_arg.h>
#include <vul/vul_file.h>
#include <vil/vil_image_view_base.h>
#include <vil/vil_save.h>

#include <boxm2/boxm2_scene.h>
#include <boxm2/boxm2_util.h>
#include <boxm2/ocl/boxm2_opencl_cache.h>

#include <bocl/bocl_manager.h>
#include <bocl/bocl_device.h>
#include <bocl/bocl_kernel.h>

#include <brdb/brdb_value.h>
#include <brdb/brdb_selection.h>
#include <bprb/bprb_batch_process_manager.h>
#include <bprb/bprb_parameters.h>
#include <bprb/bprb_macros.h>
#include <bprb/bprb_func_process.h>

int main(int argc,  char** argv)
{
  //init vgui (should choose/determine toolkit)
  vul_arg<vcl_string> scene_file("-scene", "scene filename", "");
  vul_arg<vcl_string> img_dir("-imgdir", "update image directory",""); 
  vul_arg<vcl_string> cam_dir("-camdir", "update camera directory", ""); 
  vul_arg<unsigned>   num_updates("-num", "Number of updates", 10); 
  vul_arg_parse(argc, argv);
  
  //create scene
  boxm2_scene_sptr scene = new boxm2_scene(scene_file());
  
  //make bocl manager (handles a lot of OpenCL stuff)
  bocl_manager_child_sptr mgr = bocl_manager_child::instance();

  //create cpu cache (lru), and create opencl_cache on the device
  boxm2_lru_cache::create(scene);

  //DEBUG - using just one gpu at first
  //make a multicache
  for(int i=0; i<mgr->gpus_.size(); ++i)
    vcl_cout<<" GPU "<<i<<": "<<mgr->gpus_[i]<<vcl_endl;
  vcl_vector<bocl_device*> gpus; 
  gpus.push_back(mgr->gpus_[1]); 
  boxm2_multi_cache mcache(scene, mgr->gpus_); 
  vcl_cout<<"Multi Cache: \n"<<mcache.to_string()<<vcl_endl;

  //-- GET UPDATE IMG/CAMS ---
  vcl_vector<vcl_string> imgs = boxm2_util::images_from_directory( img_dir() );
  vcl_vector<vcl_string> cams = boxm2_util::camfiles_from_directory( cam_dir() );
  
  //-----------------------------------------------------------
  //render setup
  //-----------------------------------------------------------
  float mean_time=0.0f; 
  int num_renders = 5; 
  boxm2_multi_render renderer; 
  int ni=1280, nj=720; 

  //-----------------------------------------------------------
  //run update/renders
  //-----------------------------------------------------------
  int numUpdates = vcl_min( (int) num_updates(), (int) imgs.size()); 
  for(int i=0; i<numUpdates; ++i) 
  {
    //update with input image
    vil_image_view_base_sptr inImg = boxm2_util::prepare_input_image(imgs[i], true); 
    vpgl_camera_double_sptr  inCam = boxm2_util::camera_from_file( cams[i] );
    vil_image_view<float>* inImgPtr = dynamic_cast<vil_image_view<float>* >(inImg.ptr()); 
    boxm2_multi_update::update(mcache, *inImgPtr, inCam); 
    
    //create cam
    double currInc    = 45.0;
    double currRadius = scene->bounding_box().height(); 
    double currAz     = i*30.0f; 
    vpgl_perspective_camera<double>* pcam;
    pcam = boxm2_util::construct_camera(currInc, currAz, currRadius, ni, nj, scene->bounding_box(), false);
    vpgl_camera_double_sptr cam = new vpgl_perspective_camera<double>(*pcam);
    
    //render/save image
    vil_image_view<float> out(ni,nj); 
    float rtime = renderer.render(mcache, out, cam); 
    vcl_cout<<"Render "<<i<<" time: "<<rtime<<vcl_endl;
    vcl_stringstream s; s<<"out_"<<i<<".tiff"; 
    vil_save(out, s.str().c_str()); 
  }

  return 0;
}