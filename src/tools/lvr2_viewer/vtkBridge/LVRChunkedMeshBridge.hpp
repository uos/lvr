#ifndef LVR2_CHUNKED_MESH_BRIDGE_HPP_
#define LVR2_CHUNKED_MESH_BRIDGE_HPP_

#include <vtkSmartPointer.h>
#include <vtkActor.h>
#include <vtkActorCollection.h>
#include <vtkRenderer.h>
#include <vtkCamera.h>

#include "lvr2/algorithm/ChunkManager.hpp"
#include "lvr2/geometry/BoundingBox.hpp"
#include "lvr2/display/MeshOctree.hpp"

#include <string>

#include "MeshChunkActor.hpp"

#include <mutex>
#include <thread>
#include <condition_variable>
#include <QObject>

typedef std::unordered_map<size_t, vtkSmartPointer<MeshChunkActor> > actorMap;
Q_DECLARE_METATYPE(actorMap)


namespace lvr2 {
    class LVRChunkedMeshBridge : public QObject
    {
        Q_OBJECT
        public:
            LVRChunkedMeshBridge(std::string file);
            void getActors(double planes[24],
                    std::vector<BaseVector<float> >& centroids, 
                    std::vector<size_t >& indices);
        
            actorMap getHighResActors() { return m_highResActors; }
            actorMap getLowResActors() { return m_chunkActors; }
                    //std::unordered_map<size_t, vtkSmartPointer<MeshChunkActor> >& actors);
            void addInitialActors(vtkSmartPointer<vtkRenderer> renderer);

            void fetchHighRes(double x, double y, double z,
                             double dir_x, double dir_y, double dir_z);

        Q_SIGNALS:
            void updateHighRes(actorMap lowRes, actorMap highRes);
                    
                   // std::unordered_map<size_t, vtkSmartPointer<MeshChunkActor> > lowResActors,
                   //            std::unordered_map<size_t, vtkSmartPointer<MeshChunkActor> > highResActors);

        protected:
            void computeMeshActors();
            inline vtkSmartPointer<MeshChunkActor> computeMeshActor(size_t& id, MeshBufferPtr& chunk);

        private:

            std::thread worker;
            std::mutex mutex;
            std::condition_variable cond_;
            double dist_;
            bool getNew_;
            bool running_;
            BoundingBox<BaseVector<float> > m_region;
            BoundingBox<BaseVector<float> > m_lastRegion;

            void highResWorker();
            lvr2::ChunkManager m_chunkManager;
            std::unordered_map<size_t, MeshBufferPtr> m_chunks;
            std::unordered_map<size_t, MeshBufferPtr> m_highRes;
            std::unordered_map<size_t, vtkSmartPointer<MeshChunkActor> > m_chunkActors;
            std::unordered_map<size_t, vtkSmartPointer<MeshChunkActor> > m_highResActors;

            MeshOctree<BaseVector<float> >* m_oct;
//            std::unordered_map<size_t, std::vector<vtkPolyData> > > m_chunkActors;
    };
} // namespace lvr2

#endif
