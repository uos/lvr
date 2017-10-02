/* Copyright (C) 2011 Uni Osnabrück
 * This file is part of the LAS VEGAS Reconstruction Toolkit,
 *
 * LAS VEGAS is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * LAS VEGAS is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA
 */


/*
 * Texturizer.hpp
 *
 *  @date 03.05.2012
 *  @author Kim Rinnewitz (krinnewitz@uos.de)
 */

#ifndef TEXTURIZER_HPP_
#define TEXTURIZER_HPP_

#include <lvr/reconstruction/PointsetSurface.hpp>
#include <lvr/io/TextureIO.hpp>
#include "TextureToken.hpp"
#include "ImageProcessor.hpp"
#include "Transform.hpp"
#include <string>

namespace lvr {

/**
 * @brief    This class creates textures
 */
template<typename VertexT, typename NormalT>
class Texturizer {
public:


    /**
     * @brief     Constructor.
     *
     * @param     pm     A PointCloudManager containing a colored PointCloud
     *
     */
    Texturizer( typename PointsetSurface<VertexT>::Ptr pm);

    /**
     * @brief   Destructor.
     */
    ~Texturizer();

    /**
     * @brief     Creates or searches a texture for the region given by its' contour
     *
     * @param    contour    The contour of the region to create or find a texture for
     *
     * @return    A TextureToken containing the generated texture
     *
    **/
    TextureToken<VertexT, NormalT>* texturizePlane(vector<VertexT> contour);

    
    int m_stats_texturizedPlanes;
    int m_stats_matchedIndTextures;
    int m_stats_matchedPatTextures;
    int m_stats_extractedPatterns;


    ///File name of texture pack
    static string m_filename;

    ///Number of colors to use for stats calculations
    static unsigned int m_numStatsColors;

    ///Number of colors to use for CCV calculation
    static unsigned int m_numCCVColors;

    ///coherence threshold for CCV calculation
    static unsigned int m_coherenceThreshold;

    ///Threshold for color based texture filtering
    static float m_colorThreshold;

    ///Threshold for cross correlation based texture filtering
    static bool m_useCrossCorr;

    ///Threshold for statistics based texture filtering
    static float m_statsThreshold;

    ///Threshold for feature based texture filtering
    static float m_featureThreshold;

    ///Threshold for pattern extraction
    static float m_patternThreshold;

    /// Flag for texture analysis
    static bool  m_doAnalysis;


    void writePlaneTexels(vector<VertexT> contour, int index);


private:

    /**
     * @brief     Creates a texture for the region given by its' contour using the colored point cloud
     *
     * @param    contour    The contour of the region to create a texture for
     *
     * @return    A TextureToken containing the generated texture
     *
    **/
    TextureToken<VertexT, NormalT>* createInitialTexture(vector<VertexT> contour);



    /**
     * \brief     Filters the given set of textures with the help of histograms and 
     *        color coherence vectors (CCVs)
     *
     * \param    textures    The set of textures to filter
     * \param    refTexture    The texture to compare the textures from the set with
     * \param    threshold    The threshold to determine if a texture is to be kept or deleted
     */
    void filterByColor(std::vector<Texture*> &textures, Texture* refTexture, float threshold);

    /**
     * \brief     Filters the given set of textures with the help of cross correlation
     *
     * \param    textures    The set of textures to filter
     * \param    refTexture    The texture to compare the textures from the set with
     */
    void filterByCrossCorr(std::vector<Texture*> &textures, Texture* refTexture);

    /**
     * \brief     Filters the given set of textures with the help of statistics
     *
     * \param    textures    The set of textures to filter
     * \param    refTexture    The texture to compare the textures from the set with
     * \param    threshold    The threshold to determine if a texture is to be kept or deleted
     */
    void filterByStats(std::vector<Texture*> &textures, Texture* refTexture, float threshold);

    /**
     * \brief     Filters the given set of textures with the help of image features
     *
     * \param    textures    The set of textures to filter
     * \param    refTexture    The texture to compare the textures from the set with
     * \param    threshold    The threshold to determine if a texture is to be kept or deleted
     */
    void filterByFeatures(std::vector<Texture*> &textures, Texture* refTexture, float threshold);

    /**
     * \brief     Filters the given set of textures with the help of the texture class and 
     *        the normal of the plane that needs to be textured
     *
     * \param    textures    The set of textures to filter
     * \param    contour        The contour of the plane to be textured
     */
    void filterByNormal(std::vector<Texture*> &textures, vector<VertexT> contour);

    /**
     * \brief     Holds the classification for different normal directions.
     *
     * \param    n    A normal to get the texture class for
     *
     * \return    The texture class for the given normal
     */
    static unsigned short int classifyNormal(NormalT n);
    
    ///The point set surface used to generate textures from the point cloud
    typename PointsetSurface<VertexT>::Ptr m_pm;

    ///A reference to the texture package
    TextureIO* m_tio;

    //TODO: remove
    void showTexture(TextureToken<VertexT, NormalT>* tt, string caption);
    //TODO: remove
    void markTexture(TextureToken<VertexT, NormalT>* tt, char color);
};
    
template<typename VertexT, typename NormalT>
inline ostream& operator<<(ostream& os, const Texturizer<VertexT, NormalT> t)
{
    os <<"-----------Texturizer statistiscs-----------"<<endl;
    os << "Texturized Planes: " <<t.m_stats_texturizedPlanes<<endl;
    os << "Matched individual textures: "<<t.m_stats_matchedIndTextures<<endl;
    os << "Matched pattern textures: "<<t.m_stats_matchedPatTextures<<endl;
    os << "Extracted Patterns: "<<t.m_stats_extractedPatterns<<endl;
    os <<"--------------------------------------------"<<endl;
    return os;
}

}

#include "Texturizer.tcc"

#endif /* TEXTURIZER_HPP_ */