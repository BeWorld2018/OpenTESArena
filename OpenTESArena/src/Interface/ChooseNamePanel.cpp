#include <cassert>
#include <unordered_map>

#include "SDL.h"

#include "ChooseNamePanel.h"

#include "Button.h"
#include "ChooseClassPanel.h"
#include "ChooseGenderPanel.h"
#include "TextAlignment.h"
#include "TextBox.h"
#include "../Entities/CharacterClass.h"
#include "../Game/Game.h"
#include "../Math/Int2.h"
#include "../Media/Color.h"
#include "../Media/FontManager.h"
#include "../Media/FontName.h"
#include "../Media/PaletteFile.h"
#include "../Media/PaletteName.h"
#include "../Media/TextureFile.h"
#include "../Media/TextureManager.h"
#include "../Media/TextureName.h"
#include "../Rendering/Renderer.h"
#include "../Rendering/Surface.h"
#include "../Rendering/Texture.h"

const int ChooseNamePanel::MAX_NAME_LENGTH = 25;

ChooseNamePanel::ChooseNamePanel(Game *game, const CharacterClass &charClass)
	: Panel(game)
{
	this->parchment = [game]()
	{
		auto &renderer = game->getRenderer();

		// Create placeholder parchment.
		SDL_Surface *surface = Surface::createSurfaceWithFormat(180, 40,
			Renderer::DEFAULT_BPP, Renderer::DEFAULT_PIXELFORMAT);
		SDL_FillRect(surface, nullptr, SDL_MapRGBA(surface->format, 166, 125, 81, 255));

		SDL_Texture *texture = renderer.createTextureFromSurface(surface);
		SDL_FreeSurface(surface);

		return texture;
	}();

	this->titleTextBox = [game, charClass]()
	{
		Int2 center(Renderer::ORIGINAL_WIDTH / 2, 90);
		Color color(48, 12, 12);
		std::string text = "What will be thy name,\n" + charClass.getDisplayName() + "?";
		auto &font = game->getFontManager().getFont(FontName::A);
		auto alignment = TextAlignment::Center;
		return std::unique_ptr<TextBox>(new TextBox(
			center,
			color,
			text,
			font,
			alignment,
			game->getRenderer()));
	}();

	this->nameTextBox = [game]()
	{
		Int2 center(Renderer::ORIGINAL_WIDTH / 2, 112);
		Color color(48, 12, 12);
		std::string text = "";
		auto &font = game->getFontManager().getFont(FontName::A);
		auto alignment = TextAlignment::Center;
		return std::unique_ptr<TextBox>(new TextBox(
			center,
			color,
			text,
			font,
			alignment,
			game->getRenderer()));
	}();

	this->backToClassButton = []()
	{
		auto function = [](Game *game)
		{
			std::unique_ptr<Panel> classPanel(new ChooseClassPanel(game));
			game->setPanel(std::move(classPanel));
		};
		return std::unique_ptr<Button>(new Button(function));
	}();

	this->acceptButton = [this, charClass]()
	{
		auto function = [this, charClass](Game *game)
		{
			std::unique_ptr<Panel> racePanel(new ChooseGenderPanel(
				game, charClass, this->name));
			game->setPanel(std::move(racePanel));
		};
		return std::unique_ptr<Button>(new Button(function));
	}();

	this->charClass = std::unique_ptr<CharacterClass>(new CharacterClass(charClass));
	this->name = std::string();
}

ChooseNamePanel::~ChooseNamePanel()
{
	SDL_DestroyTexture(this->parchment);
}

