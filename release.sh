if [[ "$OSTYPE" == "linux-gnu"* ]]; then
    EXT=so
elif [[ "$OSTYPE" == "darwin"* ]]; then
    EXT=dylib
else
        exit -1
fi

BUILD_DIR=build
RELEASE_DIR=release

if [ -d "$RELEASE_DIR" ]; then
    rm -r "$RELEASE_DIR"
fi

mkdir -p release/bin
mkdir -p release/lib

cp $BUILD_DIR/programs/cleos/cleos release/bin/cleos || exit 1
cp $BUILD_DIR/programs/keosd/keosd release/bin/keosd || exit 1
cp $BUILD_DIR/programs/nodeos/nodeos release/bin/nodeos || exit 1
cp $BUILD_DIR/programs/ipyeos/ipyeos release/bin/ipyeos || exit 1
cp $BUILD_DIR/libraries/chain_api/libchain_api.$EXT release/lib/libchain_api.$EXT || exit 1
cp $BUILD_DIR/libraries/vm_api/libvm_api.$EXT release/lib/libvm_api.$EXT || exit 1

if [[ "$OSTYPE" == "linux-gnu"* ]]; then
    tar -czf release-linux.tar.gz release
elif [[ "$OSTYPE" == "darwin"* ]]; then
    tar -czf release-macos.tar.gz release
else
    exit -1
fi
