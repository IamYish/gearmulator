#include "samplerEngine.h"

#include <algorithm>
#include <cstring>

namespace mpcSamplerLib
{
	SamplerEngine::SamplerEngine()
	{
		// Initialize default pad MIDI note mappings (C1 to D#2, notes 36-51)
		for (uint32_t i = 0; i < kNumPads; ++i)
		{
			m_padConfigs[i].midiNote = kDefaultBaseNote + static_cast<uint8_t>(i);
		}
	}

	void SamplerEngine::setSampleRate(const float _sampleRate)
	{
		m_sampleRate = _sampleRate;
	}

	void SamplerEngine::setMaxVoices(const uint32_t _maxVoices)
	{
		m_maxVoices = std::clamp(_maxVoices, 32u, kMaxVoices);
	}

	void SamplerEngine::process(const float* const* /*_inputs*/, float* const* _outputs, const size_t _numSamples)
	{
		if (!_outputs || !_outputs[0] || !_outputs[1])
			return;

		float* outL = _outputs[0];
		float* outR = _outputs[1];

		// Clear output buffers
		std::memset(outL, 0, _numSamples * sizeof(float));
		std::memset(outR, 0, _numSamples * sizeof(float));

		// Process all active voices
		for (uint32_t v = 0; v < m_maxVoices; ++v)
		{
			auto& voice = m_voices[v];
			if (!voice.isActive())
				continue;

			voice.process(outL, outR, _numSamples);
			voice.incrementAge();
		}

		// Apply master volume
		if (m_masterVolume != 1.0f)
		{
			for (size_t i = 0; i < _numSamples; ++i)
			{
				outL[i] *= m_masterVolume;
				outR[i] *= m_masterVolume;
			}
		}
	}

	void SamplerEngine::noteOn(const uint8_t /*_channel*/, const uint8_t _note, const uint8_t _velocity)
	{
		if (_velocity == 0)
		{
			noteOff(0, _note);
			return;
		}

		const int padIdx = getPadIndexForNote(_note);
		if (padIdx < 0)
			return;

		const auto& config = m_padConfigs[padIdx];
		const auto& sample = m_padSamples[padIdx];

		if (sample.isEmpty())
			return;

		// Kill voices in same mute group
		if (config.muteGroup > 0)
			killVoicesInMuteGroup(config.muteGroup);

		auto* voice = allocateVoice();
		if (!voice)
			return;

		voice->setPadIndex(static_cast<uint8_t>(padIdx));
		voice->noteOn(_note, _velocity, &sample, config, m_sampleRate);
	}

	void SamplerEngine::noteOff(const uint8_t /*_channel*/, const uint8_t _note)
	{
		for (uint32_t v = 0; v < m_maxVoices; ++v)
		{
			auto& voice = m_voices[v];
			if (voice.isActive() && voice.getNote() == _note)
				voice.noteOff();
		}
	}

	void SamplerEngine::allNotesOff()
	{
		for (uint32_t v = 0; v < m_maxVoices; ++v)
			m_voices[v].forceStop();
	}

	void SamplerEngine::setPadSample(const uint32_t _padIndex, SampleData&& _sample)
	{
		if (_padIndex >= kNumPads)
			return;
		m_padSamples[_padIndex] = std::move(_sample);
	}

	void SamplerEngine::setPadConfig(const uint32_t _padIndex, const PadConfig& _config)
	{
		if (_padIndex >= kNumPads)
			return;
		m_padConfigs[_padIndex] = _config;
	}

	const PadConfig& SamplerEngine::getPadConfig(const uint32_t _padIndex) const
	{
		return m_padConfigs[std::min(_padIndex, kNumPads - 1)];
	}

	const SampleData& SamplerEngine::getPadSample(const uint32_t _padIndex) const
	{
		return m_padSamples[std::min(_padIndex, kNumPads - 1)];
	}

	bool SamplerEngine::padHasSample(const uint32_t _padIndex) const
	{
		if (_padIndex >= kNumPads)
			return false;
		return !m_padSamples[_padIndex].isEmpty();
	}

	int SamplerEngine::getPadIndexForNote(const uint8_t _note) const
	{
		for (uint32_t i = 0; i < kNumPads; ++i)
		{
			if (m_padConfigs[i].midiNote == _note)
				return static_cast<int>(i);
		}
		return -1;
	}

	uint32_t SamplerEngine::getActiveVoiceCount() const
	{
		uint32_t count = 0;
		for (uint32_t v = 0; v < m_maxVoices; ++v)
		{
			if (m_voices[v].isActive())
				++count;
		}
		return count;
	}

	SamplerVoice* SamplerEngine::allocateVoice()
	{
		// First, find a free voice
		for (uint32_t v = 0; v < m_maxVoices; ++v)
		{
			if (!m_voices[v].isActive())
				return &m_voices[v];
		}

		// Voice stealing: find the oldest releasing voice
		SamplerVoice* oldest = nullptr;
		uint32_t maxAge = 0;

		for (uint32_t v = 0; v < m_maxVoices; ++v)
		{
			if (m_voices[v].isReleasing() && m_voices[v].getAge() >= maxAge)
			{
				maxAge = m_voices[v].getAge();
				oldest = &m_voices[v];
			}
		}

		// If no releasing voice, steal the oldest active voice
		if (!oldest)
		{
			for (uint32_t v = 0; v < m_maxVoices; ++v)
			{
				if (m_voices[v].getAge() >= maxAge)
				{
					maxAge = m_voices[v].getAge();
					oldest = &m_voices[v];
				}
			}
		}

		if (oldest)
			oldest->forceStop();

		return oldest;
	}

	void SamplerEngine::killVoicesInMuteGroup(const uint8_t _muteGroup)
	{
		if (_muteGroup == 0)
			return;

		for (uint32_t v = 0; v < m_maxVoices; ++v)
		{
			auto& voice = m_voices[v];
			if (voice.isActive() && voice.getMuteGroup() == _muteGroup)
				voice.forceStop();
		}
	}
}
