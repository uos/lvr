namespace lvr2
{

template<typename T>
boost::optional<T> HDF5Kernel::loadChannelOptional(const std::string& groupName,
    const std::string& datasetName) const
{
    boost::optional<T> ret;

    if(hdf5util::exist(m_hdf5File, groupName))
    {
        HighFive::Group g = hdf5util::getGroup(m_hdf5File, groupName, false);
        ret = load<T>(g, datasetName);
    } 

    return ret;
}


// template<typename T>
// ChannelOptional<T> HDF5Kernel::load(
//     HighFive::Group& g,
//     std::string datasetName) const
// { 
//     ChannelOptional<T> ret;

//     if(m_hdf5File && m_hdf5File->isValid())
//     {
//         if(g.exist(datasetName))
//         {
//             HighFive::DataSet dataset = g.getDataSet(datasetName);
//             std::vector<size_t> dim = dataset.getSpace().getDimensions();
            
//             size_t elementCount = 1;
//             for (auto e : dim)
//                 elementCount *= e;

//             if(elementCount)
//             {
//                 ret = Channel<T>(dim[0], dim[1]);
//                 dataset.read(ret->dataPtr().get());
//             }
//         }
//     } else {
//         throw std::runtime_error("[Hdf5 - ChannelIO]: Hdf5 file not open.");
//     }

//     return ret;
// }


template<typename T>
ChannelOptional<T> HDF5Kernel::loadChannel(std::string groupName,
    std::string datasetName)
{
    return load<T>(groupName, datasetName);
}


template<typename T>
void HDF5Kernel::save(std::string groupName,
        std::string datasetName,
        const Channel<T>& channel) const
{
    HighFive::Group g = hdf5util::getGroup(m_hdf5File, groupName);
    save(g, datasetName, channel);
}


template<typename T>
void HDF5Kernel::save(HighFive::Group& g,
    std::string datasetName,
    const Channel<T>& channel) const
{
    std::vector<hsize_t> chunks = {channel.numElements(), channel.width()};
    save(g, datasetName, channel, chunks);
}


template<typename T>
void HDF5Kernel::save(HighFive::Group& g,
    std::string datasetName,
    const Channel<T>& channel,
    std::vector<hsize_t>& chunkSizes) const
{
    if(m_hdf5File && m_hdf5File->isValid())
    {
        std::vector<size_t > dims = {channel.numElements(), channel.width()};

        HighFive::DataSpace dataSpace(dims);
        HighFive::DataSetCreateProps properties;

        // if(m_file_access->m_chunkSize)
        // {
        //     for(size_t i = 0; i < chunkSizes.size(); i++)
        //     {
        //         if(chunkSizes[i] > dims[i])
        //         {
        //             chunkSizes[i] = dims[i];
        //         }
        //     }
        //     properties.add(HighFive::Chunking(chunkSizes));
        // }
        // if(m_file_access->m_compress)
        // {
        //     //properties.add(HighFive::Shuffle());
        //     properties.add(HighFive::Deflate(9));
        // }
   
        std::unique_ptr<HighFive::DataSet> dataset = hdf5util::createDataset<T>(
            g, datasetName, dataSpace, properties
        );

        const T* ptr = channel.dataPtr().get();
        dataset->write(ptr);
        m_hdf5File->flush();
    } 
    else 
    {
        throw std::runtime_error("[Hdf5IO - ChannelIO]: Hdf5 file not open.");
    }
}

template <typename T>
bool HDF5Kernel::getChannel(const std::string group, const std::string name, boost::optional<AttributeChannel<T>>& channel)  const
{
    // TODO check group for vertex / face attribute and set flag in hdf5 channel
    HighFive::Group g = hdf5util::getGroup(m_hdf5File, "channels");
    if(m_hdf5File && m_hdf5File->isValid())
    {
        if(g.exist(name))
        {
            HighFive::DataSet dataset = g.getDataSet(name);
            std::vector<size_t> dim = dataset.getSpace().getDimensions();

            size_t elementCount = 1;
            for (auto e : dim)
                elementCount *= e;

            if(elementCount)
            {
                channel = Channel<T>(dim[0], dim[1]);
                dataset.read(channel->dataPtr().get());
            }
        }
    }
    else
    {
        throw std::runtime_error("[Hdf5 - ChannelIO]: Hdf5 file not open.");
    }
    return true;
}


template <typename T>
bool HDF5Kernel::addChannel(const std::string group, const std::string name, const AttributeChannel<T>& channel)  const
{
    if(m_hdf5File && m_hdf5File->isValid())
    {
        HighFive::DataSpace dataSpace({channel.numElements(), channel.width()});
        HighFive::DataSetCreateProps properties;

        // if(m_file_access->m_chunkSize)
        // {
        //     properties.add(HighFive::Chunking({channel.numElements(), channel.width()}));
        // }
        // if(m_file_access->m_compress)
        // {
        //     //properties.add(HighFive::Shuffle());
        //     properties.add(HighFive::Deflate(9));
        // }

        // TODO check group for vertex / face attribute and set flag in hdf5 channel
        HighFive::Group g = hdf5util::getGroup(m_hdf5File, "channels");

        std::unique_ptr<HighFive::DataSet> dataset = hdf5util::createDataset<T>(
                g, name, dataSpace, properties);

        const T* ptr = channel.dataPtr().get();
        dataset->write(ptr);
        m_hdf5File->flush();
        std::cout << timestamp << " Added attribute \"" << name << "\" to group \"" << group
                  << "\" to the given HDF5 file!" << std::endl;
    }
    else
    {
        throw std::runtime_error("[Hdf5IO - ChannelIO]: Hdf5 file not open.");
    }
    return true;
}


bool HDF5Kernel::getChannel(const std::string group, const std::string name, FloatChannelOptional& channel)  const
{
    return getChannel<float>(group, name, channel);
}


bool HDF5Kernel::getChannel(const std::string group, const std::string name, IndexChannelOptional& channel)  const
{
    return getChannel<unsigned int>(group, name, channel);
}


bool HDF5Kernel::getChannel(const std::string group, const std::string name, UCharChannelOptional& channel)  const
{
    return getChannel<unsigned char>(group, name, channel);
}


bool HDF5Kernel::addChannel(const std::string group, const std::string name, const FloatChannel& channel)  const
{
    return addChannel<float>(group, name, channel);
}


bool HDF5Kernel::addChannel(const std::string group, const std::string name, const IndexChannel& channel)  const
{
    return addChannel<unsigned int>(group, name, channel);
}


bool HDF5Kernel::addChannel(const std::string group, const std::string name, const UCharChannel& channel)  const
{
    return addChannel<unsigned char>(group, name, channel);
}


// R == 0
template<typename VariantT, int R, typename std::enable_if<R == 0, void>::type* = nullptr>
void saveVChannel(
    const VariantT& vchannel,
    const HDF5Kernel* channel_io,
    HighFive::Group& group,
    std::string name)
{
    if(R == vchannel.type())
    {
        channel_io->save(group, name, vchannel.template extract<typename VariantT::template type_of_index<R> >() );
    } else {
        std::cout << "[VariantChannelIO] WARNING: Nothing was saved" << std::endl;
    }
}

// R != 0
template<typename VariantT, int R, typename std::enable_if<R != 0, void>::type* = nullptr>
void saveVChannel(
    const VariantT& vchannel,
    const HDF5Kernel* channel_io,
    HighFive::Group& group,
    std::string name)
{
    if(R == vchannel.type())
    {
        channel_io->save(group, name, vchannel.template extract<typename VariantT::template type_of_index<R> >() );
    } else {
        saveVChannel<VariantT, R-1>(vchannel, channel_io, group, name);
    }
}



template<typename ...Tp>
void HDF5Kernel::save(
    std::string groupName,
    std::string datasetName,
    const VariantChannel<Tp...>& vchannel) const
{
    // TODO
    HighFive::Group g = hdf5util::getGroup(
        m_hdf5File,
        groupName,
        true
    );

    this->save<Tp...>(g, datasetName, vchannel);
}


template<typename ...Tp>
void HDF5Kernel::save(
    HighFive::Group& group,
    std::string datasetName,
    const VariantChannel<Tp...>& vchannel) const
{
    saveVChannel<VariantChannel<Tp...>, VariantChannel<Tp...>::num_types-1>(vchannel, this, group, datasetName);
}

// R == 0
template<typename VariantChannelT, int R, typename std::enable_if<R == 0, void>::type* = nullptr>
boost::optional<VariantChannelT> loadVChannel(
    HighFive::DataType dtype,
    const HDF5Kernel* channel_io,
    HighFive::Group& group,
    std::string name)
{
    boost::optional<VariantChannelT> ret;
    if(dtype == HighFive::AtomicType<typename VariantChannelT::template type_of_index<R> >())
    {
        auto channel = channel_io->template load<typename VariantChannelT::template type_of_index<R> >(group, name);
        if(channel) {
            ret = *channel;
        }
        return ret;
    } else {
        return ret;
    }
}

// R != 0
template<typename VariantChannelT, int R, typename std::enable_if<R != 0, void>::type* = nullptr>
boost::optional<VariantChannelT> loadVChannel(
    HighFive::DataType dtype,
    const HDF5Kernel* channel_io,
    HighFive::Group& group,
    std::string name)  
{
    boost::optional<VariantChannelT> ret;
    if(dtype == HighFive::AtomicType<typename VariantChannelT::template type_of_index<R> >())
    {
        boost::optional<VariantChannelT> ret;
        auto loaded_channel = channel_io->load<typename VariantChannelT::template type_of_index<R> >(group, name);
        if(loaded_channel)
        {
            ret = *loaded_channel;
        }
        return ret;
    } 
    else 
    {
        return loadVChannel<VariantChannelT, R-1>(dtype, channel_io, group, name);
    }
}


template<typename VariantChannelT>
boost::optional<VariantChannelT> HDF5Kernel::loadDynamic(
    HighFive::DataType dtype,
    HighFive::Group& group,
    std::string name) const
{
    return loadVChannel<VariantChannelT, VariantChannelT::num_types-1>(
        dtype, this, group, name);
}


template<typename VariantChannelT>
boost::optional<VariantChannelT> HDF5Kernel::load(
    std::string groupName,
    std::string datasetName) const 
{
    boost::optional<VariantChannelT> ret;

    if(hdf5util::exist(m_hdf5File, groupName))
    {
        HighFive::Group g = hdf5util::getGroup(m_hdf5File, groupName, false);
        ret = this->load<VariantChannelT>(g, datasetName);
    } else {
        std::cout << "[VariantChannelIO] WARNING: Group " << groupName << " not found." << std::endl;
    }

    return ret;
}


template<typename VariantChannelT>
boost::optional<VariantChannelT> HDF5Kernel::load(
    HighFive::Group& group,
    std::string datasetName) const
{
    boost::optional<VariantChannelT> ret;

    std::unique_ptr<HighFive::DataSet> dataset;

    try {
        dataset = std::make_unique<HighFive::DataSet>(
            group.getDataSet(datasetName)
        );
    } catch(HighFive::DataSetException& ex) {
        std::cout << "[VariantChannelIO] WARNING: Dataset " << datasetName << " not found." << std::endl;
    }

    if(dataset)
    {
        // name is dataset
        ret = loadDynamic<VariantChannelT>(dataset->getDataType(), group, datasetName);
    }

    return ret;
}


template<typename VariantChannelT>
boost::optional<VariantChannelT> HDF5Kernel::loadVariantChannel(
    std::string groupName,
    std::string datasetName) const
{
    return load<VariantChannelT>(groupName, datasetName);
}


} // namespace lvr2