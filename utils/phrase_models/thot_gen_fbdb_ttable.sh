# Author: Daniel Ortiz Mart\'inez
# *- bash -*

# Generates a binary phrase ttable given source and target vocabularies
# and a thot ttable

########
plain_ttable_to_id()
{
    # Given a vocabulary generated by obtain_vocab and a thot ttable,
    # obtains a new one which replace the words by their corresponding
    # numeric identifiers.

    _svoc=$1
    _tvoc=$2
    _table=$3

    ${AWK} -v s_voc=$_svoc -v t_voc=$_tvoc \
           'BEGIN{ 
                  while( (getline <s_voc) > 0)
                  {
                    s_word[$2]=$1
                  }
                  while( (getline <t_voc) > 0)
                  {
                    t_word[$2]=$1
                  }
                 }
                 {
                   # Check entry correctness and extract phrase pair
                   countSrc=1
                   correct=1
                   srcphr=""
                   trgphr=""
                   for(i=1;i<NF;++i)
                   {
                    if($i != "|||")
                    {
                      if(countSrc==1)
                      {
                       if(! ($i in s_word)) 
                       {
                         correct=0
                         break
                       }
                       else
                        srcphr=srcphr" "s_word[$i]
                      }
                      else
                       if(! ($i in t_word))
                       {
                         correct=0
                         break
                       }
                       else
                        trgphr=trgphr" "t_word[$i]
                    }
                    else
                    {
                      if(countSrc==0) break
                      if(countSrc==1) 
                        countSrc=0
                    }
                   }
                   if(length(srcphr)==0 || length(trgphr)==0)
                     correct=0

                   # Print entry information if it was correct
                   if(correct)
                     printf"%s ||| %s ||| %s %s\n",srcphr,trgphr,$(NF-1),$NF
                   else
                     printf"Warning: discarding anomalous phrase table entry at line %d (look for words outside vocabulary or empty phrases)\n",NR > "/dev/stderr"
                  }' ${_table}
}

########
obtain_vocab()
{
    sps="$1"

    ${AWK} -v sps="${sps}"\
     'BEGIN{ 
            id=0
            if(sps!="")
            {
             numw=split(sps,words," ")
             for(i=1;i<=numw;++i)
             {
              vocab[words[i]]=id
              ++id
             }
            }
           }            
           {
	    for(i=1;i<=NF;++i)
	    if(!($i in vocab)) 
            {
             freq[$i]=1
             vocab[$i]=id
             ++id
            } 
            else
            {
             ++freq[$i]
            }
	   }
        END{ 
	    for(i in vocab)
             printf"%.8d %s %d\n",vocab[i],i,freq[i]
	   }'
}

########
extract_first_phrase()
{
    $AWK  '{
              i=1; 
              while($i!="|||") 
              {
                printf"%s ",$i;++i
              }
              printf"\n"
           }'
}

########
extract_src_phrases()
{
    extract_first_phrase
}

########
extract_trg_phrases()
{
    ${bindir}/thot_flip_phr | extract_first_phrase
}

########
remove_prev_files()
{
    _files_were_removed=0
    for ext in ${svcb_ext} ${svcb_ext} ${phrdict_ext}; do
        if [ -f $out.$ext ]; then
            rm $out.$ext
            _files_were_removed=1
        fi
    done

    if [ ${_files_were_removed} -eq 1 ]; then
        echo "Warning: previously existing BDB model files were found and removed" >&2
    fi
}

########
gen_src_vocab()
{
    # Generate source vocabulary
    if [ -f ${p_val}_swm.svcb ]; then
        # If there is a single word model vocabulary already generated,
        # it should be used as the phrase model vocabulary to avoid
        # conflicts
        cat ${p_val}_swm.svcb | $AWK '{printf"%.8d %s %s\n",$1,$2,$3}' > $srcv
    else
        cat $table | extract_src_phrases $tmpdir | obtain_vocab "NULL UNKNOWN_WORD <UNUSED_WORD>" > $srcv
    fi
}

########
gen_trg_vocab()
{
    # Generate target vocabulary
    if [ -f ${p_val}_swm.tvcb ]; then
        # If there is a single word model vocabulary already generated,
        # it should be used as the phrase model vocabulary to avoid
        # conflicts
        cat ${p_val}_swm.tvcb | $AWK '{printf"%.8d %s %s\n",$1,$2,$3}' > $trgv
    else
        cat $table | extract_trg_phrases $tmpdir | obtain_vocab "NULL UNKNOWN_WORD <UNUSED_WORD>" > $trgv
    fi
}
########
gen_vocab_files()
{
    gen_src_vocab

    gen_trg_vocab
}

########
gen_fbdb_files()
{
    if [ $debug -eq 1 ]; then
        plain_ttable_to_id $srcv $trgv $table > $out.idttable
    fi

    plain_ttable_to_id $srcv $trgv $table | ${bindir}/thot_ttable_to_fbdb -o $out
}

########
if [ $# -lt 4 ]; then
    echo "thot_gen_fbdb_ttable -p <string> -o <string> [-T <string>] [-debug]"
    echo ""
    echo "-p <string>         prefix of translation model files"
    echo "-o <string>         output prefix"
    echo "-T <string>         directory for temporary files"
    echo "-debug              generates files in text format for"
    echo "                    debugging purposes"
    echo ""
else
    # Read parameters
    p_given=0
    o_given=0
    T_given=0
    tmpdir="/tmp"
    debug=0
    while [ $# -ne 0 ]; do
        case $1 in
            "-p") shift
                if [ $# -ne 0 ]; then
                    p_val=$1
                    p_given=1
                fi
                ;;
            "-o") shift
                if [ $# -ne 0 ]; then
                    out=$1
                    o_given=1
                fi
                ;;
            "-T") shift
                if [ $# -ne 0 ]; then
                    T_given=1
                    tmpdir=$1
                else
                    tmpdir="/tmp"
                fi
                ;;
            "-debug") shift
                debug=1
                ;;
        esac
        shift
    done
    
    if [ $p_given -eq 0 ]; then
        echo "Error: -p parameter not given"
        exit 1
    fi

    if [ $o_given -eq 0 ]; then
        echo "Error: -o parameter not given"
        exit 1
    fi

    # Obtain translation table file name
    table="${p_val}.ttable"

    if [ ! -f $table ]; then
        echo "Error: file "$table" does not exist"
        exit 1
    fi

    # Define file extensions
    svcb_ext=fbdb_svcb
    svcb_ext=fbdb_tvcb
    phrdict_ext=fbdb_phrdict
    
    # Generate vocabulary file names
    srcv=$out.${svcb_ext}
    trgv=$out.${tvcb_ext}

    # Generate fbdb translation table files
    echo "Starting BDB ttable generation..." >&2

    remove_prev_files
    
    gen_vocab_files

    gen_fbdb_files

    echo "BDB ttable generation process finished" >&2

fi
