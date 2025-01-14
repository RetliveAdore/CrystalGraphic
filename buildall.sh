cp ./platform/linux/makefile ./makefile
in_label=$1
if [ ! -n $in_label ]; then in_label="debug";fi
if [ "$in_label" = "release" ];
then
    echo building release
    make build
    echo building test
    make testrel
    ./out/Linux/release/Test1
else
    echo building debug
    make debug
    echo building test
    make testdbg
    ./out/Linux/debug/Test1
fi