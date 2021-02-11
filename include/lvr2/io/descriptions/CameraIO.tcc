#include "lvr2/io/yaml/Camera.hpp"

namespace lvr2
{

template <typename FeatureBase>
void CameraIO<FeatureBase>::save(
    const size_t& scanPosNo,
    const size_t& scanCamNo,
    CameraPtr cameraPtr) const
{
    auto D = m_featureBase->m_description;

    Description d = D->camera(
        D->position(scanPosNo),
        scanCamNo);

    for(size_t scanImageNo = 0; scanImageNo < cameraPtr->images.size(); scanImageNo++)
    {
        m_cameraImageIO->save(scanPosNo, scanCamNo, scanImageNo, cameraPtr->images[scanImageNo]);
    }

    // writing meta
    if(d.metaName)
    {
        YAML::Node node;
        node = *cameraPtr;
        m_featureBase->m_kernel->saveMetaYAML(*d.groupName, *d.metaName, node);
    }
}

template <typename FeatureBase>
CameraPtr CameraIO<FeatureBase>::load(
    const size_t& scanPosNo, 
    const size_t& scanCamNo) const
{
    CameraPtr ret;

    Description d_parent = m_featureBase->m_description->position(scanPosNo); 
    Description d = m_featureBase->m_description->camera(d_parent, scanCamNo);

    if(!d.groupName)
    {
        return ret;
    }

    if(!m_featureBase->m_kernel->exists(*d.groupName))
    {
        return ret;
    }

    if(d.metaName)
    {
        YAML::Node meta;
        m_featureBase->m_kernel->loadMetaYAML(*d.groupName, *d.metaName, meta);
        ret = std::make_shared<Camera>(meta.as<Camera>());
    } else {
        ret.reset(new Camera);
    }

    size_t scanImageNo = 0;
    while(true)
    {
        CameraImagePtr scanImage = m_cameraImageIO->load(scanPosNo, scanCamNo, scanImageNo);
        if(scanImage)
        {
            ret->images.push_back(scanImage);
        } else {
            break;
        }
        scanImageNo++;
    }

    return ret;
}

template <typename FeatureBase>
void CameraIO<FeatureBase>::saveCamera(
    const size_t& scanPosNo, 
    const size_t& scanCamNo, 
    CameraPtr cameraPtr) const
{
    save(scanPosNo, scanCamNo, cameraPtr);
}

template <typename FeatureBase>
CameraPtr CameraIO<FeatureBase>::loadCamera(
    const size_t& scanPosNo, const size_t& scanCamNo) const
{
    return load(scanPosNo, scanCamNo);
}

} // namespace lvr2
