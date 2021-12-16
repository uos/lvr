namespace lvr2
{

template <typename FeatureBase>
void CameraImageGroupIO<FeatureBase>::save(
    const size_t& scanPosNo,
    const size_t& camNo,
    const size_t& imgNo,
    CameraImageGroupPtr imgPtr) const
{
    // save recursively
    std::vector<size_t> imgNos = {imgNo};
    save(scanPosNo, camNo, imgNos, imgPtr);
}

template <typename FeatureBase>
void CameraImageGroupIO<FeatureBase>::save(
    const size_t& scanPosNo,
    const size_t& camNo,
    const std::vector<size_t>& imgNos,
    CameraImageGroupPtr imgPtr) const
{
    auto Dgen = m_featureBase->m_description;
    Description d = Dgen->cameraImageGroup(scanPosNo, camNo, imgNos);

    // save data
    for(size_t i=0; i<imgPtr->images.size(); i++)
    {
        // deep copy
        std::vector<size_t> imgNos_ = imgNos;
        // append id
        imgNos_.push_back(i);

        CameraImageOrGroup img_or_group = imgPtr->images[i];

        if(img_or_group.is_type<CameraImagePtr>())
        {
            // break recursion
            CameraImagePtr img;
            img <<= img_or_group;
            m_cameraImageIO->save(scanPosNo, camNo, imgNos_, img);
        } else {
            // recurse
            CameraImageGroupPtr group;
            group <<= img_or_group;
            save(scanPosNo, camNo, imgNos_, group);
        }
    }

    if(d.meta)
    {
        YAML::Node node;
        node = *imgPtr;
        m_featureBase->m_kernel->saveMetaYAML(*d.metaRoot, *d.meta, node);
    }
}

template <typename FeatureBase>
CameraImageGroupPtr CameraImageGroupIO<FeatureBase>::load(
    const size_t& scanPosNo,
    const size_t& camNo,
    const size_t& imgNo) const
{
    std::vector<size_t> imgNos = {imgNo};
    return load(scanPosNo, camNo, imgNos);
}

template <typename FeatureBase>
CameraImageGroupPtr CameraImageGroupIO<FeatureBase>::load(
    const size_t& scanPosNo,
    const size_t& camNo,
    const std::vector<size_t>& imgNos) const
{
    CameraImageGroupPtr ret;

    auto Dgen = m_featureBase->m_description;
    Description d = Dgen->cameraImageGroup(scanPosNo, camNo, imgNos);

    if(!d.dataRoot)
    {
        return ret;
    }

    if(!m_featureBase->m_kernel->exists(*d.dataRoot))
    {
        return ret;
    }
    
    if(d.meta)
    {   
        YAML::Node meta;
        if(!m_featureBase->m_kernel->loadMetaYAML(*d.metaRoot, *d.meta, meta))
        {
            return ret;
        }

        CameraImageGroupPtr group(new CameraImageGroup);

        if(YAML::convert<CameraImageGroup>::decode(meta, *group) )
        {
            // success
            ret = group;
        } else {
            // meta seems to be some other type: return empty pointer
            return ret;
        }
    } else {
        ret.reset(new CameraImageGroup);
    }
    
    if(ret)
    {
        // it is a group!
        // load data

        for(size_t i=0;;i++)
        {
            std::vector<size_t> imgNos_ = imgNos;
            imgNos_.push_back(i);

            // break recursion
            CameraImagePtr img = m_cameraImageIO->load(scanPosNo, camNo, imgNos_);
            if(img)
            {
                ret->images.push_back(img);
            }
            else
            {
                // recursion
                CameraImageGroupPtr group = load(scanPosNo, camNo, imgNos_);
                if(group)
                {
                    ret->images.push_back(group);
                } else {
                    // neither group nor image -> break loop
                    break;
                }
            }
        }
    }

    return ret;
}

template <typename FeatureBase>
boost::optional<YAML::Node> CameraImageGroupIO<FeatureBase>::loadMeta(
    const size_t& scanPosNo,
    const size_t& camNo,
    const size_t& imgNo) const
{
    std::vector<size_t> imgNos = {imgNo};
    return loadMeta(scanPosNo, camNo, imgNos);
}

template <typename FeatureBase>
boost::optional<YAML::Node> CameraImageGroupIO<FeatureBase>::loadMeta(
    const size_t& scanPosNo,
    const size_t& camNo,
    const std::vector<size_t>& imgNos) const
{
    Description d = m_featureBase->m_description->cameraImageGroup(scanPosNo, camNo, imgNos);
    return m_metaIO->load(d); 
}

template <typename FeatureBase>
void CameraImageGroupIO<FeatureBase>::saveCameraImage(
    const size_t& scanPosNr, 
    const size_t& camNr, 
    const size_t& imgNr, 
    CameraImageGroupPtr imgPtr) const
{
    save(scanPosNr, camNr, imgNr, imgPtr);
}

template <typename FeatureBase>
CameraImageGroupPtr CameraImageGroupIO<FeatureBase>::loadCameraImage(
    const size_t& scanPosNr, 
    const size_t& camNr, 
    const size_t& imgNr) const
{
    return load(scanPosNr, camNr, imgNr);
}

} // namespace lvr2
