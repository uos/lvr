/**
 * Copyright (c) 2019, University Osnabrück
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of the University Osnabrück nor the
 *       names of its contributors may be used to endorse or promote products
 *       derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL University Osnabrück BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/**
 * Main.cpp
 *
 *  @date Apr 25, 2019
 *  @author Malte Hillmann
 */

#include <lvr2/io/ModelFactory.hpp>
#include <lvr2/io/IOUtils.hpp>
#include <lvr2/registration/SLAMAlign.hpp>
#include <lvr2/io/HDF5IO.hpp>

#include <boost/program_options.hpp>
#include <boost/filesystem.hpp>
#include <iostream>
#include <chrono>
#include <fstream>

using namespace lvr2;
using namespace std;
using boost::filesystem::path;

string format_name(const string& format, int index)
{
    size_t size = snprintf(nullptr, 0, format.c_str(), index) + 1; // Extra space for '\0'
    char buff[size];
    snprintf(buff, size, format.c_str(), index);
    return string(buff);
}


string map_format(const string& format)
{
    if (format == "uos")
    {
        return "scan%03i.3d";
    }
    else if (format == "riegl_txt")
    {
        return "scan%03i.txt";
    }
    else if (format == "ply")
    {
        return "scan%03i.ply";
    }
    else
    {
        throw boost::program_options::error(string("Unknown Output format: ") + format);
    }
}

