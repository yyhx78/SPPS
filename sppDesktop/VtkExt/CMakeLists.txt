
# Set your files and resources here
SET(SimAdsVtkExt 
	adsContourGrid.cpp adsContourGrid.h
	adsCubeAxesActor2D.cpp adsCubeAxesActor2D.h
	adsCutter.cpp adsCutter.h
	adsOpenGLPolyDataMapper.cpp adsOpenGLPolyDataMapper.h
	adsStreamer.cpp adsStreamer.h
	adsStreamLine.cpp adsStreamLine.h
	mfopenglpolydatamapper.cpp mfopenglpolydatamapper.h
	stdafx.h
	StreamLine.cpp adsStreamLine.h
    )

ADD_LIBRARY( AdsVtkExt ${SimAdsVtkExt})




