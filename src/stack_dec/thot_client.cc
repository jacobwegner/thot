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
/* Module: thot_client.cc                                           */
/*                                                                  */
/* Definitions file: thot_client.cc                                 */
/*                                                                  */
/* Description: Implements a translator client                      */
/*                                                                  */
/********************************************************************/

/**
 * @file PhrLocalSwLiTm.h
 *
 * @brief Implements a translator client.
 */

//--------------- Include files --------------------------------------

#if HAVE_CONFIG_H
#  include <thot_config.h>
#endif /* HAVE_CONFIG_H */

#include "ThotDecoderClient.h"
#include "ErrorDefs.h"
#include <SmtPreprocDefs.h>
#include "options.h"
#include "ctimer.h"
#include "client_server_defs.h"
#include "thot_client_pars.h"
#include <math.h>
#include <iostream>
#include <fstream>
#include <iomanip>

//--------------- Constants ------------------------------------------


//--------------- Function Declarations ------------------------------

int process_request(const thot_client_pars& tdcPars);
int TakeParameters(int argc,
                   char *argv[],
                   thot_client_pars& tdcPars);
void printUsage(void);
void version(void);
void printDesc(void);

//--------------- Global variables -----------------------------------


//--------------- Function Definitions -------------------------------

//---------------
int main(int argc,char *argv[])
{
  thot_client_pars tdcPars;
 
  if(TakeParameters(argc,argv,tdcPars)==THOT_OK)
  {
        // Parameters ok
   double elapsed_ant,elapsed,ucpu,scpu;
   bool retVal;

   if(tdcPars.verbose)
   {
     ctimer(&elapsed_ant,&ucpu,&scpu);
   }
       // Process request
   retVal=process_request(tdcPars);
   
   if(tdcPars.verbose)
   {
     ctimer(&elapsed,&ucpu,&scpu);
     std::cerr<<"Elapsed time: " << elapsed-elapsed_ant << " secs\n";
   }
   return retVal;
 } 
 else return THOT_ERROR;  
}

