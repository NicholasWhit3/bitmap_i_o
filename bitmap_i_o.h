#ifndef BITMAP_I_O_H
#define BITMAP_I_O_H
#include <fstream>
#include <iostream>
#include <limits>
#include <string>
#include <vector>
#include <cstring>


class bitmap_i_o
{
public:

    bitmap_i_o();
    bitmap_i_o(const std::string& fileName);

    enum channel_mode
                         {
                            rgb_mode = 0,
                            bgr_mode = 1
                         };

    template<typename T>
    void read_from_stream(std::ifstream& stream,T& t);
    template<typename T>
    void write_to_stream(std::ofstream& stream, const T& t);

    bool big_endian();                                                  //direct order of bytes
    static bitmap_i_o* load_bitmap(std::string fileName);
    void create_bitmap();
    unsigned short swap(const unsigned short& s);
    unsigned int swap (const unsigned int& s);
    std::size_t file_size(const std::string& fileName) const;
    char* row(unsigned int row_index) const;
    void save(const std::string& fileName);
    void print();
    void clear(const unsigned char ch = 0x00);

    std::string fileName() const;
    void setFileName(const std::string &fileName);
    unsigned int width() const;
    void setWidth(unsigned int width);
    unsigned int height() const;
    void setHeight(unsigned int weight);
    std::vector<char> data() const;
    void setData(const std::vector<char> &data);
    unsigned int rowIncrement() const;
    void setRowIncrement(unsigned int rowIncrement);

private:


    struct rgb_type
    {
        unsigned char   red;
        unsigned char green;
       unsigned char  blue;
    };
    struct bitmap_file_header
    {
       unsigned short _type;
       unsigned int     _size;
       unsigned short _reserved1;
       unsigned short _reserved2;
       unsigned int     _offBits;

       unsigned int struct_size() const
       {
           return sizeof(_type )    +
                 sizeof(_size)           +
                 sizeof(_reserved1) +
                 sizeof(_reserved2) +
                 sizeof(_offBits ) ;
       }

       void clear()
       {
          std::memset(this, 0x00, sizeof(bitmap_file_header));
       }
    };
    struct bitmap_information_header
    {
       unsigned int   _size;
       unsigned int   _width;
       unsigned int   _height;
       unsigned short _planes;                   //number of Plane (=1) 2D/3D
       unsigned short _bitCount;
       unsigned int   _compression;           // 0 = BI_RGB no compression, 1 = BI_RLE8 8bit RLE encoding,  2 = BI_RLE4 4bit RLE encoding
       unsigned int   _sizeImage;                //  = 0 if compression = 0
       unsigned int   _xPixelPerMeter;       //horizontal
       unsigned int   _yPixelPerMeter;       //vertical
       unsigned int   _clrUsed;
       unsigned int   _clrImportant;

       unsigned int struct_size() const
       {
          return sizeof(_size            ) +
                 sizeof(_width           ) +
                 sizeof(_height          ) +
                 sizeof(_planes          ) +
                 sizeof(_bitCount       ) +
                 sizeof(_compression     ) +
                 sizeof(_sizeImage      ) +
                 sizeof(_xPixelPerMeter) +
                 sizeof(_yPixelPerMeter) +
                 sizeof(_clrUsed        ) +
                 sizeof(_clrImportant   ) ;
       }

       void clear()
       {
          std::memset(this, 0x00, sizeof(bitmap_information_header));
       }
    };

    void read_bfh(std::ifstream& stream, bitmap_file_header& bfh);
    void write_bfh(std::ofstream& stream, bitmap_file_header& bfh);
    void read_bih(std::ifstream& stream, bitmap_information_header& bih);
    void write_bih(std::ofstream& stream, bitmap_information_header& bih);


    std::vector <char> _data;
    std::string _fileName;
    channel_mode _channel_mode;
    unsigned int _width;
    unsigned int _height;
    unsigned char _signature[2];
    unsigned int _dataOffSet;                   //Raster Data
    unsigned int _bytesPerPixel;
    unsigned int _rowIncrement;             //lines


};

#endif // BITMAP_I_O_H
