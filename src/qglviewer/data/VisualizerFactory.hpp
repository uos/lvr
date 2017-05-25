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
 * DataCollectorFactory.h
 *
 *  Created on: 07.10.2010
 *      Author: Thomas Wiemann
 */

#ifndef DATACOLLECTORFACTORY_H_
#define DATACOLLECTORFACTORY_H_

#include <string>
#include <QtWidgets>

using std::string;

#include "Visualizer.hpp"

class VisualizerFactory : public QObject
{
    Q_OBJECT
public:
    VisualizerFactory();

	virtual ~VisualizerFactory() {};
	void create(string filename);

Q_SIGNALS:
    void visualizerCreated(Visualizer *);



};

#endif /* DATACOLLECTORFACTORY_H_ */
