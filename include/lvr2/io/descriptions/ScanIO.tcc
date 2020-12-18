#include "lvr2/io/yaml/Scan.hpp"
#include <boost/optional/optional_io.hpp>

namespace lvr2
{

template <typename FeatureBase>
void ScanIO<FeatureBase>::saveScan(const size_t& scanPosNo, const size_t& scanNo, const ScanPtr& scanPtr)
{
    // Default meta yaml

    // Get group and dataset names according to 
    // data fomat description and override defaults if 
    // when possible
    Description d = m_featureBase->m_description->scan(scanPosNo, scanNo);

    std::cout << "[ScanIO] Scan " << scanPosNo << "," << scanNo <<  " - Description: " << std::endl;
    std::cout << d << std::endl;

    if(d.metaName)
    {
        YAML::Node node;
        node = *scanPtr;
        m_featureBase->m_kernel->saveMetaYAML(*d.groupName, *d.metaName, node);
    }

    if(!d.dataSetName)
    {
        // Scan is not a dataset: handle as group of channels
        m_pclIO->savePointCloud(*d.groupName, scanPtr->points);
    } else {
        // Scan is a dataset: write data
        m_pclIO->savePointCloud(*d.groupName, *d.dataSetName, scanPtr->points);
    }
    
    // Save Waveform data
    if (scanPtr->waveform)
    {
	    // std::cout << "[ScanIO]Waveform found " <<std::endl;
        m_fullWaveformIO->saveFullWaveform(scanPosNo, scanNo, scanPtr->waveform);
    }

}

template <typename FeatureBase>
ScanPtr ScanIO<FeatureBase>::loadScan(
    const size_t& scanPosNo, const size_t& scanNo)
{
    ScanPtr ret(new Scan);

    Description d = m_featureBase->m_description->scan(scanPosNo, scanNo);

    // Init default values
    std::stringstream sstr;
    sstr << std::setfill('0') << std::setw(8) << scanNo;
    std::string scanName = sstr.str() + ".ply";
    std::string metaName = sstr.str() + ".yaml";
    std::string groupName = "";

    if(d.groupName)
    {
        groupName = *d.groupName;
    }

    if(d.dataSetName)
    {
        scanName = *d.dataSetName;
    }

    if(d.metaName)
    {
        metaName = *d.metaName;
    }

    // Important! First load meta data as YAML cpp seems to 
    // create a new scan object before calling decode() !!!
    // Cf. https://stackoverflow.com/questions/50807707/yaml-cpp-encoding-decoding-pointers
    if(d.metaData)
    {
        // ret = std::make_shared<Scan>((*d.metaData).as<Scan>());
        *ret = (*d.metaData).as<Scan>();
    }
    else
    {
        std::cout << timestamp << "ScanIO::load(): Warning: No meta data found for "
                  << groupName << "/" << scanName << "." << std::endl;
    }
    std::cout << ret->poseEstimation << std::endl;
    std::cout << ret->registration << std::endl;

    // Load actual data
    ret->points = m_pclIO->loadPointCloud(groupName, scanName);
    /*
    boost::shared_array<float> pointData;
    std::vector<size_t> pointDim;
    pointData = m_featureBase->m_kernel->loadFloatArray(groupName, scanName, pointDim);
    std::cout <<"load PointBuffer" << groupName << " " << scanName << std::endl;
    PointBufferPtr pb = PointBufferPtr(new PointBuffer(pointData, pointDim[0]));
    ret->points = pb;
    */
    // Get Waveform data
    Description waveformDescr = m_featureBase->m_description->waveform(scanPosNo, scanNo);
    if(waveformDescr.dataSetName)
    {
        std::string dataSetName;
        std::tie(groupName, dataSetName) = getNames("", "", waveformDescr);

        if (m_featureBase->m_kernel->exists(groupName))
        {
            std::cout << "[LabelIO] Loading Waveform" << std::endl;
            WaveformPtr fwPtr = m_fullWaveformIO->loadFullWaveform(scanPosNo, scanNo);
            ret->waveform = fwPtr;
            //boost::shared_array<uint16_t> waveformData(new uint16_t[fwPtr->waveformSamples.size()]);
            //std::memcpy(waveformData.get(), fwPtr->waveformSamples.data(), fwPtr->waveformSamples.size() * sizeof(uint16_t));
            //Channel<uint16_t>::Ptr waveformChannel(new Channel<uint16_t>(fwPtr->waveformSamples.size() / fwPtr->maxBucketSize, static_cast<size_t>(fwPtr->maxBucketSize), waveformData));
            //ret->points->addChannel<uint16_t>(waveformChannel, "waveform");
        } else{
            std::cout << "[LabelIO] No Waveform found" << groupName << std::endl;
        }
    }


    return ret;
}

template <typename FeatureBase>
ScanPtr ScanIO<FeatureBase>::loadScan(
    const size_t& scanPosNo, 
    const size_t& scanNo, 
    ReductionAlgorithmPtr reduction)
{
    ScanPtr ret(new Scan);

    Description d = m_featureBase->m_description->scan(scanPosNo, scanNo);

    // Init default values
    std::stringstream sstr;
    sstr << std::setfill('0') << std::setw(8) << scanNo;
    std::string scanName = sstr.str() + ".ply";
    std::string metaName = sstr.str() + ".yaml";
    std::string groupName = "";

    if(d.groupName)
    {
        groupName = *d.groupName;
    }

    if(d.dataSetName)
    {
        scanName = *d.dataSetName;
    }

    if(d.metaName)
    {
        metaName = *d.metaName;
    }

    // Important! First load meta data as YAML cpp seems to 
    // create a new scan object before calling decode() !!!
    // Cf. https://stackoverflow.com/questions/50807707/yaml-cpp-encoding-decoding-pointers
    if(d.metaData)
    {
        // ret = std::make_shared<Scan>((*d.metaData).as<Scan>());
        *ret = (*d.metaData).as<Scan>();
    }
    else
    {
        std::cout << timestamp << "ScanIO::load(): Warning: No meta data found for "
                  << groupName << "/" << scanName << "." << std::endl;
    }
    std::cout << ret->poseEstimation << std::endl;
    std::cout << ret->registration << std::endl;

    // Load actual data
    ret->points = m_pclIO->loadPointCloud(groupName, scanName, reduction);
    /*
    boost::shared_array<float> pointData;
    std::vector<size_t> pointDim;
    pointData = m_featureBase->m_kernel->loadFloatArray(groupName, scanName, pointDim);
    std::cout <<"load PointBuffer" << groupName << " " << scanName << std::endl;
    PointBufferPtr pb = PointBufferPtr(new PointBuffer(pointData, pointDim[0]));
    ret->points = pb;
    */
    // Get Waveform data
    Description waveformDescr = m_featureBase->m_description->waveform(scanPosNo, scanNo);
    if(waveformDescr.dataSetName)
    {
        std::string dataSetName;
        std::tie(groupName, dataSetName) = getNames("", "", waveformDescr);

        if (m_featureBase->m_kernel->exists(groupName))
        {
            std::cout << "[LabelIO] Loading Waveform" << std::endl;
            WaveformPtr fwPtr = m_fullWaveformIO->loadFullWaveform(scanPosNo, scanNo);
            ret->waveform = fwPtr;
            //boost::shared_array<uint16_t> waveformData(new uint16_t[fwPtr->waveformSamples.size()]);
            //std::memcpy(waveformData.get(), fwPtr->waveformSamples.data(), fwPtr->waveformSamples.size() * sizeof(uint16_t));
            //Channel<uint16_t>::Ptr waveformChannel(new Channel<uint16_t>(fwPtr->waveformSamples.size() / fwPtr->maxBucketSize, static_cast<size_t>(fwPtr->maxBucketSize), waveformData));
            //ret->points->addChannel<uint16_t>(waveformChannel, "waveform");
        } else{
            std::cout << "[LabelIO] No Waveform found" << groupName << std::endl;
        }
    }


    return ret;
}


template <typename FeatureBase>
bool ScanIO<FeatureBase>::isScan(const std::string& name)
{
    return true;
}

} // namespace lvr2
