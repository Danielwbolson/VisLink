#ifndef VISLINK_DISPLAY_TEXTURE_DISPLAY_H_
#define VISLINK_DISPLAY_TEXTURE_DISPLAY_H_

#include "VisLink/image/Texture.h"
#include "VisLink/display/Display.h"

namespace vislink {

struct TextureDisplayState;

class TextureDisplay : public Display{
public:
	TextureDisplay(Texture tex, int width, int height, int xPos = 0, int yPos = 0, int xTexOffset = 0, int yTexOffset = 0);
	TextureDisplay(Texture left, Texture right, int width, int height, int xPos = 0, int yPos = 0, int xTexOffset = 0, int yTexOffset = 0);
	virtual ~TextureDisplay();

	void init();
	void render();
	void finish();
	void display();
	void useContext();
	void releaseContext();

private:
	void renderTexture();

	Texture left, right;
	int xPos;
	int yPos;
	int width;
	int height;
	int xTexOffset;
	int yTexOffset;
	bool stereo;
	TextureDisplayState* state;
	int frame;
};

}

#endif