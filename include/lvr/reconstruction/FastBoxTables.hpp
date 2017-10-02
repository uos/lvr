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
 * FastBoxTables.hpp
 *
 *  Created on: 09.03.2011
 *      Author: Thomas Wiemann
 */

#ifndef FASTBOXTABLES_HPP_
#define FASTBOXTABLES_HPP_

namespace lvr
{

const static int neighbor_table[12][3] = {
  {12, 10,  9}, // 0
  {22, 12, 21}, // 1
  {16, 12, 15}, // 2
  { 4,  3, 12}, // 3
  {14, 10, 11}, // 4
  {23, 22, 14}, // 5
  {14, 16, 17}, // 6
  { 4,  5, 14}, // 7
  { 4,  1, 10}, // 8
  {22, 19, 10}, // 9
  { 4,  7, 16}, // 10
  {22, 25, 16}  // 11
};

const static int neighbor_vertex_table[12][3] = {
  { 4,  2,  6},
  { 3,  5,  7},
  { 0,  6,  4},
  { 1,  5,  7},
  { 0,  6,  2},
  { 3,  7,  1},
  { 2,  4,  0},
  { 5,  1,  3},
  { 9, 11, 10},
  { 8, 10, 11},
  {11,  9,  8},
  {10,  8,  9}
};

const static int vertex_edge_table[12][2] = {
    {0, 1},
    {1, 2},
    {2, 3},
    {3, 0},
     {4, 5},
    {5, 6},
    {6, 7},
    {7, 4},
    {0, 4},
    {1, 5},
    {3, 7},
    {2, 6}
};

const static int edge_vertex_table[8][3] = {
  { 0,  3,  8}, // 0
  { 0,  1,  9}, // 1
  { 1,  2,  11},// 2
  { 2,  3,  10}, // 3
  { 4,  7,  8}, // 4
  { 9,  4,  5}, // 5
  { 5,  6,  11},// 6
  { 10,  6,  7} // 7
};

const static int edge_neighbour_table[8][12] = {
  { 4,  -1,  -1,  10,  -1,  -1,  -1,  -1,  12,  -1,  -1,  -1}, // 0
  { 22, 10,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  12,  -1,  -1}, // 1
  { -1, 16,  22,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  12}, // 2
  { -1, -1,   4,  16,  -1,  -1,  -1,  -1,  -1,  -1,  12,  -1},  // 3
  { -1, -1,  -1,  -1,   4,  -1,  -1,  10,  14,  -1,  -1,  -1}, // 4
  { -1, -1,  -1,  -1,  22,  10,  -1,  -1,  -1,  14,  -1,  -1}, // 5
  { -1, -1,  -1,  -1,  -1,  16,  22,  -1,  -1,  -1,  -1,  14}, // 6
  { -1, -1,  -1,  -1,  -1,  -1,   4,  16,  -1,  -1,  14,  -1}  // 7
};


} // namespace lvr
#endif /* FASTBOXTABLES_HPP_ */