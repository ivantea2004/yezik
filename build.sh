
print_usage() {
    echo Usage:
    echo sh build.sh [in] [out] [CC] [CFLAGS]
}

if [ $# -lt 1 ]; then
    echo [in] is missing
    print_usage
    exit 1
fi
if [ $# -lt 2 ]; then
    echo [out] is missing
    print_usage
    exit 1
fi
if [ $# -lt 3 ]; then
    echo [CC] is missing
    print_usage
    exit 1
fi

in=$1
out=$2
CC=$3
CFLAGS=$4

echo CC=$CC
echo CFLAGS=$CFLAGS

echo
echo Building build/transpiler
echo $CC $CFLAGS -o build/transpiler transpiler.c 
$CC $CFLAGS -o build/transpiler transpiler.c || exit 1

echo
echo Transpiling $in
echo ./build/transpiler $in $out.c
./build/transpiler $in $out.c || exit 1

echo
echo Building $out
echo $CC $CFLAGS -o $out $out.c
$CC $CFLAGS -o $out $out.c || exit 1