//---------------
int process_request(const thot_client_pars& tdcPars)
{
  std::string s;
  std::vector<std::string> v;
  std::string translatedSentence;
  std::string bestHypInfo;
  ThotDecoderClient thotDecoderClient;
  int retVal=THOT_OK;
  double elapsed_ant,elapsed,ucpu,scpu;

      // Connect to translation server
  if(tdcPars.verbose)
  {
    std::cerr<<"----------------------------------------------------"<<std::endl;
    std::cerr<<"User ID: "<<tdcPars.user_id<<std::endl;
    std::cerr<<"Connecting to server..."<<std::endl;
    ctimer(&elapsed_ant,&ucpu,&scpu);
  }

  retVal=thotDecoderClient.connectToTransServer(tdcPars.serverIP.c_str(),
                                                tdcPars.server_port);
  if(retVal==THOT_ERROR) return THOT_ERROR;

      // Print connection latency information
  if(tdcPars.verbose)
  {
    ctimer(&elapsed,&ucpu,&scpu);
    std::cerr<<"Connection latency: " << elapsed-elapsed_ant << " secs\n";
  }

      // Send request to the translation server
  if(tdcPars.verbose)
  {
    std::cerr<<"Client: sending request to the server, request code "<<tdcPars.server_request_code<<std::endl;
  }
      // Get time
  ctimer(&elapsed_ant,&ucpu,&scpu);

  switch(tdcPars.server_request_code)
  {
    case OL_TRAIN_PAIR: retVal=thotDecoderClient.sendSentPairForOlTrain(tdcPars.user_id,tdcPars.stlStringSrc.c_str(),tdcPars.stlStringRef.c_str());
      ctimer(&elapsed,&ucpu,&scpu);
      if(tdcPars.verbose) std::cerr<<"Client: return value= "<<retVal<<std::endl;
      break;
    case TRAIN_ECM: retVal=thotDecoderClient.sendStrPairForTrainEcm(tdcPars.user_id,tdcPars.stlString1.c_str(),tdcPars.stlString2.c_str());
      ctimer(&elapsed,&ucpu,&scpu);
      if(tdcPars.verbose) std::cerr<<"Client: return value= "<<retVal<<std::endl;
      break;
    case TRANSLATE_SENT: retVal=thotDecoderClient.sendSentToTranslate(tdcPars.user_id,tdcPars.sentenceToTranslate.c_str(),translatedSentence,bestHypInfo);
      if(tdcPars.verbose) std::cerr<<"Client: return value= "<<retVal<<std::endl;
      std::cout<<translatedSentence<<std::endl;
      break;
    case TRANSLATE_SENT_HYPINFO: retVal=thotDecoderClient.sendSentToTranslate(tdcPars.user_id,tdcPars.sentenceToTranslate.c_str(),translatedSentence,bestHypInfo);
      if(tdcPars.verbose) std::cerr<<"Client: return value= "<<retVal<<std::endl;
      std::cout<<bestHypInfo<<std::endl;
      std::cout<<translatedSentence<<std::endl;
      break;
    case VERIFY_COV: retVal=thotDecoderClient.sendSentPairVerCov(tdcPars.user_id,tdcPars.stlStringSrc.c_str(),tdcPars.stlStringRef.c_str(),translatedSentence);
      ctimer(&elapsed,&ucpu,&scpu);
      if(tdcPars.verbose) std::cerr<<"Client: return value= "<<retVal<<std::endl;
      std::cout<<translatedSentence<<std::endl;
      break;
    case START_CAT: retVal=thotDecoderClient.startCat(tdcPars.user_id,tdcPars.sentenceToTranslate.c_str(),translatedSentence);
      ctimer(&elapsed,&ucpu,&scpu);
      if(tdcPars.verbose) std::cerr<<"Client: return value= "<<retVal<<std::endl;
      std::cout<<translatedSentence<<std::endl;
      break;
    case ADD_STR_TO_PREF: retVal=thotDecoderClient.addStrToPref(tdcPars.user_id,tdcPars.strToAddToPref.c_str(),translatedSentence);
      ctimer(&elapsed,&ucpu,&scpu);
      if(tdcPars.verbose) std::cerr<<"Client: return value= "<<retVal<<std::endl;
      std::cout<<translatedSentence<<std::endl;
      break;
    case RESET_PREF: retVal=thotDecoderClient.resetPref(tdcPars.user_id);
      ctimer(&elapsed,&ucpu,&scpu);
      if(tdcPars.verbose) std::cerr<<"Client: return value= "<<retVal<<std::endl;
      break;
    case CLEAR_TRANS: retVal=thotDecoderClient.sendClearRequest(tdcPars.user_id);
      ctimer(&elapsed,&ucpu,&scpu);
      if(tdcPars.verbose) std::cerr<<"Client: return value= "<<retVal<<std::endl;
      break;
    case PRINT_MODELS: retVal=thotDecoderClient.sendPrintRequest(tdcPars.user_id);
      ctimer(&elapsed,&ucpu,&scpu);
      if(tdcPars.verbose) std::cerr<<"Client: return value= "<<retVal<<std::endl;
      break;
    case END_SERVER: retVal=thotDecoderClient.sendEndServerRequest(tdcPars.user_id);
      ctimer(&elapsed,&ucpu,&scpu);
      if(tdcPars.verbose) std::cerr<<"Client: return value= "<<retVal<<std::endl;
      break;
    default:
      ctimer(&elapsed,&ucpu,&scpu);
      break;
  }
  if(tdcPars.verbose)
  {
    std::cerr<<"Request latency: " << elapsed-elapsed_ant << " secs\n";
  }

      //thotDecoderClient.disconnect(); // (disconnect is not required since the server only
      //                                   dispatch one request per client execution)
  return retVal;
}

