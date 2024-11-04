IF(NO_ETISS EQUAL 1)
    message("Using softvector-internal softfloat library")
    ADD_SUBDIRECTORY(softfloat)
ENDIF()