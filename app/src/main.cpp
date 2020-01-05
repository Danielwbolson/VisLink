#include <iostream>
#include <VisLink/net/Server.h>
#include <VisLink/net/Client.h>

int main(int argc, char**argv) {
	vislink::VisLinkAPI* api = NULL;
	if (argc > 1) {
		api = new vislink::Client();
	}
	else {
		api = new vislink::Server();
	}

	api->getSharedTexture("hi");

	std::cout << "Hello World." << std::endl;
	std::cin.ignore();

	return 0;
}


