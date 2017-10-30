//
// Created by imitschke on 15.08.17.
//

#ifndef LAS_VEGAS_LINEREADER_HPP
#define LAS_VEGAS_LINEREADER_HPP

#include <boost/shared_array.hpp>
#include <lvr/io/DataStruct.hpp>
#include <exception>


enum fileType{
    XYZ, XYZRGB, XYZN, XYZNRGB
};

struct fileAttribut{
    std::string m_filePath;
    size_t m_filePos;
    size_t m_elementAmount;
    fileType m_fileType;
    size_t m_PointBlockSize;
    bool m_ply;
    bool m_binary;
    size_t m_line_element_amount;
};

struct  __attribute__((packed)) xyz
{
    lvr::coord<float> point;
};

struct  __attribute__((packed)) xyzn : xyz
{
    lvr::coord<float> normal;
};

struct  __attribute__((packed)) xyznc : xyzn
{
    lvr::color<unsigned char> color;
};
struct  __attribute__((packed)) xyzc : xyz
{
    lvr::color<unsigned char> color;
};


#include <string>
#include <lvr/io/DataStruct.hpp>



class LineReader
{
public:
    LineReader();
    LineReader(std::string filePath);
    LineReader(std::vector<std::string> filePaths);
    void open(std::string filePath);
    void open(std::vector<std::string> filePaths);
    size_t getNumPoints();
    bool getNextPoint(xyznc& point);
//        boost::shared_array<xyzn> getNextPoints(size_t &return_amount, size_t amount = 1000000);
//        boost::shared_array<xyzc> getNextPoints(size_t &return_amount, size_t amount = 1000000);
//        boost::shared_array<xyznc> getNextPoints(size_t &return_amount, size_t amount = 1000000);
        boost::shared_ptr<void> getNextPoints(size_t &return_amount, size_t amount = 1000000);
    fileType getFileType(size_t i);
    fileType getFileType();
    void rewind(size_t i);
    void rewind();
    bool ok();

    class readException : public std::exception
    {
    public:
        readException(std::string what) : error_msg(what){}
        virtual const char* what() const throw()
        {
            return error_msg.c_str();
        }

    private:
        std::string error_msg;
    };

private:
    std::vector<std::string> m_filePaths;
    std::vector<size_t> m_filePos;
    size_t m_elementAmount;
    fileType m_fileType;
    size_t m_PointBlockSize;
    bool m_ply;
    bool m_binary;
    size_t m_line_element_amount;
    size_t m_numFiles;
    size_t m_currentReadFile;
    bool m_openNextFile;
    std::vector<fileAttribut> m_fileAttributes;
};


#endif //LAS_VEGAS_LINEREADER_H
