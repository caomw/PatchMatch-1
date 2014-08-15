#include <vw/Image/ImageView.h>
#include <vw/Math/BBox.h>
#include <vw/FileIO.h>
#include <vw/Stereo/DisparityMap.h>

#include <ARAPDataTerm.h>
#include <PatchMatch2NCC.h>
#include <NelderMead.h>

using namespace vw;

int main(int argc, char **argv) {

  DiskImageView<float>
    left_disk_image("arctic/asp_al-L.crop.16.tif"),
    right_disk_image("arctic/asp_al-R.crop.16.tif");
  BBox2i search_region(Vector2i(-70,-25),
                       Vector2i(105,46));

  ImageView<PixelMask<Vector2i> > pm_disparity =
    stereo::patch_match_ncc(left_disk_image,
                            right_disk_image,
                            search_region/2,
                            Vector2i(15, 15), 2, 3);
  write_image("patchmatch16-D.tif", pm_disparity);

  // Defining the superpixels
  std::vector<BBox2i> box_vec =
    image_blocks(pm_disparity, 32, 32);
  std::vector<std::pair<BBox2i, Vector2> > superpixels;
  superpixels.reserve(box_vec.size());
  for (std::vector<BBox2i>::iterator it =
         box_vec.begin(); it != box_vec.end(); it++) {
    superpixels.push_back
      (std::make_pair
       (*it,
        Vector2(it->min()) +
        Vector2(it->size()) / 2));
  }
  std::cout << "Number of superpixels: "
            << superpixels.size() << std::endl;

  // Define the surfaces
  std::vector<Vector<double, 10> > superpixel_surfaces;
  stereo::define_superpixels(pm_disparity,
                             superpixels,
                             superpixel_surfaces);

  // Track superpixel 100
  std::cout << "Superpixel 100:\n"
            << superpixels[100].first << "\n"
            << superpixels[100].second << "\n"
            << superpixel_surfaces[100] << std::endl;

  // Evaluate the cost for each of the subpixels
  for (size_t i = 0; i < superpixels.size(); i++ ) {
    std::cout << i << " -> "
              << stereo::evaluate_superpixel(left_disk_image,
                                             right_disk_image,
                                             superpixels[i].first,
                                             superpixels[i].second,
                                             superpixel_surfaces[i])
              << std::endl;
  }

  // Render an image of what the surfaces represent
  ImageView<PixelMask<Vector2f> > quad_disparity(pm_disparity.cols(), pm_disparity.rows());
  stereo::render_disparity_image(superpixels,
                                 superpixel_surfaces,
                                 quad_disparity);
  write_image("initial_quad16-D.tif", quad_disparity);

  for (int i = 0; i < 10; i++ ) {
    // Perform a simplex algorithm to solve for a better fitting surface

    // Perform a second order fitting for the entire image.
  }

  return 0;
}