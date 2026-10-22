
print_usage() {
    echo Usage:
    echo sh build-transpiler.sh [CC] [CFLAGS]
}

if [ $# -lt 1 ]; then
    echo [CC] is missing
    print_usage
    exit 1
fi

CC=$1
CFLAGS=$2

echo CC=$CC
echo CFLAGS=$CFLAGS

echo
echo Building build/transpiler
echo $CC $CFLAGS -o build/transpiler transpiler.c 
$CC $CFLAGS -o build/transpiler transpiler.c || exit 1
