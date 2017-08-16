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
*  @date 17.06.2017
*  @author Jan Philipp Vogtherr <jvogtherr@uni-osnabrueck.de>
*  @author Kristin Schmidt <krschmidt@uni-osnabrueck.de>
*/

#ifndef LVR2_ALGORITHM_TEXTURIZER_H_
#define LVR2_ALGORITHM_TEXTURIZER_H_

#include <boost/shared_ptr.hpp>
#include <boost/smart_ptr/make_shared.hpp>

#include <lvr2/geometry/BaseMesh.hpp>
#include <lvr2/geometry/BaseVector.hpp>
#include <lvr2/geometry/ClusterSet.hpp>
#include <lvr2/geometry/HalfEdgeMesh.hpp>
#include <lvr2/geometry/Handles.hpp>
#include <lvr2/geometry/Normal.hpp>
#include <lvr2/geometry/Normal.hpp>
#include <lvr2/geometry/Point.hpp>
#include <lvr2/geometry/Vector.hpp>
#include <lvr2/reconstruction/PointsetSurface.hpp>

namespace lvr2
{

struct TexturizerResult
{
    // TODO: ClusterMap<TexHandle> texHandleClusterMap;
    // TODO: liste von texturen
    // TODO: texture coords map
};

template<typename BaseVecT>
TexturizerResult generateTextures(
    HalfEdgeMesh<BaseVecT>& mesh,
    ClusterSet<FaceHandle>& faceHandleClusterSet,
    PointsetSurfacePtr<BaseVecT> surface
);

} // namespace lvr2

#include <lvr2/algorithm/Texturizer.tcc>

#endif /* LVR2_ALGORITHM_TEXTURIZER_H_ */