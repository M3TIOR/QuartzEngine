include(Platform/Windows-MSVC)
set(_COMPILE_CPCH " /TP")
if(NOT CMAKE_C_COMPILER_VERSION VERSION_LESS 18.0)
  set(_FS_CPCH " /FS")
endif()
__windows_compiler_msvc(CPCH)