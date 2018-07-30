#ifndef CHANNELHANDLER_HPP
#define CHANNELHANDLER_HPP

#include <lvr2/io/DataStruct.hpp>

#include <iostream>

namespace lvr2
{

template<typename T>
class AttributeProxy
{
public:
    template<typename BaseVecT>
    AttributeProxy operator=(const BaseVecT& v)
    {
        std::cout << m_ptr << std::endl;
        if( m_ptr && (m_w > 2))
        {
            m_ptr[0] = v.x;
            m_ptr[1] = v.y;
            m_ptr[2] = v.z;
        }
        return *this;
    }

    template<typename BaseVecT>
    AttributeProxy operator+=(const BaseVecT& v)
    {
        //std::cout << m_w << std::endl;
        if( m_ptr && (m_w > 2))
        {
            m_ptr[0] += v.x;
            m_ptr[1] += v.y;
            m_ptr[2] += v.z;
        }
        return *this;
    }

    AttributeProxy(T* pos = nullptr, unsigned w = 0) : m_ptr(pos), m_w(w) {}

    T& operator[](int i)
    {
        if(m_ptr && (i < m_w))
        {
            return m_ptr[i];
        }
        else
        {
            return 0;
        }
    }

private:

    T*              m_ptr;
    unsigned        m_w;
};

template<typename T>
class AttributeChannel
{
public:
    using DataPtr = boost::shared_array<T>;

    AttributeChannel(size_t n, unsigned width)
        : m_width(width), m_numAttributes(n)
    {
        m_data = DataPtr(new T[m_numAttributes * width]);
    }

    AttributeChannel(size_t n, unsigned width, DataPtr ptr)
        : m_numAttributes(n),
          m_width(width),
          m_data(ptr)
    {

    }

    AttributeProxy<T> operator[](const unsigned& idx)
    {
        T* ptr = m_data.get();
        return AttributeProxy<T>(&ptr[idx * m_width], m_width);
    }

    DataPtr&     get() { return m_data;}
    unsigned    width() { return m_width;}
    size_t      n() { return m_numAttributes;}

private:
    size_t          m_numAttributes;
    unsigned        m_width;
    DataPtr         m_data;
};


class ChannelHandler
{
public:
    ChannelHandler() {};

    using FloatChannel = AttributeChannel<float>;
    using UCharChannel = AttributeChannel<unsigned char>;

    using FloatProxy = AttributeProxy<float>;
    using UCharProxy = AttributeProxy<unsigned char>;

    using FloatChannelPtr = std::shared_ptr<FloatChannel>;
    using UCharChannelPtr = std::shared_ptr<UCharChannel>;

    void addFloatChannel(
            floatArr array,
            std::string name,
            size_t n,
            unsigned width);

    void addUCharChannel(
            ucharArr array,
            std::string name,
            size_t n,
            unsigned width);

    void addFloatChannel(
            FloatChannelPtr data,
            std::string name,
            size_t n,
            unsigned width);

    void addUCharChannel(
            UCharChannelPtr data,
            std::string name,
            size_t n,
            unsigned width);

    bool hasUCharChannel(std::string name);
    bool hasFloatChannel(std::string name);

    unsigned ucharChannelWidth(std::string name);
    unsigned floatChannelWidth(std::string name);

    FloatProxy getFloatHandle(int idx, const std::string& name);
    UCharProxy getUCharHandle(int idx, const std::string& name);

    FloatProxy operator[](size_t idx);

    floatArr getFloatArray(size_t& n, unsigned& w, const std::string name);
    ucharArr getUCharArray(size_t& n, unsigned& w, const std::string name);

    FloatChannel& getFloatChannel(std::string name);
    UCharChannel& getUCharChannel(std::string name);

    void addFloatChannel(FloatChannelPtr data, std::string name);
    void addUCharChannel(UCharChannelPtr data, std::string name);
private:

    std::map<std::string, FloatChannelPtr>  m_floatChannels;
    std::map<std::string, UCharChannelPtr>  m_ucharChannels;

    using FloatChannelMap = std::map<std::string, FloatChannelPtr>;
    using UCharChannelMap = std::map<std::string, UCharChannelPtr>;
};

} // namespace lvr2

#endif // CHANNELHANDLER_HPP
