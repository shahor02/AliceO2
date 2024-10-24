#!/usr/bin/env bash

inp=particles.root
seed=$RANDOM
nev=1
out=

usage()
{
    cat <<EOF
Usage: $0 [OPTIONS]

Options:

  -i,--input   FILENAME    Input HepMC file ($inp)
  -s,--seed    SEED        Random number seed ($seed)
  -n,--nevents EVENTS      Number of events ($nev)
  -o,--output  NAME        Ouptut name
  --                       Rest of command line sent to o2-sim

The input file FILENAME is assumed to contain a TTree with a
TBranch holding a TClonesArray of TParticle objects. Such a
file can be generated by the script myeg.sh in this directory.

Below follows the help output of o2-sim

EOF
}

while test $# -gt 0 ; do
    case $1 in
        -i|--input)   inp=$2 ; shift ;;
        -s|--seed)    seed=$2 ; shift ;;
        -n|--nevents) nev=$2 ; shift ;;
        -o|--output)  out=$2 ; shift ;;
        -h|--help) usage; o2-sim --help full ; exit 0 ;;
        --)           shift ; break ;;
        *) echo "Unknown option '$1', did you forget '--'?" >/dev/stderr
           exit 1
           ;;
    esac
    shift
done

if test "x$out" = "x" ; then
    out=`basename $inp .root`
fi
out=`echo "$out" | tr ' ' '_'`

set -e
export VMCWORKDIR=${O2_ROOT}/share
o2-sim -g tparticle --configKeyValues "GeneratorFileOrCmd.fileNames=${inp}" \
  --outPrefix "$out" --seed $seed --nEvents $nev $@

