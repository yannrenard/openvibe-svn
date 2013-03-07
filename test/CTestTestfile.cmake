# This file specifies the test which should be executed

# The file should be placed in the binary directory ${CTEST_BINARY_DIRECTORY}.

#Each test consists of:

#    The unique name of the test ( eg.: testname1 )
#    The full path to the executable of the test ( eg.: "$ENV{HOME}/bin/TEST_EXECUTABLE_1.sh" )
#    A List of arguments to the executable ( eg.: "ARGUMENT_1" "ARGUMENT_2" etc. ) 


# basic test (just for sample) check that binary directory is readable 
#ADD_TEST(LS_BINARY_PATH "ls" "-all")
#ADD_TEST(PWD_BINARY_PATH "pwd")

## -- Other Tests : Place a file named DartTestfile.txt in this path with tests.

SUBDIRS("${CTEST_SOURCE_DIRECTORY}/openvibe-applications/designer/trunc/test")
SUBDIRS("${CTEST_SOURCE_DIRECTORY}/openvibe-plugins/samples/trunc/test")
SUBDIRS("${CTEST_SOURCE_DIRECTORY}/openvibe-kernel-omk/trunc/test")
SUBDIRS("${CTEST_SOURCE_DIRECTORY}/openvibe-modules/trunc/test")
SUBDIRS("${CTEST_SOURCE_DIRECTORY}/openvibe-toolkit/trunc/test")
SUBDIRS("${CTEST_SOURCE_DIRECTORY}/openvibe-plugins/trunc/test")
SUBDIRS("${CTEST_SOURCE_DIRECTORY}/openvibe-scenarios/trunc/test")
SUBDIRS("${CTEST_SOURCE_DIRECTORY}/openvibe-externals/trunc/test")



 
