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
 * Texture.hpp
 *
 *  @date 08.09.2011
 *  @author Kim Rinnewitz (krinnewitz@uos.de)
 *  @author Sven Schalk (sschalk@uos.de)
 */

#ifndef TEXTURE_HPP_
#define TEXTURE_HPP_

#include <cstring>
#include <math.h>
#include <cstdio>
#include <lvr/io/PPMIO.hpp>

namespace lvr {


/**
 * @brief    This class represents a texture.
 */
class Texture {
public:


    /**
     * @brief     Constructor.
     *
     */
    Texture( );

    /**
     * @brief     Constructor.
     *
     */
    Texture(unsigned short int width, unsigned short int height, unsigned char numChannels,
        unsigned char numBytesPerChan, unsigned short int textureClass, unsigned short int numFeatures,
        unsigned char numFeatureComponents, float* features, float* keyPoints, float* stats, bool isPattern,
        unsigned char numCCVColors, unsigned long* CCV);

    /**
     * @brief     Constructor.
     *
     */
    Texture(Texture &other);


    /**
     * \brief    Compares two textures based on their distance values
     *
     * \param    t1    The first texture
     * \param    t2    The second texture
      *
     * \return    true if t1->m_distance < t2->m_distance
     */
    static bool cmpTextures(Texture* t1, Texture* t2);

    /**
     * Destructor.
     */
    virtual ~Texture();

    /**
     * @brief    write the texture to an image file
     *
     * @param    i    The number to use in the filename
     *
     */
    void save(int i);

    ///The dimensions of the texture
    unsigned short int m_width, m_height;
    
    ///The texture data
    unsigned char* m_data;

    ///The number of color channels
    unsigned char m_numChannels;

    ///The number of bytes per channel
    unsigned char m_numBytesPerChan;

    ///The class of the texture
    unsigned short int m_textureClass;

    ///The pixel size determines the resolution
    static float m_texelSize;

    ///The precalculated feature descriptors (SIFT/SURF/...)
    float* m_featureDescriptors;

    ///The positions of the precalculated features
    float* m_keyPoints;
    
    ///The number of entries of each feature descriptor
    unsigned char m_numFeatureComponents;

    ///The number of feature descriptors
    unsigned short int m_numFeatures;
    
    ///14 statistical values characterizing the texture
    float* m_stats;

    ///Determines whether this texture is a pattern texture or not
    bool m_isPattern;

    ///Holds the number of colors used for CCV calculation
    unsigned char m_numCCVColors;

    ///CCV
    unsigned long* m_CCV;

    ///value indicating how well this texture fits the reference texture
    float m_distance;

    
};

}

#endif /* TEXTURE_HPP_ */