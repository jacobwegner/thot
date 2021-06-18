/*
thot package for statistical machine translation
Copyright (C) 2013 Daniel Ortiz-Mart\'inez

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public License
as published by the Free Software Foundation; either version 3
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with this program; If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef _PhrHypDataStr_h
#define _PhrHypDataStr_h

//--------------- Include files --------------------------------------

#include "nlp_common/PositionIndex.h"
#include "stack_dec/SourceSegmentation.h"

#include <string>
#include <vector>

//--------------- Classes --------------------------------------------

class PhrHypDataStr
{
public:
  // Partial translation
  std::vector<std::string> ntarget;

  // Translation model info
  SourceSegmentation sourceSegmentation;
  std::vector<PositionIndex> targetSegmentCuts;
};

#endif
