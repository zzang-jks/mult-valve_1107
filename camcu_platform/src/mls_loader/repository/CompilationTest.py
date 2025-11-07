# -*- coding: utf-8 -*-
"""
Created on Mon Oct 18/10/2017

@author: grm
"""

import sys, subprocess

makecommand = "make"

# Compilation test makefile
makearguments = "-f"
makefile = "CompilationTest.mk"

# Target to get the values list of specified variable
maketarget = "get_var"
makegetvar = "GET_VAR="
makesilent = "-s"
makeaddargs = ""

# Clean and make tagrets with for selected configuration
maketarget_clean = "clean"
maketarget_build = "all"

# if argument list is empty then call the makefile with that
if (len(sys.argv) > 1):
    makeaddargs = " ".join(sys.argv[1:])
    print("Make add arguments: ", makeaddargs)

# Get the list of optional variables
cmd = subprocess.Popen(makecommand+" "+makearguments+" "+makefile+" "
                        +maketarget+" "+makesilent+" "+makeaddargs,
                        stdout=subprocess.PIPE, stderr=subprocess.PIPE, shell=True)
cmd_out, cmd_err = cmd.communicate()

optionalVariablesList = cmd_out.decode(sys.stdout.encoding).split()
print("List of optional variables:", optionalVariablesList)

allOptVarsList = []

# Get values list for each optional variable
for optionalVar in optionalVariablesList:
    cmd = subprocess.Popen(makecommand+" "+makearguments+" "+makefile+" "
                            +maketarget+" "+makesilent+" "+makegetvar+optionalVar+" "+makeaddargs,
                            stdout=subprocess.PIPE, stderr=subprocess.PIPE, shell=True)
    cmd_out, cmd_err = cmd.communicate()
    optValuesList = cmd_out.decode(sys.stdout.encoding).split()
    allOptVarsList.append([optionalVar, optValuesList])
    print(optionalVar, ":", optValuesList)
print("")

# Protect from empty configuration list
if (len(allOptVarsList) == 0):
    print("Optional variable list is empty!; exit")
    sys.exit(1)

# [0] - Max values for each optionalVariable
# [1] - Current value for each optionalVariable (initialized with "1")
for optVar in allOptVarsList:
    lenOptVar = len(optVar[1])
    # The check is needed to make the iterator work correclty with empty values list
    if (len(optVar[1]) > 0):
        optVar.append([lenOptVar, 1])
    else:
        optVar.append([lenOptVar, 0])

# Generate build configurations for all options combinations
configurations = []
continueIterate = True

# Form the configuration string (iterate through all options combinations)
while (continueIterate):
    # create one configuration
    confString = ""
    for optVar in allOptVarsList:
        currVarIdx = optVar[2][1] - 1
        if (optVar[2][0] != 0):
            confString = confString + optVar[0].replace("OPT_", "SEL_", 1) + "=\"" + optVar[1][currVarIdx] + "\" "

    # save configuration
    configurations.append([confString,"TO BE CHECKED"])

    # iterate through all options combinations
    optVarIdxMax = len(allOptVarsList) - 1

    # a. increment the lowest option index value
    allOptVarsList[optVarIdxMax][2][1] = allOptVarsList[optVarIdxMax][2][1] + 1

    for idx in range (0, optVarIdxMax + 1):
        workingOptVarIdx = optVarIdxMax - idx
        if (allOptVarsList[workingOptVarIdx][2][1] > allOptVarsList[workingOptVarIdx][2][0]):
            allOptVarsList[workingOptVarIdx][2][1] = 1

            if (workingOptVarIdx != 0):
                hiOptVarIdx = workingOptVarIdx - 1
                allOptVarsList[hiOptVarIdx][2][1] = allOptVarsList[hiOptVarIdx][2][1] + 1
            else:
                continueIterate = False

commonReturnCode = 0;

# Run all the options one by one
for configuration in configurations:
    output = makecommand+" "+makearguments+" "+makefile+" "+maketarget_clean+" "\
             +maketarget_build+" "+configuration[0]+" "+makeaddargs
    cmd = subprocess.Popen(output, stdout=subprocess.PIPE, stderr=subprocess.STDOUT, shell=True)
    cmd_out, cmd_err = cmd.communicate()
    output = output + "\n" + cmd_out.decode(sys.stdout.encoding)
    retCode = cmd.returncode

    # in the case of error
    if (retCode != 0):
        print("*** FAILED ***")
        print("Failed configuration: \n", configuration[0])
        print("\nReturn code: ", retCode)
        print("\nError logs:")
        print(output)
        configuration[1]="FAILED"
        commonReturnCode = retCode
    else: # for normal build
        print("*** PASSED ***")
        print("Configuration:", configuration[0], "\n")
        print(output)
        configuration[1]="PASSED"

# Output the build summary
print("\n=== Configurations build summary ===")
for configuration in configurations:
    print(configuration[1], ": ", configuration[0])

sys.exit(commonReturnCode)
#