VERSION=v1.0.0
TARGET=fork
git push $TARGET :refs/tags/$VERSION
git tag -d $VERSION
git tag $VERSION -m $VERSION
git push -f $TARGET $VERSION
