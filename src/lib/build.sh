#! /bin/sh

# $1 = prefix of (installed) lib dir.
# $2 = prefix of lib dir (in source).
# $3 = library name
# $4 = xtra arguments to configure.

cd $2/$3

./configure --prefix=$1 --disable-shared --enable-static $4 && \
make && \
make INST_GNOME_CONFIG_HACK=$1 install && \
touch $2/$3.build
