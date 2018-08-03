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


 /**
 * @file       PLYIO.hpp
 * @brief      I/O support for PLY files.
 * @details    I/O support for PLY files: Reading and writing meshes and
 *             pointclouds, including color information, confidence, intensity
 *             and normals.
 * @author     Lars Kiesow (lkiesow), lkiesow@uos.de
 * @version    110929
 * @date       Created:       2011-09-16 17:28:28
 * @date       Last modified: 2011-09-29 14:23:36
 */


#ifndef __PLY_IO_H__
#define __PLY_IO_H__

#include <lvr2/io/BaseIO.hpp>

#include <rply.h>
#include <stdint.h>
#include <cstdio>
#include <vector>

#include <locale.h>

namespace lvr2
{

/**
 * \class PLYIO PLYIO.hpp "io/PLYIO.hpp"
 * \brief A class for input and output to ply files.
 *
 * The PLYIO class provides functionalities for reading and writing the Polygon
 * File Format, also known as Stanford Triangle Format. Both binary and ascii
 * modes are supported. For the actual file handling the RPly library is used.
 * \n \n
 * The following list is a short description of all handled elements and
 * properties of ply files. In short the elements \c vertex and \c face
 * specifies a mesh and the element \c point specifies a pointcloud. However
 * there is one exception to this: If neither \c point nor \c face is defined,
 * it is assumed that the read vertices are meant to be points and thus are
 * loaded as pointcloud.
\verbatim
ELEMENT vertex
   PROPERTY              x (float)
   PROPERTY              y (float)
   PROPERTY              z (float)
   PROPERTY            red (unsigned char)
   PROPERTY          green (unsigned char)
   PROPERTY           blue (unsigned char)
   PROPERTY             nx (float)
   PROPERTY             ny (float)
   PROPERTY             nz (float)
   PROPERTY      intensity (float)
ELEMENT point
   PROPERTY              x (float)
   PROPERTY              y (float)
   PROPERTY              z (float)
   PROPERTY            red (unsigned char)
   PROPERTY          green (unsigned char)
   PROPERTY           blue (unsigned char)
   PROPERTY             nx (std::cout << it << std::endl;float)
   PROPERTY             ny (float)
   PROPERTY             nz (float)
   PROPERTY      intensity (float)
ELEMENT face
   PROPERTY vertex_indices (LIST uchar int)
   PROPERTY   vertex_index (LIST uchar int)  <<  [only read]
\endverbatim
 */
class PLYIO : public BaseIO
{
    public:
        /**
         * \brief Constructor.
         **/
        PLYIO()
        {
            setlocale (LC_ALL, "C");
            m_model.reset();
        }

        ~PLYIO() {}


        /**
         * \brief Save PLY with previously specified data.
         *
         * Save a PLY file with given filename. The mode is automatically set
         * to little endian binary.
         *
         * \param filename  Filename of the output file.
         **/
        void save( string filename );

        void save(ModelPtr model, string filename)
        {
        	m_model = model;
        	save(filename);
        }


        /**
         * \brief Read specified PLY file.
         *
         * Read a specified PLY file. The additional parameters specify the
         * data to be read. The default is to read all available data.
         *
         * \param filename        Filename of file to read.
         * \param readColor       Specifies if color should be read.
         * \param readConfidence  Specifies if confidence should be read.
         * \param readIntensity   Specifies if intensity should be read.
         * \param readNormals     Specifies if normals should be read.
         * \param readFaces       Specifies if faces should be read.
         **/
        ModelPtr read( string filename, bool readColor, bool readConfidence = true,
                bool readIntensity = true, bool readNormals = true, 
                bool readFaces = true );


        /**
         * \brief Read specified PLY file.
         *
         * Read a specified PLY file with all available data.
         *
         * \param filename        Filename of file to read.
         **/
        ModelPtr read( string filename );


    private:


        /**
         * \brief Callback for read vertices.
         * \param argument  Argument to pass the read data.
         **/
        static int readVertexCb( p_ply_argument argument );


        /**
         * \brief Callback for read color information.
         * \param argument  Argument to pass the read data.
         **/
        static int readColorCb( p_ply_argument argument );


        /**
         * \brief Callback for read faces.
         * \param argument  Argument to pass the read data.
         **/
        static int readFaceCb( p_ply_argument argument );

};

}

#endif
