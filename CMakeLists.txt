cmake_minimum_required(VERSION 3.20)
project(kaleidoscope-llvm)

find_package(LLVM REQUIRED CONFIG)

file(GLOB_RECURSE KAL_SRC_FILES RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" src/*.cpp)
file(GLOB_RECURSE KAL_HEADER_FILES RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" include /*.h)

set(CMAKE_CXX_STANDARD 17)

if(WIN32)
    add_definitions("/wd4624")
endif(WIN32)

message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")
message(STATUS "Using LLVMConfig.cmake in: ${LLVM_DIR}")

add_executable(kaleidoscope-compiler ${KAL_SRC_FILES} ${KAL_HEADER_FILES})
target_include_directories(kaleidoscope-compiler PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include ${LLVM_INCLUDE_DIRS})

separate_arguments(LLVM_DEFINITIONS_LIST NATIVE_COMMAND ${LLVM_DEFINITIONS})
message(STATUS "LLVM Definitions : ${LLVM_DEFINITIONS}")
target_compile_definitions(kaleidoscope-compiler PRIVATE ${LLVM_DEFINITIONS_LIST} /wd4624)

set(llvm_libs   LLVMWindowsManifest LLVMXRay LLVMLibDriver LLVMDlltoolDriver LLVMTelemetry
                LLVMTextAPIBinaryReader LLVMCoverage LLVMLineEditor LLVMXCoreDisassembler
                LLVMXCoreCodeGen LLVMXCoreDesc LLVMXCoreInfo LLVMX86TargetMCA LLVMX86Disassembler
                LLVMX86AsmParser LLVMX86CodeGen LLVMX86Desc LLVMX86Info LLVMWebAssemblyDisassembler
                LLVMWebAssemblyAsmParser LLVMWebAssemblyCodeGen LLVMWebAssemblyUtils LLVMWebAssemblyDesc
                LLVMWebAssemblyInfo LLVMVEDisassembler LLVMVEAsmParser LLVMVECodeGen LLVMVEDesc LLVMVEInfo
                LLVMSystemZDisassembler LLVMSystemZAsmParser LLVMSystemZCodeGen LLVMSystemZDesc LLVMSystemZInfo LLVMSPIRVCodeGen
                LLVMSPIRVDesc LLVMSPIRVInfo LLVMSPIRVAnalysis LLVMSparcDisassembler LLVMSparcAsmParser LLVMSparcCodeGen
                LLVMSparcDesc LLVMSparcInfo LLVMRISCVTargetMCA LLVMRISCVDisassembler LLVMRISCVAsmParser LLVMRISCVCodeGen
                LLVMRISCVDesc LLVMRISCVInfo LLVMPowerPCDisassembler LLVMPowerPCAsmParser LLVMPowerPCCodeGen LLVMPowerPCDesc
                LLVMPowerPCInfo LLVMNVPTXCodeGen LLVMNVPTXDesc LLVMNVPTXInfo LLVMMSP430Disassembler LLVMMSP430AsmParser
                LLVMMSP430CodeGen LLVMMSP430Desc LLVMMSP430Info LLVMMipsDisassembler LLVMMipsAsmParser LLVMMipsCodeGen
                LLVMMipsDesc LLVMMipsInfo LLVMLoongArchDisassembler LLVMLoongArchAsmParser LLVMLoongArchCodeGen
                LLVMLoongArchDesc LLVMLoongArchInfo LLVMLanaiDisassembler LLVMLanaiCodeGen LLVMLanaiAsmParser
                LLVMLanaiDesc LLVMLanaiInfo LLVMHexagonDisassembler LLVMHexagonCodeGen LLVMHexagonAsmParser
                LLVMHexagonDesc LLVMHexagonInfo LLVMBPFDisassembler LLVMBPFAsmParser LLVMBPFCodeGen LLVMBPFDesc
                LLVMBPFInfo LLVMAVRDisassembler LLVMAVRAsmParser LLVMAVRCodeGen LLVMAVRDesc LLVMAVRInfo
                LLVMARMDisassembler LLVMARMAsmParser LLVMARMCodeGen LLVMARMDesc LLVMARMUtils LLVMARMInfo
                LLVMAMDGPUTargetMCA LLVMAMDGPUDisassembler LLVMAMDGPUAsmParser LLVMAMDGPUCodeGen LLVMAMDGPUDesc
                LLVMAMDGPUUtils LLVMAMDGPUInfo LLVMAArch64Disassembler LLVMAArch64AsmParser LLVMAArch64CodeGen
                LLVMAArch64Desc LLVMAArch64Utils LLVMAArch64Info LLVMOrcDebugging LLVMOrcJIT LLVMWindowsDriver
                LLVMMCJIT LLVMJITLink LLVMInterpreter LLVMExecutionEngine LLVMRuntimeDyld LLVMOrcTargetProcess
                LLVMOrcShared LLVMDWP LLVMDebugInfoLogicalView LLVMDebugInfoGSYM LLVMOption LLVMObjectYAML
                LLVMObjCopy LLVMMCA LLVMMCDisassembler LLVMLTO LLVMPasses LLVMHipStdPar LLVMCFGuard LLVMCoroutines
                LLVMipo LLVMVectorize LLVMSandboxIR LLVMLinker LLVMInstrumentation LLVMFrontendOpenMP
                LLVMFrontendOffloading LLVMFrontendOpenACC LLVMFrontendHLSL LLVMFrontendDriver LLVMFrontendAtomic
                LLVMExtensions LLVMDWARFLinkerParallel LLVMDWARFLinkerClassic LLVMDWARFLinker LLVMGlobalISel
                LLVMMIRParser LLVMAsmPrinter LLVMSelectionDAG LLVMCodeGen LLVMTarget LLVMObjCARCOpts LLVMCodeGenTypes
                LLVMCGData LLVMIRPrinter LLVMInterfaceStub LLVMFileCheck LLVMFuzzMutate LLVMScalarOpts LLVMInstCombine
                LLVMAggressiveInstCombine LLVMTransformUtils LLVMBitWriter LLVMAnalysis LLVMProfileData LLVMSymbolize
                LLVMDebugInfoBTF LLVMDebugInfoPDB LLVMDebugInfoMSF LLVMDebugInfoCodeView LLVMDebugInfoDWARF LLVMObject
                LLVMTextAPI LLVMMCParser LLVMIRReader LLVMAsmParser LLVMMC LLVMBitReader LLVMFuzzerCLI LLVMCore
                LLVMRemarks LLVMBitstreamReader LLVMBinaryFormat LLVMTargetParser LLVMTableGen LLVMSupport LLVMDemangle)

message(STATUS "LLVM Libs : ${llvm_libs}")
target_link_libraries(kaleidoscope-compiler ${llvm_libs})
