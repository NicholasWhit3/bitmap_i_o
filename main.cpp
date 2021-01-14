#include <iostream>
#include "bitmap_i_o.h"

int main()
{
    std::string outputName ("outputImage.bmp");
    bitmap_i_o* test = bitmap_i_o::load_bitmap("image.bmp");
    test->print();
    test->save(outputName);
}
