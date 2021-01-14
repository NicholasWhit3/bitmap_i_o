#include "bitmap_i_o.h"
#include <fstream>
#include <iostream>
#include <limits>
#include <string>
#include <vector>

bitmap_i_o::bitmap_i_o()
    : _fileName(""),
      _channel_mode(bgr_mode),
      _width(0),
      _height(0),
      _bytesPerPixel(3),
      _rowIncrement(0)

{

}

bitmap_i_o::bitmap_i_o(const std::string &fileName)
    : _fileName(fileName),
      _channel_mode(bgr_mode),
      _width(0),
      _height(0),
      _bytesPerPixel(0),
      _rowIncrement(0)
{

}

bool bitmap_i_o::big_endian()
{
    unsigned int v = 0x01;
    return (1 != reinterpret_cast<char*>(&v)[0]);       //priority
}

 bitmap_i_o* bitmap_i_o::load_bitmap(std::string fileName)
{
    bitmap_i_o* bmImage = new bitmap_i_o(fileName);

    std::ifstream stream (bmImage->_fileName.c_str(), std::ios::binary);

    if(!stream)
    {
        std::cout << "load_bitmap ERROR : Bitmap image - " << bmImage->_fileName << " not found!" << std::endl;
    }

   bmImage->_width = 0;
   bmImage->_height = 0;

   bitmap_file_header bfh;
   bitmap_information_header bih;

   bfh.clear();
   bih.clear();

   bmImage->read_bfh(stream,bfh);
   bmImage->read_bih(stream,bih);

   if(bfh._type != 19778)
   {
       std::cout << "load_bitmap ERROR : Invalid type value " << bfh._type << ", expected 19778." << std::endl;

       bfh.clear();
       bih.clear();

       stream.close();
       return nullptr;
   }

   if(bih._bitCount != 24)
   {
       std::cout << "load_bitmap ERROR : Invalid bit depth " << bih._bitCount << ", expected 24" << std::endl;

       bfh.clear();
       bih.clear();

       stream.close();
       return nullptr;
   }

   if(bih._size != bih.struct_size())
   {
       std::cout << "load_bitmap ERROR : Invalid Bitmap Information Header size " << bih._size << ", expected " << bih.struct_size() << std::endl;

       bfh.clear();
       bih.clear();

       stream.close();
       return nullptr;
   }

   bmImage->_width = bih._width;
   bmImage->_height = bih._height;

   bmImage->_bytesPerPixel = bih._bitCount >> 3;

   unsigned int padding = (4 - ( (3 * bmImage->_width) % 4) ) % 4;               //filling
   char padding_data[4] = {0x00, 0x00, 0x00, 0x00};                                        //filling data

   std::size_t bitmap_file_size = bmImage->file_size(bmImage->_fileName);

   std::size_t bitmap_logical_size = (bmImage->_height * bmImage->_width * bmImage->_bytesPerPixel) +
                                                         (bmImage->_height * padding) +
                                                         bih.struct_size() + bfh.struct_size();

   if(bitmap_file_size != bitmap_logical_size)
   {
       std::cout << "bitmap_load ERROR : Deference between logica and physical size: \n" <<  "Logical: " << bitmap_logical_size << "\nPhysical:  "<< bitmap_file_size;

       bfh.clear();
       bih.clear();

       stream.close();
       return nullptr;
   }

   bmImage->create_bitmap();

   for(unsigned int i = 0; i < bmImage->_height; ++i)
   {
        char* data_ptr = bmImage->row(bmImage->_height - i - 1);                                   //reading in inverted way
       stream.read(reinterpret_cast<char*>(data_ptr), sizeof(char) * bmImage->_bytesPerPixel * bmImage->_width);
       stream.read(padding_data, padding);
   }

   return bmImage;
}

void bitmap_i_o::create_bitmap()
{
    _rowIncrement = _width * _bytesPerPixel;
    _data.resize(_height * _rowIncrement );
}

unsigned short bitmap_i_o::swap(const unsigned short& s)
{
    return ((s >> 8) | (s << 8));
}

unsigned int bitmap_i_o::swap(const unsigned int &s)
{
    return (
             ((s & 0xFF000000) >> 0x18) |
             ((s & 0x000000FF) << 0x18) |
             ((s & 0x00FF0000) >> 0x08) |
             ((s & 0x0000FF00) << 0x08));
}

