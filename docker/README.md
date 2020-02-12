

# Build docker container for wdb2ts

## Build dependencies

### wdb2ts-build-deps
Create a simple container with all ubuntu packages installed and build the cetcd library (etcd c-interface) and the weathet_symbol library.
 
The resulting library and header files is copied to the volume /artifacts.

``` 
  docker build -f build/Dockerfile_wdb2ts_build_deps  -t wdb2ts-build-deps .
```

### wdb2ts-build
Create a container to build wdb2ts, wdb2ts-build. It depends on wdb2ts-build-deps.
 
```
  docker build -f build/Dockerfile_wdb2ts_build  -t wdb2ts-build .
```
    
The container has three volumes that can be overrided
  	
  - VOLUME /artifacts
  - VOLUME /src
  - VOLUME /build
 
This container can be run in two modes. 
  
 - To build wdb2ts and copy the result it to the /artifacts volume and exit.
 - For development. Give a bash prompt after configuring the wdb2ts source tree. 
  	
```
 docker run -v /path/to/src:/src -v /path/to/buld:/build -v /path/to/artifacts:/artifacts wdb2ts-build OPTION 
``` 
 
Where OPTION is --devel or --build, --build is the default.
 	
