# Doxygen configuration

set(SRCDIR ${CMAKE_SOURCE_DIR}/src)
set(doxyfile_in ${CMAKE_SOURCE_DIR}/doc/Doxyfile.in)
set(doxyfile ${CMAKE_BINARY_DIR}/doc/Doxyfile)
configure_file(${doxyfile_in} ${doxyfile} @ONLY)
add_custom_target(doc ALL
  ${DOXYGEN_EXECUTABLE} ${doxyfile}
  WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/doc
  COMMENT "Generating API documentation with Doxygen" VERBATIM
)
