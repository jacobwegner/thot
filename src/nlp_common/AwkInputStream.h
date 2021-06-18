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

/**
 * @file AwkInputStream.h
 *
 * @brief AWK-like input stream.
 */

#ifndef _awk_input_stream_h
#define _awk_input_stream_h

#include <stdio.h>
#include <stdlib.h>
#if THOT_HAVE_UNISTD_H
#include <unistd.h>
#endif
#include "nlp_common/ErrorDefs.h"
#include "nlp_common/getline.h"

#include <fstream>
#include <iostream>
#include <string>

//--------------- AwkInputStream class: awk-like input stream class

class AwkInputStream
{
public:
  unsigned int NF;
  unsigned int FNR;

  char FS;
  std::string fileName;

  AwkInputStream(void);
  AwkInputStream& operator=(const AwkInputStream& awk);
  bool getln(void);
  std::string dollar(unsigned int n);
  bool open(const char* str);
  bool open_stream(FILE* stream);
  void close(void);
  bool rwd(void);
  void printFields(void);
  ~AwkInputStream();

protected:
  std::string fieldStr;
  char* buff;
  size_t buftlen;
  FILE* filePtr;
  bool fopen_called;

  int get_NF(void);
  void retrieveField(unsigned int n);
};

#endif
