/*
thot package for statistical machine translation
Copyright (C) 2013-2017 Daniel Ortiz-Mart\'inez, Adam Harasimowicz
 
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
 * @file IncrPhraseModel.cc
 * 
 * @brief Definitions file for IncrPhraseModel.h
 */

//--------------- Include files --------------------------------------

#include "IncrPhraseModel.h"


//--------------- Function definitions

#ifdef THOT_HAVE_CXX11
//-------------------------
void IncrPhraseModel::printTTable(FILE* file,
                                  int n)
{
  HatTriePhraseTable* ptPtr=0;

  ptPtr=dynamic_cast<HatTriePhraseTable*>(basePhraseTablePtr);

  if(ptPtr) // C++ RTTI
  {
    HatTriePhraseTable::const_iterator phraseTIter;

    for(phraseTIter=ptPtr->begin();phraseTIter!=ptPtr->end();++phraseTIter)
    {
      HatTriePhraseTable::SrcTableNode srctn;
      HatTriePhraseTable::SrcTableNode::iterator srctnIter;
      const PhraseTransTableNodeData& t=phraseTIter->first;
      ptPtr->getEntriesForTarget(t,srctn);

      if(n<0 || (int)srctn.size()<=n)
      {
        for(srctnIter=srctn.begin();srctnIter!=srctn.end();++srctnIter)
        {
          printTTableEntry(file,t,srctnIter);
        }
      }
      else
      {
        NbestTableNode<PhraseTransTableNodeData> nbt;
        for(srctnIter=srctn.begin();srctnIter!=srctn.end();++srctnIter)
        {
          nbt.insert(srctnIter->second.second.get_c_st(),srctnIter->first);
        }

        int count=0;
        float remainder=0;
        NbestTableNode<PhraseTransTableNodeData>::iterator nbtIter;
        for(nbtIter=nbt.begin();nbtIter!=nbt.end();++nbtIter)
        {
          count++;
          if(count<=n)
          {
            srctnIter=srctn.find(nbtIter->second);
            printTTableEntry(file,t,srctnIter);
          }
          else
          {
            remainder+=nbtIter->first;
          }
        }

        if(remainder>0)
        {
          fprintf(file,"<UNUSED_WORD> |||");
          std::vector<WordIndex>::const_iterator vectorWordIndexIter;
          for(vectorWordIndexIter=t.begin();vectorWordIndexIter!=t.end();++vectorWordIndexIter)
            fprintf(file," %s",wordIndexToTrgString(*vectorWordIndexIter).c_str());
          fprintf(file," ||| 0 %.8f\n",remainder);
        }
      }
    }
  }
}

#else
//-------------------------
void IncrPhraseModel::printTTable(FILE* file,
                                  int n)
{
  StlPhraseTable* ptPtr=0;

  ptPtr=dynamic_cast<StlPhraseTable*>(basePhraseTablePtr);

  if(ptPtr) // C++ RTTI
  {
    StlPhraseTable::TrgPhraseInfo::const_iterator phraseTIter;

    for(phraseTIter=ptPtr->beginTrg();phraseTIter!=ptPtr->endTrg();++phraseTIter)
    {
      StlPhraseTable::SrcTableNode srctn;
      StlPhraseTable::SrcTableNode::iterator srctnIter;
      const PhraseTransTableNodeData& t=phraseTIter->first;
      ptPtr->getEntriesForTarget(t,srctn);

      if(n<0 || (int)srctn.size()<=n)
      {
        for(srctnIter=srctn.begin();srctnIter!=srctn.end();++srctnIter)
        {
          printTTableEntry(file,t,srctnIter);
        }
      }
      else
      {
        NbestTableNode<PhraseTransTableNodeData> nbt;
        for(srctnIter=srctn.begin();srctnIter!=srctn.end();++srctnIter)
        {
          nbt.insert(srctnIter->second.second.get_c_st(),srctnIter->first);
        }

        int count=0;
        float remainder=0;
        NbestTableNode<PhraseTransTableNodeData>::iterator nbtIter;
        for(nbtIter=nbt.begin();nbtIter!=nbt.end();++nbtIter)
        {
          count++;
          if(count<=n)
          {
            srctnIter=srctn.find(nbtIter->second);
            printTTableEntry(file,t,srctnIter);
          }
          else
          {
            remainder+=nbtIter->first;
          }
        }

        if(remainder>0)
        {
          fprintf(file,"<UNUSED_WORD> |||");
          std::vector<WordIndex>::const_iterator vectorWordIndexIter;
          for(vectorWordIndexIter=t.begin();vectorWordIndexIter!=t.end();++vectorWordIndexIter)
            fprintf(file," %s",wordIndexToTrgString(*vectorWordIndexIter).c_str());
          fprintf(file," ||| 0 %.8f\n",remainder);
        }
      }
    }
  }
}

#endif

//-------------------------
IncrPhraseModel::~IncrPhraseModel()
{
  delete basePhraseTablePtr;  
}

//-------------------------
