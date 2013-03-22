/*
 * mpi.cpp
 *
 *  Created on: 1.02.2013
 *      Author: Dominik Feldschnieders
 */

/*
 *
 * Programmierbeispiel der Stanfort Universität.
 *  http://www.slac.stanford.edu/comp/unix/farm/mpi.html
 */

#include <iostream>
#include <fstream>
#include <stdio.h>
#include <mpi.h>
#include <unistd.h>

// Las vegas Toolkit
#include "io/PointBuffer.hpp"
#include "io/Model.hpp"
#include "io/ModelFactory.hpp"
#include "src/mpi/KdTree.hpp"
#include "geometry/ColorVertex.hpp"
#include "geometry/Normal.hpp"

#include "reconstruction/AdaptiveKSearchSurface.hpp"

#include <boost/program_options.hpp>

using namespace lssr;
namespace po = boost::program_options;


int main (int argc , char *argv[]) {
  int count_serv = 0;  
  fstream f;
  char test_aufgabe_name[64];
  
  
  Timestamp start;

    int kd, kn, ki; 
    long int max_points;
    
    
    po::options_description desc("Allowed options");
    desc.add_options()
      ("help"      , "produce help message")
      ("kd"        , po::value<int>(&kd)->default_value(40), "set kd")
      ("ki"        , po::value<int>(&ki)->default_value(40), "set ki")
      ("kn"        , po::value<int>(&kn)->default_value(40), "set kn")
      ("maxpoints" , po::value<long int>(&max_points)->default_value(100000), "set maxpoints, min 1000")
      ("file"      , po::value<string>(), "Inputfile")
    ;
    
    po::variables_map vm;
    po::store(po::parse_command_line(argc, argv, desc), vm);
    po::notify(vm);    
  
    if (vm.count("help")) {
      cout << desc << "\n";
      return 1;
    }
        

	// Kd Tree
    // A list for all Nodes with less than MAX_POINTS
    std::list<KdNode<ColorVertex<float, unsigned char>>*> m_nodelist;

	//Las Vegas Toolkit
    // m_ for Master
    // s_ for Slave

	// A shared-Pointer for the model, with the pointcloud in it
	ModelPtr 				m_model;

	// The pointcloud
	PointBufferPtr          m_loader;

	// The currently stored points
	coord3fArr   			m_points;

	coord3fArr               c_normals;

	// Create a point loader object - Aus der Main (reconstruct)
	ModelFactory 			io_factory;

	// Number of points in the point cloud
	size_t 					m_numpoint;

	// Number of points in the point cloud (Child)
	int c_sizepackage;


	// for calculate normals

	// MPI

	// Anzahl an Processen
	int numprocs;
	// Rank / ID des Processes
	int rank;
	int i;

	int namelen = 0;
	char processor_name[MPI_MAX_PROCESSOR_NAME];
	char idstring[32];
	char con_msg[128];
	long int num_all_points = 0;
				

	//if send this, +1 because this the Master in MPI has the rank 0 but for all Indizes this is more pleasant
	int client_serv_data = 0;

	int count = 1;


	// Initializes the connection
	MPI::Init(argc, argv);

	// gives the number of processes
	numprocs = MPI::COMM_WORLD.Get_size();

	// gives the Id / Rang of the processes
	rank = MPI::COMM_WORLD.Get_rank();

	// returns the name of the processor (computer on which it runs)
	MPI::Get_processor_name(processor_name, namelen);


	MPI::Request status[numprocs-1];
	// Master-Process
	if (rank == 0){
		int * int_numpoint = new int[numprocs - 1];
		int * tmp_int = new int[1];
	  
	/*temporär wird noch ausgelesen, später Übergabe durch das Programm */
		//Read the file and get the pointcloud
		m_model = io_factory.readModel( vm["file"].as<string>() );

		if (m_model != NULL)
		{
			m_loader = m_model->m_pointCloud;
		}
std::cout << "k-d tree wird erstellt." << std::endl;
		// Building the Kd tree with max max_points in every packete
		KdTree<cVertex> KDTree(m_loader, max_points);
std::cout << "k-d tree wurde erstellt. YEAH!!!!" << std::endl;
		// get the list with all Nodes with less than MAX_POINTS
		m_nodelist = KDTree.GetList();



		// Send an announcement to all other processes
		for (i = 1; i < numprocs; i++)
		{
			sprintf(con_msg, "Hey Nummer %d...", i);
			MPI::COMM_WORLD.Send(con_msg, 128, MPI::CHAR, i, 0);

		}

		// wait for their answer
		for ( i = 1; i < numprocs; i++)
		{
			MPI::COMM_WORLD.Recv(con_msg, 128, MPI::CHAR, i, 0);
			printf("%s\n", con_msg);
		}

/*************************************** Connection is successful *****************/


		//dynamisches array für die Normalen anlegen
		float ** normals = new float*[numprocs - 1];

		//dynamisches array für die Indices anlegen
		long unsigned int ** Indizes = new long unsigned int*[numprocs - 1];

		// für laufvariable beim zusammenfügen
		int laufvariable[numprocs - 1];


		typename std::list<KdNode<cVertex>*>::	iterator it= m_nodelist.begin();


		// get number of all points
		for ( it = m_nodelist.begin() ; it != m_nodelist.end(); ++it)
		{
			num_all_points += (*it)->getnumpoints();
		}

		it = m_nodelist.begin();

		// a buffer to store all the normals
		float * m_normal = new float[3 * num_all_points];

		// Send all data packets
		while ( it  != m_nodelist.end() )
		{
std::cout << "\n ------------------Anfang der while-Schleife mit dem Process: " << client_serv_data + 1 << std::endl;
		  // get the next points in row
			m_points = (*it)->getPoints();

			// send the number of points that will follow
			int_numpoint[client_serv_data] = (*it)->getnumpoints();
			tmp_int[0] = int_numpoint[client_serv_data];
			
			MPI::COMM_WORLD.Send(tmp_int, 1, MPI::INT, client_serv_data + 1, 2);

std::cout << "Übertragen der größe erfolgreich mit: " << tmp_int[0] << std::endl;
			// allokiere normal array dynamically
			normals[client_serv_data] = new float [3 * int_numpoint[client_serv_data]];
std::cout << "Normalen wurden alokiert" << std::endl;
			// get the indices for the original sequence
			Indizes[client_serv_data] = (*it)->indizes.get();

			laufvariable[client_serv_data] = (*it)->getnumpoints();

std::cout << "senden wird gestartet" << std::endl;
			// send Data
			MPI::Request req2 = MPI::COMM_WORLD.Isend(m_points.get(), 3 *  int_numpoint[client_serv_data], MPI::FLOAT, client_serv_data + 1, 1);


			// Reciev Normals
			MPI::Request tmp;
			tmp = MPI::COMM_WORLD.Irecv(normals[client_serv_data], 3 * int_numpoint[client_serv_data], MPI::FLOAT, client_serv_data + 1, 4);
			status[client_serv_data] = tmp;


			req2.Wait();

std::cout << "Senden ist fertig, jetzt gehts zum warten auf den nächsten Process" << std::endl;
/*******************************abspeichern  **************/


			if (count >= numprocs - 1)
			{
				client_serv_data = MPI::Request::Waitany( numprocs - 1, status);
std::cout << "\n*****************Normalen empfangen vom Process: " << client_serv_data + 1 << std::endl;

				// store normals on correct position
				for (int x = 0; x < laufvariable[client_serv_data] ; x ++)
				{
					int n_buffer_pos = 3 * Indizes[client_serv_data][x];

					m_normal[n_buffer_pos]     = normals[client_serv_data][3 * x];
					m_normal[n_buffer_pos + 1] = normals[client_serv_data][ (3 * x) + 1];
					m_normal[n_buffer_pos + 2] = normals[client_serv_data][ (3 * x) + 2];

				}
				normals[client_serv_data][0] = 0;




std::cout << "\n+++++++++++++++ Das abspeichern der Normalen gerade ist beendet!" << std::endl;
				count++;
				it++;
				//client_serv_data++;
				//client_serv_data = (client_serv_data % (numprocs - 1));


			}// end if
			else
			{
				count++;
				it++;
				client_serv_data++;
				client_serv_data = (client_serv_data % (numprocs - 1));
			}
		}// End while
std::cout << " Alle Daten wurden übertragen, jetzt auf die letzten Normalen warten" << std::endl;
		//store all data which is still not stored
		MPI::Request::Waitall( numprocs - 1, status);
std::cout << "Alle sind da!" << std::endl;
		client_serv_data = 0;
		for (int y = 0 ; y < numprocs - 1 ; y++)
		{

			// check if some data is in there
			if (normals[client_serv_data][0] != 0)
			{
				// store normals on correct position
				for (int x = 0; x < laufvariable[client_serv_data] ; x ++)
				{
					int n_buffer_pos = 3 * Indizes[client_serv_data][x];

					m_normal[n_buffer_pos]     = normals[client_serv_data][3 * x];
					m_normal[n_buffer_pos + 1] = normals[client_serv_data][ (3 * x) + 1];
					m_normal[n_buffer_pos + 2] = normals[client_serv_data][ (3 * x) + 2];

				}
			}
			client_serv_data++;
		}



		//Points put back into proper shape for PointBufferPtr
		boost::shared_array<float> norm (m_normal);

		long unsigned int tmp = static_cast<unsigned int>(m_loader->getNumPoints());


		// set normals
		m_loader->setPointNormalArray(norm, m_loader->getNumPoints() );

		m_model->m_pointCloud = m_loader;

		// save data
		io_factory.saveModel(m_model, "Normal.ply");

		
		// Complete connection
		int end[1] = {-1};
		int j = 1;
		for (j = 1 ; j < numprocs ; j++)
		{
			MPI::COMM_WORLD.Send(end , 1, MPI::INT, j, 2);
		}


		
		for (int i = 0 ; i < (numprocs -1) ; i++)
		{
			delete [] normals[i];
		}

		delete [] normals;

	}// Ende If
/**********************************************************************************************************/
	// Slave-Process
	else
	{
//für die tests zum Zeitmessen

		// Wait for the first Message (INIT)
		MPI::COMM_WORLD.Recv(con_msg, 128, MPI::CHAR, 0,0);

		// create answer
		sprintf(idstring, "Processor %d on ", rank);
		strcat(idstring, processor_name);
		strcat(con_msg,idstring);
		strcat(con_msg, "...roger roger, we can go on!");

		MPI::COMM_WORLD.Send(con_msg, 128, MPI::CHAR, 0, 0);

/************************************ Connection is successful *******************/

		// Loop for receiving the data, -1 cancels operation
		while(true)
		{
			MPI::COMM_WORLD.Recv( &c_sizepackage, 1, MPI::INT , 0,2);

			//termination condition
			if (c_sizepackage == -1)
			{
				break;
			}
			else
			{
count_serv++;

			
				// Recv the data
				float * tmp = new float[3 * c_sizepackage];
				MPI::Request client_req2 = MPI::COMM_WORLD.Irecv(tmp, 3 * c_sizepackage, MPI::FLOAT, 0, 1);

				// wait till transmission complete
				client_req2.Wait();
std::cout << "Ich Process: " << rank << " soll jetzt Normalen berechnen und zwar so viele: " << c_sizepackage << std::endl;
				// Points put back into proper shape for PointBufferPtr
				boost::shared_array<float> punkte (tmp);


				// The factory requires a model to save.
				// The model in turn requires a Pointbuffer and a Meshbuffer (can be emtpy).
				// The Pointbuffer contains the Indexlist.
				PointBufferPtr pointcloud(new PointBuffer());
				pointcloud->setPointArray(punkte, c_sizepackage);

				PointsetSurface<ColorVertex<float, unsigned char> >* surface;
				surface = new AdaptiveKSearchSurface<ColorVertex<float, unsigned char>, Normal<float> >(pointcloud, "FLANN");

				// Set search options for normal estimation and distance evaluation
// Nur vorübergehend, da wenn die Anzahl der Punkte kleiner ist als einer derr Werte, seg-fault
// muss noch in kdtree.tcc gelöst werden, dass der fall nicht eintreten kann
				if (c_sizepackage > kd || c_sizepackage > ki || c_sizepackage > kn)
				{
				  surface->setKd(kd);
				  surface->setKi(ki);
				  surface->setKn(kn);
				}
				else
				{
std::cout << "\nFall ist eingetreten, es sind weniger Punkte als kd, ki oder kn!" << std::endl;			
				  surface->setKd(c_sizepackage - 1);
				  surface->setKi(c_sizepackage - 1);
				  surface->setKn(c_sizepackage - 1);
				}
				
				// calculate the normals
				//Timestamp ts;
				surface->calculateSurfaceNormals();
				//cerr << ts.getElapsedTimeInMs() << endl;

//				ModelPtr pn( new Model);
//				pn->m_pointCloud = surface->pointBuffer();


				pointcloud = surface->pointBuffer();
				size_t size_normal;
				c_normals = pointcloud->getIndexedPointNormalArray(size_normal);

				// send the normals back to the Masterprocess
				MPI::COMM_WORLD.Send(c_normals.get(), 3 * c_sizepackage, MPI::FLOAT, 0, 4);
std::cout << "\n Berechnen der Normalen ist abgeschlossen!" << std::endl;
			}
		}

	}// End else
	if (rank == 0)
	{  
	  std::cout << "so lange hat es gebraucht: " << start.getElapsedTimeInMs() << std::endl;
	}
	sprintf(test_aufgabe_name, "Ausgabe%00d.dat" , rank);
	f.open(test_aufgabe_name, ios::out);
	f << "Beende den Prozess: " << rank << ", dieser hat " << count_serv << " Pakete bearbeiet. Und hat mit kd, kn und ki gearbeitet:" << kd << kn << ki << endl; 
	f.close();
	MPI_Finalize();

}
