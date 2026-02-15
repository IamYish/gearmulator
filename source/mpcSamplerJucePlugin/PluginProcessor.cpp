#include "PluginProcessor.h"
#include "PluginEditorState.h"
#include "mpcController.h"

// ReSharper disable once CppUnusedIncludeDirective
#include "BinaryData.h"
#include "jucePluginLib/processorPropertiesInit.h"

#include "mpcSamplerLib/samplerDevice.h"

namespace
{
	juce::PropertiesFile::Options getOptions()
	{
		juce::PropertiesFile::Options opts;
		opts.applicationName = "DSP56300EmulatorMPCSampler";
		opts.filenameSuffix = ".settings";
		opts.folderName = "DSP56300EmulatorMPCSampler";
		opts.osxLibrarySubFolder = "Application Support/DSP56300EmulatorMPCSampler";
		return opts;
	}
}

namespace mpcSamplerJucePlugin
{
	AudioPluginAudioProcessor::AudioPluginAudioProcessor()
		: Processor(BusesProperties()
			.withInput("Input", juce::AudioChannelSet::stereo(), true)
			.withOutput("Output", juce::AudioChannelSet::stereo(), true)
			, getOptions(), pluginLib::initProcessorProperties())
	{
		getController();
		const auto latencyBlocks = getConfig().getIntValue("latencyBlocks", static_cast<int>(getPlugin().getLatencyBlocks()));
		Processor::setLatencyBlocks(latencyBlocks);
	}

	AudioPluginAudioProcessor::~AudioPluginAudioProcessor()
	{
		destroyEditorState();
	}

	jucePluginEditorLib::PluginEditorState* AudioPluginAudioProcessor::createEditorState()
	{
		return new PluginEditorState(*this);
	}

	synthLib::Device* AudioPluginAudioProcessor::createDevice()
	{
		synthLib::DeviceCreateParams params;
		params.hostSamplerate = static_cast<float>(getSampleRate());
		params.preferredSamplerate = static_cast<float>(getSampleRate());
		return new mpcSamplerLib::SamplerDevice(params);
	}

	void AudioPluginAudioProcessor::getRemoteDeviceParams(synthLib::DeviceCreateParams& _params) const
	{
		Processor::getRemoteDeviceParams(_params);
	}

	pluginLib::Controller* AudioPluginAudioProcessor::createController()
	{
		return new Controller(*this);
	}
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
	return new mpcSamplerJucePlugin::AudioPluginAudioProcessor();
}
