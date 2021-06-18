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

#ifndef _BestLgProbForTrgWord_h
#define _BestLgProbForTrgWord_h

#include "nlp_common/Prob.h"
#include "nlp_common/WordIndex.h"

#include <map>
#include <utility>

typedef std::map<std::pair<unsigned int, WordIndex>, LgProb> BestLgProbForTrgWord;

#endif