std::size_t bitmap_i_o::file_size(const std::string &fileName) const
{
    std::ifstream file(fileName.c_str(), std::ios::in | std::ios::binary);
    if(!file)
    {
        return  0;
    }
    file.seekg(0, std::ios::end);
    return static_cast<std::size_t>(file.tellg());
}

char *bitmap_i_o::row(unsigned int row_index) const
{
    return const_cast< char*>(&_data[(row_index * _rowIncrement)]);
}

void bitmap_i_o::save(const std::string& fileName)
{
    std::ofstream stream(fileName.c_str(),std::ios::binary);

    if(!stream)
    {
        std::cout << "Can't open file " << fileName << std::endl;
    }

    bitmap_information_header bih;

    bih._width = _width;
    bih._height = _height;
    bih._bitCount = static_cast<unsigned char>(_bytesPerPixel << 3);
    bih._clrImportant = 0;
    bih._clrUsed = 0;
    bih._compression = 0;
    bih._planes = 1;
    bih._size = bih.struct_size();
    bih._xPixelPerMeter = 0;
    bih._yPixelPerMeter = 0;
    bih._sizeImage = (((bih._width * _bytesPerPixel) + 3) & 0x0000FFFC) * bih._height;

    bitmap_file_header bfh;

    bfh._type = 19778;                              //ASCII table "16th"
    bfh._size = bfh.struct_size() + bih.struct_size() + bih._sizeImage;
    bfh._reserved1 = 0;
    bfh._reserved2 = 0;
    bfh._offBits = bih.struct_size() + bfh.struct_size();

    write_bfh(stream,bfh);
    write_bih(stream,bih);

    unsigned int padding = (4 - ((3 * _width) % 4)) % 4;
    char padding_data[4] = {0x00, 0x00, 0x00, 0x00};

    for(unsigned int i = 0; i < _height; i++)
    {
        const  char* data_ptr = &_data[(_rowIncrement * (_height - i - 1))];
        stream.write(data_ptr, sizeof( char) * _bytesPerPixel * _width);
        stream.write(padding_data,padding);
    }
    stream.close();
}

void bitmap_i_o::print()
{
    unsigned int padding = (4 - ((3 * _width) % 4)) % 4;

    for(unsigned int i = 0; i < _height; i++)
    {

        for(int j = 0; j < ( _bytesPerPixel * _width); j++ )
        {
            std::cout << _data[i *_bytesPerPixel * _width + j] + 1;
        }
        for(int j = 0; j < padding; j++)
        {
            std::cout << "1";
        }
        std::cout << "'\n";
    }

//    for(unsigned int i = 1; i < _data.size(); i++)
//    {
//        int buffer = _data[i] + 1;
//        if(i % 3 == 0)
//        {
//            std::cout <<  buffer;
//        }
//        //int jump = _data.size() / _width;
//        if(i % ((_width - 1)* 3)  == 0)
//          {
//            for(int j = 0; j < padding; j++ )
//            {

//            }
//              std::cout << "\n";
//          }
//    }


}

void bitmap_i_o::clear(const unsigned char ch)
{
    std::fill(_data.begin(), _data.end(), ch);
}

std::string bitmap_i_o::fileName() const
{
    return _fileName;
}

void bitmap_i_o::setFileName(const std::string &fileName)
{
    _fileName = fileName;
}

unsigned int bitmap_i_o::width() const
{
    return _width;
}

void bitmap_i_o::setWidth(unsigned int width)
{
    _width = width;
}

unsigned int bitmap_i_o::height() const
{
    return _height;
}

void bitmap_i_o::setHeight(unsigned int height)
{
    _height = height;
}

void bitmap_i_o::read_bfh (std::ifstream &stream, bitmap_i_o::bitmap_file_header &bfh)
{
          read_from_stream(stream,bfh._type);
          read_from_stream(stream,bfh._size);
          read_from_stream(stream,bfh._reserved1);
          read_from_stream(stream,bfh._reserved2);
          read_from_stream(stream,bfh._offBits);

          if (big_endian())
          {
             bfh._type      = swap(bfh._type);
             bfh._size      = swap(bfh._size);
             bfh._reserved1 = swap(bfh._reserved1);
             bfh._reserved2 = swap(bfh._reserved2);
             bfh._offBits  = swap(bfh._offBits);
          }
}