//---------------
int TakeParameters(int argc,
                   char *argv[],
                   thot_client_pars& tdcPars)
{
 int err;

 if(argc==1)
 {
   printDesc();
   return THOT_ERROR;   
 }

 err=readOption(argc,argv,"--help");
 if(err!=-1)
 {
   printUsage();
   return THOT_ERROR;   
 }      

     /* Verify --version option */
 err=readOption(argc,argv, "--version");
 if(err!=-1)
 {
   version();
   return THOT_ERROR;
 }

     /* Take the server IP*/
 err=readSTLstring(argc,argv, "-i", &tdcPars.serverIP);
 if(err==-1)
 {
   printUsage();
   return THOT_ERROR;   
 }

     /* Take the -p parameter */
 err=readUnsignedInt(argc,argv, "-p", &tdcPars.server_port);
 if(err!=0)
 {
   tdcPars.server_port=DEFAULT_SERVER_PORT;
 }

     /* Take the -uid parameter */
 err=readInt(argc,argv, "-uid", &tdcPars.user_id);
 if(err!=0)
 {
   tdcPars.user_id=DEFAULT_USER_ID;
 }

     /* Verify verbose option */
 tdcPars.verbose=0;
 err=readOption(argc,argv, "-v");
 if(err==0) tdcPars.verbose=1;

     /* Take the sentence pair to be trained */
 err=readTwoSTLstrings(argc,argv, "-tr", &tdcPars.stlStringSrc,&tdcPars.stlStringRef);
 if(err==0)
 {
   tdcPars.server_request_code=OL_TRAIN_PAIR;
   return THOT_OK;
 }

 //     /* Take the string pair for error correcting model training */
 // err=readTwoSTLstrings(argc,argv, "-tre", &tdcPars.stlString1,&tdcPars.stlString2);
 // if(err==0)
 // {
 //   tdcPars.server_request_code=TRAIN_ECM;
 //   return THOT_OK;
 // }

     /* Take the sentence to be translated */
 err=readSTLstring(argc,argv, "-t", &tdcPars.sentenceToTranslate);
 if(err==0)
 {
   tdcPars.server_request_code=TRANSLATE_SENT;
   return THOT_OK;
 }

     /* Take the sentence to be translated */
 err=readSTLstring(argc,argv, "-th", &tdcPars.sentenceToTranslate);
 if(err==0)
 {
   tdcPars.server_request_code=TRANSLATE_SENT_HYPINFO;
   return THOT_OK;
 }

     /* Take the sentence pair for coverage verifying */
 err=readTwoSTLstrings(argc,argv, "-c", &tdcPars.stlStringSrc,&tdcPars.stlStringRef);
 if(err==0)
 {
   tdcPars.server_request_code=VERIFY_COV;
   return THOT_OK;
 }

     /* Take the sentence to be translated in a CAT scenario using the
      * n-best technique*/
 err=readSTLstring(argc,argv, "-sc", &tdcPars.sentenceToTranslate);
 if(err==0)
 {
   tdcPars.server_request_code=START_CAT;
   return THOT_OK;
 }

     /* Take the sentence to be translated */
 err=readSTLstring(argc,argv, "-ap",&tdcPars.strToAddToPref);
 if(err==0)
 {
   tdcPars.server_request_code=ADD_STR_TO_PREF;
   return THOT_OK;
 }

     /* Check -rp option */
 err=readOption(argc,argv,"-rp");
 if(err!=-1)
 {
   tdcPars.server_request_code=RESET_PREF;
   return THOT_OK;
 }

     /* Verify clear option */
 err=readOption(argc,argv, "-clear");
 if(err==0)
 {
   tdcPars.server_request_code=CLEAR_TRANS;
   return THOT_OK;
 }

      /* Verify print option */
 err=readOption(argc,argv, "-pr");
 if(err==0)
 {
   tdcPars.server_request_code=PRINT_MODELS;
   return THOT_OK;
 }

     /* Verify -e option */
 err=readOption(argc,argv, "-e");
 if(err==0)
 {
   tdcPars.server_request_code=END_SERVER;
   return THOT_OK;
 }

 return THOT_ERROR;
}

//---------------
void printDesc(void)
{
  std::cerr<<"thot_client is part of the thot package "<<std::endl;
  std::cerr<<"thot version "<<THOT_VERSION<<std::endl;
  std::cerr<<"thot is GNU software written by Daniel Ortiz"<<std::endl;
}

//---------------
void printUsage(void)
{
  std::cerr<<"Usage: thot_client           -i <string> [-p <int>] [-uid <int>]\n";
  std::cerr<<"                             { -tr <srcstring> <refstring> | \n";
  // std::cerr<<"                          | -tre <srcsent> <refsent> | \n";
  std::cerr<<"                             | -t <string> | -th <string> |\n";
  std::cerr<<"                             | -c <srcstring> <refstring> |\n";
  std::cerr<<"                             | -sc <string> | -ap <string> | -rp |\n";
  std::cerr<<"                             | -clear | -o <string> | -e } [ -v ]\n";
  std::cerr<<"                             [--help] [--version]\n\n";
  std::cerr<<"-i <string>                  Set IP address of the server.\n";
  std::cerr<<"-p <int>                     Server port.\n";
  std::cerr<<"-uid <int>                   Set user id ("<<DEFAULT_USER_ID<<" by default).\n";
  std::cerr<<"-tr <srcstring> <refstring>  Train server models given a sentence pair.\n";
  // std::cerr<<"-tre <srcsent> <refsent>  Train error correcting model given a string pair.\n";
  std::cerr<<"-t <string>                  Translate sentence.\n";
  std::cerr<<"-th <string>                 Translate sentence (returns hypothesis\n";
  std::cerr<<"                             information).\n";
  std::cerr<<"-c <srcstring> <refstring>   Verify model coverage for reference sentence.\n";
  std::cerr<<"-sc <string>                 Start CAT system for the given sentence, using\n";
  std::cerr<<"                             the null string as prefix.\n";
  std::cerr<<"-ap <string>                 Add string to prefix.\n";
  std::cerr<<"-rp <string>                 Reset prefix.\n";
  std::cerr<<"-clear                       Clear loaded models.\n";
  std::cerr<<"-pr                          Print models.\n";
  std::cerr<<"-e                           End server.\n";
  std::cerr<<"-v                           Verbose mode.\n";
  std::cerr<<"--help                       Display this help and exit.\n";
  std::cerr<<"--version                    Output version information and exit.\n";
}

//---------------
void version(void)
{
  std::cerr<<"thot_client is part of the thot package\n";
  std::cerr<<"thot version "<<THOT_VERSION<<std::endl;
  std::cerr<<"thot is GNU software written by Daniel Ortiz\n";
}
