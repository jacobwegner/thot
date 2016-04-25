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
 
/********************************************************************/
/*                                                                  */
/* Module: SentenceHandler                                          */
/*                                                                  */
/* Prototype file: SentenceHandler.h                                */
/*                                                                  */
/* Description: Defines the SentenceHandler class.                  */
/*              SentenceHandler class allow to access a set of      */
/*              sentence pairs.                                     */
/*                                                                  */
/********************************************************************/

#ifndef _SentenceHandler_h
#define _SentenceHandler_h

//--------------- Include files --------------------------------------

#if HAVE_CONFIG_H
#  include <thot_config.h>
#endif /* HAVE_CONFIG_H */

#include "awkInputStream.h"
#include <fstream>
#include <string.h>
#include "BaseSentenceHandler.h"

//--------------- Constants ------------------------------------------


//--------------- typedefs -------------------------------------------

//--------------- function declarations ------------------------------

//--------------- Classes --------------------------------------------

//--------------- SentenceHandler class

class SentenceHandler: public BaseSentenceHandler
{
  public:

       // Constructor
   SentenceHandler(void);   

       // Functions to read and add sentence pairs
   bool readSentencePairs(const char *srcFileName,
                          const char *trgFileName,
                          const char *sentCountsFile,
                          pair<unsigned int,unsigned int>& sentRange);
       // NOTE: when function readSentencePairs() is invoked, previously
       //       seen sentence pairs are removed

   void addSentPair(Vector<std::string> srcSentStr,
                    Vector<std::string> trgSentStr,
                    Count c,
                    pair<unsigned int,unsigned int>& sentRange);
   unsigned int numSentPairs(void);
       // NOTE: the whole valid range in a given moment is
       // [ 0 , numSentPairs() )
   int nthSentPair(unsigned int n,
                   Vector<std::string>& srcSentStr,
                   Vector<std::string>& trgSentStr,
                   Count& c);
   int getSrcSent(unsigned int n,
                  Vector<std::string>& srcSentStr);
   int getTrgSent(unsigned int n,
                  Vector<std::string>& trgSentStr);
   int getCount(unsigned int n,
                Count& c);

       // Functions to print sentence pairs
   bool printSentPairs(const char *srcSentFile,
                       const char *trgSentFile,
                       const char *sentCountsFile);

       // Clear function
   void clear(void);
   
  protected:

   Vector<pair<Vector<std::string>,Vector<std::string> > > sentPairCont;
   Vector<Count> sentPairCount;
};

#endif