void ChooseNamePanel::handleEvent(const SDL_Event &e)
{
	bool escapePressed = (e.type == SDL_KEYDOWN) &&
		(e.key.keysym.sym == SDLK_ESCAPE);
	bool enterPressed = (e.type == SDL_KEYDOWN) &&
		((e.key.keysym.sym == SDLK_RETURN) || (e.key.keysym.sym == SDLK_KP_ENTER));

	if (escapePressed)
	{
		this->backToClassButton->click(this->getGame());
	}

	// Only accept the name if it has a positive size.
	if (enterPressed && (this->name.size() > 0))
	{
		this->acceptButton->click(this->getGame());
	}

	// Upper and lower case English characters.
	const std::unordered_map<SDL_Keycode, std::pair<char, char>> letters =
	{
		{ SDLK_a, { 'A', 'a' } },
		{ SDLK_b, { 'B', 'b' } },
		{ SDLK_c, { 'C', 'c' } },
		{ SDLK_d, { 'D', 'd' } },
		{ SDLK_e, { 'E', 'e' } },
		{ SDLK_f, { 'F', 'f' } },
		{ SDLK_g, { 'G', 'g' } },
		{ SDLK_h, { 'H', 'h' } },
		{ SDLK_i, { 'I', 'i' } },
		{ SDLK_j, { 'J', 'j' } },
		{ SDLK_k, { 'K', 'k' } },
		{ SDLK_l, { 'L', 'l' } },
		{ SDLK_m, { 'M', 'm' } },
		{ SDLK_n, { 'N', 'n' } },
		{ SDLK_o, { 'O', 'o' } },
		{ SDLK_p, { 'P', 'p' } },
		{ SDLK_q, { 'Q', 'q' } },
		{ SDLK_r, { 'R', 'r' } },
		{ SDLK_s, { 'S', 's' } },
		{ SDLK_t, { 'T', 't' } },
		{ SDLK_u, { 'U', 'u' } },
		{ SDLK_v, { 'V', 'v' } },
		{ SDLK_w, { 'W', 'w' } },
		{ SDLK_x, { 'X', 'x' } },
		{ SDLK_y, { 'Y', 'y' } },
		{ SDLK_z, { 'Z', 'z' } }
	};

	// Punctuation (some duplicates exist to keep the shift behavior for quotes).
	const std::unordered_map<SDL_Keycode, std::pair<char, char>> punctuation =
	{
		{ SDLK_COMMA, { ',', ',' } },
		{ SDLK_MINUS, { '-', '-' } },
		{ SDLK_PERIOD, { '.', '.' } },
		{ SDLK_QUOTE, { '"', '\'' } }
	};

	if (e.type == SDL_KEYDOWN)
	{
		const uint8_t *keys = SDL_GetKeyboardState(nullptr);
		const SDL_Keycode keyCode = e.key.keysym.sym;
		bool shiftPressed = keys[SDL_SCANCODE_LSHIFT] || keys[SDL_SCANCODE_RSHIFT];

		// See if the pressed key is a recognized letter.
		if (letters.find(keyCode) != letters.end())
		{
			// Add the letter to the name if there is room.
			if (this->name.size() < ChooseNamePanel::MAX_NAME_LENGTH)
			{
				const auto &pair = letters.at(keyCode);
				this->name.push_back(shiftPressed ? pair.first : pair.second);
			}
		}
		else if (punctuation.find(keyCode) != punctuation.end())
		{
			// The pressed key is recognized punctuation. Add it.
			if (this->name.size() < ChooseNamePanel::MAX_NAME_LENGTH)
			{
				const auto &pair = punctuation.at(keyCode);
				this->name.push_back(shiftPressed ? pair.first : pair.second);
			}
		}
		else if (keyCode == SDLK_SPACE)
		{
			// The pressed key is space. Add a space.
			if (this->name.size() < ChooseNamePanel::MAX_NAME_LENGTH)
			{
				this->name.push_back(' ');
			}
		}
		else if (keyCode == SDLK_BACKSPACE)
		{
			// The pressed key is backspace. Erase one letter if able.
			if (this->name.size() > 0)
			{
				this->name.pop_back();
			}
		}

		// Update the displayed name.
		this->nameTextBox = [this]
		{
			return std::unique_ptr<TextBox>(new TextBox(
				Int2(Renderer::ORIGINAL_WIDTH / 2, 112),
				Color(48, 12, 12),
				this->name,
				this->getGame()->getFontManager().getFont(FontName::A),
				TextAlignment::Center,
				this->getGame()->getRenderer()));
		}();
	}
}

void ChooseNamePanel::render(Renderer &renderer)
{
	// Clear full screen.
	renderer.clearNative();
	renderer.clearOriginal();

	// Set palette.
	auto &textureManager = this->getGame()->getTextureManager();
	textureManager.setPalette(PaletteFile::fromName(PaletteName::Default));

	// Draw background.
	const auto &background = textureManager.getTexture(
		TextureFile::fromName(TextureName::CharacterCreation),
		PaletteFile::fromName(PaletteName::BuiltIn));
	renderer.drawToOriginal(background.get());

	// Draw parchment: title.
	int parchmentWidth, parchmentHeight;
	SDL_QueryTexture(this->parchment, nullptr, nullptr, &parchmentWidth, &parchmentHeight);
	const double parchmentXScale = 1.5;
	const double parchmentYScale = 1.65;
	const int parchmentNewWidth = static_cast<int>(parchmentWidth * parchmentXScale);
	const int parchmentNewHeight = static_cast<int>(parchmentHeight * parchmentYScale);

	renderer.drawToOriginal(this->parchment,
		(Renderer::ORIGINAL_WIDTH / 2) - (parchmentNewWidth / 2),
		(Renderer::ORIGINAL_HEIGHT / 2) - (parchmentNewHeight / 2),
		parchmentNewWidth,
		parchmentNewHeight);

	// Draw text: title, name.
	renderer.drawToOriginal(this->titleTextBox->getTexture(),
		this->titleTextBox->getX(), this->titleTextBox->getY());
	renderer.drawToOriginal(this->nameTextBox->getTexture(),
		this->nameTextBox->getX(), this->nameTextBox->getY());

	// Scale the original frame buffer onto the native one.
	renderer.drawOriginalToNative();

	// Draw cursor.
	const auto &cursor = textureManager.getTexture(
		TextureFile::fromName(TextureName::SwordCursor));
	auto mousePosition = this->getMousePosition();
	renderer.drawToNative(cursor.get(),
		mousePosition.getX(), mousePosition.getY(),
		static_cast<int>(cursor.getWidth() * this->getCursorScale()),
		static_cast<int>(cursor.getHeight() * this->getCursorScale()));
}
