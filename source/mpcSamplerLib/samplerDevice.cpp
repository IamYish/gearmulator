#include "samplerDevice.h"

#include <cstring>

#include "synthLib/midiTypes.h"

namespace mpcSamplerLib
{
	SamplerDevice::SamplerDevice(const synthLib::DeviceCreateParams& _params) : Device(_params)
	{
		if (_params.preferredSamplerate > 0.0f)
			m_sampleRate = _params.preferredSamplerate;
		else if (_params.hostSamplerate > 0.0f)
			m_sampleRate = _params.hostSamplerate;

		m_engine.setSampleRate(m_sampleRate);
	}

	SamplerDevice::~SamplerDevice() = default;

	float SamplerDevice::getSamplerate() const
	{
		return m_sampleRate;
	}

	bool SamplerDevice::setSamplerate(const float _samplerate)
	{
		m_sampleRate = _samplerate;
		m_engine.setSampleRate(_samplerate);
		return true;
	}

	bool SamplerDevice::isValid() const
	{
		return true;
	}

	bool SamplerDevice::getState(std::vector<uint8_t>& _state, synthLib::StateType _type)
	{
		// Serialize pad configurations
		// Format: [numPads][padConfig0][padConfig1]...[padConfig15]
		_state.clear();
		_state.push_back(static_cast<uint8_t>(kNumPads));

		for (uint32_t i = 0; i < kNumPads; ++i)
		{
			const auto& config = m_engine.getPadConfig(i);
			_state.push_back(config.midiNote);

			// Store float values as bytes (simplified serialization)
			auto pushFloat = [&_state](float v)
			{
				const auto* bytes = reinterpret_cast<const uint8_t*>(&v);
				_state.insert(_state.end(), bytes, bytes + sizeof(float));
			};

			pushFloat(config.volume);
			pushFloat(config.pan);
			pushFloat(config.tune);
			pushFloat(config.attack);
			pushFloat(config.decay);
			pushFloat(config.sustain);
			pushFloat(config.release);
			_state.push_back(config.muteGroup);
			_state.push_back(config.oneShot ? 1 : 0);
		}

		return true;
	}

	bool SamplerDevice::setState(const std::vector<uint8_t>& _state, synthLib::StateType _type)
	{
		if (_state.empty())
			return false;

		const uint8_t numPads = _state[0];
		if (numPads != kNumPads)
			return false;

		size_t offset = 1;

		auto readFloat = [&_state, &offset]() -> float
		{
			if (offset + sizeof(float) > _state.size())
				return 0.0f;
			float v;
			std::memcpy(&v, &_state[offset], sizeof(float));
			offset += sizeof(float);
			return v;
		};

		for (uint32_t i = 0; i < kNumPads && offset < _state.size(); ++i)
		{
			PadConfig config;
			config.midiNote = _state[offset++];
			config.volume = readFloat();
			config.pan = readFloat();
			config.tune = readFloat();
			config.attack = readFloat();
			config.decay = readFloat();
			config.sustain = readFloat();
			config.release = readFloat();
			if (offset < _state.size())
				config.muteGroup = _state[offset++];
			if (offset < _state.size())
				config.oneShot = _state[offset++] != 0;

			m_engine.setPadConfig(i, config);
		}

		return true;
	}

	void SamplerDevice::readMidiOut(std::vector<synthLib::SMidiEvent>& _midiOut)
	{
		_midiOut.insert(_midiOut.end(), m_pendingMidiOut.begin(), m_pendingMidiOut.end());
		m_pendingMidiOut.clear();
	}

	void SamplerDevice::processAudio(const synthLib::TAudioInputs& _inputs, const synthLib::TAudioOutputs& _outputs, const size_t _samples)
	{
		const float* inputs[2] = {_inputs[0], _inputs[1]};
		float* outputs[2] = {_outputs[0], _outputs[1]};

		m_engine.process(inputs, outputs, _samples);
	}

	bool SamplerDevice::sendMidi(const synthLib::SMidiEvent& _ev, std::vector<synthLib::SMidiEvent>& _response)
	{
		const uint8_t status = _ev.a & 0xf0;
		const uint8_t channel = _ev.a & 0x0f;

		switch (status)
		{
		case synthLib::M_NOTEON:
			m_engine.noteOn(channel, _ev.b, _ev.c);
			return true;

		case synthLib::M_NOTEOFF:
			m_engine.noteOff(channel, _ev.b);
			return true;

		case synthLib::M_CONTROLCHANGE:
			if (_ev.b == synthLib::MC_ALLNOTESOFF || _ev.b == synthLib::MC_ALLSOUNDOFF)
			{
				m_engine.allNotesOff();
				return true;
			}
			if (_ev.b == synthLib::MC_MAINVOLUME)
			{
				m_engine.setMasterVolume(static_cast<float>(_ev.c) / 127.0f);
				return true;
			}
			return true;

		default:
			return true;
		}
	}
}
