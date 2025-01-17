LIB_DIR="/usr/lib/x86_64-linux-gnu"
run_cmake() {
	cmake -L . -B cmbuild \
		-DQt_Root=$LIB_DIR \
		-DFEBio_SDK=febio-sdk \
		-DFFMPEG_LIB_DIR=$LIB_DIR \
		-DUSE_FFMPEG=ON \
		-DTETGEN_LIB_DIR=$LIB_DIR \
		-DUSE_TETGEN=ON \
		-DUSE_MMG=ON \
		-DSSH_LIB_DIR=$LIB_DIR \
		-DUSE_SSH=ON \
		-DUSE_SSL=ON \
		-DCAD_FEATURES=OFF \
		-DNETGEN_LIB_DIR=/usr/local \
		-DUSE_NETGEN=ON \
		-DUSE_ITK=ON
}
