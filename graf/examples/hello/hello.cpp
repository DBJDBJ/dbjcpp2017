#include "../../tigr.h"
#include "hello.h"




int main(int argc, char *argv[])
{
#if 0
	// on empty pointer handlers show the msg box and exit the app
	dbj::tf::SimpleHandler fnt(
		tigrLoadImageMem( callibiri_font_bytes.data(), callibiri_font_bytes.size()) );

	// font = tfont; // tfont is inbuilt default font
	dbj::tf::FontHandler font(tigrLoadFont(fnt, 1252));
#endif	
	dbj::tf::SimpleHandler screen(tigrWindow(320, 240, "Hello", 0));
	while (!tigrClosed(screen) && !tigrKeyDown(screen, TK_ESCAPE))
	{
		tigrClear(  screen, tigrRGB(0x80, 0x90, 0xa0));
		tigrPrint(  screen, tfont, 120, 110, tigrRGB(0xff, 0xff, 0xff), "Hello, world.");
		tigrUpdate( screen );
	}
	return 0;
}