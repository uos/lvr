#include "lvr2/io/yaml/HyperspectralCamera.hpp"

namespace lvr2 {

template <typename Derived>
void HyperspectralPanoramaIO<Derived>::save(
    const size_t& scanPosNo, 
    const size_t& hCamNo, 
    const size_t& hPanoNo,
    HyperspectralPanoramaPtr pano) const
{
    auto Dgen = m_featureBase->m_description;
    Description d = Dgen->position(scanPosNo);
    d = Dgen->hyperspectralCamera(d, hCamNo);
    d = Dgen->hyperspectralPanorama(d, hPanoNo);

    // std::cout << "[HyperspectralPanoramaIO - save]" << std::endl;
    // std::cout << d << std::endl;

    if(!d.groupName)
    {
        d.groupName = "";
    }


    for(size_t i=0; i<pano->channels.size(); i++)
    {
        m_hyperspectralPanoramaChannelIO->save(scanPosNo, hCamNo, hPanoNo, i, pano->channels[i]);
    }

    if(d.metaName)
    {
        YAML::Node node;
        node = *pano;
        m_featureBase->m_kernel->saveMetaYAML(*d.groupName, *d.metaName, node);
    }

    // store hyperspectral channels

}

template <typename Derived>
HyperspectralPanoramaPtr HyperspectralPanoramaIO<Derived>::load(
    const size_t& scanPosNo,
    const size_t& hCamNo,
    const size_t& hPanoNo) const
{
    HyperspectralPanoramaPtr ret;

    auto Dgen = m_featureBase->m_description;
    Description d = Dgen->position(scanPosNo);
    d = Dgen->hyperspectralCamera(d, hCamNo);
    d = Dgen->hyperspectralPanorama(d, hPanoNo);

    // std::cout << "[HyperspectralPanoramaIO - load]" << std::endl;
    // std::cout << d << std::endl;

    if(!d.groupName)
    {
        d.groupName = "";
    }

    // check if group exists
    if(!m_featureBase->m_kernel->exists(*d.groupName))
    {
        return ret;
    }

    if(d.metaName)
    {
        if(!m_featureBase->m_kernel->exists(*d.groupName, *d.metaName))
        {
            return ret;
        } 

        YAML::Node meta;
        m_featureBase->m_kernel->loadMetaYAML(*d.groupName, *d.metaName, meta);
        ret = std::make_shared<HyperspectralPanorama>(meta.as<HyperspectralPanorama>());
    } else {
        
        // no meta name specified but scan position is there: 
        ret.reset(new HyperspectralPanorama);
    }

    // Load Hyperspectral Channels
    size_t channelNo = 0;
    while(true)
    {
        HyperspectralPanoramaChannelPtr hchannel = m_hyperspectralPanoramaChannelIO->load(scanPosNo, hCamNo, hPanoNo, channelNo);
        if(hchannel)
        {
            ret->channels.push_back(hchannel);
        } else {
            break;
        }
        channelNo++;
    }


    return ret;
}

} // namespace lvr2