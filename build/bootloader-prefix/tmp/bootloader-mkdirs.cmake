# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

cmake_minimum_required(VERSION 3.5)

file(MAKE_DIRECTORY
  "/home/tazukiswift/esp/esp-idf/components/bootloader/subproject"
  "/home/tazukiswift/UoA/FSAE/Dash/dashboard/build/bootloader"
  "/home/tazukiswift/UoA/FSAE/Dash/dashboard/build/bootloader-prefix"
  "/home/tazukiswift/UoA/FSAE/Dash/dashboard/build/bootloader-prefix/tmp"
  "/home/tazukiswift/UoA/FSAE/Dash/dashboard/build/bootloader-prefix/src/bootloader-stamp"
  "/home/tazukiswift/UoA/FSAE/Dash/dashboard/build/bootloader-prefix/src"
  "/home/tazukiswift/UoA/FSAE/Dash/dashboard/build/bootloader-prefix/src/bootloader-stamp"
)

set(configSubDirs )
foreach(subDir IN LISTS configSubDirs)
    file(MAKE_DIRECTORY "/home/tazukiswift/UoA/FSAE/Dash/dashboard/build/bootloader-prefix/src/bootloader-stamp/${subDir}")
endforeach()