void bitmap_i_o::write_bfh(std::ofstream &stream, bitmap_i_o::bitmap_file_header &bfh)
{
    if (big_endian())
    {
       write_to_stream(stream,swap(bfh._type));
       write_to_stream(stream,swap(bfh._size));
       write_to_stream(stream,swap(bfh._reserved1));
       write_to_stream(stream,swap(bfh._reserved2));
       write_to_stream(stream,swap(bfh._offBits));
    }
    else
    {
       write_to_stream(stream,bfh._type);
       write_to_stream(stream,bfh._size );
       write_to_stream(stream,bfh._reserved1);
       write_to_stream(stream,bfh._reserved2);
       write_to_stream(stream,bfh._offBits);
    }
}

void bitmap_i_o::read_bih(std::ifstream &stream, bitmap_i_o::bitmap_information_header &bih)
{
      read_from_stream(stream,bih._size);
      read_from_stream(stream,bih._width);
      read_from_stream(stream,bih._height);
      read_from_stream(stream,bih._planes);
      read_from_stream(stream,bih._bitCount);
      read_from_stream(stream,bih._compression);
      read_from_stream(stream,bih._sizeImage);
      read_from_stream(stream,bih._xPixelPerMeter);
      read_from_stream(stream,bih._yPixelPerMeter);
      read_from_stream(stream,bih._clrUsed);
      read_from_stream(stream,bih._clrImportant);

      if(big_endian())
      {
          bih._size = swap(bih._size);
          bih._width = swap(bih._width);
          bih._height = swap(bih._height);
          bih._planes = swap(bih._planes);
          bih._bitCount = swap(bih._bitCount);
          bih._compression = swap(bih._compression);
          bih._sizeImage = swap(bih._sizeImage);
          bih._xPixelPerMeter = swap(bih._xPixelPerMeter);
          bih._yPixelPerMeter = swap(bih._yPixelPerMeter);
          bih._clrUsed = swap(bih._clrUsed);
          bih._clrImportant = swap(bih._clrImportant);
      }
}

void bitmap_i_o::write_bih(std::ofstream &stream, bitmap_i_o::bitmap_information_header &bih)
{
    if (big_endian())
    {
       write_to_stream(stream,swap(bih._size));
       write_to_stream(stream,swap(bih._width));
       write_to_stream(stream,swap(bih._height));
       write_to_stream(stream,swap(bih._planes));
       write_to_stream(stream,swap(bih._bitCount));
       write_to_stream(stream,swap(bih._compression));
       write_to_stream(stream,swap(bih._sizeImage));
       write_to_stream(stream,swap(bih._xPixelPerMeter));
       write_to_stream(stream,swap(bih._yPixelPerMeter));
       write_to_stream(stream,swap(bih._clrUsed));
       write_to_stream(stream,swap(bih._clrImportant));

    }
    else
    {
        write_to_stream(stream,(bih._size));
        write_to_stream(stream,(bih._width));
        write_to_stream(stream,(bih._height));
        write_to_stream(stream,(bih._planes));
        write_to_stream(stream,(bih._bitCount));
        write_to_stream(stream,(bih._compression));
        write_to_stream(stream,(bih._sizeImage));
        write_to_stream(stream,(bih._xPixelPerMeter));
        write_to_stream(stream,(bih._yPixelPerMeter));
        write_to_stream(stream,(bih._clrUsed));
        write_to_stream(stream,(bih._clrImportant));
    }
}

unsigned int bitmap_i_o::rowIncrement() const
{
    return _rowIncrement;
}

void bitmap_i_o::setRowIncrement(unsigned int rowIncrement)
{
    _rowIncrement = rowIncrement;
}

std::vector<char> bitmap_i_o::data() const
{
    return _data;
}

void bitmap_i_o::setData(const std::vector<char> &data)
{
    _data = data;
}

template<typename T>
void bitmap_i_o::read_from_stream(std::ifstream &stream,T &t)
{
    stream.read(reinterpret_cast<char*>(&t), sizeof (T));

}

template<typename T>
void bitmap_i_o::write_to_stream(std::ofstream &stream, const T &t)
{
    stream.write(reinterpret_cast<const char*>(&t), sizeof (T));
}


