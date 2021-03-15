#include "prompt.h"
using std::cout;
using std::cin;
using std::endl;
using std::string;

extern unit models[modelsize];
extern std::mutex modelmut;

void listen() {
	cout << "Enter commands to move units." << endl;
	while (true) {
		cout << "> ";
		string s;
		cin >> s;
		
		string op = s.substr(0, s.find_first_of(" "));
		if (!op.compare("exit")) {
			cout << "exiting input thread." << endl;
			break;
		} else if (!op.compare("move")) {
			cout << "new x: ";
			float xdest;
			cin >> xdest;

			cout << "new z: ";
			float zdest;
			cin >> zdest;
	
			modelmut.lock();
			models[u0].travelto(glm::vec3(xdest, 0.0, zdest));
			modelmut.unlock();
		} else {
			cout << "unknown command!" << endl;
		}
	}
}
