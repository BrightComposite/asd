//---------------------------------------------------------------------------

#include <iostream>
#include <chrono>
#include <random>

#include <application/starter.h>

#include <core/function/method.h>
#include <core/function/Thread.h>

#include <meta/xpack.h>

#include <graphics/Provider.h>

#include <freeimage/FreeImageConverter.h>
#include <freetype/FreeTypeDecoder.h>

#include <ui/Text.h>
#include <ui/UIPalette.h>
#include <ui/WidgetLayers.h>
#include <ui/Window.h>

#include <graphics/font/Font.h>

#ifdef WIN32
#include <windows/Message.h>
#include <vld.h>
#endif

//---------------------------------------------------------------------------

/**
 * [ ] Create graphics and windows from Display
 * [ ] Inject font and image libraries to graphics
 * [ ] Change global thread loop to event loop
 */

namespace asd
{
	static entrance open([]() {
		FreeTypeDecoder::initialize(); //! GLOBAL STATE
		FreeImageConverter::initialize(); //! GLOBAL STATE

		Color backgroundColor(0.2f, 0.2f, 0.2f);

		auto graphics = GraphicsProvider::provide(); /// Create graphics from Display
		graphics->setClearColor(backgroundColor);

		handle<Window> window(graphics, 0, 0, 800, 600, "asd::ui test"); /// MAKE FACTORY?
		window->setBackgroundColor(backgroundColor);

		StandartUIPalette palette(window);

		auto * w = palette.create("label", window->root());

		ColoredButtonDecorator decorator;
		decorator.background({0.4f, 0.4f, 0.4f});
		decorator.pressed({0.1f, 0.1f, 0.1f});
		decorator.hovered({0.6f, 0.6f, 0.6f});
		decorator.boundary({0.5f, 0.5f, 0.5f}, 1);
		decorator.apply(w);

		window->show();
		window->centralize();

		subscription(*window) {
//			onmessage(KeyUpMessage) {
//				switch(msg->key) {
//					/*case VK_ESCAPE:
//						dest.close();
//						break;*/
//				}
//			};

			onmessage(WindowCloseMessage) {
				thread_loop::stop();
			};
		}

#ifdef WIN32
		thread_loop::add(processWindowMessage);
#endif
		
		thread_loop::add(make_method(window, mouseUpdate));

/*
		thread th([&window]() {
			subscription(WindowCloseMessage, *window) {
				thread_loop::stop();
			};
*/
		thread_loop::add(make_method(window, update));
/*
			thread_loop::run();
		});
*/
		std::cout << "Run thread loop..." << std::endl;
		
		thread_loop::run();

		return 0;
	});
}

//---------------------------------------------------------------------------