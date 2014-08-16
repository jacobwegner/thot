# Author: Daniel Ortiz Mart\'inez
# *- bash -*

# Executes language and translation model tuning.

# \textbf{Categ}: modelling

########
print_desc()
{
    echo "thot_cat_tune written by Daniel Ortiz"
    echo "thot_cat_tune tunes the CAT system parameters"
    echo "type \"thot_cat_tune --help\" to get usage information"
}

########
version()
{
    echo "thot_cat_tune is part of the thot package"
    echo "thot version "${version}
    echo "thot is GNU software written by Daniel Ortiz"
}

########
usage()
{
    echo "thot_cat_tune           [-pr <int>] -c <string>"
    echo "                        -s <string> -t <string> -o <string>"
    echo "                        [-qs <string>]"
    echo "                        [-tdir <string>] [-debug] [--help] [--version]"
    echo ""
    echo "-pr <int>               Number of processors (1 by default)"
    echo "-c <string>             Configuration file"
    echo "-s <string>             File with source sentences"
    echo "-t <string>             File with target sentences"
    echo "-o <string>             Output directory common to all processors."
    echo "-qs <string>            Specific options to be given to the qsub"
    echo "                        command (example: -qs \"-l pmem=1gb\")"
    echo "                        NOTES:"
    echo "                         a) ignore this if not using a PBS cluster"
    echo "                         b) -qs option may be crucial to ensure the correct"
    echo "                            execution of the tool. The main purpose of -qs"
    echo "                            is to reserve the required cluster resources."
    echo "                            If the necessary resources are not met the"
    echo "                            execution will abort."
    echo "                            Resources are reserved in different ways depending"
    echo "                            on the cluster software. For instance, if using SGE"
    echo "                            software, -qs \"-l h_vmem=1G,h_rt=10:00:00\","
    echo "                            requests 1GB of virtual memory and a time limit"
    echo "                            of 10 hours." 
    echo "-tdir <string>          Directory for temporary files (/tmp by default)."
    echo "                        NOTES:"
    echo "                         a) give absolute paths when using pbs clusters"
    echo "                         b) ensure there is enough disk space in the partition"
    echo "-sdir <string>          Absolute path of a directory common to all"
    echo "                        processors. If not given, \$HOME will be used."
    echo "                        NOTES:"
    echo "                         a) give absolute paths when using pbs clusters"
    echo "                         b) ensure there is enough disk space in the partition"
    echo "-debug                  After ending, do not delete temporary files"
    echo "                        (for debugging purposes)"
    echo "--help                  Display this help and exit."
    echo "--version               Output version information and exit."
}

