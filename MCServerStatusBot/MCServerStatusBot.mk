##
## Auto Generated makefile by CodeLite IDE
## any manual changes will be erased      
##
## Debug
ProjectName            :=MCServerStatusBot
ConfigurationName      :=Debug
WorkspaceConfiguration :=Debug
WorkspacePath          :=/home/mart/Dev/MCServerStatusBot
ProjectPath            :=/home/mart/Dev/MCServerStatusBot/MCServerStatusBot
IntermediateDirectory  :=../build-$(WorkspaceConfiguration)/MCServerStatusBot
OutDir                 :=$(IntermediateDirectory)
CurrentFileName        :=
CurrentFilePath        :=
CurrentFileFullPath    :=
User                   :=mart
Date                   :=11/10/23
CodeLitePath           :=/home/mart/.codelite
MakeDirCommand         :=mkdir -p
LinkerName             :=/bin/clang++
SharedObjectLinkerName :=/bin/clang++ -shared -fPIC
ObjectSuffix           :=.o
DependSuffix           :=.o.d
PreprocessSuffix       :=.i
DebugSwitch            :=-g 
IncludeSwitch          :=-I
LibrarySwitch          :=-l
OutputSwitch           :=-o 
LibraryPathSwitch      :=-L
PreprocessorSwitch     :=-D
SourceSwitch           :=-c 
OutputDirectory        :=/home/mart/Dev/MCServerStatusBot/build-$(WorkspaceConfiguration)/bin
OutputFile             :=../build-$(WorkspaceConfiguration)/bin/$(ProjectName)
Preprocessors          :=
ObjectSwitch           :=-o 
ArchiveOutputSwitch    := 
PreprocessOnlySwitch   :=-E
ObjectsFileList        :=$(IntermediateDirectory)/ObjectsList.txt
PCHCompileFlags        :=
LinkOptions            :=  
IncludePath            :=  $(IncludeSwitch). $(IncludeSwitch)$(WorkspacePath)/depend/ $(IncludeSwitch). 
IncludePCH             := 
RcIncludePath          := 
Libs                   := $(LibrarySwitch)ssl $(LibrarySwitch)crypto $(LibrarySwitch)dpp 
ArLibs                 :=  "ssl" "crypto" "dpp" 
LibPath                := $(LibraryPathSwitch). 

##
## Common variables
## AR, CXX, CC, AS, CXXFLAGS and CFLAGS can be overridden using an environment variable
##
AR       := /bin/ar rcu
CXX      := /bin/clang++
CC       := /bin/clang
CXXFLAGS := -std=c++20 -gdwarf-2 -O0 -Wall $(Preprocessors)
CFLAGS   := -std=c++20 -gdwarf-2 -O0 -Wall $(Preprocessors)
ASFLAGS  := 
AS       := /bin/as


##
## User defined environment variables
##
CodeLiteDir:=/usr/share/codelite
Objects0=$(IntermediateDirectory)/main.cpp$(ObjectSuffix) $(IntermediateDirectory)/botresponses.cpp$(ObjectSuffix) $(IntermediateDirectory)/network.cpp$(ObjectSuffix) 



Objects=$(Objects0) 

##
## Main Build Targets 
##
.PHONY: all clean PreBuild PrePreBuild PostBuild MakeIntermediateDirs
all: MakeIntermediateDirs $(OutputFile)

$(OutputFile): $(IntermediateDirectory)/.d $(Objects) 
	@$(MakeDirCommand) "$(IntermediateDirectory)"
	@echo "" > $(IntermediateDirectory)/.d
	@echo $(Objects0)  > $(ObjectsFileList)
	$(LinkerName) $(OutputSwitch)$(OutputFile) @$(ObjectsFileList) $(LibPath) $(Libs) $(LinkOptions)

MakeIntermediateDirs:
	@$(MakeDirCommand) "$(IntermediateDirectory)"
	@$(MakeDirCommand) "$(OutputDirectory)"

$(IntermediateDirectory)/.d:
	@$(MakeDirCommand) "$(IntermediateDirectory)"

PreBuild:


##
## Objects
##
$(IntermediateDirectory)/main.cpp$(ObjectSuffix): main.cpp $(IntermediateDirectory)/main.cpp$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "/home/mart/Dev/MCServerStatusBot/MCServerStatusBot/main.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/main.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/main.cpp$(DependSuffix): main.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/main.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/main.cpp$(DependSuffix) -MM main.cpp

$(IntermediateDirectory)/main.cpp$(PreprocessSuffix): main.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/main.cpp$(PreprocessSuffix) main.cpp

$(IntermediateDirectory)/botresponses.cpp$(ObjectSuffix): botresponses.cpp $(IntermediateDirectory)/botresponses.cpp$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "/home/mart/Dev/MCServerStatusBot/MCServerStatusBot/botresponses.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/botresponses.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/botresponses.cpp$(DependSuffix): botresponses.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/botresponses.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/botresponses.cpp$(DependSuffix) -MM botresponses.cpp

$(IntermediateDirectory)/botresponses.cpp$(PreprocessSuffix): botresponses.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/botresponses.cpp$(PreprocessSuffix) botresponses.cpp

$(IntermediateDirectory)/network.cpp$(ObjectSuffix): network.cpp $(IntermediateDirectory)/network.cpp$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "/home/mart/Dev/MCServerStatusBot/MCServerStatusBot/network.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/network.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/network.cpp$(DependSuffix): network.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/network.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/network.cpp$(DependSuffix) -MM network.cpp

$(IntermediateDirectory)/network.cpp$(PreprocessSuffix): network.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/network.cpp$(PreprocessSuffix) network.cpp


-include $(IntermediateDirectory)/*$(DependSuffix)
##
## Clean
##
clean:
	$(RM) -r $(IntermediateDirectory)


