#
# file: 	building daq_browser2.Makefle (for building daq_browser2.mexa64)  
# created on:	2019 Sep 03
# created by:	Davit Kalantaryan (davit.kalantaryan@desy.de)
#

# Some declared variables
projectName	:= mexdaq_browser2
outputName	:= daq_browser2.mexa64
cCompiler	:= gcc
cppCompiler	:= g++
aLinker 	:= g++
cParamsDecl 	:= -fPIC -I/afs/ifh.de/SL/6/x86_64/opt/matlab/R2016b/extern/include
linkParams	:= -Wl,-E -pie -ldl

# Some calculations (for some of them soultions found from websides below)  
# https://stackoverflow.com/questions/18136918/how-to-get-current-relative-directory-of-your-makefile/23324703  
# https://stackoverflow.com/questions/18136918/how-to-get-current-relative-directory-of-your-makefile  
# https://www.gnu.org/software/make/manual/html_node/File-Name-Functions.html  
# https://stackoverflow.com/questions/4879592/whats-the-difference-between-and-in-makefile  

lsbCodeName 	:= $(shell lsb_release -c | cut -f 2)
mkfile_path	:= $(abspath $(lastword $(MAKEFILE_LIST)))
mkfile_dir	:= $(dir $(mkfile_path))

# Some more calculations
targetName	:= $(projectName)
repositoryRoot	:= $(mkfile_dir)../../..
sourceBaseDir	:= $(repositoryRoot)/src
targetDir	:= $(repositoryRoot)/sys/$(lsbCodeName)/mbin
objectFilesDir	:= $(repositoryRoot)/sys/$(lsbCodeName)/.objects/$(projectName)
targetPath	:= $(targetDir)/$(outputName)
cParams 	:= $(cParamsDecl) -I$(sourceBaseDir)/tools2
cppParams	:= $(cParams) -std=c++0x


# Now let's specify all objects, that we will need for finall linking
OBJECTS = \
	$(objectFilesDir)/tools2/mainentry_daq_browser2.o		\
	$(objectFilesDir)/tools2/mexentry_daq_browser2.o		\
	$(objectFilesDir)/tools2/daq_root_reader.o

default: $(targetName)


$(objectFilesDir)/%.o: $(sourceBaseDir)/%.cpp
	mkdir -p $(@D)
	$(cppCompiler) $(cppParams) -o $@ -c $<
	
$(objectFilesDir)/%.o: $(sourceBaseDir)/%.c
	mkdir -p $(@D)
	$(cCompiler) $(cParams) -o $@ -c $<


$(targetName): $(OBJECTS)
	mkdir -p $(targetDir)
	$(aLinker) $(OBJECTS) $(linkParams) -o $(targetPath)

#install: $(TARGET_NAME)
#	sudo cp $(TARGET_FILE_PATH) /boot/.

# .PHONY means always out of date
.PHONY: clean

clean:
	rm -f $(OBJECTS) $(targetPath)	
