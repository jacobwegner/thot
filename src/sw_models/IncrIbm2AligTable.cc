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
 * @file IncrIbm2AligTable.cc
 *
 * @brief Definitions file for IncrIbm2AligTable.h
 */

#include "sw_models/IncrIbm2AligTable.h"

IncrIbm2AligTable::IncrIbm2AligTable()
{
}

void IncrIbm2AligTable::setAligNumer(aSource as, PositionIndex i, float f)
{
  AligNumerElem& aligNumerElem = aligNumer[as];
  if (aligNumerElem.size() != as.slen + 1)
    aligNumerElem.resize(as.slen + 1);
  aligNumerElem[i] = f;
}

float IncrIbm2AligTable::getAligNumer(aSource as, PositionIndex i, bool& found) const
{
  auto iter = aligNumer.find(as);
  if (iter != aligNumer.end())
  {
    if (iter->second.size() == as.slen + 1)
    {
      found = true;
      return iter->second[i];
    }
  }

  found = false;
  return 0;
}

void IncrIbm2AligTable::setAligDenom(aSource as, float f)
{
  aligDenom[as] = f;
}

float IncrIbm2AligTable::getAligDenom(aSource as, bool& found) const
{
  auto iter = aligDenom.find(as);
  if (iter != aligDenom.end())
  {
    // s is stored in aligDenom
    found = true;
    return iter->second;
  }
  else
  {
    // s is not stored in aligDenom
    found = false;
    return 0;
  }
}

void IncrIbm2AligTable::setAligNumDen(aSource as, PositionIndex i, float num, float den)
{
  setAligNumer(as, i, num);
  setAligDenom(as, den);
}

void IncrIbm2AligTable::reserveSpace(aSource as)
{
  aligNumer[as];
  aligDenom[as];
}

bool IncrIbm2AligTable::load(const char* aligNumDenFile, int verbose)
{
#ifdef THOT_ENABLE_LOAD_PRINT_TEXTPARS
  return loadPlainText(aligNumDenFile, verbose);
#else
  return loadBin(aligNumDenFile, verbose);
#endif
}

bool IncrIbm2AligTable::loadPlainText(const char* aligNumDenFile, int verbose)
{
  // Clear data structures
  clear();

  if (verbose)
    std::cerr << "Loading alignd file in plain text format from " << aligNumDenFile << std::endl;

  AwkInputStream awk;
  if (awk.open(aligNumDenFile) == THOT_ERROR)
  {
    if (verbose)
      std::cerr << "Error in alignment nd file, file " << aligNumDenFile << " does not exist.\n";
    return THOT_ERROR;
  }
  else
  {
    // Read entries
    while (awk.getln())
    {
      if (awk.NF == 6)
      {
        aSource as;
        as.j = atoi(awk.dollar(1).c_str());
        as.slen = atoi(awk.dollar(2).c_str());
        as.tlen = atoi(awk.dollar(3).c_str());
        PositionIndex i = atoi(awk.dollar(4).c_str());
        float numer = (float)atof(awk.dollar(5).c_str());
        float denom = (float)atof(awk.dollar(6).c_str());
        setAligNumDen(as, i, numer, denom);
      }
    }
    return THOT_OK;
  }
}

bool IncrIbm2AligTable::loadBin(const char* aligNumDenFile, int verbose)
{
  // Clear data structures
  clear();

  if (verbose)
    std::cerr << "Loading alignd file in binary format from " << aligNumDenFile << std::endl;

  // Try to open file
  std::ifstream inF(aligNumDenFile, std::ios::in | std::ios::binary);
  if (!inF)
  {
    if (verbose)
      std::cerr << "Error in alignment nd file, file " << aligNumDenFile << " does not exist.\n";
    return THOT_ERROR;
  }
  else
  {
    // Read register
    bool end = false;
    while (!end)
    {
      aSource as;
      PositionIndex i;
      float numer;
      float denom;
      if (inF.read((char*)&as.j, sizeof(PositionIndex)))
      {
        inF.read((char*)&as.slen, sizeof(PositionIndex));
        inF.read((char*)&as.tlen, sizeof(PositionIndex));
        inF.read((char*)&i, sizeof(PositionIndex));
        inF.read((char*)&numer, sizeof(float));
        inF.read((char*)&denom, sizeof(float));
        setAligNumDen(as, i, numer, denom);
      }
      else
        end = true;
      end = true;
    }
    return THOT_OK;
  }
}

bool IncrIbm2AligTable::print(const char* aligNumDenFile) const
{
#ifdef THOT_ENABLE_LOAD_PRINT_TEXTPARS
  return printPlainText(aligNumDenFile);
#else
  return printBin(aligNumDenFile);
#endif
}

bool IncrIbm2AligTable::printPlainText(const char* aligNumDenFile) const
{
  std::ofstream outF;
  outF.open(aligNumDenFile, std::ios::out);
  if (!outF)
  {
    std::cerr << "Error while printing alignment nd file." << std::endl;
    return THOT_ERROR;
  }
  else
  {
    // print file with alignment nd values
    for (auto& elem : aligNumer)
    {
      for (PositionIndex i = 0; i < elem.second.size(); ++i)
      {
        outF << elem.first.j << " ";
        outF << elem.first.slen << " ";
        outF << elem.first.tlen << " ";
        outF << i << " ";
        outF << elem.second[i] << " ";
        bool found;
        float denom = getAligDenom(elem.first, found);
        outF << denom << std::endl;
      }
    }
    return THOT_OK;
  }
}

bool IncrIbm2AligTable::printBin(const char* aligNumDenFile) const
{
  std::ofstream outF;
  outF.open(aligNumDenFile, std::ios::out | std::ios::binary);
  if (!outF)
  {
    std::cerr << "Error while printing alignment nd file." << std::endl;
    return THOT_ERROR;
  }
  else
  {
    // print file with alignment nd values
    for (auto& elem : aligNumer)
    {
      for (PositionIndex i = 0; i < elem.second.size(); ++i)
      {
        outF.write((char*)&elem.first.j, sizeof(PositionIndex));
        outF.write((char*)&elem.first.slen, sizeof(PositionIndex));
        outF.write((char*)&elem.first.tlen, sizeof(PositionIndex));
        outF.write((char*)&i, sizeof(PositionIndex));
        outF.write((char*)&elem.second[i], sizeof(float));
        bool found;
        float denom = getAligDenom(elem.first, found);
        outF.write((char*)&denom, sizeof(float));
      }
    }
    return THOT_OK;
  }
}

void IncrIbm2AligTable::clear()
{
  aligNumer.clear();
  aligDenom.clear();
}
