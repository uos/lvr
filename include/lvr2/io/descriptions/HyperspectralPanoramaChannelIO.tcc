#include "lvr2/io/yaml/HyperspectralCamera.hpp"

namespace lvr2 {

template <typename Derived>
void HyperspectralPanoramaChannelIO<Derived>::save(
    const size_t& scanPosNo, 
    const size_t& hCamNo, 
    const size_t& hPanoNo,
    const size_t& channelId,
    HyperspectralPanoramaChannelPtr hchannel) const
{
    auto Dgen = m_featureBase->m_description;
    Description d = Dgen->hyperspectralPanoramaChannel(scanPosNo, hCamNo, hPanoNo, channelId);

    // std::cout << "[HyperspectralPanoramaChannelIO - save]" << std::endl;
    // std::cout << d << std::endl;

    if(!d.dataRoot)
    {
        d.dataRoot = "";
    }

    // save image
    m_imageIO->save(*d.dataRoot, *d.data, hchannel->channel);

    // save meta
    if(d.meta)
    {
        YAML::Node node;
        node = *hchannel;
        m_featureBase->m_kernel->saveMetaYAML(*d.metaRoot, *d.meta, node);
    }
}

template <typename Derived>
HyperspectralPanoramaChannelPtr HyperspectralPanoramaChannelIO<Derived>::load(
    const size_t& scanPosNo,
    const size_t& hCamNo,
    const size_t& hPanoNo,
    const size_t& channelId) const
{
    HyperspectralPanoramaChannelPtr ret;

    auto Dgen = m_featureBase->m_description;

    Description d = Dgen->hyperspectralPanoramaChannel(scanPosNo, hCamNo, hPanoNo, channelId);

    // std::cout << "[HyperspectralPanoramaChannelIO - load]" << std::endl;
    // std::cout << d << std::endl;

    if(!d.dataRoot)
    {
        d.dataRoot = "";
    }

    // check if group exists
    if(!m_featureBase->m_kernel->exists(*d.dataRoot))
    {
        return ret;
    }

    /// META
    if(d.meta)
    {
        if(!m_featureBase->m_kernel->exists(*d.metaRoot, *d.meta))
        {
            return ret;
        } 

        YAML::Node meta;
        m_featureBase->m_kernel->loadMetaYAML(*d.metaRoot, *d.meta, meta);
        ret = std::make_shared<HyperspectralPanoramaChannel>(meta.as<HyperspectralPanoramaChannel>());
    } else {
        
        // no meta name specified but scan position is there: 
        ret.reset(new HyperspectralPanoramaChannel);
    }
    
    /// DATA
    ret->channel = *m_imageIO->load(*d.dataRoot, *d.data);

    return ret;
}

} // namespace lvr2