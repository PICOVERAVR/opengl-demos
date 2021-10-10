#include "header.h"

int main(int argc, char **argv) {
	
	glm::vec4 vec(1.0f, 0.0f, 0.0f, 1.0f);
	glm::mat4 trans = glm::mat4(1.0f); //set to identity matrix
	
	trans = glm::translate(trans, glm::vec3(1.0f, 1.0f, 0.0f));
	//trans = glm::rotate(trans, glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
	
	vec = trans * vec;
	
	cout << vec.x << " " << vec.y << " " << vec.z << endl;
    
		
	return 0;
}
