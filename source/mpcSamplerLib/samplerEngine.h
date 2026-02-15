#pragma once

#include "samplerVoice.h"

#include <array>
#include <cstdint>
#include <mutex>
#include <string>

namespace mpcSamplerLib
{
	static constexpr uint32_t kNumPads = 16;
	static constexpr uint32_t kMaxVoices = 64;
	static constexpr uint32_t kDefaultVoices = 64;
	static constexpr uint8_t kDefaultBaseNote = 36;  // C1 - standard MPC pad mapping

	class SamplerEngine
	{
	public:
		SamplerEngine();
		~SamplerEngine() = default;

		void setSampleRate(float _sampleRate);
		float getSampleRate() const { return m_sampleRate; }

		void setMaxVoices(uint32_t _maxVoices);
		uint32_t getMaxVoices() const { return m_maxVoices; }

		void process(const float* const* _inputs, float* const* _outputs, size_t _numSamples);

		void noteOn(uint8_t _channel, uint8_t _note, uint8_t _velocity);
		void noteOff(uint8_t _channel, uint8_t _note);
		void allNotesOff();

		// Pad management
		void setPadSample(uint32_t _padIndex, SampleData&& _sample);
		void setPadConfig(uint32_t _padIndex, const PadConfig& _config);
		const PadConfig& getPadConfig(uint32_t _padIndex) const;
		const SampleData& getPadSample(uint32_t _padIndex) const;
		bool padHasSample(uint32_t _padIndex) const;

		void setMasterVolume(float _volume) { m_masterVolume = _volume; }
		float getMasterVolume() const { return m_masterVolume; }

		// Get pad index for a MIDI note, returns -1 if not mapped
		int getPadIndexForNote(uint8_t _note) const;

		uint32_t getActiveVoiceCount() const;

	private:
		SamplerVoice* allocateVoice();
		void killVoicesInMuteGroup(uint8_t _muteGroup);

		std::array<PadConfig, kNumPads> m_padConfigs;
		std::array<SampleData, kNumPads> m_padSamples;
		std::array<SamplerVoice, kMaxVoices> m_voices;

		float m_sampleRate = 44100.0f;
		float m_masterVolume = 1.0f;
		uint32_t m_maxVoices = kDefaultVoices;
	};
}
