#include "lvr2/io/yaml/ScanImage.hpp"
namespace lvr2
{


template <typename FeatureBase>
ScanImagePtr ScanImageIO<FeatureBase>::loadScanImage(
    const size_t& scanPos, 
    const size_t& camNr, 
    const size_t& imgNr)
{
    ScanImagePtr ret(new ScanImage);

    Description d = m_featureBase->m_description->scanImage(scanPos, camNr, imgNr);

    // Init default values
    std::stringstream sstr;
    sstr << std::setfill('0') << std::setw(8) << imgNr;
    std::string scanImageName = sstr.str() + ".png";
    std::string metaName = sstr.str() + ".yaml";
    std::string groupName = "";

    if(d.groupName)
    {
        groupName = *d.groupName;
    }

    if(d.dataSetName)
    {
        scanImageName = *d.dataSetName;
    }

    if(d.metaName)
    {
        metaName = *d.metaName;
    }

    if(d.metaData)
    {
        *ret = (*d.metaData).as<ScanImage>();
    }
    else
    {
        std::cout << timestamp << "ScanImageIO::loadScanImage(): Warning: No meta data found for "
                  << groupName << "/" << scanImageName << "." << std::endl;
    }

    ret->imageFile = scanImageName;
    //TODO load data
    return ret;
}

template <typename FeatureBase>
void  ScanImageIO<FeatureBase>::saveScanImage(
    const size_t& scanPos, 
    const size_t& camNr, 
    const size_t& imgNr, 
    ScanImagePtr imgPtr) const
{
    // TODO
    Description d = m_featureBase->m_description->scanImage(scanPos, camNr, imgNr);

    std::cout << "[ScanImageIO] Image " << scanPos << "," << camNr << "," << imgNr <<  " - Description: " << std::endl;
    std::cout << d << std::endl;

    if(d.metaName)
    {
        // add image file to meta
        imgPtr->imageFile = *d.dataSetName;
        YAML::Node node;
        node = *imgPtr;
        m_featureBase->m_kernel->saveMetaYAML(*d.groupName, *d.metaName, node);
    }

    m_imageIO->save(*d.groupName, *d.dataSetName, imgPtr->image);
}

} // namespace lvr2
