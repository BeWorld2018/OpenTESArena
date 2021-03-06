#ifndef CHOOSE_RACE_PANEL_H
#define CHOOSE_RACE_PANEL_H

#include <string>

#include "Button.h"
#include "Panel.h"
#include "../Entities/CharacterClass.h"
#include "../Math/Vector2.h"
#include "../Rendering/Texture.h"

class Renderer;

class ChooseRacePanel : public Panel
{
private:
	// The mask ID for no selected province.
	static const int NO_ID;

	Button<Game&> backToGenderButton;
	Button<Game&, int> acceptButton;

	// Gets the initial parchment pop-up.
	static std::unique_ptr<Panel> getInitialSubPanel(Game &game);

	// Gets the mask ID associated with some pixel location, or "no ID" if none found.
	int getProvinceMaskID(const Int2 &position) const;

	void drawProvinceTooltip(int provinceID, Renderer &renderer);	
public:
	ChooseRacePanel(Game &game);
	virtual ~ChooseRacePanel() = default;

	virtual Panel::CursorData getCurrentCursor() const override;
	virtual void handleEvent(const SDL_Event &e) override;
	virtual void render(Renderer &renderer) override;
	virtual void renderSecondary(Renderer &renderer) override;
};

#endif