########
is_absolute_path()
{
    case $1 in
        /*) echo 1 ;;
        *) echo 0 ;;
    esac
}

########
get_absolute_path()
{
    file=$1
    # Check if an absolute path was given
    absolute=`is_absolute_path $file`
    if [ $absolute -eq 1 ]; then
        echo $file
    else
        echo $PWD/$file
    fi
}

########
create_lm_files()
{
    # Check availability of lm files
    nlines=`ls ${lmfile}* 2>/dev/null | $WC -l`
    if [ $nlines -eq 0 ]; then
        echo "Error! language model files could not be found: ${lmfile}"
        exit 1
    fi

    # Create lm files
    for file in `ls ${lmfile}*`; do
        if [ $file = ${lmfile}.weights ]; then
            # Create regular file for the weights
            cp ${lmfile}.weights ${outd}/lm || { echo "Error while preparing language model files" >&2 ; exit 1; }
        else
            # Create hard links for the rest of the files
            $LN -f $file ${outd}/lm || { echo "Error while preparing language model files" >&2 ; exit 1; }
        fi
#        cp $file ${outd}/lm || { echo "Error while preparing language model files" >&2 ; exit 1; }
    done
}


########
lm_downhill()
{
    # Export required variables
    export LM=$newlmfile
    export TEST=$tcorpus
    export ORDER=`cat $lmfile.weights | $AWK '{printf"%d",$1}'`
    export NUMBUCK=`cat $lmfile.weights | $AWK '{printf"%d",$2}'`
    export BUCKSIZE=`cat $lmfile.weights | $AWK '{printf"%d",$3}'`
    export QS="${qs_par}"

    # Generate information for weight initialisation
va_opt=`${bindir}/thot_gen_init_file_with_jmlm_weights ${ORDER} ${NUMBUCK} ${BUCKSIZE} -0 | $AWK '{for(i=4;i<=NF;++i) printf"%s ",$i}'`
iv_opt=`${bindir}/thot_gen_init_file_with_jmlm_weights ${ORDER} ${NUMBUCK} ${BUCKSIZE} 0.5 | $AWK '{for(i=4;i<=NF;++i) printf"%s ",$i}'`

    # Execute tuning algorithm
${bindir}/thot_dhs_min -tdir $sdir -va ${va_opt} -iv ${iv_opt} \
-ftol ${ftol_lm} -o ${outd}/lm_adjw -u ${bindir}/thot_dhs_trgfunc_jmlm ${debug_opt} || exit 1
}

########
tune_lm()
{
    # Obtain path of lm file
    lmfile=`$GREP "\-lm " $cmdline_cfg | $AWK '{printf"%s",$2}'`
    baselmfile=`basename $lmfile`

    # Create directory for lm files
    if [ -d ${outd}/lm ]; then
        echo "Warning! directory for language model does exist" >&2 
    else
        mkdir -p ${outd}/lm || { echo "Error! cannot create directory for language model" >&2; exit 1; }
    fi
    
    # Create initial lm files
    create_lm_files

    # Obtain new lm file name
    newlmfile=${outd}/lm/${baselmfile}

    # Tune language model
    lm_downhill
}

########
create_tm_dev_files()
{
    # Check availability of tm_dev files
    nlines=`ls ${tmfile}* 2>/dev/null | $WC -l`
    if [ $nlines -eq 0 ]; then
        echo "Error! translation model files could not be found: ${tmfile}"
        exit 1
    fi

    # Create tm files
    for file in `ls ${tmfile}*`; do
        if [ $file != ${tmfile}.ttable ]; then
            # Create hard links for all of the files except the phrase table
            $LN -f $file ${outd}/tm_dev || { echo "Error while preparing translation model files" >&2 ; exit 1; }
        fi
    done
}

########
create_tm_files()
{
    # Check availability of tm_dev files
    nlines=`ls ${tmfile}* 2>/dev/null | $WC -l`
    if [ $nlines -eq 0 ]; then
        echo "Error! translation model files could not be found: ${tmfile}"
        exit 1
    fi

    # Create tm files
    for file in `ls ${tmfile}*`; do
            # Create hard links for each file
            $LN -f $file ${outd}/tm || { echo "Error while preparing translation model files" >&2 ; exit 1; }
    done
}

########
filter_ttable()
{
${bindir}/thot_pbs_filter_ttable -t ${tmfile}.ttable \
        -c $scorpus -n 20 -T $tdir ${qs_opt} "${qs_par}" -o ${outd}/tm_dev/${basetmfile}.ttable
}

########
create_cfg_file_for_tuning()
{
    cat $cmdline_cfg | $AWK -v nlm=$newlmfile -v ntm=$newtmdevfile \
                         '{
                           if($1=="-lm") $2=nlm
                           if($1=="-tm") $2=ntm
                           printf"%s\n",$0
                          }'
}

########
obtain_loglin_nonneg_const()
{
    smtval=`$SERVER --config 2>&1 | $GREP "\- Weights for the smt" | $AWK -F , '{for(i=1;i<=NF;++i) {if(i==1 || i==3) printf"0 "; else printf"1 "}}'`
    ecval=`$SERVER --config 2>&1 | $GREP "\- Weights for the EC model" | $AWK -F , '{for(i=1;i<=NF;++i) printf"1 "}'`
    catval=`$SERVER --config 2>&1 | $GREP "\- Weights for the assisted translator" | $AWK -F , '{for(i=1;i<=NF;++i) printf"1 "}'`

    echo "$smtval $ecval $catval"
}

########
obtain_loglin_va_opt_values()
{
    smtval="-0 -0 -0 -0 -0 -0 -0 0"
    # smtval=`$SERVER --config 2>&1 | $GREP "\- Weights for the smt" | $AWK -F , '{for(i=1;i<=NF;++i) printf"-0 "}'`
    ecval="128 0.8 -0 -0 -0"
    # ecval=`$SERVER --config 2>&1 | $GREP "\- Weights for the EC model" | $AWK -F , '{for(i=1;i<=NF;++i) printf"-0 "}'`
    catval="1 -0"
    # catval=`$SERVER --config 2>&1 | $GREP "\- Weights for the assisted translator" | $AWK -F , '{for(i=1;i<=NF;++i) printf"-0 "}'`

    echo "$smtval $ecval $catval"
}

########
obtain_loglin_iv_opt_values()
{
    smtval=`$SERVER --config 2>&1 | $GREP "\- Weights for the smt" | $AWK -F , '{for(i=1;i<=NF;++i) printf"1 "}'`
    ecval=`$SERVER --config 2>&1 | $GREP "\- Weights for the EC model" | $AWK -F , '{for(i=1;i<=NF;++i) printf"1 "}'`
    catval=`$SERVER --config 2>&1 | $GREP "\- Weights for the assisted translator" | $AWK -F , '{for(i=1;i<=NF;++i) printf"1 "}'`

    echo "$smtval $ecval $catval"
}

########
loglin_downhill()
{
    # Export required variables
    export CFGFILE=${outd}/tune_loglin.cfg
    export TEST=$scorpus
    export REF=$tcorpus
export SERVER=${bindir}/thot_server
    export SERVER_IP="127.0.0.1"
    export PORT=$RANDOM
    export QS="${qs_par}"

    # Generate information for weight initialisation
    export NON_NEG_CONST=`obtain_loglin_nonneg_const`
    va_opt=`obtain_loglin_va_opt_values`
    iv_opt=`obtain_loglin_iv_opt_values`

    # Execute tuning algorithm
${bindir}/thot_dhs_min -tdir $sdir -va ${va_opt} -iv ${iv_opt} \
-ftol ${ftol_loglin} -o ${outd}/cat_adjw -u ${bindir}/thot_dhs_cat_trgfunc ${debug_opt} || exit 1
}

########
num_smtw()
{
    ${SERVER} --config 2>&1 | grep "Weights for the smt model" | $AWK -F "," '{printf"%d",NF}'
}

########
num_catw()
{
    ${SERVER} --config 2>&1 | grep "Weights for the assisted translator" | $AWK -F "," '{printf"%d",NF}'
}

########
separate_weights()
{
    # Obtain number of weights for each model
    NUMW=`echo "$weights" | $AWK '{printf"%d",NF}'`
    NSMTW=`num_smtw`
    NCATW=`num_catw`
    NECW=`expr $NUMW - $NSMTW - $NCATW`

    # Separate weights in groups
    SMTW=`echo "$weights" | ${AWK} -v ntmw=$NSMTW '{for(i=1;i<=ntmw;++i) printf"%s ",$i;}'`
    ECW=`echo "$weights" | ${AWK} -v ntmw=$NSMTW -v necw=$NECW '{for(i=ntmw+1;i<=ntmw+necw;++i) printf"%s ",$i;}'`
    CATW=`echo "$weights" | ${AWK} -v ntmw=$NSMTW -v necw=$NECW '{for(i=ntmw+necw+1;i<=NF;++i) printf"%s ",$i;}'`
}

########
create_cfg_file_for_tuned_sys()
{
    # Print data regarding development files
    echo "# [SCRIPT_INFO] tool: thot_cat_tune"
    echo "# [SCRIPT_INFO] source dev. file: $scorpus" 
    echo "# [SCRIPT_INFO] target dev. file: $tcorpus" 
    echo "# [SCRIPT_INFO]"

    # Obtain log-linear weights
    weights=`cat ${outd}/cat_adjw.out`

    # Separate weights
    separate_weights

    # Create file from command line file
    cat ${outd}/tune_loglin.cfg | $SED s'@/tm_dev/@/tm/@'| \
        $AWK -v tmw="$SMTW" -v catw="$CATW" -v ecw="$ECW" \
                            '{
                               if($1=="#" && $2=="-tmw")
                                 printf"-tmw %s\n",tmw
                               else
                               {
                                if($1=="#" && $2=="-catw")
                                  printf"-catw %s\n",catw
                                else
                                {
                                 if($1=="#" && $2=="-ecw")
                                   printf"-ecw %s\n",ecw
                                 else
                                   printf "%s\n",$0
                                }
                               }
                             }'
}

########
tune_loglin()
{
    # Obtain path of lm file
    lmfile=`$GREP "\-lm " $cmdline_cfg | $AWK '{printf"%s",$2}'`
    baselmfile=`basename $lmfile`

    # Create directory for lm files
    if [ -d ${outd}/lm ]; then
        lm_dir_already_exist=1
    else
        mkdir -p ${outd}/lm || { echo "Error! cannot create directory for language model" >&2; exit 1; }
        lm_dir_already_exist=0
    fi

    if [ ${lm_dir_already_exist} -eq 0 ]; then
        # Create initial lm files
        create_lm_files
    fi

    # Obtain new lm file name
    newlmfile=${outd}/lm/${baselmfile}

    ######

    # Obtain path of tm file
    tmfile=`$GREP "\-tm " $cmdline_cfg | $AWK '{printf"%s",$2}'`
    basetmfile=`basename $tmfile`

    # Create directory for tm files for development corpus
    if [ -d ${outd}/tm_dev ]; then
        echo "Warning! directory for dev. translation model does exist" >&2 
    else
        mkdir -p ${outd}/tm_dev || { echo "Error! cannot create directory for translation model" >&2; exit 1; }
    fi

    # Create initial tm_dev files
    create_tm_dev_files

    # Obtain new tm file name for development corpus
    newtmdevfile=${outd}/tm_dev/${basetmfile}

    # Filter translation table
    filter_ttable

    # Create cfg file for tuning
    create_cfg_file_for_tuning > ${outd}/tune_loglin.cfg

    # Tune log-linear model
    loglin_downhill

    ######

    # Create directory for tm files
    if [ -d ${outd}/tm ]; then
        echo "Warning! directory for translation model does exist" >&2 
    else
        mkdir -p ${outd}/tm || { echo "Error! cannot create directory for translation model" >&2; exit 1; }
    fi

    # Create initial tm files
    create_tm_files

    # Create cfg file of tuned system
    create_cfg_file_for_tuned_sys > ${outd}/tuned_for_dev.cfg
}

########
if [ $# -lt 1 ]; then
    print_desc
    exit 1
fi

# Read parameters
pr_given=0
pr_val=1
c_given=0
s_given=0
t_given=0
o_given=0
qs_given=0
unk_given=0
tdir_given=0
tdir="/tmp"
sdir_given=0
sdir=$HOME
debug=0

while [ $# -ne 0 ]; do
    case $1 in
        "--help") usage
            exit 0
            ;;
        "--version") version
            exit 0
            ;;
        "-pr") shift
            if [ $# -ne 0 ]; then
                pr_val=$1
                pr_given=1
            fi
            ;;
        "-c") shift
            if [ $# -ne 0 ]; then
                cmdline_cfg=$1
                c_given=1
            fi
            ;;
        "-s") shift
            if [ $# -ne 0 ]; then
                scorpus=$1
                s_given=1
            fi
            ;;
        "-t") shift
            if [ $# -ne 0 ]; then
                tcorpus=$1
                t_given=1
            fi
            ;;
        "-o") shift
            if [ $# -ne 0 ]; then
                outd=$1
                o_given=1
            fi
            ;;
        "-qs") shift
            if [ $# -ne 0 ]; then
                qs_opt="-qs"
                qs_par="$1"
                qs_given=1
            else
                qs_given=0
            fi
            ;;
        "-unk") unk_given=1
            unk_opt="-unk"
            ;;
        "-tdir") shift
            if [ $# -ne 0 ]; then
                tdir=$1
                tdir_given=1
            fi
            ;;
        "-sdir") shift
            if [ $# -ne 0 ]; then
                sdir=$1
                sdir_given=1
            fi
            ;;
        "-debug") debug=1
            debug_opt="-debug"
            ;;
    esac
    shift
done

# Check parameters
if [ ${c_given} -eq 0 ]; then
    echo "Error! -cfg parameter not given" >&2
    exit 1
else
    if [ ! -f ${cmdline_cfg} ]; then
        echo "Error! file ${cmdline_cfg} does not exist" >&2
        exit 1
    else
        # Obtain absolute path
        cmdline_cfg=`get_absolute_path ${cmdline_cfg}`
    fi
fi

if [ ${s_given} -eq 0 ]; then
    echo "Error! -s parameter not given!" >&2
    exit 1
else
    if [ ! -f ${scorpus} ]; then
        echo "Error! file ${scorpus} does not exist" >&2
        exit 1
    else
        # Obtain absolute path
        scorpus=`get_absolute_path $scorpus`
    fi
fi

if [ ${t_given} -eq 0 ]; then        
    echo "Error! -t parameter not given!" >&2
    exit 1
else
    if [ ! -f ${tcorpus} ]; then
        echo "Error! file ${tcorpus} does not exist" >&2
        exit 1
    else
        # Obtain absolute path
        tcorpus=`get_absolute_path $tcorpus`
    fi
fi

if [ ${o_given} -eq 0 ]; then
    echo "Error! -o parameter not given!" >&2
    exit 1
else
    if [ -d ${outd} ]; then
        echo "Warning! output directory does exist" >&2 
        # echo "Error! output directory should not exist" >&2 
        # exit 1
    else
        mkdir -p ${outd} || { echo "Error! cannot create output directory" >&2; exit 1; }
    fi
    # Obtain absolute path
    outd=`get_absolute_path $outd`
fi

if [ ${tdir_given} -eq 1 ]; then
    if [ ! -d ${tdir} ]; then
        echo "Error! directory ${tdir} does not exist" >&2
        exit 1            
    fi
fi

if [ ${sdir_given} -eq 1 ]; then
    if [ ! -d ${sdir} ]; then
        echo "Error! directory ${sdir} does not exist" >&2
        exit 1            
    fi
fi

# Set default parameters
ftol_lm=0.1
ftol_loglin=0.1

# Tune models
tune_lm
tune_loglin
