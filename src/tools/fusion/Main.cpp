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
 * Main.cpp
 *
 *  Created on: July 14, 2013
 *      Author: Henning Deeken {hdeeken@uos.de}
 *              Ann-Katrin Häuser {ahaeuser@uos.de}
 */

// Program options for this tool
#include "Options.hpp"

// Local includes
#include <lvr/io/PLYIO.hpp>
#include <lvr/io/Timestamp.hpp>
#include <lvr/io/Progress.hpp>
#include <lvr/io/Model.hpp>
#include <lvr/io/ModelFactory.hpp>

#include <lvr/geometry/ColorVertex.hpp>
#include <lvr/geometry/Fusion.hpp>

#include <iostream>

using namespace lvr;


typedef ColorVertex<float, unsigned char>           fVertex;
typedef Normal<float>                            fNormal;

/**
 * @brief   Main entry point for the LSSR fusion executable
 */
int main(int argc, char** argv)
{
    MeshBufferPtr mesh_buffer1;
    MeshBufferPtr mesh_buffer2;
    // Create an empty mesh
    Fusion<fVertex, fNormal> mesh;
    // Parse command line arguments
    fusion::Options options(argc, argv);
    try
    {
    
        // Exit if options had to generate a usage message
        // (this means required parameters are missing)

        if ( options.printUsage() )
        {
            return 0;
        }

        // Create a mesh loader object
        ModelFactory io_factory;
        ModelPtr model1 = io_factory.readModel( options.getMesh1FileName() );
        ModelPtr model2 = io_factory.readModel( options.getMesh2FileName() );


        // Parse loaded data
        if ( !model1 )
        {
            cout << timestamp << "IO Error: Unable to parse " << options.getMesh1FileName() << endl;
            exit(-1);
        }

        mesh_buffer1 = model1->m_mesh;

        if(!mesh_buffer1)
        {
            cout << timestamp << "Given file contains no supported mesh information" << endl;
            exit(-1);
        }    

        if ( !model2 )
        {
            cout << timestamp << "IO Error: Unable to parse " << options.getMesh2FileName() << endl;
            exit(-1);
        }

        mesh_buffer2= model2->m_mesh;

        if(!mesh_buffer2)
        {
            cout << timestamp << "Given file contains no supported mesh information" << endl;
            exit(-1);
        }
        cout << timestamp << "Successfully loaded both meshes." << endl;
        
        mesh.setVerbosity(options.getVerbosity());
        mesh.setDistanceThreshold(options.getDistanceTreshold());
        
    }catch(...)
        {
            std::cout << "Unable to parse options. Call 'fusion --help' for more information." << std::endl;
        }
    // Load and integrate meshes
    mesh.addMeshAndRemoteIntegrateOnly(mesh_buffer1);
    mesh.addMeshAndRemoteIntegrateOnly(mesh_buffer2);
    
    /*mesh.addMesh(mesh_buffer1);
    mesh.lazyIntegrate();

    mesh.addMesh(mesh_buffer2);
    mesh.integrate();*/

    mesh.finalize();

    // Write Result to .ply
    // Create output model and save to file
    ModelPtr m( new Model( mesh.meshBuffer() ) );
    try{
        if(options.outputFileNameSet())
        {
            ModelFactory::saveModel(m, options.getFusionMeshFileName());
            cout << "Wrote resulting mesh to " << options.getFusionMeshFileName() << endl;
        }
        else
        {
            ModelFactory::saveModel(m, "fusion_mesh.ply");
            cout << "Wrote resulting mesh to " <<  "fusion_mesh.ply" << endl;
        }
    }
    catch(...)
    {
        std::cout << "Unable to parse options. Call 'fusion --help' for more information." << std::endl;
    }

    cout << endl << timestamp << "Program end." << endl;
    return 0;
}