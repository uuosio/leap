VERSION=v1.0.3
TARGET=fork
git push $TARGET :refs/tags/$VERSION
git tag -d $VERSION
git tag $VERSION -m $VERSION
git push -f $TARGET $VERSION
