#pragma once

#include "jucePluginEditorLib/pluginEditor.h"

namespace jucePluginEditorLib
{
	class FocusedParameter;
	class Processor;
}

namespace mpcSamplerJucePlugin
{
	class Controller;

	class Editor final : public jucePluginEditorLib::Editor
	{
	public:
		Editor(jucePluginEditorLib::Processor& _processor, const jucePluginEditorLib::Skin& _skin);
		~Editor() override;

		Editor(Editor&&) = delete;
		Editor(const Editor&) = delete;
		Editor& operator = (Editor&&) = delete;
		Editor& operator = (const Editor&) = delete;

		void create() override;

		std::pair<std::string, std::string> getDemoRestrictionText() const override;

		Controller& getMpcController() const { return m_controller; }

	private:
		Controller& m_controller;
		std::unique_ptr<jucePluginEditorLib::FocusedParameter> m_focusedParameter;
	};
}
