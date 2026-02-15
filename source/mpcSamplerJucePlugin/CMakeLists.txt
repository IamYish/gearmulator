cmake_minimum_required(VERSION 3.15)
project(mpcSamplerJucePlugin VERSION ${CMAKE_PROJECT_VERSION})

set(SOURCES
	parameterDescriptions_mpc.json

	PluginEditorState.cpp PluginEditorState.h
	PluginProcessor.cpp PluginProcessor.h
	mpcController.cpp mpcController.h
	mpcEditor.cpp mpcEditor.h

	skins/mpcDefault/mpcDefault.rml
	skins/mpcDefault/mpcDefault.rcss
)

SET(ASSETS "parameterDescriptions_mpc.json")

addSkin("MPCSampler" "mpcDefault" "skins/mpcDefault" "mpcDefault.rml")

buildSkinHeader()

juce_add_binary_data(mpcSamplerJucePlugin_BinaryData SOURCES ${ASSETS} ${ASSETS_SKINS})

createJucePlugin(mpcSamplerJucePlugin "MPCSampler" TRUE "Tmps" mpcSamplerJucePlugin_BinaryData mpcSamplerLib)
