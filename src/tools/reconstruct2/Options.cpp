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
 * Options.cpp
 *
 *  Created on: Nov 21, 2010
 *      Author: Thomas Wiemann
 */

#include "Options.hpp"
#include <lvr/config/lvropenmp.hpp>

#include <fstream>

namespace reconstruct{

using namespace boost::program_options;

Options::Options(int argc, char** argv)
    : BaseOption(argc, argv)
{
    // Create option descriptions
    m_descr.add_options()
        ("help", "Produce help message")
        ("inputFile", value< vector<string> >(), "Input file name. Supported formats are ASCII (.pts, .xyz) and .ply")
        ("voxelsize,v", value<float>(&m_voxelsize)->default_value(10), "Voxelsize of grid used for reconstruction.")
        ("noExtrusion", "Do not extend grid. Can be used  to avoid artefacts in dense data sets but. Disabling will possibly create additional holes in sparse data sets.")
        ("intersections,i", value<int>(&m_intersections)->default_value(-1), "Number of intersections used for reconstruction. If other than -1, voxelsize will calculated automatically.")
        ("pcm,p", value<string>(&m_pcm)->default_value("FLANN"), "Point cloud manager used for point handling and normal estimation. Choose from {STANN, PCL, NABO}.")
        ("ransac", "Set this flag for RANSAC based normal estimation.")
        ("decomposition,d", value<string>(&m_pcm)->default_value("PMC"), "Defines the type of decomposition that is used for the voxels (Standard Marching Cubes (MC), Planar Marching Cubes (PMC), Standard Marching Cubes with sharp feature detection (SF) or Tetraeder (MT) decomposition. Choose from {MC, PMC, MT, SF}")
        ("optimizePlanes,o", "Shift all triangle vertices of a cluster onto their shared plane")
        ("clusterPlanes,c", "Cluster planar regions based on normal threshold, do not shift vertices into regression plane.")
        ("cleanContours", value<int>(&m_cleanContourIterations)->default_value(0), "Remove noise artifacts from contours. Same values are between 2 and 4")
        ("planeIterations", value<int>(&m_planeIterations)->default_value(3), "Number of iterations for plane optimization")
        ("fillHoles,f", value<int>(&m_fillHoles)->default_value(30), "Maximum size for hole filling")
        ("rda", value<int>(&m_rda)->default_value(0), "Remove dangling artifacts, i.e. remove the n smallest not connected surfaces")
        ("pnt", value<float>(&m_planeNormalThreshold)->default_value(0.85), "(Plane Normal Threshold) Normal threshold for plane optimization. Default 0.85 equals about 3 degrees.")
        ("smallRegionThreshold", value<int>(&m_smallRegionThreshold)->default_value(10), "Threshold for small region removal. If 0 nothing will be deleted.")
        ("writeClassificationResult,w", "Write classification results to file 'clusters.clu'")
        ("exportPointNormals,e", "Exports original point cloud data together with normals into a single file called 'pointnormals.ply'")
        ("saveGrid,g", "Writes the generated grid to a file called 'fastgrid.grid. The result can be rendered with qviewer.")
        ("saveOriginalData,s", "Save the original points and the estimated normals together with the reconstruction into one file ('triangle_mesh.ply')")
        ("scanPoseFile", value<string>()->default_value(""), "ASCII file containing scan positions that can be used to flip normals")
        ("kd", value<int>(&m_kd)->default_value(5), "Number of normals used for distance function evaluation")
        ("ki", value<int>(&m_ki)->default_value(10), "Number of normals used in the normal interpolation process")
        ("kn", value<int>(&m_kn)->default_value(10), "Size of k-neighborhood used for normal estimation")
        ("mp", value<int>(&m_minPlaneSize)->default_value(7), "Minimum value for plane optimzation")
        ("retesselate,t", "Retesselate regions that are in a regression plane. Implies --optimizePlanes.")
        ("lft", value<float>(&m_lineFusionThreshold)->default_value(0.01), "(Line Fusion Threshold) Threshold for fusing line segments while tesselating.")
        ("generateTextures", "Generate textures during finalization.")
        ("textureAnalysis", "Enable texture analysis features for texture matchung.")
        ("texelSize", value<float>(&m_texelSize)->default_value(1), "Texel size that determines texture resolution.")
        ("classifier", value<string>(&m_classifier)->default_value("PlaneSimpsons"),"Classfier object used to color the mesh.")
        ("recalcNormals,r", "Always estimate normals, even if given in .ply file.")
        ("threads", value<int>(&m_numThreads)->default_value( lvr::OpenMPConfig::getNumThreads() ), "Number of threads")
        ("sft", value<float>(&m_sft)->default_value(0.9), "Sharp feature threshold when using sharp feature decomposition")
        ("sct", value<float>(&m_sct)->default_value(0.7), "Sharp corner threshold when using sharp feature decomposition")
        ("reductionRatio", value<float>(&m_edgeCollapseReductionRatio)->default_value(0.0), "Percentage of faces to remove via edge-collapse (0.0 means no reduction, 1.0 means to remove all faces which can be removed)")
        ("tp", value<string>(&m_texturePack)->default_value(""), "Path to texture pack")
        ("co", value<string>(&m_statsCoeffs)->default_value(""), "Coefficents file for texture matching based on statistics")
        ("nsc", value<unsigned int>(&m_numStatsColors)->default_value(16), "Number of colors for texture statistics")
        ("nccv", value<unsigned int>(&m_numCCVColors)->default_value(64), "Number of colors for texture matching based on color information")
        ("ct", value<unsigned int>(&m_coherenceThreshold)->default_value(50), "Coherence threshold for texture matching based on color information")
        ("colt", value<float>(&m_colorThreshold)->default_value(FLT_MAX), "Threshold for texture matching based on colors")
        ("stat", value<float>(&m_statsThreshold)->default_value(FLT_MAX), "Threshold for texture matching based on statistics")
        ("feat", value<float>(&m_featuresThreshold)->default_value(FLT_MAX), "Threshold for texture matching based on features")
        ("cro", "Use texture matching based on cross correlation.")
        ("patt", value<float>(&m_patternThreshold)->default_value(100), "Threshold for pattern extraction from textures")
        ("mtv", value<int>(&m_minimumTransformationVotes)->default_value(3), "Minimum number of votes to consider a texture transformation as correct")
    ;

    setup();
}

float Options::getVoxelsize() const
{
    return m_variables["voxelsize"].as<float>();
}

float Options::getSharpFeatureThreshold() const
{
    return m_variables["sft"].as<float>();
}

float Options::getSharpCornerThreshold() const
{
    return m_variables["sct"].as<float>();
}

int Options::getNumThreads() const
{
    return m_variables["threads"].as<int>();
}

int Options::getKi() const
{
    return m_variables["ki"].as<int>();
}

int Options::getKd() const
{
    return m_variables["kd"].as<int>();
}

int Options::getKn() const
{
    return m_variables["kn"].as<int>();
}

int Options::getIntersections() const
{
    return m_variables["intersections"].as<int>();
}

int Options::getPlaneIterations() const
{
    return m_variables["planeIterations"].as<int>();
}

string Options::getInputFileName() const
{
    return (m_variables["inputFile"].as< vector<string> >())[0];
}

string Options::getPCM() const
{
    return (m_variables["pcm"].as< string >());
}

string Options::getClassifier() const
{
    return (m_variables["classifier"].as< string >());
}

string Options::getDecomposition() const
{
    return (m_variables["decomposition"].as< string >());
}

string Options::getScanPoseFile() const
{
    return (m_variables["scanPoseFile"].as<string>());
}

float Options::getEdgeCollapseReductionRatio() const
{
    return (m_variables["reductionRatio"].as<float>());
}

int    Options::getDanglingArtifacts() const
{
    return (m_variables["rda"].as<int> ());
}

int    Options::getFillHoles() const
{
    return (m_variables["fillHoles"].as<int> ());
}

int   Options::getMinPlaneSize() const
{
    return (m_variables["mp"].as<int> ());
}


bool Options::printUsage() const
{
  if(!m_variables.count("inputFile"))
    {
      cout << "Error: You must specify an input file." << endl;
      cout << endl;
      cout << m_descr << endl;
      return true;
    }

  if(m_variables.count("help"))
    {
      cout << endl;
      cout << m_descr << endl;
      return true;
    }
  return false;
}

bool Options::saveFaceNormals() const
{
    return m_variables.count("saveFaceNormals");
}

bool Options::writeClassificationResult() const
{
    return m_variables.count("writeClassificationResult")
            || m_variables.count("w");
}

bool Options::doTextureAnalysis() const
{
    return m_variables.count("textureAnalyis");
}

bool Options::filenameSet() const
{
    return (m_variables["inputFile"].as< vector<string> >()).size() > 0;
}

bool Options::recalcNormals() const
{
    return (m_variables.count("recalcNormals"));
}

bool Options::savePointNormals() const
{
    return (m_variables.count("exportPointNormals"));
}

bool Options::saveNormals() const
{
    return (m_variables.count("saveNormals"));
}

bool Options::saveGrid() const
{
    return (m_variables.count("saveGrid"));
}

bool Options::useRansac() const
{
    return (m_variables.count("ransac"));
}

bool Options::saveOriginalData() const
{
    return (m_variables.count("saveOriginalData"));
}

bool Options::optimizePlanes() const
{
    return m_variables.count("optimizePlanes")
        || m_variables.count("retesselate");
}

bool Options::clusterPlanes() const
{
    return m_variables.count("clusterPlanes");
}

bool Options::extrude() const
{
    if(m_variables.count("noExtrusion"))
    {
        return false;
    }
    else
    {
        return true;
    }
}

bool Options::colorRegions() const
{
    return m_variables.count("colorRegions");
}

bool Options::retesselate() const
{
    return m_variables.count("retesselate");
}

bool Options::generateTextures() const
{
    return m_variables.count("generateTextures");
}

float Options::getNormalThreshold() const
{
    return m_variables["pnt"].as<float>();
}

int Options::getSmallRegionThreshold() const
{
    return m_variables["smallRegionThreshold"].as<int>();
}

int Options::getCleanContourIterations() const
{
    return m_variables["cleanContours"].as<int>();
}

float Options::getTexelSize() const
{
    return m_texelSize;
}

float Options::getLineFusionThreshold() const
{
    return m_variables["lft"].as<float>();
}

string Options::getTexturePack() const
{
    return m_variables["tp"].as<string>();
}

unsigned int Options::getNumStatsColors() const
{
    return m_variables["nsc"].as<unsigned int>();
}

unsigned int Options::getNumCCVColors() const
{
    return m_variables["nccv"].as<unsigned int>();
}

unsigned int Options::getCoherenceThreshold() const
{
    return m_variables["ct"].as<unsigned int>();
}

float Options::getColorThreshold() const
{
    return m_variables["colt"].as<float>();
}

float Options::getStatsThreshold() const
{
    return m_variables["stat"].as<float>();
}

float Options::getFeatureThreshold() const
{
    return m_variables["feat"].as<float>();
}

bool Options::getUseCrossCorr() const
{
    return m_variables.count("cro");
}

float Options::getPatternThreshold() const
{
    return m_variables["patt"].as<float>();
}

int Options::getMinimumTransformationVotes() const
{
    return m_variables["mtv"].as<int>();
}

float* Options::getStatsCoeffs()const
{
    float* result = new float[14];
        std::ifstream in (m_variables["tp"].as<string>().c_str());
    if (in.good())
    {
        for(int i = 0; i < 14; i++)
        {
            in >> result[i];
        }
        in.close();
    }
    else
    {
        for(int i = 0; i < 14; i++)
        {
            result[i] = 0.5;
        }
    }
    return result;
}

Options::~Options() {
    // TODO Auto-generated destructor stub
}

} // namespace reconstruct
