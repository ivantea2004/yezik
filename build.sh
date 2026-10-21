
usage_error() {
    echo Usage:
    echo sh build.sh in.yezik build/out [CC] [CFLAGS]
    exit 1
}

if [ $# -lt 1 ]; then
    echo error: Missing input file
    usage_error
fi

if [ $1 = "--transpiler-only" ]; then
    if [ $# -lt 2 ]; then 
        echo error: Missing CC
        usage_error
    fi
    build_transpiler_only=1
    CC=$2
    CFLAGS=$3
else
    if [ $# -lt 2 ]; then 
        echo error: Missing output file
        usage_error
    fi
    if [ $# -lt 3 ]; then 
        echo error: Missing CC
        usage_error
    fi
    build_transpiler_only=
    in=$1
    out=$2
    CC=$3
    CFLAGS=$4
fi

echo Using CC=$CC
echo Using CFLAGS=$CFLAGS
echo

echo Building bootstrap transpiler
echo $CC $CFLAGS bootstrap-transpiler.c -o build/bootstrap-transpiler
$CC $CFLAGS bootstrap-transpiler.c -o build/bootstrap-transpiler || exit 1

if [ -z "$build_transpiler_only" ]; then
    echo
    echo Building $in

    echo ./build/bootstrap-transpiler $in $out.c
    ./build/bootstrap-transpiler $in $out.c || exit 1

    echo $CC $CFLAGS $out.c -o $out
    $CC $CFLAGS $out.c -o $out || exit 1
fi
