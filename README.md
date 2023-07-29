# c
The C Language implementation of the FCAP protocol

## Development
Requirements
* vscode
* docker

Development is exclusively within the dev container and vscode. This sets up the the build system (cmake) and all required dependencies using vcpkg.

### Compiling
The library can be run using the vscode build function. This will automatically use the cmake config and gcc toolchain

### Testing
Unit tests can be run with CTest. This uses the GTest framework.