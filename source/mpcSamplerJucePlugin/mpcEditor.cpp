#include "mpcEditor.h"

#include "PluginProcessor.h"
#include "mpcController.h"

#include "jucePluginEditorLib/focusedParameter.h"

namespace mpcSamplerJucePlugin
{
	Editor::Editor(jucePluginEditorLib::Processor& _processor, const jucePluginEditorLib::Skin& _skin)
		: jucePluginEditorLib::Editor(_processor, _skin)
		, m_controller(dynamic_cast<Controller&>(_processor.getController()))
	{
	}

	void Editor::create()
	{
		jucePluginEditorLib::Editor::create();

		m_focusedParameter.reset(new jucePluginEditorLib::FocusedParameter(m_controller, *this));
	}

	Editor::~Editor()
	{
		m_focusedParameter.reset();
	}

	std::pair<std::string, std::string> Editor::getDemoRestrictionText() const
	{
		return {"MPC Sampler",
			"MPC Sampler runs in demo mode\n"
			"\n"
			"The following features are disabled:\n"
			"- Saving/Exporting Presets\n"
			"- Plugin state is not preserved"
		};
	}
}