int main(int argc, char** argv)
{
    // =============== parse options ===============
    SLAMOptions options;

    path dir;
    int start = -1;
    int end = -1;
    string format = "uos";
    string pose_format = "pose";
    bool isHDF = false;

    bool write_scans = false;
    string output_format;
    bool write_pose = false;
    string output_pose_format;
    bool no_frames = false;
    path output_dir;

    bool help;

    try
    {
        using namespace boost::program_options;

        options_description general_options("General Options");
        options_description icp_options("ICP Options");
        options_description loopclosing_options("Loopclosing Options");

        general_options.add_options()
        ("start,s", value<int>(&start)->default_value(start),
         "The first Scan to process.\n"
         "-1 (default): Search for first Scan.")

        ("end,e", value<int>(&end)->default_value(end),
         "The last Scan to process.\n"
         "-1 (default): Continue until no more Scans found.")

        ("format,f", value<string>(&format)->default_value(format),
         "The format of the Scans in <dir>.\n"
         "This can be a predefined Format (uos, ply, riegl_txt), or a printf Format String like \"scan%03i.3d\",\n"
         "containing one %i or %d that will be replaced with the Scan Index.")

        ("pose-format", value<string>(&pose_format)->default_value(pose_format),
         "The File extension of the Pose files.\n"
         "Currently supported are: pose, dat, frames.")

        ("reduction,r", value<double>(&options.reduction)->default_value(options.reduction),
         "The Voxel size for Octree based reduction.\n"
         "-1 (default): No reduction.")

        ("min,m", value<double>(&options.minDistance)->default_value(options.minDistance),
         "Ignore all Points closer than <value> to the origin of the Scan.\n"
         "-1 (default): No filter.")

        ("max,M", value<double>(&options.maxDistance)->default_value(options.maxDistance),
         "Ignore all Points farther away than <value> from the origin of the Scan.\n"
         "-1 (default): No filter.")

        ("trustPose,p", bool_switch(&options.trustPose),
         "Use the unmodified Pose for ICP. Useful for GPS Poses or unordered Scans.\n"
         "false (default): Apply the relative refinement of previous Scans.")

        ("metascan", bool_switch(&options.metascan),
         "Match Scans to the combined Pointcloud of all previous Scans instead of just the last Scan.")

        ("noFrames,F", bool_switch(&no_frames),
         "Don't write \".frames\" files.")

        ("writePose,w", value<string>(&output_pose_format)->implicit_value("<pose-format>"),
         "Write Poses to directory specified by --output.")

        ("writeScans,W", value<string>(&output_format)->implicit_value("<format>"),
         "Write Scans to directory specified by --output.")

        ("output,o", value<path>(&output_dir),
         "Changes output directory of --writePose and --writeScans. Does not affect \".frames\" files\n"
         "default: <dir>/output.")

        ("verbose,v", bool_switch(&options.verbose),
         "Show more detailed output. Useful for fine-tuning Parameters or debugging.")

        ("hdf,H", bool_switch(&options.useHDF),
         "Opens the given hdf5 file. Then registrates all scans in '/raw/scans/'\nthat are named after the scheme: 'position_00001' where '1' is the scans number.\nAfter registration the calculated poses are written to the finalPose dataset in the hdf5 file.\n")

        ("help,h", bool_switch(&help),
         "Print this help. Seriously how are you reading this if you don't know the --help Option?")
        ;

        icp_options.add_options()
        ("icpIterations,i", value<int>(&options.icpIterations)->default_value(options.icpIterations),
         "Number of iterations for ICP.\n"
         "ICP should ideally converge before this number is met, but this number places an upper Bound on calculation time.")

        ("icpMaxDistance,d", value<double>(&options.icpMaxDistance)->default_value(options.icpMaxDistance),
         "The maximum distance between two points during ICP.")

        ("maxLeafSize", value<int>(&options.maxLeafSize)->default_value(options.maxLeafSize),
         "The maximum number of Points in a Leaf of a KDTree.")

        ("epsilon", value<double>(&options.epsilon)->default_value(options.epsilon),
         "The epsilon difference between ICP-errors for the stop criterion of ICP.")
        ;

        loopclosing_options.add_options()
        ("loopClosing,L", bool_switch(&options.doLoopClosing),
         "Use simple Loopclosing.\n"
         "At least one of -L and -G must be specified for Loopclosing to take place.")

        ("graphSlam,G", bool_switch(&options.doGraphSLAM),
         "Use complex Loop Closing with GraphSLAM.\n"
         "At least one of -L and -G must be specified for Loopclosing to take place.")

        ("closeLoopDistance,c", value<double>(&options.closeLoopDistance)->default_value(options.closeLoopDistance),
         "The maximum distance between two poses to consider a closed loop in Loopclosing or an Edge in the GraphSLAM Graph.\n"
         "Mutually exclusive to --closeLoopPairs.")

        ("closeLoopPairs,C", value<int>(&options.closeLoopPairs)->default_value(options.closeLoopPairs),
         "The minimum pair overlap between two poses to consider a closed loop in Loopclosing or an Edge in the GraphSLAM Graph.\n"
         "Mutually exclusive to --closeLoopDistance.\n"
         "Pairs are judged using slamMaxDistance.\n"
         "-1 (default): use --closeLoopDistance instead.")

        ("loopSize,l", value<int>(&options.loopSize)->default_value(options.loopSize),
         "The minimum number of Scans to be considered a Loop to prevent Loopclosing from triggering on adjacent Scans.\n"
         "Also used in GraphSLAM when considering other Scans for Edges\n"
         "For Loopclosing, this value needs to be at least 6, for GraphSLAM at least 1.")

        ("slamIterations,I", value<int>(&options.slamIterations)->default_value(options.slamIterations),
         "Number of iterations for SLAM.")

        ("slamMaxDistance,D", value<double>(&options.slamMaxDistance)->default_value(options.slamMaxDistance),
         "The maximum distance between two points during SLAM.")

        ("slamEpsilon", value<double>(&options.slamEpsilon)->default_value(options.slamEpsilon),
         "The epsilon difference of SLAM corrections for the stop criterion of SLAM.")
        ;

        options_description hidden_options("hidden_options");
        hidden_options.add_options()
        ("dir", value<path>(&dir))
        ;

        positional_options_description pos;
        pos.add("dir", 1);

        options_description all_options("options");
        all_options.add(general_options).add(icp_options).add(loopclosing_options).add(hidden_options);

        variables_map variables;
        store(command_line_parser(argc, argv).options(all_options).positional(pos).run(), variables);
        notify(variables);

        if (help)
        {
            cout << "The Scan Registration Tool" << endl;
            cout << "Usage: " << endl;
            cout << "\tlvr2_registration [OPTIONS] <dir>" << endl;
            cout << endl;
            general_options.print(cout);
            cout << endl;
            icp_options.print(cout);
            cout << endl;
            loopclosing_options.print(cout);
            cout << endl;
            cout << "<dir> is the directory to search scans in" << endl;
            return EXIT_SUCCESS;
        }

        if (variables.count("dir") != 1)
        {
            throw error("Missing <dir> Parameter");
        }

        if (variables.count("output") == 0)
        {
            output_dir = dir / "output";
        }

        if (format.find('%') == string::npos)
        {
            format = map_format(format);
        }

        if (variables.count("writePose") == 1)
        {
            write_pose = true;
            if (output_pose_format[0] == '<')
            {
                output_pose_format = pose_format;
            }
        }
        if (variables.count("writeScans") == 1)
        {
            write_scans = true;
            if (output_format[0] == '<')
            {
                output_format = format;
            }
            else if (output_format.find('%') == string::npos)
            {
                format = map_format(format);
            }
        }

        options.createFrames = !no_frames;
    }
    catch (const boost::program_options::error& ex)
    {
        std::cerr << ex.what() << endl;
        std::cerr << endl;
        std::cerr << "Use '--help' to see the list of possible options" << endl;
        return EXIT_FAILURE;
    }

    // shared pointer containing the HDF file
    shared_ptr<HDF5IO> inHDF;

    if(options.useHDF)
    {
        cout << "Es wird HDF5 benutzt!" << endl;
        ifstream f(dir.c_str());
        if (f.good())
        {
            // data.h5 exists
            //  dir now contains the file name
            inHDF.reset(new HDF5IO(dir.c_str(), "scans", HighFive::File::ReadWrite));
            // set start to 0, so the scan searching is not triggered
            start = 0;
        }
        else
        {
            cerr << "The given HDF5 file could not be opened! Oben" << endl;
            return EXIT_FAILURE;
        }
    }

    // =============== search scans ===============
    if (start == -1)
    {
        if (!options.useHDF)
        {
            for (int i = 0; i < 100; i++)
            {
                path file = dir / format_name(format, i);
                if (exists(file))
                {
                    start = i;
                    cout << "First scan: " << file.filename() << endl;
                    break;
                }
            }
            if (start == -1)
            {
                cerr << "Could not find a starting scan. are you using the right format?" << endl;
                return EXIT_FAILURE;
            }
        }
    }
    if (!options.useHDF)
    {
        // make sure all scan and pose files are in the directory
        for (int i = start; end == -1 || i <= end; i++)
        {
            path file = dir / format_name(format, i);
            if (!exists(file))
            {
                if (end != -1 || i == start)
                {
                    cerr << "Missing scan " << file.filename() << endl;
                    return EXIT_FAILURE;
                }
                end = i - 1;
                cout << "Last scan: \"" << format_name(format, end) << '"' << endl;
                break;
            }
            file.replace_extension(pose_format);
            if (!exists(file))
            {
                cerr << "Missing pose file " << file.filename() << endl;
                return EXIT_FAILURE;
            }
        }
    }

    SLAMAlign align(options);
    vector<SLAMScanPtr> scans;

    int count = end - start + 1;

    // contains all the scans from the hdf file
    vector<lvr2::ScanPtr> rawScans = inHDF->getRawScans();
    if (options.useHDF)
    {
        // for loop handles each scan in HDF FILE
        for (int i = 0; i < rawScans.size(); i++)
        {
            SLAMScanPtr slamScan = SLAMScanPtr(new SLAMScanWrapper(rawScans.at(i)));
            scans.push_back(slamScan);
            align.addScan(slamScan);
        }
    }
    else
    {
        // case for not using HDF5
        // TODO: change to ScanDirectoryParser once that is done

        for (int i = 0; i < count; i++)
        {
            path file = dir / format_name(format, start + i);
            auto model = ModelFactory::readModel(file.string());

            if (!model)
            {
                cerr << "Unable to read Model from: " << file.string() << endl;
                return EXIT_FAILURE;
            }
            if (!model->m_pointCloud)
            {
                cerr << "file does not contain Points: " << file.string() << endl;
                return EXIT_FAILURE;
            }

            file.replace_extension(pose_format);
            Transformd pose = getTransformationFromFile<double>(file);

            ScanPtr scan = ScanPtr(new Scan());
            scan->m_points = model->m_pointCloud;
            scan->m_poseEstimation = pose;

            SLAMScanPtr slamScan = SLAMScanPtr(new SLAMScanWrapper(scan));
            scans.push_back(slamScan);
            align.addScan(slamScan);
        }
    }
    

    auto start_time = chrono::steady_clock::now();

    align.finish();

    auto required_time = chrono::steady_clock::now() - start_time;
    cout << "SLAM finished in " << required_time.count() / 1e9 << " seconds" << endl;

    if (write_pose || write_scans)
    {
        create_directories(output_dir);
    }

    path file;

    if (options.useHDF)
    {
        // write poses to hdf
        for(int i = 0; i < scans.size(); i++)
        {
            auto pose = scans[i]->pose();
            string scanGroup = "/raw/scans/position_";
            // scanstring contains the scan number with leading zeroes
            string scanNumber = to_string(i);
            string scanString = string(5 - scanNumber.length(), '0').append(scanNumber);
            // the pose is represented as a 4x4 matrix
            std::vector<size_t> dimPose = {4,4};
            float* pose_data = new float[16];
            // the pose needs to be transposed before writing to hdf
            pose.transposeInPlace();
            copy(pose.data(), pose.data() + 16, pose_data);
            // the pose needs to be transposed before writing to hdf
            boost::shared_array<float> poseArray(pose_data);
            string deleteString = scanGroup.append(scanString);
            inHDF.get()->deleteDataset(string(deleteString + "/finalPose").data());
            //rewrite the newly calculated pose
            inHDF.get()->addArray(string("/raw/scans/position_").append(scanString+"/"), string("finalPose"), dimPose, poseArray);
        }
    }

    for (int i = 0; i < count; i++)
    {
        auto& scan = scans[i];

        if (!no_frames)
        {
            file = dir / format_name(format, start + i);
            file.replace_extension("frames");

            scan->writeFrames(file.string());
        }

        if (write_pose)
        {
            file = output_dir / format_name(write_scans ? output_format : format, start + i);
            file.replace_extension(output_pose_format);
            ofstream out(file.string());

            auto pose = scan->pose();
            for (int y = 0; y < 4; y++)
            {
                for (int x = 0; x < 4; x++)
                {
                    out << pose(y, x);
                    if (x < 3)
                    {
                        out << " ";
                    }
                }
                out << endl;
            }
        }

        if (write_scans)
        {
            file = output_dir / format_name(output_format, start + i);

            size_t n = scan->numPoints();

            auto model = make_shared<Model>();
            auto pointCloud = make_shared<PointBuffer>();
            floatArr points = floatArr(new float[n * 3]);

            #pragma omp parallel for schedule(static)
            for (size_t i = 0; i < n; i++)
            {
                auto point = scan->rawPoint(i);
                points[i * 3] = point[0];
                points[i * 3 + 1] = point[1];
                points[i * 3 + 2] = point[2];
            }

            pointCloud->setPointArray(points, n);
            model->m_pointCloud = pointCloud;
            ModelFactory::saveModel(model, file.string());
        }
    }
    return EXIT_SUCCESS;
}
